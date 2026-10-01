---
title: "Plan 3, étapes D.21b1/b2 : les branches de pli jamais exécutées des TU deviennent des `assert` à message — `fold_entity` de `pj` supprimée, non-NULL de `fold_untagged` (`ga`) et `0u` de `fold_ref` (`lq`) exclus, repli `FROZEN_STRUCT_TAG` hors `hv` retiré de sept TU ; aucune empreinte ne bouge"
description: "La sortie de D a laissé dans les TU des branches de pli que Md1, Md4, Me1 et Me2 laissent vertes, au parent comme à l'enfant : elles ressemblent à de la couverture et n'en sont pas. D.21b1 (pj, ga, lq) et D.21b2 (g_, ga, hj, mx, nl, os, qh) remplacent chacune par un `assert` exécuté qui exclut le cas, ou la suppriment avec son seul appelant mort. `FROZEN_STRUCT_TAG` passe de 12 TU / 28 occ. à 4 TU (fr, hy, js, nr) / 12 occ."
status: needs-triage
---

## Problem Statement

**Écrit sur l'arbre commis `d6db288`** (branche `claude/plan-pilot-3-7zdft0`), pendant qu'un autre agent a des éditions D.21a non commitées dans `src/pico8_planet.h`, `src/pico8_planet_family_trade.h`, `src/pico8_ui.c` et `docs/trous-de-portage.md`. Aucun fichier de `tests/` n'y est touché : **les numéros de ligne de `tests/` cités ici sont ceux de `d6db288` et restent ceux du parent de D.21b1**. Seuls `grep`, `sed` et `git show` ont tourné. **Rien n'a été compilé ni lancé.** Ce que ce PRD dit de l'exécution vient des notes « Découvert à D.20a1 », « D.20a2 », « D.20b2 » (`docs/plan-3.md`, l. 294-295), des amendements de sonde de `docs/prd/done/plan3-phaseD-etape20a-tu-sans-marqueur-de-type.md`, et du point M2 de la revue de sortie de D. L'implémenteur le revérifie par `gcov` au parent avant toute édition (« Testing Decisions »).

Du point de vue du mainteneur : plusieurs TU gardent des branches de pli écrites « au cas où » pendant D.20a. Elles plient des mots figés (`FROZEN_STRUCT_TAG`, `0u`, `0xEEEE`) dans des empreintes gelées, mais **aucun test ne les atteint** : une mutation de leurs octets ne rougit rien, au parent comme à l'enfant. Un lecteur y voit une couverture qui n'existe pas, sous les noms de la couche retirée (« tag », « untagged »). Et si un jour un test y arrivait, il plierait en silence des octets que personne n'a jamais vérifiés.

### Les branches et la preuve qu'elles sont mortes

| # | Sous-étape | Site (`d6db288`) | Ligne exacte | Preuve de mort |
|---|---|---|---|---|
| P1 | b1 | `test_pico8_pj.c:221-230` (`fold_entity`) et `:233` (`fold_ed`) | `:233` `    return pj->ed != NULL ? fold_entity(fnv(h, 1u), pj->ed) : fold_fixed_member(h, ed_fixed(pj));` | Sonde D.20a2 : `pj->ed` n'atteint jamais `fold_ref`, les tests n'ont que `ed` NULL (`gcov` : branche non NULL de `fold_ed` à 0, au parent comme à l'enfant). **Me1 `pj`** (`FROZEN_STRUCT_TAG` → `0x9E5C57C8u`) vert, vert au parent. Les trois octets de `fold_entity` mutés : verts. |
| G1 | b1 | `test_pico8_ga.c:133-139` (`fold_untagged`) | `:138` `    return fnv(fnv(h, 0xEEEEu), 0u);` | Sonde D.20a1 : les deux appels (items, aujourd'hui `:467` ; `lq->ra`, aujourd'hui `:524`) **jamais exécutés**. **Md4 `ga`** (`0xEEEE` → `0xEEEF`) et `0u` → `1u` verts, verts au parent. |
| L1 | b1 | `test_pico8_lq.c:105-112` (`fold_ref`) | `:111` `    return fnv(h, 0u);` | Sonde D.20a2 : `lq:112` (le test d'étiquette d'alors, après le test NULL) jamais atteint. **Me2 `lq`** (`0u` → `1u`) vert, vert au parent. D.20b2 l'a écrit en commentaire (`:105` « this TU folds no non-NULL one »). |
| S1 | b2 | `test_pico8_g_.c:133` | `    return fnv(fnv(h, 0xEEEEu), FROZEN_STRUCT_TAG);` | **Md1 `g_`** vert, vert au parent. Le `fold_ref` de `g_` n'a qu'un appelant, `fold_et` (`:462`), qui écarte `et` NULL ; la sonde après coupe voit `fold_ref` ENTITY 3 et `is_bv_target` ENTITY 8 : la branche `is_bv_target` (`:130-132`) **est exécutée**. |
| S2 | b2 | `test_pico8_ga.c:130` | `    return fnv(fnv(h, 0xEEEEu), FROZEN_STRUCT_TAG);` | **Md1 `ga`** vert, vert au parent. Sonde D.20a1 : `ga:127` jamais atteint, `fold_ref_member(jo.lq)` NULL seul ; le test le dit lui-même (`:600-601` : `fold_ref_member(h, as_ga(a)->jo.lq); assert(as_ga(a)->jo.lq == NULL);`). **`fold_ref` de `ga` n'est jamais entré.** |
| S3 | b2 | `test_pico8_hj.c:140` | idem | **Md1 `hj`** vert, vert au parent. Sonde D.20a1 : `hj:137` et `hj:122` (`is_bv_target`) jamais atteints ; la branche `is_bv_target` (`:137-139`) est donc morte elle aussi. |
| S4 | b2 | `test_pico8_mx.c:129` | idem | **Md1 `mx`** vert, vert au parent. Le seul appel de `fold_ref` est `:504` `fold_ref(h, NULL)` : **aucun pointeur non NULL n'y entre**, la branche `listed` (`:126-128`) est morte elle aussi. |
| S5 | b2 | `test_pico8_nl.c:124` | idem | **Md1 `nl`** vert, vert au parent. Sonde D.20a1 : `nl:121` jamais atteint. |
| S6 | b2 | `test_pico8_os.c:138` | idem | **Md1 `os`** vert, vert au parent. Sonde D.20a1 : `os:135` et `os:120` (`is_bv_target`) jamais atteints ; branche `is_bv_target` (`:135-137`) morte. |
| S7 | b2 | `test_pico8_qh.c:155` | idem | **Md1 `qh`** vert, vert au parent. Sonde D.20a1 : `qh:152` jamais atteint, mais `is_bv_target` ENTITY (`qh:137`, 1 appel) ; `:417` asserte une cible `et` hors de `hv` : la branche `is_bv_target` (`:152-154`) **est exécutée**. |

Chaque TU de S1-S7 définit aussi `FROZEN_STRUCT_TAG` avec son commentaire de piège, deux lignes au-dessus : `g_:49-51`, `ga:49-51`, `hj:46-48`, `mx:52-54`, `nl:47-49`, `os:45-47`, `qh:45-47`, et `pj:42-44` pour P1.

**Hors périmètre** : les TU `fr`, `nr`, `hy`, `js`, qui plient `FROZEN_STRUCT_TAG` sur un chemin vivant (`fr:497`, `nr:483`, `hy:289`, `js:293` ; Md1/Me1 rouges) et gardent donc leur `#define` et leurs replis (`fr:128`, `nr:126`, `js:216`, `js:1111`), non examinés ici, le `fold_untagged` de `g_` (Md4 et `0u → 1u` rouges), le `0u` final du `fold_ref` de `pj` (`:201`, Me2 `pj` rouge), `hp:135`.

### Comptes de départ (vérifiés sur `d6db288`)

- `grep -c FROZEN_STRUCT_TAG tests/*.c` : `fr` 3, `g_` 2, `ga` 2, `hj` 2, `hy` 2, `js` 4, `mx` 2, `nl` 2, `nr` 3, `os` 2, `pj` 2, `qh` 2 → **12 TU / 28 occ.** ; `src/` : 0.
- `fold_entity` dans `tests/` : 2 occ. (`pj:222`, `:233`).
- `FROZEN_INSTANCE_TAG` : `g_` 2, `hj` 2, `js` 3, `os` 2, `qh` 2 → 5 TU / 11 occ.
- `is_bv_target` : `g_` 5, `hj` 2, `mz` 3, `os` 2, `qh` 2.

## Solution

Du point de vue du mainteneur : un pli de TU ne contient plus que des branches que les tests exécutent. Là où un cas n'arrive jamais, un `assert` à message l'exclut, et il dit quoi faire si un test l'atteint un jour. `FROZEN_STRUCT_TAG` ne reste que dans les quatre TU qui le plient vraiment (`fr`, `hy`, `js`, `nr`). **Aucune constante `*_HASH`, aucun `FROZEN_*` encore lu ne change ; aucune ligne de `src/`, du `Makefile`, de `tools/` ou de `tests/e2e/` ne change.**

Deux commits, un par sous-étape : `refactor(D.21b1): …` (`pj`, `ga`, `lq`, puis la note du plan) et `refactor(D.21b2): …` (`g_`, `ga`, `hj`, `mx`, `nl`, `os`, `qh`, puis la note).

### La règle de la dernière condition vivante (liant, G)

Un `assert` qui n'est jamais exécuté ne prouve rien : la preuve (2) (« `gcov` après : chaque `assert` exécuté ») et la preuve (3) (« `assert` nié → rouge ») exigent qu'il soit sur un chemin vivant. D'où une règle unique, appliquée à chaque site avec le `gcov` du parent :

1. Dans la fonction du site, on lit au parent le compte `gcov` de chaque branche (`if` et son corps, bras d'un ternaire).
2. La **dernière branche vivante** (corps exécuté au moins une fois) devient `assert(<sa condition> && "<message>");` suivi de son corps, désindenté. Les branches **qui la suivent** (toutes à `#####`) partent. Les branches **qui la précèdent** restent telles quelles, mortes ou non.
3. Si aucune branche de la fonction n'est vivante (la fonction n'est **jamais entrée**), la règle s'applique **au site d'appel**, dans l'aide qui l'appelle ; la fonction, devenue orpheline, part (`-Werror=unused-function`). On remonte au plus d'aide en aide ; **jamais dans le corps d'une fonction `test_*` ni dans une fonction qui appelle `check`** : si la remontée devrait y entrer, le site est laissé en place, déclaré, et ses comptes sortent de la cible (repli, plus bas).
4. Une branche qui a un compte non nul **reste**, dans ce TU, et le commit le déclare.

**Pourquoi l'empreinte ne peut pas bouger.** Sur toute entrée qu'un test fournit, l'ancienne fonction a pris une branche vivante. La nouvelle évalue les mêmes conditions précédentes, dans le même ordre (`listed`, `is_bv_target`, les tests NULL sont purs : `pico8_hv()` rend `&g_hv`), puis la condition de l'`assert`, qui est vraie par construction (c'est la condition de la dernière branche vivante, et aucune branche postérieure n'a jamais été prise), puis le même `return`. Les lignes retirées ont un compte `gcov` nul : elles n'ont écrit aucun octet dans aucune empreinte. `NDEBUG` n'est défini nulle part (`CFLAGS ?= -std=c99 -Wall -Wextra -Werror -g`), donc la condition est toujours évaluée. Enfin Md1, Md4, Me1, Me2, rejouées au parent, restent vertes : muter les octets de ces branches n'a jamais changé une empreinte.

### Sites, forme prévue et suppression (prévision ; `gcov` au parent tranche)

**D.21b1**

- **P1 `pj`** — `fold_ed` est exécutée (`test_…` qui plie `other`, `:427`), sa branche non NULL ne l'est pas. `:233` devient :
  `assert(pj->ed == NULL && "<message>");` puis `return fold_fixed_member(h, ed_fixed(pj));`.
  `fold_entity` (`:221-230`, commentaire compris), son seul appelant étant mort, **est supprimée**. `FROZEN_STRUCT_TAG` (`:42-44`, commentaire compris) n'a plus de lecteur : **supprimé**. `listed`, `rank_in`, `pico8_mh_t` gardent d'autres lecteurs (`fold_ref` `:189`, `:740`, `:757`). Message, sens imposé : aucun test ne plie un `pj` qui tient un `ed` ; en laisser passer un demande de plier son rang dans `hv` (`0x4E` + rang) comme `fold_ref`, ou `0xEEEE` puis le tag figé `0x9E5C57C7` hors de `hv`.
- **G1 `ga`** — prévision : `fold_untagged` n'est jamais entrée (sonde D.20a1). Règle 3 aux deux sites d'appel :
  - `fold_tuple` (`:466-470`) : le bras `} else if (item->is_ref) { h = fold_untagged(h, item->ref); }` (`:466-467`) part ; le `else` final (`:468-470`) devient `} else { assert(!item->is_ref && "<message>"); h = fold_fixed(…); }`. Si ce `else` est lui-même à `#####`, le site passe en repli (règle 3), déclaré ;
  - `:523-526` `if (lq->ra != NULL) { h = fold_untagged(…); n++; }` → `assert(lq->ra == NULL && "<message>");`.
  `fold_untagged` (`:133-139`, commentaire compris) devient orpheline : **supprimée**.
  **Si `gcov` montre `fold_untagged` entrée** (`:136` exécutée), la forme du plan s'applique à sa place : `:138` → `assert(p == NULL && "<message>")` après le test NULL, qui devient lui-même la dernière branche vivante (`assert(p == NULL …); return fnv(h, 0xDEADu);`), et la fonction reste. Message, sens imposé : ce TU ne plie aucun item à pointeur ni `lq->ra` non NULL ; le pli qu'avaient les empreintes pour ce cas est `0xEEEE` puis `0`.
- **L1 `lq`** — si `fold_ref` est entrée avec NULL (`:109` exécutée) : `:108-111` deviennent `assert(ref == NULL && "<message>"); return fnv(h, 0xDEADu);` après la ligne `REF_KIND` (`:107`), qui reste. Le commentaire `:105` devient faux (« Items fold as untagged pointers (0) ») : **supprimé**, son contenu passe au message. Si `fold_ref` n'est jamais entrée : règle 3 dans `fold_tuple_item` (`:122-133`), `fold_ref` orpheline supprimée ; `REF_KIND` garde son lecteur `:140`. Message, sens imposé (plan : « ce qui le rendrait atteignable ») : un item de tuple qui pointe (rangée, `lq`, vecteur) arriverait ici ; les empreintes le pliaient `0`, sans rang.

**D.21b2**

| Site | Branches vivantes prévues | Forme prévue | Part avec | Orphelins prévus |
|---|---|---|---|---|
| S1 `g_` | NULL ?, `listed` ?, `is_bv_target` **oui** | `:130` `if (is_bv_target(ref)) {` → `assert(is_bv_target(ref) && "…");`, `:131` désindentée | `:132-133` | `FROZEN_STRUCT_TAG` `:49-51` |
| S2 `ga` | aucune (jamais entrée) | règle 3 : `fold_ref_member` `:143` → `assert(ref == NULL && "…"); return fold_absent(h);` | `fold_ref` `:123-131` | `fold_ref`, `FROZEN_STRUCT_TAG` `:49-51` ; `listed` (`:414`), `rank_in` (`:290`, `:409`…) restent lus |
| S3 `hj` | NULL ?, `listed` ? ; `is_bv_target` **non** | `:133` → `assert(listed(pico8_hv(), ref) && "…");` (ou `assert(ref == NULL …)` si `listed` est mort) | `:137-140` | `is_bv_target` `:124-127`, `FROZEN_INSTANCE_TAG` `:42-44`, `FROZEN_STRUCT_TAG` `:46-48` ; include de `pico8_struct.h` si c'était son dernier usage (vérifié par l'implémenteur) |
| S4 `mx` | NULL seul | `:123` → `assert(ref == NULL && "…"); return fnv(h, 0xDEADu);` | `:126-129` | `FROZEN_STRUCT_TAG` `:52-54` ; `listed`, `rank_in` restent lus |
| S5 `nl` | NULL ?, `listed` ? | comme `hj` sans `is_bv_target` | `:124` (et `:121-123` si `listed` mort) | `FROZEN_STRUCT_TAG` `:47-49` |
| S6 `os` | comme `hj` | comme `hj` | `:135-138` | `is_bv_target` `:122-125`, `FROZEN_INSTANCE_TAG` `:41-43`, `FROZEN_STRUCT_TAG` `:45-47` ; include comme `hj` |
| S7 `qh` | `is_bv_target` **oui** | comme `g_` : `:152` → `assert(is_bv_target(ref) && "…")` | `:154-155` | `FROZEN_STRUCT_TAG` `:45-47` |

Message commun, sens imposé : aucun test de ce TU ne plie une entité hors de `hv` (ou, selon la forme, un `et` non NULL / un pointeur) ; si un test en plie une, les empreintes figées pliaient `0xEEEE` puis `0x9E5C57C7`, comme `test_pico8_fr.c`.

**Comptes de Sortie attendus**

| Motif | Départ | Après D.21b1 | Après D.21b2 |
|---|---|---|---|
| `FROZEN_STRUCT_TAG` (TU / occ.) | 12 / 28 | 11 / 26 (`pj` sort) | **4 / 12** : `fr` 3, `hy` 2, `js` 4, `nr` 3 |
| `fold_entity` dans `tests/` | 2 | **0** | 0 |
| `FROZEN_INSTANCE_TAG` (TU / occ.) | 5 / 11 | 5 / 11 | prévu 3 / 7 (`g_`, `js`, `qh`) si `hj` et `os` perdent leur branche `is_bv_target` ; sinon 5 / 11, déclaré |
| `is_bv_target` | `g_` 5, `hj` 2, `mz` 3, `os` 2, `qh` 2 | idem | prévu `g_` 5, `mz` 3, `qh` 2 |
| `fold_untagged` (TU) | `g_`, `ga` | prévu `g_` seul | `g_` |
| `*_HASH` changés | — | 0 | 0 |

Tout écart à ces prévisions vient d'un `gcov` au parent qui montre une branche vivante (règle 4) ou d'un repli (règle 3) : il est déclaré au commit avec la ligne et son compte, et les comptes sont recalculés.

## User Stories

1. En tant que mainteneur, je veux qu'un pli de TU ne contienne que des branches exécutées, afin qu'une ligne de test ne se fasse pas passer pour une couverture.
2. En tant que mainteneur, je veux qu'un cas jamais plié soit exclu par un `assert` à message, afin qu'un test futur qui l'atteint rougisse au lieu de plier en silence des octets jamais vérifiés.
3. En tant que mainteneur, je veux que le message dise quels octets les empreintes figées pliaient pour ce cas, afin de pouvoir rouvrir la branche sans relire l'historique.
4. En tant que gardien des empreintes, je veux qu'aucune constante `*_HASH` ni `FROZEN_*` encore lue ne change, afin que D.21b soit un nettoyage et pas une re-baseline.
5. En tant que gardien des empreintes, je veux une preuve `gcov` au parent, ligne à ligne, que chaque ligne retirée est à `#####`, afin qu'aucun octet plié ne disparaisse.
6. En tant que gardien des empreintes, je veux que chaque `assert` nouveau soit exécuté au moins une fois (`gcov` après), afin qu'il protège vraiment quelque chose.
7. En tant que gardien des tests, je veux que chaque `assert` nié rougisse son TU, afin de prouver qu'il est sur un chemin vivant et que sa condition est bien celle du chemin.
8. En tant que gardien des tests, je veux que Md1, Md4, Me1 et Me2 rejouées au parent restent vertes, afin de confirmer que ces branches n'ont jamais plié un octet.
9. En tant que gardien des tests, je veux qu'une branche vivante reste dans son TU et soit déclarée, afin de ne jamais retirer un chemin exercé pour tenir un compte.
10. En tant que lecteur, je veux que `FROZEN_STRUCT_TAG` ne reste que dans `fr`, `hy`, `js` et `nr`, afin que sa présence signifie « ce TU plie ce mot ».
11. En tant que lecteur, je veux que `fold_entity` disparaisse de `pj`, afin qu'aucune aide n'existe sans appelant vivant.
12. En tant que lecteur, je veux que les commentaires qui décrivaient les branches retirées partent avec elles (`ga:133`, `lq:105`, `pj:221`, les commentaires de `#define`), afin qu'aucun commentaire ne décrive du code absent.
13. En tant que relecteur, je veux que les orphelins `-Werror` (fonctions statiques sans appelant) et les `#define` sans lecteur soient retirés et nommés au commit, afin que le diff ne laisse rien derrière lui.
14. En tant que relecteur, je veux qu'un include parte avec son dernier usage, et seulement alors, afin que les includes restent justes.
15. En tant que relecteur, je veux des greps de Sortie aux exclusions nommées, afin de vérifier le diff sans interprétation.
16. En tant que relecteur, je veux qu'aucune jonction de lignes, aucun alias, aucun renommage ni aucun cast ne serve à faire baisser un compte, afin que les comptes mesurent un vrai retrait.
17. En tant que gardien de la mémoire, je veux un balayage ASan+UBSan des TU touchés, afin qu'un `assert` ajouté ne lise rien hors d'un objet.
18. En tant que pilote du plan, je veux un budget d'environ 50 minutes par sous-étape et un repli écrit, afin qu'un site difficile ne bloque pas les autres.
19. En tant que pilote du plan, je veux une note « Découvert à D.21b1/b2 » avec les comptes réels, afin que D.21c parte d'un état mesuré.

## Implementation Decisions

Pas d'humain dans la boucle. Les choix ci-dessous sont tranchés comme le ferait un agent `guideline` et marqués **(G)**. La règle commune de D.21 fait foi : aucune empreinte ne bouge ; un changement d'empreinte est `FAILED:`.

### Modules

- **D.21b1** : les TU `pj`, `ga`, `lq`. Rien d'autre sous `tests/`.
- **D.21b2** : les TU `g_`, `ga`, `hj`, `mx`, `nl`, `os`, `qh`. `ga` est dans les deux sous-étapes, sur des lignes disjointes (`fold_untagged` et ses appelants, puis `fold_ref` et `fold_ref_member`).
- **`docs/plan-3.md`** : la note « Découvert à D.21b1 » (resp. b2) en fin de ligne D.21b1 (resp. b2), rien d'autre.
- Aucune ligne de `src/`, du `Makefile`, de `tools/`, de `tests/e2e/`. Aucun `*.sha256`.

### Formes (G)

- `assert(<condition> && "<message>");`, jamais `assert(0 && …)` ni `abort()` : la condition est celle de la branche vivante, c'est ce qui la rend exécutable et niable.
- Le message est en anglais comme le code, une phrase, sans renvoi au plan ni à une étape (règle des commentaires de `CLAUDE.md`, « un piège qu'on réintroduirait : un `assert` porteur de message »). Il dit le cas exclu et les octets figés à plier si on le rouvre.
- Le corps de la branche gardée est désindenté, rien d'autre : pas de variable nouvelle, pas de nouvelle aide.
- **Rejeté** : garder `FROZEN_STRUCT_TAG` « pour plus tard » dans les sept TU (une constante sans lecteur est ce que la revue reproche) ; déplacer le test `is_bv_target` avant `listed` (changerait l'ordre des conditions : si une cible jetable entrait un jour dans `hv`, les octets changeraient) ; un en-tête de test partagé pour le message (le précédent est par TU).

### Anti-triche (liant)

- Pas de jonction de lignes, pas d'alias (`#define` d'un nom retiré vers un autre), pas de renommage d'une aide pour esquiver un compte, **pas de cast nouveau**.
- Aucune ligne `check(…)` ni `*_HASH` ne change. Aucune fonction `test_*` n'est modifiée.

### Orphelins (liant)

L'implémenteur compile chaque TU touché avec les flags du `Makefile` (`-Werror`) : toute fonction statique sans appelant est retirée dans le même commit et nommée au message. Un `#define` sans lecteur est retiré avec son commentaire de piège. Un include est retiré **si et seulement si** plus aucun symbole de son en-tête n'est lu dans le TU (vérifié par `grep` des symboles, puis par la compilation) ; chaque include retiré est nommé au message. Prévus : `fold_entity` et `FROZEN_STRUCT_TAG` (`pj`) ; `fold_untagged` (`ga`, si règle 3) ; `fold_ref` (`ga`, `lq` si règle 3) ; `is_bv_target` et `FROZEN_INSTANCE_TAG` (`hj`, `os`) ; `FROZEN_STRUCT_TAG` (sept TU de b2) ; un include de `pico8_struct.h` dans `hj`/`os` seulement s'il n'a plus d'usage.

### Message de commit (liant)

Chaque commit porte : le tableau site → ligne `gcov` au parent (compte de chaque branche) → forme appliquée ; les lignes gardées vivantes (règle 4) ; les remontées (règle 3) ; les orphelins et includes retirés ; les comptes avant/après (tableau « Comptes de Sortie ») ; les mutations et leur issue ; le balayage et les filets ; le tableau « Runs » (ou son renvoi au journal). Message en français. Trailers de session du dépôt.

### Budget et repli (G)

- **≈ 50 min par sous-étape jusqu'au commit.** Rythme : `gcov` au parent ≈ 10, éditions ≈ 10, `gcov` après ≈ 5, mutations (asserts niés + rejeux au parent) ≈ 10, filets et ASan+UBSan ≈ 10, mesures et message ≈ 5.
- **Repli D.21b1** : les sites sont faits dans l'ordre `pj`, `lq`, `ga`. Au plafond, on committe les sites finis et prouvés ; le reste est déclaré dans la note avec ses lignes et devient D.21b1′ avant D.21b2.
- **Repli D.21b2** : ordre `mx`, `nl`, `hj`, `os`, `qh`, `g_`, `ga`. Au plafond, on committe les TU finis et prouvés ; le reste est déclaré dans la note, `FROZEN_STRUCT_TAG` reste dans ces TU, et les comptes sont recalculés (b2′).
- **Repli de la règle 3** (remontée qui entrerait dans une fonction `test_*` ou dans une fonction qui appelle `check`) : le site reste tel quel, déclaré ; ce n'est pas `FAILED:`. Risque identifié : `mx:504` appelle `fold_ref(h, NULL)` dans un ternaire `pico8_d_(placed) ? … : fold_ref(h, NULL)`, dans un corps de test ; si ce bras NULL est lui-même à `#####`, `fold_ref` de `mx` n'est jamais entrée et `mx` garde son `FROZEN_STRUCT_TAG` (4 → 5 TU, 12 → 14 occ.). De même pour `hj`, `nl`, `os`, `qh` si ni `fold_et` ni le ternaire n'entrent dans `fold_ref`.

## Testing Decisions

Un bon test ici ne vérifie que ce que le TU plie et ce que ses `assert` refusent ; aucun test nouveau n'est écrit : les `assert` ajoutés **sont** la détection gagnée (une entité hors de `hv`, un `ed`, un item pointeur feraient rougir). Prior art : la règle du parent et les mutations Md/Me de D.20a ; `gcov` avec `-D_exit=exit` de D.20a/D.20b2 ; `FROZEN_LIST_TAG` gardé dans les seuls TU qui le plient (D.12d).

### Preuve 1 — `gcov` au parent, avant toute édition

Dans une copie jetable du parent (scratchpad), les TU de la sous-étape sont compilés avec les flags du `Makefile` plus `--coverage` (`GCOV_CFLAGS`) et **`-D_exit=exit`** (ils forkent tous par `in_fresh_child` ; sans cela les compteurs des enfants sont perdus), lancés comme le `Makefile` depuis la racine, puis `gcov` sur le `.c` du TU. Pour chaque site, on relève le compte de chaque ligne de la fonction (test NULL, `listed`, `is_bv_target`, repli), et des sites d'appel si la règle 3 s'applique. **Chaque ligne retirée ou remplacée doit être à `#####`.** Une ligne à compte non nul reste (règle 4) et est déclarée.

### Preuve 2 — `gcov` après

Même recette sur l'arbre édité : chaque ligne `assert(` ajoutée a un compte ≥ 1.

### Preuve 3 — `assert` nié

Pour chaque `assert` ajouté, une mutation à la fois : `assert(<cond> && …)` → `assert(!(<cond>) && …)`, dans une copie jetable ou par `cp` / `cp` + `touch` (jamais `git checkout` ni `stash`). Le TU est lancé comme le `Makefile` : **il doit rougir** (SIGABRT). Un `assert` nié qui reste vert est `FAILED:` (il n'est pas sur un chemin vivant).

### Preuve 4 — rejeu au parent

Au parent de chaque commit : **Md1** (`FROZEN_STRUCT_TAG` → `0x9E5C57C8u`) dans `g_`, `ga`, `hj`, `mx`, `nl`, `os`, `qh` ; **Md4** (`0xEEEEu` → `0xEEEFu` à `ga:138`) ; **Me1** (`FROZEN_STRUCT_TAG` → `0x9E5C57C8u` dans `pj`) ; **Me2** (`0u` → `1u` à `lq:111`). Toutes **vertes**. Une mutation rouge au parent : la branche plie des octets, elle reste (règle 4), déclarée ; si elle a quand même été retirée, `FAILED:`.

### Filets

- `make test` vert ; `make e2e` **26 conformes**. Aucun e2e ajouté ni retiré.
- **ASan+UBSan des TU touchés** : copie jetable, flags du `Makefile` plus `-fsanitize=address,undefined -fno-sanitize-recover=undefined -fno-omit-frame-pointer`, lancés comme le `Makefile` avec `ASAN_OPTIONS=detect_leaks=0`. Propre.
- `make raylib` non exigé ; **jamais l'appli raylib**.

### Greps de Sortie (exclusions nommées ; `<parent>` = parent du commit)

1. `grep -rn 'fold_entity' tests/` → vide. Exclusions : aucune.
2. `grep -c FROZEN_STRUCT_TAG tests/*.c | grep -v ':0$'` → après b1 : les 11 TU sauf `pj` ; après b2 : exactement `fr:3`, `hy:2`, `js:4`, `nr:3`. `grep -rn FROZEN_STRUCT_TAG src tests | wc -l` → 26 puis **12**. Exclusions : aucune (sauf sites de repli déclarés, nommés au commit).
3. Constantes : `git diff <parent> -- tests | grep -E '^\+' | grep -E '_HASH|identity\[|FROZEN_(LIST|INSTANCE|STRUCT)_TAG|VIEW_[A-Z_]+ =|TABLE_INDEX_KEY =|_KIND = '` → vide, **sauf** la ligne désindentée `+    return fnv(fnv(h, 0xEEEEu), FROZEN_INSTANCE_TAG);` dans `g_` et `qh` (forme `is_bv_target`), chacune appariée à la ligne `-        return fnv(fnv(h, 0xEEEEu), FROZEN_INSTANCE_TAG);` du même fichier. Aucune ligne `-` de cette grep ne porte `_HASH`.
4. Includes : `git diff <parent> -- tests | grep -E '^\+\s*#include'` → vide ; `grep -E '^-\s*#include'` → seulement les includes déclarés au commit.
5. Casts : `git diff <parent> -- tests | grep -E '^\+' | grep -E '\(\s*(const\s+)?void\s*\*\s*\)|\(\s*(const\s+)?[a-z0-9_]+_t\s*\*\s*\)'` → vide. Exclusions : aucune.
6. `assert` à message : `git diff <parent> -- tests | grep -E '^\+.*assert\(' | grep -v '&& "'` → vide. Exclusions : aucune.
7. Périmètre : `git diff --stat <parent> -- src Makefile tools tests/e2e` → vide ; `git diff --name-only <parent> | grep -E '\.sha256$'` → vide ; `git show --stat HEAD` → les TU de la sous-étape et `docs/plan-3.md`.
8. Tests intouchés : `git diff <parent> -- tests | grep -E '^[-+].*\bcheck\('` → vide.

### Budget et tableau « Runs »

| Cible | Durée attendue |
|---|---|
| `gcov` au parent (3 ou 7 TU) | ≈ 5-10 min |
| `make tmp/test_pico8_<t>` + lancement, par TU | quelques secondes |
| `make test` | de l'ordre de la minute |
| `make e2e` | ~1 s |
| asserts niés + rejeux au parent (b1 ≈ 3 + 4 ; b2 ≈ 7 + 7) | < 10 min |
| ASan+UBSan des TU touchés | 2-5 min |

L'implémenteur tient un tableau « Runs » par sous-étape, une ligne par lancement :

| # | Commande exacte | État de l'arbre | Code de sortie | Ligne clé | Durée |
|---|---|---|---|---|---|
| 1 | `grep -c FROZEN_STRUCT_TAG tests/*.c \| grep -v ':0$'` | parent | 0 | 12 TU, 28 occ. | … |
| 2 | `gcov` au parent, TU `pj ga lq` | copie jetable | 0 | comptes des lignes P1, G1, L1 | … |
| … | … | … | … | … | … |

### Critères d'échec (`FAILED:` pour le vérificateur de `prd-autopilot`)

- `FAILED:` si une constante `*_HASH`, `identity[]`, `FROZEN_LIST_TAG`, `FROZEN_INSTANCE_TAG` encore lue, `VIEW_*`, `TABLE_INDEX_KEY` ou `*_KIND` change, ou si un TU demande une re-baseline.
- `FAILED:` si une ligne retirée ou remplacée n'était pas à `#####` au parent ; si un `assert` ajouté a un compte nul après ; si un `assert` nié laisse son TU vert ; si Md1, Md4, Me1 ou Me2 rejouée au parent rougit sans que la branche soit restée.
- `FAILED:` si une ligne de `src/`, du `Makefile`, de `tools/` ou de `tests/e2e/` change, si un `*.sha256` change, ou si une fonction `test_*` ou une ligne `check(` change.
- `FAILED:` si une grep de Sortie ne rend pas ce qui y est écrit, exclusions nommées comprises.
- `FAILED:` pour une jonction de lignes, un alias, un renommage d'esquive ou un cast nouveau.
- `FAILED:` si un orphelin reste, si un include est retiré alors qu'un symbole de son en-tête est encore lu, ou si un orphelin ou un include retiré n'est pas nommé au commit.
- `FAILED:` si `make test` ou `make e2e` (26 conformes) reste rouge après 3 tentatives, ou si ASan+UBSan n'est pas propre.
- `FAILED:` s'il manque la note « Découvert à D.21b1/b2 », le tableau « Runs », ou le tableau `gcov` au message.
- `FAILED:` si un commentaire ajouté ou reformulé sort des quatre sortes de `CLAUDE.md`, ou renvoie au plan.
- Pas `FAILED:` : une branche gardée vivante (règle 4) ou un site en repli (règle 3), s'ils sont déclarés avec leur compte `gcov` et que les comptes sont recalculés.

## Out of Scope

- Les TU `fr`, `nr`, `hy`, `js` (y compris leurs replis hors `hv`, non examinés), le `fold_untagged` de `g_`, le `0u` de `pj:201`, `hp:135` : ils plient des octets.
- Retirer ou renommer les autres noms de la couche retirée dans les TU (`fold_untagged` de `g_`, « tag » dans les commentaires de `FROZEN_LIST_TAG`/`FROZEN_INSTANCE_TAG`) : la phase 3 ou une passe `reprendre-les-commentaires`.
- Les branches mortes d'autres aides que celles nommées ici (par exemple un test NULL mort qui précède une branche vivante : il reste, règle 2).
- Le type des `void *` publics (B3 de la revue) : phase 2.
- D.21a, D.21c-f.

## Further Notes

- **Questions ouvertes (défauts pris)** :
  1. *Aide jamais entrée.* Dans `ga`, `fold_untagged` et `fold_ref` ne sont probablement jamais appelées : un `assert` dedans ne s'exécuterait jamais. Défaut : l'`assert` monte dans l'aide qui les appelle (`fold_tuple`, le bloc `lq->ra`, `fold_ref_member`), et l'aide morte part. C'est plus large que la ligne du plan (« `assert(p == NULL)` dans `fold_untagged` »), mais c'est la seule forme prouvable.
  2. *Branche `is_bv_target` de `hj` et `os`.* Elle est aussi hors de `hv` et aussi morte (sonde D.20a1). Défaut : elle part avec le repli, et `FROZEN_INSTANCE_TAG` quitte ces deux TU. Si `gcov` la montre vivante, elle reste et porte l'`assert` (forme `g_`).
  3. *`mx`, `fold_ref(h, NULL)` dans un corps de test.* Si ce bras n'est jamais pris, on ne remonte pas dans le test : `mx` garde son repli, et la cible devient 5 TU / 14 occ., déclaré.
- Le point M2 de la revue propose `assert(listed(pico8_hv(), e))` partout ; ce PRD le garde là où `listed` est la dernière branche vivante, et utilise la condition de la vraie dernière branche vivante ailleurs (`is_bv_target` dans `g_`/`qh`, `ref == NULL` dans `mx`), pour que l'`assert` soit exécuté et que l'ordre des conditions ne change pas.
- Numéros de ligne : ceux de `d6db288`. D.21b1 ne décale que `pj`, `ga`, `lq` ; les numéros de `ga` pour D.21b2 sont remesurés par contenu sur le parent de D.21b2.

## Annexe — reproduire (lecture seule, sans build)

```sh
grep -c FROZEN_STRUCT_TAG tests/*.c | grep -v ':0$'            # 12 TU
grep -rn FROZEN_STRUCT_TAG src tests | wc -l                   # 28
grep -rn fold_entity tests/                                    # pj:222, :233
for t in g_ ga hj mx nl os qh pj lq; do
  grep -n "fold_ref(\|fold_ref_member(\|fold_untagged(\|fold_entity(\|fold_ed(" tests/test_pico8_$t.c | grep -v '^[0-9]*:static'
done                                                           # sites d'appel
sed -n 600,601p tests/test_pico8_ga.c                          # jo.lq plié puis asserté NULL
sed -n 504p tests/test_pico8_mx.c                              # seul appel : fold_ref(h, NULL)
grep -n NDEBUG Makefile                                        # vide
```

## Amendement D.21b2 — `fold_entity` de `test_pico8_ui.c`

Ajouté au périmètre de D.21b2 par le pilote : `grep -rn fold_entity tests/` rendait encore `test_pico8_ui.c:146` (définition), `:359` et `:418` (appels, dans le corps d'un test qui appelle `check("hover", …)`), présents dès `d6db288` et absents du compte « 2 » de ce PRD. Règle : mesurée au parent par `gcov` (`-D_exit=exit`) ; si ses branches sont mortes comme dans `pj`, même traitement (`assert` sur la condition exécutée, lignes `#####` retirées) ; sinon elle reste, déclarée. Grep de Sortie 1 amendée : `grep -rn 'fold_entity' tests/` → vide, exclusion : `test_pico8_ui.c` (« vivante, sans branche »).

Mesure (parent 66b6357) : `fold_entity` n'a **aucune branche** (deux lignes droites : présence `ref != NULL`, puis rang dans `hv`) ; `:147` et `:148` exécutées 2 fois chacune, appelée 2 fois (`:359` 1, `:418` 1). Mutations au parent : `:147` `ref != NULL` → `ref == NULL` et `:148` rang `+ 1` → **rouges** toutes deux (`check` à `:170`, rc 134). Elle plie des octets : **elle reste**, non touchée ; grep 1 → les 3 lignes de `test_pico8_ui.c`, seule exclusion.
