# Catalogue des scénarios e2e

**Statut : à valider par l'humain.** Aucun fichier d'événements n'est écrit
avant cette validation (étape 9 de `docs/prd/triage/e2e-rapide.md`).

Chaque scénario deviendra `tests/e2e/<nom>.json`, accompagné de `<nom>.txt`
(ce qu'il fait, frames des actions clés) et de `<nom>.json.sha256`. Tous
partent de l'écran titre et doivent tenir en 180 frames après minimisation
(600 en exception listée dans `tools/e2e_long_exceptions.txt`), et ajouter au
moins une ligne aux autres e2e (`make e2e-judge`).

Sources : `pico8/lua.lua`, tables décodées de `pico8/map.json` (`nb`, `fd`,
`dr`), `docs/glossaire-noms.md`, les TU et fixtures actuelles, et des mesures
de couverture faites le 2026-09-27 (détail en fin de document).

## Ce que disent les mesures

- **Écart actuel : toute la référence**, 6 372 lignes (`make e2e-gap`,
  `tests/e2e/` vide).
- **Les 11 rejeux courts du filet couvrent déjà 5 883 de ces lignes (92 %)**,
  alors qu'ils partent du raccourci de difficulté. Les trois longs
  enregistrements et `trading_hub_session_events` n'apportent que
  **489 lignes** en plus. Les nouveaux e2e doivent donc d'abord réenregistrer
  ces 11 rejeux depuis le titre, puis aller chercher ces 489 lignes.
- Où sont ces 489 lignes : les structures spéciales (184 : `mx:kf`
  synthétiseur, `nr:*` slipgate/infragate, `nl:ly` hub, `pm:pq` protostar,
  `hj:kj` ascension), le commerce (`jo:cn` 38, `jo:gh` avec partenaires 24,
  `dh` paliers 17, `db` 8), l'écran titre (49, tous couverts par un simple
  clic de difficulté depuis le titre), le code de triche (17), le glisser de
  caméra au clic droit (`bg:rdown`/`bg:hq`, 25), le D-pad en butée (`qb:ph`,
  5), la bascule de l'année par le temps (`hy:nx`), « -no money- »
  (`hy:lu`), l'ouverture du panneau de score par `ix[3]`, la recette qui
  change le type de planète (`jo:hu`, branche `go`), une recette améliorée
  (`hp:po` branche `mc`, `hp:od` ×1,5), une sonde qui survole un slipway
  (`bv`).
- **`src/main.c` : la référence est périmée.** Elle a été figée (41fc09f)
  avant que le pilote exige son argument (f0b253b), qui a modifié `main.c`.
  24 lignes `main.c:N` de la référence ne correspondent plus au fichier
  actuel, et 23 lignes couvertes aujourd'hui n'y figurent pas. Aucun
  scénario ne les fermera (question 5).
- **Lignes hors référence.** Certaines rangées ne sont jamais passées dans
  les longues sessions : « toggle sfx » et « how to play? » du menu pause
  (`ha[3]`, `ha[4]`), le rabais `nanomaterials` (`hp:od` ×0,85). Un
  scénario qui les couvre n'aide pas à fermer l'écart, mais le critère de
  garde le garde quand même, puisqu'il ajoute des lignes.
- **Rejeux hors limite** (`docs/e2e-rejeux-hors-limite.md`) : aucun
  n'entre. `trading_hub_session_events` tombe à 4 172 frames après
  minimisation, soit sept fois 600 ; la passe guidée n'a pas été lancée sur
  les trois longs enregistrements (des heures d'oracle pour un résultat
  toujours au-delà de 600 frames). Leurs lignes font partie des 489
  ci-dessus.

### Coûts qui fondent les estimations

Estimations **après minimisation**, sur les fixtures actuelles, à vérifier
en headless :

| action | frames |
|---|---|
| clic de difficulté au titre | ~2 (un appui dès la frame 0 est pris, mesuré) |
| clic sur un bouton ou une rangée | 2–3 |
| sauter une année (`ix[1]`) | 2 ; 25 sauts finissent la partie |
| sonde `mz` : tenue ≥ 15 ticks + front de révélation | ~40 |
| menu de construction (`jo:fe`) : glissement + clic | ~10 |
| inventer (`ix[2]`, glissement, clic sur la rangée) | ~15 |
| glisser un slipway | ~8 |
| code de triche O O X X O O X X | ~16 |

**Le mur de la recherche.** Sans laboratoire, on gagne 1 point de recherche
par année et une partie dure 25 années : 25 points au plus. Coûts
(`nb`, `gi`, sans labo) : ère 1 = 4/5/6/7 ; ère 2 = 10/12/14/16/18 ;
ère 3 = 19/22/25/28/31 ; ère 4 = 34/38/42/46/50. On passe d'ère après
2, 4 et 7 inventions (`ek`). Sans labo ni triche, on atteint donc l'ère 1,
et en ère 2 `xenofoods` (19 ans), `slipgates` (21) et `geoharvesting` (23).
`trade league` (25 points) arrive trop tard, et tout ce qui est au-delà
aussi. Il faut sinon un réseau de labos approvisionnés (des centaines de
frames et de la planification), ou le code de triche (`fm.jk=99`,
`fm.nz=200`, rejouable), qui débloque les ères 1 à 3 en une fois. La
référence contient les lignes de `apply()` : les longues sessions s'en
servaient. Le PRD exclut la triche « pour sauter du temps de jeu »
(question 1).

**Le tirage.** La galaxie est tirée au clic de difficulté, et les modules
d'affichage consomment `rnd()` à chaque frame du titre : changer le préfixe
change toute la carte. Règle appliquée ci-dessous, conforme au PRD :
- **agent** quand la cible est une interface fixe ou « n'importe quelle
  planète » (lue sur le dump) ;
- **humain** quand il faut un type de planète précis, une configuration
  commerciale ou un plan à plusieurs colonies.

## Le catalogue

`Frames` : estimation depuis le titre. ⚠ = exception 600 probable,
✗ = au-delà de 600 probable.

### Écrans

| # | fichier | ce qu'il fait | frames | écrit par | remplace (TU) | supplante (rejeu du filet) |
|---|---|---|---|---|---|---|
| 1 | `ecran_titre_difficulte` | survole « need help? » (`fv.hd`), clique une difficulté, quelques frames de barre du haut | ~10 | agent | `top_menubar` | `top_menubar`, `screencast_capture` ¹ |
| 2 | `ecran_pause_rangees` | `ix[4]` ouvre la pause ; « toggle music », puis on la rouvre pour « toggle sfx », puis « how to play? » | ~30 | agent | `pause_menu_music` | `pause_menu_music` |
| 3 | `ecran_pause_recommencer` | pause, « restart » (`_init`), retour au titre, une autre difficulté | ~15 | agent | — | — |
| 4 | `ecran_score_ouvrir_fermer` | `ix[3]` ouvre le panneau de score (0 étoile), un clic le ferme | ~25 | agent | `score_panel_close`, `score_panel_stars` ² | — |
| 5 | `ecran_fin_score_final` | 25 années sautées, compte à rebours, écran « final score », clic « again? » (`_init`) | ~70 | agent | `game_end` | `game_end` |
| 6 | `ecran_fin_banqueroute` | pose des labos (15 $, 2 mois chacun) jusqu'à la ruine : l'année passe par le temps (`hy:nx`, au 6ᵉ labo), un achat refusé affiche « -no money- », l'entretien des labos vide la caisse, écran « bankrupt » ; l'ordre exact se règle en headless | ~90 | agent | — | — |
| 7 | `boutique_survol_infobulle` | survole les icônes `ie` du bas (labo, processeur) : bulle `jb` et `fillp` | ~15 | agent | `shop_tooltip_fillp` | `shop_button_hover` |
| 8 | `triche_code_ox` | séquence O/X complète, séquence interrompue, séquence complète (comme la TU) | ~45 | agent | `cheat_code` | — |
| 9 | `camera_glisser_dpad` | glisser au clic droit, puis D-pad tenu jusqu'à la butée [-64, 320] | ~70 | agent | — | — |

¹ La fixture `screencast_capture_events.json` reste : `test_pico8_headless_capture`
s'en sert, et ce test ne fait pas partie de la conversion.
² Seulement à 0 étoile : allumer une étoile demande 2 000 points de score, et
l'ordre des étoiles allumées, que vérifie la TU, n'est pas atteignable en
600 frames (question 3).

### Exploration, colonisation, réseau

| # | fichier | ce qu'il fait | frames | écrit par | remplace (TU) | supplante (rejeu) |
|---|---|---|---|---|---|---|
| 10 | `sonde_revele_planetes` | premier clic dans le vide : `g_` ; second : `mz` tenu sur une planète, révélation ; puis une cible « -too far- » | ~50 | agent | — | `phase4_playthrough` (en partie) |
| 11 | `planete_vide_disparait` | révèle une planète vide (`ll`, 45 % par planète), la regarde s'effacer (> 60 ticks), pose un labo | ~140 | agent, humain en repli | `empty_planet_cleared` | `empty_planet_cleared` |
| 12 | `colonie_panneau_inspection` | colonise une planète révélée (`jo:fe`), ouvre son panneau (`jo:gh`) puis celui d'une planète non colonisée | ~100 | agent | `built_planet_panel_nil_row` | `built_planet_panel` |
| 13 | `slipway_construire` | deux colonies, glisser le slipway, relâcher ; puis une sonde qui survole le slipway (`bv`) | ~170 ⚠ | agent | `slipway_drag`, `playability` | `slipway_drag`, `phase4_playthrough` |
| 14 | `slipway_refus` | slipways refusés : trop long, passant sur une planète, croisant un autre (`cl`), en double | ~300 ⚠ | humain | — | — |
| 15 | `commerce_paliers` | petit réseau dont les besoins sont servis : paliers content, puis prosperous (`dh`, `jo:cn`, `db`), panneau avec partenaires | ~500 ⚠/✗ | humain | `rich_planet_exports` ³ | — |

³ Seulement si le palier 4 (rich) est atteint, ce qui est très improbable en
600 frames (question 3).

### Technologies

Les 19 technologies (`nb`) ont presque toutes un effet porté par une ligne
`ov(...)`/`oj(...)` exécutée quelle que soit la technologie : la couverture
de lignes ne les distingue pas. Un scénario par technologie produirait donc
surtout des fichiers rejetés par le critère de garde. Le catalogue garde
une ligne par code réellement propre à une technologie ; le tableau suivant
rattache chaque technologie à une ligne du catalogue.

| # | fichier | ce qu'il fait | frames sans triche | frames avec triche | écrit par | remplace (TU) | supplante (rejeu) |
|---|---|---|---|---|---|---|---|
| 16 | `techno_premiere_invention` | 4 années, écran techno, invente `space folding` (ère 1) | ~35 | — | agent | `tech_screen` | `tech_screen_invent` |
| 17 | `techno_nanomaterials_rabais` | invente `nanomaterials`, ouvre un menu de construction (×0,85, hors référence) | ~110 | — | agent | — | — |
| 18 | `techno_xenofoods_recettes` | 2 technos d'ère 1 + `xenofoods` (19 ans), menu d'une planète earth-like ou ocean (`hp:po` → `mc`, `hp:od` ×1,5) | ~140 | ~90 | agent si une planète e/o est révélée, sinon humain | — | — |
| 19 | `techno_geoharvesting_recolte` | 23 années, 3 inventions, « harvest » sur une planète révélée : éclat `ga` | ~160 | ~110 | agent | `geoharvest_burst` | `geoharvest_burst` |
| 20 | `techno_slipgates_relais` | `slipgates` (21 ans), pose un slipgate, y branche des slipways (`nr:ldown`, `nr:gv`, `nr:hb`), le commerce le traverse (`nr:cn`) | ~300 ⚠ | ~250 ⚠ | humain | — | — |
| 21 | `techno_trade_league_hub` | `trade league`, pose un hub, lui livre 2 ressources (`nl:ly`, `mi>=2`) | ✗ | ~300 ⚠ | humain | — | (`trading_hub_session_events`) |
| 22 | `techno_replication_mechanize` | `replication`, « mechanize » : la planète devient forgeworld (`jo:hu`, branche `go`) | ✗ | ~130 | humain | — | — |
| 23 | `techno_starbirth_protostar` | `starbirth` (ère 3), « collapse » d'une géante gazeuse ou d'une planète minérale : `pm` + `ga` | ✗ | ~180 ⚠ | humain (type précis) | — | — |
| 24 | `techno_infraspace_infragate` | `infraspace` (ère 3), pose un infragate, route qui croise un slipway | ✗ | ~250 ⚠ | humain | — | — |
| 25 | `techno_void_synthesis_synthetiseur` | ère 4 (8 inventions, 2 triches), pose un synthétiseur, le relie à une colonie en manque (`mx:kf`) | ✗ | ~280 ⚠ | humain | — | — |
| 26 | `techno_ascension_porte` | ère 4, pose une porte d'ascension (`hj:kj`) | ✗ | ~220 ⚠ | humain | — | — |

Où finit chaque technologie :

| technologie (ère, coût) | couverte par |
|---|---|
| space folding (1, 4) | 16 |
| geneseeds (1, 5) | chemin de 18/19 (effet : recettes, données seules) |
| nanomaterials (1, 6) | 17 |
| drillbots (1, 7) | aucun code propre (recette `excavate`, données seules) |
| xenofoods (2, 10) | 18 |
| slipgates (2, 12) | 20 |
| geoharvesting (2, 14) | 19 |
| trade league (2, 16) | 21 |
| replication (2, 18) | 22 |
| autoassemblers (3, 19) | aucun code propre (`ov` dans `jo:hu`) |
| skill implants (3, 22) | aucun code propre (`ov` dans `hy:lj`) |
| starbirth (3, 25) | 23 |
| biome hacking (3, 28) | même branche `go` que 22 (« terraform ») |
| infraspace (3, 31) | 24 |
| superhuman ai (4, 34) | même branche `mc` que 18 |
| void synthesis (4, 38) | 25 |
| gene rewriting (4, 42) | même ×1,5 que 18 (« uplift » : données seules) |
| time compression (4, 46) | aucun code propre (`ov` dans `hy:nx`) |
| ascension (4, 50) | 26 |

### Types de planète

Le type (`df`) ne change que des données : le sprite, le poids du tirage
et la liste de recettes que `bj` filtre. Une fois qu'une colonie existe
(12), une autre colonie sur un autre type ne devrait ajouter aucune
ligne. Les types qui ont du code propre sont déjà dans le catalogue : la
planète vide (11), la géante gazeuse et la planète minérale (collapse, 23),
et les cibles de mechanize/terraform (22). Chaque fichier ci-dessous
demande de trouver ce type à portée : c'est le tirage, donc l'humain.

| # | fichier | type | recettes de départ | frames | apport attendu |
|---|---|---|---|---|---|
| 27 | `planete_earthlike` | earth-like (`e`) | hiveworld, agroworld, luxury | ~100 | nul |
| 28 | `planete_forgeworld` | forgeworld (`f`) | nanotech, bots, goods, gadgets | ~100 | nul |
| 29 | `planete_mineral` | mineral (`m`) | mine | ~100 | nul |
| 30 | `planete_ocean` | ocean (`o`) | colony, algae, breed | ~100 | nul |
| 31 | `planete_remnant` | remnant (`r`) | strip, dismantle, scavenge | ~100 | nul |
| 32 | `planete_xeno` | xeno (`x`) | colony, breed | ~100 | nul |
| 33 | `planete_jungle` | jungle (`j`) | hunt, tourism | ~100 | nul (tourism : `f-`, rangée « makes: », même ligne) |
| 34 | `planete_iceball` | iceball (`i`) | aucune sans technologie | ~60 | nul, menu vide |
| 35 | `planete_barren` | barren (`s`) | aucune sans technologie | ~60 | nul, menu vide |
| 36 | `planete_gasgiant` | gas giant (`g`) | aucune sans technologie | ~60 | nul, menu vide |

Proposition : jouer d'abord deux types (par exemple 30 et 34), les passer
à `make e2e-judge`, et n'écrire les autres que si l'un d'eux ajoute des
lignes (question 2).

## Récapitulatif

- **36 scénarios** : 9 écrans, 6 d'exploration et de réseau, 11 de
  technologie, 10 de type de planète.
- **Agent en headless : 15** (1–10, 12, 13, 16, 17, 19). **Agent avec repli
  humain : 2** (11, 18). **Humain dans raylib : 19** (14, 15, 20–26, 27–36).
  Si la triche est admise, 22, 25 et 26 peuvent passer à l'agent quand la
  cible n'est pas un type précis.
- **Exceptions 600 probables** : 13, 14, 15, 20, et 21, 23–26 avec la triche.
  **Au-delà de 600 probable** : 15 si on vise le palier rich, et 21–26 sans
  la triche. Leurs lignes iraient alors dans
  `tools/line_coverage_unreachable.txt`.
- **TU converties** : les 14 TU du PRD sont couvertes. Pour
  `score_panel_stars` et `rich_planet_exports`, seul le chemin est rejoué :
  l'état qu'elles forcent (étoiles allumées, palier 4) n'est pas atteignable
  dans les limites.
- **Rejeux du filet supplantés** : les 11. `trading_hub_session_events`
  seulement si 21 entre.

## Questions pour l'humain

1. **Le code de triche dans les scénarios de technologie.** Le PRD exclut
   la triche « pour sauter du temps de jeu ». Donner 99 points de recherche,
   est-ce sauter du temps ? Si la triche est interdite, 21–26 dépassent
   presque sûrement 600 frames, et environ 200 lignes de structures
   spéciales (plus une partie du commerce) iraient dans la liste des
   inatteignables. Si elle est admise, ces scénarios tiennent en 130 à 300
   frames.
2. **La structure du catalogue.** Garder un fichier par type de planète
   (27–36) et par technologie comme le demande le PRD, en sachant que la
   plupart seront rejetés par le critère de garde ? Ou garder la structure
   proposée ici (un scénario par code propre, les types en pilote) ?
3. **Les TU dont l'état n'est pas atteignable.** `score_panel_stars`
   (étoiles allumées à gauche) et `rich_planet_exports` (rangée « exports: »
   au palier 4) protègent chacune un vrai piège, qu'une empreinte à 0
   étoile ou au palier 2 ne verra pas. Faut-il les convertir en TU sur état
   capturé, comme `ga_export_panel_by` et `planet_gh_row_lq` (étape 7),
   plutôt qu'en e2e ?
4. **13 ou 14 TU ?** Le PRD dit « les 13 TU qui rejouent des événements »
   mais en nomme 14. Il n'y en a pas d'autre qui boote le cart pour rejouer
   des événements (`demo`, `headless_capture`, `recording_continuity`,
   `marker_coherence` et `replay_from_title` testent l'outillage, pas une
   partie). Il faut sans doute lire 14 ; ou alors
   `score_panel_stars` et `rich_planet_exports` étaient comptées à part,
   puisqu'elles réutilisent la fixture d'une autre.
5. **`main.c` dans la référence.** Refiger les lignes `src/main.c` de la
   référence sur le pilote actuel, ou les lister comme inatteignables avec
   la raison « référence prise avant f0b253b » ? Tant qu'on ne tranche pas,
   `e2e-gap` ne peut pas tomber à zéro.
6. **Les noms.** Des noms en français préfixés par axe (`ecran_`,
   `boutique_`, `sonde_`, `slipway_`, `techno_`, `planete_`), ou des noms
   anglais dans la continuité des fixtures (`pause_menu_music`,
   `slipway_drag`…) ?
7. **Ce que joue l'humain.** Les 19 scénarios marqués « humain » (dont 10
   de types de planète) : lesquels veux-tu jouer, et dans quel ordre ?
   Proposition : 13 et 15 d'abord (réseau et commerce, les plus riches en
   lignes), puis 20 et 21 selon la réponse à la question 1, et les types de
   planète en dernier.

## Annexe : la référence par zone

Mesuré le 2026-09-27 avec `tools/line_coverage.sh cover` sur le binaire
gcov. « Filet court » = union des 11 rejeux courts actuels, joués avec le
raccourci.

| zone | lignes de la référence | dont couvertes par le filet court | restent |
|---|---|---|---|
| moteur, gfx, objets, entrée, `main.c` | 1 732 | 1 687 | 45 |
| titre, menus, HUD, boîtes, triche | 1 159 | 1 103 | 56 |
| carte, caméra, sonde, trou de ver, particules | 736 | 677 | 59 |
| planètes, colonisation, recherche | 1 005 | 961 | 44 |
| structures spéciales (`planet_family*`) | 336 | 152 | 184 |
| slipways et commerce | 505 | 452 | 53 |
| économie et score | 899 | 851 | 48 |
| **total** | **6 372** | **5 883** | **489** |

Apport propre de chaque rejeu court par rapport aux dix autres :
`built_planet_panel` 185, `game_end` 170, `geoharvest_burst` 118,
`empty_planet_cleared` 71, `pause_menu_music` 35, `slipway_drag` 9 ; les
cinq autres 0. Un clic de difficulté depuis le titre
(`title_click_reasonable`) ajoute 49 des 489 lignes restantes.
