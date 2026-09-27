---
title: "Plan 3, étape 1a.32 : `trade` (la route commerciale) devient une struct racine à trois membres `lp`/`rl`/`qe`, sans méthode par nom ni écrivain"
description: "`trade` est construite 21 032 fois sur le filet (6/15 rejeux), toujours par `pico8_trade_new` avec les trois clés `lp`, `rl`, `qe`, depuis `jo:ib` ou `trade:kg`. Elle passe en struct `pico8_trade_t` (en-tête, `lp`, `rl`, `qe`), au descripteur sans méthode et sans écrivain, avec un lecteur pour `bd` qui lit `lp`/`rl` par l'API générique. `to`, `kg`, `eq` et le champ `t` des propositions de route prennent le type struct ; les trois lectures par `pico8_instance_get_field` hors du module (`jo:cn`, `nr:cn`, `dh`) passent au membre ou à l'API générique. Aucun test de régression n'est touché."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Étapes de la phase 1a ») :

> - 1a.32 — `trade` : racine (`trade.c:14`).

Pas de « Tests : » : aucun test de régression n'est nommé (vérifié plus bas, aucun ne lit un champ de route).

Les numéros de ligne n'ont pas dérivé pour la déclaration. À HEAD 0adebfd :
- **`pico8_trade.c`** :
  - `g_trade_class = pico8_class_declare(NULL, NULL)` à `:14`, nom de couverture à `:15` ;
  - `pico8_trade_new` à `:29` (`pico8_class_instantiate`) ;
  - `pico8_trade_to` à `:33`, lit `lp` (`:38`) ;
  - `pico8_trade_kg` à `:58`, lit `lp`/`rl`/`qe` (`:80`, `:83`, `:84`) et reconstruit une route par une table d'overrides (`:106-112`) ;
  - `pico8_trade_eq` à `:115`, lit `lp` des deux côtés (`:126-127`).
  Toutes ces lectures passent par `pico8_instance_get_field`.
- **`pico8_planet_trade.h:23-24`** : `struct pico8_jo_trade_proposal` porte `pico8_instance_t *t`.
- **`pico8_planet_trade.c`** : `gu_trade` (`:46`) caste chaque élément en `pico8_instance_t *` pour `pico8_trade_eq` (`:56`) ; `jo:cn` (`:100`) lit `t.qe` par `pico8_instance_get_field` (`:117`) et appelle `kg` (`:210`) ; `jo:ib` construit `lw.t` (`:270-281`).
- **`pico8_planet_family_trade.c`** : `nr:cn` lit `t.rl` par `pico8_instance_get_field` (`:185`) et appelle `kg` (`:195`, `gf.t` à `:198`).
- **`pico8_economy_tiers.c`** : `bd_route_visit` lit `rl`/`lp` par `field_list` (`:125-126`, déjà `pico8_entity_get_field`) et appelle `pico8_trade_to` (`:162`) ; `dh` lit `im.qe` par `pico8_instance_get_field` (`:301`, `TROU-3`).
- **`pico8_economy_score.c:160`** : `db` appelle `pico8_trade_to` sur chaque élément de `pt.by`.
- **`pico8_cart.c:65`, `:131`** : `pico8_trade_init`/`pico8_trade_shutdown`.
- **`tests/test_pico8_os.c:421-435`** : `export_to` construit une route par `pico8_trade_new` avec la seule clé `lp`.

Règles du §5 qui s'appliquent à ce commit :

> Chaque classe devient un `struct` dont le premier membre est la struct de sa classe parente ; chaque champ de sa chaîne (défauts et champs écrits sur instance) devient un membre de type `pico8_value_t`, **nom minifié conservé**. Nil reste `pico8_value_ref(NULL)`.
>
> **Héritage entre formes et classes-valeurs** : une classe convertie garde sa `pico8_class_t` enregistrée […] jusqu'à la dernière étape de 1a. Dans le commit de conversion, sa `pico8_class_t` reçoit un pointeur vers le constructeur de struct […].
>
> **Ordre** : […] Les racines sans parent dans n'importe quel ordre.

`trade` est une **vraie racine sans fille** (écarts relevés du §2 : « vraies racines sans fille : `trade`, `lq`, `jb`, `hy`, `hp` »). Elle ne descend pas de `ke` : pas de `lt`, jamais dans `hv`/`cv`/`cg`. Son précédent direct est `jb` (1a.1) : struct à en-tête seul, constructeur posé par `pico8_class_set_struct_new`, pas de `pq` de classe.

§7 :
- un seul commit ;
- `make class-coverage-net` identique à `tools/class_coverage_net_expected.txt` : `trade  21032  6/15`, toutes les autres lignes inchangées ;
- **preuve des tests par `gcov` et 2 à 3 mutations** ;
- **note « Découvert à 1a.32 »** réduite à ce qui change une étape suivante ;
- un nouveau test dont le rejeu dépasse 30 s va dans `UNIT_TESTS_SLOW` ;
- un écart port/cart découvert est préservé et décrit par un `TROU-N`.

### Ce que 1a.1 à 1a.31 ont posé, réutilisé tel quel

- `PICO8_TYPE_TAG_STRUCT`, `pico8_struct_header_t`, descripteur `pico8_struct_class_t` (`methods` indexé par l'enum, `field`, `set_field`).
  - `pico8_entity_get_field` rend d'abord l'entrée du descripteur, sinon le lecteur ; un membre nil est absent.
  - `set_field` NULL : une écriture générique avorte (précédents `nf`, `mh`, `iv`, `pj`, `ui`).
- Racine convertie (1a.1, `jb`) : en-tête en premier membre, constructeur `static` posé par `pico8_class_set_struct_new`, appelé aussi directement par `pico8_<classe>_new` ; il enregistre la couverture ; assert porteur sur toute clé non mesurée.
- Fonctions propres d'une classe convertie retypées sur la struct quand leur `self` est toujours de cette classe (`pj` 1a.29, `ui` 1a.31).

### Ce que fait le cart

`pico8/lua.lua` :

```
:86-88     kv:jr(rj) : rj.ns = métatable des instances ; ob=setmetatable(mw(ob), rj.ns)
:500-510   jo:ib()   : lw={t=trade({lp={self},rl={w},qe=qe}), mu=…, kf=…}  ; w:eb(self):cn(lw)
:478-498   jo:cn(lw) : t=lw.t ; lit t.qe ; lw:kf(t:kg(self))
:1116-1121 nr:cn(lw) : t=lw.t ; not gu(t.rl,w) ; gf.t=t:kg(self,w)
:603-605   trade=kv:jr() ; trade:to() = self.lp[#self.lp]
:606-608   trade:kg(np,jc) = trade({lp=la(self.lp,{np}), rl=la(self.rl,{jc}), qe=self.qe})
:609-611   trade.ns.__eq(l,r) = l.lp[1]==r.lp[1] and l:to()==r:to()
:1264-1275 bd()     : pt.by=pt:ib() ; for e in all(pt.by) : e.rl[i], e.lp[i+1].id ; add(e:to().cc, e)
:1294-1298 dh(pt)   : #jp(pt.cc, function(im) return im.qe==i.qe end)
:1327-1331 db(pt)   : for e in all(pt.by) : oz=e:to()
```

Points décisifs :
- **Deux voies de construction**, toutes deux avec les trois clés `lp`, `rl`, `qe` : `jo:ib` et `trade:kg`. Pas de spec, pas de défaut, pas de `pq`.
- **Pas de méthode cherchée par nom** : `to`, `kg` sont appelées directement (`e:to()`, `t:kg(…)`) ; `__eq` est une métaméthode, portée comme la fonction nommée `pico8_trade_eq`, appelée par `gu_trade` (le `gu` du cart compare par `==`, donc par `__eq`).
- **Jamais écrite après construction** : `kg` rend une route **neuve**.

### Emprise réelle de `trade` (mesurée)

**Méthode.**
- Grep de `src/`, `tests/`, `tools/` et du `Makefile` à 0adebfd.
- **Sonde** sur une copie `git archive HEAD` jetable du scratchpad, supprimée après usage. Binaires en `-O0 -fno-omit-frame-pointer -no-pie`, appelants résolus par `addr2line`. Elle journalise, pour toute instance de la classe `trade` :
  - chaque `pico8_class_instantiate` (clés reçues, nil marqué ; appelant de `pico8_trade_new`) ;
  - chaque `pico8_instance_get_field` (nom ; source : propre, classe, absent ; appelant) ;
  - chaque `pico8_entity_get_field` et `pico8_entity_set_field` ;
  - chaque `pico8_instance_fields` ;
  - dans `kg`, chaque `qe` absent sur `self` (branche `pico8_value_bool(0)`).
- Rejoués : les 17 `tests/fixtures/*events.json` et `events_recorded_1790198294.json` (`--from-title` pour les 4 enregistrements), **18 rejeux en sortie 0**.
- **Fil piège** : `make unit` puis les 8 cibles de `UNIT_TESTS_SLOW`, même copie, journal par `write` atomique (les fils forkés sortent par `_exit`), sortie 0.

**Constructions** :
- **filet** : 21 032 sur 6/15 rejeux (`built_planet_panel` 9, `phase4_playthrough` 9, `trading_hub_session` 204, `events_recorded_1790198294` 1 217, `long_session` 1 377, `session` 18 216) — égal à la référence ;
- **toujours** par `pico8_trade_new`, **toujours** avec exactement `lp`, `rl`, `qe`, **jamais nil** : 15 144 depuis `jo:ib` (`planet_trade.c:280`), 5 888 depuis `kg` (`trade.c:112`) ;
- **TU** : 87 220 constructions dans 18 binaires (`bg`, `built_planet_panel_nil_row`, `hj`, `ie`, `jc`, `jo`, `js`, `mx`, `mz`, `nl`, `nr`, `os`, `playability`, `pm`, `qh`, `rich_planet_exports`, `slipway_drag`, `ui`), toutes `lp`/`rl`/`qe` depuis `jo:ib` ou `kg`, **sauf 2** : `test_pico8_os:431` (`export_to`), clé `lp` seule.

**Noms lus sur une route** (rejeux et TU, mêmes sites) :
- `lp` : `to` (`trade.c:38`), `kg` (`:80`), `eq` (`:126-127`), `bd_route_visit` par `field_list` → `pico8_entity_get_field` ;
- `rl` : `kg` (`:83`), `nr:cn` (`family_trade.c:185`), `bd_route_visit` par `field_list` ;
- `qe` : `kg` (`:84`), `jo:cn` (`planet_trade.c:117`), `dh` (`economy_tiers.c:301`).
- **Toujours propres** : aucune lecture ne tombe sur la classe, **aucune ne trouve le nom absent**. Aucun autre nom n'est lu (pas de `to`, `kg`, `me`, `nu`…).
- `kg` ne voit **jamais** `qe` absent (0 passage dans la branche `pico8_value_bool(0)`).

**Champs écrits sur une route** : **aucun** (0 `pico8_entity_set_field`, 0 `pico8_instance_fields`, rejeux et TU).

**Accès `->defaults`** : aucun sur la classe `trade` (grep).

**Tests qui touchent une route** (grep) :
- aucun test ne lit un champ de route, n'appelle `pico8_trade_to`/`kg`/`eq`, ni ne manipule `pico8_jo_trade_proposal_t` ;
- `test_pico8_os:421-435` (`export_to`, pour `db`) construit une route par `pico8_trade_new` avec `lp` seul, et déclare `pico8_instance_t *t` ;
- les tests de régression atteints (`built_planet_panel_nil_row`, `playability`, `rich_planet_exports`, `slipway_drag`) ne passent par les routes qu'à travers le code du jeu : **diff vide**.

**Outils** : `tools/float_audit_c1.txt` ne nomme aucune expression de `pico8_trade.c` ; ses lignes de `pico8_db` (`pt_mi`, `oz_mi_val.as.fixed`) et de `bb_visit` ne changent pas de texte. `pico8_trade.c` porte 5 `PICO8_REF_AS_LIST_CONST` (sur `lp`/`rl`), gardés.

**→ Membres de `trade`** : `lp`, `rl`, `qe` (ordre du cart).
**→ Méthodes par le descripteur** : aucune. **Écrivain** : aucun. **Lecteur** : les 3 membres (nil = absent), requis par `bd_route_visit`.

## Solution

1. **Enum : inchangée.**
2. **Struct `trade`**, publique dans `pico8_trade.h` : `pico8_struct_header_t` en premier membre, puis `lp`, `rl`, `qe`.
3. **Constructeur** (`static`, posé par `pico8_class_set_struct_new` dans `pico8_trade_init`, appelé directement par `pico8_trade_new`) :
   1. alloue un `pico8_trade_t`, pose l'en-tête (tag STRUCT, descripteur de `trade`) ;
   2. copie `lp`, `rl`, `qe` des overrides **tels quels** (REF d'origine, nil compris), nil pour une clé absente ;
   3. **asserte** qu'aucune autre clé, nommée ou positionnelle, n'est passée (message porteur, arrêt §7) ;
   4. enregistre `"trade"` au crochet de couverture.

   Overrides NULL : les trois membres nil.
4. **Descripteur de `trade`** (`static const`) : toutes les méthodes NULL ; **lecteur** des 3 membres (nil = absent), rien d'autre ; **écrivain NULL**.
5. **Fonctions de `trade` sur la struct** :
   - `pico8_trade_new` rend `pico8_trade_t *` ;
   - `pico8_trade_to`, `pico8_trade_eq` prennent `const pico8_trade_t *`, `pico8_trade_kg` prend `const pico8_trade_t *` et rend `pico8_trade_t *`. Mêmes gardes NULL ; les membres sont lus directement ;
   - chacune asserte qu'un `self` (ou `l`/`r`) non NULL est une struct au descripteur de `trade` (message porteur) ;
   - `kg` copie `self->qe` tel quel dans la route neuve (voir décisions).
6. **Propositions de route** : `struct pico8_jo_trade_proposal` porte `pico8_trade_t *t` (`planet_trade.h` inclut `pico8_trade.h`).
7. **Consommateurs hors du module** :
   - `jo:cn` (`planet_trade.c`) lit `t->qe` directement ; `extended` typé `pico8_trade_t *` ;
   - `gu_trade` caste les éléments en `const pico8_trade_t *` ;
   - `nr:cn` (`family_trade.c`) lit `t->rl` directement ; `extended` typé `pico8_trade_t *` ;
   - `dh` (`economy_tiers.c:301`) : `pico8_instance_get_field` → `pico8_entity_get_field`, rien d'autre ;
   - `bd_route_visit` : lectures `field_list` inchangées (le lecteur du descripteur répond), cast au seul appel de `pico8_trade_to` ;
   - `db` (`economy_score.c:160`) : cast au seul appel de `pico8_trade_to`.
8. **Classe `trade`** : gardée, enregistrée à la même place (emplacement `""` du registre `fz`), sans `pq`, constructeur de struct posé.
9. **Tests** : nouveau `test_pico8_trade` ; `test_pico8_os` adapté dans sa forme seulement (type de `t`). Voir Testing Decisions.
10. **Plan** : à la ligne 1a.32 de `docs/plan-3.md`, ajouter « Découvert à 1a.32 : … », réduit à ce qui change une étape suivante (§7) — a priori : « pas d'écrivain ; lecteur pour `bd` ; `pico8_jo_trade_proposal_t.t` typé `pico8_trade_t *` ; enum inchangée ».

## User Stories

1. En tant qu'autopilote de la phase 1a, je veux que `trade` soit la seule classe convertie par ce commit, afin de respecter « une classe par commit » et de laisser `hp`, `lq`, `hy` et les instances sans classe à leurs étapes.
2. En tant qu'autopilote, je veux l'emprise de `trade` mesurée (constructions et clés, noms lus et leur source, écritures, `->defaults`), afin que chaque membre, l'absence d'écrivain et l'absence de méthode soient justifiés par une mesure.
3. En tant que lecteur du code, je veux une struct `trade` qui n'a que l'en-tête et `lp`, `rl`, `qe`, noms minifiés, afin que la traçabilité vers `pico8/lua.lua` tienne.
4. En tant que joueur, je veux que les routes d'export se forment comme avant (`jo:ib` propose, `jo:cn` accepte selon `qe`, `nr:cn` relaie par les slipgates), afin que les compteurs « exports: n/m » et les revenus ne changent pas.
5. En tant que joueur, je veux que `bd` marque les slipways traversés (`hg`) et ajoute chaque route au `cc` de sa destination, afin que les niveaux de planète (`dh`) et le score (`db`) restent identiques.
6. En tant que mainteneur, je veux que deux routes soient égales quand elles partent et arrivent aux mêmes planètes, quelle que soit leur longueur, afin que `gu_trade` refuse toujours une proposition en double.
7. En tant que mainteneur, je veux que `kg` rende une route neuve sans toucher l'ancienne, avec `np` ajouté à `lp`, `jc` ajouté à `rl` sauf s'il est nil, et le même `qe`, afin que l'allongement d'une route reste celui du cart.
8. En tant que mainteneur, je veux que le constructeur n'accepte que `lp`, `rl`, `qe` et avorte sur toute autre clé, afin qu'une clé jamais mesurée soit un arrêt (§7).
9. En tant que mainteneur, je veux qu'aucun écrivain ne soit posé, afin qu'une écriture générique jamais mesurée sur une route soit un arrêt.
10. En tant que mainteneur, je veux un lecteur qui répond `lp`, `rl`, `qe`, afin que `bd_route_visit` lise une route par l'API générique sans changer de texte.
11. En tant que mainteneur, je veux que `to`, `kg`, `eq` et le champ `t` des propositions prennent `pico8_trade_t *`, afin que le type dise ce que la mesure a prouvé : ce sont toujours des routes.
12. En tant que mainteneur, je veux que `to`, `kg`, `eq` assertent le descripteur de `trade` sur un pointeur non NULL, afin qu'un élément de `by` ou `cc` qui ne serait pas une route soit détecté au lieu d'être lu de travers.
13. En tant que mainteneur, je veux que plus aucun site hors de `pico8_class.c` ne passe une route à `pico8_instance_get_field`, afin que la forme struct ne soit jamais lue par la table d'une instance.
14. En tant que gardien de la couverture, je veux que le constructeur enregistre `"trade"` une fois par route, afin que `class-coverage-net` reste `trade  21032  6/15`.
15. En tant que gardien du filet, je veux qu'aucun `.sha256`, aucune entrée de `FRAMES_EVENTS`/`RECORDINGS`, aucune ligne des références de couverture ni de `float_audit_c1.txt` ne bouge, afin que l'étape ne se valide pas en déplaçant sa cible.
16. En tant que vérificateur, je veux une caractérisation de `trade` figée sur l'ancien code, afin de prouver que la conversion ne change rien d'observable, y compris ce que le filet ne voit pas (`qe` des routes, `eq` sur des routes de longueurs différentes, `jc` nil dans `kg`).
17. En tant que vérificateur, je veux un rapport `gcov` montrant chaque ligne ajoutée ou modifiée exécutée, et 2 à 3 mutations ciblées, afin de juger la preuve sans monter mon propre banc.
18. En tant que gardien des caractérisations, je veux que `test_pico8_os` ne change que le type de sa route, sans valeur figée touchée, afin que l'étape de `os` reste prouvée.
19. En tant que gardien des tests de régression, je veux qu'aucun test de régression ne change, afin que leur objet reste intact (aucun ne lit une route).
20. En tant que lecteur du code, je veux que les commentaires ajoutés ne portent qu'un contrat C, un piège ou un `TROU-N` nu (CLAUDE.md), et que ceux que ce changement rend faux soient corrigés ou retirés.
21. En tant qu'autopilote, je veux qu'un écart port/cart découvert soit préservé et décrit par un `TROU-N`, afin que la conversion ne corrige rien en passant.
22. En tant que développeur, je veux que `make test` reste rapide, afin qu'un test qui dépasse 30 s tourne sous `make e2e` (`UNIT_TESTS_SLOW`).

## Implementation Decisions

Aucun outil de création d'agent n'est disponible dans cette session pour lancer un agent `guideline` frais aux points où `to-prd` consulte l'humain (modules attendus, modules à tester, découpage). J'ai appliqué moi-même le skill `guideline` : hypothèses explicites, simplicité, changement chirurgical, critères vérifiables. Les décisions tranchées ainsi sont marquées (G).

- **Modules touchés** :
  - la route `trade` : struct, constructeur, descripteur, lecteur, `to`/`kg`/`eq` sur la struct ;
  - les propositions de route (`planet_trade.h`) : type de `t` ;
  - `jo:cn`, `gu_trade`, `jo:ib` (`planet_trade.c`), `nr:cn` (`family_trade.c`) : types et lecture directe du membre ;
  - `dh`, `bd_route_visit` (`economy_tiers.c`), `db` (`economy_score.c`) : un appel d'API ou un cast.

  Rien d'autre dans `src/`. Pas de changement dans `entity`, `pico8_struct.h` (enum), `class`, `cart.c`.
- **Clés acceptées : `lp`, `rl`, `qe`, chacune facultative** (G). Le jeu passe toujours les trois ; `test_pico8_os` passe `lp` seul (2 constructions), et un membre nil de trop est sans effet (§5). Rejeté : exiger les trois (modifierait l'intention de `export_to`).
- **Lecteur, pas de méthode, pas d'écrivain** (G). `bd_route_visit` lit `lp`/`rl` par `pico8_entity_get_field` (43 774 fois chacun dans les TU) : le lecteur le sert sans toucher ce site. Aucun nom de méthode n'est cherché sur une route ; aucune écriture générique.
- **Lectures hors du module** (G) : là où la variable est typée route (`t` de `jo:cn`/`nr:cn`, qui vient de `lw->t`), lecture directe du membre ; là où la route est un élément d'une liste générique (`cc` dans `dh`, `by` dans `bd`/`db`), l'API générique ou un cast au seul appel de `pico8_trade_to` — changement minimal, même sémantique (nil = absent, et `qe`/`rl` ne sont jamais nil mesuré).
- **`qe` absent dans `kg`** (tranché par l'orchestrateur, §3 et §7 du plan). Aujourd'hui, un `self` sans `qe` donne `qe=false` à la route neuve. Mesuré : **0 passage** sur 18 rejeux et toutes les TU. Le comportement du port est **préservé tel quel** : `kg` pose `false` quand `self->qe` est nil (un écart port/cart se préserve, ne se corrige jamais dans ce plan). La caractérisation fige `kg` sur une route sans `qe` (`qe` rendu `false`), et gcov exige la branche exécutée.
- **Asserts d'entrée de `to`/`kg`/`eq`** (G) : un pointeur non NULL est une struct au descripteur de `trade` (message porteur). Les gardes NULL restent (contrats d'en-tête inchangés). Le contrat de `pico8_trade.h` est réécrit : « absent, non-REF ou vide » devient « nil ou vide ».
- **`pico8_trade_eq` reste une fonction nommée** : l'identité de pointeur de `pico8_gu`/`pico8_value_equal` et `gu_trade` ne changent pas (commentaire de l'en-tête gardé, relu).
- **Pas d'ordre de constructeur** : ni `pq`, ni `lt`, ni défaut. La mutation « ordre du constructeur » du §7 est sans objet ; remplacée ci-dessous.
- **Mémoire** : la struct n'est jamais libérée, comme l'instance d'avant (TROU-7, déjà au site de `kg`) ; `lp`/`rl` sont empruntés (REF des overrides, non copiés). Contrat à écrire à la déclaration de la struct.
- **Commentaires** (CLAUDE.md) :
  - à ajouter : contrat de la struct (qui libère : personne, TROU-7 ; `lp`/`rl` empruntés) ; l'assert de clé ; l'assert de descripteur ;
  - à corriger ou retirer, rendus faux : l'en-tête de `pico8_trade.h` (« root class […] instance ») s'il parle d'instance ; les contrats de `to`/`kg`/`eq` qui parlent de champ « absent, non-REF » ; celui de `gu_trade` (« freshly built instance ») ;
  - tout le reste va dans le message de commit.

## Implementation Chunks

**Un seul commit** (§7). Chaque morceau laisse `make test` vert. La caractérisation est figée sur l'ancien code **avant toute modification de `src/`**.

1. **Caractérisation `test_pico8_trade` sur l'ancien code** (voir Testing Decisions), valeurs figées. Mesurer la durée : > 30 s → `UNIT_TESTS_SLOW`, sinon `UNIT_TESTS`. Arrêt ici si une route du jeu n'a pas exactement `lp`/`rl`/`qe` : la mesure est fausse et doit être corrigée dans le PRD.
2. **`trade` converti** : struct, constructeur, descripteur, lecteur, `set_struct_new` ; `to`/`kg`/`eq` sur la struct ; `pico8_jo_trade_proposal_t.t` ; `jo:cn`, `gu_trade`, `jo:ib`, `nr:cn`, `dh`, `bd_route_visit`, `db` ; `test_pico8_os` dans sa forme. Toutes les caractérisations passent **sans changer leurs valeurs figées** ; les vérifications de forme s'ajoutent à `test_pico8_trade`.
3. **Preuves** : `make e2e`, rapport `gcov`, 2 à 3 mutations, `make class-coverage-net`, `make ref-audit`, ligne 1a.32 du plan. Commit.

## Testing Decisions

- **Un bon test observe le comportement** : ce que rendent `pico8_trade_to`/`kg`/`eq` et `pico8_entity_get_field`, le contenu et l'identité des listes `lp`/`rl`, `by`/`cc` des planètes, les pixels, le compte de couverture. Jamais l'agencement mémoire au-delà du tag et du descripteur.
- **Nouveau test `test_pico8_trade`** : lié comme `test_pico8_ui`/`test_pico8_pj` (cart réel, `ENGINE_SRCS`), avec des dépendances Makefile sur les fixtures qu'il rejoue.
  - Il lit les routes **uniquement** par `pico8_entity_get_field` et les fonctions publiques `to`/`kg`/`eq`, en passant des `void *` (le même code compile avant et après, aux vérifications de forme près).
  - Chaque scénario tourne dans un fils forké juste après `pico8_cart_load()`, pour l'état de `rnd` ; la partie est lancée comme le filet.
  - Toute présence se lit par un `lookup` qui confond nil et absent.
  - **Caractérisation dirigée, figée sur l'ancien code** :
    1. **Construction** : `pico8_trade_new` avec `lp`/`rl`/`qe` (listes de vraies planètes d'une partie, `qe` une ressource) : les trois lus sont les REF passées ; absents : `to`, `kg`, `me`, `nu`, `id` ; overrides NULL : les trois absents ; `lp` seul (forme de `test_pico8_os`) : `rl`, `qe` absents.
    2. **`to`** : dernier élément de `lp` ; `lp` vide ou nil → 0 et sortie intacte ; NULL → 0.
    3. **`kg`** : route neuve (pointeur différent), `self` inchangé (mêmes REF, mêmes comptes) ; `lp` = ancien + `np` ; `rl` = ancien + `jc`, **inchangé en longueur** si `jc` nil ; `qe` = même valeur ; NULL → NULL.
    4. **`eq`** : même départ et même arrivée, longueurs différentes → 1 ; départ différent → 0 ; arrivée différente → 0 ; `lp` vide d'un côté → 0 ; NULL → 0.
  - **Rejeux courts** (départ comme le filet, jusqu'à la dernière frame) : `built_planet_panel_events.json` (9 routes : `jo:ib`, `kg`, `eq`, `to`, `jo:cn`, `dh`, `bd`) et `trading_hub_session_events.json` (`--from-title` ; 204 routes, seul rejeu court qui atteint `nr:cn`). À chaque frame, pour chaque planète de `cg.jo` dans l'ordre : le compte de `by` et de `cc` ; pour chaque route de `by`, les rangs dans `hv` des planètes de `lp` et des slipways de `rl`, `qe`, le rang de `to()`. S'ajoute une empreinte d'écran finale et le nombre de routes construites (compteur local du test, pas la couverture). Figé.
  - **Forme** (ajoutée au morceau 2) :
    - une route est `STRUCT` au descripteur de `trade` (accesseur public du descripteur, comme `pico8_ui_struct_class()`) ; toutes les méthodes NULL ; `field` posé, `set_field` NULL ;
    - `pico8_entity_set_field(route, "qe", …)` avorte (dans un fils) ;
    - `pico8_trade_new` avec une clé autre (`"x"`) ou positionnelle avorte ;
    - `pico8_trade_to` sur une struct d'une autre classe (le `ok` de la partie) avorte ;
    - `pico8_trade_class()->struct_new` est posé, son `pq` NULL.
- **Tests existants** : `test_pico8_os` ne change que le type de `t` dans `export_to` ; tous les autres, dont les tests de régression atteints (`built_planet_panel_nil_row`, `playability`, `rich_planet_exports`, `slipway_drag`), gardent un **diff vide** et restent verts.
- **Preuve des tests (§7, à partir de 1a.23)** :
  - **`gcov`** : dans une copie jetable du scratchpad (arbre du commit candidat, supprimée après usage), compiler en `--coverage -O0` `test_pico8_trade`, `test_pico8_os` et `test_pico8_nr`, avec les objets du moteur. Les lancer, puis `gcov` sur `pico8_trade.c`, `pico8_planet_trade.c`, `pico8_planet_family_trade.c`, `pico8_economy_tiers.c`, `pico8_economy_score.c`.
    - Même piège qu'à 1a.23-1a.31 : dans la copie seulement, `-include` d'un en-tête qui fait précéder les `_exit(c)` de `__gcov_dump()`.
    - Les branches d'assert atteintes seulement dans un fils qui avorte ne sont pas comptées : elles sont prouvées par le `SIGABRT` attendu.
    - Le rapport est collé dans le message de commit : lignes exécutables ajoutées ou modifiées de `src/` et leur compte (constructeur, lecteur, `to`/`kg`/`eq` y compris `jc` nil, `jo:cn`, `gu_trade`, `jo:ib`, `nr:cn`, `dh`, `bd_route_visit`, `db`). Les lignes à 0 sont données avec leur justification.
  - **2 à 3 mutations**, chacune rouge sur `test_pico8_trade` (ou un autre test nommé), notées dans le commit avec le test qui l'attrape :
    1. **membre du constructeur** (à la place de « valeur d'un défaut propre », `trade` n'en a pas) : `qe` non copié. `jo:cn` n'accepte plus aucune route ; scénario 1 et rejeux rouges ;
    2. **entrée du descripteur** (le lecteur, seule entrée) : le lecteur répond `lp` pour `rl`. `bd` marque d'autres `hg` ; rejeu `built_planet_panel` rouge ;
    3. **corps de `kg`** (à la place de « ordre du constructeur », sans objet) : `qe` de la route neuve pris à nil, ou `jc` nil ajouté à `rl`. Scénario 3 rouge.
- **`make ref-audit`** (hors suite) : lancé une fois, sans nouveau `NOT COVERED` (les 5 `PICO8_REF_AS_LIST_CONST` de `pico8_trade.c` restent).
- **Couverture** : `make class-coverage-net` rend 0, sortie identique à la référence.
- **Prior art** :
  - `test_pico8_ui`/`test_pico8_pj` : structure, `lookup`, `must_abort`, fils par scénario, rejeu court, forme ;
  - `test_pico8_jb_ol` et le PRD de 1a.1 : racine sans parent, constructeur posé par `set_struct_new` ;
  - `test_pico8_os` (`export_to`, `fold_db`) : construire une route à la main ; `test_pico8_nr` (`test_hub`) : rejeu de `trading_hub_session` avec `--from-title`.

## Critères du vérificateur

Chacun se traduit en `FAILED:` par le vérificateur de `prd-autopilot` (§7 ; c'est le seul arrêt que `plan-pilot` honore). Le vérificateur **relit le diff et le rapport `gcov`** du message de commit ; il ne monte **pas** de banc de mutations propre.

- `FAILED:` si `make test` ou `make e2e` est encore rouge après **3 tentatives** de correction sur ce commit.
- `FAILED:` si le diff modifie ou supprime un `*.sha256` du filet, ou retire une entrée de `FRAMES_EVENTS` ou de `RECORDINGS`.
- `FAILED:` si `make class-coverage-net`, relancé sur le commit, ne rend pas 0 ou si sa sortie n'est pas identique à `tools/class_coverage_net_expected.txt` (en particulier `trade  21032  6/15`). Également si ce fichier, `tools/class_coverage_exempt.txt` ou `KNOWN[]` est modifié.
- `FAILED:` si le message de commit ne contient pas le rapport `gcov` (lignes ajoutées ou modifiées de `src/` et leur compte), si une ligne ajoutée ou modifiée y est à 0 sans test ajouté ni justification, ou si les 2 à 3 mutations ne sont pas notées avec le test qui les attrape.
- `FAILED:` si un champ ou une clé ne tombe sous aucune règle du §5 et reçoit pourtant un membre ou un traitement improvisé. En particulier :
  - si un membre autre que `lp`, `rl`, `qe` est ajouté ;
  - si le constructeur accepte une autre clé sans avorter, ou si son assertion se déclenche sur un rejeu ou un test hors `must_abort` ;
  - si un écrivain est posé, ou une entrée de méthode non NULL ;
  - si l'enum change.
- `FAILED:` si un site hors de `pico8_class.c` passe une route à `pico8_instance_get_field` ou `pico8_instance_fields` (grep dans `src/` et `tests/`), ou si `pico8_trade.c` appelle encore `pico8_class_instantiate`.
- `FAILED:` si `pico8_trade_to`, `kg` ou `eq` acceptent un pointeur non NULL qui n'est pas une route sans avorter, ou changent de comportement sur une route (valeur figée de `test_pico8_trade`).
- `FAILED:` si une autre classe que `trade` est convertie ou change de forme (`hp`, `lq`, `hy`, `fm`, les enveloppes…), si la classe `trade` cesse d'être déclarée et enregistrée à la même place, ou si le comportement d'une autre classe change (valeur figée de sa caractérisation, ligne de couverture).
- `FAILED:` si un test de régression (§6) a un diff non vide.
- `FAILED:` si une valeur figée de `test_pico8_trade` a été changée après la conversion, ou une de `test_pico8_os` ou d'une autre caractérisation. Également si `test_pico8_os` change au-delà du type de sa route.
- `FAILED:` si une ligne est ajoutée à `tools/float_audit_c1.txt` ou en est retirée, ou si `make float-audit` est rouge.
- `FAILED:` si `test_pico8_trade` dure plus de 30 s et n'est pas dans `UNIT_TESTS_SLOW`.
- `FAILED:` si un écart port/cart découvert est corrigé au lieu d'être préservé et décrit par un `TROU-N`.
- `FAILED:` si la note « Découvert à 1a.32 » manque à la ligne 1a.32 de `docs/plan-3.md`.

Critères de réussite (en plus) :

- la struct `trade` n'a que l'en-tête et les 3 membres ; son descripteur n'a aucune méthode, un lecteur, pas d'écrivain ;
- grep :
  - `PICO8_CLASS_COVERAGE_NAME(g_trade_class, "trade")` reste ;
  - `pico8_class_set_struct_new(g_trade_class, …)` est présent ;
  - aucun `pico8_instance_get_field` dans `pico8_trade.c`, ni sur `t` dans `pico8_planet_trade.c`/`pico8_planet_family_trade.c`, ni à `dh` ;
  - `pico8_jo_trade_proposal_t.t` est `pico8_trade_t *` ;
- le message de commit dit « découvert à 1a.32 ».

## Out of Scope

- Les autres classes et instances sans classe : `hp` (1a.33), `lq` (1a.34), `hy` (1a.35), `fm` (1a.36), les `{}`/`et={}`/`{qs}`/`re` jetables (1a.37-1a.40), l'enveloppe de `pz` sur `g_ha` (1a.41).
- Les propositions de route `lw` elles-mêmes (déjà une struct C locale, `pico8_jo_trade_proposal_t`), les listes `by`/`cc` et les planètes qu'elles portent.
- Supprimer la classe `trade`, les registres et `pico8_class.c` : 1a.42.
- Typer les membres (`lp`/`rl` en `pico8_list_t *`, `qe` en ressource) : phase 1b. Faire de `eq` un opérateur ou changer `pico8_gu`/`pico8_value_equal` : hors plan.
- Libérer les routes et leurs listes (TROU-7, Plan 2 §8).
- L'audio : hors périmètre. Lancer l'appli raylib : jamais.

## Further Notes

- **Racine, pas de parente à fermer** : aucune fonction partagée ne passe à la forme struct seule ici ; aucune double forme ouverte ne se solde.
- **Ce que le filet voit des routes** : les compteurs d'exports et les revenus, sur 6 rejeux. Il ne voit pas `qe` directement, ni `eq` sur des routes de longueurs différentes, ni `kg` avec `jc` nil séparément : la caractérisation dirigée et les mutations 1 et 3 les visent.
- **Plus petite emprise que les étapes précédentes** : 3 membres, aucune méthode, aucun écrivain, aucun défaut ; le PRD est proportionné en conséquence.
- Mesures :
  - grep sur `src/`, `tests/`, `tools/` et le `Makefile` à 0adebfd ;
  - lecture de `pico8/lua.lua:86-88,478-510,603-611,1116-1121,1264-1298,1327-1331` ;
  - lecture de `pico8_trade.c/h`, `pico8_planet_trade.c/h`, `pico8_planet_family_trade.c`, `pico8_economy_tiers.c`, `pico8_economy_score.c`, `pico8_class.c`, `pico8_entity.c`, `pico8_struct.h`, `pico8_layout.c` (`jb`), `test_pico8_os`, `test_pico8_nr`, de `tools/class_coverage_net_expected.txt` ;
  - lecture des PRD de 1a.1 et 1a.31 ;
  - sonde dans une copie jetable du scratchpad, supprimée après usage : 18 rejeux en sortie 0 ; 21 032 routes sur le filet, toutes `lp`/`rl`/`qe` non nil (15 144 par `jo:ib`, 5 888 par `kg`) ; lectures toujours propres, jamais absentes ; aucune écriture ; `qe` jamais absent dans `kg` ;
  - fil piège (`make unit` puis `UNIT_TESTS_SLOW`, même copie, sortie 0) : 87 220 routes dans 18 binaires, mêmes sites de lecture, aucune écriture ; seule exception de clés : `test_pico8_os:431` (`lp` seul, 2 fois).
