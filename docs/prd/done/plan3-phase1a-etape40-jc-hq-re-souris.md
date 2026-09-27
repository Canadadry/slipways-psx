---
title: "Plan 3, étape 1a.40 : le `re={ra,p}` de `jc:hq` devient une struct sans parent à deux membres `ra`/`p`, allouée à chaque appel"
description: "Quand le curseur d'un slipway tenu ne survole rien qui réponde à `gv`, `pico8_jc_hq` alloue à chaque frame une instance sans classe (tag `INSTANCE`, table `{ra,p}`, jamais libérée, TROU-7) ; elle devient une struct sans parent à deux membres `ra` et `p`, toujours allouée par appel parce qu'elle porte des valeurs propres à chaque appel, avec un lecteur `ra`/`p`, sans écrivain ni méthode."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Étapes de la phase 1a ») :

> 1a.40 — `re={ra,p}` : sans classe (`slipway.c:112`).

Le numéro de ligne n'est plus le bon. À HEAD fd28da6, le `{ra,p}` est construit dans `pico8_jc_hq`, `src/pico8_slipway.c:155-165`, sur la branche où l'entité survolée ne répond pas à `gv` :

- `:156` `malloc` d'un `pico8_instance_t` ; `:157` `malloc` d'un `pico8_vec_t` (`p_heap`) ;
- `:158-160` tag `INSTANCE`, `cls` NULL, `fields = pico8_ob_table_new()` ;
- `:161` `ra = mp`, **la REF reçue**, pas une copie ;
- `:162-163` `*p_heap = mp/128`, `p = REF(p_heap)` ;
- `:164-166` rangé dans `self->re`.

Le `/* TROU-7 */` qui marque cette fuite est au-dessus du contrat de `pico8_jc_hq`, `src/pico8_slipway.h:59` (le contrat, `:60-64`, dit « a fresh classless `{ra,p}` instance »).

C'est le port littéral de `pico8/lua.lua:622` :

```lua
function jc:hq(mp)self.re=ok.o_.gv and ok.o_ or {ra=mp,p=mp/128}
```

`ra` et `p` sont des **clés nommées**, pas des positionnels : la notation `re={ra,p}` du plan est un raccourci. La règle « positionnels → membre liste » (1a.34, `lq`) ne s'applique donc pas ; ce sont deux champs de l'instance posés à la construction.

Appelant unique : `ui:bh` (`lua.lua:244`, `pico8_ui.c:464`), `mv(held,"hq",self.qa,je.ra)` : `mp` est `ui.qa`, un vecteur neuf à chaque frame (`self.qa=je.ra+mo.ra`, `lua.lua:218`, `pico8_ui.c:310-314`, lui-même jamais libéré). `ra` partage donc la REF du `qa` de la frame, comme en Lua.

Règles du §5 qui s'appliquent :

> Chaque classe devient un `struct` […] ; chaque champ de sa chaîne (défauts et champs écrits sur instance) devient un membre de type `pico8_value_t`, **nom minifié conservé**. Nil reste `pico8_value_ref(NULL)`.
>
> **Instances sans classe** : converties comme des classes à part entière, sans parent.
>
> **Coexistence** : une instance convertie porte un nouveau tag (`PICO8_TYPE_TAG_STRUCT`) et un pointeur vers sa table de méthodes, dans le même premier emplacement qu'aujourd'hui.

§7 : un seul commit ; `make class-coverage-net` identique à `tools/class_coverage_net_expected.txt` ; preuve par `gcov` et 2 à 3 mutations ; note « Découvert à 1a.40 » réduite à ce qui change une étape suivante ; un écart port/cart découvert est préservé par un `TROU-N`, jamais corrigé.

### Précédents réutilisés

- **`et={}` de `bv` (1a.38, 493d9b0)** et **`{qs="-too far-"}` de `jc:ce` (1a.39, 2cf67b9)** : objet jetable par appel, devenu struct sans parent avec un descripteur `static const`, un lecteur limité à ses membres, un accesseur public du descripteur (`pico8_bv_et_struct_class()`, `pico8_jc_too_far_struct_class()`), tests adaptés dans leur seule façon de reconnaître l'objet, constante de pliage repliée à l'identique pour garder les empreintes.
- **Enveloppe `mz.jj` (1a.11)** : descripteur **sans écrivain** (`set_field` NULL), parce qu'aucun écrivain générique n'atteint l'objet.
- **La différence avec 1a.38/1a.39 est la forme d'allocation** : ces deux objets ne portaient rien de propre à l'appel (un `{}`, un `qs` fixe, plus un `fo` que personne ne relit), et sont devenus un objet statique unique. Le `{ra,p}` porte la position du curseur de *cet* appel. Voir la décision « par appel » ci-dessous.

### Emprise (HEAD fd28da6, lecture du code et grep ; rien n'a été lancé)

| Aspect | Constat |
|---|---|
| Construction | 1 site : `pico8_jc_hq`, `slipway.c:155-165`. Ni classe, ni `pico8_class_instantiate`, ni `lt` : jamais dans `hv`/`cv`/`cg`/`ic`, jamais compté par le crochet de couverture. |
| Vie / propriété | jamais libéré (TROU-7), ni lui ni son `p_heap`. Seule REF gardée : `self.re` du `jc` qui a appelé `hq`, écrasée au `hq` suivant (le `{ra,p}` précédent est abandonné) ou par `jc:ey` sur succès (`self.re = re`, la planète). Sur un `ey` refusé (`p_`), `re` reste le `{ra,p}` jusqu'à ce que `dg` retire le `jc` au début de la frame suivante (`lua.lua:1187`, `dg()` avant `ok:lj()`). |
| Champs | `ra` = REF `mp` (partagée avec `ui.qa` de la frame, jamais modifiée en place) ; `p` = REF d'un vecteur neuf `mp/128` (zéro si `mp` n'est pas une REF de vecteur, par `pico8_jc_value_vec`). Si `mp` est nil, le port stocke aujourd'hui une entrée `ra` nil **présente** dans la table (`pico8_ob_table_set` ne retire pas), là où Lua n'aurait pas de clé ; en jeu `mp` n'est jamais nil (`qa` est posé par `ui:lj` avant `bh`). |
| Écrivains | construction seule. Aucun site de `src/` n'écrit un nom sur `re` : `ey` écrit sur son *argument* (`re:kf`), `bv`/`jc:hq` écrivent `fo` sur `et` (jamais `re`), `na` sur `self.ra` d'une planète. |
| Lecteurs (tous par `pico8_entity_get_field`, déjà à double forme) | `ra` : `pico8_gd` (`entity_geom.c:55`, via `jc:kj`, `slipway_display.c:182`), `pico8_jc_lg` (`slipway_display.c:156`, extrémité du trait), `pico8_jc_ch` (`slipway_display.c:118-119`, particules). `p` : `pico8_dj` (`economy_formulas.c:165`, coût `od`), `pico8_jc_ce` (`slipway.c:291`, portée et segment), prédicat `jc` de `ce` (`slipway.c:271-272`, `sw.re.p`), prédicat `jc` de `bv` (`probe.c:156-158`, `sw.re.p`, la double forme de 1a.13). Noms absents lus : `mt` (`slipway.c:174`), `id` (`slipway_display.c:112`). |
| Identité | `jc:ev` compare `qu ~= re` (`slipway.c:381`) ; `ce` compare `w:eb(qu) == self.re` (`slipway.c:317`, `w` ∈ `qu.oe`, slipways posés dont `re` est une planète) ; `eb` rend `self.re` au prédicat ci-dessus. Aucune de ces comparaisons ne peut rendre vrai avec un `{ra,p}`, qu'il soit partagé ou non. |
| Méthodes | aucune. Aucun `mv(re, …)` n'atteint le `{ra,p}` (l'argument de `ey` vient de `jz("gv", held)`, une entité enregistrée). |
| Tests qui l'atteignent (lu, non lancé) | `test_pico8_jc` seul : `is_throwaway` (`:144`, tag `INSTANCE` + `cls` NULL) → `re_kind` (`:195`, `RE_THROWAWAY`) → `fold_jc` (`:289-293`, catégorie, `fold_value(re)` = constante `0x7A` par `is_throwaway`, puis `fold_vec(re, "p")`). Pinné par : `test_hq` (`far`, `HQ_HASH`), `test_too_far` (`far` puis `near`, `kj` de `far` sous la caméra = extrémité à `re.ra`, `ev` de `near` = `od` depuis `re.p`, `re` de `far` inchangé après un `ey` refusé, `fold_jc(far)` **après** le `hq` de `near` : `TOO_FAR_HASH`), et peut-être les rejeux `drag`/`hub`/`playthrough` (H2). `test_pico8_jc` ne sait pas encore « reconnaître » un `{ra,p}` autrement que par le tag. |
| Autres tests | `test_pico8_mz`, `test_pico8_g_`/`qh`/`hj`/`os` lisent `re.p` de slipways **posés** (planète) : non concernés. Le repli de `fold_value` de `test_pico8_g_`/`qh` qui plie `pico8_type_tag_of` d'une REF inconnue n'est appliqué, à la lecture, à aucun `re` de `jc` (H3). |
| Couverture | aucun nom : absent de `KNOWN[]` et de `tools/class_coverage_net_expected.txt`, qui ne doivent pas bouger. |
| Écart port/cart | la fuite (TROU-7) est propre au port. L'entrée `ra` nil présente (ligne « Champs ») n'est observée par aucun lecteur de `src/` : tous traitent « présent nil » comme « absent ». |

**Hypothèses non mesurées (consigne : aucun test lancé pendant la rédaction), à vérifier par l'implémenteur et reportées dans le message de commit :**

- H1. Le filet (`make e2e`, `frames-check`) atteint très probablement ce `{ra,p}` : tout glisser de slipway passe au-dessus du vide avant d'atteindre une planète. Toute régression d'extrémité de trait ou de coût s'y verrait. Rien n'est à mesurer au-delà du vert du filet.
- H2. Les rejeux `drag`/`hub`/`playthrough` de `test_pico8_jc` plient probablement un `jc` tenu dont `re` est le `{ra,p}`. Avec un reconnaisseur adapté qui rend `RE_THROWAWAY` et replie la même constante `0x7A`, et `p` lu par `pico8_entity_get_field`, leurs empreintes ne bougent pas qu'ils l'atteignent ou non.
- H3. `test_pico8_jc` est le seul fichier de test à changer. Si un autre test rougit : soit une reconnaissance par le tag (adapter de même, sans toucher d'empreinte, précédent `is_bv_target` de `test_pico8_g_`), soit un changement de comportement (corriger le code).
- H4. Aucun lecteur ne distingue `ra` « présent nil » de « absent » (vérifié par lecture : `pico8_jc_entity_value`, `pico8_entity_geom_field_vec`, `pico8_jc_display_entity_vec` rendent la même valeur par défaut). Si l'implémenteur en trouve un, c'est un écart à préserver (TROU-N), pas à corriger.

## Solution

1. **Struct sans parent, deux membres** : `pico8_struct_header_t`, puis `ra` et `p` (`pico8_value_t`), noms minifiés conservés. Pas de `pico8_ke_t`, pas de liste.
2. **Descripteur `static const`** : aucune méthode ; un lecteur qui sert `ra` (absent quand il est nil, comme tout membre) et `p`, et rend 0 pour tout autre nom, sans assert ; **pas d'écrivain** (`set_field` NULL, précédent `mz.jj`) : aucune écriture générique n'atteint l'objet, et une écriture de test avorte sur l'assert de `pico8_entity_set_field`.
3. **Allocation par appel, comme aujourd'hui** : la branche « rien de survolable » de `pico8_jc_hq` alloue une struct neuve (`malloc`), tag `STRUCT`, descripteur, `ra = mp` (la même REF, pas une copie), `p = REF` d'un vecteur neuf `mp/128` (le `p_heap` actuel, inchangé). Plus de `pico8_instance_t` ni de `pico8_ob_table_new`. Rien n'est libéré : le `/* TROU-7 */` de `pico8_jc_hq` reste, l'entrée TROU-7 aussi.
4. **Accesseur public du descripteur** dans `pico8_slipway.h` (nom au choix, par ex. `pico8_jc_re_struct_class()`), seul moyen pour les tests de reconnaître l'objet. Pas d'accesseur d'objet.
5. **Lecteurs inchangés** : `gd`, `lg`, `ch`, `dj`, `ce`, les deux prédicats `jc` lisent déjà par `pico8_entity_get_field`, qui aiguille sur le tag.
6. **Contrats rendus faux corrigés, rien d'autre** : le contrat de `pico8_jc_hq` dans `pico8_slipway.h` (« a fresh classless `{ra,p}` instance » → une struct `{ra,p}` neuve, jamais libérée). Le reste du fichier ne bouge pas.
7. **Tests adaptés dans leur façon de lire, pas dans leur intention** : dans `test_pico8_jc`, la reconnaissance du `{ra,p}` passe par l'accesseur du descripteur ; `re_kind` rend toujours `RE_THROWAWAY` pour lui, `fold_value` plie toujours `0x7A`. Aucune empreinte, aucun `#define …_HASH`, aucune valeur figée édité.
8. **Plan** : à la ligne 1a.40 de `docs/plan-3.md`, numéro de ligne corrigé si l'implémenteur le souhaite, et « Découvert à 1a.40 : » réduit à ce qui change une étape suivante — a priori : struct à deux membres `ra`/`p` allouée par appel (lecteur `ra`/`p`, sans écrivain ni méthode), accesseur public du descripteur ; enum inchangée ; après ce commit, plus aucune instance sans classe n'est construite dans `pico8_slipway.c`, et `is_throwaway` de `test_pico8_jc` ne reconnaît plus que la forme struct ; la condition de l'assert de `pico8_jc_hq` et le type `pico8_instance_t *` des paramètres restent jusqu'à 1a.42.

## User Stories

1. En tant qu'autopilote de la phase 1a, je veux que le `{ra,p}` de `jc:hq` soit la seule chose convertie par ce commit, afin de respecter « une classe par commit » (§7).
2. En tant qu'autopilote, je veux le site actuel retrouvé (la ligne du plan a bougé), afin de ne pas convertir un autre objet par erreur.
3. En tant qu'autopilote, je veux l'emprise établie (construction, vie, champs, écrivains, lecteurs, identité, tests), afin que la forme choisie soit justifiée.
4. En tant que lecteur du plan, je veux que l'objet devienne une struct sans parent, afin d'appliquer « converties comme des classes à part entière, sans parent ».
5. En tant que mainteneur, je veux `ra` et `p` en membres nommés, parce que ce sont des clés nommées du cart et non des positionnels.
6. En tant que mainteneur, je veux que `ra` reste la REF reçue, afin que le `{ra,p}` partage son vecteur avec `ui.qa` comme la table Lua partage `mp`.
7. En tant que mainteneur, je veux que `p` reste un vecteur neuf par appel, afin qu'un `{ra,p}` ancien garde sa position quand un autre `hq` en crée un nouveau.
8. En tant que mainteneur, je veux que chaque `hq` au-dessus du vide rende un objet distinct, parce que l'objet porte la position de cet appel et que deux `jc` peuvent en porter un à la fois (les tests le font).
9. En tant que joueur qui tire un slipway au-dessus du vide, je veux que le trait finisse au curseur, comme aujourd'hui.
10. En tant que joueur qui tire un slipway au-dessus du vide, je veux voir le même coût `od$` au curseur, comme aujourd'hui.
11. En tant que joueur qui tire un slipway trop loin, je veux voir « -too far- » comme aujourd'hui (la portée se mesure sur `re.p`).
12. En tant que joueur qui tire un slipway à travers une planète ou un slipway, je veux la même cible `et` qu'aujourd'hui (le segment se mesure sur `re.p`).
13. En tant que joueur qui relâche un slipway au-dessus du vide, je veux que la pose soit refusée sans rien dépenser, comme aujourd'hui.
14. En tant que joueur qui relâche un slipway sur une planète, je veux que `re` devienne cette planète, comme aujourd'hui.
15. En tant que joueur qui promène une sonde pendant qu'un slipway est visible, je veux la même cible qu'aujourd'hui (le prédicat `jc` de `bv` lit `sw.re.p`).
16. En tant que gardien des particules, je veux que `ch` lise `ra` et un `id` absent sur le `{ra,p}` comme aujourd'hui, afin de ne pas désynchroniser le `rnd`.
17. En tant que mainteneur, je veux qu'une écriture générique sur le `{ra,p}` arrête le programme, afin qu'un champ jamais mesuré ne soit pas rangé en silence.
18. En tant que mainteneur, je veux que l'objet n'ait aucune méthode, afin qu'un `mv` sur lui rende « absent » comme sur la table du cart.
19. En tant que gardien de la couverture, je veux qu'aucun nom ne soit ajouté à `KNOWN[]` ni compté, afin que `make class-coverage-net` reste identique à la référence.
20. En tant que gardien des tests, je veux que `test_pico8_jc` ne change que sa façon de reconnaître l'objet, avec ses empreintes intactes.
21. En tant que vérificateur, je veux une caractérisation figée sur l'ancien code de ce qu'aucun test ne pinne directement (partage de `ra` avec `mp`, `p` = `mp/128`, un objet neuf par appel, l'ancien intact, absence de `mt`/`id`, hors registres), afin que la conversion ne le casse pas sans bruit.
22. En tant que vérificateur, je veux un rapport `gcov` et 2 à 3 mutations ciblées, afin de juger la preuve sans monter mon propre banc.
23. En tant que vérificateur, je veux un journal de chaque lancement de test (commande, état de l'arbre, sortie, ligne clé, durée), afin de réutiliser les résultats au lieu de tout relancer.
24. En tant que gardien du filet, je veux qu'aucun `.sha256` ni aucune entrée de `FRAMES_EVENTS`/`RECORDINGS` ne bouge.
25. En tant qu'auteur des étapes 1a.41 et 1a.42, je veux que l'enveloppe de `g_ha`, `mz.jj`, la cible de `bv`, le `{qs}` de `ce` et les `pico8_class_t` restent intacts.
26. En tant que lecteur du code, je veux que le contrat de `pico8_jc_hq` dise vrai après ce commit, et qu'aucun commentaire de récit de chantier ne soit ajouté (règle des quatre destinations de CLAUDE.md).
27. En tant qu'autopilote, je veux qu'un écart port/cart découvert soit préservé par un `TROU-N`, jamais corrigé.

## Implementation Decisions

Aucun humain n'est disponible et cette session n'a pas d'outil pour lancer un agent frais : j'ai chargé et appliqué moi-même le skill `guideline` (hypothèses explicites, simplicité, changement chirurgical, critères vérifiables). Décisions tranchées ainsi marquées (G).

- **Par appel, pas statique** (G). Le cart fabrique un `{ra,p}` neuf à chaque `hq` au-dessus du vide ; contrairement au `{}` de `bv` et au `{qs}` de `ce`, il porte des **valeurs propres à l'appel**. En jeu, un seul `jc` tenu à la fois (seuls `jo:ldown`/`nr:ldown` rendent un `jc`, seulement dans `lheld`), et un `jc` refusé est retiré par `dg` avant le `hq` suivant : un statique partagé serait peut-être inobservable **sur le filet**, mais pas prouvablement, et il est **observable dans les tests figés** : `test_too_far` fait un `hq` de `far` vers (-640,-640), puis un `hq` de `near` près de `origin`, puis plie `far.re.p` dans `TOO_FAR_HASH` ; avec un objet partagé, `far.re.p` prendrait la valeur de `near` et l'empreinte figée changerait (critère d'échec). Allouer par appel garde le comportement et l'identité exacts, pour le coût d'aujourd'hui (même fuite, TROU-7). Rejeté : statique, ou tampon tournant.
- **Membres nommés, pas une liste** (G) : `{ra=mp,p=mp/128}` n'a pas de positionnels ; la liste de 1a.34 était pour `lq`, dont les champs sont positionnels. Deux membres gardent les lecteurs littéraux (`re.ra`, `re.p`).
- **`ra` partage la REF de `mp`** : c'est le sens de `{ra=mp}` en Lua. Pas de copie.
- **`p` garde son vecteur alloué à part** (G) : c'est le `p_heap` d'aujourd'hui, ligne inchangée. L'embarquer dans la struct serait possible (même sémantique, un `malloc` de moins), mais c'est un changement que l'étape ne demande pas (ce sera la phase 1b, où `p` prend un type `pico8_vec_t`).
- **`ra` nil lu absent** (G) : convention de toutes les structs converties (un membre nil est absent). Aujourd'hui l'entrée nil est présente, mais aucun lecteur de `src/` ne fait la différence (H4), et `mp` n'est jamais nil en jeu ; le cart n'aurait pas de clé non plus. Ce n'est donc pas corriger un écart observable. `p` est toujours présent (toujours un vecteur).
- **Pas d'écrivain** (G) : aucun écrivain de `src/` n'atteint l'objet (table « Emprise »). `set_field` NULL, l'assert existant de `pico8_entity_set_field` arrête le programme.
- **Lecteur** : pas d'assert sur un nom inconnu (`mt`, `id` sont lus absents par le code ; les tests interrogent des noms absents). Il ne peut pas être NULL (`pico8_entity_get_field` l'exige).
- **Nom C** : le cart ne nomme pas ce `{ra,p}` au-delà du champ `re` qui le reçoit ; un nom C rattaché à `jc:hq` (par ex. « point de souris de `jc:hq` »), au choix de l'implémenteur, sans nom minifié inventé ni entrée de glossaire.
- **Assert de `pico8_jc_hq` inchangé** (G) : il porte sur `et`, pas sur `re`. Sa condition (`INSTANCE || STRUCT`) et son message restent.
- **`TROU-7` gardé** au contrat de `pico8_jc_hq` : l'objet et son `p` fuient toujours. `ui.qa` fuit aussi, hors de cette étape.
- **Signatures inchangées** : `pico8_jc_hq` prend toujours `pico8_value_t mp` ; aucun type de paramètre ne change (1a.42).
- **Commentaires** (CLAUDE.md) : au plus un contrat C sur la struct (allouée par `hq`, jamais libérée, `ra` partagé avec l'appelant) ; le contrat de `pico8_jc_hq` corrigé ; tout le reste dans le message de commit.

## Testing Decisions

Un bon test ici lit le comportement externe (champs par `pico8_entity_get_field`, écran, crédits, identité des REF que le cart partage), pas la disposition mémoire ; seule la reconnaissance de l'objet passe par l'accesseur du descripteur.

### Ce qui pinne déjà le chemin (lu, non lancé)

`test_pico8_jc` : `test_hq` (catégorie `RE_THROWAWAY`, `re.p` plié), `test_too_far` (trait de `far` à l'écran = `re.ra` ; coût de `near` affiché = `dj(qu, re.p)` ; `re` de `far` identique après un `ey` refusé ; `far.re.p` plié après le `hq` de `near`). Mutation qui prouve que ce filet mord **sur l'ancien code** (copie jetable) : `p` calculé comme `mp/64` au lieu de `mp/128` → `test_hq` rouge (`HQ_HASH`). À noter dans le journal.

### Nouvelle caractérisation, figée sur l'ancien code

Écrite **avant** la conversion, vue verte sur l'ancien code, puis gardée **inchangée** (copie gardée, diff vide sur la fonction après conversion ; seules les fonctions de reconnaissance partagées peuvent changer). Dans `test_pico8_jc` (rapide, `make test`), une fonction lancée par `in_fresh_child`, par exemple `test_hq_mouse_point` :

- `world()`, `origin = qu` du slipway, `a = build_jc(origin)` ;
- `m1 = vec_ref(-640, -640)`, `hq_over(a, NULL, m1)` ; `r1 = re` de `a` : `get(r1, "ra")` est **la même REF** que `m1` ; `vec_of(r1, "p") == m1/128` ; `r1` n'est listé ni dans `hv`, ni dans `cg("jc")`, ni dans `cg("np")` ; `mt` et `id` absents sur `r1` ;
- `m2 = vec_ref(…)` différent, `hq_over(a, NULL, m2)` : `re` de `a` est un objet **différent** de `r1` ; `r1` garde `ra == m1` et son `p` d'avant ;
- `b = build_jc(origin)`, `hq_over(b, NULL, m1)` : `re` de `b` est distinct de celui de `a` ; `a` garde le sien ;
- `hq_over(a, planète, …)` : `re` de `a` est la planète ; `r1` intact ;
- une empreinte figée (`#define …_HASH`) de ces valeurs, repliée avec les catégories existantes.

### Adaptations permises, et seulement celles-là

Dans `test_pico8_jc` : le corps de `is_throwaway` (ou un reconnaisseur qui le remplace à ses deux appelants) reconnaît le `{ra,p}` par l'accesseur du descripteur ; `re_kind` rend `RE_THROWAWAY` pour lui, `fold_value` plie `0x7A` pour lui. Aucune autre ligne des tests existants. Vérifier dans l'ordre : adaptation verte sur l'ancien code (reconnaissance des deux formes à ce stade), rouge sur le code converti sans elle, verte avec.

### Ajouté après conversion

`test_mouse_point_is_a_struct` (nom au choix) : `re` d'un `jc` au-dessus du vide est une struct au descripteur de l'accesseur ; `ra` et `p` présents ; `mt`, `id`, `qs`, `fo`, `me`, `nu`, `t`, `kf`, `eb`, `hq` absents ; `pico8_mv(re, "kf"/"hq"/"eb", …)` rend 0 ; `must_abort` sur une écriture générique de `ra`, de `p` et de `fo` ; un `hq` avec `mp` nil donne `ra` absent et `p` = (0,0).

### `gcov`

`test_pico8_jc` compilé en `--coverage -O0` dans une copie jetable du scratchpad (supprimée ensuite ; précédents 1a.38/1a.39 : `__gcov_dump` avant `_exit` par `-include`, les tests tournant dans des fils). Chaque ligne ajoutée ou modifiée de `pico8_slipway.c` est exécutée : allocation et remplissage de la struct, lecteur (`ra` servi, `ra` nil → 0, `p` servi, autre nom → 0), accesseur. Toute ligne à 0 est couverte par un test ajouté ou justifiée dans le message de commit.

### Mutations sur le nouveau code (au moins trois, copie jetable, restaurées, `cmp` contre l'arbre)

1. la branche rend un unique objet statique au lieu d'en allouer un → caractérisation rouge (objets distincts, `r1` intact) et `TOO_FAR_HASH` rouge ;
2. le lecteur ne sert plus `p` → `test_hq` rouge (`fold_vec` plie `0xDEAD`, `od` change) ;
3. `ra` reçoit une copie du vecteur au lieu de la REF → caractérisation rouge (même REF que `mp`) ;
4. (facultative) le lecteur ne sert plus `ra` → `test_too_far` rouge (trait de `far` à (0,0)).

Rapportées dans le message de commit avec la ligne qui rougit.

### Tests de régression du §6

Aucun n'est touché (la ligne du plan n'en nomme aucun). `slipway_drag` lit `qu`/`od` de `lheld`, pas `re` ; il doit rester vert tel quel.

### Budget et journal des lancements

Budget : **environ 20 minutes de temps total de tests**. Ordre de grandeur (CLAUDE.md, et 1a.39) : `make test` ~20 s à une minute ; `make e2e` ~4-6 min ; `make class-coverage-net` ~3-4 min. Répartition indicative : `test_pico8_jc` seul pour la boucle (caractérisation sur l'ancien code, mutation de l'ancien code, adaptation, mutations, `gcov`) ; `make test` 2 à 3 fois ; `make e2e` une fois en fin ; `make class-coverage-net` une fois en fin. Au-delà du budget, l'implémenteur s'arrête et note ce qui manque plutôt que de relancer.

**Chaque lancement est journalisé dans le journal de progression** : commande exacte, état de l'arbre (HEAD + « ancien code » / « converti » / « mutation N, copie jetable »), code de sortie, ligne clé (par ex. `test_pico8_jc: all tests passed`, `frames-check : 15 conformes`, ligne d'assert qui rougit), durée. Le vérificateur réutilise ce journal et ne relance que ce qui est postérieur au dernier changement de l'arbre.

Commandes : `timeout 10m make test` ; `timeout 15m make e2e` ; `timeout 15m make class-coverage-net`. Jamais l'appli raylib.

### Critères d'échec (le vérificateur de `prd-autopilot` les traduit en `FAILED:`)

- le filet reste rouge après 3 tentatives de correction sur le même commit ;
- le diff modifie ou supprime un `*.sha256` du filet, ou retire une entrée de `FRAMES_EVENTS`/`RECORDINGS` ;
- `make class-coverage-net` n'est pas identique à `tools/class_coverage_net_expected.txt`, ou un nom est ajouté à `KNOWN[]` ;
- un champ ne tombe sous aucune règle du §5 (par exemple, un écrivain découvert sur l'objet) ;
- un test de régression devrait changer d'intention pour passer ;
- une autre classe ou instance sans classe est convertie ou modifiée (enveloppe de `g_ha`, `mz.jj`, cible de `bv`, `{qs}` de `ce`, …) ;
- une valeur figée ou une empreinte (`#define …_HASH`, `check(...)` attendu) est éditée, ou la nouvelle caractérisation est modifiée après avoir été vue verte sur l'ancien code ;
- un écart port/cart est corrigé au lieu d'être préservé.

## Out of Scope

- 1a.41 (enveloppe de `pz()` sur `g_ha`), 1a.42 (suppression des `pico8_class_t`, du type `pico8_instance_t *` des paramètres, passage de `dl`/`d_`/`bv` à la forme struct seule).
- Libérer le `{ra,p}` ou son `p`, ou le `ui.qa` : TROU-7, Plan 2 §8.
- Embarquer `p` dans la struct, typer `ra`/`p` en `pico8_vec_t` (phase 1b).
- Resserrer la condition de l'assert de `pico8_jc_hq`.
- Tout changement de rendu ou de comportement ; l'audio.

## Further Notes

- Étape d'un seul commit, sans morceaux.
- Numéros de ligne à HEAD fd28da6 ; rien n'a été compilé ni lancé pour ce PRD.
- Le message de commit porte : site (et la ligne corrigée du plan), forme choisie (par appel, `ra`/`p`, sans écrivain) et pourquoi pas statique (`TOO_FAR_HASH`), le journal résumé des lancements, la caractérisation et sa preuve sur l'ancien code, l'adaptation de `test_pico8_jc` et pourquoi ses empreintes ne bougent pas, rapport `gcov`, mutations tuées, réponses à H1-H4, « Découvert à 1a.40 ».
