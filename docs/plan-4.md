# Plan 4 — Zones mémoire fixes à la place de `malloc`

Ce document est **le plan** ; aucune implémentation n'est engagée par sa rédaction. Exécution prévue par `plan-pilot`, une PRD par étape (§5).

## 1. Objectif

**Zéro `malloc`, `calloc`, `realloc`, `free` dans le cœur** (`CORE_SRCS`). Toute la mémoire du jeu vit dans des zones de taille fixe, connue à la compilation. But : la reprise PSX, dont la RAM est bornée.

Les outils d'hôte (tests, `main*.c`, backend raylib, rejeu, `pico8_assets.c`, `pico8_dump.c`) gardent la libc : ce sont eux qui prouvent que le cœur n'a rien cassé.

## 2. Les zones

| Zone | Ce qu'elle contient | Durée de vie |
|---|---|---|
| **Frame** | textes fabriqués (concaténations, `tostr`, suffixes), petites boîtes (`vec`, listes enveloppées) | remise à zéro au début de chaque frame, jamais de `free` |
| **Interface** | panneaux `lq`, messages `iv`/`mh`, menus, boutons | nombre maximum d'objets vivants ; une place est rendue quand l'objet quitte l'écran |
| **Particules** | `g_:mm`, `qy`, éclats `ga` | nombre maximum de particules vivantes ; place rendue à leur mort |
| **Partie** | planètes, relais, slipways, routes `trade`, recherche, trous de ver ; les tableaux croissants (`ents`, `ptrs`, registres) qui les contiennent | taille fixe ; vidée entièrement à chaque nouvelle partie |

Les tableaux croissants ne sont pas réécrits : ils gardent leur forme et prennent leur capacité dans leur zone au lieu de `realloc`.

## 3. Règles communes

- **Limites au doigt mouillé.** Pas de mesure préalable ; une limite se change en un chiffre. Seul garde-fou : la somme des zones tient dans **1,5 Mo**. C'est ce qui reste sur PSX une fois retirés le noyau, les bibliothèques du SDK, le code du jeu compilé MIPS (~210–256 Ko mesurés), la pile et la mémoire PICO-8.
- **Limite atteinte = erreur fatale.** Le cœur appelle une fonction `pico8_fatal(message)` fournie par la plateforme. Sur PC : message nommant la zone et sa limite sur `stderr`, puis `exit`. Sur PSX : plus tard. Le jeu n'est **pas** modifié, il n'y a ni refus silencieux ni fin de partie forcée.
- **Invariant de chaque commit :** `make test-all` passe, et tous les `*.sha256` de `tests/e2e/` restent identiques. Retirer `malloc` ne change pas un pixel. Une limite trop basse fait échouer un e2e avec le message de la zone, ce qui suffit à la remonter.
- **Accès après la durée de vie.** Sous ASan (build de test), la zone frame empoisonne sa mémoire à chaque remise à zéro (`ASAN_POISON_MEMORY_REGION`), et les zones à places empoisonnent une place rendue. Un objet utilisé hors de sa durée de vie fait planter le rejeu, au lieu de corrompre la mémoire en silence. Les e2e tournent sous ce build au moins une fois par étape.

## 4. Classement des sites

Chaque étape classe les sites qu'elle touche dans sa zone, en suivant le tableau du §2. Un site qu'on ne sait pas classer avec certitude (objet référencé au-delà de la frame, callback qui garde un `self`) va dans la zone à durée de vie la plus longue qui le couvre, jamais dans la plus courte. Le crash ASan est l'oracle qui tranche quand la lecture ne suffit pas.

## 5. Étapes

Chacune est une PRD et un commit (ou une courte suite de commits), et respecte l'invariant du §3.

0. **Socle.** `pico8_fatal`, avec son implémentation PC dans le rejeu et le backend raylib. Un module de zone générique (zone frame à pointeur montant, zone à places de taille fixe), avec son empoisonnement ASan et ses tests unitaires. Une cible qui lance les e2e sous ASan. Aucun site du jeu n'est encore converti.
1. **Zone frame.** Textes et petites boîtes. Les `free` correspondants disparaissent.
2. **Zone interface.**
3. **Zone particules.**
4. **Zone partie,** tableaux croissants compris ; remise à zéro sur nouvelle partie (`pause_restart` le couvre).
5. **Verrou.** Une cible de `make test` échoue si un objet du cœur référence `malloc`, `calloc`, `realloc` ou `free` (`nm -u` sur les objets de `CORE_SRCS`). Les fichiers d'hôte encore classés dans le cœur (`pico8_input.c`, `pico8_coverage.c`) sont soit sortis de `CORE_SRCS`, soit listés dans une liste d'exclusion, avec une justification par ligne. `TROU-7` est retiré de `docs/trous-de-portage.md` et des sources qui le citent, et l'entrée `malloc` du `README` est retirée.

## 6. Hors périmètre

- Afficher un budget de slipways et de relais, et bloquer l'ajout quand il est plein. C'est une évolution du jeu, à faire séparément.
- Les économies de taille pour la PSX : la table `pico8_quarter_wave` (64 Ko, réductible de moitié), les chaînes d'`assert` (~29 Ko), les assets en binaire au lieu de JSON.
- Remplacer les appels libc restants du cœur (`fopen`, `snprintf`, `__divdi3`, …). C'est un chantier PSX.
