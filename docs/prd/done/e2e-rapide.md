---
title: "e2e rapide : des rejeux courts depuis l'écran titre, choisis par la couverture de lignes"
description: "Ramener make e2e sous 20 s de rejeu en remplaçant les longs enregistrements et les raccourcis par des rejeux courts, joués depuis l'écran titre, dont chacun ajoute de la couverture de lignes."
status: done
---

## Problem Statement

La suite complète est trop lente pour une boucle de développement : l'humain la mesure à 8–10 min. Sur le conteneur (4 cœurs), `make test` prend déjà 6 s (9 s à froid), mais `make e2e` prend 1 min 27 s, et c'est lui qui borne la boucle.

La cause n'est pas la lecture des `events.json` (le parseur est linéaire) mais la simulation : ~0,57 ms par frame en `-O2`, ~1,8 ms en `-O0`. Trois enregistrements humains dominent :

| rejeu | frames | durée (-O2) |
|---|---|---|
| session_events | 116 045 | 66,6 s |
| long_session_events | 27 273 | 14,4 s |
| events_recorded_1790198294 | 19 832 | 10,3 s |
| trading_hub_session_events | 4 921 | 2,6 s |

`session_events` à lui seul fixe un plancher de 66 s à `make e2e`, quel que soit le parallélisme ; `float-audit` le rejoue une seconde fois. Ce sont des enregistrements de partie, pas des e2e : ils ne testent rien de précis, ils se contentent de passer par beaucoup de code.

À côté, les rejeux courts du filet et une quinzaine de TU s'appuient sur un raccourci : le pilote choisit la difficulté avant la frame 0 et saute l'écran titre. Ce n'est pas une vraie partie, et ce raccourci peut masquer des écarts. Des TU qui bootent le cart et bouclent sur des frames ne sont pas des tests unitaires.

## Solution

- Chaque e2e part de l'écran titre, comme une vraie partie, et tient en **180 frames de jeu** (3 s à `_update60`) après minimisation ; exception listée jusqu'à **600 frames** pour ce qui ne s'atteint pas plus vite.
- Un e2e n'est gardé que s'il **ajoute de la couverture de lignes** (gcov) par rapport aux autres e2e. Les TU ne comptent jamais dans ce calcul : TU et e2e ont des rôles et des coûts différents.
- La couverture de l'e2e complet actuel est figée comme **référence**. Le travail consiste à combler l'écart entre cette référence et le nouvel `e2e-fast`, d'abord par un catalogue de scénarios de jeu, puis par des scénarios ciblés sur les lignes restantes.
- Les nouveaux e2e sont écrits par l'agent en headless quand le scénario s'y prête, sinon joués par l'humain dans raylib ; dans les deux cas minimisés puis relus par l'humain dans raylib.
- À la fin : `e2e-fast` devient `make e2e`, sous 20 s de rejeu (objets à jour, compilation exclue) ; les trois longs enregistrements sont supprimés du dépôt, le raccourci aussi, `float-audit` aussi.

## User Stories

1. En tant que développeur, je veux que `make e2e` rejoue en moins de 20 s, afin de le lancer après chaque changement.
2. En tant que développeur, je veux que chaque e2e parte de l'écran titre, afin que le filet teste une vraie partie et non un état fabriqué.
3. En tant que développeur, je veux qu'aucun e2e ne dépasse 180 frames de jeu après minimisation, afin que le coût de la suite reste borné.
4. En tant que développeur, je veux pouvoir admettre un e2e jusqu'à 600 frames quand une ligne ne s'atteint pas plus vite, afin de ne pas abandonner du code pour une contrainte de durée, à condition qu'il soit listé comme exception.
5. En tant que développeur, je veux que chaque e2e gardé ajoute au moins une ligne couverte, afin que la suite ne contienne aucun test inutile.
6. En tant que développeur, je veux mesurer la couverture de lignes d'un ensemble de fichiers d'événements, afin de juger n'importe quel rejeu.
7. En tant que développeur, je veux connaître les lignes couvertes par l'e2e complet actuel, figées et versionnées, afin de garder la cible une fois les longs enregistrements supprimés.
8. En tant que développeur, je veux voir les lignes de la référence qu'`e2e-fast` ne couvre pas encore, afin de savoir quoi écrire ensuite.
9. En tant que développeur, je veux savoir ce qu'un fichier donné apporte par rapport aux autres, afin d'appliquer le critère de garde.
10. En tant que développeur, je veux une passe de nettoyage finale qui retire, du plus long au plus court, tout fichier dont les lignes sont couvertes par les autres, afin que la suite finale soit maigre.
11. En tant que développeur, je veux que chaque nouveau fichier soit jugé contre ceux déjà présents, afin de ne pas refaire tout le calcul à chaque ajout.
12. En tant qu'humain qui joue, je veux qu'on me demande la partie la plus courte possible pour un scénario donné, afin de ne jouer que ce qui manque.
13. En tant qu'humain qui joue, je veux que ma session soit minimisée automatiquement, afin de pouvoir jouer à mon rythme sans payer les frames d'hésitation.
14. En tant que développeur, je veux que la minimisation retire toutes les frames inactives (souris immobile, aucun bouton, aucun d-pad), afin de ne garder que les frames utiles.
15. En tant que développeur, je veux que la minimisation retire au moins 80 % des frames de déplacement de souris entre deux transitions de bouton, afin de raccourcir les trajets.
16. En tant que développeur, je veux que la minimisation puisse retirer davantage tant que les lignes couvertes restent identiques à la session brute, afin d'aller au plus court sans perdre de couverture.
17. En tant qu'humain relecteur, je veux relire chaque fichier minimisé dans le binaire raylib, afin de vérifier qu'il fait encore ce que j'ai joué — la minimisation décale le tirage `rnd` partagé, et une sonde peut donner autre chose.
18. En tant qu'agent, je veux écrire moi-même un fichier d'événements en headless (rejeu + dump du framebuffer pour placer les clics), afin de ne pas mobiliser l'humain pour les scénarios courts et déterministes.
19. En tant qu'agent, je veux rendre la main à l'humain quand un scénario dépend du tirage ou demande de la planification, afin de ne pas perdre de temps sur ce que je fais mal.
20. En tant qu'humain, je veux valider le catalogue des scénarios avant qu'on en écrive un seul, afin que la structure de la suite me convienne.
21. En tant que développeur, je veux un catalogue de départ — un scénario par type de planète, par technologie, par écran (pause, boutique, score, fin de partie…) — afin que les fichiers portent le nom de ce qu'ils font.
22. En tant que développeur, je veux qu'une fois le catalogue épuisé, les lignes restantes soient regroupées par fonction et traduites en actions de jeu (via le glossaire des noms), afin d'écrire des scénarios ciblés.
23. En tant que relecteur, je veux à côté de chaque fichier d'événements une description (ce qu'il fait, les frames des actions clés), afin de relire et de comprendre un rejeu qui diverge plus tard.
24. En tant qu'humain, je veux transmettre un enregistrement par push sur la branche ou en pièce jointe, afin d'utiliser ce qui est le plus pratique sur le moment.
25. En tant que développeur, je veux que les lignes jugées inatteignables en 600 frames soient listées une par une (fichier:ligne + raison) dans un fichier versionné, afin que l'abandon de couverture soit explicite.
26. En tant que développeur, je veux que les longs enregistrements ne soient supprimés qu'une fois toutes les lignes de la référence couvertes ou listées inatteignables, afin de ne rien perdre en silence.
27. En tant que développeur, je veux que les 13 TU qui rejouent des événements deviennent des e2e rejouées depuis le titre, afin que les tests unitaires restent unitaires.
28. En tant que développeur, je veux que chacune de ces 13 TU ne disparaisse qu'une fois son e2e de remplacement dans le filet, afin qu'aucun scénario ne reste sans protection pendant la transition.
29. En tant que développeur, je veux que ces e2e converties soient soumises au même critère de garde que les autres, afin de ne garder aucun test inutile.
30. En tant que développeur, je veux que les deux TU qui bootent le cart puis appellent une fonction deviennent de vraies TU construites sur un état capturé au moment de l'appel, afin de garder leur assertion sans booter le cart.
31. En tant que développeur, je veux que les rejeux actuels hors limite passent par la même minimisation avant d'être jugés, afin de ne pas jeter un rejeu qui tiendrait après minimisation.
32. En tant que développeur, je veux que `e2e-fast` réutilise le binaire `-O0` que `make test` construit déjà, afin de ne pas recompiler le moteur une seconde fois et de garder un binaire lisible sous gdb.
33. En tant que développeur, je veux que `e2e-fast` ne fasse que la comparaison d'empreinte, afin de ne payer qu'un rejeu par fichier.
34. En tant que développeur, je veux que `float-audit` soit supprimé entièrement, afin de ne plus maintenir un outil de migration terminée.
35. En tant que développeur, je veux que le pilote de rejeu, `make replay` et `make screencast` exigent le fichier d'événements en argument, afin qu'aucun chemin par défaut ne pointe vers un fichier supprimé ni ne rejoue quelque chose en silence.
36. En tant que développeur, je veux que le raccourci de choix de difficulté disparaisse en dernier, quand plus aucun fichier ne s'en sert, afin que la transition ne casse rien.
37. En tant que développeur, je veux pouvoir re-baseliner les `.sha256` librement pendant ce chantier, afin de ne pas être bloqué par des empreintes qui changent avec les fichiers — le code du jeu, lui, ne change pas.
38. En tant que développeur, je veux que `e2e-fast` devienne `make e2e` à la fin et que l'arbre d'objets `-O2` disparaisse, afin qu'il n'y ait qu'une seule suite e2e.

## Implementation Decisions

- **Mesure de couverture de lignes (module profond).** Entrée : une liste de fichiers d'événements. Sortie : l'ensemble des lignes couvertes, en `fichier:ligne`, sur tout ce qui est lié dans le pilote de rejeu (moteur, point d'entrée, capture). Arbre d'objets dédié compilé avec l'instrumentation gcov, séparé des autres arbres (règle existante : un jeu de flags, un arbre). Opérations exposées : couverture d'un ensemble ; différence entre deux ensembles (l'écart) ; apport d'un fichier par rapport aux autres (le critère de garde). Cible Makefile hors suite.
- **Référence versionnée.** La couverture de l'e2e complet actuel (les 15 rejeux du filet, raccourci compris pour les fixtures qui l'utilisent), figée une fois, versionnée. S'y ajoute la liste des lignes inatteignables (`fichier:ligne` + raison), éditée à la main. Un contrôle compare `e2e-fast` à la référence moins la liste et nomme les lignes manquantes. Tant que l'écart n'est pas nul, ce contrôle est un outil de pilotage, pas une porte de la suite.
- **Minimiseur.** Entrée : un fichier d'événements. Sortie : un fichier plus court, même format. Deux étages :
  1. Règles mécaniques, pures et déterministes : retirer toute frame inactive (souris immobile par rapport à la frame précédente, aucun bouton, aucun d-pad) ; entre deux transitions de bouton (appui ou relâchement), retirer au moins 80 % des frames de déplacement de souris. Les frames de transition elles-mêmes sont toujours gardées.
  2. Passe guidée par la couverture : retirer davantage tant que la mesure de couverture rend exactement les lignes de la session brute.
  Le minimiseur réutilise le chargeur et l'écrivain d'événements existants.
- **Nettoyage final.** Entrée : l'ensemble de lignes de chaque fichier du filet. Sortie : la liste des fichiers à retirer — du plus long au plus court, tout fichier dont toutes les lignes sont couvertes par l'union des autres restants. Au fil de l'eau, un nouveau fichier est jugé seulement contre les fichiers déjà présents.
- **Critère de garde.** 180 frames de jeu après minimisation (600 en exception listée), et au moins une ligne ajoutée. Il s'applique à tout : nouveaux fichiers, rejeux actuels, TU converties.
- **`e2e-fast`.** Cible frames-check sur la liste des nouveaux e2e, avec le binaire de rejeu `-O0` de `make test`. Pas d'audit, pas de second rejeu. À la fin, elle devient `make e2e` ; l'arbre `-O2` et ses flags disparaissent.
- **Emplacement et format.** Les nouveaux e2e vivent dans un répertoire à part, séparé des fixtures des TU : `<nom>.json` (la source, même format que les enregistrements raylib), `<nom>.txt` (description : ce que fait le rejeu, frames des actions clés) et `<nom>.json.sha256`. Pas de mini-langage de script.
- **Écriture d'un e2e.** Par défaut l'agent, en headless : rejeu jusqu'à la frame voulue, lecture du dump texte du framebuffer pour situer les boutons, placement des clics. Repli sur l'humain qui joue dans raylib quand le scénario dépend du tirage ou demande de la planification. Relecture humaine dans le binaire raylib dans tous les cas.
- **Découverte.** D'abord un catalogue tiré du cart (un scénario par type de planète, par technologie, par écran), soumis à l'humain avant toute écriture. Ensuite les lignes restantes, groupées par fonction, traduites en actions de jeu via le glossaire des noms minifiés, un scénario ciblé à la fois.
- **Pilote de rejeu.** Le chemin d'événements par défaut disparaît : sans argument, usage et échec. `make replay` et `make screencast` exigent `EVENTS=`. Le rapport de `ref-audit` reçoit la liste des rejeux du filet du Makefile au lieu d'un chemin codé en dur. Le raccourci de difficulté (pilote de rejeu et pilote de couverture de classes) et l'option qui le désactive disparaissent en dernier ; tout rejeu part alors du titre.
- **`float-audit` supprimé.** Script, liste C1, TU, module d'audit, macros d'audit dans les sources, arbre d'objets d'audit, entrée dans `make e2e`. Justification : le code appelle désormais `flr()`, fidèle au cart ; l'audit ne prouvait que l'équivalence de la migration, terminée.
- **Conversion des TU qui bootent le cart.** Les 13 qui rejouent des événements (`built_planet_panel_nil_row`, `cheat_code`, `empty_planet_cleared`, `game_end`, `geoharvest_burst`, `pause_menu_music`, `playability`, `rich_planet_exports`, `score_panel_close`, `score_panel_stars`, `shop_tooltip_fillp`, `slipway_drag`, `tech_screen`, `top_menubar`) deviennent des scénarios e2e réenregistrés depuis le titre ; leurs assertions disparaissent, l'empreinte suffit. Chacune, avec sa fixture, reste jusqu'à ce que son e2e de remplacement soit dans le filet. Les 2 qui bootent puis appellent une fonction (`ga_export_panel_by`, `planet_gh_row_lq`) : capture de l'état au moment de l'appel, reconstruit ensuite sans boot, comme les TU déjà construites sur `pico8_data_init()`.
- **Fin du chantier.** Quand l'écart est nul (lignes couvertes ou listées inatteignables) : passe de nettoyage, suppression de `session_events`, `long_session_events` et `events_recorded_1790198294` du dépôt, suppression du raccourci, renommage `e2e-fast` → `e2e`.

## Implementation Chunks

1. **Supprimer `float-audit`.** Indépendant, allège `make e2e` tout de suite.
2. **Mesure de couverture de lignes + référence figée.** Tout le reste en dépend ; la référence doit être prise tant que les longs enregistrements et le raccourci existent.
3. **Pilote à argument obligatoire.** `make replay`, `make screencast`, rapport de `ref-audit`.
4. **`e2e-fast` + répertoire des nouveaux e2e + contrôle d'écart.** Vide ou presque au départ.
5. **Minimiseur** (étage mécanique, puis passe guidée par la couverture) et **nettoyage final** (l'outil ; la passe elle-même tourne à la fin).
6. **Minimisation des rejeux actuels hors limite** et jugement de chacun ; en pratique seul `trading_hub_session_events` part du titre et peut entrer.
7. **Conversion des 2 TU à état capturé.**
8. **Catalogue** soumis à l'humain.
9. **Boucle de scénarios** (catalogue, puis lignes restantes) — itérative, humain dans la boucle, hors autopilote. Chaque TU convertie sort au moment où son remplaçant entre.
10. **Fin du chantier** : liste des inatteignables validée, nettoyage final, suppressions, retrait du raccourci, renommage.

## Testing Decisions

- Un bon test vérifie le comportement externe d'un outil sur des entrées fabriquées, pas sa mécanique interne.
- **Mesure de couverture** : TU en script sur de petits fichiers d'événements fabriqués — un fichier qui ne fait rien couvre strictement moins qu'un fichier qui clique ; différence et apport rendent les bons ensembles ; un fichier absent ou malformé fait échouer.
- **Minimiseur, étage mécanique** : frames inactives retirées, transitions de bouton toujours gardées, au moins 80 % des déplacements retirés entre deux transitions, fichier déjà minimal inchangé, format de sortie relu par le chargeur existant.
- **Nettoyage final** : sur des ensembles de lignes fabriqués, deux fichiers identiques n'en laissent qu'un (le plus court) ; un fichier seul à couvrir une ligne n'est jamais retiré ; l'ordre du plus long au plus court est respecté.
- **Référence** : couverte par son propre contrôle.
- **Makefile, suppression de `float-audit`, pilote, TU converties** : `make test` et `make e2e` restent verts.
- Prior art : `test_class_coverage`, `test_float_audit`, `test_frames_check` (outils en script testés sur entrées fabriquées) ; `test_pico8_input` (chargeur/écrivain d'événements) ; `test_pico8_bg_kj` et `test_pico8_data_init` (TU sur un état construit sans boot).

## Out of Scope

- Modifier le comportement du jeu : aucun changement de code du cart porté ; les re-baselines d'empreinte ne viennent que des fichiers d'événements.
- Accélérer le moteur (profilage, optimisation) : la vitesse vient de rejeux plus courts.
- Couverture de branches ou de fonctions : seule la couverture de lignes compte.
- Faire compter les TU dans la couverture.
- Démarrer un rejeu depuis un état sauvegardé ou via un code de triche pour sauter du temps de jeu.
- Assertions sur l'état final dans les e2e.
- `make test`, déjà sous 10 s.
- L'audio.

## Further Notes

- Mesures (conteneur, 4 cœurs) : `make test` 9 s à froid, 5,7 s à chaud ; `make e2e` 1 min 27 s. Après retrait des frames inactives seules, `session_events` passerait de 116 045 à 32 370 frames (~18 s) : l'élagage seul ne suffit pas, d'où les scénarios courts.
- La minimisation change le jeu, pas seulement le fichier : les modules d'affichage consomment `rnd()` à chaque frame et partagent le générateur avec les sondes et les familles de planètes. Retirer des frames décale tous les tirages suivants. D'où la vérification par la couverture et la relecture humaine.
- Rejeux courts actuels du filet par nombre de frames : `top_menubar` 4, `screencast_capture` 8, `shop_button_hover` 25, `pause_menu_music` 32, `tech_screen_invent` 58, `game_end` 74, `phase4_playthrough` 224, `slipway_drag` 226, `empty_planet_cleared` 229, `built_planet_panel` 278, `geoharvest_burst` 293. Tous partent du raccourci : sans lui, leurs clics tomberaient sur l'écran titre.
- `class-coverage` et `class-coverage-net` restent hors suite ; ils suivront le retrait des enregistrements et du raccourci.
