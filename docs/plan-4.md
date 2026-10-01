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

- **Limites au doigt mouillé.** Pas de mesure préalable ; une limite se change en un chiffre. Seul garde-fou : la somme des zones tient dans **1,5 Mo**. Si relever une limite ferait dépasser cette somme, l'étape s'arrête et le signale à l'humain (zone, limite voulue, somme actuelle) : elle ne déplace pas le plafond et ne rogne pas une autre zone pour faire de la place. C'est ce qui reste sur PSX une fois retirés le noyau, les bibliothèques du SDK, le code du jeu compilé MIPS (~210–256 Ko mesurés), la pile et la mémoire PICO-8.
- **Limite atteinte = erreur fatale.** Le cœur appelle une fonction `pico8_fatal(message)` fournie par la plateforme. Sur PC : message nommant la zone et sa limite sur `stderr`, puis `exit`. Sur PSX : plus tard. Le jeu n'est **pas** modifié, il n'y a ni refus silencieux ni fin de partie forcée.
- **Invariant de chaque commit :** `make test-all` et `make sanitize-net` passent, et tous les `*.sha256` de `tests/e2e/` restent identiques. Retirer `malloc` ne change pas un pixel. Une limite trop basse fait échouer un e2e avec le message de la zone, ce qui suffit à la remonter.
- **Accès après la durée de vie.** Sous ASan (build de test), toute remise à zéro empoisonne la mémoire de la zone (`ASAN_POISON_MEMORY_REGION`) — la zone frame à chaque frame, la zone partie à chaque nouvelle partie —, et les zones à places empoisonnent une place rendue. Un objet utilisé hors de sa durée de vie fait planter le rejeu, au lieu de corrompre la mémoire en silence. L'oracle est `make sanitize-net` (existant, vert au départ), qui rejoue tous les e2e sous ASan+UBSan.

## 4. Classement des sites

Chaque étape classe les sites qu'elle touche dans sa zone, en suivant le tableau du §2. Un site qu'on ne sait pas classer avec certitude (objet référencé au-delà de la frame, callback qui garde un `self`) va dans la zone à durée de vie la plus longue qui le couvre, jamais dans la plus courte. Le crash ASan est l'oracle qui tranche quand la lecture ne suffit pas.

## 5. Étapes

Chacune est une PRD et un commit (ou une courte suite de commits), et respecte l'invariant du §3.

0a. **Socle — outils.** Une PRD.
   - `pico8_fatal`, avec son implémentation PC dans le rejeu et le backend raylib.
   - Un module de zone générique (zone frame à pointeur montant, zone à places de taille fixe), avec son empoisonnement ASan et ses tests unitaires. Critère de ces tests : sous ASan, un pointeur gardé d'avant une remise à zéro (frame précédente, partie précédente), ou vers une place rendue, fait encore planter **après** que la zone a servi de nouvelles allocations. Sinon la réutilisation masque le bug, et l'oracle du §4 ne tranche rien.
   - Une vérification mécanique (build ou `make test`) qui échoue si la somme des tailles de zones dépasse 1,5 Mo.
   - Le **verrou**, dans `make test` : il échoue si un objet du cœur référence `malloc`, `calloc`, `realloc` ou `free` (`nm -u` sur les objets de `CORE_SRCS`), sauf pour les fichiers d'une liste d'exceptions qui contient au départ tous les fichiers du cœur qui allouent, chacun avec son nombre d'appels d'allocation restants. Ce nombre ne peut que baisser : un cliquet, qui bloque toute réintroduction dans un fichier à moitié converti. On tranche ici le sort des fichiers d'hôte classés dans le cœur (`pico8_input.c`, `pico8_coverage.c`) : sortis de `CORE_SRCS`, ou exceptions permanentes justifiées ligne par ligne.
   - *Découvert :* `pico8_input.c` coupé — `btn`/`stat`/pas de frame restent au cœur, les E/S d'`events.json` partent dans `pico8_input_json.c` (hôte) ; `pico8_coverage.c` sort de `CORE_SRCS` : aucune exception permanente au verrou. Verrou par `objdump -r` (relocations, pas `nm -u` : il faut compter les appels), égalité stricte, liste initiale 36 fichiers / 148 appels sur 58 objets. Zones : allocateurs en `unsigned char *` (composition-guard refuse `void *`) ; sous ASan la zone montante alterne deux moitiés et la zone à places tourne son curseur — un pointeur gardé deux remises à zéro, ou un tour complet, n'est donc plus attrapé. Tailles provisoires : frame 256 Ko, interface 256 Ko, particules 128 Ko, partie 768 Ko (somme 1408 Ko / 1536). `pico8_fatal` PC sort en 3.
0b. **Socle — zones choisies par l'appelant.** Une PRD, après 0a : le verrou est déjà en place.
   - Les tableaux croissants partagés (`ents`, `ptrs`, ceux des panneaux `lq`) prennent leur capacité dans une zone désignée par leur propriétaire, et les fonctions partagées qui fabriquent textes et boîtes (`tostr`, concaténations, boîtes `vec`) dans une zone désignée par l'appelant : l'identifiant d'une planète, fait par les mêmes fonctions qu'un texte de menu, vit toute la partie. Les étapes 1 à 4 n'ont plus qu'à choisir une zone par propriétaire ou par appel ; une liste temporaire (filtrée, concaténée) va en zone frame.
   - Une zone **transitoire**, adossée à la libc, reproduit le comportement actuel (l'appelant libère encore) : c'est elle que passe tout appelant pas encore converti. Ses allocations sont comptées par le cliquet comme les autres ; les étapes 1 à 4 en retirent leurs appelants, l'étape 5 la supprime.
   - Pendant 0a et 0b, aucun site du jeu n'est encore converti.
   - *Découvert :* poignée `pico8_zone_choisie_t` (alloc/grow/release ; `pico8_zone_choisie_bump` sur une zone montante, où grow copie) et `pico8_zone_transitoire` (malloc/realloc/free). Prennent leur zone : `ents`/`ptrs` (à l'init, gardée par la liste ; `g_cg[i].list` reçoit désormais la sienne dans `pico8_entity_init`), `ents_cat`, `jp_ents`, `jp_ptrs`, `la_ptrs`, `nv_ptrs`, `sub`, `tostr_fixed`, `ne`, `qz`, `lq_new`, `lq_copy`, `jb_new` (le panneau garde la zone pour `lq_add`). Les cinq `concat2` statiques deviennent `pico8_concat2`. Les boîtes `vec` n'ont pas de fonction partagée : sites en ligne, traités par leur étape. Le verrou compte chaque référence à `pico8_zone_transitoire` : les comptes sont passés des fonctions partagées à leurs appelants, seule hausse permise du plan — **plus aucun compte ne monte après 0b**. Sortent `ents`, `ptrs`, `str`, `collect`, `geom` ; entrent `entity`, `cart`, `zone_transitoire` ; `layout` 6→2. Liste laissée : 34 fichiers, 228 appels.
1. **Zone frame.** Textes et petites boîtes. Les `free` correspondants disparaissent.
2. **Zone interface.**
3. **Zone particules.**

   Ces deux étapes créent un point de libération qui n'existe pas aujourd'hui (TROU-7 : panneaux, messages et particules fuient). Elles ne sont finies que si chaque sorte convertie rend sa place, ce qui se montre : après une session scriptée, le nombre de places vivantes revient à son niveau de départ. Dans une zone à places, un `pico8_fatal` est d'abord lu comme une place jamais rendue ; la limite n'est relevée qu'une fois ce cas écarté, et la note « Découvert » dit lequel des deux c'était.

4. **Zone partie,** et ses propres tableaux croissants ; remise à zéro sur nouvelle partie. L'étape n'est finie que si `pause_restart` passe `make sanitize-net` sous cet empoisonnement.
5. **Clôture.** La liste d'exceptions du verrou ne contient plus que les fichiers d'hôte justifiés. `TROU-7` est retiré de `docs/trous-de-portage.md` et des sources qui le citent, et l'entrée `malloc` du `README` est retirée.

Une étape de 1 à 4 est finie quand les sites de sa zone sont convertis et que le compte du cliquet a baissé d'autant dans chaque fichier concerné. Beaucoup de fichiers allouent pour plusieurs zones : ils ne sortent de la liste qu'avec la dernière.

Chaque étape se termine en ajoutant sous sa propre entrée une courte note « Découvert » : les sites classés autrement que dans le §2 (par la règle du §4 ou par un crash ASan), la zone attribuée à chaque propriétaire, les limites relevées, et les comptes du cliquet qu'elle laisse. Si un constat contredit le tableau du §2, l'étape corrige le tableau dans le même commit.

## 6. Décision actée, à ne pas rouvrir

**Pas de partie longue enregistrée, ni comme prérequis ni comme filet.** Le plan tourne sans humain. Les e2e courts suffisent : une limite trop basse se révèle en jouant, par un `pico8_fatal` qui la nomme, et on la relève. Un accès à une place rendue ou remise à zéro est le travail de l'empoisonnement ASan (§3), pas d'une session longue.

## 7. Hors périmètre

- Afficher un budget de slipways et de relais, et bloquer l'ajout quand il est plein. C'est une évolution du jeu, à faire séparément.
- Les économies de taille pour la PSX : la table `pico8_quarter_wave` (64 Ko, réductible de moitié), les chaînes d'`assert` (~29 Ko), les assets en binaire au lieu de JSON.
- Remplacer les appels libc restants du cœur (`fopen`, `snprintf`, `__divdi3`, …). C'est un chantier PSX.
