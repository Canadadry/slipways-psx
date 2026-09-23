---
title: "Plan 2, Phase E0 : mesurer les quatre points du §6 sur l'arbre actuel"
description: "La Phase E n'a qu'une porte, les quatre points du §6, mais trois d'entre eux n'ont aucune cible make qui les mesure : on ne peut ni dire aujourd'hui ce qui les sépare de « passe », ni les verrouiller en E4."
status: needs-triage
---

## Problem Statement

Texte de l'étape, recopié de `docs/plan-2.md` (§7, Phase E) :

> ### Phase E — Fermeture
> Seule porte de E : les quatre points du §6. Décisions humaines prises en fin de Phase D :
> - **§6.1 porte sur le cœur seul**, conformément au §4 : `CORE_SRCS` et les en-têtes qu'ils incluent par défaut (sans les définitions `PICO8_FLOAT_*`). Le reste de `src/` (backend raylib, visualiseur, builds d'audit) est exclu par une liste versionnée, une ligne de justification par fichier.
> - **§6.4 se lit « toute divergence justifiée par une ligne du relevé »** : C2b (bloc A) et D1 (bloc B) sont admis.
> - **Le build d'observation est retiré** (`make float-observe`, `src/pico8_float_observe.*`) : plus aucun cast à observer, l'audit avant/après le remplace.
> - **TROU-1 est restreint, pas fermé** : il ne décrit plus que `sin`/`cos` hors des 64 points du relevé.
>
> Étapes :
> - **E0 — Mesurer.** Une cible `make` par point du §6, lancée sur l'arbre actuel **avant toute modification**, résultat consigné : 6.1 (grep sur l'ensemble défini ci-dessus, via `cc -MM`, plus la liste d'exclusion), 6.2 (`$(TMP_DIR)/pico8_replay` lié sans `-lm` — c'est « la cible de lien du cœur »), 6.3 (`float-guard`), 6.4 (registre : chaque commit depuis `30dfd6c` qui touche un `*.frames.hash` a son `tools/frames_reports/<étape>.txt` et sa ligne de relevé). L'écart entre « inventaire = 0 » et « §6 passe » y apparaît (prototypes et commentaire de `pico8_fixed.h`, mots de commentaires d'en-têtes).

Le §6 en entier :

> ## 6. Critère de fin — objectivement vérifiable
>
> Aucun de ces points ne demande de jugement ni de relecture :
>
> 1. `grep -rnE '\b(double|float)\b' src/` → vide, hors les helpers de test explicitement isolés et nommés comme tels.
> 2. La cible de lien du cœur perd `-lm`, et le build passe.
> 3. Les objets du cœur **compilent avec `-mgeneral-regs-only`** (erreur du compilateur sur tout flottant à l'exécution, constantes repliées tolérées). **C'est le seul critère qui prouve réellement l'objectif PSX** : les points 1 et 2 peuvent passer alors qu'un flottant subsiste dans une constante non repliée par le compilateur.
>    *Angle mort sur x86_64, mesuré en B2 :* la garde rattrape les retours et opérations `double`, pas un argument `double` passé à une fonction externe — GCC le passe sur la pile sans erreur. `pico8_fixed_from_double(<littéral>)` compile donc sous la garde : voir « État de B2 » au §7.
>    *Pourquoi pas `nm` :* sur l'hôte x86_64 le FPU est matériel, donc aucun symbole soft-float de `libgcc` (`__muldf3`, `__adddf3`…) n'est jamais émis — `nm` était **déjà vide avant tout changement** (mesuré : 0 symbole, alors que `pow`/`sin`/`cos` sont importés de la glibc). Un critère qui passe sur le code de départ ne prouve rien. *Décision humaine (Phase D) :* la vérification PSX (`nm` sur un build `mipsel` soft-float) est **hors périmètre** ; ce chantier reste sur du C pour PC, et §6.3 se juge sur la garde hôte seule, angle mort compris.
> 4. `frames.hash` **identique** avant/après pour les populations (a) et (b). Les seules divergences autorisées relèvent de la population (c) et des 5 sites réécrits, **chacune justifiée par une ligne du relevé de sonde**.
>
> Le point 4 est ce qui empêche ce chantier de devenir une réécriture à l'aveugle : il sépare mécaniquement « j'ai transformé » de « j'ai changé ».

**Où en est-on.** Après D5, l'inventaire (`tools/float_inventory.txt`) compte **10 jetons**, tous « autre » et tous affectés à E : les 6 des définitions `pico8_fixed_from_double`/`_to_double` de `pico8_fixed.c`, et 4 mots de commentaires (`float` dans `pico8_fixed.c` hors fonction, `double-free` dans `pico8_bw`, `pico8_fixed_from_double` dans `pico8_ob.c` hors fonction, `double comma` dans `pico8_ob_step`). La garde (`tools/float_guard.txt`) n'a plus qu'un fichier en échec, `pico8_fixed.c`.

Mais sur les quatre points du §6, **un seul a une cible** (`make float-guard`, 6.3). Les trois autres ne sont mesurés par rien :

- **6.1** — aucune cible ne définit « l'ensemble du cœur » pour le grep. L'inventaire ne le remplace pas : il ne lit que les `.c` du cœur, alors que le grep voit aussi les en-têtes (les deux prototypes de `pico8_fixed.h` et son commentaire « representable as a double », des mots de commentaire comme « double duty » ou « double-free » dans d'autres en-têtes) ; et à l'inverse l'inventaire compte des jetons d'identifiant (`pico8_fixed_to_double`, la mention `pico8_fixed_from_double` de `pico8_ob.c`) que `\b(double|float)\b` ne voit pas, `_` étant un caractère de mot. « Inventaire = 0 » et « §6.1 passe » sont donc deux prédicats différents, et personne ne sait aujourd'hui de combien ils s'écartent. Le reste de `src/` (30 fichiers : points d'entrée, backend raylib, visualiseur, `pico8_assets`, `pico8_dump`, builds d'audit et d'observation…) doit être exclu par une liste versionnée qui n'existe pas encore.
- **6.2** — `$(TMP_DIR)/pico8_replay` est lié avec `-lm`. Personne n'a vérifié qu'il se lierait sans ; si un symbole de `libm` restait appelé, E3 le découvrirait en cassant le build de `make test`.
- **6.4** — le critère est lu dans des « États » en prose. Rien ne relie mécaniquement l'historique git des `*.frames.hash` aux rapports de `tools/frames_reports/` et aux lignes du relevé. Un re-baseline sans rapport, ou un rapport qui ne couvre pas tous les fichiers réécrits, passerait inaperçu.

Sans ces mesures, E1–E3 avancent à l'aveugle vers une porte dont on ignore l'écart, et E4 n'a rien à verrouiller.

## Solution

E0 **ajoute des instruments de mesure, sans toucher `src/`**, puis consigne leur résultat d'aujourd'hui dans un « État de E0 » de `docs/plan-2.md`.

Une cible `make` par point du §6, chacune hors de `make test` (E4 les y fera entrer), chacune (sauf la 6.3, déjà là) portée par un script de `tools/` testé sur des entrées fabriquées dans `make unit`, sur le modèle de `float-guard` / `test_float_guard` :

| § | Cible | Ce qu'elle mesure | Échoue quand |
|---|---|---|---|
| 6.1 | `make float-grep` | `\b(double\|float)\b` sur l'ensemble du cœur : `CORE_SRCS` + les en-têtes qu'ils incluent par défaut (via `$(CC) -MM`, sans `PICO8_FLOAT_*`), le reste de `src/` exclu par une liste versionnée justifiée ligne à ligne | une ligne trouvée, ou la partition de `src/` incohérente |
| 6.2 | `make float-link` | les objets de `$(TMP_DIR)/pico8_replay`, liés **sans `-lm`** dans un binaire jetable ; la vraie ligne de lien reste intacte jusqu'à E3 | le lien échoue ; les symboles non résolus sont listés |
| 6.3 | `make float-guard` | inchangée | un fichier du cœur en échec sous `-mgeneral-regs-only` |
| 6.4 | `make frames-ledger` | un registre versionné confronté à l'historique git depuis `30dfd6c` : chaque commit qui touche un `*.frames.hash` y a sa ligne ; chaque référence réécrite est couverte par un rapport versionné dans le même commit, et justifiée par une ligne existante du relevé | un commit sans ligne, une ligne sans commit, un rapport manquant ou incohérent, une ligne de relevé introuvable |

Puis on lance les quatre sur l'arbre actuel et on consigne. Résultat attendu, mesuré à la rédaction de cette PRD (à re-mesurer, pas à recopier) :

- **6.1 échoue**, 13 lignes dans 8 fichiers : `pico8_fixed.c` 5 (les deux définitions sur 4 lignes, et le commentaire « no host float »), `pico8_fixed.h` 3 (deux prototypes, un commentaire), `pico8_galaxy.c` (« double-frees »), `pico8_ob.c` (« double comma »), et trois en-têtes : `pico8_geom.h` (« double duty »), `pico8_research.h` (« double-free »), `pico8_type_tag.h` (« double stride »). Ensemble du cœur : 119 fichiers (58 `.c`, 61 en-têtes, dont `pico8_quarter_wave.h`) ; exclusions : 30 fichiers.
- **6.2 passe** (lien d'essai sans `-lm` réussi à la rédaction ; `math.h` n'est plus inclus que par `pico8_float_audit.c`, hors de `pico8_replay`).
- **6.3 échoue** sur un fichier, `pico8_fixed.c`.
- **6.4 passe** si le registre est juste : 6 commits depuis `30dfd6c`, dont 2 re-baselines (`2b1151d` C2b, bloc A ; `d57e758` D1, bloc B) et 4 poses de références neuves de D0b (`6bb2265`, `457991c`, `4c40a1a`, `66d3c98`).

L'« État de E0 » écrit aussi l'écart entre inventaire et §6.1, dans les deux sens, pour qu'E1 et E2 sachent exactement ce qu'ils ont à retirer.

## User Stories

1. En tant que mainteneur du plan, je veux une cible `make` par point du §6, afin que la porte de la Phase E soit une commande et non une relecture.
2. En tant que mainteneur, je veux que ces cibles soient lancées sur l'arbre actuel avant toute modification de E, afin de connaître l'écart de départ et de ne pas confondre « jamais passé » et « cassé par E1 ».
3. En tant que mainteneur, je veux que `make float-grep` applique exactement l'expression du §6.1, afin que la cible et le critère écrit ne puissent pas diverger.
4. En tant que mainteneur, je veux que l'ensemble grep pour 6.1 soit calculé par le compilateur (`-MM`) à partir de `CORE_SRCS`, afin qu'un en-tête nouvellement inclus par le cœur y entre sans qu'on le nomme à la main.
5. En tant que mainteneur, je veux que ce calcul se fasse sans les définitions `PICO8_FLOAT_*`, afin que les en-têtes d'audit et d'observation, inclus seulement par ces builds, restent hors du cœur, conformément à la décision humaine.
6. En tant que mainteneur, je veux une liste d'exclusion versionnée avec une justification par fichier, afin que tout fichier de `src/` hors du cœur soit exclu par une décision lisible, pas par omission.
7. En tant que mainteneur, je veux que `float-grep` échoue quand un fichier de `src/` n'est ni dans le cœur ni dans la liste d'exclusion, afin qu'un nouveau module ne soit jamais soustrait en silence au critère.
8. En tant que mainteneur, je veux qu'il échoue quand un fichier est à la fois dans le cœur et dans la liste d'exclusion, afin qu'une exclusion ne puisse pas masquer un fichier que le cœur inclut réellement.
9. En tant que mainteneur, je veux qu'il échoue quand la liste nomme un fichier qui n'existe plus, ou une ligne sans justification, afin que la liste ne se périme pas.
10. En tant que mainteneur, je veux que les mots de commentaire comptent pour 6.1, afin que la cible soit le grep du §6.1 tel qu'écrit, et que l'E2 prévue (mots de commentaires) ait une mesure.
11. En tant que mainteneur, je veux que chaque ligne trouvée par `float-grep` s'imprime en `fichier:ligne:texte`, triée, afin de pouvoir recopier la liste dans l'État et la comparer d'un commit à l'autre.
12. En tant que mainteneur, je veux un compte final (fichiers examinés, fichiers exclus, lignes trouvées), afin que l'État cite des nombres et qu'un ensemble vide ne passe pas pour un succès.
13. En tant que mainteneur, je veux que `float-grep` rende 2, pas 0, sur un ensemble du cœur vide ou une erreur du compilateur pendant `-MM`, afin qu'une mesure sur rien ne passe pour verte.
14. En tant que mainteneur, je veux que `make float-link` lie exactement les objets de `$(TMP_DIR)/pico8_replay`, afin que 6.2 porte sur « la cible de lien du cœur » et non sur une approximation.
15. En tant que mainteneur, je veux que ce lien sans `-lm` produise un binaire jetable, sans toucher la vraie ligne de lien, afin que `make test` et `make replay` restent tels quels jusqu'à E3.
16. En tant que mainteneur, je veux que la liste de sources de `pico8_replay` et celle de `float-link` ne puissent pas diverger, afin qu'un module ajouté au rejeu soit aussi vérifié sans `-lm`.
17. En tant que mainteneur, je veux qu'en cas d'échec `float-link` liste les symboles non résolus, afin de savoir quel appel `libm` reste sans relire la sortie de l'éditeur de liens.
18. En tant que mainteneur, je veux que le binaire lié par `float-link` ne soit jamais exécuté, afin que la mesure reste un fait de lien, pas un rejeu.
19. En tant que mainteneur, je veux que 6.3 reste `make float-guard` sans modification, afin de ne pas perturber un instrument déjà versionné et testé.
20. En tant que mainteneur, je veux un registre versionné des re-baselines depuis `30dfd6c`, une ligne par commit, afin que §6.4 soit lu dans un fichier et vérifié contre git, pas reconstitué de mémoire à partir des États.
21. En tant que mainteneur, je veux que `make frames-ledger` échoue quand un commit touche un `*.frames.hash` sans ligne au registre, afin qu'aucun re-baseline ne passe sans justification.
22. En tant que mainteneur, je veux qu'il échoue quand le registre nomme un commit qui ne touche aucun `*.frames.hash` depuis la base, afin que le registre ne s'orne pas de lignes fictives.
23. En tant que mainteneur, je veux distinguer une **pose** (une référence neuve, fichier ajouté) d'une **divergence** (une référence existante réécrite), afin que les fixtures ajoutées en D0b ne soient pas forcées d'avoir un rapport de divergence qu'elles ne peuvent pas avoir.
24. En tant que mainteneur, je veux qu'une ligne « pose » échoue si son commit réécrit une référence existante, afin qu'on ne puisse pas déguiser un re-baseline en pose.
25. En tant que mainteneur, je veux qu'une ligne « divergence » exige `tools/frames_reports/<étape>.txt` et `<étape>-attribution.txt` présents dans ce commit même, afin que le rapport soit versionné avec le re-baseline, comme la sortie de Phase C et D l'exige.
26. En tant que mainteneur, je veux que l'ensemble des références réécrites par un commit « divergence » soit exactement l'ensemble des fichiers `DIVERGENT` de son rapport, afin qu'aucune référence ne soit réécrite hors rapport, et qu'aucun fichier déclaré divergent ne soit resté sur son ancienne référence.
27. En tant que mainteneur, je veux que chaque ligne « divergence » cite un bloc et une ligne de `tools/probe_out.txt`, et que la vérification la retrouve textuellement dans la section de ce bloc, afin que « justifiée par une ligne du relevé » soit une vérification, pas une affirmation.
28. En tant que mainteneur, je veux que la base `30dfd6c` soit écrite dans le registre lui-même, afin que la donnée et son point de départ soient versionnés ensemble.
29. En tant que mainteneur, je veux que `frames-ledger` rende 2 quand la base n'est pas joignable (clone superficiel, hors dépôt git), afin qu'un historique tronqué ne passe pas pour conforme.
30. En tant que mainteneur, je veux qu'une suppression ou un renommage de `*.frames.hash` fasse échouer `frames-ledger` avec un message explicite, afin que ce cas, absent aujourd'hui, ne soit pas classé en silence.
31. En tant que mainteneur, je veux que chaque script ait un test sur entrées fabriquées dans `make unit`, afin que l'outil lui-même soit prouvé capable d'échouer avant de servir de porte.
32. En tant que mainteneur, je veux que ces quatre cibles restent hors de `make test` en E0, afin que la suite ne soit pas rouge pendant E1–E3 alors que l'échec de 6.1 et 6.3 est attendu.
33. En tant que mainteneur, je veux un « État de E0 » dans le plan avec le résultat des quatre cibles, afin qu'E1–E3 aient une ligne de départ citée et qu'E4 sache ce qu'il verrouille.
34. En tant que mainteneur, je veux que cet État liste l'écart entre inventaire et §6.1 dans les deux sens, afin qu'E1 et E2 sachent quels jetons d'en-tête l'inventaire ne voyait pas, et quels jetons d'inventaire le grep ne verra jamais.
35. En tant que relecteur humain, je veux que la légende en tête du `Makefile` décrive les trois nouvelles cibles comme les autres, afin de les trouver sans lire les scripts.
36. En tant qu'agent, je veux que toute la mesure se fasse sans fenêtre et sans lancer raylib, afin de respecter `CLAUDE.md`.

## Implementation Decisions

**Aucun fichier de `src/` ne change.** E0 ajoute des scripts, des tests, deux fichiers de données versionnés, des cibles `Makefile`, et un « État de E0 » dans `docs/plan-2.md`. `make frames-check` reste identique sans `frames-baseline` : rien de ce qui est compilé dans le cœur ne bouge.

**6.1 — `float-grep` (`tools/float_grep.sh`, liste `tools/float_grep_exclusions.txt`).**
- Arguments : le compilateur, ses flags, le fichier d'exclusion, le répertoire à partitionner (`src`), puis `CORE_SRCS`. Le `Makefile` passe `$(CC)` et `$(CFLAGS)`, jamais `AUDIT_CFLAGS` ni `OBSERVE_CFLAGS`.
- Ensemble du cœur : les `.c` reçus, plus les fichiers que `$(CC) $(CFLAGS) -MM` sur ces `.c` nomme (en-têtes du projet seulement ; `-MM` écarte déjà les en-têtes système). Mesuré à la rédaction : 119 fichiers.
- Partition : tout `.c`/`.h` de `src/` est **soit** dans l'ensemble du cœur, **soit** dans la liste d'exclusion — jamais les deux, jamais aucun. Violations signalées et code 2 (erreur de configuration, distincte d'un flottant trouvé).
- Format de la liste : une ligne `<chemin> | <justification>`, commentaires `#` admis. Justification vide, doublon ou chemin inexistant : code 2. Justification par familles : point d'entrée (`main*.c`), outil d'hôte du §4 (`pico8_assets`, `pico8_dump`), backend raylib, visualiseur/enregistrement d'écran, rejeu et empreintes, builds d'audit/observation (`pico8_float_audit.*`, `pico8_float_observe.*`, qui sortiront d'ailleurs à E1). Le module d'aides de test d'E1 y entrera à son tour, et c'est ce qui réalise « hors les helpers de test explicitement isolés » du §6.1.
- Recherche : l'expression du §6.1 telle quelle, `\b(double|float)\b` en ERE, **commentaires compris** (le §6.1 est un grep brut ; l'E2 prévue retire justement ces mots). Sortie : les lignes `fichier:ligne:texte` triées, puis `float-grep coeur <n> exclus <m> lignes <k>`. Code 1 si `k > 0`.
- Pas de rapport versionné : la mesure est consignée dans l'État, et E4 la rendra bloquante. Seule la liste d'exclusion est versionnée.

**6.2 — `float-link` (`tools/float_link.sh`).**
- Arguments : le compilateur, ses flags, le chemin du binaire jetable, puis les objets. Lie sans `-lm` ; code 1 si le lien échoue, avec la liste triée et dédoublonnée des symboles non résolus extraite du diagnostic (le diagnostic complet reste dans un fichier à côté) ; code 2 sans objet. Le binaire n'est jamais lancé.
- La liste de sources de `pico8_replay` (`src/main.c src/pico8_screencast.c $(ENGINE_SRCS)`) devient une variable du `Makefile`, utilisée par la règle de `pico8_replay` et par `float-link` : c'est la seule retouche d'une règle existante, sans effet sur sa ligne de lien ni sur ses objets, et elle empêche 6.2 de mesurer une autre liste que « la cible de lien du cœur ». `-lm` reste sur la ligne de lien de `pico8_replay` (E3).
- Les objets viennent de l'arbre par défaut (`tmp/obj`), déjà construit par `make test`.

**6.3 — `float-guard`.** Inchangée ; E0 la lance et recopie la liste dans l'État.

**6.4 — `frames-ledger` (`tools/frames_ledger.sh`, registre `tools/frames_ledger.txt`).**
- Registre, lignes `#` ignorées, une ligne `base <commit>` puis une ligne par commit :
  - `<commit> | <étape> | pose` — le commit n'**ajoute** que des `*.frames.hash` ;
  - `<commit> | <étape> | divergence | <bloc> | <ligne du relevé>` — le commit réécrit des références existantes.
  Aujourd'hui : `base 30dfd6c` ; `2b1151d | C2b | divergence | a | 0.001 0x0000.0041` ; `6bb2265 | D0b | pose` ; `457991c | D0b | pose` ; `4c40a1a | D0b | pose` ; `66d3c98 | D0b | pose` ; `d57e758 | D1 | divergence | b | spr(-0.5,4) -> 6,4`. Le choix de la ligne citée : la constante et le cas `spr` auxquels les fichiers d'attribution rattachent toutes les frames divergentes (`0.001` pour C2b, `flr` de `spr` négatif pour D1).
- Arguments : le dépôt, le registre, le relevé, le répertoire des rapports (relatif au dépôt). Commits comparés par empreinte complète (le registre peut les abréger).
- Contrôles :
  1. commits de `base..HEAD` touchant un `*.frames.hash` (statut par fichier, ajout A ou modification M) = commits du registre, dans les deux sens ;
  2. `pose` : aucun M ;
  3. `divergence` : au moins un M ; `<répertoire>/<étape>.txt` et `<étape>-attribution.txt` présents **dans l'arbre de ce commit** ; l'ensemble des fichiers d'événements `DIVERGENT` de `<étape>.txt` (référence = `<événements>.frames.hash`) égale l'ensemble des références M du commit ; les ajouts A dans un commit de divergence sont admis sans justification ;
  4. `<bloc>` est une section `## <bloc> …` de `tools/probe_out.txt`, et la ligne citée y figure mot pour mot ;
  5. toute suppression, renommage ou autre statut : échec, message « non prévu par le registre ».
- Sortie : une ligne par commit (`ok` ou le motif), puis `frames-ledger commits <n> poses <p> divergences <d> erreurs <e>`. Code 1 sur erreur de registre ; code 2 si la base est injoignable ou le répertoire n'est pas un dépôt git.
- **Hypothèse surfacée.** Le texte d'E0 dit « chaque commit … qui touche un `*.frames.hash` a son `tools/frames_reports/<étape>.txt` ». Pris à la lettre, il échouerait sur les 4 commits de D0b, qui ont posé les références de fixtures neuves sans rien réécrire : une référence neuve n'a pas d'« avant » et ne peut pas diverger. Le §6.4 ne parle que de divergences ; la distinction pose/divergence en est la lecture qui ne fabrique pas de rapport vide. Si l'humain la conteste, la seule conséquence est d'écrire 4 rapports « aucune référence » (ce que `tools/frames_report.sh` sait déjà produire).
- Les modifications non commitées d'un `*.frames.hash` sont hors de cette cible : c'est le travail de `frames-check`.

**Makefile.** Trois cibles `.PHONY` nouvelles, hors de `test` ; trois binaires de test (`test_float_grep`, `test_float_link`, `test_frames_ledger`) ajoutés à `unit` et à sa recette, avec leur script en prérequis, comme `test_float_guard`. Légende en tête enrichie des trois lignes, « hors suite ».

**Plan.** Un « État de E0 » à la suite des étapes de la Phase E : pour chacune des quatre cibles, code de sortie, comptes et, en échec, la liste ; la ligne E0 marquée faite ; l'écart inventaire/§6.1 : vus par le grep et pas par l'inventaire (les 3 lignes de `pico8_fixed.h`, les mots de `pico8_geom.h`, `pico8_research.h`, `pico8_type_tag.h`) ; vus par l'inventaire et pas par le grep (les jetons d'identifiant `pico8_fixed_to_double` de la définition, la mention `pico8_fixed_from_double` de `pico8_ob.c` ligne 201). Le texte d'E2 annonce « les 3 d'en-têtes » : l'État confirme ou corrige ce compte d'après la mesure.

**Commentaires.** Les en-têtes de script suivent `tools/float_guard.sh` : usage, codes de sortie, et le piège à ne pas réintroduire (par exemple, dans `float_link.sh`, que le binaire ne doit pas être exécuté ; dans `float_grep.sh`, pourquoi `-MM` se fait sans `PICO8_FLOAT_*`). Rien d'autre, conformément à `CLAUDE.md`.

## Implementation Chunks

1. **6.1 — `float-grep`** : script, liste d'exclusion (30 lignes justifiées), test fabriqué, cible. Vérification : `make unit` vert ; `make float-grep` rend 1 avec les lignes attendues.
2. **6.2 — `float-link`** : script, variable de sources partagée avec `pico8_replay`, test fabriqué, cible. Vérification : `make unit` vert, `make test` inchangé ; `make float-link` rend 0.
3. **6.4 — `frames-ledger`** : script, registre (7 lignes utiles), test sur dépôt git fabriqué, cible. Vérification : `make unit` vert ; `make frames-ledger` rend 0.
4. **Mesure et État** : lancer les quatre cibles (dont `float-guard`) sur l'arbre de ce commit, écrire l'« État de E0 » dans `docs/plan-2.md`. Vérification : `make test` vert, `make frames-check` identique sans `frames-baseline`.

Les chunks 1 à 3 sont indépendants ; 4 dépend des trois.

## Testing Decisions

- **Ce qui fait un bon test ici** : on teste le script par ce qu'il doit empêcher, sur des entrées fabriquées dans un répertoire jetable sous `tmp/`, en lisant sa sortie et son code de sortie — jamais le dépôt réel, jamais la liste d'exclusion ou le registre versionnés (qui sont des données, vérifiées par la cible elle-même). Prior art : `tests/test_float_guard.c` (sources fabriquées, code de sortie relu depuis un fichier sans `<sys/wait.h>`), `tests/test_frames_report.c`, `tests/test_float_inventory_check.c`.
- **`test_float_grep`** : un mini-`src/` fabriqué avec un `.c` du cœur qui inclut un en-tête local contenant `double` en commentaire → ligne trouvée dans l'en-tête, code 1 ; un en-tête inclus seulement sous `#ifdef PICO8_FLOAT_AUDIT` → hors de l'ensemble du cœur, donc à exclure ; fichier ni cœur ni exclu → code 2 ; fichier cœur et exclu → code 2 ; exclusion d'un fichier absent → code 2 ; justification vide → code 2 ; identifiant `pico8_fixed_to_double` seul → aucune ligne (la frontière de mot du §6.1) ; `double-free` → une ligne ; cœur sans flottant → code 0 et `lignes 0` ; aucun `.c` → code 2 ; deux passes → sortie identique.
- **`test_float_link`** : deux objets entiers → code 0, binaire produit ; un objet qui appelle `sin` sur un argument `volatile`, compilé `-fno-builtin` → code 1 et `sin` dans la liste des non résolus, **sur un hôte où `libm` est séparée de la libc** ; le test le détecte d'abord (un tel programme se lie-t-il sans `-lm` ?) et, sinon, imprime qu'il saute ce cas au lieu de réussir en silence ; aucun objet → code 2.
- **`test_frames_ledger`** : un dépôt git fabriqué sous `tmp/` (identité git locale au dépôt), un `probe_out` fabriqué avec deux sections, des commits : base ; ajout d'une référence (pose) ; réécriture d'une référence avec rapport et attribution dans le même commit (divergence). Cas : registre juste → code 0 ; commit manquant au registre → 1 ; commit du registre absent de l'historique → 1 ; pose qui réécrit → 1 ; divergence sans rapport, ou avec le rapport seulement dans un commit ultérieur → 1 ; rapport dont l'ensemble `DIVERGENT` ne couvre pas les références réécrites, ou en déclare une de trop → 1 ; ligne de relevé absente, ou présente dans un autre bloc → 1 ; suppression d'une référence → 1 ; base inconnue → 2.
- **Pas de nouveau test pour `float-guard`** : `test_float_guard` existe.
- Aucune des quatre cibles n'entre dans `make test` en E0. Le coût de `make unit` augmente de trois petits binaires et d'un dépôt git fabriqué ; l'État le mesure si l'écart dépasse la seconde.

## Out of Scope

- Toute modification de `src/` : déplacer `from_double`/`to_double` (E1), retirer le build d'observation (E1), réécrire les mots de commentaires et restreindre TROU-1 (E2), retirer `-lm` de la ligne de lien de `pico8_replay` (E3).
- Faire entrer les cibles 6.1, 6.3, 6.4 dans `make test` (E4).
- Toute vérification PSX / `mipsel` / `nm` soft-float (décision humaine, Phase D).
- Le volet `malloc` (§8).
- Réécrire les rapports de `tools/frames_reports/` ou les références d'empreintes existantes ; poser un rapport pour les poses de D0b.
- Réparer `make constant-variants` ou `make float-observe`, constatés cassés en C2b et D1.
- L'audio, et tout lancement de l'application raylib.

## Further Notes

- Chiffres de la section Solution mesurés à la rédaction de cette PRD (GCC 13.3, x86_64) : `-MM` sur les 58 `CORE_SRCS` → 119 fichiers ; `src/` en compte 149, donc 30 à exclure ; grep → 13 lignes, 8 fichiers ; lien d'essai des objets de `pico8_replay` sans `-lm` réussi sur des objets existants de `tmp/obj`. L'État de E0 doit les re-mesurer sur l'arbre de son commit.
- La frontière de mot de `\b(double|float)\b` rend le §6.1 aveugle aux identifiants `pico8_fixed_to_double` : c'est E1 (prototype absent de l'en-tête du cœur, `-Werror` sur déclaration implicite) qui ferme cet angle, pas le grep.
- Les références `D0b` n'ont pas de rapport dans `tools/frames_reports/` : leur ligne de registre `pose` documente pourquoi, sans rien inventer après coup.
- `tools/frames_reports/D3.txt` et `D5.txt` existent sans commit de re-baseline (étapes sans divergence) : le registre ne les exige pas et ne les interdit pas.
