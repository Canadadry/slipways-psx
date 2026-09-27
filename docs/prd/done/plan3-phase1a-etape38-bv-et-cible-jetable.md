---
title: "Plan 3, étape 1a.38 : la cible jetable `et={}` de `bv` devient une struct sans parent à un membre `fo`, en stockage statique"
description: "Quand `bv` ne trouve ni planète ni slipway sous une sonde hors du panneau, `pico8_bv` alloue à chaque appel une instance sans classe (tag `INSTANCE`, table vide, jamais libérée, TROU-7) sur laquelle il écrit `fo` ; elle devient un unique objet statique, struct sans parent à un membre `fo`, avec un lecteur et un écrivain qui ne connaissent que `fo`."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Étapes de la phase 1a ») :

> 1a.38 — `et={}` : sans classe (`probe.c:169`).

Le numéro de ligne a dérivé. À HEAD 5330b68, le `{}` est construit dans `pico8_bv`, `pico8_probe.c:189-194`, sur la dernière branche (ni planète à moins de `dw`, ni `jc` sous le point, et `not ui.hw(rp)`) :

- `:189` `/* TROU-7 */` ;
- `:190-193` `malloc` d'un `pico8_instance_t` : tag `INSTANCE`, `fields = pico8_ob_table_new()`, `cls` NULL ;
- `:194` `et = pico8_value_ref(empty)`, puis `:201` `self.et = et` et `:207` `et.fo = self` (par `pico8_entity_set_field`).

C'est le port littéral de `pico8/lua.lua:552-559` :

```lua
function bv(self,mp,rp)self.ra,self.p=mp,mp/128
local nq,c=ij(self.p,cg.np)self.et=
nq<self.dw and c or
jp(cg.jc,function(sw) return be(sw.qu.p,sw.re.p,self.p,0.0032)end)[1]or
not ui.hw(rp)and {}
if (self.et)self.et.fo=self
end
```

`bv` est le `hq` de `g_` (`:722`), de `mz` (appelé en tête de `mz:hq`, `:767`) et de `qh` (`:915`), donc de toute la famille `jo`, de `fv` et des sondes : il tourne à chaque frame pendant qu'on tient l'objet. Chaque frame au-dessus du vide, hors panneau, alloue un `{}` neuf, et le précédent est perdu (TROU-7).

Règles du §5 qui s'appliquent :

> Chaque classe devient un `struct` […] ; chaque champ de sa chaîne (défauts et champs écrits sur instance) devient un membre de type `pico8_value_t`, **nom minifié conservé**. Nil reste `pico8_value_ref(NULL)`.
>
> **Instances sans classe** : converties comme des classes à part entière, sans parent.
>
> **Coexistence** : une instance convertie porte un nouveau tag (`PICO8_TYPE_TAG_STRUCT`) et un pointeur vers sa table de méthodes, dans le même premier emplacement qu'aujourd'hui.

§7 : un seul commit ; `make class-coverage-net` identique à `tools/class_coverage_net_expected.txt` ; preuve par `gcov` et 2 à 3 mutations ; note « Découvert à 1a.38 » réduite à ce qui change une étape suivante ; un écart port/cart découvert est préservé par un `TROU-N`, jamais corrigé.

### Précédents réutilisés

- **Enveloppe `mz.jj` (1a.11, d85626d)** : instance sans classe devenue struct statique unique à en-tête plus un membre (`qs`), lecteur limité à ce membre, descripteur `static const` sans méthode, accesseur public du descripteur (`pico8_mz_jj_struct_class()`) pour que les tests la reconnaissent.
- **`fm` provisoire (1a.36, 7ab4129)** : objet statique unique **écrit** (`jk`/`nz` en membres), sans méthode.
- **`{}` de `nr:gv` (1a.37, 527d087)** : même forme, sans membre ; alloc/free par appel remplacés par un objet statique.
- **`lq` (1a.34, 4158302)** : un pli de test qui hachait le tag d'un objet hors `hv` a été réécrit pour reconnaître la struct et plier la constante `PICO8_TYPE_TAG_INSTANCE`, empreinte figée inchangée. C'est le modèle des adaptations de tests ci-dessous.

### Emprise (HEAD 5330b68)

**Méthode.** Lecture du cart et du code, grep de `src/` et `tests/`, puis sonde sur une copie `git archive HEAD` jetable du scratchpad (supprimée depuis) : un registre des `{}` créés par `bv`, journalisation de chaque lecture et écriture de champ sur eux, et de leur passage dans `dl`, `d_`, `mz:kj`, `mz:hq`. Rejoués : les binaires de `tests/` (dont `UNIT_TESTS_SLOW`) et `pico8_replay_e2e` sur chaque `tests/fixtures/*events*.json`, avec et sans `--from-title`. Ces mesures ont été faites avant qu'une consigne interdise de lancer des tests pendant la rédaction du PRD ; ce qui n'a pas été mesuré est donné plus bas comme **hypothèse à vérifier**.

| Aspect | Mesure |
|---|---|
| Construction | 1 site : `pico8_bv` (`:189-194`), un objet neuf par appel qui prend la dernière branche. Ni classe, ni `pico8_class_instantiate`, ni `lt` : jamais dans `hv`/`cv`/`cg`, jamais compté par le crochet de couverture. |
| Vie / propriété | jamais libéré (TROU-7). Le `self.et` suivant écrase la REF : dans les longs tests, la plupart des créations remplacent un `{}` précédent (`jo` 135 sur 154, `session_events --from-title` 126 sur 137). Rien ne garde la REF ailleurs que le `et` de celui qui tient l'objet. |
| Écrivains | un seul champ, **`fo`**, écrit par `pico8_bv` `:207` sur l'objet qu'il vient de calculer. Aucun autre nom n'est écrit. `jc:hq` (`slipway.c:140`) écrit `fo` sur *son* `et` (planète, `jc`, `{qs=…}`), jamais sur ce `{}`. |
| Lecteurs dans `src/` | `dl` (`probe.c:238-245`) : vérité de `et`, puis `et.qs`, **toujours absent** ici, donc sprite 31. `d_` (`:295`) : vérité seule. `mz:kj` (`wormhole_display.c:70`) : vérité seule (pas de cercle). `mz:hq` (`wormhole.c:127`) : peut remplacer `et` par le `jj` sans le lire. **Personne dans `src/` ne lit `fo` sur ce `{}`** (les seuls lecteurs de `fo` lisent `self.fo` d'une planète ou d'un `jc`). |
| Lectures mesurées | `qs` (par `dl`) et `fo` (par les tests seulement). Aucun autre nom, aucune méthode. |
| Identité | jamais comparée dans `src/` (vérité, `qs`, écrasement). Le cart ne compare pas non plus. Aucun test ne compare deux `{}` entre eux. Un `fo` resté sur l'objet (dernier `bv` appelé) n'est lu nulle part. |
| Filet | atteint dans les 4 rejeux `RECORDINGS` (`--from-title`) : `session_events` 137 créations (`dl` 137, `mz:kj` 26), `events_recorded_1790198294` 9, `long_session_events` 7, `trading_hub_session_events` 1. Les 11 autres rejeux du filet ne l'atteignent pas. `mz:hq` remplace un `{}` par `jj` seulement hors filet (rejeux sans `--from-title`). |
| Jamais exercé | **`d_` avec un `{}` en `et`** (poser une sonde ou un bâtiment au-dessus du vide, hors panneau : refus, `rn(57)`, `p_=true`) : zéro occurrence dans tous les tests et tous les rejeux. |
| Tests qui l'atteignent | `g_`, `jc`, `nl`, `trade`, `mz` (dans `make test`) ; `hj`, `ie`, `jo`, `mx`, `nr`, `os`, `pm`, `qh` (`UNIT_TESTS_SLOW`). `fo` n'est lu sur lui que par `test_pico8_g_` (`fold_hq`, `:499`) et `test_pico8_mz` (`fold_hq`, `:409`). |
| Couverture | aucun nom : absent de `KNOWN[]` et de `tools/class_coverage_net_expected.txt`, qui ne doivent pas bouger. |
| Écart port/cart | aucun nouveau. La fuite (TROU-7) est un écart de mémoire propre au port, pas un comportement du cart. |

### Tests qui reconnaissent aujourd'hui le `{}` par sa forme

Un prototype de la conversion (copie jetable, supprimée) a rendu **cinq** tests rouges, uniquement parce qu'ils reconnaissent le `{}` par son tag `INSTANCE`/`cls` NULL :

- `test_pico8_mz` : `et_kind` (`:180-181`) → `ET_THROWAWAY` ; sinon `fold_hq` asserte (`:398`).
- `test_pico8_g_` : `et_kind` (`:441`) asserte `INSTANCE` et `cls` NULL ; et `fold_value` (`:179`) plie le tag de l'objet hors `hv` (empreinte `life`).
- `test_pico8_hj` (`:191`, empreinte `recorded`), `test_pico8_os` (`:179`, `long`), `test_pico8_qh` (`:191`, `life`, `long`, `session`) : `fold_value` plie `pico8_type_tag_of` de l'objet hors `hv`.

Avec la reconnaissance adaptée (le descripteur au lieu du tag, et la constante `PICO8_TYPE_TAG_INSTANCE` pliée comme avant, précédent `lq`), le prototype a passé `make test`, `make e2e` (15 rejeux « conforme ») et `make class-coverage-net` (OK, identique), **sans toucher à une seule valeur figée**. Les autres copies du même `fold_value` (`nl`, `mx`, `pm`, `nr`, `fr`, `jo`) sont restées vertes : elles ne plient jamais ce `{}`, on ne les touche pas.

## Solution

1. **Struct sans parent, un membre** : `pico8_struct_header_t` puis `fo` (`pico8_value_t`, nil au départ). `fo` est écrit sur l'instance (§5, « champs écrits sur instance »). `qs` n'est pas un membre : il est lu, jamais écrit ni défini par défaut (précédent 1a.37). Pas de `pico8_ke_t`.
2. **Descripteur `static const`** : aucune méthode ; un lecteur qui sert `fo` (absent quand il est nil, comme tout membre) et rend 0 pour tout autre nom ; un écrivain qui n'accepte que `fo` et rend 0 pour tout autre nom, ce qui fait avorter `pico8_entity_set_field` (« champ sans règle »).
3. **Un unique objet statique non `const`**, tag `STRUCT` et descripteur fixés à la compilation, privé à `pico8_probe.c`. La dernière branche de `pico8_bv` met son adresse dans `et` au lieu du `malloc` + `pico8_ob_table_new`. Le `/* TROU-7 */` de `:189` part avec l'allocation ; l'entrée TROU-7 et le `/* TROU-7 */` de `pico8_bv` dans le `.h` (le `p` alloué, `:157`) restent.
4. **Accesseur public** `pico8_bv_et_struct_class()` dans `pico8_probe.h` (précédent `pico8_mz_jj_struct_class()`), seul moyen pour les tests de reconnaître l'objet.
5. **`pico8_bv`, `dl`, `d_`, `mz:hq`, `mz:kj`** : inchangés dans leur logique. L'écriture `et.fo = self` passe déjà par `pico8_entity_set_field`, qui aiguille sur le tag ; `dl` lit `qs` par `pico8_entity_get_field`. Le message de l'assert de `:204-206` (« class-less throwaway instance ») est corrigé pour dire vrai.
6. **Contrat corrigé** : le commentaire de `pico8_bv` dans `pico8_probe.h` (`:103-106`, « a class-less throwaway instance ») dit « une struct statique sans champ lisible autre que `fo` » ; « collapsing the throwaway to nil would change `d_` » reste vrai et reste. Le commentaire de `et_kind` de `test_pico8_g_` (`:419-420`) suit.
7. **Tests adaptés dans leur façon de lire, pas dans leur intention** : les cinq sites ci-dessus reconnaissent l'objet par `pico8_bv_et_struct_class()` ; les plis gardent la constante `PICO8_TYPE_TAG_INSTANCE` pour cet objet. Aucune empreinte, aucun `#define` de hash, aucune valeur figée n'est édité.
8. **Plan** : à la ligne 1a.38 de `docs/plan-3.md`, site corrigé (`probe.c:189`) et « Découvert à 1a.38 : » réduit à ce qui change une étape suivante — a priori : struct statique à un membre `fo` (écrivain `fo` seul), accesseur `pico8_bv_et_struct_class()` ; enum inchangée ; `dl` lit toujours `qs` par `pico8_entity_get_field` (son `et` est aussi une planète, un `jc`, le `jj` et, jusqu'à 1a.39, le `{qs="-too far-"}` de `jc:ce`) ; `pico8_bv` garde `pico8_instance_t *self` jusqu'à 1a.42.

## User Stories

1. En tant qu'autopilote de la phase 1a, je veux que la cible jetable de `bv` soit la seule chose convertie par ce commit, afin de respecter « une classe par commit » (§7).
2. En tant qu'autopilote, je veux l'emprise du `{}` mesurée (construction, vie, écrivains, lecteurs, identité, filet, tests), afin que la forme choisie soit justifiée par une mesure.
3. En tant que lecteur du plan, je veux que le `{}` devienne une struct sans parent, afin d'appliquer « converties comme des classes à part entière, sans parent ».
4. En tant que mainteneur, je veux que `fo` soit un membre, parce que `bv` l'écrit sur cette instance (§5).
5. En tant que mainteneur, je veux que `qs` ne soit pas un membre, parce que rien ne l'écrit sur ce `{}` : `dl` le lit et le trouve absent.
6. En tant que joueur qui promène une sonde au-dessus du vide hors du panneau, je veux voir le même sprite 31 au curseur, afin que la conversion ne change rien.
7. En tant que joueur qui promène une sonde au-dessus du panneau, je veux que `et` reste `false`, sans objet jetable, comme aujourd'hui.
8. En tant que joueur qui promène un `mz` au-dessus du vide, je veux que le cercle de portée reste caché, comme aujourd'hui.
9. En tant que joueur qui lâche une sonde ou un bâtiment au-dessus du vide hors du panneau, je veux que la pose soit refusée comme aujourd'hui (`d_` voit un `et` vrai).
10. En tant que joueur qui promène un `mz` trop loin, je veux que `mz:hq` remplace toujours `et` par le `jj`.
11. En tant que gardien de la mémoire, je veux que `bv` n'alloue plus rien pour sa cible jetable, afin qu'une frame passée au-dessus du vide cesse de fuir une table.
12. En tant que gardien du filet, je veux que la lecture de `qs` sur l'objet rende toujours « absent », afin que `dl` dessine comme aujourd'hui.
13. En tant que gardien du filet, je veux que `fo` lu juste après un `bv` soit celui qui vient de l'écrire, afin que les tests `g_` et `mz` restent verts.
14. En tant que mainteneur, je veux qu'une écriture sur un autre nom que `fo` arrête le programme, afin qu'un champ jamais mesuré ne soit pas rangé en silence.
15. En tant que mainteneur, je veux que l'objet n'ait aucune méthode, afin qu'une lecture de méthode sur lui rende « absent » comme sur une table vide.
16. En tant que gardien de la couverture, je veux qu'aucun nom ne soit ajouté à `KNOWN[]` ni compté, afin que `make class-coverage-net` reste identique à la référence.
17. En tant que gardien des tests, je veux que `g_`, `mz`, `hj`, `os`, `qh` ne changent que leur façon de reconnaître l'objet, avec leurs empreintes intactes.
18. En tant que gardien des tests, je veux que tous les autres fichiers de test gardent un diff vide.
19. En tant que vérificateur, je veux une caractérisation du chemin jamais exercé (`d_` sur la cible jetable), écrite et vue verte sur l'ancien code, afin que la conversion ne le casse pas sans bruit.
20. En tant que vérificateur, je veux un rapport `gcov` et 2 à 3 mutations ciblées, afin de juger la preuve sans monter mon propre banc.
21. En tant que gardien du filet, je veux qu'aucun `.sha256` ni aucune entrée de `FRAMES_EVENTS`/`RECORDINGS` ne bouge.
22. En tant qu'auteur des étapes 1a.39 à 1a.42, je veux que `{qs="-too far-"}`, `re={ra,p}`, l'enveloppe de `g_ha` et les `pico8_class_t` restent intacts.
23. En tant que lecteur du code, je veux que le contrat de `pico8_bv` et le message de son assert disent vrai après ce commit.
24. En tant que lecteur du code, je veux qu'aucun commentaire de récit de chantier ne soit ajouté (règle des quatre destinations de CLAUDE.md).
25. En tant qu'autopilote, je veux qu'un écart port/cart découvert soit préservé par un `TROU-N`, jamais corrigé.

## Implementation Decisions

Pas d'outil pour lancer un agent `guideline` frais dans cette session : j'ai appliqué moi-même le skill `guideline` (hypothèses explicites, simplicité, changement chirurgical, critères vérifiables). Décisions tranchées ainsi marquées (G).

- **`fo` membre, `qs` non** : `fo` est écrit sur l'instance (§5) ; `qs` est seulement lu. Rejeté : un membre `qs` nil, qui n'a pas de règle.
- **Statique, pas une allocation par appel** (G). Le cart fabrique un `{}` neuf par appel. Un seul objet partagé n'est observable que par son identité ou par un `fo` resté d'un appel précédent. Mesuré : dans `src/`, personne ne compare cette identité ni ne lit `fo` sur l'objet ; les deux tests qui lisent `fo` le lisent juste après le `bv` qui l'a écrit. Le statique suit tous les précédents (`mz.jj`, `fm`, `{}` de `nr:gv`) et supprime une allocation qui fuyait à chaque frame. Rejeté : un `malloc` par appel, qui garderait la fuite TROU-7 pour une identité que rien n'observe.
- **Non `const`** : l'écrivain doit pouvoir poser `fo`.
- **Pas de remise à nil de `fo` dans `pico8_probe_init`** (G). À la différence de `fm`, aucun lecteur ne lit `fo` sur cet objet sans qu'un `bv` vienne de l'écrire. Une remise à zéro ne protégerait rien d'observable. Si l'implémenteur trouve un lecteur qui peut voir un `fo` resté, c'est un fait nouveau : il le décrit dans le commit et ajoute la remise à zéro (précédent `fm`).
- **Retrait du `/* TROU-7 */` de ce site** (G). Il marquait l'allocation jamais libérée ; sans allocation, il n'a plus d'objet. Ce n'est pas corriger un écart port/cart : le cart ne fuit pas (il a un ramasse-miettes) et rien de visible ne change. L'entrée TROU-7 de `docs/trous-de-portage.md` reste (d'autres sites fuient encore).
- **Accesseur public du descripteur** : cinq tests doivent reconnaître l'objet. Précédent `pico8_mz_jj_struct_class()`. Aucun accesseur de l'objet lui-même n'est nécessaire.
- **Lecteur** : sert `fo` quand il n'est pas nil, rend 0 pour tout autre nom, sans assert (`dl` lit `qs` et tolère son absence). Il ne peut pas être NULL : `pico8_entity_get_field` l'asserte.
- **Écrivain** : n'accepte que `fo` ; pour tout autre nom, rend 0, et `pico8_entity_set_field` avorte.
- **Nom** : le cart n'a pas de nom pour ce `{}` au-delà du champ `et` qui le reçoit. Un nom C descriptif rattaché à `bv` (ex. « cible jetable de `bv` »), au choix de l'implémenteur, sans inventer de nom minifié ni d'entrée de glossaire.
- **Signatures inchangées** : `pico8_bv(pico8_instance_t *self, …)` et les lecteurs à double forme de `et` restent ; le type tombe à 1a.42.
- **Écarts port/cart** : aucun connu. Un écart découvert est préservé et décrit par `TROU-N`.
- **Commentaires** (CLAUDE.md) : au plus un contrat C sur l'objet (statique, jamais libéré, `fo` seul, un `bv` suivant l'écrase) ; le contrat de `pico8_bv` et le message d'assert corrigés ; tout le reste dans le message de commit.

## Testing Decisions

- **Ce que les tests existants figent déjà** (mesuré, voir plus haut) : l'existence et la nature de la cible (`et_kind` de `g_` et `mz`, plis de `hj`/`os`/`qh`), son `fo` égal à celui qui vient d'appeler `bv` (`g_`, `mz`), et l'absence de `qs` (sprite 31 dessiné par `dl`, plié dans les empreintes d'écran). Ils sont réutilisés comme caractérisation.
- **Adaptations permises, et seulement celles-là** : dans `test_pico8_mz` (`et_kind`), `test_pico8_g_` (`et_kind`, `fold_value`), `test_pico8_hj`, `test_pico8_os`, `test_pico8_qh` (`fold_value`), reconnaître l'objet par `pico8_bv_et_struct_class()` et plier `PICO8_TYPE_TAG_INSTANCE` pour lui, comme avant. **Aucune empreinte ni valeur figée n'est éditée** ; l'intention de chaque test reste la même. Les autres copies de `fold_value` ne sont pas touchées.
- **Nouvelle caractérisation, écrite et vue verte sur l'ancien code d'abord, puis gardée inchangée** : le chemin `d_` avec la cible jetable n'est exercé nulle part (mesuré). Ajouter à `test_pico8_g_` (rapide, `make test`) un cas : une sonde (`g_`) reçoit `hq` sur un point au-dessus du vide, avec un `rp` hors de `ui.hw` ; `et` est vrai et n'est ni une planète ni un `jc` ; `ey`/`d_` refuse (`p_` vrai, résultat faux) et ne dépense rien (`fm` inchangé). Le test ne lit que le comportement externe et reconnaît la cible par la même fonction adaptée que `et_kind`.
- **Hypothèses non mesurées, à vérifier par l'implémenteur** (une consigne a interdit de relancer des mesures pendant la rédaction) :
  1. les cinq fichiers ci-dessus sont les seuls à changer : si un autre test tombe rouge, c'est soit une autre reconnaissance par le tag (à adapter de la même façon), soit un changement de comportement (à corriger dans le code, pas dans le test) ;
  2. `test_pico8_jc` (`is_throwaway`, `:143-145`) ne plie jamais cette cible : il est resté vert avec le prototype, mais sa fonction sert aussi `{qs=…}` et `re={ra,p}` et ne doit pas bouger ;
  3. les comptes de rejeux atteints (plus haut) sont des mesures à HEAD 5330b68 : l'implémenteur n'a pas à les refaire, le filet tranche.
- **`gcov`** (test compilé en `--coverage`, copie jetable du scratchpad, supprimée après) sur `test_pico8_g_` et `test_pico8_mz` : chaque ligne ajoutée ou modifiée de `pico8_probe.c` est exécutée (branche du lecteur qui sert `fo`, branche qui rend 0, écrivain `fo`, branche `bv` qui prend l'objet statique). La branche « nom inconnu » de l'écrivain (qui mène à l'abort) est soit couverte par un cas `must_abort` ajouté, soit justifiée dans le message de commit.
- **Mutations ciblées sur le nouveau code** (au moins deux, rapportées dans le message de commit ; les rouges attendus sont ceux du prototype) :
  1. l'écrivain accepte `fo` sans le stocker → `test_pico8_g_`, `test_pico8_mz` rouges (constaté sur le prototype) ;
  2. le lecteur sert un `qs` présent → `dl` écrit du texte au lieu du sprite 31 → `test_pico8_mz`, `test_pico8_os`, `test_pico8_qh` rouges (constaté sur le prototype), filet rouge ;
  3. `bv` met `false` au lieu de l'objet → la nouvelle caractérisation `d_` (pose acceptée ou `fm:lu` consulté) et `et_kind` rouges.
- **Tests de régression du §6** : aucun n'est touché (la ligne du plan n'en nomme aucun).
- **Commandes** : `timeout 10m make test` ; `timeout 15m make e2e` (filet + `UNIT_TESTS_SLOW`, dont `hj`, `os`, `qh`) ; `timeout 15m make class-coverage-net`, sortie identique à `tools/class_coverage_net_expected.txt`. Jamais l'appli raylib.

### Critères d'échec (le vérificateur de `prd-autopilot` les traduit en `FAILED:`)

- le filet reste rouge après 3 tentatives de correction sur le même commit ;
- le diff modifie ou supprime un `*.sha256` du filet, ou retire une entrée de `FRAMES_EVENTS`/`RECORDINGS` ;
- `make class-coverage-net` n'est pas identique à la référence, ou un nom est ajouté à `KNOWN[]` ;
- un champ ne tombe sous aucune règle du §5 (par exemple, un écrivain découvert sur l'objet pour un autre nom que `fo`) ;
- un test de régression devrait changer d'intention pour passer ;
- une autre classe ou instance sans classe est convertie (`{qs="-too far-"}`, `re={ra,p}`, enveloppe de `g_ha`, `jj`, …) ;
- une valeur figée ou une empreinte (`#define …_HASH`, `check(...)` attendu) de `g_`, `mz`, `hj`, `os`, `qh` ou d'un autre test est éditée, ou la nouvelle caractérisation `d_` est modifiée après avoir été vue verte sur l'ancien code ;
- un écart port/cart est corrigé au lieu d'être préservé.

## Out of Scope

- 1a.39 (`{qs="-too far-"}` de `jc:ce`), 1a.40 (`re={ra,p}`), 1a.41 (enveloppe de `pz()` sur `g_ha`), 1a.42 (suppression des `pico8_class_t` et du type `pico8_instance_t *` des paramètres et slots, dont `pico8_bv`).
- Le `p` alloué par `pico8_bv` (`:157`, TROU-7) et les autres fuites TROU-7.
- La lecture de `et` par `pico8_entity_get_field` dans `dl`/`d_` : elle reste à double forme tant que `{qs=…}` (1a.39) peut arriver dans `et`.
- Tout changement de rendu ou de comportement ; l'audio.

## Further Notes

- Étape d'un seul commit, sans morceaux.
- Mesures faites à HEAD 5330b68 sur une copie jetable supprimée ensuite ; les numéros de ligne cités sont ceux de ce HEAD.
- Le message de commit porte : sites, comptes de la sonde, forme choisie (statique, `fo` seul, pas de remise à nil) et pourquoi, mutations tuées, rapport `gcov`, les cinq tests adaptés et pourquoi leurs empreintes ne bougent pas, « Découvert à 1a.38 ».
