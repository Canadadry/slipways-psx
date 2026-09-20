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
- **Maths/listes/chaînes** (socle de la Phase 2, arithmétique fixe + primitives dynamiques — toutes explicitement affectées à la Phase 2 dans le détail de cette phase ci-dessous) : `sqrt`, `cos`, `sin`, `mid`, `flr`, `abs`, `rnd`, `sgn`, `band`, `bor`, `shl`, `add`, `del`, `all`, `foreach`, `sub`, `tostr`.
- **Explicitement hors périmètre** : `sfx`, `music` (son abandonné entièrement pour ce portage — deviennent des no-op dans la couche de mimétisme) ; tout le reste de l'API PICO-8 non listé ci-dessus n'est tout simplement pas appelé par `lua.lua` (`sspr`, `fget`/`fset`, `mget`, `sset`, `pal`, `cursor`, `color`, `flip`, `btnp`, `peek2/4`, `poke2/4`, `cartdata`, `dget`/`dset`, coroutines, `atan2`…) et n'est donc pas construit.

## Architecture : backend interchangeable sous une API fixe

`main.c` et toutes les fonctions portées n'appellent **jamais** raylib directement — uniquement la couche de mimétisme PICO-8 ci-dessus. Deux implémentations de cette même API existent, choisies au lien/à la compilation, sans toucher au code de jeu :

1. **Backend rendu logiciel + rejeu scripté** (Phases 1 à 4) : framebuffer 128×128/8 couleurs en mémoire, entrée lue depuis un `events.json` (position souris + boutons souris + les 4 booléens `btn()` D-pad, par frame — couvre exactement `stat(32/33/34)` et `btn(0-3)`), sortie dumpée en texte (format proche PPM, pas de PNG) pour être lisible directement par un LLM sans décodage d'image.
2. **Backend raylib interactif** (Phase 5) : fenêtre raylib réelle, entrée souris/clavier live, même framebuffer 128×128 uploadé comme texture et affiché à l'échelle. Inclut un **mode enregistrement** : chaque frame de la session, l'entrée live (souris + D-pad) est écrite dans un `events.json` au même format que celui rejoué par le Backend 1 — ce que l'humain vient de faire redevient un rejeu scripté que l'agent peut réexécuter et inspecter en headless, sans jamais avoir vu la session se dérouler.

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

Construire en C, sans dépendance raylib, l'interface de dessin/entrée listée ci-dessus (`cls`, `pset`, `spr`, `map`, `mset`, `sget`, `circ`, `circfill`, `rect`, `rectfill`, `line`, `print`, `camera`, `palt`, `fillp`, `btn`, `stat`, `peek`, `poke`, `memcpy`), backée par un framebuffer logiciel 128×128/8 couleurs. `main.c` charge automatiquement `sprite.png` et `map.json` (déjà extraits dans `pico8/`). Lecture d'un `events.json` de rejeu (souris + D-pad par frame). Sortie : dump texte du framebuffer (format simple, pas de PNG). Un petit programme C de démonstration exerce l'interface pour valider la boucle bout en bout avant de commencer le portage réel en Phase 3 : au moins un appel par catégorie (pixel `pset`, forme `line`/`rect`/`circ`, sprite `spr`, carte `map`, texte `print`), une lecture `btn`+`stat`, une frame pilotée par `events.json`, et un dump texte du framebuffer — le tout branché sur la cible `make test` que cette phase ajoute. `make test` fait des assertions précises sur le résultat, pas juste « le programme tourne » : après chaque appel de démonstration, les octets du framebuffer (ou les valeurs lues via `peek`/`memcpy`) attendus à des positions connues sont comparés à des valeurs de référence calculées à la main.

### Phase 2 — Arithmétique fixe & primitives dynamiques

Type fixe 16.16 (`+`, `-`, `*`, `/`, débordement identique à PICO-8), `rnd()`/`sin()`/`cos()`/`sqrt()`/`mid()`/`flr()`/`abs()`/`sgn()`/`band()`/`bor()`/`shl()` reproduisant les algorithmes PICO-8, opérations vectorielles (`km`), géométrie pure (`cl`/`ee`/`be`), primitives de liste (`add`/`del`/`all`/`foreach`) et de chaîne (`sub`/`tostr`), le mini-langage de description de données (`ob()`/`fi()`) et le mécanisme d'entités/dispatch qui en dépend directement (`kv:jr` appelle `ob()` dès sa première ligne — `hv`/`cv`/`cg`, `dc`, `mv`, `lt`, `he`), tous portés littéralement. Tests unitaires C pour les fonctions pures (arithmétique fixe, `rnd`/`sin`/`cos`/`sqrt`/`mid`/`flr`/`abs`/`sgn`/`band`/`bor`/`shl`, `km`, `cl`/`ee`/`be`, `add`/`del`/`all`/`foreach`, `sub`/`tostr`) ; pour `ob()`/`fi()`/`kv:jr`/`hv`/`cv`/`cg`/`dc`/`mv`/`lt`/`he`, pas de test unitaire fonction par fonction — vérifiés par comparaison source/port comme en Phase 3 (cf. Stratégie de test), plus un test de fumée dédié : définir 1-2 types d'entités factices via `ob()`/`kv:jr` et exercer `dc`/`mv`/`lt`/`he` dessus, branché sur la même cible `make test` que la Phase 1. Le test de fumée fait des assertions précises, pas juste « ça tourne » : `dc()`/`mv()` appellent bien la méthode attendue sur l'entité factice, et `lt()`/`he()` l'ajoutent/la retirent effectivement des collections `hv`/`cv`/`cg` concernées. Sortie de phase : ce test de fumée passe (avec ces assertions), en plus des tests unitaires ci-dessus.

### Phase 3 — Portage fonction par fonction

Chaque fonction de `lua.lua` restante est portée en C : nom minifié conservé, code Lua source collé en commentaire juste au-dessus, logique copiée (pas réécrite). Pas d'ordre de portage lié à une « tranche verticale » de fonctionnalités — la notion ne s'applique pas bien à une copie littérale d'un monolithe ; l'ordre suit plutôt les dépendances entre fonctions (ce qui est appelé avant ce qui appelle).

Une liste de suivi (portée / restante), groupée par vague de dépendances, est tenue à jour pendant toute la phase dans [`docs/portage-progress.md`](portage-progress.md) (fichier dédié plutôt qu'une section de ce plan : ~140 fonctions à suivre, ça alourdirait ce document). Checkpoint après chaque lot porté (un lot = une vague de dépendances du tracker ci-dessus) : le projet compile et `make test` passe toujours dans son ensemble (démonstration de la Phase 1, tests unitaires et test de fumée de la Phase 2) — cette phase porte des fonctions qui s'appuient aussi bien sur les primitives de dessin/mémoire de la Phase 1 (`camera`, `spr`, `map`, l'astuce de palette via `memcpy`/`peek`/`poke`...) que sur le mécanisme d'entités/dispatch de la Phase 2, donc la plus susceptible de régresser l'un ou l'autre. Chaque lot inclut aussi une vérification bon marché contre la duplication d'aide interne entre modules : avant d'ajouter une fonction utilitaire privée, grepper les modules `src/` existants pour une équivalente déjà écrite (constat Phase 1 : `read_whole_file` réinventée indépendamment dans deux fichiers, cf. Notes de portage). C'est le seul contrôle intermédiaire, pas de test automatisé par fonction (cf. Stratégie de test). Sortie de phase (déclenche la Phase 4) : toute fonction atteignable depuis `_init`, `_update60` ou `_draw` est portée.

### Phase 4 — Vérification de jouabilité en mode headless

Une fois la Phase 3 terminée (toute fonction atteignable depuis `_init`/`_update60`/`_draw` portée), exécution pas à pas via le backend rejeu scripté (`events.json` + dump texte), sans jamais ouvrir de fenêtre. Vérifier que le jeu tourne sans crash et que les mécaniques de base s'enchaînent (sonde → wormhole → colonisation → slipway → commerce → score).

Sortie de phase (déclenche la Phase 5) : un rejeu scripté complet couvrant cette chaîne (sonde → wormhole → colonisation → slipway → commerce → score) tourne sans crash jusqu'au bout, avec à chaque étape : le nombre de planètes qui reste dans une plage plausible (jamais négatif, jamais au-delà de 90 — pas de valeur aberrante signe d'un bug de comptage), et les crédits qui restent dans un ordre de grandeur cohérent avec une partie normale (les crédits peuvent légitimement devenir négatifs en jeu — cf. `hy:hk()`, `self.nz+=self.ge` sans plancher — donc l'invariant n'est pas « jamais négatif » mais l'absence de saut brutal type débordement/valeur aberrante).

### Phase 5 — Intégration raylib interactive

Basculer vers le backend raylib (fenêtre réelle, entrée souris/clavier live) sans toucher `main.c` ni aucune fonction portée — seul le backend change. Premier moment où un humain joue réellement et donne son retour.

Le backend raylib enregistre toujours la session en `events.json` (mode enregistrement, cf. Architecture ci-dessus) — l'humain n'a rien à activer. Si l'humain rapporte un bug ou un comportement suspect, l'agent n'a pas besoin qu'on le lui décrive : il reprend directement l'`events.json` de la session concernée et le rejoue en headless via le Backend 1 (mêmes outils que la Phase 4) pour reproduire et investiguer, sans jamais avoir besoin d'ouvrir de fenêtre lui-même.

Avant de faire appel à un humain : l'agent compile et lie le backend raylib (compilation/édition de liens uniquement, jamais d'exécution — cf. politique d'exécution) et s'assure que ça réussit. Ce n'est pas la sortie de phase, juste un contrôle préalable bon marché pour ne pas gâcher le cycle humain, coûteux et difficile à répéter, sur une erreur de lien ou une signature incompatible avec l'API de mimétisme.

Sortie de phase (fin du portage) : un humain termine une partie complète de la boucle centrale (sonde → wormhole → colonisation → slipway → commerce → score) via le backend raylib sans crash, et confirme que les mécaniques correspondent à sa compréhension de la cartouche PICO-8 originale.

## Garde-fous

- **Traçabilité** : toute fonction portée garde son nom minifié et son code Lua source en commentaire au-dessus. Une signification de champ/fonction non confirmée par `slipways-mechanics.md` reste documentée comme incertaine plutôt que devinée.
- **Pas de réécriture déguisée** : si une fonction semble « améliorable » en la réécrivant plutôt qu'en la portant, ce n'est pas fait sans discussion explicite — l'objectif du portage littéral est justement d'éviter ce piège.
- **Son hors périmètre** : `sfx`/`music` ne sont pas portés ; ce n'est pas un report à une phase ultérieure, c'est explicitement exclu de ce projet de portage.
- **Jamais de fenêtre interactive lancée par l'agent** avant la Phase 5 (voir `CLAUDE.md`).
- **PSX en pause, pas abandonné** : aucun travail PSX tant qu'une version PC n'est pas pleinement jouable en C.

## Notes de portage

Journal à compléter au fil des Phases 1 à 5 (contrairement à « Décisions actées » ci-dessous, figée à l'issue du cadrage initial) : chaque résolution d'un cas ambigu rencontré pendant le développement (ex. sémantique exacte d'un index `stat()`, arrondi/débordement d'un cas limite en virgule fixe, ou un chemin insuffisamment exercé par le rejeu scripté de la Phase 4 et découvert seulement pendant le test humain de la Phase 5 — pas seulement les cas rencontrés en Phase 3) est consignée en une ligne — fonction/zone concernée, problème, résolution retenue — pour qu'une décision prise une fois ne soit pas relitigée ou tranchée différemment plus tard sur un cas similaire. Quand un cas est découvert en Phase 5 parce qu'il n'était pas couvert par le rejeu scripté de la Phase 4, le corpus `events.json` de la Phase 4 est étendu pour l'exercer désormais — concrètement, en réutilisant (éventuellement raccourci) l'`events.json` enregistré par le mode enregistrement du backend raylib pendant la session où le cas a été découvert, plutôt qu'en écrivant un nouveau scénario à la main — la consignation dans ce journal documente la décision, mais ne remplace pas la protection par le test de non-régression headless existant.

Deuxième catégorie d'entrées, distincte des ambiguïtés sémantiques ci-dessus : les constats *structurels* remontés par une revue `warn-review` en fin de phase (module qui grossit disproportionnellement, aide/fonction dupliquée entre fichiers, convention de commentaires incohérente entre modules...). Chaque constat est consigné en une ligne — ce qui a été observé, et la décision retenue (corrigé maintenant / reporté / explicitement accepté comme ne valant pas une convention) — pour la même raison que ci-dessus : ne pas rejouer la même discussion à chaque future revue, alors que la Phase 3 distribue le reste du portage sur beaucoup plus de fonctions et d'agents indépendants (sans mémoire partagée entre eux) que la Phase 1.

- `pico8_gfx`/`pico8_input` (Phase 1, fin de phase) : `warn-review` a relevé `pico8_gfx.c` disproportionnellement plus gros que les autres modules (regroupe primitives de dessin, sprite/carte et table de police), `read_whole_file` dupliqué indépendamment dans `pico8_assets.c` et `pico8_input.c`, et une convention de commentaires incohérente (`pico8_input.h` très commenté, les autres headers pas du tout). Décision : pas de correction rétroactive sur la Phase 1 (hors scope de ce constat), mais deux mesures pour la Phase 3 ci-dessous (checkpoint de vague + colonne « module cible » du tracker) pour rendre ce type de dérive visible plus tôt et moins coûteux à corriger.

- `pico8_gfx`/`print` (Phase 1, chunk 6) : aucune donnée de la police PICO-8 réelle n'existe dans les assets `pico8/` extraits (police intégrée au runtime, pas à la cart) → table de glyphs bitmap 3x5 (avance de 4px/caractère) inventée à la main, pas une reproduction fidèle ; ensemble couvert = espace, chiffres 0-9, minuscules a-z, ponctuation `! $ % ' ( ) + , - . / : = ?` (dérivé des littéraux de chaîne de `pico8/lua.lua` réellement atteignables depuis `print()`/`io()`) ; tout caractère hors de cet ensemble (ex. majuscules) dessine un bloc plein 3x5 (glyph "placeholder"), délibérément distinct de tout glyph réel et de l'espace, plutôt qu'une case vide silencieuse.

- `pico8_fixed`/`pico8_fixed_div` (Phase 2, chunk 1) : bug UB réel trouvé dans `pico8_fixed_div` — le numérateur `(int64_t)a << PICO8_FIXED_SHIFT` faisait un décalage à gauche d'une valeur signée potentiellement négative, undefined behavior en C (C11 6.5.7p4, plus strict que le décalage à droite d'une valeur négative qui n'est qu'implementation-defined), reproduit sous UBSan (GCC 13 et Clang 18, `-O0`/`-O2`) ; le résultat numérique était correct par hasard sous le codegen x86 actuel, donc aucun test existant ne l'attrapait. Cause structurelle : la discipline « calculer via le type non signé correspondant, puis réinterpréter » appliquée par `pico8_fixed_add`/`sub`/`neg` dans le même fichier n'avait pas été appliquée de façon cohérente à `pico8_fixed_div`. Décision : correction alignée sur le même motif (décalage en `uint64_t`, cast vers `int64_t` avant la division), plus ajout d'une cible Makefile dédiée (`tmp/test_pico8_fixed_ubsan`, build du même binaire de test avec `-fsanitize=undefined -fno-sanitize-recover=undefined`, exécutée dans `unit`/`test`) comme garde-fou de suivi pour ce module — pertinent pour tout futur chunk de Phase 2/3 qui touche à de l'arithmétique bit-à-bit.

## Décisions actées

- Cible active : PC via raylib, rendu logiciel uniquement (raylib = fenêtrage/présentation/entrée, jamais de dessin raylib).
- Stratégie : portage littéral fonction par fonction de `pico8/lua.lua`, pas de réécriture depuis la doc de design.
- Noms minifiés conservés + code Lua source en commentaire au-dessus de chaque fonction portée, comme méthode de traçabilité et de vérification principale (Phase 3).
- Mécanisme d'entités/dispatch (`kv:jr`, `hv`/`cv`/`cg`, `dc`/`mv`/`lt`/`he`) porté littéralement, pas réécrit en C idiomatique.
- Arithmétique fixe 16.16 et algorithmes `rnd`/`sin`/`cos` PICO-8 reproduits exactement (pas de flottant natif).
- Pas d'oracle Lua différentiel (investigué et écarté — cf. Stratégie de test) ; tests unitaires C pour l'arithmétique/les primitives pures (Phase 2), comparaison source/port pour le reste (Phase 3), rejeu headless pour l'intégration (Phase 4).
- Backend interchangeable sous une API fixe : software+rejeu JSON+dump texte (Phases 1-4) vs raylib+entrée live (Phase 5), sans jamais modifier `main.c` ni les fonctions portées.
- Le backend raylib enregistre systématiquement chaque session en `events.json` (mode enregistrement) : toute session humaine est rejouable et inspectable en headless par l'agent sans description manuelle du bug.
- Son (`sfx`/`music`) explicitement hors périmètre, pas seulement différé.
- PSX en pause : repris uniquement une fois une version PC pleinement jouable en C obtenue, en repartant de ce C plutôt que de la doc de design.
- Prototype Love2D `game/` supprimé (incarnait l'approche « réécriture » abandonnée).
