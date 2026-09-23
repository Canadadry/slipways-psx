---
title: "Plan 2, Phase E2 : mots de commentaires, TROU-1 restreint"
description: "Après E1, le cœur ne contient plus aucun flottant exécutable, mais §6.1 échoue encore sur 6 lignes de commentaires (« double-free », « double duty », « no host float »…), l'inventaire compte 4 jetons, un commentaire de pico8_ob.c cite un emplacement périmé de from_double, et TROU-1 décrit encore des écarts qui n'existent plus."
status: needs-triage
---

## Problem Statement

Texte de l'étape, recopié de `docs/plan-2.md` (§7, Phase E) :

> ### Phase E — Fermeture
> Seule porte de E : les quatre points du §6. Décisions humaines prises en fin de Phase D :
> - **§6.1 porte sur le cœur seul**, conformément au §4 : `CORE_SRCS` et les en-têtes qu'ils incluent par défaut (sans les définitions `PICO8_FLOAT_*`). Le reste de `src/` (backend raylib, visualiseur, builds d'audit) est exclu par une liste versionnée, une ligne de justification par fichier.
> - **§6.4 se lit « toute divergence justifiée par une ligne du relevé »** : C2b (bloc A) et D1 (bloc B) sont admis.
> - **Le build d'observation est retiré** (`make float-observe`, `src/pico8_float_observe.*`) : plus aucun cast à observer, l'audit avant/après le remplace. ✅ fait en E1.
> - **TROU-1 est restreint, pas fermé** : il ne décrit plus que `sin`/`cos` hors des 64 points du relevé.
>
> Étapes :
> - **E2 — Mots de commentaires** : les 4 de l'inventaire et les 3 d'en-têtes, plus la mention devenue périmée de `pico8_ob.c`. TROU-1 restreint.

**Où en est-on (« État de E1 », remesuré en préparant ce PRD).**

| § | Cible | Code | Résultat |
|---|---|---|---|
| 6.1 | `make float-grep` | 1 | `float-grep coeur 119 exclus 30 lignes 6` |
| 6.2 | `make float-link` | 0 | `float-link objets 62 non-resolus 0` |
| 6.3 | `make float-guard` | 0 | `garde fichiers 58 en-echec 0` |
| 6.4 | `make frames-ledger` | 0 | `frames-ledger commits 6 poses 4 divergences 2 erreurs 0` |

Les 6 lignes du 6.1, toutes des commentaires en anglais :

- `pico8_fixed.c` 117 — « one integer square root, **no host float**, and the result truncates » (doc de `pico8_fixed_sqrt`) ;
- `pico8_galaxy.c` 133 — « a repeated pico8_bw() neither leaks nor **double-frees** the pair » ;
- `pico8_ob.c` 347 — « drops an empty pending token (a stray **double comma**) » ;
- `pico8_geom.h` 9 — « a bare sign with 0 doing **double duty** » ;
- `pico8_research.h` 21 — « freeing both would **double-free** » ;
- `pico8_type_tag.h` 15 — « walks its backing array at **double stride** past the end ».

L'inventaire (`tools/float_inventory.txt`, qui ne lit que les `.c` du cœur) compte `autre 4`, `total 4` : `pico8_fixed.c` 117, `pico8_galaxy.c` 133, `pico8_ob.c` 347, et `pico8_ob.c` 201 — la mention `pico8_fixed_from_double` du commentaire de `pico8_ob_parse_number` (« rather than through pico8_fixed_from_double, which pico8_fixed.h reserves for tests »), invisible au grep (le `_` est un caractère de mot) et **périmée** depuis E1 : `pico8_fixed.h` ne déclare plus rien de tel, la fonction vit dans l'aide de test `pico8_fixed_test_double`, et un appel depuis le cœur est une erreur de compilation.

TROU-1 (`docs/trous-de-portage.md`), titré « L'arithmétique flottante n'est pas bit-exacte », décrit encore les littéraux décimaux et dit « `pico8_fixed_from_double` reste défini » — deux phrases fausses ou hors sujet depuis C2b et E1 — et renvoie à `docs/plan-2.md` comme programme d'élimination du flottant, programme qui s'achève.

Le problème, pour celui qui ferme le chantier : §6.1 ne peut pas passer tant que ces mots anglais ordinaires sont dans le cœur, l'inventaire ne peut pas valoir 0, et la documentation des écarts ment sur l'état du code.

## Solution

Chaque commentaire concerné est **reformulé sans les mots `double` ni `float`**, sens intact, ou **raccourci** quand la partie qui porte le mot n'est plus l'une des quatre choses qu'un commentaire a le droit de porter (CLAUDE.md : sens d'un nom minifié, renvoi TROU, piège, contrat C). Aucune ligne de code ne change.

Reformulations retenues (le texte exact reste à l'implémenteur, sous les contraintes de ce PRD) :

| Site | Avant | Après (proposé) | Pourquoi |
|---|---|---|---|
| `pico8_fixed.c` sqrt | « one integer square root, no host float, and the result truncates… » | « one integer square root, and the result truncates… » | « no host float » est désormais vrai de tout le cœur et imposé par la garde : redondant, supprimé. Le calcul et la troncature restent (contrat). |
| `pico8_galaxy.c` `pico8_bw` | « neither leaks nor double-frees the pair » | « neither leaks nor frees the pair twice » | piège ; sens identique |
| `pico8_ob.c` 347 | « (a stray double comma) » | « (two commas in a row) » | contrat ; sens identique |
| `pico8_ob.c` 201 | « Reads the decimal digits into 16.16 bits directly rather than through pico8_fixed_from_double, which pico8_fixed.h reserves for tests. Handles exactly… » | « Handles exactly what a spec-string number token can be: … No exponent, no hex. » | la première phrase est périmée, et le piège qu'elle signalait est désormais tenu par le compilateur ; la seconde est un contrat, gardée telle quelle |
| `pico8_geom.h` | « a bare sign with 0 doing double duty, loses one half of that » | « a bare sign, where 0 would have to mean both, loses one half of that » | contrat ; sens identique |
| `pico8_research.h` | « so freeing both would double-free » | « so freeing both would free them twice » | contrat/piège ; sens identique |
| `pico8_type_tag.h` | « walks its backing array at double stride past the end » | « walks its backing array at twice the stride, past the end » | piège ; sens identique |

**Piège de reformulation, mesuré en préparant ce PRD :** l'inventaire compte les **sous-chaînes** `double` et `float` d'une ligne et exige qu'elles soient toutes classées ; un mot comme « floating », « doubled » ou « doubles » n'est classé dans aucune catégorie (pas de frontière de mot après le jeton) et fait **échouer** `tools/float_inventory.sh` (« jeton hors catégorie »). Aucune reformulation ne doit contenir ces sous-chaînes, même dans un mot plus long.

**TROU-1** est restreint, pas fermé : titre et corps ne parlent plus que de `pico8_fixed_sin`/`_cos` hors des 64 points `i/64` du relevé. Le paragraphe sur les littéraux décimaux (y compris « `pico8_fixed_from_double` reste défini ») et la phrase « `docs/plan-2.md` a l'élimination du flottant à son programme » sortent. Restent : la table de quart d'onde, l'exactitude aux 64 points (assertée par `make test`), l'approximation non sourcée ailleurs, la conséquence sur le rejeu, le renvoi de `rnd` à TROU-17, les fichiers. Les marqueurs `/* TROU-1 */` de `pico8_fixed_sin`/`_cos` (`.c` et `.h`) restent, puisque l'écart est encore là.

Résultat attendu, à mesurer et consigner (pas à supposer) :

- **6.1** `make float-grep` → code **0**, `float-grep coeur 119 exclus 30 lignes 0`.
- **Inventaire** régénéré → `compte total 0` (toutes catégories à 0) ; `make float-inventory-check` vert.
- **6.2**, **6.3**, **6.4** inchangés.
- `make frames-check` identique sur les 12 fichiers de `FRAMES_EVENTS`, **sans** `frames-baseline`.
- `make test` vert.

## User Stories

1. En tant qu'agent qui ferme la Phase E, je veux que `make float-grep` rende 0, afin que le point 6.1 du critère de fin passe sur le cœur.
2. En tant qu'agent, je veux que l'inventaire régénéré compte 0 jeton au total, afin que la sortie de la Phase E s'énonce en « compte = 0 » comme la règle de C et D le demande.
3. En tant qu'agent, je veux que `make float-inventory-check` reste vert dans `make test`, afin que l'inventaire versionné et l'arbre ne divergent pas après le commit.
4. En tant que lecteur de `pico8_bw`, je veux toujours comprendre que libérer le contexte en même temps que l'état `gn` évite une fuite **et** une double libération, afin de ne pas réintroduire le bug en réordonnant ces lignes.
5. En tant que lecteur de `pico8_research_shutdown`, je veux toujours savoir que les entrées nommées de `gb` sont des alias des positionnelles et que les libérer aussi libérerait deux fois, afin de ne pas « compléter » la boucle.
6. En tant que lecteur de `pico8_type_tag.h`, je veux toujours savoir qu'une confusion de type fait lire le tableau avec un pas deux fois trop grand, afin de comprendre pourquoi l'étiquette existe.
7. En tant que lecteur de `pico8_geom.h`, je veux toujours comprendre pourquoi l'énumération a trois valeurs et pas un signe nu, afin de ne pas la « simplifier » en `int`.
8. En tant que lecteur du parseur `ob`, je veux toujours savoir que la garde `ctx->s < i` absorbe un jeton vide entre deux virgules, afin de ne pas la retirer.
9. En tant que lecteur de `pico8_ob_parse_number`, je veux que son commentaire ne décrive que ce qu'elle accepte (signe, chiffres, point, chiffres ; ni exposant, ni hexa), afin qu'il ne cite plus un emplacement de `from_double` qui n'existe plus.
10. En tant que lecteur de `pico8_fixed_sqrt`, je veux garder l'explication du calcul (racine entière de `x_raw * 65536`) et le fait que le résultat tronque, afin de connaître le contrat d'arrondi, sans la mention « no host float » que la garde rend redondante.
11. En tant qu'implémenteur, je veux qu'aucune reformulation ne contienne les sous-chaînes `double` ou `float` (« floating », « doubled »…), afin que `tools/float_inventory.sh` ne s'arrête pas sur un jeton hors catégorie.
12. En tant que mainteneur des commentaires, je veux que chaque commentaire touché reste l'une des quatre choses autorisées par CLAUDE.md, ou soit raccourci, afin qu'E2 ne réintroduise pas de récit de chantier.
13. En tant que mainteneur, je veux qu'aucune ligne de code (hors commentaire) ne change, afin que l'étape soit neutre par construction et que `frames-check` le confirme.
14. En tant qu'agent, je veux lancer `make frames-check` sans `frames-baseline` et constater les 12 fichiers identiques, afin de prouver la neutralité mesurée.
15. En tant que lecteur de `docs/trous-de-portage.md`, je veux que TROU-1 ne parle plus que de `sin`/`cos` hors des 64 points relevés, afin qu'il décrive un écart qui existe encore et rien d'autre.
16. En tant que lecteur de TROU-1, je veux que la phrase « `pico8_fixed_from_double` reste défini » disparaisse, afin qu'aucune documentation ne suggère que le cœur a encore une conversion flottante.
17. En tant que lecteur de TROU-1, je veux garder le renvoi de `rnd` à TROU-17 et la conséquence sur le rejeu contre le vrai PICO-8, afin que l'écart reste compréhensible sans aller lire le plan.
18. En tant que lecteur du code, je veux que les `/* TROU-1 */` de `pico8_fixed_sin`/`_cos` restent en place, afin que le renvoi pointe toujours vers le bon endroit.
19. En tant qu'agent, je veux relancer les quatre cibles du §6 et consigner leur ligne et leur code dans un « État de E2 » de `docs/plan-2.md`, et marquer la puce E2 « ✅ faite », afin qu'E3 parte d'un état mesuré.
20. En tant qu'agent, je veux consigner dans l'« État de E2 » ce que deviennent les rapports qui dépendent de l'inventaire (voir Further Notes sur `float-coverage`), afin qu'un rapport figé ne passe pas pour à jour.

## Implementation Decisions

- **Modules touchés** : les commentaires de six fichiers du cœur (`pico8_fixed.c`, `pico8_galaxy.c`, `pico8_ob.c` ×2, `pico8_geom.h`, `pico8_research.h`, `pico8_type_tag.h`), `docs/trous-de-portage.md` (TROU-1), `tools/float_inventory.txt` (régénéré par `make float-inventory`, jamais édité à la main), `docs/plan-2.md` (État de E2, puce E2).
- **Règle de choix entre reformuler et supprimer** (CLAUDE.md, arbitrée par le skill `guideline`, faute d'humain) : on garde tout ce qui est piège ou contrat ; on supprime seulement le fragment qui a cessé d'être l'un des quatre — « no host float » (redondant avec la garde), et la première phrase du commentaire de `pico8_ob_parse_number` (périmée, et piège désormais tenu par le compilateur). On ne réécrit pas le reste de ces commentaires : changement chirurgical.
- **Contrainte lexicale** : ni `double` ni `float` en sous-chaîne dans les lignes touchées (voir le piège ci-dessus). Les mots anglais de remplacement (« twice », « two commas in a row », « would have to mean both ») sont proposés, pas imposés.
- **Numéros de ligne** : les commentaires raccourcis décalent les lignes de `pico8_fixed.c` et `pico8_ob.c`. L'inventaire identifie ses jetons par fichier/fonction/catégorie/rang, et ne contiendra plus aucun jeton ; `tools/float_annotation.txt` n'a plus de cast et ne change pas. `tools/float_audit_c1.txt` ne cite pas de lignes de ces fichiers — à vérifier par `make float-audit` vert.
- **TROU-1** : titre recentré sur `sin`/`cos` (par exemple « `sin`/`cos` ne sont pas bit-exacts hors des 64 points relevés ») ; corps réduit à ce que la décision humaine dit ; « Fichiers : » inchangé (`src/pico8_fixed.c`, `src/pico8_quarter_wave.h`). Le numéro TROU-1 est conservé (les marqueurs du code y renvoient).
- **Aucun `*.frames.hash` ne change** : pas de rapport `tools/frames_reports/E2*.txt`, pas de ligne de registre.
- **Les exclusions du 6.1 ne changent pas** (30 lignes) ; le cœur reste de 119 fichiers.

## Testing Decisions

- **Un bon test ici est un effet observable sur l'arbre** : `make float-grep` code 0 avec `lignes 0` ; `make float-inventory` puis `compte total 0` ; `make float-inventory-check` vert ; `make frames-check` identique ; `make test` vert. Aucun test d'implémentation sur le texte des commentaires.
- **Aucun nouveau test automatisé.** Faire entrer `float-grep` dans `make test` est l'objet d'E4 ; `float-inventory-check`, déjà dans la suite, verrouille dès maintenant l'inventaire à 0 sur les `.c` du cœur.
- **Vérification de neutralité** : `git diff` (lecture, pas de commit par l'agent de ce PRD) ne montre que des lignes de commentaire dans `src/`.
- **Prior art** : E1 (étape neutre prouvée par `frames-check` identique, inventaire régénéré dans le même commit, quatre cibles consignées dans un « État »).

## Out of Scope

- **E3** (`-lm` retiré de `pico8_replay`) et **E4** (verrouillage dans `make test`, réécriture du §6 et sortie de phase).
- Tout autre commentaire du dépôt, même bavard : seuls les 7 sites listés sont touchés (pour un nettoyage général, le skill `reprendre-les-commentaires`).
- Les mots `double`/`float` hors du cœur (backend raylib, visualiseur, aide de test, builds d'audit, `tools/`, tests) : exclus du 6.1 par décision humaine.
- Réécrire `tools/float_coverage.sh` ou `tools/constant_variants.sh` pour qu'ils acceptent un inventaire vide.
- Fermer TROU-1 ou toucher `pico8_fixed_sin`/`_cos` ; `pico8_fixed_rnd` (TROU-17).
- Le volet `malloc` (§8). Lancer l'appli raylib : jamais.

## Further Notes

- **`make float-coverage` rendra 2 après E2, par conception** : `tools/float_coverage.sh` refuse une jointure vide (« aucun site ») pour qu'elle ne passe pas pour une couverture complète. La cible est hors suite ; `tools/float_coverage.txt` reste la photographie d'E1 (4 sites non couverts, tous des commentaires). Ne pas modifier l'outil ; consigner le fait dans l'« État de E2 », comme constat pour l'humain, à côté du constat déjà fait en E1 sur `make constant-variants`.
- Le grep du §6.1 et l'inventaire ne voient pas les mêmes lignes (le grep voit les en-têtes, l'inventaire voit les identifiants avec `_`) ; après E2 les deux valent zéro, ce qui ferme l'écart décrit dans l'« État de E0 ».
