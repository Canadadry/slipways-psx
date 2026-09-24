---
title: "Une empreinte globale par rejeu au lieu d'une empreinte par frame"
description: "Remplacer les *.frames.hash (une ligne par frame, 2 Mo sur les grosses sessions) par un sha256 de toute la capture, et outiller la seule étape dure du diagnostic : reconstruire la capture de référence."
status: needs-triage
---

## Problem Statement

Le filet de non-régression versionne, pour chacun de ses 12 rejeux, un fichier `<events>.frames.hash` qui porte une empreinte par frame. Sur la grosse session `events.json`, c'est 116 045 lignes, soit près de 2 Mo réécrits à chaque re-baseline.

Ce détail ne sert pas à grand-chose. Une seule empreinte suffit pour savoir si le rendu a changé. Pour comprendre ce qui a changé, il faut de toute façon les pixels des deux côtés, et le plan 2 le disait déjà (§5 : « on régénère cette frame des deux côtés et on diffe »). L'empreinte par frame ne fait que localiser la première frame fautive, ce qu'un `cmp` sur les deux captures fait aussi bien.

Autour de ce format s'est accumulé un appareil devenu sans objet :

- les rapports `frames-report` et `tools/frames_reports/` ;
- le registre `frames-ledger` et `tools/frames_ledger.txt`, déjà en retard sur l'historique réel ;
- la garde `ETAPE` de `frames-baseline` ;
- et un doublon : `screencast-check` / `screencast-sign` hachent déjà la capture `events.json` en entier, dans une empreinte `events.json.sha256` qui n'est même pas posée.

L'historique des références est dans git : il n'a pas besoin d'un registre.

Ce qui est vraiment pénible à la main, c'est d'obtenir la capture de référence. Il faut retrouver le commit qui a posé l'empreinte, en faire un worktree, y compiler le pilote de rejeu, rejouer, rapatrier la capture et nettoyer. Rien de tout cela n'est outillé aujourd'hui.

## Solution

Trois gestes, et rien d'autre :

1. **Détecter : `make frames-check`, dans `make test`.** Chaque rejeu du filet a une empreinte versionnée `<events>.sha256`, le sha256 de sa capture `.bin`. La capture n'a ni en-tête ni version (c'est la concaténation brute des frames) : son empreinte est donc exactement celle de toutes les frames, nombre de frames compris. En cas de divergence, le message indique `make frames-ref`.
2. **Poser : `make frames-baseline`.** C'est un acte humain, jamais lancé par `make test`. Il réécrit les `<events>.sha256`, sans garde `ETAPE` et sans rapport.
3. **Diagnostiquer : `make frames-ref`, sans argument, hors suite.** Pour chaque rejeu dont l'empreinte diverge :
   - `git blame` sur `<events>.sha256` désigne le commit de référence ;
   - on construit un seul worktree par commit distinct, on y compile le pilote de rejeu et on rejoue le fichier d'événements tel qu'il est à ce commit ;
   - la capture de référence est rapatriée à côté de la capture courante, puis le worktree est supprimé ;
   - l'outil affiche la première frame divergente, calculée par `cmp`.

   Pour voir une frame des deux côtés, on utilise l'outil existant `pico8_frames frame <capture> <N>`, puis `diff`.

`screencast-check`, `screencast-sign` et `events.json.sha256` sont fusionnés dans le filet : `events.json` devient un rejeu du filet comme les autres.

## User Stories

1. En tant que développeur, je veux que `make test` échoue quand un rejeu ne dessine plus la même chose, afin qu'aucune régression de rendu ne passe sans être vue.
2. En tant que développeur, je veux qu'une seule empreinte par rejeu soit versionnée, afin de ne pas réécrire 2 Mo à chaque re-baseline d'une grosse session.
3. En tant que développeur, je veux qu'un rejeu conforme soit annoncé comme tel dans la sortie de `frames-check`, afin de savoir quels rejeux ont été réellement comparés.
4. En tant que développeur, je veux qu'un rejeu sans empreinte posée ne soit ni capturé ni comparé, et que `frames-check` le dise en indiquant comment poser la référence, afin qu'une fixture neuve ne fasse pas échouer la suite.
5. En tant que développeur, je veux que `frames-check` examine tous les rejeux avant de conclure, afin de voir d'un coup tous ceux qui divergent.
6. En tant que développeur, je veux que le message de divergence de `frames-check` nomme le rejeu fautif, les deux empreintes et la commande `make frames-ref`, afin de savoir quoi faire ensuite sans relire le `Makefile`.
7. En tant que développeur, je veux qu'une capture courante absente soit une erreur de `frames-check` et non un « conforme », afin qu'un rejeu qui n'a rien produit ne passe pas pour juste.
8. En tant qu'humain qui re-baseline, je veux que `make frames-baseline` réécrive les empreintes des 12 rejeux et affiche ce qu'il a posé, afin que la re-baseline tienne en une commande.
9. En tant qu'humain qui re-baseline, je veux que `frames-baseline` ne soit jamais appelé par `make test`, afin qu'une régression ne puisse pas devenir référence par accident.
10. En tant que développeur qui enquête, je veux lancer `make frames-ref` sans argument, afin de ne pas avoir à recopier le chemin d'un fichier d'événements.
11. En tant que développeur qui enquête, je veux que `frames-ref` ne traite que les rejeux dont l'empreinte diverge, afin de ne pas payer la reconstruction des rejeux conformes.
12. En tant que développeur qui enquête, je veux que le commit de référence soit trouvé par `git blame` sur `<events>.sha256`, afin d'utiliser le seul commit dont on sait qu'il a produit cette empreinte.
13. En tant que développeur qui enquête, je veux qu'un seul worktree soit construit par commit distinct, afin que les rejeux qui partagent une référence (le cas courant) ne coûtent qu'une compilation.
14. En tant que développeur qui enquête, je veux que la référence soit rejouée avec le fichier d'événements tel qu'il est au commit de référence, afin que la capture reconstruite corresponde à l'empreinte posée.
15. En tant que développeur qui enquête, je veux que la capture de référence reste sur disque à côté de la capture courante, afin d'émettre n'importe quelle frame N des deux côtés sans rien reconstruire.
16. En tant que développeur qui enquête, je veux que le worktree temporaire soit supprimé à la fin, y compris quand la reconstruction échoue, afin de ne pas laisser de worktrees orphelins dans le dépôt.
17. En tant que développeur qui enquête, je veux que `frames-ref` affiche la première frame divergente de chaque rejeu, afin d'aller droit à la frame à examiner.
18. En tant que développeur qui enquête, je veux que la première frame divergente soit annoncée aussi quand les deux captures ont un préfixe identique et des longueurs différentes, afin qu'un rejeu tronqué ou allongé soit localisé lui aussi.
19. En tant que développeur qui enquête, je veux que `frames-ref` s'arrête avec un message clair quand l'empreinte n'est pas encore commitée (le blame ne désigne aucun commit), afin de ne pas reconstruire une référence qui n'existe pas.
20. En tant que développeur qui enquête, je veux que `frames-ref` échoue bruyamment quand le vieux commit ne compile pas ou que le rejeu échoue, afin de ne jamais comparer avec une capture partielle.
21. En tant que développeur qui enquête, je veux continuer à émettre une frame N d'une capture au format du dump, afin de comparer 128 lignes avec `diff`.
22. En tant que mainteneur, je veux que le code maison de hachage par frame disparaisse, afin de ne pas entretenir ce qu'un outil standard fait déjà.
23. En tant que mainteneur, je veux que `screencast-check` et `screencast-sign` disparaissent au profit du filet, afin qu'il n'existe plus qu'un seul mécanisme d'empreinte sur le même rejeu.
24. En tant que mainteneur, je veux que `make screencast` et le visualiseur d'enregistrement restent, afin de pouvoir encore regarder une session.
25. En tant que mainteneur, je veux que `frames-report`, `frames-ledger`, le registre, les rapports versionnés, leurs scripts et leurs tests disparaissent, afin que le dépôt ne porte plus d'outillage sans usage.
26. En tant que mainteneur, je veux que la migration prouve que les nouvelles empreintes portent sur les mêmes pixels que les anciennes, afin de ne pas baptiser une régression en référence au moment du changement de format.
27. En tant que mainteneur, je veux que la migration tienne en un seul commit, afin que le blame de chaque `.sha256` désigne un commit dont le code produit exactement ces pixels.
28. En tant que lecteur des plans, je veux que le critère du plan 3 et le §5 du plan 2 parlent des `.sha256`, afin que la documentation décrive l'outillage réel.
29. En tant que lecteur du `Makefile`, je veux que la légende en tête ne liste que les cibles qui existent, afin de ne pas chercher `frames-report` ou `screencast-sign`.
30. En tant que développeur sous macOS, je veux que l'empreinte soit calculée par la même commande sha256 portable que celle déjà choisie par le `Makefile`, afin que `frames-check` marche sur les deux plateformes.

## Implementation Decisions

- **Format de l'empreinte** : le sha256 hexadécimal de la capture `.bin` entière, une seule ligne par fichier, nommée `<events>.sha256` et placée à côté du fichier d'événements. Le calcul passe par la variable sha256 portable déjà définie dans le `Makefile` (`sha256sum`, ou `shasum -a 256` sous Darwin).
- **Le filet** : la liste explicite des 12 fichiers d'événements ne change pas. `events.json` en fait déjà partie et partage sa capture avec `make screencast`. Un rejeu sans référence posée n'est ni capturé ni comparé.
- **`frames-check`** : pour chaque rejeu dont la référence est posée, il compare l'empreinte versionnée à l'empreinte de la capture courante. Il examine tous les rejeux avant de conclure, puis rend un code non nul si l'un diverge ou si une capture courante manque. Le message de divergence nomme le rejeu et les deux empreintes, et indique `make frames-ref`. Le script existant peut être gardé et simplifié, ou la vérification peut être inlinée dans le `Makefile`. Le critère : ce qui est le plus simple à tester depuis `make unit`.
- **`frames-baseline`** : il copie les empreintes courantes sur les références, sans argument et sans garde. Il affiche chaque référence posée.
- **`frames-ref`** : une cible hors suite, sans argument. Pour chaque rejeu divergent :
  1. le commit de référence est obtenu par `git blame` sur la ligne de `<events>.sha256` ; un blame qui ne désigne aucun commit (empreinte pas encore commitée) arrête l'outil avec un message ;
  2. les rejeux sont groupés par commit, avec un worktree par commit distinct, créé dans le répertoire temporaire du projet, où l'on compile seulement le pilote de rejeu ;
  3. chaque rejeu est rejoué dans le worktree avec le fichier d'événements de ce commit, et la capture est rapatriée à côté de la capture courante, sous le nom `<events>.ref.bin` ;
  4. le worktree est supprimé dans tous les cas, succès ou échec ;
  5. la première frame divergente est calculée par `cmp` : si l'octet N (compté à partir de 1) diffère, la frame est `(N-1) / 8192`. Un préfixe identique avec des longueurs différentes donne la frame qui suit la fin de la capture la plus courte ;
  6. la sortie donne, par rejeu, la première frame divergente et les deux commandes `pico8_frames frame` à lancer.
- **Pas de vérification de cohérence** de la capture reconstruite contre l'empreinte posée : décision explicite de l'utilisateur, le blame suffit.
- **`pico8_frames`** : il ne garde que la sous-commande `frame <capture> <N>`. La sous-commande `hashes` est supprimée, avec le module de hachage par frame (FNV-1a 64 bits) et son test.
- **Retraits** :
  - `screencast-check`, `screencast-sign` et `events.json.sha256` (ce dernier absent aujourd'hui), ainsi que la ligne `screencast-check` dans les prérequis de `make test` ;
  - `frames-report`, `frames-ledger`, le registre des références, le répertoire des rapports versionnés, leurs scripts et leurs tests unitaires ;
  - les lignes correspondantes de la légende du `Makefile`.
  Un retrait est complet : scripts, tests de `make unit`, règles, cibles et lignes de légende, comme les outils retirés après la Phase E du plan 2.
- **Commentaires** : deux commentaires de test citent `make screencast-check`, et la règle du projet (quatre destinations, voir `CLAUDE.md`) s'y applique. Il faut les réécrire pour nommer `frames-check`, ou les retirer s'ils ne portent plus rien.
- **Documentation** :
  - le critère du plan 3 (« tous les `*.frames.hash` identiques octet pour octet ») devient « tous les `*.sha256` identiques » ;
  - le §5 du plan 2 (Instrument 2) décrit une empreinte par rejeu et `frames-ref` ;
  - les mentions de `frames.hash` dans l'étape 0 du plan 3 suivent.
  Les PRD sous `docs/prd/done/` et les rapports historiques ne sont pas réécrits : ce sont des traces.

## Implementation Chunks

Un seul commit, pour que le blame de chaque `.sha256` désigne un commit dont le code produit ces pixels. Ordre de travail à l'intérieur de ce commit :

1. **Précondition** : sur l'arbre de départ, avant toute modification, `make frames-check` est conforme sur les 12 rejeux avec les anciens `*.frames.hash`. Si ce n'est pas le cas, on s'arrête : la migration gèlerait une régression.
2. **Outillage** : nouveau `frames-check`, `frames-baseline` simplifié, `frames-ref`, `pico8_frames` réduit à `frame`, retraits.
3. **Migration des références** : on pose les 12 `<events>.sha256` avec `frames-baseline`, sur les captures produites par le même code que celui de l'étape 1, puis on supprime les 12 `*.frames.hash`.
4. **Documentation** et légende du `Makefile`.
5. **Vérification** : `make test` est vert. Puis une vérification manuelle de `frames-ref`, non commitée : on altère volontairement un pixel, on lance `frames-check` (divergent), puis `frames-ref` (première frame annoncée cohérente avec l'altération), et on annule l'altération. Cette vérification demande que le commit de migration existe, puisque le blame doit désigner un commit : elle se fait donc juste après avoir commité, avant de pousser.

## Testing Decisions

- Un bon test vérifie le comportement externe (code de sortie, messages, fichiers produits), pas la structure interne des scripts.
- **`frames-check` est testé** : on réécrit `test_frames_check` sur le nouveau format, avec des empreintes et des captures fabriquées. Trois cas au minimum :
  - conforme : sortie 0 et rejeu annoncé conforme ;
  - divergent : sortie non nulle, rejeu nommé, `make frames-ref` présent dans le message, et tous les rejeux examinés même quand le premier diverge ;
  - référence absente : sortie 0, rejeu déclaré non comparé.
  Une capture courante manquante donne une sortie non nulle.
- **`pico8_frames frame`** reste testé : on adapte `test_pico8_frames_tool`, dont la partie `hashes` disparaît avec la sous-commande.
- **`frames-ref` n'est pas testé automatiquement** : décision explicite. C'est un outil de diagnostic hors suite, dont l'échec se voit tout de suite, alors qu'un test demanderait un dépôt git jetable et un faux pilote. Il est vérifié une fois à la main, comme décrit plus haut.
- Prior art : `test_frames_check` (un script shell testé par ce qu'il doit empêcher, sur des fichiers fabriqués) et `test_pico8_frames_tool` (l'outil lancé comme `make` le lance, sur une vraie capture).

## Out of Scope

- Le contenu du rendu lui-même : cette migration ne doit changer aucun pixel, et le plan 3 interdit toute re-baseline.
- Tout rapport de divergence au-delà de la première frame : plages, comptage, attribution.
- Toute traçabilité des re-baselines hors de git.
- Un `diff` intégré des deux dumps dans `frames-ref`.
- La vérification que la capture reconstruite reproduit l'empreinte posée.
- Un argument pour choisir le rejeu ou le commit de référence de `frames-ref`.
- La réécriture des traces historiques (`docs/prd/done/`, rapports du plan 2).

## Further Notes

- La mise à jour du registre `frames-ledger` avait déjà décroché : les commits `de111da` et `3a8ee31`, qui réécrivent `events.json.frames.hash`, n'y figurent pas. C'est un argument de plus pour le retirer plutôt que le porter.
- `float-audit` affiche lui aussi une « frame du premier ancien != nouveau ». Il est indépendant des `*.frames.hash` et n'est pas touché.
- `frames-ref` reconstruit un vieux commit avec l'outillage de ce commit. Tant que la cible de compilation du pilote de rejeu et l'appel `pico8_replay <events> <capture>` restent stables, ce n'est pas un problème. S'ils changent un jour, `frames-ref` échouera bruyamment (user story 20), sans rien comparer.
