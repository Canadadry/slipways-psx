---
title: "Plan 5, sous-étape 5c — renommages de la famille N : 33 entrées qui éditent, un commit chacune, en sept morceaux ordonnés ; 6 entrées dont la proposition garde tel quel"
description: "Appliquer les 39 entrées N du registre (« ok » = proposition), chacune dans son propre commit, par la procédure N ou par la commande de l'entrée : D-44 d'abord, avec la procédure G qui suit l'exemption laissée par 5b ; puis les entrées sans AST, les petits renommages AST du bas vers le haut, et les trois grandes entrées D-21, D-29 et D-38 chacune dans son morceau. Le tableau « Restes comptés » de la clôture sert de cliquet après chaque commit, sur une extraction fraîche."
status: needs-triage
---

## Texte de l'étape (copié de `docs/plan-5.md`, §6, à `7aa8609`)

> 5. **Clôture.** Condition d'entrée : chaque entrée de `docs/decisions-plan-5.md` porte un verdict humain, pris dans l'ensemble fermé du registre (« Conventions » : `proposition`, `variante nommée`, `garder tel quel`, `reporter hors plan 5`), l'alerte 14 de D-68 comprise. Une PRD par sous-étape, dans l'ordre 5a–5f ; chacune applique les entrées de sa famille (colonne « Application » de l'index du registre) et finit par son contrôle de sortie, avec l'invariant du §5. Une entrée `garder tel quel` ou `reporter hors plan 5` n'édite rien, sauf la ligne d'exception d'un reste compté (5d).
>
> 5c. **Renommages** (famille N), un commit par entrée, par la procédure N ou la commande de l'entrée. Une entrée large dont le verdict édite a sa propre PRD ou son morceau, comme les morceaux de l'étape 4 ; sites remesurés à `1b4b949` (`grep` des conventions du registre) : D-20 248 (207 à `91f99a6` : `main_ui` 142, `main_mouse` 21, `main_mouse_typed` 31, `main_mouse_placeholder_class` 3, `main_camera_typed` 51), D-21 154 occurrences (143 à `91f99a6`), 54 déclarations, D-43 88 + 115 déclarations, D-46 4 + 163 (`main_economy` 53, `main_economy_typed` 93, `main_economy_placeholder` 12, `main_economy_placeholder_class` 5), D-30 96 ; D-38 (80 déclarations dans 40 fichiers, plus leurs emplois) est de la même taille.
>
>    **Sortie** : à chaque commit, le `grep` des sites des conventions sur chaque ancien nom de l'entrée rend 0, `make test-all` vert, `tests/e2e/*.sha256` inchangés ; le tableau « Restes comptés » de la clôture du registre sert de cliquet : sur une extraction fraîche, aucun compteur sans `SUBSTEP` ne le dépasse après un commit (un renommage qui fait d'un mot courant un ancien nom, D-36, se voit là).

Phase 4 du pilotage (tronçon 4 : étape 5). 5c est la troisième sous-étape de l'entrée 5, après 5a (famille O, close) et 5b (famille G), avant 5d (familles X et T).

**Entrées lues**, toutes au commit `7aa8609` :
- `CLAUDE.md` ;
- `docs/plan-5.md` : §1 à §5, la procédure et la méthode des sous-étapes de l'étape 3, l'entrée 5 et 5a à 5f, la note « Découvert » de 5a ;
- `docs/decisions-plan-5.md` : « Conventions » (procédure N, procédure G, portée, familles, verdicts), l'index, les 39 entrées de colonne `N`, D-36, D-42, D-44, la « Clôture (4l4) » et son tableau « Restes comptés » ;
- `docs/vocabulaire.md`, `docs/renommage.tsv` (en-tête, lignes des noms en cause), `tools/rename_substeps.tsv`, `tools/rename_overrides.tsv`, la légende du `Makefile` ;
- `tools/rename_count.py` (classe `Names`), `tools/rename_derive.py` (`read_overrides`), `tools/ast_rename/plan.py` et `tools/rename_check.py` (mots-clés) ;
- la PRD de 5a (`git show 4bb53ec`) et celle de 5b (`git show 7601b2b:docs/prd/triage/phase4-etape5b-gardes-a-motifs.md`, retirée de `triage/` par `f439a01`), pour la forme.

**Aucun humain n'était joignable** pendant la rédaction. Là où `to-prd` demande de vérifier avec l'utilisateur (modules, tests, découpage), l'agent rédacteur a tranché seul. Chaque décision est écrite au § « Décisions prises sans humain », pour être relue au prochain point d'arrêt.

**5b n'est pas faite au moment de la rédaction.** Tout ce que cette PRD suppose de son issue est écrit en hypothèse `H-5b-k` (§ « Hypothèses sur 5b »), avec les sections à relire quand la note « Découvert » de 5b existera.

**Relu contre la note « Découvert » de 5b (`7601b2b`)**, et contre le code à `f439a01` sur un clone jetable (5b a touché le `Makefile`, `tools/composition_residue.sh`, `tools/data_residue.sh`, leurs deux tests et `docs/plan-5.md`, rien d'autre ; `f439a01` ne fait que retirer sa PRD de `triage/`) :
- **H-5b-1** — tient. 5b est close, sa note est sous l'entrée 5b. Ses relevés de fin, sur une extraction fraîche sans `warning:` (4 `partial`), redonnent exactement le tableau « Restes comptés » : `cart-names-count` 0/0/1/0, `french-count` 36, `cart-citations` 6/2/7, `cryptic-locals` 12, `scope-count` 12. Le morceau 0 de 5c les remesure.
- **H-5b-2** — tient, avec numéros corrigés. L'exemption `planet_inspection_panel_ctx_t.unmet_needs` est intacte, à la ligne 123 de `tools/composition_residue.sh` (`REGISTRIES_EXEMPT`), et son commentaire à la ligne 111 (145 et 133 à `7aa8609`). Les deux lignes `chaîne` du TSV sont intactes. La note relève sa vivacité : retirée seule, elle fait sortir `planet_display.c:88`. Le tableau D-44 est corrigé.
- **H-5b-3** — tient. Les violations plantées sont `planet_t`, `pointer_list_t`, `emitter_of`, `self->parent`, `planet_get_pos`, `planet_draw(void *self)`, `g_planet_spec`, `planet_default_fixed`, `field_truthy`… Dans les deux tests, à `f439a01`, aucun ancien nom d'une entrée N, aucun nom neuf de 5c, et aucun texte apparié par un contrôle de 5c (`g_…_class|kind`, `typed_main_`, `probe_own`, statiques `mem`/`gfx`, `received_needs`, `targeting_own`, `…_class_def`, `planet_kinds`, `checked_main_`, `REACHABILITY_LIST`, `mouse_world`, `mouse_screen`, `screen_pos`). Il reste trois littéraux de fixture, présents avant 5b, qui contiennent `me`, `gfx` et `mem` : `entity_` `state_of(me)` dans `test_composition_residue.c`, et `spr_default(gfx, mem, …)` et `draw_bar_default(gfx, mem, …)` dans `test_data_residue.c`. Ce sont des chaînes qui ne sont pas exactement le nom, à des sites que le TSV ne liste pas : le compteur ne les voit pas, et D-29 et D-38 ne les touchent pas.
- **H-5b-4** — tient. `ATT` a toujours 7 sites, puisque la nouvelle liste d'atteinte de 5b écrit `DIR "/reachability.txt"` sans passer par `ATT`. Le tableau D-58 vaut à `f439a01`.
- **H-5b-5** — tient. Ni `docs/renommage.tsv`, ni `tools/rename_overrides.tsv`, ni `tools/rename_substeps.tsv`, ni `tools/french_exceptions.txt`, ni les outils du dictionnaire, ni `src/`, ni les tests des entrées N n'ont bougé (`git diff --stat 7aa8609 f439a01`). Seules les listes de garde ont changé : `COMPOSITION_RESIDUE_GUARD` (10 catégories) et `DATA_RESIDUE_GUARD` (4).
- **H-5b-6** — tient. `--detail` et `--fields` sont égaux à l'octet à la mesure de départ de 5b. Les lignes `void-ptr-r2` restent à leur place, imprimées après `header-accessors`. Le morceau 0 de 5c relève ce `--detail` comme « avant » de D-44.
- **H-5b-7** — tient. Aucune convention de marquage, et le registre n'est pas édité (`docs/decisions-plan-5.md` est inchangé de `7aa8609` à `f439a01`).
- **H-5b-8** — tient. Les tests de vivacité plantent dans un mini-arbre et lisent la variable de garde du `Makefile`. Aucun ne cite `REGISTRIES_EXEMPT` ni l'exemption `unmet_needs` (0 mention dans les deux tests). D-44 ne touche donc à aucun test de 5b.
- **H-5b-9** — tient. `tools/french_exceptions.txt` est inchangé, et `french-count` rend toujours 36.

**Mesures faites pour cette PRD** sur un clone jetable au commit `7aa8609`, dans le scratchpad de l'agent, supprimé ensuite. Aucune commande git d'écriture n'a été lancée sur le dépôt. Les bindings libclang et le vendor raylib étaient des liens vers ceux du dépôt.
- `make rename-extract` : 2 min 11 s, aucune ligne `warning:`, 4 lignes `partial`.
- Compteurs sans `SUBSTEP` : exactement le tableau « Restes comptés ». `cart-names-count` `TOTAL` 0/0/1/0 ; `french-count` `string` 36 ; `cart-citations` 6/2/7 ; `cryptic-locals` 12 ; `scope-count` 12 (`static`).
- Sites : le `grep` des conventions du registre (`grep -rowI` avec les exclusions des outils du dictionnaire), sur chaque ancien et chaque nouveau nom ; identités et portées lues dans `tmp/ast_rename/declarations.tsv`.

## Condition d'entrée

- Les commits de 5b sont poussés sur la branche du chantier et sa note « Découvert » est présente sous l'entrée 5b du plan (H-5b-1).
- La sortie de 5b tient : sur une extraction fraîche sans `warning:`, les compteurs sans `SUBSTEP` rendent le tableau « Restes comptés » de la clôture, pas un de plus.
- **Lecture des verdicts.** Celle que 5a a consignée : « ok » vaut `proposition`. 5c applique donc la **proposition prête** de chaque entrée N, telle qu'elle est écrite, compléments compris (« Complément (4x) »). Une proposition qui « garde » n'édite rien. Les lignes `Verdict` du registre ne sont pas éditées.
- 5c ne rouvre aucune entrée. Quand la lettre d'une proposition ne peut pas s'appliquer telle quelle (mesure périmée, contrôle faux, sens « à confirmer »), la PRD tranche, le dit au § « Décisions prises sans humain », et la note « Découvert » le répète.

## Problem Statement

Le registre tient 39 entrées de nommage (famille N), toutes avec le verdict « ok ». L'étape 3 et l'étape 4 ont appliqué tels quels les noms en cause. Il en reste qui mentent :
- `probe_own` porte un `targeting_t` ;
- `rest_count` est une position ;
- `structure_icon_sprite_counter` compte des identifiants ;
- `unmet_needs` tient les besoins reçus.

D'autres bégaient (`data_init_data_shutdown`), gardent un préfixe ou un digramme du cart (`PICO8_CLASS_COVERAGE`, `pico8_allocs.c`, `c_truthy`, `k_live_frames`), ou contredisent le §4 :
- `g_` sur 54 tables `static const` de classe ;
- `_t` sur deux variables ;
- deux paramètres de vtable sur trois qui gardent leur digramme (`qa`, `ra`, `yr`…) ;
- 80 statiques `mem`/`gfx` des tests.

Trois risques entourent ces renommages :

1. **Un renommage peut rendre une garde aveugle ou rouge.** D-44 renomme un champ qu'une exemption de `composition-guard` cite. D-21 renomme des tables qu'un motif pourrait citer. Un fixture planté par 5b pourrait porter un nom que 5c renomme.
2. **Un renommage peut créer un ancien nom.** Une ligne `conservé` qui passe `renommé` fait de son nom courant un ancien nom du TSV. Le compteur anti-cart se met alors à le chercher partout : en texte s'il est distinctif, entre backticks, en `short` de `cart-citations` s'il fait une ou deux lettres (D-36, et la note de 5a). Le cliquet doit le voir, après chaque commit et sur une extraction fraîche.
3. **5c est large.** 33 entrées éditent, et D-21, D-29 et D-38 touchent chacune des dizaines de déclarations dans des dizaines de fichiers. L'entrée 5c demande un commit par entrée et un morceau, ou une PRD, pour chaque entrée large.

## Solution

**Une PRD, sept morceaux ordonnés.** Chaque entrée qui édite fait un commit et un seul. Chaque morceau se termine par `make test-all` vert et le cliquet, et peut être repris seul par `prd-autopilot`.

1. **D-44** d'abord, avec la procédure G : l'exemption que 5b a laissée suit le champ dans le même commit.
2. **Entrées sans AST**, par la commande de l'entrée : D-26, D-3, D-4, D-62, D-18, D-34, D-53, D-2.
3. **Petits renommages par la procédure N**, du bas vers le haut, dans l'ordre des sous-étapes 3b à 3l : D-14, D-35, D-17, D-39, D-41, D-15, D-31, D-45, D-19, D-32, D-47, D-30, D-48, D-49, D-51, D-22, D-23, D-24, D-50, D-54, D-58.
4. **D-21** : les tables de classe et de sorte.
5. **D-29** : les paramètres de méthode de la vtable.
6. **D-38** : les statiques `mem`/`gfx` des tests.
7. **Note « Découvert »** de 5c et relevés de fin.

**Six entrées n'éditent rien**, puisque leur proposition est « garder » : D-16, D-20, D-25, D-43, D-46 et D-52. Trois d'entre elles (D-20, D-43, D-46) sont citées par l'entrée 5c parmi les grandes ; leur verdict ne leur fait pourtant rien éditer.

À la fin :
- chaque ancien nom de chaque entrée qui édite rend 0 à son contrôle ;
- `make test-all` est vert et `tests/e2e/*.sha256` sont inchangés ;
- les cinq compteurs sans `SUBSTEP` ne dépassent nulle part le tableau « Restes comptés ».

## User Stories

1. En tant que lecteur du code, je veux que `probe_own`, `rest_count`, `structure_icon_sprite_counter` et le champ `unmet_needs` disent ce qu'ils tiennent, pour ne plus lire le contraire de ce qui est stocké.
2. En tant que lecteur, je veux que plus aucun identifiant, fichier de fixture, macro de compilation ou variable d'environnement ne garde un préfixe ou un digramme du cart que le registre a jugé, pour que le §2 tienne hors de la portée des compteurs.
3. En tant que lecteur, je veux que les tables `static const` de classe et de sorte ne portent plus le `g_` que le §4 réserve aux statiques mutables, pour qu'un `g_` veuille dire « état mutable ».
4. En tant que lecteur d'une méthode de la vtable, je veux que ses paramètres disent `mouse_world`, `mouse_screen`, `year`, `slipway`, `offer`, `dropped`, `parallax`, comme sa définition sœur, pour ne plus traduire un digramme en lisant.
5. En tant que lecteur des tests, je veux que les 40 statiques `mem`/`gfx` s'appellent `memory`/`graphics`, comme les locales homonymes de `test_demo.c`.
6. En tant que relecteur, je veux un commit par entrée, avec le numéro de l'entrée dans le message, pour relire une décision à la fois et pouvoir en annuler une seule.
7. En tant que relecteur, je veux que les entrées dont la proposition garde tel quel n'éditent rien, et que la note le dise, pour ne pas chercher un commit qui n'existe pas.
8. En tant que mainteneur de `composition-guard`, je veux que l'exemption `planet_inspection_panel_ctx_t.unmet_needs` passe au nom neuf dans le commit qui renomme le champ, avec `--detail` égal avant et après, pour que la garde ne rougisse pas et ne devienne pas aveugle.
9. En tant que pilote, je veux qu'après chaque commit les cinq compteurs sans `SUBSTEP`, sur une extraction fraîche, ne dépassent pas le tableau « Restes comptés », module par module, pour voir tout de suite un renommage qui crée un ancien nom.
10. En tant que pilote, je veux que le contrôle de chaque entrée soit une commande écrite, qui rend 0, pour qu'il n'y ait pas de jugement à la fin d'un commit.
11. En tant que mainteneur du pont inverse, je veux que `docs/renommage.tsv` dise ce qui a vraiment été appliqué : une ligne passe au nom neuf, l'`ancien` du cart est gardé (D-42), et une ligne de `tools/rename_overrides.tsv` fixe le nom.
12. En tant que mainteneur de `tools/rename_overrides.tsv`, je veux qu'une surcharge neuve remplace la ligne de même clé au lieu de s'y ajouter, puisque `read_overrides` refuse une clé en double.
13. En tant qu'exécutant, je veux que chaque nom neuf passe le refus de mots-clés de `rename-apply`, devenu celui de `rename_check.py` en 5a (59 mots, dont 15 neufs), pour qu'un commit ne s'arrête pas sur un refus imprévu.
14. En tant qu'exécutant, je veux connaître d'avance les entrées dont la lettre ne s'applique pas telle quelle (D-18, D-21, D-24, D-29, D-35, D-38), et la décision prise pour chacune.
15. En tant que mainteneur d'une chaîne, je veux que le commit qui tient l'écrivain traduise tous les lecteurs en même temps, pour qu'aucun test ne compare un nom neuf à un ancien.
16. En tant que mainteneur de `e2e-gap`, je veux qu'aucun renommage ne change le nombre de lignes d'un fichier de `tools/line_coverage_reference.txt`, pour que la référence reste valide sans être réécrite.
17. En tant qu'humain au prochain point d'arrêt, je veux lire dans la note les décisions prises sans humain, les écarts aux mesures du registre et ce qui reste à trancher (les autres `static const g_`, les `ra` qui ne sont pas des paramètres de méthode), pour trancher avant 5d et 5e.
18. En tant qu'exécutant de 5d, je veux que 5c ne crée aucun reste compté neuf, pour que les lignes d'exception de 5d restent celles du registre.
19. En tant qu'agent, je veux que l'appli raylib ne soit jamais lancée : `make raylib` et `make screencast-viewer` sont compilés, jamais exécutés.

## Implementation Decisions

### Règles communes à tous les commits

**Procédure N** (« Conventions » du registre), pour toute entrée qui renomme un identifiant :

1. `make rename-extract` frais. On relit l'identité de chaque ligne dans `tmp/ast_rename/declarations.tsv`. Une identité à offset (locale, paramètre, statique de fonction) se relève **à chaque commit** : chaque commit qui précède décale les offsets. Les offsets donnés par le registre ne servent que d'indice.
2. Les lignes `ancien nouveau fichier identité` vont dans `tmp/ast_rename/decision.tsv`, sous l'en-tête `ancien	nouveau	fichier	identité`, puis `make rename-apply TABLE=tmp/ast_rename/decision.tsv`. Le rapport `tmp/ast_rename/report.txt` fait foi pour le nombre d'occurrences, et ce nombre va au message du commit.
3. Dans le même commit, la ligne de `docs/renommage.tsv` est mise à jour, et une ligne de `tools/rename_overrides.tsv` fixe le nom :
   - **Ligne du TSV.** La colonne `nouveau` prend le nom neuf. Une ligne `conservé` passe `renommé`. L'`ancien` ne change jamais (D-42 (a) : pont inverse). L'`identité` d'une ligne déjà `renommé` reste celle qu'elle avait ; celle d'une ligne `conservé` qui passe `renommé` prend l'identité relevée à l'étape 1. Dans les deux cas, cette identité disparaît de l'extraction après l'application, et la partie AST du compteur la compte 0.
   - **Ligne de surcharge.** Une `name`, `field parent.nom` ou `identity`, à la clé de la ligne du TSV : l'`ancien` du cart pour une ligne déjà renommée, le nom courant pour une ligne `conservé`. Si une ligne de même `(sorte, clé)` existe, elle est **remplacée**, jamais doublée. Contrôle : les surcharges se lisent sans erreur par `rename_derive.read_overrides`.
4. Les mentions entre backticks, en commentaire et dans les chaînes sont réécrites à la main dans le même commit (« Portée d'une décision de nommage »).

**Commande de l'entrée.** Pour les entrées hors AST (un `sed`, une retouche de test, une surcharge), c'est la commande écrite dans la proposition, sans procédure N. Il n'y a ni ligne au TSV ni surcharge quand le nom n'est pas au TSV : D-2, D-3, D-4 et D-62 n'y sont pas.

**Propriété des chaînes.** Le commit qui tient l'écrivain d'une chaîne traduit tous ses lecteurs en même temps. C'est la règle de 3a (« la sous-étape qui tient l'écrivain renomme à tous ses sites »), ramenée au commit. Elle s'applique ainsi en 5c :
- D-44 : écrivain `planet_display.c` ; lecteurs : l'exemption et son commentaire dans `tools/composition_residue.sh`, et les deux lignes `chaîne` du TSV ;
- D-17 : les chaînes `CASES[]` et `expect_clean` du cas, le libellé de `check_int` ;
- D-45 : le chemin de la fixture dans `GRAB_ARENAS`, et sa mention dans `tools/rename_substeps.tsv` ;
- D-2 : le `-D` du `Makefile` ; lecteurs : les `#ifdef` de `src/` et le texte de `tools/float_grep_exclusions.txt` ;
- D-3 : `setenv` et `getenv` du même fichier ;
- D-34 et D-53 : écrivain `tools/class_coverage.sh` (`  exempt (…)`, `failed:`) ; lecteur : le test.

**Règle D-36 (b) de 5a.** Un nom ambigu dont **toutes** les lignes `renommé` (catégorie `identifiant`) sont de sorte `locale` ou `paramètre` ne compte en `backtick` que dans les `fichier` de ces lignes. Conséquences pour 5c :
- **D-38** fait passer `renommé` 80 lignes de sorte `statique` pour `mem` et `gfx`. Ces deux noms sortent de la règle : un `` `mem` `` ou un `` `gfx` `` comptera partout. Il y en a 0 dans le code à `7aa8609`, donc pas d'effet aujourd'hui, mais c'est un piège pour plus tard, que la note consigne.
- La note de 5a range D-38 parmi les « renommages de locale ou de paramètre » de 5c. Ce sont des statiques de fichier, la note se trompe sur ce point. D-43, qu'elle cite aussi, n'édite rien.
- **D-29** : `ra`, `qa`, `me`, `yr`, `sw` et `lw` ont déjà des lignes `renommé` de sorte `champ`, et comptent donc déjà partout entre backticks. `fy` et `kv` n'ont aucune ligne `renommé` : après D-29, ils ne compteront entre backticks que dans leurs fichiers.
- D-36 (b) ne s'étend pas à `short` de `cart-citations` (note de 5a). `fy`, `kv` (D-29) et `gy` (D-32) deviennent des anciens noms de deux lettres : une mention nue dans un commentaire, n'importe où, comptera en `short`. Mesuré à `7aa8609` : 0 mention nue en commentaire. Le cliquet le surveille.

**Mots-clés refusés par `rename-apply`** depuis 5a (`plan.py` importe `C_KEYWORDS` de `rename_check.py`, 59 mots). Les 15 que 5a a ajoutés sont : `alignas`, `alignof`, `bool`, `constexpr`, `false`, `nullptr`, `static_assert`, `thread_local`, `true`, `typeof`, `typeof_unqual`, `_BitInt`, `_Decimal32`, `_Decimal64`, `_Decimal128`. Aucun nom neuf de 5c n'en fait partie (vérifié sur la liste du § « Entrées »). Une variante choisie en cours de route est contrôlée contre cette liste.

**Hauteur constante.** 59 fichiers de `src/` sont dans `tools/line_coverage_reference.txt`. Aucun renommage ni aucune réécriture de commentaire n'y ajoute ni n'y retire de ligne, même si une ligne s'allonge (pas de recoupe). C'est cité pour D-47 et D-50, mais vaut aussi pour D-19, D-21, D-29, D-44 et D-24.

**Croisement des noms neufs** (mesuré à `7aa8609`) :
- aucun nom neuf n'est un ancien nom d'une ligne `renommé` du TSV, sauf `a` et `b` de D-62. Ce sont des noms de fichier de fixture (`a.c`, `b.c`) dans des littéraux. Comme noms ambigus, ils ne comptent en `string` que si le littéral est exactement le nom, à un site listé par le TSV : ce n'est pas le cas ;
- chaque nom neuf a 0 site, sauf `picker` (10 ailleurs, D-31), `is_shown` (14 : le champ homonyme, D-50), `emitter` (champ et étiquette de struct, D-51) et les noms courts de D-4 et D-62 dans d'autres fichiers. Ces noms sont dans un autre espace de noms ou un autre fichier ; `rename-apply` arbitre les masquages.

**Le cliquet.** Après chaque commit :
1. `make rename-extract` frais, sans `warning:` (4 `partial` attendues) ; cette extraction sert aussi d'étape 1 à l'entrée suivante ;
2. les cinq compteurs sans `SUBSTEP` : `cart-names-count`, `french-count`, `cart-citations`, `cryptic-locals`, `scope-count` ;
3. comparaison **module par module** avec la capture de départ (morceau 0), pas seulement sur le total. Une ligne qui monte, ou une ligne neuve, arrête le morceau. Une ligne qui baisse est nommée dans le message du commit. Aucune baisse n'est attendue : 5c ne touche aucun reste compté.

`old-names-guard`, `ast-guard` et `french-guard` (5a) donnent les mêmes tableaux avec un code. On peut s'en servir, mais la référence reste les cinq compteurs.

**Interdits.**
- Pas de `make rename-derive` ni de `make rename-strings` sur le vrai TSV : les deux réécrivent `docs/renommage.tsv`, et `rename_strings.py` y ajouterait 239 lignes (ancre `pico8_%s_struct_class`, note de 5a sur D-27). Toute preuve se fait sur une copie sous `tmp/5c/`.
- Pas de `rename-rekey` : il ne réécrit que les lignes « pas encore appliquées » des sous-étapes suivantes, et après 3l il n'y en a plus. `rename-rekey-check SUBSTEP=3l` reste dans les relevés de fin.
- Pas de `rename-table-check` comme contrôle : il est rouge par construction (D-42).

### Les 39 entrées N

Sites : `grep` des conventions du registre à `7aa8609`, mot entier, sur `src tests tools Makefile`, hors outils du dictionnaire. « Contrôle » : ce qui doit rendre 0 au commit, en plus de `make test-all` vert, de `tests/e2e/*.sha256` inchangés et du cliquet.

**Entrées qui n'éditent rien** (proposition « garder ») :

| Entrée | Proposition | Sites à `7aa8609` |
|---|---|---|
| D-16 `probe_link_target` | (a) garder ; redécoupage hors plan 5 | — |
| D-20 singletons `main_*` | (a) garder | `main_ui` 142, `main_mouse` 21, `main_mouse_typed` 31, `main_mouse_placeholder_class` 3, `main_camera_typed` 51 (248, comme à `1b4b949`) |
| D-25 `p8_input_*`, `p8_fatal` | (a) garder | — |
| D-43 paramètres `gfx`/`mem` | (a) garder, « à revoir avec D-36 et D-38 » : D-36 (b) et D-38 n'en changent pas la lettre | 88 + 115 déclarations |
| D-46 `main_economy*` dans `probe.c` | (a) ; (b) n'est qu'un complément « si l'humain veut le §4 à la lettre », et « ok » ne le demande pas | — |
| D-52 `…_published_pico8_one` | (a) garder | 2 |

**Morceau 1 — D-44, procédure N et G** :

| Entrée | Commande | Sites | Contrôle |
|---|---|---|---|
| D-44 | Procédure N : `unmet_needs	received_needs	src/game/planet_display.c	c:@SA@planet_inspection_panel_ctx_t@FI@unmet_needs`. Surcharge `field	pico8_jo_gh_ctx_t.ji	received_needs	gh`, qui remplace la ligne actuelle (`unmet_needs`). Puis procédure G : l'entrée de `REGISTRIES_EXEMPT` de `tools/composition_residue.sh` et son commentaire passent à `planet_inspection_panel_ctx_t.received_needs`, et les deux lignes `chaîne` du TSV (`symbole cité`, écrivain et lecteur) passent au nom neuf | champ : 7 lignes de `planet_display.c` (88, 107, 196, 236, 237, 240, 241) ; exemption : ligne 123 et commentaire ligne 111 à `f439a01` (145 et 133 à `7aa8609`, avant les retraits de 5b ; H-5b-2). La locale `unmet_needs` de `planet_inspection_panel` (ligne 158, portée 131) garde son nom | `grep -n 'ctx->unmet_needs\|filter_context.unmet_needs\|ctx_t.unmet_needs' src tools` → 0 ; `--detail` de `composition_residue.sh` et de `data_residue.sh` égal à l'octet avant et après ; vivacité sur le vrai arbre : avec l'exemption retirée seule sur une copie du script, `planet_display.c:88` sort ; `make composition-guard` vert |

**Morceau 2 — entrées sans AST** :

| Entrée | Commande | Sites | Contrôle |
|---|---|---|---|
| D-26 | Deux lignes `string	pz	=` et `string	go	=` dans `tools/rename_overrides.tsv` (sorte `string` lue par `read_overrides`) | 2 lignes du TSV : `go` (`tests/test_button.c`) et `pz` (`tests/test_research_project.c`), `conservé` | sur une **copie** du TSV : `rename_strings.py --table tmp/5c/d26.tsv` redonne `conservé` à ces deux lignes ; les autres différences de cette copie (ancre de D-27) ne sont pas en cause et ne sont pas reportées |
| D-3 | `sed -i 's/\bPICO8_MALLOC_FILLS\b/MALLOC_FILLS/g' tests/test_entity_header.c` | 2 | `grep -rnw PICO8_MALLOC_FILLS src tests tools Makefile` → 0 |
| D-4 | Un `sed` par paire, mot entier, dans `tests/test_alloc_guard.c` seulement : `pico8_allocs`→`allocs`, `pico8_clean`→`clean`, `pico8_gone`→`gone`, `pico8_transient_user`→`transient_user`, `pico8_zone_transitoire`→`transient_zone`, `pico8_malloc_count`→`malloc_count`, `pico8_grow`→`grow`, `pico8_half`→`half`, `pico8_kept`→`kept`, `pico8_use`→`use` | 29 lignes `pico8_` dans le fichier (33 occurrences à la fin de 4l4). `pico8_half` a aussi 2 sites dans `test_float_guard.c` (D-62) : le `grep` global ne tombe à 0 qu'après D-62 | `grep -c 'pico8_' tests/test_alloc_guard.c` → 0 ; noms neufs : 0 site avant dans ce fichier |
| D-62 | Un `sed` mot entier par nom, dans les trois fichiers seulement : `pico8_floating`→`floating`, `pico8_integer`→`integer`, `pico8_another`→`another`, `pico8_half`→`half`, `pico8_third`→`third`, `pico8_a`→`a`, `pico8_b`→`b`, `PICO8_HOST_BUILD`→`HOST_BUILD` | 7 + 2 + 1 lignes (`test_float_guard.c`, `test_constants_check.c`, `test_float_grep.c`) | `grep -c 'pico8_\|PICO8_'` des trois fichiers → 0 ; après D-4 et D-62, le `grep` des conventions sur chaque ancien nom → 0 |
| D-18 | Retirer `"main_mouse"` du tableau `tags[]` de `tests/test_main_mouse.c` | 1 | `grep -n '"main_mouse"' tests/test_main_mouse.c` → 0 ; `make test` vert. Voir la décision 1 : la boucle **dépend** de la taille du tableau |
| D-34 | `strstr(log, "exempt")` → `strstr(log, "  exempt (test)")` dans `tests/test_class_coverage.c` | 1 (ligne 69) | `make test` vert ; sur une copie, faire écrire autre chose que `test` à l'`awk` du cas rend l'assertion rouge |
| D-53 | `strstr(log, "unknown") == NULL` → `strstr(log, "failed:") == NULL` | 1 (ligne 61) | `make test` vert ; violation plantée hors du dépôt (un rejeu qui rend 3, donc une ligne `failed:`) qui la fait échouer |
| D-2 | `grep -rl PICO8_CLASS_COVERAGE src tools/float_grep_exclusions.txt Makefile \| xargs sed -i 's/PICO8_CLASS_COVERAGE/CLASS_COVERAGE_BUILD/g'` (sans `\b` : le `-D` colle la macro) | 35 : 31 dans 21 fichiers de `src/`, 2 dans `tools/float_grep_exclusions.txt`, 2 dans le `Makefile`. Les 4 de `tests/test_rename_strings.py` (outil du dictionnaire, exclu) restent | `grep -rn PICO8_CLASS_COVERAGE src tests tools Makefile` hors outils du dictionnaire → 0 ; `make class-coverage` et `make float-grep` verts ; `make rename-extract` sans `warning:`, avec les 4 `partial` de `class_coverage` |

**Morceau 3 — petits renommages, procédure N**, du bas vers le haut (sous-étapes 3b à 3l du module qui déclare) :

| Entrée | Lignes de `decision.tsv` (identité à relever) | Sites | Contrôle |
|---|---|---|---|
| D-14 | Les trois lignes du registre : `ZONE_GAME_BYTES`→`GAME_ZONE_BYTES`, `ZONE_GENERATION_BYTES`→`GENERATION_ZONE_BYTES`, `ZONE_PARTICLES_BYTES`→`PARTICLES_ZONE_BYTES` (`macro:src/engine/zone_sizes.h:…`). `ZONE_BUDGET_BYTES` est gardé | 5 + 4 + 3 ; noms neufs 0 | `grep` des conventions sur les trois anciens → 0 |
| D-35 | Le paramètre `lua` de `P8_FIXED_LIT` → `decimal`, à la main (paramètre de macro, hors extraction). Procédure N pour les trois tests du registre et, par le complément 4g, pour `cart_positions_items`, `CART_POSITIONS`, `cart_routed_positions_items`, `CART_ROUTED_POSITIONS` → `frozen_…`/`FROZEN_…` (`tests/test_research_project.c`) | 1 ; 2 + 3 + 2 ; 2 × 4 | Contrôle corrigé (décision 5) : `grep -rnoiwE '([a-z0-9]+_)*(lua\|cart)(_[a-z0-9]+)*' src/p8 tests/test_fixed.c tests/test_gfx.c tests/test_research_project.c` ne rend que `cart_draw` ; `DEFAULTS_HASH` inchangée ; `make constants-check` vert |
| D-17 | Les dix lignes du tableau du registre. `entity_list_list_size` a 6 identités (`test_trading_hub`, `test_structure`, `test_planet`, `test_processor`, `test_slipgate`, `test_infraspace_gate`) et `entity_list_pointer_list_size` 4 (`test_ascension`, `test_synthesizer`, `test_burst`, `test_protostar`), toutes vers `entity_list_count`, aucun fichier n'ayant les deux. `targeting_track_target_targeted_by` : 2 identités (`test_probe`, `test_wormhole`). Le cas : `c:test_particle_zone.c@F@case_emitter_init_help_screen_takes_a_particles_help_screen_list`. À la main dans le même commit : les deux chaînes du cas (`CASES[]` et `expect_clean`) et le libellé `"update_vanishing_update_entities_calls"` de `check_int`, qui devient `"update_vanishing_calls"` | 4, 2 (+1 libellé), 5, 8, 2 (+2 chaînes), 14, 11, 3, 3, 8 ; noms neufs 0 | `grep` des conventions sur chaque ancien nom, chaînes comprises → 0 |
| D-39 | La ligne du registre (`…decode_real_cart_content` → `…decode_real_game_data`) | 2 | `grep -rnwiE '[a-z_]*(lua\|cart)[a-z_]*' tests/test_data_init.c` → 0 |
| D-41 | La ligne du registre (`…by_their_r` → `…by_their_weight`) | 2 | `grep -rn 'by_their_r\b' src tests tools` → 0 |
| D-15 | Les quatre lignes `probe_own`→`targeting_own` du registre (identités courantes `c:@SA@planet_t@FI@probe_own`… ; au TSV, les identités restent celles de l'origine, `c:@SA@pico8_jo_t@FI@probe_own`…). Lignes du TSV `conservé` → `renommé` | 24 | `grep` des conventions sur `probe_own` → 0 (mentions nues de `probe.h`, `wormhole.h`, `targeting.h` comprises) |
| D-31 | Paramètre `oc` de `planet_set_type_picker` → `picker` : 2 identités (définition `planet.c`, prototype `planet.h`). Surcharge `field	planet_set_type_picker.oc	picker` | 3 | `grep -nw oc src/game/planet.c src/game/planet.h` → 0 |
| D-45 | Les six lignes du registre (`test_live_slots.c`). `git mv tests/fixtures/grab_arenas_refused.json tests/fixtures/grabs_refused.json` et la valeur de `GRABS`. Le module `tests/fixtures/grab_arenas_refused` de la ligne 3f de `tools/rename_substeps.tsv` et la liste de 4f2 de `docs/plan-5.md` passent au nom neuf. Ligne `fichier` au TSV avec sa surcharge `file` (décision 6) | 2 × 5 ; `GRAB_ARENAS` 6 ; fixture : 1 en code + `rename_substeps.tsv` + plan | `grep -rnw 'grab_arena\|grab_arenas\|GRAB_ARENAS\|grab_arenas_refused' tests/test_live_slots.c` ne rend que `placed->grab_arena` ; `make rename-substeps-check` vert |
| D-19 | `planet_upkeep	economy_score_planet_upkeep	src/game/economy_score.h	c:@F@planet_upkeep` | 19 (11 au registre : s'y ajoutent les appels et les commentaires des tests de 3i, `fnv(h, planet_upkeep(e))`, et les mentions nues de leurs en-têtes de commentaire) | `grep` des conventions sur `planet_upkeep` → 0 ; aucun hachage ne change (la valeur pliée, pas le nom) |
| D-32 | `rest_count`→`rest_cursor` (`c:@SA@structure_icon_archetype_t@FI@rest_count`), `shared_revealed_count`→`shared_cursor`, `revealed_count_before`→`cursor_before` (locale de `test_directed`, à relever), paramètre `gy` de `structure_icon_set_cursor` → `cursor` (2 identités). Surcharges : `field	structure_icon_set_cursor.gy	cursor`, et pour `rest_count` une ligne plus précise que le `token	eh	rest_count` existant (ligne 271), sans toucher au token (décision 7) | 11 + 8 + 4 + 3 | `grep` des conventions sur `rest_count`, `shared_revealed_count`, `revealed_count_before` → 0 ; `grep -nw gy src/game/structure_icon.[ch]` → 0. La locale `gy` de `test_restart` (`tests/test_structure_icon.c`), hors proposition, reste |
| D-47 | Les trois lignes du registre. Les surcharges remplacent les lignes 766, 768 et 769 de `tools/rename_overrides.tsv` (numéros inchangés à `7aa8609`). Le message de `structure_icon_left_press` : « sprite counter » → « id counter », à hauteur constante | 4 + 5 + 5 | `grep` des conventions sur les trois anciens → 0 ; `git grep -n 'sprite counter' src tests` → 0 |
| D-30 | `typed_main_camera`, `typed_main_economy`, `typed_main_mouse` → `checked_main_…`, une ligne par identité (`c:<fichier>.c@F@typed_main_X`, 6 identités dans `test_background`, `test_camera`, `test_ui`, `test_economy`, `test_probe`) | 24 + 68 + 4 = 96 | `grep -rn 'typed_main_' src tests tools Makefile` → 0 |
| D-48 | Les lignes du registre : deux tests de `test_ui.c`, `test_recompute_on_placeholder` × 2 fichiers, `RECOMPUTE_MOUSE_WORLD_X`/`_Y` × 2 fichiers | 2 + 2 + 4 + 4 + 4 | `grep` des conventions sur les cinq anciens → 0 |
| D-49 | Les quatre lignes du registre (`outgoing_routes_of`, `build_trading_hub`, `fold_trading_hub`, `is_trading_hub_archetype`) | 2 + 6 + 3 + 3 | `grep` des conventions sur les quatre anciens → 0 |
| D-51 | Locale `spacer` → `emitter` de `fold_ascension`, `fold_burst`, `fold_protostar`, `fold_synthesizer` (4 identités à offset, à relever). Surcharges `identity`. Au TSV, les lignes `em`→`spacer` gardent leur `ancien` (D-42) : `spacer` ne devient pas un ancien nom | 54 (55 au registre) ; 7 lectures du champ `->spacer`, qui restent | `grep -nP '(?<![>.])\bspacer\b' tests/test_ascension.c tests/test_burst.c tests/test_protostar.c tests/test_synthesizer.c` → 0 |
| D-22 | Les deux lignes du registre | 5 + 4 | `grep` des conventions sur les deux anciens → 0 |
| D-23 | La ligne du registre | 2 | `grep` des conventions sur l'ancien → 0 |
| D-24 | `DIFFICULTY_OPTIONS_ROW_CAP	HELP_DIFFICULTY_OPTIONS_ROW_CAP	src/game/help.c	macro:src/game/help.c:DIFFICULTY_OPTIONS_ROW_CAP`. `DIFFICULTY_OPTIONS_COUNT` ne suit pas (décision 4) | 3 | `grep` des conventions sur l'ancien → 0 |
| D-50 | `c_truthy`→`is_shown` (`score_panel_update`) et `s_str`→`score_text` (`score_panel_update_map`), deux identités à offset (9538 et 4936 à `7aa8609`, à relever). Surcharges `identity`. Le commentaire de `score_panel_update` garde sa hauteur | 2 + 3 | `grep` des conventions sur les deux anciens → 0 |
| D-54 | Proposition (b) : `k_multiplier`→`level_multipliers`, `k_replayed_frames`→`replayed_frames`, `k_live_frames`→`live_frames` (2 identités), `k_source_frames`→`source_frames`, `k_events_valid`→`events_valid_frames`, `k_events_valid_count`→`events_valid_count` (7 identités stables du registre) | 26 | `git grep -nwE 'k_(multiplier\|replayed_frames\|live_frames\|source_frames\|events_valid\|events_valid_count)' -- src tests` → 0 ; `k_entry`, `k_key` restent (hors entrée) |
| D-58 | `ATT	REACHABILITY_LIST	tests/test_composition_residue.c	macro:tests/test_composition_residue.c:ATT`, ligne `conservé` → `renommé`, surcharge `name	ATT	REACHABILITY_LIST` | 7 à `7aa8609`, à remesurer après 5b (H-5b-4) | `grep -cw ATT tests/test_composition_residue.c` → 0 |

**Morceau 4 — D-21** :
- Proposition (a) : `g_X_class` → `X_class_def`, `g_X_kind` → `X_kind_def`.
- **54 déclarations** `static const entity_class_t|entity_kind_t g_…` dans 25 fichiers, **53 noms**. `g_layout_row_class` est déclaré deux fois dans `layout.c` (déclaration provisoire et définition) : c'est une identité, à vérifier dans `declarations.tsv`.
- **154 occurrences** (143 au registre), surtout dans `planet_family.c` (35) et `planet_family_trade.c` (17), plus 9 dans trois tests (`test_main_camera`, `test_ui`, `test_toast`).
- Aucun nom en `_def` n'existe (0 site), et aucun nom neuf n'est en double.
- **Plus `g_planet_kind`** (`planet_family.c:682-685`) : `static const struct {…} g_planet_kind[]`, table des sortes par nom de classe, que le contrôle de l'entrée apparie (décision 2).
- Une ligne de `decision.tsv` par identité, avec les identités de statiques de fichier relevées. Les lignes du TSV gardent leur `ancien` du cart (`g_lq_struct_class`…) et prennent le nom neuf ; une surcharge `name` par ancien.
- **Contrôle** : `grep -rnE '\bg_[a-z0-9_]+_(class|kind)\b' src tests` → 0. `make class-coverage-net` donne une sortie égale à celle d'avant : le nom de classe est une chaîne, la table n'en est que le porteur.

**Morceau 5 — D-29** :
- Proposition (b), « sens **à confirmer** ».
- Règle d'application retenue (décision 3) : les paramètres déclarés dans les membres d'`entity_class_t` (`src/engine/entity_class.h`), et ceux de chaque fonction rangée dans l'un de ces membres par un désignateur (`.hit_test = f`), définition et prototype.
- 49 implémentations à `7aa8609`. 92 déclarations de paramètre de la famille : `ra` 48, `qa` 23, `yr` 5, `kv` 5, `sw` 4, `lw` 4, `fy` 2, `me` 1.

Noms donnés par la proposition et la décision 3 :

| Membre | Paramètre | Nom neuf |
|---|---|---|
| `hit_test`, `left_press`, `right_press`, `drag` | `qa`, `ra` | `mouse_world`, `mouse_screen` (comme `ui_send_to_hovered`, 4h) |
| `draw`, `draw_overlay` | `ra` | `screen_pos` (décision 3 : ce n'est pas la souris) |
| `new_year` | `yr` | `year` |
| `attach_slipway` | `sw` | `slipway` |
| `offer_trade` | `lw` | `offer` |
| `drop` | `kv` | `dropped` |
| `apply_camera` | `fy` | `parallax` |

S'ajoutent :
- **Les sœurs non-méthodes** qu'une méthode appelle avec le même sens : `camera_apply` (`fy`), `planet_offer_trade` et `slipgate_offer_trade` (`lw`), `planet_attach_slipway` (`sw`).
- **Les deux prototypes du complément 4j**, qui prennent le nom de leur définition : `score_panel_new_year(… yr)` → `year`, et `cart_start_game(button_t *nj)` → `chosen_option`.
- **`me` → `state`** sur les 12 paramètres `me`, qui tiennent tous un nom d'état (`entity_state_of`, `entity_set_state`, `entity_init_instance`, `planet_grab_arena_zone_released`, `targeting_tooltip_show`, et les assistants de test `first_planet_with_state` × 2 et `first_with_state`).

Pour chaque fonction : surcharges `field <fonction>.<param>` ; les lignes `conservé` du TSV passent `renommé`.

- **Hors règle, gardés** (§1, portée ≤ 40) et listés dans la note : `vec_from_polar` (`fy`, une longueur), `pos_copies` × 8, `build`, `build_protostar`, `with_pos`, `holds_copy_of`, `spy_draw` hors désignateur, `draw_layers`, `emitter_host_layer_pos`, `probe_host_set_positions`, `apply_camera_to`, `year_box`.
- **Repli sur un masquage** que `rename-apply` refuse : `state` → `state_name`, `screen_pos` → `draw_pos` (précédent de `button_draw`). Le repli est contrôlé contre les mots-clés et nommé dans le message du commit.
- **Contrôle**, sur une extraction fraîche : aucun `param` nommé `qa|ra|yr|sw|lw|kv|fy|me` dont le parent est une fonction de la famille, une sœur, ou un membre d'`entity_class.h`. Plus aucun paramètre `me`. Le compte de chaque nom hors famille est égal à celui d'avant, et listé.

**Morceau 6 — D-38** :
- Proposition (b) : `static p8_memory_t mem;` → `memory`, `static p8_graphics_t gfx;` → `graphics`.
- 80 déclarations dans 40 fichiers, 80 identités `c:<test>.c@mem` et `@gfx`, **523 occurrences** à `7aa8609`. Dans ces fichiers, `memory` et `graphics` ont 0 site.
- Les 97 locales, 11 paramètres et 3 statiques de fonction `mem`/`gfx` des tests restent (D-43 (a), §1).
- Une ligne de `decision.tsv` par identité ; surcharges `identity` ; les 80 lignes `statique` `conservé` du TSV passent `renommé`.
- **Contrôle** : `grep -rhE '^static p8_(memory|graphics)_t (mem|gfx);' tests | wc -l` → 0. Aucune occurrence d'une des 80 identités dans `occurrences.tsv` frais. Les mentions entre backticks ou en commentaire qui désignent la statique sont réécrites : 0 ligne de commentaire en cause mesurée, à remesurer au commit.
- Le contrôle « `grep` des conventions → 0 » de l'entrée 5c ne peut pas valoir pour `mem`/`gfx`, qui restent des noms de paramètres et de locales. Le contrôle propre de l'entrée le remplace (décision 8).

### Décisions prises sans humain

1. **D-18, la boucle dépend de la taille.** La proposition dit « vérifier d'abord que la boucle sur `tags` ne dépend pas de sa taille ». C'est faux : `registered()` (`tests/test_main_mouse.c:43-57`) parcourt `buckets[]` et `tags[]` dans une seule boucle `for (i = 0; i < 5; i++)`. Retirer `"main_mouse"` sans toucher la boucle lirait `tags[4]` hors du tableau. Décision : deux boucles, chacune bornée par `sizeof x / sizeof x[0]`, dans le même commit `test:`. Le comportement est inchangé : la recherche de l'étiquette retirée rendait toujours une liste vide. La ligne `chaîne` `je`→`main_mouse` (`tests/test_main_mouse.c`, lecteur) reste au TSV comme pont, et `rename-strings-verify SUBSTEP=3h` y verra un `unseen` de plus.
2. **D-21, la portée.** La proposition et son contrôle visent les tables de classe et de sorte. Le registre ajoute que « 7 autres `static const` préfixées `g_` » seraient couvertes par le même verdict, sans les nommer et sans leur donner de nom neuf. Décision :
   - 5c applique (a) aux 54 déclarations de classe et de sorte ;
   - 5c l'applique aussi à `g_planet_kind`, que le contrôle apparie. Le nom tiré de la règle à la lettre, `planet_kind_def`, ferait lire « la définition d'une sorte », alors que c'est une table de sortes par nom de classe : le nom retenu est `planet_kinds` (0 site), simplement sans le `g_` ;
   - les autres `static const g_…`, mesurées à `7aa8609` (`g_message_names`, `g_synthesizer_synthesizable_resources_spent`, `g_slipgate_left_press_empty`, `g_outline_offsets`, `g_galaxy_menu_on_click`, `g_galaxy_menu_refresh`, `g_handles`, `g_planet_name_grab_arena`, et dans les tests `g_without_birth`, `g_layer_ten`, `g_pos_nil`, `g_giver_layer`), ne sont pas renommées et vont dans la note pour décision humaine. Aucun nom neuf ne leur est proposé, et certains (`g_handles` → `handles`) heurteraient un nom existant.
3. **D-29, le sens « à confirmer ».** `ra` → `mouse_screen` vaut pour `hit_test`, `left_press`, `right_press` et `drag`, où `ui.c` passe la souris. Pour `draw` et `draw_overlay`, `draw_layers` passe `*qc->pos`, la position de l'entité : `mouse_screen` mentirait. Décision : `screen_pos` (au lieu de `mouse_screen`), le nom que 4e, 4f2 et 4i ont donné aux sœurs `planet_draw`, `burst_draw` et `wormhole_draw`. Pour `me` → `state` : aucun `me` n'est un paramètre de membre (le seul dans `entity_class.h` est celui d'`entity_state_of`, une fonction `static inline`). Les 12 qui existent tiennent tous un nom d'état, et la proposition nomme `me` : décision, les 12. `fy` de `vec_from_polar` est une longueur, pas une parallaxe : il est gardé.
4. **D-24, `DIFFICULTY_OPTIONS_COUNT`.** La proposition dit qu'il « suit le même verdict » s'il est « du même fichier ». La mesure dit le contraire : c'est une constante d'énumération de `src/game/game_data.h:21`, employée par `help.c` et `game_data.c`. Décision : il ne suit pas. La note le dit (une question de module `game_data`, hors D-24).
5. **D-35, le contrôle faux.** `grep -rnwiE '[a-z_]*(lua|cart)[a-z_]*'` apparie aussi `evaluated` et `evaluates` (« eva-lua-ted ») : il rend `src/p8/fixed.h:21` et `test_lit_never_evaluates_its_literal`. Décision : le contrôle découpe en mots sur `_`, `grep -rnoiwE '([a-z0-9]+_)*(lua|cart)(_[a-z0-9]+)*'`. Celui de D-39 garde sa forme, puisque `tests/test_data_init.c` ne contient pas ce faux positif (à vérifier au commit).
6. **D-45, la ligne `fichier`.** La proposition demande une « ligne `fichier` » pour la fixture. Le TSV n'a aucune ligne pour elle : seul `docs/vocabulaire.md:591` la nomme. Décision : une ligne `tests/fixtures/grab_arenas_refused.json	tests/fixtures/grabs_refused.json	tests/test_live_slots.c	fichier	fixture	renommé	tests/test_live_slots …`, avec sa surcharge `file`, comme pour les binaires de 3l. On ne vérifie pas qu'une régénération la reproduit, puisque `rename-strings` n'est pas relancé (D-27). `docs/vocabulaire.md` est le dictionnaire : il n'est pas édité.
7. **D-32, `eh`.** `rest_count` vient du `token	eh	rest_count` (ligne 271 de `tools/rename_overrides.tsv`), qui nomme aussi d'autres dérivés. Décision : une surcharge plus précise qu'un token, `field` ou `identity` sur `pico8_ie_def_t.eh` (l'identité d'origine de la ligne du TSV), et le token reste. L'exécutant vérifie, dans `rename_derive.py`, que cette sorte l'emporte sur `token`.
8. **Contrôle de sortie là où le nom vit ailleurs.** Le `grep` des conventions sur l'ancien nom ne peut pas rendre 0 pour `mem`/`gfx` (D-38), `ra`/`qa`/`fy`… (D-29), `oc` (D-31), `gy` (D-32), `unmet_needs` (D-44, locale gardée), `spacer` (D-51, champ gardé), ni pour `pico8_half` entre D-4 et D-62 : ces noms restent pour un autre objet, ou le même nom appartient à deux entrées. Décision : pour ces entrées, le contrôle est celui de l'entrée, restreint (colonne « Contrôle »). La sortie de l'entrée 5c est lue ainsi, et la note le dit.
9. **Découpage.** Une PRD avec des morceaux ordonnés, plutôt qu'une PRD-cadre et des PRD filles. Justification :
   - **un seul cadre.** Toutes les entrées partagent la procédure N, le cliquet, la capture de départ et les hypothèses sur 5b. Des PRD filles les recopieraient, et le cliquet, qui est un seul tableau, pourrait diverger d'une copie à l'autre ;
   - **la taille vient des occurrences, pas du jugement.** Les trois grandes entrées passent par `rename-apply`, qui réécrit les occurrences à partir de l'AST. Ce qui demande du jugement tient dans leur décision (2, 3, 8), et leurs morceaux sont isolés pour que `prd-autopilot` puisse s'arrêter et reprendre à leur frontière ;
   - **l'étape 4 n'est pas comparable.** Ses morceaux avaient chacun leur PRD parce que chaque module demandait une lecture par un agent.
10. **Ordre.**
    - **D-44 d'abord** : la mesure de vivacité de 5b est fraîche, et l'exemption laissée par 5b est le seul lien entre les deux sous-étapes.
    - **Ensuite les entrées sans AST**, dont `src/` n'a que D-2 : elles ne dépendent d'aucune identité.
    - **Puis les petits renommages, du bas vers le haut** : un nom de socle renommé avant ses usages du jeu, dans l'ordre des sous-étapes qui déclarent.
    - **Les trois grandes entrées à la fin**, chacune seule. D-38 est la dernière : elle sort `mem`/`gfx` de la règle D-36 (b), et rien après elle ne doit écrire ces noms entre backticks.
11. **Registre et vocabulaire.** Ni les lignes `Verdict` du registre ni `docs/vocabulaire.md` ne sont édités, comme en 5a. Il n'y a pas de marque « appliqué » (H-5b-7).

## Implementation Chunks

Chaque commit a pour message `refactor:` (un renommage dans `src/`) ou `test:` (une fixture, une assertion, un nom de test), en français, avec le numéro de l'entrée, le nombre d'occurrences du rapport de `rename-apply`, la liste des sites réécrits à la main, et les lignes d'attribution de la session. Chacun garde `make test-all` vert et `tests/e2e/*.sha256` inchangés. Il est suivi du cliquet.

0. **Mesure de départ**, sans commit, dans `tmp/5c_baseline/`, sur l'arbre de fin de 5b :
   - `make rename-extract` (sans `warning:`, 4 `partial`) ;
   - les cinq compteurs sans `SUBSTEP` ;
   - `--detail` et `--fields` de `composition_residue.sh`, `--detail` de `data_residue.sh` ;
   - le `grep` des conventions sur chaque ancien et chaque nouveau nom du § « Les 39 entrées N » ;
   - le croisement des noms neufs contre les anciens noms des lignes `renommé` du TSV ;
   - les sorties de `class-coverage-net` et `e2e-gap`.

   Un écart aux valeurs de `7aa8609` données ici va dans la note, et le tableau correspondant est remesuré avant d'ouvrir le morceau qu'il touche.
1. **D-44**, un commit `refactor:`. Procédure N, puis procédure G dans le même commit. Rouge joué d'abord : le champ renommé seul (`rename-apply` sans traduire l'exemption) fait sortir `make composition-guard` sur `planet_display.c:88`. Puis l'exemption, son commentaire et les deux lignes `chaîne` du TSV passent au nom neuf, et la garde est verte. `--detail` est égal à l'octet.
2. **Entrées sans AST**, huit commits dans l'ordre D-26, D-3, D-4, D-62, D-18, D-34, D-53, D-2. D-34 et D-53 jouent leur violation hors du dépôt. Pour D-2, `make class-coverage` et `make float-grep`.
3. **Petits renommages**, 21 commits dans l'ordre du tableau du morceau 3.
4. **D-21**, un commit.
5. **D-29**, un commit. Le relevé de la famille (fonctions, membres, sœurs, paramètres gardés) va au message.
6. **D-38**, un commit.
7. **Note « Découvert »** de 5c, un commit `docs:` sous l'entrée 5c de `docs/plan-5.md`. Elle contient :
   - la lecture de « ok » ;
   - la liste des 33 commits et des 6 entrées sans édition ;
   - les décisions 1 à 11 ;
   - les écarts aux mesures du registre (D-19 19 et non 11, D-21 154 et non 143, D-32 `rest_count` 11, D-51 54, D-4 29 lignes) ;
   - ce qui reste à trancher : les `static const g_` hors D-21, les paramètres courts hors famille de D-29, `DIFFICULTY_OPTIONS_COUNT` ;
   - le piège de D-38 sur D-36 (b) ;
   - les anciens noms de deux lettres créés (`fy`, `kv`, `gy`) ;
   - tout écart à une hypothèse `H-5b-k` ;
   - les relevés de fin.

   Si un constat contredit le §4, le §5 ou l'entrée 5c, le plan est corrigé dans le même commit.

**Total : 34 commits** (1 + 8 + 21 + 1 + 1 + 1 + 1).

## Testing Decisions

- **Le filet de 5c est le compilateur, puis les tests.** Une occurrence oubliée par `rename-apply` casse le build. Une chaîne oubliée casse le test qui la lit, puisque l'écrivain et les lecteurs passent ensemble. Les e2e prouvent que rien ne bouge à l'écran : renommer ne change pas un pixel.
- **Les assertions retouchées le sont en rouge d'abord** (D-34, D-53) : la violation est jouée hors du dépôt, sur une copie. Un test qui ne peut pas échouer ne compte pas comme une garde (alerte 14 de D-68).
- **Pas de test neuf de fichier**, pas de cible neuve : 5c n'ajoute aucun comportement. Les tests modifiés gardent leur forme (C99, `assert`, `system()` sur un script pour les tests de garde).
- **Les gardes qui lisent des noms** (`composition-guard`, `data-guard`, `class-coverage-net`, `float-grep`, `constants-check`) se relancent au commit qui touche ce qu'elles lisent : D-44, D-2, D-21, D-35.
- **Prior art** : les commits de l'étape 3 (`rename-apply` puis chaînes, l'écrivain d'abord) ; ceux de l'étape 4 qui ont renommé des locales (`spacer` → `emitter` en 4f2 et 4j, `pos` → `screen_pos` en 4e, 4f2 et 4i) ; la vivacité des exemptions traduites en 3c, 3e, 3f et 3g (exemption retirée seule sur le vrai arbre) ; la capture avant/après de 5a.

**Relevés de fin** (sortie de 5c et invariant du §5) :
- `make test-all` vert, `tests/e2e/*.sha256` inchangés ;
- `make raylib` et `make screencast-viewer` compilent et linkent, jamais lancés ;
- `class-coverage-net` et `e2e-gap` : sorties égales à celles du morceau 0, aux numéros de ligne du `Makefile` près ;
- `rename-tool-test`, les `rename-*-test`, `scope-count-test`, `french-count-test` et `cart-citations-test` verts ;
- `rename-substeps-check` vert ; `rename-rekey-check SUBSTEP=3l` : 0 introuvable ;
- `rename-strings-verify SUBSTEP=3h` : seul le `unseen` de D-18 en plus ;
- après `rename-extract` frais sans `warning:` (4 `partial`), les cinq compteurs sans `SUBSTEP` rendent exactement le tableau « Restes comptés », module par module ;
- `old-names-guard`, `ast-guard` et `french-guard` sont rouges de ces seuls restes, avec les codes de 5a.

## Out of Scope

- Toute entrée hors famille N : les lignes d'exception (D-1, D-13, D-33, D-37, D-55, D-56, D-59, D-60, D-61, 5d), `TROU` → `GAP` (D-64, 5d), le blocage des gardes (5e), les docs et skills (D-65, 5f), les gardes à motifs (5b) hors l'exemption de D-44.
- Les entrées N dont la proposition garde tel quel : D-16, D-20, D-25, D-43, D-46 (et son complément (b)), D-52.
- Les `static const g_…` hors classes et sortes (sauf `g_planet_kind`) ; les paramètres courts hors famille de D-29 ; `DIFFICULTY_OPTIONS_COUNT` ; `k_entry` et `k_key` (hors D-54) ; la locale `gy` de `test_restart`. Tous vont dans la note.
- `docs/vocabulaire.md`, les lignes `Verdict` du registre, les mentions des anciens noms dans `docs/` hors de la liste de 4f2 du plan (D-45). Les mentions dans les autres docs relèvent de 5f.
- Les fixtures arbitraires de `tests/test_composition_residue.c` et `tests/test_data_residue.c` (D-57, D-63, 5b) ; les 4 mentions de `PICO8_CLASS_COVERAGE` de `tests/test_rename_strings.py`, outil du dictionnaire.
- L'ancre `pico8_%s_struct_class` de `rename_strings.py` (note de 5a) ; étendre D-36 (b) à `short`.
- Lancer `rename-derive` ou `rename-strings` sur le vrai TSV, `rename-tool-setup`, `e2e-baseline`, ou l'appli raylib.

## Hypothèses sur 5b

Écrites à `7aa8609`, 5b non faite, d'après sa PRD (`git show 7601b2b:docs/prd/triage/phase4-etape5b-gardes-a-motifs.md`). Chacune se vérifie sur la note « Découvert » de 5b et sur l'arbre de sa fin. Une hypothèse fausse fait relire les sections nommées avant d'ouvrir 5c.

- **H-5b-1** — 5b est poussée et sa note « Découvert » existe. Sa sortie tient, et elle ne change aucun compteur : sur une extraction fraîche sans `warning:`, les cinq compteurs sans `SUBSTEP` rendent le tableau « Restes comptés », à l'identique. *À relire* : Condition d'entrée ; Mesure de départ (morceau 0) ; Le cliquet ; Relevés de fin.
- **H-5b-2** — 5b n'édite pas l'exemption `planet_inspection_panel_ctx_t.unmet_needs` de `REGISTRIES_EXEMPT`, ni son commentaire, ni les deux lignes `chaîne` du TSV qui la citent. Sa note relève la vivacité de cette exemption sur le vrai arbre (retirée seule, elle fait sortir `planet_display.c:88`). Les numéros de ligne du script changent (145 et 133 à `7aa8609`) : on les relit à la fin de 5b ; relus : 123 et 111 à `f439a01`. *À relire* : Morceau 1 (tableau D-44) ; Implementation Chunks, point 1 ; Propriété des chaînes.
- **H-5b-3** — Les violations plantées et les fixtures que 5b écrit (`tests/test_composition_residue.c`, `tests/test_data_residue.c`) n'emploient :
  - aucun ancien nom d'une entrée N (sa PRD, user story 4, le promet) ;
  - aucun **nom neuf** de 5c : `received_needs`, `targeting_own`, `planet_kinds`, un `…_class_def` ou `…_kind_def`, `checked_main_…`, `mouse_world`, `mouse_screen`, `screen_pos` comme paramètre, `memory` ou `graphics` comme statique, `REACHABILITY_LIST`… ;
  - aucun texte qu'un contrôle de 5c apparie : `\bg_[a-z0-9_]+_(class|kind)\b` dans `tests` (D-21), `typed_main_` (D-30), `probe_own` (D-15), `^static p8_(memory|graphics)_t (mem|gfx);` (D-38), `\bATT\b` hors de la macro de D-58.

  Sinon le nom neuf n'aurait plus 0 site, ou le contrôle de l'entrée ne rendrait plus 0. Un fixture `static const entity_class_t g_planet_class` planté pour `parent` ou `void-self`, par exemple, ferait rougir le contrôle de D-21. *À relire* : Croisement des noms neufs ; Morceaux 3 (D-15, D-30, D-58), 4 et 6 ; Morceau 0 (croisement).
- **H-5b-4** — La macro `ATT` de `tests/test_composition_residue.c` survit à 5b, avec son rôle (la liste d'atteinte de fixture) et un nombre de sites que 5b peut changer (7 à `7aa8609`). Sa PRD la met hors périmètre (« la macro `ATT` (D-58, 5c) »). *À relire* : Morceau 3, D-58.
- **H-5b-5** — 5b ne touche que `tools/composition_residue.sh`, `tools/data_residue.sh`, leurs deux tests et `COMPOSITION_RESIDUE_GUARD` et `DATA_RESIDUE_GUARD` du `Makefile`. Elle ne touche ni `docs/renommage.tsv`, ni `tools/rename_overrides.tsv`, ni `tools/rename_substeps.tsv`, ni les outils du dictionnaire, ni les compteurs, ni aucun fichier qu'une entrée N édite (`tests/test_alloc_guard.c`, `test_float_guard.c`, `test_constants_check.c`, `test_float_grep.c`, `test_class_coverage.c`, `test_main_mouse.c`, `src/`). *À relire* : Morceau 2 ; Règles communes (procédure N, point 3) ; Morceau 0.
- **H-5b-6** — Le `--detail` des deux scripts à la fin de 5b (lignes `void-ptr-r2` imprimées après `header-accessors`, égal à l'octet à celui de `7aa8609` selon la note) est l'« avant » de la procédure G de D-44, et D-44 le laisse égal à l'octet : l'exemption est renommée avec le champ, donc `planet_display.c:88` reste exempté. Si 5b a déplacé la place des lignes `void-ptr-r2`, c'est son `--detail` de fin qui fait foi, pas celui de `7aa8609`. *À relire* : Morceau 1 ; Implementation Chunks, point 1.
- **H-5b-7** — 5b ne fixe aucune convention pour marquer une entrée appliquée dans le registre (comme 5a, H-5a-6). Si elle en fixe une, 5c la suit pour ses 39 entrées dans le commit `docs:` final. *À relire* : Décision 11 ; Implementation Chunks, point 7.
- **H-5b-8** — Les tests de vivacité de 5b lisent la liste de garde du `Makefile` et plantent leurs violations dans un mini-arbre, pas dans le vrai `src/`. Aucun d'eux n'éprouve l'exemption `unmet_needs` par son nom : sinon, D-44 devrait les traduire dans son commit, et le § « Propriété des chaînes » s'étendrait à ce test. *À relire* : Morceau 1 ; Propriété des chaînes.
- **H-5b-9** — 5b ne crée aucune ligne neuve dans `tools/french_exceptions.txt` et n'ajoute aucun mot français dans les tests de garde. Le reste `french-count` (36) est celui que 5c reçoit. *À relire* : Le cliquet.

## Further Notes

- **Ce que l'entrée 5c appelle « grandes » et ce qui édite.** D-20 (248 sites), D-43 (88 + 115 déclarations) et D-46 (4 + 163) sont citées pour leur taille, mais leur proposition garde : rien n'est édité. Les grandes entrées qui éditent sont D-21 (154 occurrences), D-29 (92 déclarations de la famille, plus les sœurs et `me`), D-38 (523 occurrences) et, plus petite, D-30 (96). D-30 tient en un commit simple de six identités : elle reste dans le morceau 3.
- **Coût.** Une extraction fraîche par commit (≈ 2 min 11 s ici), soit ≈ 75 min d'extraction pour les 34 commits, plus `make test-all` (≈ 30 s) chaque fois. L'extraction d'après un commit sert d'étape 1 au suivant.
- **Mesures du registre périmées.** D-19 (`planet_upkeep` 19, dont 8 appels et mentions dans les tests de 3i, `fnv(h, planet_upkeep(e))`), D-21 (154), D-4 (29 lignes), D-51 (54), D-32 (`rest_count` 11). Les numéros de ligne de `tools/rename_overrides.tsv` cités par D-47 (766, 768, 769) sont encore justes à `7aa8609`, mais un commit antérieur de 5c qui ajoute une ligne avant eux les décale : on retrouve la ligne par sa clé, jamais par son numéro.
- **Identités du TSV.** Pour une ligne déjà `renommé`, `docs/renommage.tsv` garde l'identité d'origine (USR du cart : `c:@SA@pico8_jo_t@FI@probe_own`), pas l'identité courante de `declarations.tsv` (`c:@SA@planet_t@FI@probe_own`). La procédure N lit l'identité courante pour `rename-apply`, et la ligne du TSV se retrouve par son `ancien`, son `fichier` et son identité d'origine.
