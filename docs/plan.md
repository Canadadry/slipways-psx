# Plan de développement — Slipways sur PC (raylib, portage littéral)

Objectif : reproduire *Slipways* (édition PICO-8) en C, jouable sur PC via [raylib](https://www.raylib.com/) en rendu logiciel (raylib sert uniquement de couche fenêtrage/présentation/entrée, aucune primitive raylib n'est utilisée pour dessiner).

Référence de gameplay : [`docs/design/slipways-mechanics.md`](design/slipways-mechanics.md). Idées d'amélioration hors scope : [`docs/after-v1.md`](after-v1.md) (écrit pour l'ancien plan manette PSX — à relire avec un œil critique, certaines idées n'ont plus de sens une fois qu'on porte `btn()`/la souris PICO-8 tels quels).

## Contexte et changement de cap

Le plan précédent (portage direct PSX, réécriture en C à partir de la doc de design, conservé dans l'historique git) s'est heurté à un problème méthodologique de fond : une réécriture complète est très coûteuse à prouver correcte, faute d'oracle de référence pendant tout le développement.

Nouvelle stratégie : **porter `pico8/lua.lua` (le code source réel de la cartouche, pas juste une référence de lecture) fonction par fonction vers le C, en copiant la logique plutôt qu'en réécrivant depuis la doc de design.** Cible : PC, via raylib en rendu logiciel — un environnement de développement et de test immédiat, sans toolchain croisée ni émulateur.

**PSX est mis en pause, pas abandonné** : il ne sera repris qu'une fois obtenue une version PC pleinement jouable en C. Le C porté sur PC devient alors la base du portage PSX (au lieu de repartir d'une réécriture depuis la doc de design). Aucun travail PSX n'est engagé avant ce jalon.

`game/` (prototype Love2D from-scratch : `Spot`/`Camera`/`Panel`/`Probe`) a été supprimé : il incarnait l'ancienne approche « réécriture depuis la doc de design », explicitement abandonnée au profit du portage littéral décrit ici.

## Stratégie de portage : copie fonction par fonction, pas de réécriture

- `pico8/lua.lua` est minifié (noms de fonctions/variables réduits à 1-2 lettres). Chaque fonction portée en C **garde son nom minifié d'origine** (`ft()`, `->ra`, `->qe`, etc.) et est précédée en commentaire du **code Lua source exact** de cette fonction. La comparaison ligne à ligne entre le commentaire et le C juste en dessous est le principal moyen de vérifier la fidélité du portage (cf. Stratégie de test).
- Le mécanisme d'entités/dispatch que `lua.lua` se construit lui-même (`kv:jr` — système de classes par chaîne de prototypes, `hv`/`cv`/`cg` — collections d'entités indexées par tag, `dc()`/`mv()`/`lt()`/`he()` — dispatch par nom de méthode) est **porté littéralement**, pas réécrit en C idiomatique (pas de struct-par-type-d'entité + enum + switch). Les entités C portent un sac de champs optionnels (union/petite table clé-valeur) qui reproduit la forme dynamique des tables Lua ; `dc()`, `mv()`, `lt()`, `he()` sont portées quasi verbatim par-dessus cette structure.
- Renommer en clair est explicitement écarté (perdrait la traçabilité de diff contre la source) — les seuls ajouts sont des commentaires ponctuels quand une signification a été confirmée via `slipways-mechanics.md`.

## Fidélité numérique

Les nombres PICO-8 sont en virgule fixe 16.16, pas en flottant. Le portage C **émule ce type exactement** (dépassement/troncature identiques) ainsi que les algorithmes PICO-8 de `rnd()`, `sin()`, `cos()` (convention « tours » 0-1, signe inversé pour `sin`). Objectif : une graine donnée doit produire exactement la même galaxie/le même déroulé que l'original — pas une approximation en `double`.

## Périmètre de l'API PICO-8 à mimer

Scanné directement dans `lua.lua` (pas deviné, pas construit par anticipation) :

- **Dessin/entrée** : `cls`, `pset`, `spr`, `map`, `mset`, `sget`, `circ`, `circfill`, `rect`, `rectfill`, `line`, `print`, `camera`, `palt`, `fillp`, `btn` (D-pad — utilisé nativement par `qb:ph()` pour le déplacement caméra), `stat` (souris — `stat(32/33/34)`, plus quelques index annexes), `peek`, `poke`, `memcpy` (astuce de palette et décodage `unstash`).
- **Maths/listes/chaînes** (socle de la Phase 2, arithmétique fixe + primitives dynamiques) : `sqrt`, `cos`, `sin`, `mid`, `flr`, `abs`, `rnd`, `sgn`, `band`, `bor`, `shl`, `add`, `del`, `all`, `foreach`, `sub`, `tostr`.
- **Explicitement hors périmètre** : `sfx`, `music` (son abandonné entièrement pour ce portage — deviennent des no-op dans la couche de mimétisme) ; tout le reste de l'API PICO-8 non listé ci-dessus n'est tout simplement pas appelé par `lua.lua` (`sspr`, `fget`/`fset`, `mget`, `sset`, `pal`, `cursor`, `color`, `flip`, `btnp`, `peek2/4`, `poke2/4`, `cartdata`, `dget`/`dset`, coroutines, `atan2`…) et n'est donc pas construit.

## Architecture : backend interchangeable sous une API fixe

`main.c` et toutes les fonctions portées n'appellent **jamais** raylib directement — uniquement la couche de mimétisme PICO-8 ci-dessus. Deux implémentations de cette même API existent, choisies au lien/à la compilation, sans toucher au code de jeu :

1. **Backend rendu logiciel + rejeu scripté** (Phases 1 à 4) : framebuffer 128×128/8 couleurs en mémoire, entrée lue depuis un `events.json` (position souris + boutons souris + les 4 booléens `btn()` D-pad, par frame — couvre exactement `stat(32/33/34)` et `btn(0-3)`), sortie dumpée en texte (format proche PPM, pas de PNG) pour être lisible directement par un LLM sans décodage d'image.
2. **Backend raylib interactif** (Phase 5) : fenêtre raylib réelle, entrée souris/clavier live, même framebuffer 128×128 uploadé comme texture et affiché à l'échelle.

## Stratégie de test

Une piste écartée après investigation : faire tourner `lua.lua` lui-même sous un interpréteur Lua standard comme oracle de différence (comparer chaque fonction portée à son original exécuté). Rejetée car :
- La syntaxe PICO-8 (`+=`, littéraux binaires `0b...`, `if (cond) stmt` sans `then`/`end`) est un problème mineur et mécanique (~76 occurrences, préprocesseur trivial à écrire).
- Le vrai problème : `lua.lua` embarque son propre mini-langage de description de données (`ob()`, un parseur caractère par caractère fait main) et un système de classes par chaîne de prototypes (`kv:jr`). Construire une seule entité de test réaliste exige d'avoir cette infrastructure et les singletons globaux (`dk`, `fm`) correctement amorcés — un chantier presque aussi lourd que le portage lui-même, avec le risque que des bugs dans l'oracle Lui-même invalident silencieusement la comparaison.

Stratégie retenue, à trois niveaux, alignée sur les phases :
- **Phase 2 (arithmétique/primitives dynamiques)** : tests unitaires C classiques, valeurs attendues calculées à la main (comportement documenté de PICO-8 pour `rnd`/`sin`/`cos`/débordement fixe). C'est aussi là que vivent les fonctions vraiment pures (opérations vectorielles `km`, géométrie `cl`/`ee`/`be`) — pas d'entité, pas d'état global, donc testables isolément sans le problème ci-dessus.
- **Phase 3 (portage fonction par fonction du reste)** : pas de test automatisé forcé — la comparaison entre le commentaire (source Lua exacte) et le C porté juste en dessous *est* la vérification.
- **Phase 4 (intégration)** : exécution headless pas à pas via le backend rejeu scripté (`events.json` en entrée, dump texte en sortie), lue par l'agent — invariants grossiers (nombre de planètes, crédits, pas de crash), pas de comparaison fine.
- **Phase 5** : test interactif par un humain via le backend raylib — premier moment où le jeu est réellement joué.

**Politique d'exécution** : jamais de fenêtre interactive lancée par l'agent, à aucune phase avant la 5. Seules des vérifications non-interactives sont permises (N frames puis sortie automatique, dump du framebuffer). Voir `CLAUDE.md`.

## Plan d'action

### Phase 1 — Interface PICO-8 minimale + rendu logiciel

Construire en C, sans dépendance raylib, l'interface de dessin/entrée listée ci-dessus (`cls`, `pset`, `spr`, `map`, `mset`, `sget`, `circ`, `circfill`, `rect`, `rectfill`, `line`, `print`, `camera`, `palt`, `fillp`, `btn`, `stat`, `peek`, `poke`, `memcpy`), backée par un framebuffer logiciel 128×128/8 couleurs. `main.c` charge automatiquement `sprite.png` et `map.json` (déjà extraits dans `pico8/`). Lecture d'un `events.json` de rejeu (souris + D-pad par frame). Sortie : dump texte du framebuffer (format simple, pas de PNG). Un petit programme C de démonstration exerce l'interface (quelques primitives de dessin) pour valider la boucle bout en bout avant de commencer le portage réel en Phase 3.

### Phase 2 — Arithmétique fixe & primitives dynamiques

Type fixe 16.16 (`+`, `-`, `*`, `/`, débordement identique à PICO-8), `rnd()`/`sin()`/`cos()` reproduisant les algorithmes PICO-8, opérations vectorielles (`km`), géométrie pure (`cl`/`ee`/`be`), primitives de liste (`add`/`del`/`all`/`foreach`), le mécanisme d'entités/dispatch (`kv:jr`, `hv`/`cv`/`cg`, `dc`, `mv`, `lt`, `he`) porté littéralement. Tests unitaires C.

### Phase 3 — Portage fonction par fonction

Chaque fonction de `lua.lua` restante est portée en C : nom minifié conservé, code Lua source collé en commentaire juste au-dessus, logique copiée (pas réécrite). Pas d'ordre de portage lié à une « tranche verticale » de fonctionnalités — la notion ne s'applique pas bien à une copie littérale d'un monolithe ; l'ordre suit plutôt les dépendances entre fonctions (ce qui est appelé avant ce qui appelle).

### Phase 4 — Vérification de jouabilité en mode headless

Une fois l'essentiel porté, exécution pas à pas via le backend rejeu scripté (`events.json` + dump texte), sans jamais ouvrir de fenêtre. Vérifier que le jeu tourne sans crash et que les mécaniques de base s'enchaînent (sonde → wormhole → colonisation → slipway → commerce → score).

### Phase 5 — Intégration raylib interactive

Basculer vers le backend raylib (fenêtre réelle, entrée souris/clavier live) sans toucher `main.c` ni aucune fonction portée — seul le backend change. Premier moment où un humain joue réellement et donne son retour.

## Garde-fous

- **Traçabilité** : toute fonction portée garde son nom minifié et son code Lua source en commentaire au-dessus. Une signification de champ/fonction non confirmée par `slipways-mechanics.md` reste documentée comme incertaine plutôt que devinée.
- **Pas de réécriture déguisée** : si une fonction semble « améliorable » en la réécrivant plutôt qu'en la portant, ce n'est pas fait sans discussion explicite — l'objectif du portage littéral est justement d'éviter ce piège.
- **Son hors périmètre** : `sfx`/`music` ne sont pas portés ; ce n'est pas un report à une phase ultérieure, c'est explicitement exclu de ce projet de portage.
- **Jamais de fenêtre interactive lancée par l'agent** avant la Phase 5 (voir `CLAUDE.md`).
- **PSX en pause, pas abandonné** : aucun travail PSX tant qu'une version PC n'est pas pleinement jouable en C.

## Décisions actées

- Cible active : PC via raylib, rendu logiciel uniquement (raylib = fenêtrage/présentation/entrée, jamais de dessin raylib).
- Stratégie : portage littéral fonction par fonction de `pico8/lua.lua`, pas de réécriture depuis la doc de design.
- Noms minifiés conservés + code Lua source en commentaire au-dessus de chaque fonction portée, comme méthode de traçabilité et de vérification principale (Phase 3).
- Mécanisme d'entités/dispatch (`kv:jr`, `hv`/`cv`/`cg`, `dc`/`mv`/`lt`/`he`) porté littéralement, pas réécrit en C idiomatique.
- Arithmétique fixe 16.16 et algorithmes `rnd`/`sin`/`cos` PICO-8 reproduits exactement (pas de flottant natif).
- Pas d'oracle Lua différentiel (investigué et écarté — cf. Stratégie de test) ; tests unitaires C pour l'arithmétique/les primitives pures (Phase 2), comparaison source/port pour le reste (Phase 3), rejeu headless pour l'intégration (Phase 4).
- Backend interchangeable sous une API fixe : software+rejeu JSON+dump texte (Phases 1-4) vs raylib+entrée live (Phase 5), sans jamais modifier `main.c` ni les fonctions portées.
- Son (`sfx`/`music`) explicitement hors périmètre, pas seulement différé.
- PSX en pause : repris uniquement une fois une version PC pleinement jouable en C obtenue, en repartant de ce C plutôt que de la doc de design.
- Prototype Love2D `game/` supprimé (incarnait l'approche « réécriture » abandonnée).
