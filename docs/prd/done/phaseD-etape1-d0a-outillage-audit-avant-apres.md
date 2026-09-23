---
title: "Plan 2, Phase D0a : outillage de D — audit avant/après par site, garde de C1 figée et dans make test"
description: "Les étapes D vont changer volontairement des valeurs (troncature → flr, tostr, ^0.75). Il leur faut un audit qui dise, par site et par fichier d'événements, combien d'appels changent et à quelle frame le premier change, sans perdre la garde de C1 qui repose aujourd'hui sur « tout appel de pico8_fixed_flr_int est un site C1 »."
status: needs-triage
---

## Problem Statement

La Phase C est close. Il reste, d'après `tools/float_inventory.txt`, 39 casts `(int)pico8_fixed_to_double(…)` (22 d'étape D1, 17 « à auditer »), les 3 jetons D2 de `pico8_value_tostr` (`%.4f`) et les 5 jetons D3 de `pico8_dj` (`pow`). Contrairement à C1, les étapes D **changent volontairement des valeurs** : D1 passe de la troncature à `flr`, D3 remplace `pow` par `sqrt(x)·sqrt(sqrt(x))`. `frames.hash` divergera, et le plan exige (Phase D, « Règle d'attribution ») que la première frame divergente F de chaque fichier d'événements soit rattachée à **un site**, et (« Corrections invisibles au filet ») que tout site dont la correction ne change aucun appel sur le filet soit listé.

L'outil d'attribution de C2b, `make constant-variants`, ne sait remplacer que des littéraux : il n'a plus rien à remplacer. Le build d'audit de C1 (`make float-audit`, `-DPICO8_FLOAT_AUDIT`) est le bon point de départ, mais il ne suffit pas, pour quatre raisons :

1. **Il ne connaît pas la frame.** Il compte passages, négatifs et écarts par (fichier, ligne) et garde le premier argument fautif, pas la frame où il est apparu. La règle d'attribution a besoin de « la frame du premier ancien ≠ nouveau ».
2. **Il additionne les rejeux.** Les 8 fichiers de `FRAMES_EVENTS` déversent dans un seul fichier d'enregistrements. Or F se lit **par fichier d'événements** (`tools/frames_reports/<étape>.txt`) : il faut la frame du premier écart par site **et par fichier**.
3. **Sa garde suppose que tout site est C1.** Il échoue sur tout argument négatif et tout écart, à chaque appel de `pico8_fixed_flr_int`. Dès que D1 appellera la même fonction à un site (c), un négatif et un écart y seront la correction voulue : la porte tomberait en rouge, ou il faudrait l'affaiblir partout — et les 37 sites C1 perdraient leur garde. L'en-tête d'audit le dit déjà (« la porte devra alors distinguer les sites »). Or les lignes `| C1 |` ont été retirées de `tools/float_annotation.txt` en C1 : plus rien, dans le dépôt, ne dit quels appels sont les 37 sites C1.
4. **Il est hors de `make test`.** La preuve de C1 n'est rejouée qu'à la main. Une étape D qui toucherait par erreur un site C1 ne serait vue que si quelqu'un pensait à lancer la cible.

D0a ne change aucun comportement du jeu : c'est de l'outillage, posé **avant** D0b et D1, pour qu'il ait été vu passer sur l'arbre actuel avant de servir.

## Solution

Le build d'audit de C1 devient un **audit avant/après par site**, avec deux classes de sites, et entre dans `make test`.

- **Par site et par appel**, l'enregistreur compte : les appels, les appels où l'ancienne expression diffère de la nouvelle, les appels à **argument fractionnaire** (partie basse 16 bits non nulle), les arguments négatifs, les appels où la nouvelle valeur diffère d'une **référence `flr` indépendante**, et, pour chaque rejeu, la **frame du premier ancien ≠ nouveau**. La frame vient du numéro de frame que la boucle de rejeu expose déjà (`current_frame` de l'entrée courante, lue par le contexte de dispatch) : aucune modification du pilote de rejeu.
- **Un fichier d'enregistrements par fichier d'événements**, que le rapport ne fusionne que pour les totaux par site ; il imprime en plus, par fichier d'événements, la plus petite frame de premier ancien ≠ nouveau et le ou les sites qui l'atteignent — la donnée exacte que la règle d'attribution compare à F.
- **Les 37 sites C1 sont figés** dans une liste versionnée, à clé stable (fichier, fonction, extrait, occurrence), sur le modèle de l'annotation. Tout appel de `pico8_fixed_flr_int` du cœur est de classe **C1** s'il est dans la liste, de classe **D** sinon. Verdict par classe : C1 échoue sur tout négatif, tout ancien ≠ nouveau et tout site non atteint (la porte de C1, inchangée) ; D échoue sur tout nouveau ≠ `flr`. Pour un site D, négatifs, écarts et fractionnaires sont des **mesures**, pas des échecs ; un site D sans aucun ancien ≠ nouveau sur le filet (non atteint compris) est listé **« correction non vue par le filet »**.
- **La paire de D1** est celle de C1 (ancien : troncature flottante ; nouveau : `pico8_fixed_flr_int`) : D1 n'aura qu'à convertir ses casts pour être audité. D2 et D3 brancheront **leur propre paire** sur le même enregistreur quand elles l'écriront ; D0a leur livre le point d'entrée générique, pas les paires.
- **`make float-audit` entre dans `make test`.** Il passe aujourd'hui : 37 sites C1, 0 site D, 0 échec.

## User Stories

1. En tant qu'agent qui mènera D1, je veux connaître, pour chaque site converti et chaque fichier d'événements, la frame du premier appel où l'ancienne et la nouvelle expression diffèrent, pour appliquer la règle d'attribution sans rejeu supplémentaire.
2. En tant qu'agent qui mènera D1, je veux que le rapport donne, par fichier d'événements, la plus petite de ces frames et le site qui l'atteint, pour comparer directement à la première frame divergente F de `tools/frames_reports/D1.txt`.
3. En tant qu'agent, je veux que deux sites qui atteignent la même plus petite frame soient tous deux nommés, pour qu'une attribution ambiguë se voie au lieu d'être tranchée en silence.
4. En tant qu'agent, je veux que les frames de l'audit soient numérotées comme celles de `tools/frames_check.sh` et `tools/frames_report.sh` (à partir de 0, frame i = i-ème capture), pour que « égale F » ait un seul sens.
5. En tant qu'agent, je veux que les appels faits pendant l'initialisation, avant la première frame du rejeu, soient rattachés à la frame 0, pour qu'un changement qui ne peut se voir qu'à la première capture soit attribué à la première capture.
6. En tant qu'agent, je veux le nombre d'appels à argument fractionnaire par site, pour que D0b et D4 lisent directement quelles clés de table ont vu une clé non entière sur le filet.
7. En tant qu'agent, je veux le nombre d'appels où ancien ≠ nouveau par site, pour distinguer une correction qui agit sur le filet d'une correction invisible.
8. En tant qu'agent, je veux que tout site de classe D à zéro ancien ≠ nouveau soit listé « correction non vue par le filet », non atteint compris, pour appliquer la règle « Corrections invisibles au filet » à partir d'une liste produite, pas relue.
9. En tant qu'agent, je veux que la nouvelle valeur d'un site D soit comparée à une référence `flr` calculée indépendamment de `pico8_fixed_flr_int`, pour que « nouveau == `flr` » soit une vérification et non une tautologie.
10. En tant que mainteneur, je veux que les 37 sites C1 soient figés dans une liste versionnée, pour que leur garde ne dépende plus de l'hypothèse « tout appel de `pico8_fixed_flr_int` est C1 », fausse dès D1.
11. En tant que mainteneur, je veux que la clé d'un site C1 soit (fichier, fonction, extrait, occurrence), comme dans l'annotation, pour qu'elle survive au déplacement des lignes et à la conversion d'un site voisin.
12. En tant que mainteneur, je veux qu'une entrée de la liste C1 sans appel correspondant dans les sources fasse échouer l'audit, pour qu'une liste périmée ne passe jamais pour une garde.
13. En tant que mainteneur, je veux qu'une clé dupliquée dans la liste C1 fasse échouer l'audit, pour qu'aucune entrée n'en masque une autre.
14. En tant que mainteneur, je veux que les sites C1 gardent exactement la porte de C1 (aucun négatif, aucun ancien ≠ nouveau, aucun non atteint), pour que la preuve de C1 soit rejouée à chaque `make test`.
15. En tant que mainteneur, je veux que `make float-audit` soit dans `make test`, pour qu'une étape D qui toucherait un site C1 casse la suite au lieu de passer inaperçue.
16. En tant que mainteneur, je veux que l'audit passe sur l'arbre actuel avec 37 sites C1, 0 site D et 0 échec, pour qu'il entre dans la suite vert et ait été vu vert avant de servir.
17. En tant que mainteneur, je veux que l'audit garde ses échecs structurels (aucun site trouvé, débordement de la table de sites, rejeu mort), pour qu'une porte qui ne voit rien ne passe pas.
18. En tant qu'agent qui mènera D2 ou D3, je veux un point d'entrée d'enregistrement générique (site, argument, ancien ≠ nouveau), pour brancher ma paire `%.4f`/nouveau `tostr` ou `pow`/`sqrt·sqrt(sqrt)` sans réécrire l'enregistreur ni le rapport.
19. En tant qu'agent qui mènera D2 ou D3, je veux que ma paire apparaisse dans le même rapport, avec la même frame de premier ancien ≠ nouveau par fichier, pour appliquer la même règle d'attribution.
20. En tant qu'agent, je veux que l'audit rejoue exactement `FRAMES_EVENTS`, pour que « tout le filet » ait le même sens que pour `frames-check`.
21. En tant qu'agent, je veux que le rapport imprime des comptes par classe (sites C1, sites D, atteints, non atteints, échecs, corrections non vues), pour lire la porte sans interpréter les lignes.
22. En tant qu'agent, je veux que le script de rapport soit testé sur des enregistrements et des sources fabriqués qui exercent chaque verdict et chaque échec, pour que la porte ne puisse pas être verte par construction.
23. En tant qu'agent, je veux que D0a ne change aucune valeur du jeu, pour que `frames-check` reste identique sans `frames-baseline`.
24. En tant qu'agent, je veux que le build par défaut reste textuellement inchangé par l'instrumentation, pour que `frames-check` et l'inventaire ne dépendent pas de l'audit.
25. En tant qu'agent, je veux que l'inventaire et l'annotation ne soient pas touchés, pour ne pas mêler D0a au classement des casts (D0b).
26. En tant que mainteneur, je veux que `docs/plan-2.md` reçoive un « État de D0a » avec la sortie réelle de l'audit (sites par classe, passages, fractionnaires des sites C1, durée mesurée dans `make test`), pour que D0b parte d'un état consigné.
27. En tant que mainteneur, je veux que la légende du `Makefile` dise que `float-audit` est dans la suite et ce qu'il vérifie par classe, pour que la cible reste trouvable et lisible.
28. En tant qu'humain, je veux que D0a ne lance jamais l'application raylib, pour que la vérification reste entièrement headless (CLAUDE.md).

## Implementation Decisions

- **Numéro de frame : lu, pas ajouté.** La boucle du pilote de rejeu enregistre l'entrée courante dans le contexte de dispatch (avant l'initialisation, puis à chaque tour) et l'entrée porte `current_frame`, qui vaut i pendant le tour i et 0 pendant l'initialisation. L'enregistreur d'audit lit ce numéro par le getter du contexte de dispatch. Conséquences, toutes voulues :
  - aucune ligne du pilote de rejeu ni du cœur ne change, et rien n'est compilé conditionnellement dans la boucle ;
  - la numérotation est celle de la capture (frame i = capture après le dessin du tour i), donc celle de `frames_check.sh`/`frames_report.sh` ;
  - les appels de l'initialisation (avant la boucle, entrée déjà posée à 0) comptent en frame 0 ;
  - si aucune entrée n'est posée (getter nul), la frame est enregistrée comme « hors frame » (−1) ; cela n'arrive pas dans le binaire de rejeu audité, et le rapport l'affiche tel quel s'il arrive.
  L'alternative — un crochet dans la boucle compilé sous le drapeau d'audit — est écartée : elle touche le pilote pour une information qu'il expose déjà.

- **Enregistreur : un point d'entrée générique, une paire livrée.**
  - Point d'entrée : un appel qui reçoit la position du site (fichier, fonction, ligne), l'argument brut 16.16, le fait que l'ancienne valeur diffère de la nouvelle, et le fait que la nouvelle diffère de la référence. Il met à jour, par (fichier, ligne) : appels, négatifs, fractionnaires (partie basse 16 bits non nulle), ancien ≠ nouveau, nouveau ≠ référence, premier argument fautif en hexa brut, et **frame du premier ancien ≠ nouveau**. C'est le seul endroit qui lit la frame.
  - Paire livrée par D0a, celle de `pico8_fixed_flr_int` (C1 aujourd'hui, D1 demain) : nouveau = la vraie fonction ; ancien = `(int)pico8_fixed_to_double(x)` ; référence = `flr` calculé par la libc sur le `double` (`floor`), indépendant de l'implémentation entière du module fixe. L'enregistreur a le droit au flottant et à la libc (§4 : les outils gardent la libc).
  - Paires D2 et D3 : **non livrées** (Simplicity First — aucune ne peut être exercée sur un site réel avant que D2/D3 n'écrivent leur nouveau code). Chacune ajoutera, dans son étape, son enveloppe qui calcule ancien et nouveau puis appelle le point d'entrée générique. Le contrat que D0a leur garantit : le point d'entrée, le format d'enregistrement et le rapport acceptent n'importe quel site, et la classe d'un site non C1 est D. L'en-tête d'audit le dit en une ligne.
  - Déversement : comme aujourd'hui, à la sortie, en mode ajout, vers le chemin de la variable d'environnement existante ; la cible donne **un chemin par fichier d'événements**. Une ligne d'enregistrement par site porte les nouveaux champs (fractionnaires, nouveau ≠ référence, frame du premier ancien ≠ nouveau ou « - »). La table de sites garde sa taille fixe et son marqueur de débordement.

- **Liste C1 figée.**
  - Un nouveau fichier versionné sous `tools/`, une ligne par site : fichier | fonction | extrait | occurrence. Extrait = l'argument de `pico8_fixed_flr_int(` tel qu'écrit (parenthèse assortie, comme l'extrait de l'inventaire) ; occurrence = rang parmi les appels de même extrait dans la même fonction (par exemple les deux `s_val.as.fixed` de la même fonction de `pico8_planet_display.c` sont 1 et 2). En-tête : ce que dit une ligne, et qu'un cast reclassé (b) par D0b et converti la rejoint dans le même commit.
  - Contenu initial : les 37 appels actuels de `pico8_fixed_flr_int` du cœur, tous C1 aujourd'hui. Il est **produit** à partir des sources (la même lecture statique que le rapport), puis versionné et édité à la main ensuite. Vérification : 37 lignes, et la liste des fichiers égale la répartition de l'« État de C1 ».
  - Fichier séparé plutôt que retour des lignes `| C1 |` dans `tools/float_annotation.txt` : l'annotation ne porte que les casts restants, et l'inventaire rejette toute annotation sans cast (erreur 4) ; y remettre C1 imposerait de modifier `tools/float_inventory.sh`, que (ii) n'exige pas. **L'inventaire, l'annotation et leurs noms d'étape codés en dur ne sont pas touchés.**

- **Rapport : classes et verdicts.**
  - Le script de rapport existant (POSIX `sh`/`awk`) garde sa lecture statique des appels `pico8_fixed_flr_int(` dans `CORE_SRCS`, hors de la définition, et y ajoute extrait et occurrence pour joindre la liste C1. Il reçoit en plus la liste C1 et, au lieu d'un seul fichier d'enregistrements, un fichier par fichier d'événements, étiqueté par le nom de ce fichier d'événements. La forme exacte des arguments est laissée à l'implémentation ; elle est couverte par le test.
  - Classe : C1 si la clé du site est dans la liste, D sinon. Erreurs de liste (échec) : entrée sans site correspondant, clé dupliquée.
  - Verdict C1, inchangé : `ECART` (ancien ≠ nouveau), `NEGATIF`, `NON ATTEINT` → échec.
  - Verdict D : `NOUVEAU != FLR` → échec. Négatifs, fractionnaires et ancien ≠ nouveau sont imprimés comme mesures. Zéro ancien ≠ nouveau (non atteint compris) → statut « correction non vue par le filet », **pas un échec** : la règle du plan exige alors un test au niveau valeur ou une entrée du plan, décidé par l'étape D qui convertit, pas par l'outil.
  - Pour les sites d'un appel de paire D2/D3 (enregistrements dont la position n'est pas un appel de `pico8_fixed_flr_int` dans les sources), le rapport les imprime en classe D, sans lecture statique ; D2/D3 décideront s'ils ajoutent leur lecture statique. Aujourd'hui il n'y en a aucun.
  - Une ligne par site : fichier | fonction | ligne | classe | appels | négatifs | fractionnaires | ancien≠nouveau | nouveau≠flr | statut, puis la frame du premier ancien ≠ nouveau **par fichier d'événements** pour les sites qui en ont une.
  - Une section par fichier d'événements : « premier ancien ≠ nouveau : frame N, site(s) … » ou « aucun ». C'est l'entrée de la règle d'attribution.
  - Comptes : sites C1, sites D, atteints, non atteints, échecs par cause, corrections non vues, passages. Granularité : la ligne, comme aujourd'hui ; deux appels d'une même ligne partagent leurs compteurs et sont marqués (ligne partagée). À noter pour D1 : `pico8_qb_md` (`cp.x`, `cp.y`) et `pico8_mz_kj` (`cp.x`, `cp.y`) partagent chacun une ligne ; une frame de premier écart y désigne la ligne, pas l'appel.
  - Codes de sortie : ceux d'aujourd'hui (aucun site, débordement, échec de site), plus une erreur de liste C1 et un enregistrement mal formé ; échecs de site signalés après le rapport entier.

- **Cible et suite.**
  - `make float-audit` : un fichier d'enregistrements par entrée de `FRAMES_EVENTS`, effacés avant la passe ; la cible dépend de la liste C1 et du script. Toujours pas de rapport versionné : la porte se lit sur la sortie et le code de retour.
  - `test` dépend de `float-audit` ; la légende du `Makefile` passe `float-audit` de « hors suite » à « dans make test », avec les deux verdicts par classe. Durée mesurée par la requête du plan : environ 15 s ; la mesure réelle va dans l'« État de D0a ».

- **Invariants de comportement.** Aucune ligne du cœur ne change hors de ce que le drapeau d'audit instrumente déjà ; `frames-check` identique sur les 8 fichiers, sans `frames-baseline` ; `float-inventory-check` et `constants-check` inchangés.

- **Commentaires** (CLAUDE.md) : le contrat du point d'entrée (ce qu'un appelant D2/D3 doit lui passer ; frame 0 pour l'initialisation, −1 hors frame) ; le piège existant (`pico8_fixed.c` hors du drapeau), et le remplacement de la phrase « la porte devra alors distinguer les sites » par ce qui la distingue désormais (la liste C1) ; les codes de sortie du script ; l'aide à la lecture des commandes du `Makefile`. Le récit va dans les messages de commit.

## Implementation Chunks

1. **Liste C1 figée et classes dans le rapport.** Extrait et occurrence dans la lecture statique ; la liste C1 produite puis versionnée (37 lignes) ; jointure, erreurs de liste, classe C1/D, verdict D `NOUVEAU != FLR`, statut « correction non vue par le filet ». Le format d'enregistrement ne change pas encore : les champs nouveaux manquants valent 0 / « - ». Test du rapport étendu. Acceptation : `make test` vert ; `make float-audit` vert à la main, avec 37 sites C1, 0 site D.
2. **Enregistreur avant/après et frame.** Point d'entrée générique, paire `pico8_fixed_flr_int` réécrite dessus avec la référence `flr` libc, fractionnaires, nouveau ≠ référence, frame du premier ancien ≠ nouveau lue par le contexte de dispatch ; nouveau format d'enregistrement ; un fichier d'enregistrements par fichier d'événements dans la cible ; section par fichier d'événements dans le rapport. Test du rapport étendu (nouveaux champs, frames par fichier, égalité de frames). Acceptation : `make float-audit` vert, 37 C1, 0 D, section par fichier d'événements « aucun » partout ; `frames-check` identique.
3. **Entrée dans `make test`, légende, consignation.** `test` dépend de `float-audit` ; légende du `Makefile` ; « État de D0a » dans `docs/plan-2.md` (sortie réelle de l'audit, fractionnaires vus aux sites C1, durée ajoutée à la suite), et la puce D0a marquée faite. Acceptation : `make test` vert en entier, `frames-check` identique sans `frames-baseline`.

## Testing Decisions

- Un bon test porte sur ce que l'unité rend (le rapport, le code de sortie) à partir d'entrées qu'il fabrique ; il exerce d'abord ce que l'unité doit empêcher, et n'inspecte ni les expressions `awk` ni les fichiers intermédiaires.
- **Script de rapport** (extension de `tests/test_float_audit.c`, sources et enregistrements fabriqués, tous les cas actuels conservés) :
  - un site listé C1 avec un écart, un négatif ou non atteint échoue, comme aujourd'hui ;
  - le même site, **absent** de la liste C1, est classe D : négatif et écart imprimés, pas d'échec ;
  - un site D avec nouveau ≠ `flr` échoue ;
  - un site D à zéro ancien ≠ nouveau, et un site D non atteint, sortent « correction non vue par le filet » sans échec ;
  - une entrée de liste C1 sans site, et une clé dupliquée, échouent ;
  - deux appels de même extrait dans une fonction prennent les occurrences 1 et 2, et la liste peut n'en désigner qu'un ;
  - avec deux fichiers d'événements, la frame du premier ancien ≠ nouveau est rapportée séparément pour chacun, et les totaux par site s'additionnent ;
  - la section par fichier d'événements donne la plus petite frame et son site, et nomme les deux sites en cas d'égalité ;
  - un enregistrement à l'ancien format (champs manquants) fait échouer le rapport avec un message, au lieu d'être compté à zéro en silence ;
  - deux exécutions donnent la même sortie.
- **Enregistreur** : pas de test unitaire dédié, comme en C1 (précédents : `-DPICO8_FLOAT_OBSERVE`, `-DPICO8_REF_AUDIT`). Il est exercé par `make float-audit`, désormais dans `make test`, qui doit trouver 37 sites C1 atteints et 0 échec. La lecture de la frame est vérifiée indirectement au chunk 2 par la section « aucun » (aucun écart aujourd'hui) ; D1 en sera le premier usage réel, confronté à F.
- **Liste C1** : vérifiée par l'audit lui-même (entrée orpheline ou dupliquée = échec) et par son compte, 37, consigné dans l'« État de D0a ».
- **Non-régression** : `make frames-check` identique sur les 8 fichiers de `FRAMES_EVENTS` à chaque chunk.

## Out of Scope

- **Toute conversion de site** : aucun cast D1 ni « à auditer » converti, aucune ligne de `pico8_value_tostr` ni de `pico8_dj` modifiée.
- **Les paires D2 (`%.4f` contre nouveau `tostr`) et D3 (`pow` contre `sqrt·sqrt(sqrt)`)** : leurs étapes les branchent sur le point d'entrée générique.
- **Le classement des 17 « à auditer »** et leur revue de clé (D0b), la chaîne PSn00bSDK et la mesure `sin`/`cos` (D0c).
- **L'inventaire, l'annotation, et les noms d'étape codés en dur dans `tools/float_inventory.sh`** (remarque de warn-review) : (ii) ne l'exige pas, la liste C1 est un fichier séparé.
- **Tout `frames-baseline`**, tout rapport de divergence d'étape.
- **Un rapport d'audit versionné** : la porte se lit sur la sortie.
- **Un crochet dans la boucle de rejeu** ou toute modification du pilote de rejeu.
- **Le volet `malloc`** (§8).
- **Lancer l'application raylib.**

## Further Notes

- **Texte de l'étape (docs/plan-2.md, §7 Phase D).** « **D0a — Outillage de D.** (i) Le build d'audit de C1 devient un build avant/après pour chaque étape D : par site modifié, nombre d'appels, appels où ancien ≠ nouveau, appels à argument fractionnaire, et **frame du premier ancien ≠ nouveau**. Même motif pour D2 (ancien `%.4f` contre nouveau) et D3 (ancien `pow` contre nouveau). C'est l'outil d'attribution de D — `constant-variants` ne remplace que des littéraux et n'a plus rien à remplacer. (ii) La garde de C1 est préservée : les 37 sites C1 sont figés dans une liste versionnée, la vérification se fait **par classe de site** (C1 : aucun négatif et ancien == nouveau ; D1 : nouveau == `flr`), et `float-audit` entre dans `make test`. »
- **Règle d'attribution d'une étape D** (même section) : « la première frame divergente F de `tools/frames_reports/<étape>.txt` doit égaler la plus petite « frame du premier ancien ≠ nouveau » parmi les sites de l'étape ; ce site est désigné. Même règle qu'en C2b, lue dans un seul rejeu. » — D0a fournit, par fichier d'événements, cette plus petite frame et son site.
- **Corrections invisibles au filet** (même section) : « pour chaque étape D, les sites à zéro appel ancien ≠ nouveau sur le filet sont listés « correction non vue par le filet » (comme les 13 constantes de C2b). Chacun reçoit un test au niveau valeur de l'expression convertie (par exemple une entrée négative fractionnaire qui doit donner `flr`) ou une entrée explicite dans ce plan. Critère de couverture, pas de ligne `gcov` : C a montré que NON COUVERT ne prouve rien dans un sens comme dans l'autre. Structurel pour D1 : `oi`, `sr` et `flr(ratio)` ne sont (c) que par le repli de `qc.t` à 32 768 frames, alors que le plus long fichier du filet en fait 9 729. » — D0a produit la liste ; il ne fournit ni les tests ni les entrées.
- **Limite connue de l'attribution.** Un site D peut changer de valeur sans changer de pixel ; sa frame de premier écart sera alors plus petite que F. La règle du plan exige l'égalité : dans ce cas, c'est l'étape D qui constate l'écart et l'explique (plus petite frame à effet visible), pas l'outil qui le masque. D0a imprime toutes les frames de premier écart par site et par fichier d'événements pour que ce constat soit possible.
- **Fractionnaires aux sites C1.** L'« État de C1 » cite `radius_f` (`pico8_jo_kj`) comme le seul argument C1 observé fractionnaire. Le compte de D0a le confirmera ou le corrigera ; c'est aussi la première lecture de la donnée dont D4 aura besoin pour les 16 clés C1.
- **Coût dans la suite.** `float-audit` recompile son arbre d'objets à la première passe puis rejoue les 8 fichiers ; environ 15 s mesurées d'après la demande. Si la mesure dépasse nettement ce chiffre, le consigner, ne pas sortir la cible de la suite sans décision.
