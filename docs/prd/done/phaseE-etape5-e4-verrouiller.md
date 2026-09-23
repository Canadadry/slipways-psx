---
title: "Plan 2, Phase E4 : verrouiller 6.1 et 6.3 dans make test, 6.4 par cible explicite, clore le volet flottant"
description: "Après E3, les quatre points du §6 passent, mais 6.1 et 6.3 ne sont vérifiés que par des cibles hors suite : rien n'empêche un mot « double » ou un retour flottant de revenir sans que make test ne rougisse, et le §6 du plan décrit encore un critère que les décisions humaines ont modifié."
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
> - **E4 — Verrouiller** : les cibles 6.1, 6.3 (liste vide) et 6.4 entrent dans `make test` ; 6.2 est verrouillé par le lien de `pico8_replay` que `make test` construit déjà.
>
> **Sortie de phase :** les quatre points du §6 passent.

**Où en est-on (attendu après E3, à remesurer en début d'étape).**

| § | Cible | Code | Résultat | Dans `make test` ? |
|---|---|---|---|---|
| 6.1 | `make float-grep` | 0 | `float-grep coeur 119 exclus 30 lignes 0` | non |
| 6.2 | lien de `tmp/pico8_replay` sans `-lm` (et `make float-link`) | 0 | `float-link objets 62 non-resolus 0` | oui (le lien, via `replay`) ; `float-link` non |
| 6.3 | `make float-guard` | 0 | `garde fichiers 58 en-echec 0` | non |
| 6.4 | `make frames-ledger` | 0 | `frames-ledger commits 6 poses 4 divergences 2 erreurs 0` | non |

`make test` = `unit replay screencast-check frames-check float-inventory-check constants-check float-audit`. Les outils des trois cibles sont déjà testés sur des entrées fabriquées dans `make unit` (`test_float_grep`, `test_float_guard`, `test_float_link`, `test_frames_ledger`), mais les cibles elles-mêmes, sur l'arbre réel, ne tournent que si on y pense.

Coût mesuré en préparant ce PRD (GCC 13.3, x86_64, arbres d'objets à jour) : `float-grep` 0,7 s, `float-guard` 2,2 s (recompile les 58 fichiers du cœur, sans cache), `frames-ledger` 0,3 s, `float-link` 0,15 s ; `make test` ≈ 38 s (« État de E1 »). Ajout retenu (`float-grep` + `float-guard`) : ≈ 2,9 s, moins de 10 %.

Autres constats de cadrage :

- **`frames-ledger` exige que la base `30dfd6c` soit joignable.** Elle est à 91 commits de la tête ; le clone de mesure est superficiel mais l'atteint. Un clone plus court (une profondeur de 50, courante pour les sessions distantes) ne l'atteint pas : le script rend 2 (« base injoignable ») — volontairement, un historique tronqué ne doit pas passer pour conforme. Dans `make test`, cela ferait échouer la suite dans chaque session neuve, avec un message qui ne dit pas le remède — d'où la décision ci-dessous de la garder hors suite.
- **`tools/float_guard.txt` porte un en-tête devenu faux**, écrit par `tools/float_guard.sh` : « Liste vide attendue a la fin de la Phase E seulement » et « Angle mort sur x86_64 : un argument double passe a une fonction externe (pico8_fixed_from_double(<litteral>)) compile sans erreur ». Depuis E1, cet appel est une erreur de compilation dans le cœur (mesuré, « État de E1 »). Le commentaire du `Makefile` au-dessus de `float-guard` dit aussi « ce qui est l'état attendu jusqu'à la Phase E ».
- **`float-guard` réécrit un fichier versionné** : dans `make test`, sur un arbre vert, la réécriture est identique à l'octet ; sur un arbre rouge, le diff du fichier montre le fichier fautif, ce qui est le comportement voulu depuis B2.
- **Le §6 du plan ne dit plus vrai.** §6.1 annonce `grep -rnE … src/` « hors les helpers de test », alors que la décision humaine le restreint au cœur avec une liste d'exclusion ; §6.2 ne nomme pas la cible ; §6.3 affirme que la garde « est le seul critère qui prouve réellement l'objectif PSX », alors que la décision humaine l'a ramenée à une garde hôte (vérification `mipsel`/`nm` hors périmètre) et qu'E1 a fermé son angle mort par un autre moyen ; §6.4 parle des « populations (a) et (b) » et des « 5 sites réécrits », alors que la décision le lit « toute divergence justifiée par une ligne du relevé » (C2b, qui a divergé, portait sur des constantes). §11, décision 9, dit encore que le critère de fin inclut la vérification `nm`.

## Solution

1. **`make test` gagne `float-grep` et `float-guard`**, après les cibles actuelles. La première ligne de la légende du `Makefile` et les lignes de légende de ces deux cibles passent de « hors suite » à « dans make test ».
   **`frames-ledger` reste une cible explicite, hors suite** (décision de l'orchestrateur, qui s'écarte de la puce E4 du plan) : le §6.4 est une propriété de l'historique, pas de l'arbre, et une suite rouge par environnement (clone de profondeur 50, base à 91 commits) finit ignorée. Elle est lancée une fois à la clôture d'E4, sur un clone qui atteint la base, et son résultat est consigné dans l'« État de E4 » et la « Sortie de phase E ». Sa ligne de légende dit pourquoi elle est hors suite.
2. **`float-link` n'entre pas dans `make test`** (décision ci-dessous) ; sa ligne de légende dit qu'il reste hors suite parce que le lien de `tmp/pico8_replay`, construit par `make test`, est le même lien.
3. **En-têtes et commentaires devenus faux, corrigés au minimum** : l'en-tête que `tools/float_guard.sh` écrit dans `tools/float_guard.txt` dit que la liste doit être vide (et non « à la fin de la Phase E »), et que l'angle mort x86_64 (argument `double` passé à une fonction externe) est tenu par l'absence de prototype flottant dans le cœur (erreur de déclaration implicite) — sans citer d'étape. `tools/float_guard.txt` est régénéré par `make float-guard`. Le commentaire du `Makefile` au-dessus de `float-guard` ne garde que l'aide à la lecture de la commande (le rapport est réécrit avant que le code de sortie du script soit rendu).
4. **Le message d'erreur « base injoignable » de `tools/frames_ledger.sh` nomme le remède** (approfondir le clone : `git fetch --unshallow`, ou `--deepen` jusqu'à atteindre la base). Code de retour inchangé (2).
5. **`docs/plan-2.md`** :
   - **§6 réécrit pour dire vrai**, point par point, en gardant sa forme (quatre points objectifs, sans jugement) : 6.1 = `make float-grep`, grep `\b(double|float)\b` sur le cœur défini par la décision humaine, reste de `src/` exclu par `tools/float_grep_exclusions.txt` (une justification par ligne ; l'aide de test `pico8_fixed_test_double` y figure) ; 6.2 = la cible de lien du cœur est `$(TMP_DIR)/pico8_replay` (`REPLAY_SRCS`), liée sans `-lm` par `make test` ; 6.3 = garde **hôte** `-mgeneral-regs-only` sur `CORE_SRCS` (`make float-guard`), son angle mort x86_64 fermé par l'absence de prototype flottant dans le cœur (E1), la vérification `mipsel`/`nm` hors périmètre par décision humaine — la phrase « seul critère qui prouve réellement l'objectif PSX » est retirée ou requalifiée en conséquence, le paragraphe « Pourquoi pas `nm` » réduit à ce qui reste vrai ; 6.4 = `make frames-ledger`, toute divergence de `*.frames.hash` depuis la base est justifiée par une ligne du relevé et a son rapport ; propriété de l'historique, vérifiée par cette cible explicite sur un clone qui atteint la base, pas par `make test`. La phrase finale (« il sépare mécaniquement “j'ai transformé” de “j'ai changé” ») reste. Le §6 dit que 6.1, 6.2 et 6.3 sont tenus par `make test`, et 6.4 par `make frames-ledger`.
   - **§11, décision 9** : annotée (non réécrite), « levée par décision humaine en Phase D : vérification `mipsel`/`nm` hors périmètre, §6.3 se juge sur la garde hôte ».
   - **« État de E4 »** : la table des quatre cibles avec code et ligne, la nouvelle composition de `make test`, sa durée avant/après, les choix sur `float-link` et `frames-ledger` (hors suite, et pourquoi), le résultat de `make frames-ledger` lancé à la clôture, le comportement sur clone superficiel et son remède.
   - **« Sortie de phase E »** : les quatre points du §6 et leur **résultat final** mesuré, chacun avec ce qui le verrouille (`make test` pour 6.1–6.3, `make frames-ledger` lancé à la clôture pour 6.4) ; les constats laissés à l'humain (`make constant-variants` en échec depuis C2b, `make float-coverage` qui rend 2 sur inventaire vide, reliquat `tostr` < 0.0001 de la Phase A).
   - **Phase E marquée « ✅ FAITE »**, puce E4 « ✅ faite », titre du §7 et en-tête du document indiquant que **le volet flottant est terminé** ; le volet `malloc` (§8) reste « cadrage NON FAIT ».

Résultat attendu, à mesurer et consigner : `make test` vert, et il échoue si l'on réintroduit (sur une copie jetable ou un changement annulé ensuite) un mot `double` dans un en-tête du cœur, un retour `double` dans un fichier du cœur, ou une ligne de registre incohérente — une preuve par cible, consignée, pas versionnée.

## User Stories

1. En tant que mainteneur du cœur, je veux que `make test` échoue si un mot `double` ou `float` réapparaît dans un fichier du cœur, en-têtes compris, afin que le 6.1 ne se relâche pas entre deux fermetures de chantier.
2. En tant que mainteneur du cœur, je veux que `make test` échoue si un fichier du cœur a de nouveau besoin d'un registre flottant, afin que le 6.3 soit tenu en continu.
3. En tant que mainteneur du filet, je veux que `make frames-ledger` reste une cible explicite, lancée et consignée à la clôture d'E4, afin que le 6.4 soit vérifié sur l'historique sans rendre `make test` rouge dans chaque session à clone court.
4. En tant que mainteneur, je veux que le 6.2 reste tenu par le lien de `pico8_replay` que `make test` construit déjà, sans cible supplémentaire, afin de ne pas payer deux fois le même lien.
5. En tant qu'agent dans une session neuve (clone de profondeur 50), je veux que `make test` reste vert sans approfondir le clone, afin qu'une suite rouge par environnement ne soit pas prise pour du bruit et ignorée.
6. En tant qu'agent sur un clone superficiel, je veux que l'échec de `frames-ledger` me dise comment approfondir le clone, afin de ne pas prendre un historique tronqué pour un registre faux.
7. En tant qu'agent, je veux que le code 2 de `frames-ledger` sur base injoignable ne change pas, afin qu'un historique tronqué ne passe jamais pour conforme.
8. En tant que lecteur de `tools/float_guard.txt`, je veux que son en-tête dise ce qui est vrai aujourd'hui (liste vide exigée, angle mort tenu par l'absence de prototype flottant), afin qu'un rapport vert ne s'accompagne pas d'un avertissement périmé.
9. En tant que lecteur du `Makefile`, je veux que la légende dise quelles cibles sont dans `make test`, afin de savoir ce que « suite verte » garantit.
10. En tant que lecteur du `Makefile`, je veux que le commentaire de `float-guard` n'évoque plus « jusqu'à la Phase E », afin que la règle de CLAUDE.md (seule l'aide à la lecture d'une commande reste) soit respectée.
11. En tant que mainteneur, je veux que `float-link` reste une cible hors suite, testée par `test_float_link`, afin de garder l'outil de diagnostic (liste des symboles non résolus) sans le rejouer à chaque suite.
12. En tant que lecteur du plan, je veux que le §6.1 décrive le grep sur le cœur avec sa liste d'exclusion justifiée, afin que le critère écrit soit celui qu'on vérifie.
13. En tant que lecteur du plan, je veux que le §6.2 nomme `$(TMP_DIR)/pico8_replay` comme cible de lien du cœur, afin qu'il n'y ait pas à deviner laquelle.
14. En tant que lecteur du plan, je veux que le §6.3 dise qu'il s'agit d'une garde hôte, que l'angle mort x86_64 est fermé par E1 et que la vérification `mipsel` est hors périmètre par décision humaine, afin que le plan ne prétende pas prouver l'objectif PSX.
15. En tant que lecteur du plan, je veux que le §6.4 se lise « toute divergence justifiée par une ligne du relevé », afin que C2b (constantes, bloc A) et D1 (bloc B) soient admis par le texte et pas seulement par une décision ailleurs.
16. En tant que lecteur du plan, je veux que la décision 9 du §11 porte la mention de sa levée, afin que la liste des décisions ne contredise pas le critère de fin.
17. En tant que lecteur du plan, je veux un « État de E4 » avec les quatre cibles, leurs codes, la nouvelle composition et la durée de `make test`, afin que le verrouillage soit mesuré.
18. En tant que lecteur du plan, je veux une « Sortie de phase E » qui donne le résultat final de chacun des quatre points et la cible qui le verrouille, afin que « les quatre points passent » soit un fait consigné.
19. En tant que lecteur du plan, je veux que la Phase E et le volet flottant soient marqués terminés, et le volet `malloc` toujours non cadré, afin que l'état du chantier se lise d'un coup d'œil.
20. En tant qu'humain qui reprend, je veux trouver dans la sortie de phase les constats laissés ouverts (`constant-variants`, `float-coverage` sur inventaire vide, `tostr` < 0.0001), afin de décider s'ils méritent un chantier.
21. En tant qu'agent, je veux prouver une fois chaque verrou (mot `double` ajouté à un en-tête du cœur → `make test` rouge sur `float-grep` ; fonction du cœur rendant un `double` → rouge sur `float-guard` ; et, hors suite, ligne de registre falsifiée → `make frames-ledger` rend 1), sur des changements jetables annulés ensuite, afin que chaque verrou soit démontré et non supposé.
22. En tant qu'agent, je veux que `make frames-check` reste identique sans `frames-baseline`, afin de confirmer qu'E4 ne touche pas au comportement.

## Implementation Decisions

- **`float-link` hors de `make test`** — tranché par le skill `guideline` (simplicité, pas de vérification redondante), faute d'humain. Après E3, le lien de `tmp/pico8_replay` sans `-lm` est construit par `make test` (cible `replay`) sur exactement les mêmes objets que `float-link` ; `float-link` n'ajouterait qu'un second lien identique. En outre, toute fonction de `libm` prend ou rend un flottant, donc un appel depuis le cœur est déjà arrêté par la déclaration implicite (sans `math.h`) ou par `float-guard` (avec). `float-link` reste une cible de diagnostic, et `test_float_link` reste dans `make unit`. Ce choix suit la puce E4 du plan (« 6.2 est verrouillé par le lien de `pico8_replay` »).
- **Ordre dans `test`** : cibles actuelles, puis `float-grep`, `float-guard`. Les deux cibles restent appelables seules ; leurs règles ne changent pas.
- **`frames-ledger` hors de `make test`** (décision de l'orchestrateur) : §6.4 est une propriété de l'historique git, pas de l'arbre courant ; une porte qui rougit par environnement serait ignorée. La cible reste explicite, lancée à la clôture d'E4 et à chaque future re-baseline de `*.frames.hash`.
- **Coût accepté** : ≈ 2,9 s ajoutées (moins de 10 % de `make test`). `float-guard` n'est pas mis en cache : l'optimiser serait une fonctionnalité non demandée.
- **Clone superficiel** : pas de repli silencieux de `frames-ledger` (ce serait l'« historique tronqué qui passe pour conforme » que le script refuse par conception) ; code 2 inchangé. Seul le message change, pour nommer le remède ; le test d'outil (`test_frames_ledger`, cas de base inconnue) n'asserte que le code de retour et reste vert — à vérifier.
- **En-tête de `tools/float_guard.txt`** : modifié dans `tools/float_guard.sh` (seul auteur du fichier), puis fichier régénéré par la cible ; jamais édité à la main. `test_float_guard` n'asserte pas l'en-tête (vérifié en cadrage) — à confirmer par `make unit`.
- **§6 réécrit sur place**, pas en annexe : le §6 est la porte du chantier ; un lecteur qui ne lit que lui doit lire le critère réellement appliqué. Les « État de B2 … E3 » ne sont pas réécrits (historique). La mention « Décision humaine (Phase D) » déjà intégrée au §6.3 est conservée sous une forme cohérente avec le nouveau texte.
- **Aucune modification de `src/`** ; aucun `*.frames.hash` touché.
- **Hors du volet flottant, rien** : §8 (`malloc`) intouché, sauf pour rester la seule partie non faite du document.

## Implementation Chunks

1. **Verrouillage** : `Makefile` (`test`, légende, commentaire de `float-guard`), en-tête de `tools/float_guard.sh`, message de `tools/frames_ledger.sh`, `tools/float_guard.txt` régénéré. Vérification : `make test` vert ; `make frames-ledger` lancé une fois à la clôture (clone atteignant `30dfd6c`), code et ligne consignés ; `make unit` vert ; les trois preuves de verrou de la story 21, chacune annulée ; durée de `make test` consignée.
2. **Plan** : §6 réécrit, §11.9 annoté, « État de E4 », « Sortie de phase E », marquages « ✅ » (puce E4, Phase E, volet flottant, en-tête). Vérification : relecture du §6 contre les décisions humaines de la Phase E et contre les sorties mesurées du chunk 1 ; chaque chiffre de la sortie de phase vient d'une commande lancée dans cette étape.

2 dépend de 1 (il cite ses mesures).

## Testing Decisions

- **Un bon test ici est une porte qui rougit sur la régression qu'elle vise** : les trois preuves de verrou (story 21), faites sur des changements jetables, annulés, consignées dans l'« État de E4 » avec la cible qui a échoué et son code. Pas de test versionné nouveau : les outils sont déjà testés sur entrées fabriquées (`test_float_grep`, `test_float_guard`, `test_frames_ledger`, `test_float_link`), et c'est la suite elle-même qui est le verrou.
- **Tests existants à relancer** : `make unit` (dont `test_frames_ledger` après le changement de message, `test_float_guard` après le changement d'en-tête), `make test` en entier.
- **Neutralité** : `make frames-check` identique sans `frames-baseline`.
- **Prior art** : l'entrée de `float-inventory-check`, `constants-check` et `float-audit` dans `make test` (Phases B2, C, D0a), et les « État de E0/E1 » pour la forme des tables.

## Out of Scope

- Réécrire les outils (`float_grep.sh`, `float_link.sh`, `frames_ledger.sh` au-delà du message, `float_coverage.sh`, `constant_variants.sh`), ou corriger `make constant-variants` / `make float-coverage` sur inventaire vide : constats pour l'humain.
- Retirer `float-link` ou `test_float_link`.
- La vérification `mipsel`/soft-float, `pico8_fixed_rnd` (TROU-17), fermer TROU-1.
- Réécrire les « État » antérieurs, les PRD de `docs/prd/done/`, ou les §1–§5 et §9–§10 du plan (hors §11.9).
- Le volet `malloc` (§8). Lancer l'appli raylib : jamais.

## Further Notes

- Sur un clone de profondeur 50, `make frames-ledger` rend 2 tant que le clone n'est pas approfondi (`git fetch --unshallow` ou `--deepen`) ; `make test` n'en dépend pas. À consigner dans l'« État de E4 ».
- Si `make test` dépasse nettement les 2,9 s annoncés (par exemple parce que `float-guard` recompile avec un compilateur plus lent), le consigner ; ce n'est pas une porte.
