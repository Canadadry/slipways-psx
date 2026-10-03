# Vocabulaire du plan 5

Statut : validé

Ce document fixe le sens et le nom anglais des concepts récurrents du code, des membres de l'en-tête commun et de ceux de la vtable des classes, ainsi que l'arborescence cible des fichiers. Il est l'entrée des étapes 1 à 3 du plan 5 (`docs/plan-5.md`) ; pour le détail symbole par symbole, il cède la place au TSV de l'étape 2b (`docs/renommage.tsv`), qui en dérive ses noms. Les noms proposés suivent le §4 du plan : anglais, mots complets, aucun préfixe pour le jeu, `p8_` pour le seul runtime PICO-8 émulé. Une colonne « doute » non vide signale où relire en priorité. Les comptes `N×` sont des occurrences dans `src/` (`grep -o`) ; « gloss. § X » renvoie à une section de l'ancien glossaire des noms, supprimé par la sous-étape 5f du plan 5 et retrouvable dans l'historique git.

## Concepts

### Noyau objet et dispatch

| nom actuel | sens | nom anglais proposé | source | doute |
|---|---|---|---|---|
| `ke` (`pico8_ke_*`) | classe racine dont descend toute entité enregistrée ; porte l'état `me`/`t` de l'en-tête | `entity` (module et préfixe des méthodes de base) | gloss. § Noyau ; `src/pico8_ke.h` (69×) | le module `pico8_entity.c` (registres) prendrait alors un autre nom, cf. `hv`/`cv`/`cg` |
| `ks` (`pico8_ke_ks`) | change l'état `me` de l'entité et remet `t` à 0 | `entity_set_state` | `src/pico8_ke.h:18`, `src/pico8_ke.c:30` | |
| `pq` (suffixe `pico8_X_pq`) | constructeur supplémentaire, appelé une fois par `pico8_X_struct_new` après les défauts | `X_construct` | gloss. § Le système de classes ; `src/pico8_ke.h:28` | `X_init` est déjà pris par l'init de module (`pico8_qb_init`…) |
| `def` (`pico8_X_def`, `pico8_X_def_t`) | l'archétype `static const` d'une classe : ses valeurs de départ (le glossaire les nomme encore `pico8_X_start`) | `X_archetype`, `X_archetype_t` | `CLAUDE.md` ; `src/pico8_game_data.h:302` (`def` : 1063×, mot entier compris) | |
| `pico8_entity_kind_t` | une sorte de planète manipulée comme valeur (`gx[]`, `ie.ke`, `kv[hp.cw]`) : son constructeur et son enregistrement `rec` | `entity_kind_t` | `src/pico8_struct.h:189` (39×) | distinct de `sorte` (places de zone) |
| `lt` (`pico8_lt`) | inscrit une entité neuve dans `hv`, `cv` et `cg` | `entity_register` | gloss. § Noyau ; `src/pico8_entity.h:48` | |
| `he` (`pico8_he`) | retire une entité de `hv`, `cv`, `cg` ; l'envoie à la file de sortie si sa classe a `rendre` | `entity_unregister` | `src/pico8_entity.h:51` | |
| `hv` (`pico8_hv`) | toutes les entités vivantes, dans l'ordre d'inscription | `live_entities` | gloss. § Noyau ; `src/pico8_entity.h:32` | |
| `cv` (`pico8_cv`) | entités inscrites par nom de message (`kj`, `ev`, `hd`, `hk`, `hb`) ; `hy:hk`, `hy:lj`, `ui:gw` le lisent directement | `entities_by_message` | `src/pico8_entity.h:38`, `src/pico8_economy.c:154` | les clés chaîne suivent les noms de la vtable (`"draw"`…), cf. § Vtable |
| `cg` (`pico8_cg`) | entités inscrites par étiquette (`tags`), d'où `cg("jo")` = toutes les planètes | `entities_by_tag` | `src/pico8_entity.h:44` (19×) | |
| `dc` (`pico8_dc`) | dessine, couche `fa` par couche, toutes les entités inscrites sous `kj` (ou `ev`) ; applique la parallaxe de la caméra par couche pour `kj` | `draw_layers` | `src/pico8_entity.c` (assert « only kj and ev ») ; gloss. § Noyau | dans le cart `dc` route les cinq messages ; le port ne lui laisse que le dessin |
| `dg` (`pico8_dg`) | une frame de logique : appelle sur chaque entité la méthode de son état `me`, incrémente `t`, détruit celles dont `p_` est levé | `update_entities` | `src/pico8_bootstrap.h:21`, `src/pico8_bootstrap.c:75` | |
| `jl` (`pico8_jl`) | (re)construit les registres et les singletons `mo`, `je`, `ok` | `reset_world` | `src/pico8_bootstrap.h:12` (12×) | |
| `_dispatch` (suffixe) | adaptateur de signature vtable vers la fonction typée (`pico8_jo_kj_dispatch` → `pico8_jo_kj`) | `X_<méthode>_method` | 272× dans `src/*.c` | peut aussi disparaître si la fonction typée prend directement la signature vtable |

### États (`me`, `pico8_state_t`)

| nom actuel | sens | nom anglais proposé | source | doute |
|---|---|---|---|---|
| `ph` (`PICO8_STATE_PH`) | tick ordinaire, état par défaut de `ke` (`iv`, `js`, `nf`, `pj`, `qb`) | `ENTITY_STATE_NORMAL`, chaîne `"normal"` | gloss. § vocabulaire des méthodes ; `src/pico8_struct.h` | `update` serait plus parlant comme méthode, moins comme état |
| `ly` (`PICO8_STATE_LY`) | posé et actif : une structure confirmée par `d_` (`dq="ly"`), une planète colonisée (`jo:hu`) | `ENTITY_STATE_SETTLED`, `"settled"` | gloss. § vocabulaire des méthodes (`dq`) ; `pico8/lua.lua:458` | « colonized » ne couvre pas les structures |
| `kz` (`PICO8_STATE_KZ`) | en cours de placement, avant validation (`qh`, `mz`) | `ENTITY_STATE_PLACING`, `"placing"` | `pico8/lua.lua:908` (`qh` : `me="kz"`) ; `src/pico8_wormhole.c` | |
| `pr` (`PICO8_STATE_PR`) | trou de ver confirmé par `d_`, son front de révélation grandit | `ENTITY_STATE_UNFOLDING`, `"unfolding"` | gloss. § Le trou de ver | |
| `ll` (`PICO8_STATE_LL`) | planète qui disparaît ; détruite après 60 ticks | `ENTITY_STATE_VANISHING`, `"vanishing"` | `src/pico8_planet.c:323` | |

### Mémoire et listes

| nom actuel | sens | nom anglais proposé | source | doute |
|---|---|---|---|---|
| `pico8_zone_choisie_t` | poignée sur une zone (`alloc`, `grow`, `release`) passée à qui alloue pour son propriétaire | `allocator_t` | `src/pico8_zone_choisie.h:18` (`zone_choisie` : 207×) | |
| `pico8_bascule_zone_t` | zone à deux bouts : chaque génération alloue depuis un bout, la précédente reste valide jusqu'à son rendu | `flip_zone_t` | `src/pico8_zone.h:66` (`bascule` : 77×) | |
| `arene` (`pico8_zone_arene_*`) | bloc pris à une zone, qui porte sa propre zone montante ; rendu d'un coup | `arena` | `src/pico8_zone_choisie.h` | |
| `prise` (`pico8_jo_prise_*`, champ `prise`) | l'arène où se construit une structure tenue, un slipway ou un trou de ver, recyclée si l'objet n'est pas posé | `grab_arena` | `src/pico8_planet.h:101-131` (94×) | |
| `sorte` / `place` (`pico8_sorte_t`, `pico8_particules_sorte_t`) | sorte d'objet d'une zone à places, et la place que chaque objet occupe | `slot_kind_t` / `slot` | `src/pico8_zone_interface.h`, `src/pico8_zone_particules.h` (157× / 310×) | `place` compte aussi l'anglais *replace* |
| `zone_partie`, `zone_chargement`, `zone_frame` | la génération de partie, la zone du chargement, la zone vidée à chaque frame | `game_zone`, `load_zone`, `frame_zone` | `src/pico8_zone_partie.h`, `src/pico8_zone_frame.h` | |
| `rendre` | rendre la mémoire d'un objet à sa zone (méthode de vtable, `_rendre` des zones) | `release` | `src/pico8_file_sortie.h` (140×) | |
| `file_sortie` | file des entités et arènes sorties de `hv`, libérées au vidage suivant | `exit_queue` | `src/pico8_file_sortie.h` | |
| `pico8_ents_t` | liste d'en-têtes d'entités (`add`/`del`/`has`/`cat`/`all`) | `entity_list_t` | `src/pico8_ents.h:14` (137×) | |
| `pico8_ptrs_t` | liste de `void *`, `del` par identité | `pointer_list_t` | `src/pico8_ptrs.h:14` (77×) | |

### Planètes et structures

| nom actuel | sens | nom anglais proposé | source | doute |
|---|---|---|---|---|
| `jo` | planète de base : toute planète de la carte, et la parente de toutes les structures | `planet` | gloss. § La planète `jo` ; `src/pico8_planet.h:64` (`pico8_jo_` : 711×) | base aussi des structures (`qh`) : `body` si on veut l'éviter |
| `qh` (lignée `PICO8_LINEAGE_QH`) | structure posée par le joueur (laboratoire et ses descendantes : `os`, `nr`, `fr`, `nl`, `mx`, `hj`, `pm`) ; état initial `kz` | `structure` | `pico8/lua.lua:905-911` ; `src/pico8_struct.h` (lignée) | `qh` lui-même s'affiche « laboratory » : `laboratory` pour la classe, `structure` pour la lignée ? |
| `os` | structure « processor » | `processor` | gloss. § sous-classes ; `pico8/lua.lua:1095` | |
| `nr` | structure « slipgate » : relaie les routes au lieu de les consommer | `slipgate` | `pico8/lua.lua:1101` (54×) | |
| `fr` | structure « infraspace » : un `nr` dont les slipways vivent sur un autre plan (`mt=1`) | `infraspace_gate` | `pico8/lua.lua:1123` | « infraspace » seul nomme aussi la techno |
| `nl` | structure « trading hub » | `trading_hub` | `pico8/lua.lua:1057` | |
| `mx` | structure « synthesizer » : synthétise les ressources de `ht` | `synthesizer` | `pico8/lua.lua:1036` | |
| `hj` | structure « ascension », émettrice de particules | `ascension` | `pico8/lua.lua:998` | |
| `pm` | structure « protostar », fait naître un `ga` | `protostar` | `pico8/lua.lua:1017` | |
| `ga` | éclat de particules en cercles, membre de la famille `jo`, sans étiquette ; secoue la caméra (`mo.ma=3`) et meurt avec sa dernière particule | `burst` | `pico8/lua.lua:1067-1094` ; `src/pico8_planet_family.c:353` | |
| `df` | catalogue des types de planète, tiré par `bw` | `planet_types` | gloss. § Tables de données (27×) | |
| `nk` (champ de `jo`) | type courant de la planète : une entrée de `df` | `planet_type` | gloss. § La planète `jo` | **deux sens** : sur une entrée de `nb`, l'icône de la techno → `icon` |
| `px` | nom affiché (rangée colorée dans l'arbre `jo`) ou clé de nom (projets, technos) | `name` | gloss. § La planète `jo` (77×) | rangée affichée contre clé chaîne : `display_name` pour la première ? |
| `hp` (champ de sous-classe) | index 1–6 du projet de recherche qui crée cette structure, lu par `jo:hu` | `project_index` | gloss. § Deux noms, deux sens | **deux sens** : la classe `hp` → `research_project` |
| `hf` | vrai sur une planète naturelle (`jo`), faux sur une structure et un `ga` ; seules les routes vers un `hf` rapportent | `is_planet` | `pico8/lua.lua:387,909,1306,1329` | le glossaire dit « hub », que le code ne confirme pas |
| `mi` | palier de développement d'une planète, 0 à 4 (`dh`) | `development_tier` | gloss. § Champs de planète (24×) | |
| `oe` | slipways branchés sur la planète | `slipways` | `src/pico8_planet_trade.c:53` (16×) | même nom que `rl` de `trade`, voulu : même sens, autre struct |

### Slipways et commerce

| nom actuel | sens | nom anglais proposé | source | doute |
|---|---|---|---|---|
| `jc` | slipway : le lien commercial entre deux planètes | `slipway` | gloss. § Le slipway `jc` ; `src/pico8_slipway.h:70` (218×) | |
| `qu`, `re` (champs de `jc`) | les deux extrémités : la planète de départ, et l'autre planète ou le point de souris pendant le tracé | `start`, `end` | gloss. § Le slipway (23× / 18×) | |
| `trade` (`pico8_trade_t`) | route commerciale : chaîne de planètes, de slipways, et ressource transportée | `trade_route_t` | gloss. § Commerce ; plan §4 (exemple) | |
| `lp`, `rl` (champs de `trade`) | planètes de la route / slipways de la route | `planets`, `slipways` | gloss. § Commerce | |
| `qe` (`pico8_qe_t`, `pico8_qe_map_t`) | une ressource (nil, faux ou un nombre), et une table indexée par ressource | `resource`, `resource_t`, `resource_map_t` | `src/pico8_trade.h:38` (41× en champ, 69× en préfixe) | |
| `jy` (`pico8_jy_t`) | besoins d'une planète : liste de `{qe}` avec `gc` satisfaits | `needs`, `need_t` | `src/pico8_research.h:18` | |
| `kw` (`pico8_kw_t`) | production d'une planète : ressource `qe`, quantité `mu` (aussi capacité d'export) | `production`, `production_t` | gloss. § Champs de planète | |
| `lw` (`pico8_jo_trade_proposal_t`) et son `kf` | une proposition de route passée de planète en planète par `cn` ; son `kf` l'accepte dans `ib` | `trade_offer_t`, `accept` | `src/pico8_planet_trade.h:13-27` | homonyme du `kf` de la vtable, à ne pas fusionner |

### Recherche et économie

| nom actuel | sens | nom anglais proposé | source | doute |
|---|---|---|---|---|
| `hp` (classe) | projet de recherche installable sur une planète | `research_project` | `src/pico8_research.h:58` (105×) | **deux sens**, cf. `hp` champ |
| `nb` | liste des technologies | `technologies` | gloss. § Tables de données (40×) | |
| `ie` | icône de structure du bandeau : révélée quand sa techno est acquise, son `ldown` construit la structure tenue | `structure_icon` | gloss. § Boîtes ; `src/pico8_research_icon.h:33` (86×) | le fichier dit `research_icon` |
| `hy` | classe économie : horloge, crédits, recherche, recalcul | `economy` | `src/pico8_fm.h:40` (75×) | |

### Sonde et trou de ver

| nom actuel | sens | nom anglais proposé | source | doute |
|---|---|---|---|---|
| `g_` | la sonde lancée d'un clic sur le vide ; `fv` en descend | `probe` | gloss. § La sonde ; `src/pico8_probe.h:35` (70×) | entre en conflit avec le composant `pico8_probe_t`, renommé ci-dessous |
| `pico8_probe_t` (composant ; `bv`, `dl`, `d_`) | le ciblage que partagent `g_`, les structures et `mz` : suivre la souris et choisir la cible (`bv`), l'infobulle (`dl`), valider et payer (`d_`) | `targeting_t` ; `targeting_track`, `targeting_tooltip`, `targeting_confirm` | `src/pico8_probe_comp.h:10-62` | l'étiquette du cart `prober` va avec : `TAG_TARGETING` ? |
| `et`, `fo` | la cible courante ; « accrochée par », le lien retour de la cible vers la sonde | `target`, `targeted_by` | `src/pico8_probe_comp.h` (17× / 18×) | |
| `mz` | trou de ver (« space folding ») : révèle les planètes autour de son trajet | `wormhole` | gloss. § Le trou de ver ; `src/pico8_wormhole.h:37` (109×) | le cart parle de pliage : `space_fold` ? |

### Particules

| nom actuel | sens | nom anglais proposé | source | doute |
|---|---|---|---|---|
| `qy` (`pico8_qy_t`) | une particule `{p,v,a,l}` / la liste d'un émetteur | `particle_t` / `particles` | `src/pico8_particle.h:26` | |
| `f_`, `ez`, `ei` | initialise la liste ; engendre avec probabilité décroissante ; avance et dessine | `emitter_init`, `emitter_spawn`, `emitter_update` | `src/pico8_particle.h:51-77` | |

### Interface

| nom actuel | sens | nom anglais proposé | source | doute |
|---|---|---|---|---|
| `ui` | répartiteur de survol et de clics, dessine le curseur | `ui` (conservé) | gloss. § L'interface `ui` (173×) | deux lettres : `pointer_dispatcher` si la garde de portée le refuse comme type |
| `ok`, `mo`, `je`, `fm`, `ju` | les singletons `ui`, caméra, souris, économie, panneau de score | `main_ui`, `main_camera`, `main_mouse`, `main_economy`, `main_score_panel` | `src/pico8_entity.h:63`, `src/pico8_ui.h:86`, `src/pico8_fm.h:61`, `src/pico8_score_panel.h:121` | un préfixe `main_` commun, ou le nom de la classe seul ? |
| `qb` | caméra : défilement au D-pad avec inertie, parallaxe par couche (`md`) | `camera` | gloss. § Caméra (74×) | |
| `nf` | lecteur de la souris (position, fronts des deux boutons) | `mouse` | `src/pico8_mouse_reader.h:23` (76×) | |
| `bg` | fond étoilé ; clic gauche sur le vide, glisser-caméra au clic droit | `background` | gloss. § Fond de carte (87×) | |
| `js`, `fq` | bouton cliquable ; fond modal qui attrape les clics derrière une boîte | `button`, `modal_backdrop` | gloss. § Boîtes (164× / 56×) | |
| `mh`, `kc` | boîte de message ; celle affichée en ce moment | `message_box`, `shown_message_box` | `src/pico8_msgbox.h:44` (76×) | |
| `iv` | toast flottant qui monte et s'efface | `toast` | `src/pico8_msgbox.h:101` (68×) | |
| `lq` | rangée d'icônes et de textes mise en page | `layout_row` | `src/pico8_layout.h:75` (264×) | |
| `jb` | bulle d'info : des lignes `lq` encadrées | `tooltip` | `src/pico8_layout.h:84` | |
| `pico8_jb_row_t`, `pico8_jb_item_t` | rangée constante générée (texte, nombres, sous-rangées) et son article : contenu des bulles, rangées, menus | `data_row_t`, `data_item_t` | gloss. § Boîtes (243× / 221×) | |
| `pi` | table des fonctions de dessin que nomme le `fn` d'un article de `bl` | `draw_functions` | gloss. § Le mini-langage (91×) | |
| `pj` | panneau de score | `score_panel` | `src/pico8_score_panel.h:25` (149×) | |
| `fv`, `cr` | écran d'aide ; écran de l'arbre technologique | `help_screen`, `tech_tree_screen` | gloss. § Boîtes, § Divers | `cr` a **deux sens** : `pj.cr` est un bloc de mise en page |

### Runtime PICO-8 et géométrie

| nom actuel | sens | nom anglais proposé | source | doute |
|---|---|---|---|---|
| préfixe `pico8_` des primitives émulées (`pico8_peek`, `pico8_spr`…) | runtime PICO-8 | préfixe `p8_` | plan §2, §4 | |
| `pico8_fixed_t` | nombre en virgule fixe 16.16 de PICO-8 | `p8_fixed_t` | plan §4 (exemple) (754×) | |
| `pico8_gfx_t`, `pico8_mem_t` | écran et mémoire émulés | `p8_graphics_t`, `p8_memory_t` | (108× / 140×) | `gfx` est un idiome : `p8_gfx_t` ? |
| `pico8_vec_t` (`km`) | vecteur `{x,y}` en virgule fixe | `vec_t` | gloss. § Géométrie (410×) | jeu ou runtime : le cart le définit, pas PICO-8 |
| `qz` (`pico8_qz_box_t`) | boîte : prédicat « ce point est-il dedans » | `box_t` | `src/pico8_geom.h:34` | |

## En-tête commun

| membre actuel | sens | nom anglais proposé | source | doute |
|---|---|---|---|---|
| `pico8_struct_header_t` | premier membre de toute entité : sa classe, son état, des liens vers ses champs partagés | `entity_t` | `src/pico8_struct.h` | `lq` et `jb` le portent sans être inscrits |
| `cls` | la classe (vtable) de l'entité | `class` | `src/pico8_struct.h` ; `->cls->` partout | mot réservé C++ : `entity_class` si un outil compile en C++ |
| `me` | nom de l'état courant (`"ph"`, `"ly"`… ou `"unknown"`/`"known"` hors enum) ; NULL = nil | `state` | gloss. § Noyau ; `pico8_state_of` | |
| `t` | frames passées dans l'état courant : remis à 0 par `ks`, +1 par `dg` | `state_time` | `src/pico8_ke.c:30`, `src/pico8_bootstrap.c:75` | |
| `fa` | couche de rendu 1–15 ; `fa >= 10` = interface, testée contre la souris écran | `layer` | gloss. § Champs d'entité ; `src/pico8_entity_geom.c` (`gk`) | |
| `ra` | position en pixels où l'entité est dessinée et testée (`gk`, `gd`, `kj`) ; `p * 128` pour une planète | `pos` | `src/pico8_galaxy.c` (`ra = p*128`), `src/pico8_probe.c:158` | le plan donne `e->p` → `e->pos` en exemple ; le code dit que c'est `ra` la position dessinée |
| `p` | position sur la carte de la galaxie, en cases (`ra / 128`) : lue par `ij` (plus proche voisin) et les slipways ; sur la caméra, la position avant secousse | `map_pos` | `src/pico8_probe.c:158` (`p = mp/128`), `src/pico8_entity_geom.c` (`ij_step`), `src/pico8_scroll.c` | sens différent sur `qb` |
| `kq` | boîte de survol, un prédicat `qz` relatif à `ra` | `hit_box` | gloss. § Champs d'entité ; `src/pico8_entity_geom.c` (`gk`) | |
| `ql` | sprite du curseur quand l'entité est survolée | `cursor_sprite` | `src/pico8_ui.c:261` | |
| `fg` | valeur écrite au GPIO `0x5f80` tant que l'entité est survolée, puis décrémentée : signal à la page hôte (« need help? ») | `gpio_signal` | `src/pico8_ui.c:325-330` ; `pico8/lua.lua:990,1158` | sens déduit de deux seuls porteurs (`fv`, bouton d'aide) |
| `mt` | plan des slipways : posé sur `fr`, hérité par un slipway de ses extrémités ; deux slipways ne se croisent que sur le même plan | `infraspace` | `src/pico8_slipway.c:47-124,212` ; `pico8/lua.lua:624-650` | `plane` si on veut un nom neutre |
| `ma` | amplitude de la secousse de position, décroissante (`pico8_ma`) : planète qu'on raccorde (`jo:kd`), caméra (`ga`) | `shake` | `src/pico8_entity_geom.c:53`, `src/pico8_planet.c:320` | le glossaire dit « halo » ; le code décale la position |
| `fx` | peut ancrer un trou de ver : vrai sur une planète colonisée, propagé le long des slipways (`jo:kf`), et sur toute sonde `g_` (archétype) | `fold_anchor` | `src/pico8_planet_trade.c:64` ; `pico8/lua.lua:458,466,765` | `in_network` si on préfère le sens à l'usage |
| `p_` | à détruire : `dg` retire l'entité en fin de frame | `is_dead` | gloss. § La sonde | sur `fq` il porte la vérité rendue par `my` |
| `m_` | survolée cette frame, posé et retiré par `ui:gw` | `is_hovered` | gloss. § L'interface ; `src/pico8_ui.c:315-321` | |
| `emitter` | l'émetteur de particules que l'instance porte | conservé | `src/pico8_particle.h:46` | |
| `probe` | le composant de ciblage (`bv`, `dl`, `d_`) que l'instance porte | `targeting` | `src/pico8_probe_comp.h` | déjà anglais, mais `probe` revient à `g_` (cf. § Sonde) |

## Vtable

Les cinq noms que route `cv` (`kj`, `ev`, `hd`, `hk`, `hb`) deviennent aussi les clés chaîne de `entities_by_message` : `"draw"`, `"draw_overlay"`, `"hit_test"`, `"new_year"`, `"refresh"`.

| membre actuel | sens | nom anglais proposé | source | doute |
|---|---|---|---|---|
| `pico8_struct_class_t` | la classe (`struct pico8_struct_class`) : un `static const` par classe, méthodes NULL si absentes | `entity_class_t` | `src/pico8_struct.h` (135×) | |
| `kj` | dessiner l'entité à sa position, couche par couche (`dc("kj")`) | `draw` | gloss. § vocabulaire des méthodes ; 12 classes | |
| `ev` | dessiner après toutes les couches : curseur, infobulles, boîtes (`dc("ev")`) | `draw_overlay` | `src/pico8_cart.c:264` ; `ui`, `dl`, `jc`, `js`, `mh` | |
| `hd` | le curseur (monde `qa`, écran `ra`) est-il sur l'entité ; lu par `ui:gw` | `hit_test` | `src/pico8_ui.c:294-299` | |
| `hk` | tick d'année, reçoit la nouvelle année (`hy:hk`) | `new_year` | `src/pico8_economy.c:154-160` | |
| `hb` | recalculer l'affichage après un recalcul de l'économie (`hy:lj`) | `refresh` | `src/pico8_economy.c:255-263` | |
| `has_hb` | l'instance a-t-elle un `hb` (NULL : un `hb` non NULL suffit) | `has_refresh` | `src/pico8_entity.c:153` | |
| `ldown` | appui du bouton gauche sur l'entité survolée ; rend l'entité tenue (`pico8_down_t`) | `left_press` | `src/pico8_ui.c` (`ui:bh`) | |
| `rdown` | même chose au bouton droit (seul `bg`) | `right_press` | `src/pico8_ui.c` (`ui:bh`) | |
| `gv` | au relâchement, l'entité survolée accepte-t-elle de recevoir l'entité tenue `s` | `accept_drop` | `src/pico8_ui.c:432`, `src/pico8_slipway.c:109` | |
| `hq` | chaque frame tant que l'entité est tenue | `drag` | `src/pico8_ui.c` (`ui:bh`) | |
| `ey` | relâchement de l'entité tenue, avec la cible trouvée par `gv` : valider | `drop` | `src/pico8_ui.c` (`ui:bh`) | |
| `kf` | une planète enregistre un slipway qu'on vient de construire | `attach_slipway` | `src/pico8_slipway.c:161-168`, `src/pico8_planet_trade.c:45` | homonyme du `kf` de `lw` (→ `accept`) |
| `cn` | une planète reçoit une proposition de route et l'accepte si elle a besoin de la ressource (`nr` la relaie) | `offer_trade` | `src/pico8_planet_trade.c:72,240` | |
| `ol` | dessiner une rangée ou une bulle en `p`, alignée selon `a` | `draw_at` | `src/pico8_layout.c:296-307` | |
| `mm` | fabriquer une particule neuve, que l'appelant possède | `spawn_particle` | `src/pico8_particle.c:87-88` | |
| `states` | la méthode de chaque état, appelée par `dg` | conservé | `src/pico8_struct.h` | |
| `tags` | masque des étiquettes sous lesquelles `lt` inscrit l'instance | conservé | `src/pico8_struct.h` | |
| `lineage` | masque des classes dont celle-ci est ou descend | conservé | `src/pico8_struct.h` | |
| `md` | appliquer la caméra avec le facteur de parallaxe de la couche (seul `qb`, appelé par `dc`) | `apply_camera` | `src/pico8_entity.c:254-255` | |
| `rendre` | rendre ce que l'instance tient dans les zones ; appelé seulement au vidage de la file de sortie | `release` | `src/pico8_file_sortie.c:49` | |

## Arborescence

Chaque fichier suivi par git sous `src/`, `tests/` et `tools/` y figure une fois, avec son répertoire et son nom cibles (§3 et §4 du plan), même quand il garde son nom ; les tables sont rangées par répertoire cible. Les noms dérivent des concepts ci-dessus ; « conservé » marque un fichier qui ne change pas de nom, et la colonne doute signale aussi tout classement hors de la liste indicative du §3.

### `src/p8/` (18)

| chemin actuel | répertoire cible | nom cible | doute |
|---|---|---|---|
| `src/pico8_assets.c` | `src/p8/` | `assets.c` |  |
| `src/pico8_assets.h` | `src/p8/` | `assets.h` |  |
| `src/pico8_audio.c` | `src/p8/` | `audio.c` |  |
| `src/pico8_audio.h` | `src/p8/` | `audio.h` |  |
| `src/pico8_fatal.h` | `src/p8/` | `fatal.h` | `.h` seul ; son implémentation PC est `platform/fatal_pc.c` |
| `src/pico8_fixed.c` | `src/p8/` | `fixed.c` |  |
| `src/pico8_fixed.h` | `src/p8/` | `fixed.h` |  |
| `src/pico8_gfx.c` | `src/p8/` | `gfx.c` |  |
| `src/pico8_gfx.h` | `src/p8/` | `gfx.h` |  |
| `src/pico8_input.c` | `src/p8/` | `input.c` | déclare aussi le chargement JSON que définit `devtools/input_json.c` : en-tête à scinder ? |
| `src/pico8_input.h` | `src/p8/` | `input.h` | déclare aussi le chargement JSON que définit `devtools/input_json.c` : en-tête à scinder ? |
| `src/pico8_mem.c` | `src/p8/` | `mem.c` |  |
| `src/pico8_mem.h` | `src/p8/` | `mem.h` |  |
| `src/pico8_palette.c` | `src/p8/` | `palette.c` |  |
| `src/pico8_palette.h` | `src/p8/` | `palette.h` |  |
| `src/pico8_quarter_wave.h` | `src/p8/` | `quarter_wave.h` |  |
| `src/pico8_str.c` | `src/p8/` | `str.c` |  |
| `src/pico8_str.h` | `src/p8/` | `str.h` |  |

### `src/engine/` (32)

| chemin actuel | répertoire cible | nom cible | doute |
|---|---|---|---|
| `src/pico8_bootstrap.c` | `src/engine/` | `bootstrap.c` |  |
| `src/pico8_bootstrap.h` | `src/engine/` | `bootstrap.h` |  |
| `src/pico8_collect.c` | `src/engine/` | `collect.c` |  |
| `src/pico8_collect.h` | `src/engine/` | `collect.h` |  |
| `src/pico8_dispatch_context.c` | `src/engine/` | `dispatch_context.c` |  |
| `src/pico8_dispatch_context.h` | `src/engine/` | `dispatch_context.h` |  |
| `src/pico8_entity.c` | `src/engine/` | `entity_registry.c` | registres `hv`/`cv`/`cg`, `lt`/`he`, `dc` ; le nom `entity` va à `ke` ; le vocabulaire dit `entity_register`, le module donnerait `entity_registry_*` |
| `src/pico8_entity.h` | `src/engine/` | `entity_registry.h` | registres `hv`/`cv`/`cg`, `lt`/`he`, `dc` ; le nom `entity` va à `ke` ; le vocabulaire dit `entity_register`, le module donnerait `entity_registry_*` |
| `src/pico8_entity_geom.c` | `src/engine/` | `entity_geom.c` | hors liste §3 : `gk`, `ma`, `ij` sur un `entity_t`, sans planète |
| `src/pico8_entity_geom.h` | `src/engine/` | `entity_geom.h` | hors liste §3 : `gk`, `ma`, `ij` sur un `entity_t`, sans planète |
| `src/pico8_ents.c` | `src/engine/` | `entity_list.c` | §3 dit `ents` ; type `entity_list_t` |
| `src/pico8_ents.h` | `src/engine/` | `entity_list.h` | §3 dit `ents` ; type `entity_list_t` |
| `src/pico8_file_sortie.c` | `src/engine/` | `exit_queue.c` |  |
| `src/pico8_file_sortie.h` | `src/engine/` | `exit_queue.h` |  |
| `src/pico8_geom.c` | `src/engine/` | `geom.c` | hors liste §3 : `ee`, `cl`, `be`, boîte `qz` ; même question que `vec` |
| `src/pico8_geom.h` | `src/engine/` | `geom.h` | hors liste §3 : `ee`, `cl`, `be`, boîte `qz` ; même question que `vec` |
| `src/pico8_ke.c` | `src/engine/` | `entity.c` | classe racine `ke` → `entity` (§ Noyau objet) |
| `src/pico8_ke.h` | `src/engine/` | `entity.h` | classe racine `ke` → `entity` (§ Noyau objet) |
| `src/pico8_ptrs.c` | `src/engine/` | `pointer_list.c` | §3 le range dans `p8/` ; or c'est un conteneur du port, pas une primitive PICO-8, et le vocabulaire le nomme `pointer_list_t` sans `p8_` |
| `src/pico8_ptrs.h` | `src/engine/` | `pointer_list.h` | §3 le range dans `p8/` ; or c'est un conteneur du port, pas une primitive PICO-8, et le vocabulaire le nomme `pointer_list_t` sans `p8_` |
| `src/pico8_struct.h` | `src/engine/` | `entity_class.h` | §3 dit `struct_header` ; porte `entity_t`, `entity_class_t` et l'enum des états |
| `src/pico8_vec.c` | `src/engine/` | `vec.c` | hors liste §3 : défini par le cart, pas par PICO-8, mais sans planète ; `engine/` ou `game/` ? |
| `src/pico8_vec.h` | `src/engine/` | `vec.h` | hors liste §3 : défini par le cart, pas par PICO-8, mais sans planète ; `engine/` ou `game/` ? |
| `src/pico8_zone.c` | `src/engine/` | `zone.c` |  |
| `src/pico8_zone.h` | `src/engine/` | `zone.h` |  |
| `src/pico8_zone_choisie.c` | `src/engine/` | `allocator.c` |  |
| `src/pico8_zone_choisie.h` | `src/engine/` | `allocator.h` |  |
| `src/pico8_zone_frame.c` | `src/engine/` | `frame_zone.c` |  |
| `src/pico8_zone_frame.h` | `src/engine/` | `frame_zone.h` |  |
| `src/pico8_zone_partie.c` | `src/engine/` | `game_zone.c` |  |
| `src/pico8_zone_partie.h` | `src/engine/` | `game_zone.h` |  |
| `src/pico8_zone_sizes.h` | `src/engine/` | `zone_sizes.h` | hors liste §3 ; budgets de toutes les zones, jeu compris |

### `src/game/` (88)

| chemin actuel | répertoire cible | nom cible | doute |
|---|---|---|---|
| `src/pico8_bg.c` | `src/game/` | `background.c` | §3 dit `bg` ; `bg` → `background` |
| `src/pico8_bg.h` | `src/game/` | `background.h` | §3 dit `bg` ; `bg` → `background` |
| `src/pico8_cart.c` | `src/game/` | `cart.c` |  |
| `src/pico8_cart.h` | `src/game/` | `cart.h` |  |
| `src/pico8_cheat.c` | `src/game/` | `cheat.c` |  |
| `src/pico8_cheat.h` | `src/game/` | `cheat.h` |  |
| `src/pico8_data_init.c` | `src/game/` | `data_init.c` |  |
| `src/pico8_data_init.h` | `src/game/` | `data_init.h` |  |
| `src/pico8_economy.c` | `src/game/` | `economy.c` |  |
| `src/pico8_economy.h` | `src/game/` | `economy.h` |  |
| `src/pico8_economy_formulas.c` | `src/game/` | `economy_formulas.c` |  |
| `src/pico8_economy_formulas.h` | `src/game/` | `economy_formulas.h` |  |
| `src/pico8_economy_score.c` | `src/game/` | `economy_score.c` |  |
| `src/pico8_economy_score.h` | `src/game/` | `economy_score.h` |  |
| `src/pico8_economy_tiers.c` | `src/game/` | `economy_tiers.c` |  |
| `src/pico8_economy_tiers.h` | `src/game/` | `economy_tiers.h` |  |
| `src/pico8_fm.h` | `src/game/` | `main_economy.h` | hors liste §3 ; `.h` seul : archétype de `hy` et singleton `fm` ; à fondre dans `economy.h` ? |
| `src/pico8_galaxy.c` | `src/game/` | `galaxy.c` |  |
| `src/pico8_galaxy.h` | `src/game/` | `galaxy.h` |  |
| `src/pico8_game_data.c` | `src/game/` | `game_data.c` |  |
| `src/pico8_game_data.h` | `src/game/` | `game_data.h` |  |
| `src/pico8_gen.c` | `src/game/` | `gen.c` |  |
| `src/pico8_gen.h` | `src/game/` | `gen.h` |  |
| `src/pico8_help.c` | `src/game/` | `help.c` |  |
| `src/pico8_help.h` | `src/game/` | `help.h` |  |
| `src/pico8_hud.c` | `src/game/` | `hud.c` |  |
| `src/pico8_hud.h` | `src/game/` | `hud.h` |  |
| `src/pico8_layout.c` | `src/game/` | `layout.c` |  |
| `src/pico8_layout.h` | `src/game/` | `layout.h` |  |
| `src/pico8_map_tables.c` | `src/game/` | `map_tables.c` |  |
| `src/pico8_map_tables.h` | `src/game/` | `map_tables.h` |  |
| `src/pico8_menu.c` | `src/game/` | `menu.c` |  |
| `src/pico8_menu.h` | `src/game/` | `menu.h` |  |
| `src/pico8_mouse.c` | `src/game/` | `mouse_button.c` | hors liste §3 ; `bu` et l'état d'un bouton de `nf` |
| `src/pico8_mouse.h` | `src/game/` | `mouse_button.h` | hors liste §3 ; `bu` et l'état d'un bouton de `nf` |
| `src/pico8_mouse_reader.c` | `src/game/` | `mouse.c` | §3 le range dans `p8/` sous `mouse_reader` ; or `nf` est une classe du cart et le module inclut `game_data`, `pi` et `zone_partie` |
| `src/pico8_mouse_reader.h` | `src/game/` | `mouse.h` | §3 le range dans `p8/` sous `mouse_reader` ; or `nf` est une classe du cart et le module inclut `game_data`, `pi` et `zone_partie` |
| `src/pico8_msgbox.c` | `src/game/` | `message_box.c` | §3 dit `msgbox` ; `mh` → `message_box` |
| `src/pico8_msgbox.h` | `src/game/` | `message_box.h` | §3 dit `msgbox` ; `mh` → `message_box` |
| `src/pico8_particle.c` | `src/game/` | `particle.c` |  |
| `src/pico8_particle.h` | `src/game/` | `particle.h` |  |
| `src/pico8_pi.c` | `src/game/` | `draw_functions.c` | hors liste §3 ; `pi` → `draw_functions` |
| `src/pico8_pi.h` | `src/game/` | `draw_functions.h` | hors liste §3 ; `pi` → `draw_functions` |
| `src/pico8_planet.c` | `src/game/` | `planet.c` |  |
| `src/pico8_planet.h` | `src/game/` | `planet.h` |  |
| `src/pico8_planet_display.c` | `src/game/` | `planet_display.c` |  |
| `src/pico8_planet_display.h` | `src/game/` | `planet_display.h` |  |
| `src/pico8_planet_family.c` | `src/game/` | `planet_family.c` |  |
| `src/pico8_planet_family.h` | `src/game/` | `planet_family.h` |  |
| `src/pico8_planet_family_trade.c` | `src/game/` | `planet_family_trade.c` |  |
| `src/pico8_planet_family_trade.h` | `src/game/` | `planet_family_trade.h` |  |
| `src/pico8_planet_trade.c` | `src/game/` | `planet_trade.c` |  |
| `src/pico8_planet_trade.h` | `src/game/` | `planet_trade.h` |  |
| `src/pico8_probe.c` | `src/game/` | `probe.c` |  |
| `src/pico8_probe.h` | `src/game/` | `probe.h` |  |
| `src/pico8_probe_comp.h` | `src/game/` | `targeting.h` | `.h` seul ; composant `pico8_probe_t` → `targeting_t` |
| `src/pico8_research.c` | `src/game/` | `research.c` |  |
| `src/pico8_research.h` | `src/game/` | `research.h` |  |
| `src/pico8_research_display.c` | `src/game/` | `research_display.c` |  |
| `src/pico8_research_display.h` | `src/game/` | `research_display.h` |  |
| `src/pico8_research_icon.c` | `src/game/` | `structure_icon.c` | `ie` → `structure_icon` ; le nom actuel dit `research_icon` |
| `src/pico8_research_icon.h` | `src/game/` | `structure_icon.h` | `ie` → `structure_icon` ; le nom actuel dit `research_icon` |
| `src/pico8_score_panel.c` | `src/game/` | `score_panel.c` |  |
| `src/pico8_score_panel.h` | `src/game/` | `score_panel.h` |  |
| `src/pico8_score_panel_display.c` | `src/game/` | `score_panel_display.c` |  |
| `src/pico8_score_panel_display.h` | `src/game/` | `score_panel_display.h` |  |
| `src/pico8_scroll.c` | `src/game/` | `scroll.c` |  |
| `src/pico8_scroll.h` | `src/game/` | `scroll.h` |  |
| `src/pico8_slipway.c` | `src/game/` | `slipway.c` |  |
| `src/pico8_slipway.h` | `src/game/` | `slipway.h` |  |
| `src/pico8_slipway_display.c` | `src/game/` | `slipway_display.c` |  |
| `src/pico8_slipway_display.h` | `src/game/` | `slipway_display.h` |  |
| `src/pico8_tech_gate.c` | `src/game/` | `tech_gate.c` |  |
| `src/pico8_tech_gate.h` | `src/game/` | `tech_gate.h` |  |
| `src/pico8_trade.c` | `src/game/` | `trade.c` | type `trade_route_t` : module `trade_route` pour que préfixe et fichier coïncident ? |
| `src/pico8_trade.h` | `src/game/` | `trade.h` | type `trade_route_t` : module `trade_route` pour que préfixe et fichier coïncident ? |
| `src/pico8_ui.c` | `src/game/` | `ui.c` |  |
| `src/pico8_ui.h` | `src/game/` | `ui.h` |  |
| `src/pico8_ui_button.c` | `src/game/` | `ui_button.c` | `js` → `button` ; nom de §3 gardé |
| `src/pico8_ui_button.h` | `src/game/` | `ui_button.h` | `js` → `button` ; nom de §3 gardé |
| `src/pico8_wormhole.c` | `src/game/` | `wormhole.c` |  |
| `src/pico8_wormhole.h` | `src/game/` | `wormhole.h` |  |
| `src/pico8_wormhole_display.c` | `src/game/` | `wormhole_display.c` |  |
| `src/pico8_wormhole_display.h` | `src/game/` | `wormhole_display.h` |  |
| `src/pico8_zone_interface.c` | `src/game/` | `ui_zone.c` | hors liste §3 ; ses sortes sont des classes du jeu (`js`, `fq`, `mh`…), d'où `game/` et pas `engine/` |
| `src/pico8_zone_interface.h` | `src/game/` | `ui_zone.h` | hors liste §3 ; ses sortes sont des classes du jeu (`js`, `fq`, `mh`…), d'où `game/` et pas `engine/` |
| `src/pico8_zone_particules.c` | `src/game/` | `particle_zone.c` | hors liste §3 ; ses sortes sont du jeu (`qy`, `ga`, traînées de slipway), d'où `game/` |
| `src/pico8_zone_particules.h` | `src/game/` | `particle_zone.h` | hors liste §3 ; ses sortes sont du jeu (`qy`, `ga`, traînées de slipway), d'où `game/` |

### `src/platform/` (5)

| chemin actuel | répertoire cible | nom cible | doute |
|---|---|---|---|
| `src/pico8_fatal_pc.c` | `src/platform/` | `fatal_pc.c` |  |
| `src/pico8_raylib_backend.c` | `src/platform/` | `raylib_backend.c` |  |
| `src/pico8_raylib_backend.h` | `src/platform/` | `raylib_backend.h` |  |
| `src/pico8_raylib_backend_coords.c` | `src/platform/` | `raylib_backend_coords.c` |  |
| `src/pico8_raylib_backend_coords.h` | `src/platform/` | `raylib_backend_coords.h` |  |

### `src/devtools/` (21)

| chemin actuel | répertoire cible | nom cible | doute |
|---|---|---|---|
| `src/pico8_coverage.c` | `src/devtools/` | `class_coverage.c` | §3 dit `coverage` ; nom aligné sur le préfixe `pico8_class_coverage_*` |
| `src/pico8_coverage.h` | `src/devtools/` | `class_coverage.h` | §3 dit `coverage` ; nom aligné sur le préfixe `pico8_class_coverage_*` |
| `src/pico8_dump.c` | `src/devtools/` | `dump.c` |  |
| `src/pico8_dump.h` | `src/devtools/` | `dump.h` |  |
| `src/pico8_fixed_test_double.c` | `src/devtools/` | `fixed_test_double.c` |  |
| `src/pico8_fixed_test_double.h` | `src/devtools/` | `fixed_test_double.h` |  |
| `src/pico8_gif.c` | `src/devtools/` | `gif.c` |  |
| `src/pico8_gif.h` | `src/devtools/` | `gif.h` |  |
| `src/pico8_input_json.c` | `src/devtools/` | `input_json.c` | `.c` seul : définit des fonctions déclarées dans `p8/input.h` |
| `src/pico8_minimize.c` | `src/devtools/` | `minimize.c` |  |
| `src/pico8_minimize.h` | `src/devtools/` | `minimize.h` |  |
| `src/pico8_replay_control.c` | `src/devtools/` | `replay_control.c` |  |
| `src/pico8_replay_control.h` | `src/devtools/` | `replay_control.h` |  |
| `src/pico8_replay_source.c` | `src/devtools/` | `replay_source.c` |  |
| `src/pico8_replay_source.h` | `src/devtools/` | `replay_source.h` |  |
| `src/pico8_screencast.c` | `src/devtools/` | `screencast.c` |  |
| `src/pico8_screencast.h` | `src/devtools/` | `screencast.h` |  |
| `src/pico8_screencast_player.c` | `src/devtools/` | `screencast_player.c` |  |
| `src/pico8_screencast_player.h` | `src/devtools/` | `screencast_player.h` |  |
| `src/pico8_session_recorder.c` | `src/devtools/` | `session_recorder.c` |  |
| `src/pico8_session_recorder.h` | `src/devtools/` | `session_recorder.h` |  |

### `src/cmd/` (7)

| chemin actuel | répertoire cible | nom cible | doute |
|---|---|---|---|
| `src/main.c` | `src/cmd/` | `replay.c` | pilote de rejeu sans fenêtre ; binaire `pico8_replay` |
| `src/main_class_coverage.c` | `src/cmd/` | `class_coverage.c` |  |
| `src/main_frames.c` | `src/cmd/` | `frames.c` |  |
| `src/main_gif.c` | `src/cmd/` | `gif.c` |  |
| `src/main_minimize.c` | `src/cmd/` | `minimize.c` |  |
| `src/main_raylib.c` | `src/cmd/` | `game.c` | pilote interactif ; binaire `main_raylib` |
| `src/main_screencast_viewer.c` | `src/cmd/` | `screencast_viewer.c` |  |

### `tests/` (92)

| chemin actuel | répertoire cible | nom cible | doute |
|---|---|---|---|
| `tests/test_alloc_guard.c` | `tests/` | `test_alloc_guard.c` | conservé |
| `tests/test_asset_loading.c` | `tests/` | `test_asset_loading.c` | conservé |
| `tests/test_class_coverage.c` | `tests/` | `test_class_coverage.c` | conservé |
| `tests/test_composition_residue.c` | `tests/` | `test_composition_residue.c` | conservé |
| `tests/test_constants_check.c` | `tests/` | `test_constants_check.c` | conservé |
| `tests/test_data_residue.c` | `tests/` | `test_data_residue.c` | conservé |
| `tests/test_e2e_cleanup.c` | `tests/` | `test_e2e_cleanup.c` | conservé |
| `tests/test_e2e_limits.c` | `tests/` | `test_e2e_limits.c` | conservé |
| `tests/test_float_grep.c` | `tests/` | `test_float_grep.c` | conservé |
| `tests/test_float_guard.c` | `tests/` | `test_float_guard.c` | conservé |
| `tests/test_float_link.c` | `tests/` | `test_float_link.c` | conservé |
| `tests/test_frames_check.c` | `tests/` | `test_frames_check.c` | conservé |
| `tests/test_line_coverage.c` | `tests/` | `test_line_coverage.c` | conservé |
| `tests/test_pico8_bg.c` | `tests/` | `test_background.c` |  |
| `tests/test_pico8_bg_kj.c` | `tests/` | `test_background_draw.c` |  |
| `tests/test_pico8_data_init.c` | `tests/` | `test_data_init.c` |  |
| `tests/test_pico8_demo.c` | `tests/` | `test_demo.c` |  |
| `tests/test_pico8_dump.c` | `tests/` | `test_dump.c` |  |
| `tests/test_pico8_entity_kind.c` | `tests/` | `test_entity_kind.c` |  |
| `tests/test_pico8_ents.c` | `tests/` | `test_entity_list.c` |  |
| `tests/test_pico8_file_sortie.c` | `tests/` | `test_exit_queue.c` |  |
| `tests/test_pico8_fixed.c` | `tests/` | `test_fixed.c` |  |
| `tests/test_pico8_fm.c` | `tests/` | `test_main_economy.c` | suit le doute `main_` des singletons |
| `tests/test_pico8_fq.c` | `tests/` | `test_modal_backdrop.c` |  |
| `tests/test_pico8_fr.c` | `tests/` | `test_infraspace_gate.c` |  |
| `tests/test_pico8_frames_tool.c` | `tests/` | `test_frames_tool.c` |  |
| `tests/test_pico8_fv.c` | `tests/` | `test_help_screen.c` |  |
| `tests/test_pico8_g_.c` | `tests/` | `test_probe.c` |  |
| `tests/test_pico8_ga.c` | `tests/` | `test_burst.c` |  |
| `tests/test_pico8_ga_export_panel_by.c` | `tests/` | `test_burst_exports_row.c` | `by` (routes reçues par `ga`) et `jo:gh` hors vocabulaire |
| `tests/test_pico8_geom.c` | `tests/` | `test_geom.c` |  |
| `tests/test_pico8_gfx.c` | `tests/` | `test_gfx.c` |  |
| `tests/test_pico8_gif.c` | `tests/` | `test_gif.c` |  |
| `tests/test_pico8_headless_capture.c` | `tests/` | `test_headless_capture.c` |  |
| `tests/test_pico8_hj.c` | `tests/` | `test_ascension.c` |  |
| `tests/test_pico8_hp.c` | `tests/` | `test_research_project.c` |  |
| `tests/test_pico8_hy.c` | `tests/` | `test_economy.c` |  |
| `tests/test_pico8_ie.c` | `tests/` | `test_structure_icon.c` |  |
| `tests/test_pico8_input.c` | `tests/` | `test_input.c` |  |
| `tests/test_pico8_iv.c` | `tests/` | `test_toast.c` |  |
| `tests/test_pico8_jb_ol.c` | `tests/` | `test_tooltip_draw_at.c` |  |
| `tests/test_pico8_jc.c` | `tests/` | `test_slipway.c` |  |
| `tests/test_pico8_je.c` | `tests/` | `test_main_mouse.c` | suit le doute `main_` des singletons |
| `tests/test_pico8_jo.c` | `tests/` | `test_planet.c` |  |
| `tests/test_pico8_jo_spec_defaults.c` | `tests/` | `test_planet_spec_defaults.c` |  |
| `tests/test_pico8_js.c` | `tests/` | `test_button.c` |  |
| `tests/test_pico8_layout_nil_item_draws_nothing.c` | `tests/` | `test_layout_nil_item_draws_nothing.c` |  |
| `tests/test_pico8_lq.c` | `tests/` | `test_layout_row.c` |  |
| `tests/test_pico8_map_tables.c` | `tests/` | `test_map_tables.c` |  |
| `tests/test_pico8_marker_coherence.c` | `tests/` | `test_marker_coherence.c` |  |
| `tests/test_pico8_mem.c` | `tests/` | `test_mem.c` |  |
| `tests/test_pico8_mh.c` | `tests/` | `test_message_box.c` |  |
| `tests/test_pico8_minimize.c` | `tests/` | `test_minimize.c` |  |
| `tests/test_pico8_mo.c` | `tests/` | `test_main_camera.c` | suit le doute `main_` des singletons |
| `tests/test_pico8_mx.c` | `tests/` | `test_synthesizer.c` |  |
| `tests/test_pico8_mz.c` | `tests/` | `test_wormhole.c` |  |
| `tests/test_pico8_nf.c` | `tests/` | `test_mouse.c` |  |
| `tests/test_pico8_nl.c` | `tests/` | `test_trading_hub.c` |  |
| `tests/test_pico8_nr.c` | `tests/` | `test_slipgate.c` |  |
| `tests/test_pico8_os.c` | `tests/` | `test_processor.c` |  |
| `tests/test_pico8_pj.c` | `tests/` | `test_score_panel.c` |  |
| `tests/test_pico8_places_vivantes.c` | `tests/` | `test_live_slots.c` |  |
| `tests/test_pico8_planet_gh_row_lq.c` | `tests/` | `test_planet_exports_row.c` | `jo:gh` hors vocabulaire |
| `tests/test_pico8_planet_kj_dispatch.c` | `tests/` | `test_draw_layers_camera.c` | teste `dc("kj")` et `mo:md`, pas la planète |
| `tests/test_pico8_pm.c` | `tests/` | `test_protostar.c` |  |
| `tests/test_pico8_probe_mm_rnd_order.c` | `tests/` | `test_probe_spawn_particle_order.c` |  |
| `tests/test_pico8_ptrs.c` | `tests/` | `test_pointer_list.c` |  |
| `tests/test_pico8_qb.c` | `tests/` | `test_camera.c` |  |
| `tests/test_pico8_qh.c` | `tests/` | `test_structure.c` | suit le doute `structure`/`laboratory` de `qh` |
| `tests/test_pico8_raylib_backend_coords.c` | `tests/` | `test_raylib_backend_coords.c` |  |
| `tests/test_pico8_recording_continuity.c` | `tests/` | `test_recording_continuity.c` |  |
| `tests/test_pico8_replay_control.c` | `tests/` | `test_replay_control.c` |  |
| `tests/test_pico8_replay_from_title.c` | `tests/` | `test_replay_from_title.c` |  |
| `tests/test_pico8_replay_source.c` | `tests/` | `test_replay_source.c` |  |
| `tests/test_pico8_replay_usage.c` | `tests/` | `test_replay_usage.c` |  |
| `tests/test_pico8_screencast.c` | `tests/` | `test_screencast.c` |  |
| `tests/test_pico8_screencast_player.c` | `tests/` | `test_screencast_player.c` |  |
| `tests/test_pico8_spec_defaults.c` | `tests/` | `test_spec_defaults.c` |  |
| `tests/test_pico8_str.c` | `tests/` | `test_str.c` |  |
| `tests/test_pico8_struct_header.c` | `tests/` | `test_entity_header.c` |  |
| `tests/test_pico8_trade.c` | `tests/` | `test_trade.c` |  |
| `tests/test_pico8_ui.c` | `tests/` | `test_ui.c` |  |
| `tests/test_pico8_vec.c` | `tests/` | `test_vec.c` |  |
| `tests/test_pico8_zone.c` | `tests/` | `test_zone.c` |  |
| `tests/test_pico8_zone_arene.c` | `tests/` | `test_arena.c` |  |
| `tests/test_pico8_zone_choisie.c` | `tests/` | `test_allocator.c` |  |
| `tests/test_pico8_zone_frame.c` | `tests/` | `test_frame_zone.c` |  |
| `tests/test_pico8_zone_interface.c` | `tests/` | `test_ui_zone.c` |  |
| `tests/test_pico8_zone_particules.c` | `tests/` | `test_particle_zone.c` |  |
| `tests/test_pico8_zone_partie.c` | `tests/` | `test_game_zone.c` |  |
| `tests/zone_tas.c` | `tests/` | `heap_allocator.c` | pas un test : l'`allocator_t` des TU adossé à `malloc` |
| `tests/zone_tas.h` | `tests/` | `heap_allocator.h` | pas un test : l'`allocator_t` des TU adossé à `malloc` |

### `tests/e2e/` (79)

| chemin actuel | répertoire cible | nom cible | doute |
|---|---|---|---|
| `tests/e2e/.gitkeep` | `tests/e2e/` | `.gitkeep` | conservé |
| `tests/e2e/camera_drag_dpad.json` | `tests/e2e/` | `camera_drag_dpad.json` | conservé |
| `tests/e2e/camera_drag_dpad.json.sha256` | `tests/e2e/` | `camera_drag_dpad.json.sha256` | conservé |
| `tests/e2e/camera_drag_dpad.txt` | `tests/e2e/` | `camera_drag_dpad.txt` | conservé |
| `tests/e2e/cheat_code.json` | `tests/e2e/` | `cheat_code.json` | conservé |
| `tests/e2e/cheat_code.json.sha256` | `tests/e2e/` | `cheat_code.json.sha256` | conservé |
| `tests/e2e/cheat_code.txt` | `tests/e2e/` | `cheat_code.txt` | conservé |
| `tests/e2e/colony_income_offscreen.json` | `tests/e2e/` | `colony_income_offscreen.json` | conservé |
| `tests/e2e/colony_income_offscreen.json.sha256` | `tests/e2e/` | `colony_income_offscreen.json.sha256` | conservé |
| `tests/e2e/colony_income_offscreen.txt` | `tests/e2e/` | `colony_income_offscreen.txt` | conservé |
| `tests/e2e/colony_panel.json` | `tests/e2e/` | `colony_panel.json` | conservé |
| `tests/e2e/colony_panel.json.sha256` | `tests/e2e/` | `colony_panel.json.sha256` | conservé |
| `tests/e2e/colony_panel.txt` | `tests/e2e/` | `colony_panel.txt` | conservé |
| `tests/e2e/empty_planet_fade.json` | `tests/e2e/` | `empty_planet_fade.json` | conservé |
| `tests/e2e/empty_planet_fade.json.sha256` | `tests/e2e/` | `empty_planet_fade.json.sha256` | conservé |
| `tests/e2e/empty_planet_fade.txt` | `tests/e2e/` | `empty_planet_fade.txt` | conservé |
| `tests/e2e/game_end_bankrupt.json` | `tests/e2e/` | `game_end_bankrupt.json` | conservé |
| `tests/e2e/game_end_bankrupt.json.sha256` | `tests/e2e/` | `game_end_bankrupt.json.sha256` | conservé |
| `tests/e2e/game_end_bankrupt.txt` | `tests/e2e/` | `game_end_bankrupt.txt` | conservé |
| `tests/e2e/game_end_final_score.json` | `tests/e2e/` | `game_end_final_score.json` | conservé |
| `tests/e2e/game_end_final_score.json.sha256` | `tests/e2e/` | `game_end_final_score.json.sha256` | conservé |
| `tests/e2e/game_end_final_score.txt` | `tests/e2e/` | `game_end_final_score.txt` | conservé |
| `tests/e2e/message_box_hold.json` | `tests/e2e/` | `message_box_hold.json` | conservé |
| `tests/e2e/message_box_hold.json.sha256` | `tests/e2e/` | `message_box_hold.json.sha256` | conservé |
| `tests/e2e/message_box_hold.txt` | `tests/e2e/` | `message_box_hold.txt` | conservé |
| `tests/e2e/pause_menu_rows.json` | `tests/e2e/` | `pause_menu_rows.json` | conservé |
| `tests/e2e/pause_menu_rows.json.sha256` | `tests/e2e/` | `pause_menu_rows.json.sha256` | conservé |
| `tests/e2e/pause_menu_rows.txt` | `tests/e2e/` | `pause_menu_rows.txt` | conservé |
| `tests/e2e/pause_restart.json` | `tests/e2e/` | `pause_restart.json` | conservé |
| `tests/e2e/pause_restart.json.sha256` | `tests/e2e/` | `pause_restart.json.sha256` | conservé |
| `tests/e2e/pause_restart.txt` | `tests/e2e/` | `pause_restart.txt` | conservé |
| `tests/e2e/place_processor_ascension.json` | `tests/e2e/` | `place_processor_ascension.json` | conservé |
| `tests/e2e/place_processor_ascension.json.sha256` | `tests/e2e/` | `place_processor_ascension.json.sha256` | conservé |
| `tests/e2e/place_processor_ascension.txt` | `tests/e2e/` | `place_processor_ascension.txt` | conservé |
| `tests/e2e/project_tooltip_inputs.json` | `tests/e2e/` | `project_tooltip_inputs.json` | conservé |
| `tests/e2e/project_tooltip_inputs.json.sha256` | `tests/e2e/` | `project_tooltip_inputs.json.sha256` | conservé |
| `tests/e2e/project_tooltip_inputs.txt` | `tests/e2e/` | `project_tooltip_inputs.txt` | conservé |
| `tests/e2e/slipgate_outgoing_first.json` | `tests/e2e/` | `slipgate_outgoing_first.json` | conservé |
| `tests/e2e/slipgate_outgoing_first.json.sha256` | `tests/e2e/` | `slipgate_outgoing_first.json.sha256` | conservé |
| `tests/e2e/slipgate_outgoing_first.txt` | `tests/e2e/` | `slipgate_outgoing_first.txt` | conservé |
| `tests/e2e/slipway_build.json` | `tests/e2e/` | `slipway_build.json` | conservé |
| `tests/e2e/slipway_build.json.sha256` | `tests/e2e/` | `slipway_build.json.sha256` | conservé |
| `tests/e2e/slipway_build.txt` | `tests/e2e/` | `slipway_build.txt` | conservé |
| `tests/e2e/slipway_refused.json` | `tests/e2e/` | `slipway_refused.json` | conservé |
| `tests/e2e/slipway_refused.json.sha256` | `tests/e2e/` | `slipway_refused.json.sha256` | conservé |
| `tests/e2e/slipway_refused.txt` | `tests/e2e/` | `slipway_refused.txt` | conservé |
| `tests/e2e/space_fold_same_spot.json` | `tests/e2e/` | `space_fold_same_spot.json` | conservé |
| `tests/e2e/space_fold_same_spot.json.sha256` | `tests/e2e/` | `space_fold_same_spot.json.sha256` | conservé |
| `tests/e2e/space_fold_same_spot.txt` | `tests/e2e/` | `space_fold_same_spot.txt` | conservé |
| `tests/e2e/synthesizer_second_link.json` | `tests/e2e/` | `synthesizer_second_link.json` | conservé |
| `tests/e2e/synthesizer_second_link.json.sha256` | `tests/e2e/` | `synthesizer_second_link.json.sha256` | conservé |
| `tests/e2e/synthesizer_second_link.txt` | `tests/e2e/` | `synthesizer_second_link.txt` | conservé |
| `tests/e2e/tech_geoharvesting_burst.json` | `tests/e2e/` | `tech_geoharvesting_burst.json` | conservé |
| `tests/e2e/tech_geoharvesting_burst.json.sha256` | `tests/e2e/` | `tech_geoharvesting_burst.json.sha256` | conservé |
| `tests/e2e/tech_geoharvesting_burst.txt` | `tests/e2e/` | `tech_geoharvesting_burst.txt` | conservé |
| `tests/e2e/tech_infraspace_infragate.json` | `tests/e2e/` | `tech_infraspace_infragate.json` | conservé |
| `tests/e2e/tech_infraspace_infragate.json.sha256` | `tests/e2e/` | `tech_infraspace_infragate.json.sha256` | conservé |
| `tests/e2e/tech_infraspace_infragate.txt` | `tests/e2e/` | `tech_infraspace_infragate.txt` | conservé |
| `tests/e2e/tech_nanomaterials_discount.json` | `tests/e2e/` | `tech_nanomaterials_discount.json` | conservé |
| `tests/e2e/tech_nanomaterials_discount.json.sha256` | `tests/e2e/` | `tech_nanomaterials_discount.json.sha256` | conservé |
| `tests/e2e/tech_nanomaterials_discount.txt` | `tests/e2e/` | `tech_nanomaterials_discount.txt` | conservé |
| `tests/e2e/tech_replication_mechanize.json` | `tests/e2e/` | `tech_replication_mechanize.json` | conservé |
| `tests/e2e/tech_replication_mechanize.json.sha256` | `tests/e2e/` | `tech_replication_mechanize.json.sha256` | conservé |
| `tests/e2e/tech_replication_mechanize.txt` | `tests/e2e/` | `tech_replication_mechanize.txt` | conservé |
| `tests/e2e/tech_slipgates_relay.json` | `tests/e2e/` | `tech_slipgates_relay.json` | conservé |
| `tests/e2e/tech_slipgates_relay.json.sha256` | `tests/e2e/` | `tech_slipgates_relay.json.sha256` | conservé |
| `tests/e2e/tech_slipgates_relay.txt` | `tests/e2e/` | `tech_slipgates_relay.txt` | conservé |
| `tests/e2e/tech_starbirth_protostar.json` | `tests/e2e/` | `tech_starbirth_protostar.json` | conservé |
| `tests/e2e/tech_starbirth_protostar.json.sha256` | `tests/e2e/` | `tech_starbirth_protostar.json.sha256` | conservé |
| `tests/e2e/tech_starbirth_protostar.txt` | `tests/e2e/` | `tech_starbirth_protostar.txt` | conservé |
| `tests/e2e/tech_trade_league_hub.json` | `tests/e2e/` | `tech_trade_league_hub.json` | conservé |
| `tests/e2e/tech_trade_league_hub.json.sha256` | `tests/e2e/` | `tech_trade_league_hub.json.sha256` | conservé |
| `tests/e2e/tech_trade_league_hub.txt` | `tests/e2e/` | `tech_trade_league_hub.txt` | conservé |
| `tests/e2e/tech_xenofoods_recipes.json` | `tests/e2e/` | `tech_xenofoods_recipes.json` | conservé |
| `tests/e2e/tech_xenofoods_recipes.json.sha256` | `tests/e2e/` | `tech_xenofoods_recipes.json.sha256` | conservé |
| `tests/e2e/tech_xenofoods_recipes.txt` | `tests/e2e/` | `tech_xenofoods_recipes.txt` | conservé |
| `tests/e2e/trade_rich_mine.json` | `tests/e2e/` | `trade_rich_mine.json` | conservé |
| `tests/e2e/trade_rich_mine.json.sha256` | `tests/e2e/` | `trade_rich_mine.json.sha256` | conservé |
| `tests/e2e/trade_rich_mine.txt` | `tests/e2e/` | `trade_rich_mine.txt` | conservé |

### `tests/fixtures/` (15)

| chemin actuel | répertoire cible | nom cible | doute |
|---|---|---|---|
| `tests/fixtures/demo_events.json` | `tests/fixtures/` | `demo_events.json` | conservé |
| `tests/fixtures/events_malformed_five_dpad.json` | `tests/fixtures/` | `events_malformed_five_dpad.json` | conservé |
| `tests/fixtures/events_malformed_missing_field.json` | `tests/fixtures/` | `events_malformed_missing_field.json` | conservé |
| `tests/fixtures/events_malformed_no_frames_key.json` | `tests/fixtures/` | `events_malformed_no_frames_key.json` | conservé |
| `tests/fixtures/events_malformed_short_dpad.json` | `tests/fixtures/` | `events_malformed_short_dpad.json` | conservé |
| `tests/fixtures/events_valid.json` | `tests/fixtures/` | `events_valid.json` | conservé |
| `tests/fixtures/events_valid_ox.json` | `tests/fixtures/` | `events_valid_ox.json` | conservé |
| `tests/fixtures/pliage_abandonne.json` | `tests/fixtures/` | `wormhole_abandoned.json` |  |
| `tests/fixtures/pliages.json` | `tests/fixtures/` | `wormholes.json` |  |
| `tests/fixtures/prises_refusees.json` | `tests/fixtures/` | `grab_arenas_refused.json` |  |
| `tests/fixtures/screencast_capture_events.json` | `tests/fixtures/` | `screencast_capture_events.json` | conservé |
| `tests/fixtures/slipways_refuses.json` | `tests/fixtures/` | `slipways_refused.json` |  |
| `tests/fixtures/sondes_refusees.json` | `tests/fixtures/` | `probes_refused.json` |  |
| `tests/fixtures/title_click_forgiving.json` | `tests/fixtures/` | `title_click_forgiving.json` | conservé |
| `tests/fixtures/title_click_reasonable.json` | `tests/fixtures/` | `title_click_reasonable.json` | conservé |

### `tools/` (32)

| chemin actuel | répertoire cible | nom cible | doute |
|---|---|---|---|
| `tools/alloc_guard.sh` | `tools/` | `alloc_guard.sh` | conservé |
| `tools/alloc_guard_exceptions.txt` | `tools/` | `alloc_guard_exceptions.txt` | conservé |
| `tools/atteinte_a_prouver.txt` | `tools/` | `reachability_questions.txt` |  |
| `tools/class_coverage.sh` | `tools/` | `class_coverage.sh` | conservé |
| `tools/class_coverage_exempt.txt` | `tools/` | `class_coverage_exempt.txt` | conservé |
| `tools/class_coverage_net_expected.txt` | `tools/` | `class_coverage_net_expected.txt` | conservé |
| `tools/composition_r2_sites.txt` | `tools/` | `composition_residue_sites.txt` | `r2` lu comme la deuxième passe de `composition_residue` : à confirmer |
| `tools/composition_residue.sh` | `tools/` | `composition_residue.sh` | conservé |
| `tools/constants_check.sh` | `tools/` | `constants_check.sh` | conservé |
| `tools/constants_compare.sh` | `tools/` | `constants_compare.sh` | conservé |
| `tools/constants_expected_ecarts.txt` | `tools/` | `constants_expected_gaps.txt` | `écart` → `gap`, comme `TROU-N` → `GAP-N` |
| `tools/data_residue.sh` | `tools/` | `data_residue.sh` | conservé |
| `tools/e2e_cleanup.sh` | `tools/` | `e2e_cleanup.sh` | conservé |
| `tools/e2e_limits.sh` | `tools/` | `e2e_limits.sh` | conservé |
| `tools/e2e_long_exceptions.txt` | `tools/` | `e2e_long_exceptions.txt` | conservé |
| `tools/float_grep.sh` | `tools/` | `float_grep.sh` | conservé |
| `tools/float_grep_exclusions.txt` | `tools/` | `float_grep_exclusions.txt` | conservé |
| `tools/float_guard.sh` | `tools/` | `float_guard.sh` | conservé |
| `tools/float_guard.txt` | `tools/` | `float_guard.txt` | conservé |
| `tools/float_link.sh` | `tools/` | `float_link.sh` | conservé |
| `tools/frames_check.sh` | `tools/` | `frames_check.sh` | conservé |
| `tools/frames_ref.sh` | `tools/` | `frames_ref.sh` | conservé |
| `tools/gen_quarter_wave.c` | `tools/` | `gen_quarter_wave.c` | conservé |
| `tools/install_psn00bsdk_toolchain.sh` | `tools/` | `install_psn00bsdk_toolchain.sh` | conservé |
| `tools/install_raylib_vendor.sh` | `tools/` | `install_raylib_vendor.sh` | conservé |
| `tools/line_coverage.sh` | `tools/` | `line_coverage.sh` | conservé |
| `tools/line_coverage_reference.txt` | `tools/` | `line_coverage_reference.txt` | conservé |
| `tools/line_coverage_unreachable.txt` | `tools/` | `line_coverage_unreachable.txt` | conservé |
| `tools/minimize.sh` | `tools/` | `minimize.sh` | conservé |
| `tools/probe.p8` | `tools/` | `pico8_reference_probe.p8` | `probe` nomme désormais la sonde `g_` ; le préfixe `pico8_` dit ici le vrai PICO-8, pas le port |
| `tools/probe_out.txt` | `tools/` | `pico8_reference_probe_out.txt` | idem `probe.p8` |
| `tools/ttf_to_pixelfont.py` | `tools/` | `ttf_to_pixelfont.py` | conservé |
