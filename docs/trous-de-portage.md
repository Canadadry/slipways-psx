# Trous de portage

Les écarts connus entre ce port C et le cart PICO-8 d'origine, et les
morceaux volontairement non faits. Le code y renvoie par un commentaire
`/* GAP-n */` à l'endroit concerné, sans répéter l'explication.

Chaque entrée est décrite avec les noms C et les chemins d'aujourd'hui ;
les noms du cart et ceux de l'API PICO-8 qu'elle cite sont rassemblés sur
sa ligne « Lua : », sous le titre.

Chaque entrée a été revérifiée contre le code au moment de la reprise des
commentaires. Une entrée supprimée est une entrée refermée : voir git.

---

## GAP-1 — `sin`/`cos` ne sont pas bit-exacts hors des 64 points relevés

Lua : `sin`, `cos`.

`p8_fixed_cos`/`p8_fixed_sin` lisent une table de quart d'onde 16.16
(`p8_quarter_wave` de `src/p8/quarter_wave.h`, 16 385 entrées, une par
fraction de tour r/65536 du premier quart, sans interpolation), générée sur
l'hôte par `tools/gen_quarter_wave.c` (`make sin-table`). Elles sont exactes
au bit près sur les 64 points i/64 du relevé
(`tools/pico8_reference_probe_out.txt`, bloc E, asserté par `make test`).
Ailleurs, elles rendent l'arrondi au plus proche de la valeur vraie, ce que
rendait l'ancien calcul `double` de la libc, **sans source PICO-8** :
l'approximation propre de PICO-8 n'est pas publiée.

Le générateur pseudo-aléatoire relève de GAP-17.

Conséquence : une partie rejouée ici peut diverger d'une partie rejouée
dans le vrai PICO-8. Les rejeux internes restent déterministes,
puisqu'ils comparent ce port à lui-même.

Fichiers : `src/p8/fixed.c`, `src/p8/quarter_wave.h`.

## GAP-2 — La deuxième manette est câblée à « rien »

Lua : `qb:ph()`, `btn(b) or btn(b,1)`, `btn(b,1)`.

`camera_update` teste le bouton du premier joueur (`p8_btn`) ou celui du
second, comme le cart. Ce port n'a aucune représentation d'un deuxième
joueur, ni dans `p8_input_t` (ses trames `p8_input_frame_t`) ni dans
`events.json`, et n'en aura pas : le bouton du second joueur (`btn_p2`)
vaut toujours faux.

Sans effet tant que personne ne branche une deuxième manette.

Fichiers : `src/game/scroll.c`, `src/p8/input.h` (le schéma de frame, qui n'a pas de second joueur), `src/platform/raylib_backend.c` (l'endroit où la capture le décide).

## GAP-3 — Les chaînes ne sont pas internées

Lua : `hy.invented`, `oj`, `jo:gg`, `ho()`, `r`, `hp.o`, `hp.dm`, `kw.qe`,
`{qe=r[c]}`.

Lua interne les littéraux : deux chaînes identiques y sont le même objet.
Ce port copie ou alloue ses chaînes (les données générées ont un tableau
distinct par jeton) : deux chaînes égales n'y sont donc pas le même
pointeur.

Contourné au cas par cas : `collect_contains_string` compare par contenu
(les chaînes de la liste `invented` de l'économie, pour
`tech_gate_is_unlocked`), et un module a sa propre comparaison par contenu
(`planet_is_colonized`). Toute nouvelle comparaison de chaînes doit choisir
explicitement.

**Hors de GAP-3 : les ressources.** Ce que le chargeur des tables de
données du cart route à travers la table des ressources (`output` et
`byproduct` d'un projet, donc `resource` d'une `production_t`) et les
besoins à ressource sont des nombres — l'indice d'une ressource —, pas des
chaînes : ils se comparent par valeur (`==` sur `p8_fixed_t`,
`resource_equal`).

Fichiers : `src/engine/collect.c`.

## GAP-4 — Le constructeur de champ ne retombe pas sur la table quand il rend faux

Lua : `oo()`, `oo(fn,a)`, `fn and fn(...) or a`, `fn(...)`, `'s'`, `spr`,
`s(...)`.

Dans le cart, la valeur d'un champ construit vaut celle du constructeur, ou
la table analysée quand le constructeur rend une valeur fausse. Ce port ne
testait que l'absence du constructeur (`NULL`) et rendait sinon son
résultat tel quel.

Devenu atteignable : le constructeur du sprite était bien enregistré depuis
l'initialisation des fonctions de dessin, et il rendait un pointeur nul,
donc une valeur fausse. Aucune spec du cart n'écrit de sprite construit
aujourd'hui — vérifié sur `pico8/lua.lua` — donc le trou est dormant, pas
actif. Il s'ouvrira le jour où une spec en écrira une.

Sans site : la fonction d'analyse de champ, le module des tables à clés
dynamiques et le constructeur du sprite sont supprimés ; le port n'analyse
plus de spec.

## GAP-5 — `update_entities` saute une entité quand une entité s'auto-supprime

Lua : `dg()`, `hv`.

`update_entities` parcourt la liste des entités vivantes
(`live_entities`) par index. Quand une entité se retire pendant sa propre
visite (`entity_unregister`), les suivantes glissent d'un cran et celle qui
prend sa place n'est pas visitée cette frame. Lua fait autrement.

Divergence assumée : une entité sautée est simplement traitée à la frame
suivante.

Fichier : `src/engine/bootstrap.c`.

## GAP-8 — Le tag `targeting` n'a aucun lecteur

Lua : `g_`, `jo`, `prober`, `cg("prober")`.

La sonde (`probe`), l'écran d'aide et toute la famille des planètes
s'enregistrent sous le tag `TAG_TARGETING`, mais rien ne fait jamais
`entities_by_tag("targeting")`. Conservé par fidélité, pas par utilité.

## GAP-9 — Pas de suppression de champ nommé

Lua : `t[nom]=nil`.

La table à clés dynamiques du port savait retirer un élément par indice,
pas un champ nommé : aucun site d'appel n'en a eu besoin. Effacer un champ
nommé n'a donc pas d'équivalent exact — écrire une valeur fausse n'est pas
la même chose qu'un champ absent.

Sans site : le module des tables à clés dynamiques est supprimé ; le port
n'a plus de table à clés dynamiques.

## GAP-10 — L'audio est hors périmètre

Lua : `sfx()`, `music()`, `ih`, `rn()`.

`p8_sfx` et `p8_music` sont des fonctions vides. Ce n'est pas un report :
le port n'aura pas de son.

Le reste de la chaîne, lui, fonctionne : le bouton « toggle sfx » du menu
pause (`pause_menu_row3_on_click_method`) bascule bien le drapeau de
sourdine (`p8_set_sfx_muted`, `p8_sfx_muted`), et `p8_play_sfx` le lit
avant de décider d'appeler `p8_sfx`. Seul l'appel final est inerte — donc
les deux boutons audio du menu pause répondent au clic sans rien produire.

Fichiers : `src/p8/audio.c`, `src/game/menu.c`.

## GAP-11 — Le repli d'arité du test de survol rendrait le survol silencieusement faux

Lua : `hd`, `"hd"`, `dc()`, `dc("hd")`, `bm`, `kj`, `ev`, `mp == rp`,
`rp = (argc >= 2) ? arg1 : arg0`.

Le test de survol d'un bouton recevait un nombre d'arguments et deux
positions, et prenait la position écran pour la seconde si le nombre était
au moins 2, la première sinon. Le seul site qui le dispatchait était
`ui_update_hover`, avec deux arguments en dur.

Mais la répartition par couches dispatchait n'importe quel message — dont
le test de survol — avec un seul argument. Rien ne le faisait ; le jour où
quelque chose le ferait, le repli comparerait `mouse_world` à elle-même au
lieu de la position écran `mouse_screen`. Faux, et silencieux.

Pas transformé en `assert` sur le nombre d'arguments : ça ferait sauter un
dispatch par ailleurs légitime. Le vrai correctif est de décider ce que le
test de survol doit faire sous la répartition par couches, ce qui dépasse
une reprise de commentaires.

Sans site : le membre `hit_test` d'`entity_class_t` a toujours deux
arguments (`mouse_world`, `mouse_screen`) et `draw_layers` ne lance que
`draw`/`draw_overlay` (`assert` sur tout autre message).

## GAP-12 — Le premier anneau du trou de ver est noir au lieu de garder la plume

Lua : `mz:kj`, `self.kk[flr(sr/self.oi*8)]`, `kk`, `kk[0]`, `self.t`, `dt`.

`wormhole_draw` colore chaque anneau par la rampe `color_ramp` de
l'archétype, à l'indice flr(sr/oi*8). L'indice tombe à 0 dès que sr < oi/8,
c'est-à-dire pour l'anneau le plus jeune, à chaque tick où le temps d'état
vient de dépasser un `dt`.

La rampe du cart est une table Lua indexée à partir de 1 : l'indice 0 y est
nil, et PICO-8 conserve alors la couleur de plume courante. Ce port
retombe sur 0 (`int_seq_at` laisse la couleur par défaut) et dessine en
noir. Ce n'est pas la même image.

Trouvé en reprenant les commentaires : le commentaire à cet endroit
affirmait l'inverse — « REAL, FAITHFULLY-REPRODUCED EDGE CASE ». L'écart
est réel, l'affirmation était fausse.

Pas transformé en `assert` : la condition est atteinte en permanence, et
c'est le comportement actuel que les rejeux enregistrent. Le corriger
changera les enregistrements existants, ce qui en fait une décision de
jeu, pas une reprise de commentaires.


**Prémisse probablement fausse** (établi pour GAP-20, même situation) :
l'indice 0 de la rampe est un nil **explicite**, que zepto8 et fake-08
convertissent en couleur 0 ; le noir du port serait alors fidèle. Non
vérifié sur le binaire PICO-8 — même vérification que GAP-20.

Fichier : `src/game/wormhole_display.c`.

## GAP-13 — `tostr` d'un champ absent rend la chaîne vide

Lua : `tostr`, `s[st.score]`, `score == 0`, `ky()`.

La rangée « score 0 » du panneau lit l'entrée 0 du score final, c'est-à-dire
un indice que `economy_score_final` ne pose jamais. Lua y verrait nil.

Ce port n'a pas de nil à mettre en chaîne : `score_panel_final_score_text`
rend `""` pour tout indice hors de 1..6. PICO-8, lui, a une représentation
dédiée pour nil dans `tostr` — donc la ligne ne s'affiche pas pareil.

Fixé par `test_scores_box_follows_is_shown` de `tests/test_message_box.c`
(`SCORES_SEQUENCE_HASH`).

Fichier : `src/game/score_panel_display.c`.

## GAP-14 — Une boucle qui supprime son propre élément en saute un

Lua : `ei()`, `qy`.

Même famille que GAP-5, à d'autres endroits et sur d'autres listes :
`emitter_update` parcourt les particules de son émetteur (`particles`) et en
supprime au milieu de sa propre boucle (`pointer_list_del`), sans compenser
l'indice. La particule qui prend la place de celle qui vient d'être retirée
n'est pas visitée cette frame. Le même motif existe dans l'affichage des
slipways.

Sans conséquence visible : une particule vit une frame de plus que prévu,
et elle sera visitée à la frame suivante. Consigné parce que c'est le
troisième site du même motif, et qu'un futur lecteur qui en corrige un
seul rendra les trois incohérents entre eux.

Fichiers : `src/game/particle.c`, `src/game/slipway_display.c`,
et `src/engine/bootstrap.c` pour la variante de GAP-5.

## GAP-15 — Là où Lua planterait, ce port ignore

Lua : `bd()`, `hg`, `lt`, `he`, `bm`, `dc`, `fa`, `ck()`.

Convention générale du portage, et non un oubli local : quand le cart
indexe une table absente, Lua lève une erreur. Ce port lit le champ comme
absent et continue. Les sites connus :

- `economy_tiers_rebuild_incoming_routes` écrit dans l'ensemble
  `served_planets` d'un slipway qui n'en a pas : écriture ignorée.
- `entity_register`/`entity_unregister` testent la **présence** du membre
  de message de la classe, pas sa véracité Lua : un membre posé à faux
  inscrit quand même l'entité.
- `entity_unregister` sur une étiquette jamais enregistrée ne fait rien.
- `draw_layers` écarte de toutes les couches une entité sans `layer`, ou
  dont le `layer` sort de 1..15.
- `economy_crowding_penalty` saute une planète dont la liste de besoins est
  absente ou vide.

Aucun de ces cas n'est atteint en partie normale — ils décrivent ce qui
arriverait sur une donnée malformée. Consignés parce qu'un lecteur qui en
« corrige » un seul en le rendant fatal rendrait le port moins tolérant
que le cart à un endroit et pas aux autres.

Fichiers : `src/game/economy_tiers.c`, `src/game/economy_formulas.c`,
`src/engine/entity.c`.

## GAP-16 — Le tirage pondéré lit hors bornes si son pool se vide

Lua : `gn()`, `gn_next()`, `oc.items[0]`, `df`, `bw(90)`, `jo:hh`,
`oc[flr(r()*0+1)]`, `until not gu(nm,e)`.

`weighted_picker_new` construit un pool de tickets **consommable** : chaque
`weighted_picker_next` retire le ticket tiré, et rien ne le regarnit. Quand
il est vide, `idx0` vaut 0 et le code lit le ticket 0 d'une liste vide —
`tickets` peut être `NULL`.

Les chiffres, calculés sur le littéral des types de planète du dépôt avec
`og` à 2 : **66 tickets**. `galaxy_populate` place 90 planètes, et
`planet_roll_type` tire un ticket pour chacune qui ne prend pas la branche
fixe (environ 55 % des cas). L'espérance reste sous 66, mais la queue de
distribution la franchit.

Trouvé en reprenant les commentaires : un agent voulait poser un `assert`
sur `tickets_count` positif et a calculé la marge avant de l'écrire —
l'assert aurait sauté. Ni le cart ni ce port ne gardent contre ce cas ; en
Lua, le tirage dans une liste vide vaut nil et la boucle de tirage tourne
indéfiniment, ce qui n'est pas mieux.

Fichier : `src/game/gen.c`.

## GAP-17 — Le générateur pseudo-aléatoire n'est pas celui de PICO-8

Lua : `rnd`.

`p8_fixed_rnd` tire d'un générateur maison (`rnd_next_u32`), pas de celui
de PICO-8. Aucune séquence ne coïncide.

Conséquence directe, et plus forte que GAP-1 : l'objectif acté
« même graine → même galaxie » **ne tient pas** face au vrai cart, et pas
seulement parce que le flottant dérive. Ici la première valeur tirée
diffère déjà.

Les rejeux internes restent déterministes — ce port se compare à
lui-même.

Fichier : `src/p8/fixed.c`.

## GAP-18 — Le remap de couleurs de `pal()` n'est pas porté

Lua : `pal()`, `dv()`.

`p8_spr` lit la couleur d'un pixel de la feuille de sprites, teste le
bit de transparence, puis l'écrit **telle quelle**. Le remap de couleurs
que `pal()` installe dans la palette de dessin n'est appliqué nulle part :
seul le bit 0x80 de l'octet de palette est lu.

Ce qui marche quand même : `hud_select_palette_block` sélectionne un bloc
de palette et le cart s'en sert surtout pour éteindre ou allumer des
couleurs, pas pour les permuter. Ce qui ne marche pas se verrait sur un
effet qui permute réellement deux couleurs.

Fichier : `src/p8/gfx.c`, marqué par un `TODO` sur place.

## GAP-19 — La police est reconstituée à la main

Les glyphes de PICO-8 appartiennent à son runtime, pas au cart : ils ne
sont dans aucun asset exportable. Ce port les redessine — 3×5 pixels dans
une cellule de 4 (`font_glyph_rows`) — et affiche un bloc plein pour tout
caractère non couvert.

Divergence visuelle permanente, assumée : la seule façon de la refermer
serait de relever les glyphes depuis PICO-8 lui-même.

Fichier : `src/p8/gfx.c`.

## GAP-20 — Un `nil` explicite comme couleur de `circ` n'est pas vérifié sur PICO-8

Lua : `ga:kj`, `circ(p.x,p.y,pt.p:qx(),self.k_[flr(pt.l/2)])`, `1 < l < 2`,
`k_`, `k_[0]`, `nil`, `circ`, `color(8) circ(64,64,10,nil)`, `rk`,
`rk_default`.

Dans `burst_draw`, chaque particule est tracée par `p8_circ`, de la couleur
de la rampe de traînée à l'indice flr(life/2) (lua.lua:1091). Les
particules de vie entre 1 et 2 donnent l'indice 0 ; la rampe du cart est
indexée à partir de 1, donc l'indice 0 y est nil et `circ` reçoit un
quatrième argument **nil explicite**, pas un argument omis. Ce port trace
alors en couleur 0 (noir).

Ce n'est probablement pas un écart. Le manuel ne dit rien d'un nil
explicite ; deux émulateurs le convertissent en 0 et posent aussi la plume
à 0 : zepto8 (lua_isnone seul marque l'argument absent, nil donne
lua_tonumber(nil) = 0) et fake-08 (lua_tonumber(L,4) dès qu'il y a plus
de 3 arguments). Le port trace donc comme eux ; il ne pose pas la plume,
sans effet visible ici.

Non vérifié sur le binaire PICO-8. La vérification : tracer un cercle de
couleur nil après avoir posé la plume à 8, dans une cartouche, et regarder
la couleur. Si PICO-8 gardait la plume, il faudrait d'abord un vrai état de
plume dans le port (voir la note sur `pen_color` plus bas) ; seules 22
frames de `geoharvest_burst` bougeraient.

Note liée : dans les deux émulateurs, **tout** tracé coloré met la plume à
jour, alors que `pen_color` du port n'est écrit que par `hud_draw_bar`. Un
tracé coloré entre un `hud_draw_bar` et un `hud_draw_bar_default` ferait
diverger le port. Non étudié.

Prémisse réfutée par diagnostic dans les émulateurs.

Fichier : `src/game/planet_family.c`.

## GAP-21 — La palette d'un éclat né d'une protoétoile est celle de la classe

Lua : `ga:kj`, `self.k_`, `k_`, `pm:pq()`, `ga`,
`k_=o(1,2,8,14,14,14,14,8,8,2,2,1,1)`, `k_=o(1,2,4,9,9,10,10,7,7,7,7,7)`.

`burst_draw` lit la rampe de traînée de l'instance quand elle en a une
(lua.lua:1091). `protostar_construct` en donne une à l'éclat qu'il crée
(1,2,8,14,14,14,14,8,8,2,2,1,1 ; lua.lua:1030, 1033), qui masque celle de la
classe (1,2,4,9,9,10,10,7,7,7,7,7 ; lua.lua:1077). Ce port lit la rampe de
la classe (`burst_trail_colors_seq`), jamais celle de l'instance : l'éclat
d'une protoétoile prend la palette de l'éclat ordinaire.

Probable, pas vérifié contre PICO-8. `protostar_construct` est exécuté par
l'e2e `tests/e2e/tech_starbirth_protostar.json` : son empreinte fige le
rendu du port, écart compris.

Fichier : `src/game/planet_family.c`.

## GAP-22 — `tostr` sous 0.0001 n'est pas sourcé

Lua : `tostr`.

`p8_tostr_fixed` écrit un nombre avec 4 décimales arrondies au pair,
puis retire les zéros de fin et le point nu. Cette règle reproduit 13 des
14 témoins du bloc C de `tools/pico8_reference_probe_out.txt`, dont
2/3 → `"0.6667"`.

Le 14ᵉ est le brut −6 (`0xffff.fffa`) : PICO-8 affiche `"-0"`, ce port
`"-0.0001"`. Aucune règle unique ne concilie ce témoin avec
2/3 → `"0.6667"`, donc elle n'est pas devinée. Ce port garde sa sortie :
brut −6 → `"-0.0001"`, bruts −1 à −3 → `"-0"`, bruts 1 à 3 → `"0"`. Pour
les autres magnitudes inférieures à 0.0001, la règle de PICO-8 est
inconnue.

Non atteint par le filet : aucune valeur affichée par le jeu n'a une
magnitude inférieure à 0.0001. Se tranche par une ligne de plus dans
`tools/pico8_reference_probe.p8`, lancée sur PICO-8.

Fichier : `src/p8/str.c`.

## GAP-23 — Une ligne de bouton qui porte son infobulle latérale ou son infobulle à nil a quand même sa pastille ou son infobulle

Lua : `h_`, `jb`, `js:pq`, `self.h_=self.h_ and lq(self.h_)`,
`self.jb=self.jb and jb(self.jb)`, `lq`, `jo:fe`, `b:jb()`, `hp:jb`, `ii`,
`jo:gh`, `kr`, `js`, `has_h_`, `has_jb`, `h_row`, `h_is_false`, `jb_row`.

Dans le cart, la construction d'un bouton ne construit rien pour une clé
nil. Ce port construit une rangée vide (`layout_row_new`), ou une
infobulle vide, dès que la ligne porte la clé, même à nil (GAP-9 : une clé
nommée écrite à nil reste présente). Seul `false` n'en construit pas.

Atteint par le filet : la ligne de `planet_projects_panel` d'un bâtiment
dont `research_project_tooltip` rend `NULL` (quand le projet a son
`icon_sprite`) reçoit une infobulle vide, dessinée au survol
(events_recorded_1790198294, session_events). La ligne de
`planet_inspection_panel` sans `net_income` recevrait une pastille vide ;
aucun rejeu ne l'atteint. La struct `button_t`, où nil vaut absent, garde
ce comportement : la présence d'une clé `side_tooltip` ou `tooltip` nil est
portée par `has_side_tooltip`/`has_tooltip` de `button_keys_t`, sans
`side_tooltip_row` ni `side_tooltip_is_false` (sans `tooltip_row`) ;
`button_construct` construit alors une rangée ou une infobulle vides. Fixé
par `test_nil_keys` de `tests/test_button.c`.

Fichier : `src/game/ui_button.c`.

## GAP-24 — `trade_extend` donne une ressource `false` à la route neuve quand la route n'a pas de ressource

Lua : `trade:kg`, `qe=false`, `self.qe`, `qe`, `trade`, `jo:cn`, `jo:ib`,
`kg`, `nr:cn`, `dh`, `kw`.

Dans le cart, l'allongement d'une route recopie sa ressource telle quelle :
nil reste nil, et aucune route n'a jamais une ressource `false`. Ce port
pose `false` quand la route allongée n'a pas de ressource. Dans la struct
`trade_route_t`, nil et absent ne font qu'un : une ressource nil, passée
explicitement ou omise, donne `false`.

`resource_t` garde les trois états et `resource_equal` est le `==` de Lua :
l'écart se voit là où une route rencontre un besoin sans ressource. Dans
`planet_offer_trade`, une route venue de `planet_rebuild_trades` sans
ressource (nil) répond à un tel besoin, comme dans le cart ; passée par
`trade_extend` (`slipgate_offer_trade`), elle vaut `false` et n'y répond
plus. Dans `economy_tiers_compute_tier`, la garde `has_resource` écarte en
plus la route nil que le cart compterait : une route sans nombre n'y compte
jamais.

Non atteint par les e2e : toute route qu'ils construisent a une ressource
non nil. `planet_rebuild_trades` passe pourtant une ressource nil quand la
`production` de la planète n'a pas de ressource. `tests/test_trade.c` fixe
les deux lectures.

Fichiers : `src/game/trade.c`, `src/game/planet_trade.c`,
`src/game/economy_tiers.c`.

## GAP-25 — La rangée « score 0 » du panneau de fin a un trou, le port en dessine les deux éléments

Lua : `o("",score=0,ra=v(64,21),c=5)`, `bz`, `pj:ph`, `st[3]`, `st[1]`, `lq`,
`lq:pq`, `nv`, `all(self)`, `#t`, `all`, `fs`, `w`, `a=0.5`, `pi.ld`, `[1]`,
`[3]`, `t[1]`.

La ligne de la rangée « score 0 » du panneau (lua.lua:803) n'a qu'un
positionnel, l'indice 1. `score_panel_update` (lua.lua:885-891) lui écrit
l'indice 3 (la chaîne de GAP-13), puis réécrit l'indice 1, et la passe à
une rangée de mise en page. La rangée porte donc les indices 1 et 3, sans
2.

Dans le cart, la construction de la rangée la parcourt jusqu'à sa bordure
(lua.lua:289-290). Le port, lui, prend toutes les clés d'indice dans
l'ordre croissant (la boucle de `score_panel_update` sur `has_value`) et
resserre le trou : le membre `items` de la rangée garde les deux éléments,
`placed_items` en a deux, et `width` compte les deux. Le centrage de la
rangée (alignement 0.5) en dépend. Atteint par le filet (`game_end`, 4
constructions).

Vérifié dans la source du cart : la ligne de la rangée, l'ordre des
écritures de `score_panel_update` et le parcours de la construction de la
rangée. Non vérifié sur PICO-8 : la bordure de cette table. Dans
l'implémentation de Lua 5.2, une table construite avec l'indice 1 puis
complétée de l'indice 3 a une bordure de 1, et le parcours ne verrait que
l'indice 1 ; PICO-8 peut rendre 3, l'autre bordure valide (l'indice 1
n'étant pas nil, 0 n'en est pas une).

Préexistant : l'instance de la rangée parcourait déjà toutes les clés
d'indice. La conversion en struct préserve l'écart.

Fichiers : `src/game/layout.c` (construction de la rangée,
`layout_row_construct`), `src/game/score_panel_display.c`.

## GAP-26 — La cible d'une planète hors écran est coupée quand l'objet tenu est rendu

Lua : `fo`, `jo:kj`, `self.fo=nil`, `gd()`, `ll`, `jc`, `dw`, `kj`, `np`.

`planet_draw` (lua.lua) sort avant d'effacer `targeted_by` quand la planète
est hors de l'écran (`entity_geom_is_on_screen`) ou à l'état
« vanishing ». Son `targeted_by` garde alors l'objet tenu (une structure
prise par une icône de recherche, un slipway) après que celui-ci a quitté
la partie ; si la planète revient à l'écran, la cartouche dessine un halo
de plus avec l'`attach_distance` de cet objet mort.

Le port rend l'objet par la file de sortie et recycle son arène : relire
`targeted_by` serait un usage après libération. `forget` (`src/game/cart.c`)
remet donc à nil le `targeted_by` de toute entité du tag `node` qui vise
l'objet rendu, et ce halo n'est pas dessiné. À l'état « vanishing », aucune
différence : dès t ≥ 60, `planet_draw` sort toujours avant de lire
`targeted_by`. Aucun e2e n'atteint ce cas.

Fichiers : `src/game/cart.c` (`forget`), `src/game/planet.c`
(`planet_forget_targeted_by`).
