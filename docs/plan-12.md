# Plan 12 — Passer l'écran en 4:3, 160×120

Brouillon, à passer en `plan-converge` avant toute exécution. Issu d'une séance grill-me.

## 1. Objectif

Le jeu dessine un framebuffer de 160×120 au lieu de 128×128 : du 4:3 exact, qui tient ×2 dans les 320×240 de la PSX (128 lignes ×2 = 256 n'y tiennent pas). Une partie se joue de bout en bout sur PC à cette taille, et tous les rejeux de `tests/e2e/` et de `worst-case/` sont reconvertis, puis vérifiés contre un oracle figé avant le changement.

## 2. Règles

- **L'interface s'ancre à son bord le plus proche.** Un élément calé à gauche ou en haut ne bouge pas ; un élément calé à droite prend +32 en x ; un élément calé en bas prend −8 en y ; un élément centré passe du centre (64,64) au centre (80,60). Toute position d'écran s'écrit depuis `P8_SCREEN_WIDTH` / `P8_SCREEN_HEIGHT`, sans 127, 128 ou 64 en dur. Les 128 qui sont une échelle du monde (`galaxy.c`, `probe.c`, `slipway.c`, `planet_display.c`, `wormhole.c`) se trient au cas par cas et ne bougent pas s'ils ne parlent pas de l'écran.
- **La carte montre davantage.** Au début de la partie, la caméra garde le même centre monde : son coin haut-gauche passe de (192,192) à (176,196). `visible` suit le cadre 160×120, sans imiter l'ancien.
- **Pas de trou de portage pour l'affichage.** Une différence n'entre dans `docs/trous-de-portage.md` que si les règles du jeu changent ou si un bug apparaît ; un comportement « à l'écran » qui change parce que le cadre est plus large n'y entre pas.
- **L'oracle est figé avant le changement**, sur le code actuel :
  - l'état normalisé de la dernière frame de chaque rejeu, c'est-à-dire la sortie de `make state` sans la ligne `camera` ni les champs `screen` et `visible` ;
  - la couverture de lignes (gcov) de l'ensemble des e2e.
- **Les `*.sha256` ne se refont qu'à la fin**, par `make e2e-baseline`, une fois l'oracle vérifié. Entre-temps, `make e2e` est rouge ; `make test` passe à chaque commit.
- Les outils de conversion et de mesure sont temporaires : ils sont supprimés à la dernière étape. Ils ne vont ni dans `make test` ni dans `make e2e`.

## 3. Étapes

1. **Figer l'oracle.**
   - Restaurer depuis `c38875e^` la mesure de couverture de lignes (`tests/test_line_coverage.c` et ce dont elle dépend), sous une cible hors de `make test-all`.
   - Écrire, pour les 26 e2e et les 5 rejeux de `worst-case/`, l'état normalisé de la dernière frame dans un répertoire versionné le temps du plan.
   - Écrire la couverture de lignes de l'ensemble des e2e.
2. **Étiqueter les rejeux sur le code actuel.** Un devtool rejoue un `events.json` et écrit, pour chaque frame, ce qui est sous le curseur : la classe de l'entité que donne le hit test existant, ou « carte ». Une table classe → ancre (gauche, droite, haut, bas, centre ; carte) accompagne cet outil. On produit un fichier d'étiquettes par rejeu.
2bis. **Affiner l'étiquette `button`.** Dans un `git worktree` sur le dernier commit dont `make e2e` est vert à 128×128 (le chunk 1 de l'étape 3), `cursor_label.c` donne à chaque bouton une étiquette dont l'ancre est unique, tirée des champs du bouton (`layer` < 10 → `button:monde`, `technology` → `button:technologies`, `on_click` du menu pause, de la boîte de difficulté, de la fin de partie) ; `button` nu n'est plus une étiquette admise, et `make cursor-labels` échoue s'il en sort un. On régénère les 31 fichiers d'étiquettes, on vérifie qu'en ramenant toute étiquette `button:*` hors menu galaxie à `button` on retrouve exactement les fichiers de l'étape 2, puis on reporte `cursor_label.c`, `cursor_anchors.txt` et les étiquettes sur la branche principale. Dans le même worktree, on relève pour chaque rejeu les frames où le coin haut-gauche de la caméra vaut x ≤ −48 ou y ≥ 316 (zone où le décalage +16/−4 sera écrêté à 160×120) ; la liste est versionnée à côté des étiquettes et lue par l'étape 5.
3. **Passer le framebuffer à 160×120.**
   - Mettre `P8_SCREEN_WIDTH` à 160 et `P8_SCREEN_HEIGHT` à 120.
   - Mettre à la même taille le `pixel_buffer` du backend raylib, la conversion fenêtre → écran, les formats de `devtools/` (en-tête du dump, screencast et ses 16 384 pixels en dur) et la grille de tuiles du fond étoilé (20×15 au lieu de 16×16).
   - Garder `RAYLIB_BACKEND_SCALE` à 6, pour une fenêtre de 960×720.
   - Placer la caméra de départ en (176,196).
   - `make test` passe : les tests unitaires qui figeaient 128×128 sont mis à jour.
4. **Ancrer l'interface.** Il faut ancrer :
   - la barre du haut et celle du bas (y 0–10 et 117–127 aujourd'hui) ;
   - les boutons calés à droite ;
   - les infobulles (`tooltip_draw_at(…, 127, 1)`) ;
   - les panneaux, menus et boîtes de message centrés sur x = 64 ;
   - l'écran de score final : tout son contenu (fonds, lignes, boutons) est centré et prend (+16, −4), alors que les barres de son cadre (`score_panel_frame` {126,127}, `hud_draw_bar` jusqu'à 127) sont ancrées en bas ;
   - l'écran titre.
   Cet ancrage passe par les tables (`game_data.c`, `map_tables.c`) comme par le code.
5. **Convertir les rejeux.** Un script lit chaque `events.json` et son fichier d'étiquettes, puis applique à chaque frame le décalage de l'ancre (+16 / −4 pour la carte). Le `.txt` de chaque e2e reprend ensuite les nouvelles coordonnées. Le devtool de l'étape 2, relancé sur le nouveau code avec les rejeux convertis, doit rendre les mêmes étiquettes frame par frame : la première frame qui diverge désigne l'ancre fausse, qu'on corrige dans la table avant de reconvertir — à condition qu'une seule ligne de la table l'explique. Une divergence qu'aucune correction de ligne n'explique (étiquette d'ancres multiples, caméra en butée −64…320 dans l'ancien ou le nouveau cadre) arrête la boucle pour ce rejeu : il est noté avec sa frame et sa cause, puis traité à l'étape 6 comme un e2e court réécrit, ou, pour un `full_game*`, remonté à l'humain.
6. **Vérifier contre l'oracle.**
   - Chaque rejeu converti finit dans l'état normalisé figé à l'étape 1, sans `p8_fatal`. Les lignes `zone` et `cap` comptent ce qui est dessiné et bougent avec le cadre : un écart limité à ces lignes est attendu, on vérifie seulement qu'elles restent sous leur limite. Un `full_game*` ne sert qu'à remplir les zones mémoire : il « échoue » seulement s'il s'arrête ou dépasse une limite `zone`/`cap`. Son état final n'est pas comparé : le cart tire `rnd` en dessinant une planète « vanishing », et le cadre plus large décale ce tirage (décision humaine).
   - La couverture de lignes des e2e ne baisse pas ; les lignes réécrites aux étapes 3 et 4 sont hors comparaison.
   - Un e2e court qui échoue, ou dont la prémisse dépend du hors-écran (`colony_income_offscreen`, tout cas GAP-26), est réécrit à partir de son `.txt`. Il est alors jugé sur la couverture, et `make state` vérifie à la frame clé que la planète visée a bien `visible 0`.
   - Si un rejeu `full_game*` échoue, on s'arrête et on revient à l'humain avant toute autre action.
7. **Rebaseliner et nettoyer.**
   - Lancer `make e2e-baseline` ; `make test-all` et `make full-game` passent.
   - Supprimer les outils temporaires, l'oracle figé et les fichiers d'étiquettes.
   - Mettre à jour le `README` (« framebuffer 128×128 ») et les commentaires qui nomment 128×128.

## 4. Critère de fin

`make test-all` et `make full-game` passent à 160×120, et l'oracle de l'étape 1 a été tenu ou ses écarts réglés un par un. L'humain relit les captures des e2e purement visuels (`empty_planet_fade`, `project_tooltip_inputs`, et ceux dont l'état ne dit rien du rendu) et joue une partie dans raylib.
