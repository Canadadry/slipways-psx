---
title: "Plan 6, étape 2 — Souris nommée"
description: "Le jeu lit la souris par des numéros de stat() PICO-8 (32, 33, 34) au lieu de champs nommés de l'entrée."
status: needs-triage
---

## Problem Statement

Le jeu lit encore la souris comme le cart Lua : `p8_stat(input, 32)` pour x, `p8_stat(input, 33)` pour y, `p8_stat(input, 34)` pour le masque de boutons, plus des masques bruts `1` et `6` passés à `mouse_button_update`. Ces numéros n'ont de sens que pour PICO-8 : qui lit `src/game/mouse.c` doit connaître la table des `stat()` pour savoir ce qu'il lit, et pourquoi le bouton droit vaut `6`. Le plan 6 retire du cœur tout ce qui n'existe que pour PICO-8 ; le plan 8 (jouer au clavier) branchera une couche boutons → souris qui doit écrire dans une souris nommée, pas dans un `stat()`.

Les données sont pourtant déjà nommées : la trame `p8_input_frame_t` porte `mouse_x`, `mouse_y`, `mouse_buttons`. Seul l'aiguillage numéroté `p8_stat` est en trop.

## Solution

`p8_stat` disparaît. L'entrée expose la souris de la trame courante sous des noms : une position x/y et des boutons repérés par des constantes nommées (gauche, droit, milieu). `mouse_update` lit cette souris une fois par tick et donne à chaque bouton un booléen « enfoncé » ; `mouse_button_update` devient une pure machine d'état held/pressed/released, sans dépendance au runtime.

Rien ne change pour le joueur ni pour les rejeux : même format `events.json`, mêmes 26 `*.sha256`, mêmes durées de `make test` et `make e2e`.

## User Stories

1. En tant que mainteneur, je veux lire `mouse_update` sans connaître la table des `stat()` de PICO-8, pour comprendre d'un coup d'œil d'où vient la position du curseur.
2. En tant que mainteneur, je veux que le bouton droit soit écrit « droit ou milieu » avec des constantes nommées, et non `6`, pour ne pas casser par mégarde le comportement du cart où le clic milieu compte comme un clic droit.
3. En tant que mainteneur, je veux que `mouse_button_update` reçoive un booléen « enfoncé », pour tester la machine held/pressed/released sans fabriquer de trame d'entrée.
4. En tant que mainteneur, je veux qu'un seul endroit décide que « pas de trame courante » se lit comme une souris à (0,0) sans bouton, pour que ce contrat ne se disperse pas dans le jeu.
5. En tant qu'auteur du plan 8, je veux une souris nommée dans l'entrée, pour que la future couche boutons → curseur produise une position et des boutons nommés plutôt que des valeurs de `stat()`.
6. En tant qu'auteur du plan 10 (PSX), je veux que le cœur ne contienne plus d'aiguillage `stat(n)`, pour ne pas porter vers la PSX une API qui n'a de sens que sur PICO-8.
7. En tant que joueur sur PC, je veux que le clic gauche, le clic droit, le clic milieu et le glisser de carte se comportent exactement comme avant, pour ne rien perdre de la jouabilité.
8. En tant que joueur, je veux que le curseur dessiné par le jeu suive la souris comme avant, au pixel près.
9. En tant que testeur, je veux que les 26 e2e de `tests/e2e/` rejouent avec des `*.sha256` identiques, pour prouver que l'étape ne change aucun pixel.
10. En tant que testeur, je veux que les fichiers `events.json` existants (e2e, fixtures, enregistrements de playtest) se chargent sans conversion, pour ne rien réenregistrer.
11. En tant que testeur, je veux que l'enregistreur de session et le backend raylib écrivent toujours les mêmes clés `mouse_x`/`mouse_y`/`mouse_buttons`, pour qu'un enregistrement d'avant l'étape rejoue après.
12. En tant que développeur, je veux que `make test` reste à ~4,6 s et `make e2e` à ~0,25 s (chaud), pour garder la boucle rapide.
13. En tant que développeur, je veux qu'un test unitaire couvre l'accesseur de souris de l'entrée (trames successives, entrée vide), pour que son contrat soit fixé.
14. En tant que développeur, je veux qu'un test couvre « milieu seul déclenche le bouton droit », parce qu'aucun e2e n'utilise le bouton milieu et que ce piège passerait inaperçu.
15. En tant que développeur, je veux que `git grep 'p8_stat\|stat(3[234])' -- src tests` ne rende rien à la fin de l'étape, critère de fin du plan 6.
16. En tant que lecteur des docs, je veux que `docs/plan-8.md` ne dise plus que le jeu lit la souris via `p8_stat(32/33/34)`, pour que les docs ne décrivent que ce qui est vrai.
17. En tant que lecteur des en-têtes, je veux que les commentaires de l'entrée, du bouton de souris et du backend raylib parlent de champs nommés et non de `stat(32)`/`stat(34)`, pour qu'aucun commentaire ne nomme une API disparue.
18. En tant que mainteneur, je veux que la touche et la souris du joueur unique restent dans la même trame, pour que le format de rejeu reste un seul objet par frame.

## Implementation Decisions

- **`p8_stat` est supprimé**, déclaration, définition et tous ses appels. Il n'est pas remplacé par un autre aiguillage numéroté.
- **Nouvel accesseur dans le module d'entrée** (`p8/input`, préfixe `p8_` conservé : le module reste dans le runtime, le renommer est hors étape) :
  - un type `p8_mouse_t` à trois champs : `x`, `y`, `buttons` (entiers, dans l'espace écran 128×128) ;
  - trois constantes nommées pour les bits de `buttons` : `P8_MOUSE_LEFT` (1), `P8_MOUSE_RIGHT` (2), `P8_MOUSE_MIDDLE` (4) — mêmes valeurs que le masque d'aujourd'hui, donc le format enregistré ne bouge pas ;
  - une fonction `p8_mouse_t p8_input_read_mouse(const p8_input_t *input)` qui rend la souris de la trame courante, **tous champs à zéro quand il n'y a pas de trame courante** (entrée vide ou `current_frame` hors bornes) — exactement le contrat de `p8_stat` aujourd'hui. Ce contrat C est écrit dans l'en-tête (un des trois cas de commentaire admis).
  - Rejeté : rendre un pointeur sur la trame courante (NULL si aucune) et laisser le jeu lire `mouse_x` directement — le test de NULL partirait dans le jeu, et le jeu dépendrait de toute la trame, d-pad compris.
- **`p8_input_frame_t` et le format `events.json` ne changent pas** : mêmes champs `mouse_x`, `mouse_y`, `mouse_buttons`, même ordre, mêmes clés JSON. Les initialiseurs positionnels des tests, `input_json`, l'enregistreur, la source de rejeu et le backend raylib restent intacts dans leur code.
- **`mouse_button_update` ne reçoit plus l'entrée** : sa signature devient `void mouse_button_update(mouse_button_t *button, int down)`. Elle ne fait plus que la transition held/pressed/released (un appelant unique : `mouse_update`). L'assert de non-NULL ne porte plus que sur le bouton. Le paramètre `oa` (masque brut) disparaît avec le commentaire « raw stat(34) bitmask ».
- **`mouse_update` lit la souris une seule fois par tick** via `p8_input_read_mouse`, construit la position depuis `x`/`y`, puis passe au bouton gauche `buttons & P8_MOUSE_LEFT` et au bouton droit `buttons & (P8_MOUSE_RIGHT | P8_MOUSE_MIDDLE)`, normalisés en 0/1. Un commentaire court à ce site dit que le milieu compte comme le droit, comme le cart (`band(stat(34),6)`) : c'est un piège qu'on réintroduirait, pas un écart — aucune entrée GAP.
- Le retour anticipé de `mouse_update` sur une entrée ou une souris NULL reste tel quel.
- **Commentaires à réécrire** (ils nomment une API qui disparaît) : l'en-tête de trame de `src/p8/input.h` (« feed stat(32)/stat(33)/stat(34) »), la doc de `p8_stat` (remplacée par celle de l'accesseur), `src/game/mouse_button.h` (« raw stat(34) bitmask (1 left, 6 right) »), `src/platform/raylib_backend.h` (« bits 0/1/2 of stat(34)'s layout » → les constantes nommées). Le backend peut écrire ses bits avec `P8_MOUSE_LEFT`/`RIGHT`/`MIDDLE` au lieu de `1`/`2`/`4` : même valeur, même enregistrement ; `make raylib` doit encore compiler (sans lancer le binaire).
- **Doc** : la phrase de `docs/plan-8.md` §2 « Le jeu ne lit la souris que via `p8_stat(32/33/34)` » est réécrite pour nommer `p8_input_read_mouse` (lu par `mouse.c` seul). `docs/trous-de-portage.md` n'a aucune entrée souris/stat : rien à y changer.
- `pico8/lua.lua` est l'archive du cart : ses `stat(32)`… restent.

### Inventaire exact (à HEAD `2250ccc`, avant l'étape 1)

| Fichier | Symbole / ligne | Changement |
|---|---|---|
| `src/p8/input.h` | `p8_stat` (déclaration + doc), commentaire d'en-tête de `p8_input_frame_t` | supprimer ; ajouter `p8_mouse_t`, `P8_MOUSE_*`, `p8_input_read_mouse` ; réécrire le commentaire |
| `src/p8/input.c` | `p8_stat` | remplacer par `p8_input_read_mouse` |
| `src/game/mouse.c` | `mouse_update` (`p8_stat(input, 32/33)`, `mouse_button_update(input, …, 1)` et `(…, 6)`) | lire la souris nommée, passer des booléens |
| `src/game/mouse_button.h/.c` | `mouse_button_update(const p8_input_t *, mouse_button_t *, int oa)`, `p8_stat(input, 34)` | nouvelle signature `(mouse_button_t *, int down)` ; l'include de `p8/input.h` n'est plus nécessaire |
| `src/game/mouse.h` | doc de `mouse_update` (« runs mouse_button_update over… ») | vérifier qu'elle reste vraie, sinon l'ajuster |
| `src/platform/raylib_backend.h/.c` | doc de `raylib_backend_poll_input`, littéraux `1`/`2`/`4` | commentaire et constantes nommées |
| `tests/test_input.c` | 16 asserts `p8_stat(…, 32/33/34)` (l. 19-21, 28-30, 37-39, 178-180, 330-332, 369), `test_stat_unsupported_index_returns_zero` (l. 92-101, indices 0 et 35) | asserts portés sur `p8_input_read_mouse` ; le test d'index non supporté est remplacé par un test « entrée vide → souris nulle » |
| `tests/test_demo.c` | 6 asserts `p8_stat` (l. 100-102, 110-112) et le commentaire « exact stat()/btn() values » | portés sur l'accesseur |
| `tests/test_mouse.c` | — | ajouter « milieu seul (`buttons` = 4) presse le bouton droit, pas le gauche » |
| `docs/plan-8.md` | §2, 3ᵉ puce | réécrire |

Aucun autre appel : `git grep p8_stat` à HEAD ne trouve, hors ces fichiers, que `menu.c` (stat 20) et `slipway.c` (stat 19), retirés par l'étape 1 (H-1-1).

## Testing Decisions

- Un bon test ici observe le comportement extérieur : la souris rendue pour une trame donnée, l'état held/pressed/released d'un bouton après des ticks scriptés, le framebuffer des e2e. Il ne vérifie pas la forme interne de la trame ni qu'une fonction en appelle une autre.
- **`p8/input`** (`tests/test_input.c`) : chaque assert `p8_stat(…, 32/33/34)` devient un assert sur les champs de `p8_input_read_mouse` — chargement de fixture puis pas successifs, aller-retour save/load, append, `buttons` 1/3/4/7 compris. Le test « index non supporté → 0 » disparaît avec le contrat qu'il couvrait ; à sa place, un test « entrée seulement initialisée → x, y, buttons à 0 » (et `current_frame` hors bornes si le cas est atteignable sans assert). Prior art : `test_load_and_step_multi_frame_matches_fields`, `test_stat_unsupported_index_returns_zero`.
- **`tests/test_demo.c`** : mêmes valeurs, lues par l'accesseur.
- **`mouse_button_update`** : pas de test isolé — elle est couverte par `mouse_update` dans `tests/test_mouse.c`, qui reste le filet de non-régression (scripted ticks, boutons qui survivent à `reset_world`, lancement bouton gauche tenu).
- **`tests/test_mouse.c`** : un seul test ajouté, « milieu seul presse le droit » (trame `buttons = P8_MOUSE_MIDDLE` puis `0` : droit pressed puis released, gauche jamais held). Aucun e2e n'a de `mouse_buttons` à 4 : sans ce test, perdre le milieu passerait inaperçu. Prior art : `frame(i, x, y, buttons)` / `script(n)` de ce fichier.
- **Non-régression** : `make test-all` vert, les 26 `*.sha256` de `tests/e2e/` inchangés (aucun `make e2e-baseline`), `make sanitize-net` vert, `make raylib` compile.
- **Durées** : aucun binaire de test nouveau, aucune cible nouvelle ; les tests ajoutés vont dans des binaires existants. `make test` ≤ ~4,6 s et `make e2e` ≤ ~0,25 s (chaud), comparés aux mesures d'avant l'étape 1.
- **Critère de fin** : `git grep -nE 'p8_stat|stat\(3[234]\)' -- src tests` ne rend rien.

## Out of Scope

- Le reste de l'étape 1 (audio, `stat 19/20`, GPIO `0x5f80`, octets `0x3101`, drapeau `0x5f2d`, deuxième manette GAP-2) : déjà traité par l'étape 1, en parallèle.
- `p8_btn` et le d-pad : ni renommés ni restructurés ici (le plan ne les vise pas à cette étape).
- Le format `events.json`, la forme de `p8_input_frame_t`, l'enregistreur, la source de rejeu.
- Renommer le module `p8/input` ou le sortir du runtime.
- Les chaînes `"left"`/`"right"`, `"pressed"`/`"released"` → `enum` : étape 3.
- La couche clavier → souris : plan 8.
- Tout changement de pixel.

## Further Notes

- Une seule implémentation, un seul commit : supprimer `p8_stat` et migrer ses appelants est atomique (tout état intermédiaire ne compile pas ou garde deux API). Pas de découpage en chunks.
- Les points « vérifier avec l'utilisateur » de `to-prd` (modules, tests à écrire, découpage) ont été posés à un agent neuf. La skill `guideline` n'était pas installée dans sa session : ses réponses sont son propre jugement, appliqué aux règles du dépôt. Il a validé l'accesseur rendant une valeur (plutôt qu'un pointeur sur la trame), `mouse_button_update` à booléen, le format inchangé, un commit unique, la mise à jour des commentaires et de `plan-8.md`, et recommandé le seul test isolé « milieu → droit » dans `test_mouse.c` plutôt qu'un test de `mouse_button` seul.

## Hypothèses sur l'issue de l'étape 1

L'étape 1 est en cours en parallèle ; ce PRD a été écrit contre HEAD `2250ccc`. Chaque hypothèse ci-dessous est à confirmer quand elle aura atterri.

- **H-1-1** — Après l'étape 1, `p8_stat` existe encore (même signature `int p8_stat(const p8_input_t *, int)`), et ses seuls appels dans `src/` sont `mouse.c` (32, 33) et `mouse_button.c` (34) : les appels `p8_stat(…, 20)` de `menu.c` et `p8_stat(…, 19)` de `slipway.c` ont disparu avec l'audio.
- **H-1-2** — `p8_stat` rend toujours 0 pour un index autre que 32/33/34 et pour une entrée sans trame courante ; `test_stat_unsupported_index_returns_zero` est toujours dans `tests/test_input.c`. (Si l'étape 1 a réduit `p8_stat` aux seuls 32/33/34 ou retiré ce test, la ligne correspondante de l'inventaire tombe.)
- **H-1-3** — Le drapeau souris `0x5f2d` est parti : `mouse_construct` et son `p8_poke`, les asserts `0x5f2d` de `tests/test_mouse.c` (l. 148-158), la mention « enables devkit mouse reporting » de `mouse.h` et les commentaires `0x5f2d` de `raylib_backend.c/.h`. L'étape 2 n'y touche pas ; s'il en reste, ce n'est pas à elle de les retirer.
- **H-1-4** — `p8_input_frame_t` garde `mouse_x`, `mouse_y`, `mouse_buttons` en tête et dans cet ordre ; le retrait de la deuxième manette (GAP-2) ne touche que le d-pad et ses commentaires `/* GAP-2 */`, pas les champs souris ni leurs clés JSON.
- **H-1-5** — `p8_input_t` reste `{frames, frame_count, current_frame}` et `dispatch_context_input()` rend toujours un `const p8_input_t *`.
- **H-1-6** — Les signatures de `mouse_update` et `mouse_button_update` sont celles de HEAD (l'étape 1 ne les touche pas).
- **H-1-7** — Les asserts `p8_stat(…, 32/33/34)` des tests sont inchangés : 16 dans `tests/test_input.c`, 6 dans `tests/test_demo.c`, aux lignes citées à quelques lignes près.
- **H-1-8** — Le commentaire d'en-tête de `p8_input_frame_t` (« feed stat(32)/stat(33)/stat(34) … O and X included ») et la doc de `raylib_backend_poll_input` (« bits 0/1/2 of stat(34)'s layout ») portent encore la mention de `stat()` ; l'étape 1 a pu les réécrire en partie (GAP-2, `0x5f2d`).
- **H-1-9** — L'étape 1 n'a ajouté aucun garde (`data-guard` ou autre) qui interdise déjà `p8_stat`, et n'a pas rallongé `make test`/`make e2e` au-delà des mesures de référence (~4,6 s, ~0,25 s).
- **H-1-10** — `docs/plan-8.md` §2 contient toujours la phrase « Le jeu ne lit la souris que via `p8_stat(32/33/34)` ».

## À revérifier une fois l'étape 1 faite

- **Implementation Decisions → Inventaire exact** : numéros de ligne, liste des appelants de `p8_stat` (H-1-1), présence de `test_stat_unsupported_index_returns_zero` (H-1-2), compte des asserts (H-1-7), état des commentaires de `input.h`, `mouse.h`, `raylib_backend.h/.c` (H-1-3, H-1-8), la phrase de `plan-8.md` (H-1-10).
- **Implementation Decisions → `p8_input_frame_t` inchangé** : forme de la trame après le retrait de GAP-2 (H-1-4, H-1-5).
- **Implementation Decisions → `mouse_button_update`** : signature de départ (H-1-6).
- **Testing Decisions → `p8/input`** : le test qui remplace « index non supporté » (H-1-2) ; **→ `test_mouse.c`** : les asserts `0x5f2d` partis (H-1-3) ; **→ Durées** : seuils mesurés avant l'étape 1, toujours valables (H-1-9).
- **Out of Scope** : que rien de l'étape 1 ne reste à faire dans les fichiers que touche l'étape 2.
