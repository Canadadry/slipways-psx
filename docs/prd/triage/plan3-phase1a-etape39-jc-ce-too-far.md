---
title: "Plan 3, étape 1a.39 : le `{qs=\"-too far-\"}` de `jc:ce` devient une struct sans parent à deux membres `qs`/`fo`, en stockage statique"
description: "Quand un slipway tiré dépasse la portée, `pico8_jc_ce` alloue à chaque appel une instance sans classe (tag `INSTANCE`, table `{qs}`, jamais libérée, TROU-7) sur laquelle `jc:hq` écrit ensuite `fo` ; elle devient un unique objet statique, struct sans parent à deux membres `qs` et `fo`, avec un lecteur `qs`/`fo` et un écrivain `fo` seul."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Étapes de la phase 1a ») :

> 1a.39 — `{qs="-too far-"}` : sans classe (`slipway.c:255`).

À HEAD 3b8e840 le numéro est juste. Le `{qs=…}` est construit dans `pico8_jc_ce`, `src/pico8_slipway.c:254-261`, sur la branche « trop loin » (`#(sp-dp) > ov("space folding",0.5,1.08)`) :

- `:255` `malloc` d'un `pico8_instance_t` ; `:256-258` tag `INSTANCE`, `cls` NULL, `fields = pico8_ob_table_new()` ;
- `:259` `qs = "-too far-"` dans la table ;
- `:260` rendu comme valeur de `ce`.

C'est le port littéral de `pico8/lua.lua:640-644` :

```lua
function jc:ce()local sp,dp=self.qu.p,self.re.p
if #(sp-dp)>ov("space folding",0.5,1.08)then
return {qs="-too far-"}
end
```

et de son unique appelant, `jc:hq` (`lua.lua:622-632`, `slipway.c:134-141`) :

```lua
self.et=self:ce()if self.et then
self.et.fo=
self
end
```

`jc:hq` tourne à chaque frame tant qu'un slipway est tenu (`ui:bh`, `pico8_ui.c:463`), puis `ey` la même frame au relâchement (`:477`). Chaque frame où le curseur est hors de portée alloue un `{qs}` neuf et abandonne le précédent (TROU-7).

Règles du §5 qui s'appliquent :

> Chaque classe devient un `struct` […] ; chaque champ de sa chaîne (défauts et champs écrits sur instance) devient un membre de type `pico8_value_t`, **nom minifié conservé**. Nil reste `pico8_value_ref(NULL)`.
>
> **Instances sans classe** : converties comme des classes à part entière, sans parent.
>
> **Coexistence** : une instance convertie porte un nouveau tag (`PICO8_TYPE_TAG_STRUCT`) et un pointeur vers sa table de méthodes, dans le même premier emplacement qu'aujourd'hui.
>
> **Champ d'un écrivain générique** : un champ écrit par un site qui reçoit un élément de `cg.<tag>` ou `hv` sans filtre de classe devient membre de l'ancêtre le plus haut qui porte ce tag […].

Ce dernier point ne s'applique pas ici : `jc:hq` écrit `fo` sur la valeur de *son propre* `ce()`, pas sur un élément de `cg`/`hv` — c'est un champ écrit sur l'instance, donc un membre de la struct elle-même (même lecture qu'à 1a.38 pour le `{}` de `bv`).

§7 : un seul commit ; `make class-coverage-net` identique à `tools/class_coverage_net_expected.txt` ; preuve par `gcov` et 2 à 3 mutations ; note « Découvert à 1a.39 » réduite à ce qui change une étape suivante ; un écart port/cart découvert est préservé par un `TROU-N`, jamais corrigé.

### Précédents réutilisés

- **Enveloppe `mz.jj` (1a.11, d85626d)** : même contenu cart (`o(qs="-too far-")`), devenue struct statique unique à en-tête plus un membre `qs`, lecteur limité à `qs`, **sans écrivain**, accesseur public `pico8_mz_jj_struct_class()`.
- **`{}` de `nr:gv` (1a.37, 527d087)** : objet par appel remplacé par un objet statique.
- **`et={}` de `bv` (1a.38, 493d9b0)** : le cas le plus proche — objet jetable par appel, `fo` écrit dessus par son appelant, devenu struct statique à un membre `fo`, lecteur et écrivain limités à `fo`, accesseur public `pico8_bv_et_struct_class()`, `/* TROU-7 */` du site retiré avec l'allocation ; tests adaptés dans leur seule façon de reconnaître l'objet, la constante `PICO8_TYPE_TAG_INSTANCE` repliée pour garder les empreintes (précédent `lq`, 1a.34).

### Emprise (HEAD 3b8e840, lecture du code et grep ; rien n'a été lancé)

| Aspect | Constat |
|---|---|
| Construction | 1 site : `pico8_jc_ce`, `slipway.c:254-261`. Ni classe, ni `pico8_class_instantiate`, ni `lt` : jamais dans `hv`/`cv`/`cg`, jamais compté par le crochet de couverture. `pico8_jc_ce` n'a qu'un appelant, `pico8_jc_hq` (`:134`). |
| Vie / propriété | jamais libéré (TROU-7). Seule REF gardée : `self.et` du `jc` qui a appelé `hq`, écrasée au `hq` suivant. |
| Écrivains | `qs` à la construction (`:259`). `fo`, par `pico8_jc_hq` (`:140`, `pico8_entity_set_field(et, "fo", self)`), juste après. Aucun autre nom n'est écrit dans `src/`. |
| Lecteurs de la valeur (`jc.et`) | `jc:kj` (`slipway_display.c:207`) : vérité seule (`et or fo` → couleur 2). `jc:ev` → `dl` (`probe.c:268-275`) : vérité, puis `et.qs` par `pico8_entity_get_field`, **présent** ici → texte « -too far- » au lieu du sprite 31. `jc:ey` → `d_` (`probe.c:325`) : vérité seule → refus (`rn(57)`, `p_=true`), `fm:lu` court-circuité. |
| Lecteurs de `fo` sur l'objet | **aucun dans `src/`** : `jo:kj` lit `fo` d'une planète, `jc:kj` celui d'un `jc`. Seul `test_pico8_jc` (`test_hq`, `:616`) le lit, juste après le `hq` qui l'a écrit. |
| Identité | jamais comparée dans `src/` ni dans le cart (vérité, `qs`, écrasement). Aucun test ne compare deux `{qs}` entre eux. |
| Méthodes | aucune ; `dl`/`d_`/`kj` ne cherchent aucune méthode sur `et`. |
| Tests qui l'atteignent (lu, non lancé) | `test_pico8_jc` (`make test`) : `test_hq` (`:600-627`) construit un `jc` loin (`hq_over(far, NULL, (-640,-640))`), asserte `et_kind == ET_QS` et `et.fo == far`, plie `et` dans `HQ_HASH`. `et_kind` (`:220`) et `fold_value` (`:163`) le reconnaissent par `is_throwaway` (tag `INSTANCE`, `cls` NULL) ; `fold_jc` le plie à chaque frame des rejeux `drag`/`hub`/`playthrough`. Aucun autre fichier de `tests/` ne plie le `et` d'un `jc` (les `fold_value` de `g_`/`hj`/`os`/`qh` plient le `et` de `bv`, pas celui de `jc`). |
| Jamais pinné directement | `dl` sur un `jc` hors de portée (le texte « -too far- ») ; `jc:kj` avec ce `et` ; `jc:ey` → `d_` avec ce `et`. Voir hypothèses. |
| Couverture | aucun nom : absent de `KNOWN[]` et de `tools/class_coverage_net_expected.txt`, qui ne doivent pas bouger. |
| Écart port/cart | aucun connu. La fuite (TROU-7) est propre au port. |

**Hypothèses non mesurées (consigne : aucun test lancé pendant la rédaction), à vérifier par l'implémenteur et reportées dans le message de commit :**

- H1. Quels rejeux du filet atteignent la branche « trop loin » (un slipway tiré au-delà de la portée) : inconnu. Le filet tranche ; l'implémenteur n'a pas à le mesurer, mais si `make e2e` rougit, c'est un changement de rendu à corriger dans le code.
- H2. Les rejeux `drag`/`hub`/`playthrough` de `test_pico8_jc` plient peut-être ce `et` (via `fold_jc`) ; avec la reconnaissance adaptée qui replie la même constante `0x7A` et la même catégorie `ET_QS`, leurs empreintes ne bougent pas qu'ils l'atteignent ou non.
- H3. `test_pico8_jc` est le seul fichier de test à changer. Si un autre test rougit : soit une autre reconnaissance par le tag (adapter de même, sans toucher d'empreinte), soit un changement de comportement (corriger le code).

## Solution

1. **Struct sans parent, deux membres** : `pico8_struct_header_t`, puis `qs` et `fo` (`pico8_value_t`). `qs` est un champ de l'instance posé à la construction (`"-too far-"`) ; `fo` est écrit sur l'instance par `jc:hq` (nil au départ). Pas de `pico8_ke_t`.
2. **Descripteur `static const`** : aucune méthode ; un lecteur qui sert `qs` et `fo` (`fo` absent quand il est nil, comme tout membre) et rend 0 pour tout autre nom, sans assert ; un écrivain qui n'accepte que `fo` et rend 0 pour tout autre nom (`qs` compris), ce qui fait avorter `pico8_entity_set_field`.
3. **Un unique objet statique non `const`**, privé à `pico8_slipway.c`, tag `STRUCT`, descripteur, `qs = "-too far-"` et `fo` nil fixés à la compilation. La branche « trop loin » de `pico8_jc_ce` rend son adresse au lieu du `malloc` + `pico8_ob_table_new`.
4. **Accesseur public du descripteur** dans `pico8_slipway.h` (précédents `pico8_mz_jj_struct_class()`, `pico8_bv_et_struct_class()`), seul moyen pour les tests de reconnaître l'objet. Pas d'accesseur de l'objet lui-même.
5. **`pico8_jc_hq`, `dl`, `d_`, `jc:kj`** : inchangés dans leur logique. `et.fo = self` passe déjà par `pico8_entity_set_field` (aiguillé sur le tag), `dl` lit déjà `qs` par `pico8_entity_get_field`.
6. **Contrats rendus faux corrigés, rien d'autre** : dans `pico8_slipway.h`, le contrat de `pico8_jc_ce` (« a fresh `{qs="-too far-"}` instance ») et celui de `pico8_jc_hq` (« a `{qs}` instance ») ; le `/* TROU-7 */` au-dessus de `pico8_jc_ce` part avec l'allocation (celui de `pico8_jc_hq` reste : le `re={ra,p}` alloué, 1a.40). Le message de l'assert de `pico8_jc_hq` (`:137-139`, « a planet or `{qs}` instance ») dit vrai ; sa condition (`INSTANCE || STRUCT`) ne bouge pas.
7. **Tests adaptés dans leur façon de lire, pas dans leur intention** : dans `test_pico8_jc`, un reconnaisseur par l'accesseur, utilisé par `et_kind` (catégorie `ET_QS` inchangée) et par `fold_value` (même constante `0x7A` qu'aujourd'hui). `is_throwaway` reste tel quel : il reconnaît encore `re={ra,p}` (1a.40). Aucune empreinte, aucun `#define …_HASH`, aucune valeur figée édité.
8. **Plan** : à la ligne 1a.39 de `docs/plan-3.md`, « Découvert à 1a.39 : » réduit à ce qui change une étape suivante — a priori : struct statique à deux membres `qs`/`fo` (écrivain `fo` seul, sans méthode), accesseur public du descripteur ; enum inchangée ; `dl` lit toujours `qs` par `pico8_entity_get_field` (son `et` est aussi une planète, un `jc`, le `jj`, la cible de `bv`) ; condition de l'assert de `pico8_jc_hq` et type `pico8_instance_t *` inchangés jusqu'à 1a.42.

## User Stories

1. En tant qu'autopilote de la phase 1a, je veux que le `{qs="-too far-"}` de `jc:ce` soit la seule chose convertie par ce commit, afin de respecter « une classe par commit » (§7).
2. En tant qu'autopilote, je veux l'emprise de l'objet établie (construction, vie, écrivains, lecteurs, identité, tests), afin que la forme choisie soit justifiée.
3. En tant que lecteur du plan, je veux que l'objet devienne une struct sans parent, afin d'appliquer « converties comme des classes à part entière, sans parent ».
4. En tant que mainteneur, je veux que `qs` soit un membre, parce que c'est un champ de l'instance posé à la construction et lu par `dl`.
5. En tant que mainteneur, je veux que `fo` soit un membre, parce que `jc:hq` l'écrit sur cette instance.
6. En tant que joueur qui tire un slipway au-delà de la portée, je veux voir « -too far- » au curseur, comme aujourd'hui.
7. En tant que joueur qui tire un slipway au-delà de la portée, je veux que le trait reste de la couleur 2, comme aujourd'hui.
8. En tant que joueur qui relâche un slipway hors de portée sur une planète, je veux que la pose soit refusée sans rien dépenser, comme aujourd'hui.
9. En tant que joueur qui tire un slipway dans la portée, je veux que la cible (slipway existant, planète ou `jc` sur le chemin, ou rien) soit la même qu'aujourd'hui.
10. En tant que joueur qui tire un `mz` trop loin, je veux que son `jj` reste un objet distinct et inchangé.
11. En tant que gardien de la mémoire, je veux que `jc:ce` n'alloue plus rien hors de portée, afin qu'une frame de glisser trop loin cesse de fuir une table.
12. En tant que gardien du filet, je veux que `qs` lu sur l'objet rende toujours « -too far- », afin que `dl` écrive le même texte.
13. En tant que gardien des tests, je veux que `fo` lu juste après un `hq` soit le `jc` qui vient de l'écrire.
14. En tant que mainteneur, je veux qu'une écriture générique de `qs` ou d'un autre nom que `fo` arrête le programme, afin qu'un champ jamais mesuré ne soit pas rangé en silence et que `qs` ne soit pas altéré pour tous.
15. En tant que mainteneur, je veux que l'objet n'ait aucune méthode, afin qu'une lecture de méthode sur lui rende « absent » comme sur la table du cart.
16. En tant que gardien de la couverture, je veux qu'aucun nom ne soit ajouté à `KNOWN[]` ni compté, afin que `make class-coverage-net` reste identique à la référence.
17. En tant que gardien des tests, je veux que `test_pico8_jc` ne change que sa façon de reconnaître l'objet, avec ses empreintes intactes.
18. En tant que gardien des tests, je veux que `is_throwaway` et `re_kind` restent inchangés, afin que `re={ra,p}` soit encore reconnu jusqu'à 1a.40.
19. En tant que vérificateur, je veux une caractérisation des chemins non pinnés (`dl`, `kj`, `ey`/`d_` sur un `jc` hors de portée), écrite et vue verte sur l'ancien code, afin que la conversion ne les casse pas sans bruit.
20. En tant que vérificateur, je veux un rapport `gcov` et 2 à 3 mutations ciblées, afin de juger la preuve sans monter mon propre banc.
21. En tant que vérificateur, je veux un journal de chaque lancement de test (commande, état de l'arbre, sortie, ligne clé, durée), afin de réutiliser les résultats au lieu de tout relancer.
22. En tant que gardien du filet, je veux qu'aucun `.sha256` ni aucune entrée de `FRAMES_EVENTS`/`RECORDINGS` ne bouge.
23. En tant qu'auteur des étapes 1a.40 à 1a.42, je veux que `re={ra,p}`, l'enveloppe de `g_ha`, `mz.jj`, la cible de `bv` et les `pico8_class_t` restent intacts.
24. En tant que lecteur du code, je veux que les contrats de `pico8_jc_ce`/`pico8_jc_hq` et le message d'assert de `hq` disent vrai après ce commit.
25. En tant que lecteur du code, je veux qu'aucun commentaire de récit de chantier ne soit ajouté (règle des quatre destinations de CLAUDE.md).
26. En tant qu'autopilote, je veux qu'un écart port/cart découvert soit préservé par un `TROU-N`, jamais corrigé.

## Implementation Decisions

Pas d'outil pour lancer un agent `guideline` frais dans cette session : j'ai chargé et appliqué moi-même le skill `guideline` (hypothèses explicites, simplicité, changement chirurgical, critères vérifiables). Décisions tranchées ainsi marquées (G).

- **`qs` et `fo` membres** : `qs` est posé sur l'instance à sa construction (le `{qs=…}` du cart) ; `fo` est écrit sur l'instance par `jc:hq` (§5, « champs écrits sur instance »). Aucun autre nom n'est lu ou écrit comme champ propre. Ce n'est pas un « champ d'un écrivain générique » : `jc:hq` n'itère pas `cg`/`hv`.
- **Objet distinct de `mz.jj`** (G). Même contenu cart, mais deux tables distinctes dans le cart, et `mz.jj` n'a pas d'écrivain (une écriture de `fo` y avorte, voulu à 1a.11). Réutiliser `jj` changerait son contrat et convertirait deux sites en un : rejeté.
- **Statique, pas une allocation par appel** (G). Le cart fabrique un `{qs}` neuf par appel. Un objet partagé n'est observable que par son identité ou par un `fo` resté d'un appel précédent. Constat : dans `src/`, personne ne compare cette identité ni ne lit `fo` sur l'objet ; le seul test qui lit `fo` le lit juste après le `hq` qui l'a écrit ; deux `jc` hors de portée en même temps partageraient l'objet, ce que seul un lecteur de `fo` sur l'objet verrait, et il n'y en a pas. Le statique suit `mz.jj`, `{}` de `nr:gv`, cible de `bv`, et supprime une fuite par frame. Rejeté : un `malloc` par appel, qui garderait TROU-7 pour une identité que rien n'observe. Si l'implémenteur trouve un lecteur de `fo` sur l'objet ou une comparaison d'identité, c'est un fait nouveau : il le décrit dans le commit et revient à une allocation par appel (forme struct, tag `STRUCT`) plutôt que de l'ignorer.
- **Non `const`** : l'écrivain pose `fo`.
- **Pas de remise à nil de `fo` à l'init** (G) : aucun lecteur ne peut voir un `fo` resté (précédent 1a.38).
- **`qs` refusé par l'écrivain** (G) : rien ne l'écrit dans `src/` ; l'objet étant partagé, une écriture de `qs` changerait le texte de tous les slipways hors de portée. L'avortement signale ce champ sans règle.
- **Retrait du `/* TROU-7 */` de `pico8_jc_ce`** (G) : il marquait l'allocation jamais libérée ; sans allocation, il n'a plus d'objet. Ce n'est pas corriger un écart port/cart (le cart ne fuit pas, rien de visible ne change). L'entrée TROU-7 de `docs/trous-de-portage.md` et le `/* TROU-7 */` de `pico8_jc_hq` restent.
- **Condition de l'assert de `pico8_jc_hq` inchangée** (G) : après ce commit, `ce` ne rend plus que des structs, mais resserrer l'assert n'est pas demandé ; seul son message, devenu faux, est corrigé. Idem pour `dl`/`d_`, qui restent à double forme.
- **Lecteur** : pas d'assert sur un nom inconnu (les tests et `pico8_mv` interrogent des noms absents). Il ne peut pas être NULL (`pico8_entity_get_field` l'exige).
- **Nom** : le cart ne nomme pas ce `{qs}` au-delà du champ `et` qui le reçoit ; un nom C rattaché à `jc:ce` (par ex. « cible hors de portée de `jc:ce` »), au choix de l'implémenteur, sans nom minifié inventé ni entrée de glossaire.
- **Signatures inchangées** : `pico8_jc_ce` rend toujours un `pico8_value_t` ; aucun type de paramètre ne change (1a.42).
- **Écarts port/cart** : aucun connu ; un écart découvert est préservé et décrit par `TROU-N`.
- **Commentaires** (CLAUDE.md) : au plus un contrat C sur l'objet (statique, jamais libéré, `qs` fixe, `fo` seul écrit, le `hq` suivant l'écrase) ; contrats et message d'assert corrigés ; tout le reste dans le message de commit.

## Testing Decisions

Un bon test ici lit le comportement externe (champs par `pico8_entity_get_field`, écran, crédits, `oe`), pas la disposition mémoire ; seule la reconnaissance de l'objet passe par l'accesseur du descripteur.

### Ce qui pinne déjà le chemin (lu, non lancé)

`test_pico8_jc::test_hq` : un `jc` hors de portée a `et_kind == ET_QS` (objet jetable dont `qs == "-too far-"`) et `et.fo == far` ; `fold_jc` plie `et_kind` et `fold_value(et)` dans `HQ_HASH`. C'est la caractérisation du constructeur et de l'écriture `fo`. Mutation qui prouve qu'elle mord (à lancer sur l'ancien code, copie jetable) : `pico8_jc_ce` rend `pico8_value_ref(NULL)` au lieu de l'objet → `test_hq` rouge (`:615`).

### Nouvelle caractérisation, figée sur l'ancien code

Écrite **avant** la conversion, vue verte sur l'ancien code, puis gardée **inchangée** (copie gardée, diff vide sur la fonction après conversion ; seules les fonctions de reconnaissance partagées peuvent changer). Dans `test_pico8_jc` (rapide, `make test`), une fonction lancée par `in_fresh_child` :

- `world()`, puis un `far = build_jc(origin)` et `hq_over(far, NULL, (-640,-640))` comme `test_hq` ;
- **`kj`** : sous la caméra du jeu (comme `test_draw`), `cls`, `kj` sur `far` ; empreinte d'écran pliée ;
- **`ev` → `dl`** : `cls`, `ev` sur `far` ; empreinte d'écran pliée, et différente de celle d'un `jc` sans `et` dans les mêmes conditions (le texte « -too far- » au lieu du sprite 31 ou de « od$ ») ;
- **`ey` → `d_`** : `ey(target)` sur `far` avec une planète réelle ; asserts : `p_` vrai, crédits inchangés, `oe` de `origin` et `target` inchangés, `re` toujours l'objet jetable, `nh` inchangé ;
- une empreinte finale figée (`#define …_HASH`) et les asserts ci-dessus. La cible est reconnue par `et_kind == ET_QS`.

Si l'implémenteur constate (H1) qu'un rejeu du filet atteint déjà ces chemins, la caractérisation reste : elle est dirigée et rapide.

### Adaptations permises, et seulement celles-là

Dans `test_pico8_jc` : un reconnaisseur par l'accesseur du descripteur ; `et_kind` rend `ET_QS` pour lui (avec `qs == "-too far-"`), `fold_value` plie `0x7A` pour lui comme pour `is_throwaway`. `is_throwaway`, `re_kind` et toutes les empreintes figées restent tels quels. Vérifier dans l'ordre : adaptation verte sur l'ancien code (reconnaisseur = `is_throwaway` + `qs` à ce stade, ou reconnaissance des deux formes), rouge sur le code converti sans elle, verte avec.

### Ajouté après conversion

`test_too_far_is_a_struct` (nom au choix) : `et` de `far` est une struct au descripteur de l'accesseur ; `qs == "-too far-"` ; `fo == far` ; `ra`, `p`, `me`, `nu`, `t`, `hq`, `ev`, `kj` absents ; `pico8_mv(et, "hq"/"ev"/"kj", …)` rend 0 ; deux `jc` hors de portée ont le même `et` (le choix statique, documenté par le test) ; `must_abort` sur une écriture générique de `qs` et d'un autre nom ; `fo` remis à nil lu absent.

### `gcov`

`test_pico8_jc` compilé en `--coverage -O0` dans une copie jetable du scratchpad (supprimée ensuite ; précédent 1a.38 : `__gcov_dump` avant `_exit` par `-include`, les tests tournant dans des fils). Chaque ligne ajoutée ou modifiée de `pico8_slipway.c` est exécutée : branche de `ce` qui rend l'objet statique, lecteur (`qs` servi, `fo` servi, `fo` nil → 0, autre nom → 0), écrivain `fo`, accesseur. La branche « nom refusé » de l'écrivain, atteinte seulement dans un fils de `must_abort` qui avorte avant tout dump, est justifiée dans le message de commit par le SIGABRT attendu et par la mutation 3.

### Mutations sur le nouveau code (au moins trois, copie jetable, restaurées, `cmp` contre l'arbre)

1. le lecteur ne sert plus `qs` → `dl` dessine le sprite 31 → caractérisation `ev` rouge, `et_kind` ≠ `ET_QS` (`test_hq` rouge) ;
2. l'écrivain accepte `fo` sans le stocker → `test_hq` rouge (`:616`) ;
3. l'écrivain accepte tout nom → `test_too_far_is_a_struct` rouge (`must_abort` sur `qs`) ;
4. (facultative) `qs` initialisé à une autre chaîne → caractérisation `ev` et `test_hq` rouges.

Rapportées dans le message de commit avec la ligne qui rougit.

### Tests de régression du §6

Aucun n'est touché (la ligne du plan n'en nomme aucun). `slipway_drag` ne lit pas `et`.

### Budget et journal des lancements

Budget : **environ 20 minutes de temps total de tests**. Ordre de grandeur (CLAUDE.md) : `make test` quelques secondes à une minute ; `make e2e` ~4-5 min ; `make class-coverage-net` quelques minutes. Répartition indicative : `test_pico8_jc` seul pour la boucle (caractérisation sur l'ancien code, adaptation, mutations, `gcov`) ; `make test` 2 à 3 fois ; `make e2e` une fois en fin ; `make class-coverage-net` une fois en fin. Au-delà du budget, l'implémenteur s'arrête et note ce qui manque plutôt que de relancer.

**Chaque lancement est journalisé dans le journal de progression** : commande exacte, état de l'arbre (HEAD + « ancien code » / « converti » / « mutation N, copie jetable »), code de sortie, ligne clé (par ex. `test_pico8_jc: all tests passed`, `frames-check : 15 conformes`, ligne d'assert qui rougit), durée. Le vérificateur réutilise ce journal et ne relance que ce qui est postérieur au dernier changement de l'arbre.

Commandes : `timeout 10m make test` ; `timeout 15m make e2e` ; `timeout 15m make class-coverage-net`. Jamais l'appli raylib.

### Critères d'échec (le vérificateur de `prd-autopilot` les traduit en `FAILED:`)

- le filet reste rouge après 3 tentatives de correction sur le même commit ;
- le diff modifie ou supprime un `*.sha256` du filet, ou retire une entrée de `FRAMES_EVENTS`/`RECORDINGS` ;
- `make class-coverage-net` n'est pas identique à `tools/class_coverage_net_expected.txt`, ou un nom est ajouté à `KNOWN[]` ;
- un champ ne tombe sous aucune règle du §5 (par exemple, un écrivain découvert sur l'objet pour un autre nom que `fo`) ;
- un test de régression devrait changer d'intention pour passer ;
- une autre classe ou instance sans classe est convertie ou modifiée (`re={ra,p}`, enveloppe de `g_ha`, `mz.jj`, cible de `bv`, …) ;
- une valeur figée ou une empreinte (`#define …_HASH`, `check(...)` attendu) est éditée, ou la nouvelle caractérisation est modifiée après avoir été vue verte sur l'ancien code ;
- un écart port/cart est corrigé au lieu d'être préservé.

## Out of Scope

- 1a.40 (`re={ra,p}` de `jc:hq`, avec son `/* TROU-7 */` et `is_throwaway`), 1a.41 (enveloppe de `pz()` sur `g_ha`), 1a.42 (suppression des `pico8_class_t`, du type `pico8_instance_t *` des paramètres, passage de `dl`/`d_` à la forme struct seule).
- Resserrer la condition de l'assert de `pico8_jc_hq` ou de `pico8_bv` à `STRUCT` seul.
- Fusionner l'objet avec `mz.jj`.
- Les autres fuites TROU-7.
- Tout changement de rendu ou de comportement ; l'audio.

## Further Notes

- Étape d'un seul commit, sans morceaux.
- Numéros de ligne à HEAD 3b8e840 ; rien n'a été compilé ni lancé pour ce PRD.
- Le message de commit porte : site, forme choisie (statique, `qs`/`fo`, écrivain `fo` seul, pas de remise à nil) et pourquoi, le journal résumé des lancements, la caractérisation et sa preuve sur l'ancien code, l'adaptation de `test_pico8_jc` et pourquoi ses empreintes ne bougent pas, rapport `gcov`, mutations tuées, réponses à H1-H3, « Découvert à 1a.39 ».
