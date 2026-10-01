# Trous de portage

Les écarts connus entre ce port C et le cart PICO-8 d'origine, et les
morceaux volontairement non faits. Le code y renvoie par un commentaire
`/* TROU-n */` à l'endroit concerné, sans répéter l'explication.

Chaque entrée a été revérifiée contre le code au moment de la reprise des
commentaires. Une entrée supprimée est une entrée refermée : voir git.

---

## TROU-1 — `sin`/`cos` ne sont pas bit-exacts hors des 64 points relevés

`pico8_fixed_cos`/`_sin` lisent une table de quart d'onde 16.16
(`src/pico8_quarter_wave.h`, 16 385 entrées, une par fraction de tour
`r/65536` du premier quart, sans interpolation), générée sur l'hôte par
`tools/gen_quarter_wave.c` (`make sin-table`). Elles sont exactes au bit
près sur les 64 points `i/64` du relevé (`tools/probe_out.txt`, bloc E,
asserté par `make test`). Ailleurs, elles rendent l'arrondi au plus proche
de la valeur vraie, ce que rendait l'ancien calcul `double` de la libc,
**sans source PICO-8** : l'approximation propre de PICO-8 n'est pas
publiée.

Le générateur pseudo-aléatoire relève de TROU-17.

Conséquence : une partie rejouée ici peut diverger d'une partie rejouée
dans le vrai PICO-8. Les rejeux internes restent déterministes,
puisqu'ils comparent ce port à lui-même.

Fichiers : `src/pico8_fixed.c`, `src/pico8_quarter_wave.h`.

## TROU-2 — La deuxième manette est câblée à « rien »

`qb:ph()` teste `btn(b) or btn(b,1)`. Ce port n'a aucune représentation
d'un deuxième joueur, ni dans `pico8_input_t` ni dans `events.json`, et
n'en aura pas : `btn(b,1)` rend toujours faux.

Sans effet tant que personne ne branche une deuxième manette.

Fichiers : `src/pico8_scroll.c`, `src/pico8_input.h` (le schéma de frame, qui n'a pas de second joueur), `src/pico8_raylib_backend.c` (l'endroit où la capture le décide).

## TROU-3 — Les chaînes ne sont pas internées

Lua interne les littéraux : deux chaînes identiques y sont le même objet.
Ce port copie ou alloue ses chaînes (les données générées ont un tableau
distinct par jeton) : deux chaînes égales n'y sont donc pas le même
pointeur.

Contourné au cas par cas : `pico8_gu_content_ptrs` compare par contenu
(les chaînes de `hy.invented`, pour `oj`), et
plusieurs modules ont leur propre comparaison par contenu (`jo:gg`,
`db`). Toute nouvelle comparaison de chaînes doit choisir explicitement.

**Hors de TROU-3 : les `qe`.** Ce que `ho()` route à travers `r` (`hp.o`,
`hp.dm`, donc `kw.qe`) et les besoins `{qe=r[c]}` sont des nombres —
l'indice d'une ressource —, pas des chaînes : ils se comparent par valeur
(`==` sur `pico8_fixed_t`, `pico8_qe_equal`). `db()`, qui compare une
chaîne, passe par le contenu.

Fichiers : `src/pico8_collect.c`.

## TROU-4 — `oo()` ne retombe pas sur la table quand le constructeur rend faux

Lua : `oo(fn,a)` vaut `fn and fn(...) or a` — si `fn(...)` rend une valeur
fausse, c'est `a`, la table analysée, qui est la valeur du champ. Ce port
ne teste que `fn == NULL` et rend sinon le résultat du constructeur tel
quel.

Devenu atteignable : `'s'` (le constructeur `spr`) est bien enregistré
depuis `pico8_pi_init()`, et `pico8_pi_spr_constructor` rendait un
pointeur nul, donc une valeur fausse. Aucune spec du cart
n'écrit `s(...)` aujourd'hui — vérifié sur `pico8/lua.lua` — donc le trou
est dormant, pas actif. Il s'ouvrira le jour où une spec en écrira une.

Sans site : `pico8_oo`, `src/pico8_ob.c` et `pico8_pi_spr_constructor`
sont supprimés ; le port n'analyse plus de spec.

## TROU-5 — `dg()` saute une entité quand une entité s'auto-supprime

`dg()` parcourt `hv` par index. Quand une entité se retire pendant sa
propre visite, les suivantes glissent d'un cran et celle qui prend sa place
n'est pas visitée cette frame. Lua fait autrement.

Divergence assumée : une entité sautée est simplement traitée à la frame
suivante.

Fichier : `src/pico8_bootstrap.c`.

## TROU-7 — Fuites mémoire assumées

Ce port n'a pas de ramasse-miettes là où Lua en a un. Deux conventions
fuient volontairement :

- les constructeurs de rangées (`jo.ex`, `hp:lq`, `pz`…) allouent une table
  que personne ne libère jamais ;
- une réaffectation de champ (`self.kw = {…}`) abandonne l'ancienne valeur.

Une partie longue croît donc lentement en mémoire. Remplacer `malloc`
(voir « À faire » du `README`) refermerait les deux.

## TROU-8 — Le tag `prober` n'a aucun lecteur

`g_` et toute la famille `jo` s'enregistrent sous le tag `prober`, mais
rien dans le cart ne fait jamais `cg("prober")`. Conservé par fidélité, pas
par utilité.

## TROU-9 — Pas de suppression de champ nommé

`pico8_ob_table_remove_index` existe, son pendant nommé non : aucun site
d'appel n'en a eu besoin. `t[nom]=nil` n'a donc pas d'équivalent exact —
écrire une valeur fausse n'est pas la même chose qu'un champ absent.

Sans site : `src/pico8_ob.c` est supprimé ; le port n'a plus de
table à clés dynamiques.

## TROU-10 — L'audio est hors périmètre

`sfx()` et `music()` sont des fonctions vides. Ce n'est pas un report : le
port n'aura pas de son.

Le reste de la chaîne, lui, fonctionne : le bouton « toggle sfx » du menu
pause bascule bien le drapeau de sourdine `ih`, et `rn()` le lit avant de
décider d'appeler `sfx()`. Seul l'appel final est inerte — donc les deux
boutons audio du menu pause répondent au clic sans rien produire.

Fichiers : `src/pico8_audio.c`, `src/pico8_menu.c`.

## TROU-11 — Le repli d'arité de `hd` rendrait le survol silencieusement faux

`pico8_js_hd_dispatch` reçoit `(argc, arg0, arg1)` et lit
`rp = (argc >= 2) ? arg1 : arg0`. Le seul site qui dispatche `"hd"`
aujourd'hui est `pico8_ui_gw`, avec `argc=2` en dur.

Mais `dc()` dispatche n'importe quel nom de `bm` — dont `"hd"` — avec
`argc=1`. Rien n'appelle `dc("hd")` aujourd'hui ; le jour où quelque chose
le fera, le repli donnera `mp == rp`, c'est-à-dire un test de survol qui
compare la position monde à elle-même au lieu de la position écran. Faux,
et silencieux.

Pas transformé en `assert(argc >= 2)` : ça ferait sauter un `dc("hd")` par
ailleurs légitime. Le vrai correctif est de décider ce que `hd` doit faire
sous `dc()`, ce qui dépasse une reprise de commentaires.

Sans site : l'emplacement `hd` a toujours deux arguments et
`dc` ne lance que `kj`/`ev` (`assert` sur tout autre nom).

## TROU-12 — Le premier anneau du trou de ver est noir au lieu de garder la plume

`mz:kj` colore chaque anneau par `self.kk[flr(sr/self.oi*8)]`. L'indice
tombe à 0 dès que `sr < oi/8`, c'est-à-dire pour l'anneau le plus jeune, à
chaque tick où `self.t` vient de dépasser un `dt`.

`kk` est une table Lua indexée à partir de 1 : `kk[0]` est nil, et PICO-8
conserve alors la couleur de plume courante. Ce port retombe sur 0 et
dessine en noir. Ce n'est pas la même image.

Trouvé en reprenant les commentaires : le commentaire à cet endroit
affirmait l'inverse — « REAL, FAITHFULLY-REPRODUCED EDGE CASE ». L'écart
est réel, l'affirmation était fausse.

Pas transformé en `assert` : la condition est atteinte en permanence, et
c'est le comportement actuel que les rejeux enregistrent. Le corriger
changera les enregistrements existants, ce qui en fait une décision de
jeu, pas une reprise de commentaires.


**Prémisse probablement fausse** (établi pour TROU-20, même situation) :
`kk[0]` est un nil **explicite**, que zepto8 et fake-08 convertissent en
couleur 0 ; le noir du port serait alors fidèle. Non vérifié sur le
binaire PICO-8 — même vérification que TROU-20.

Fichier : `src/pico8_wormhole_display.c`.

## TROU-13 — `tostr` d'un champ absent rend la chaîne vide

La rangée « score 0 » du panneau lit `s[st.score]` avec `score == 0`,
c'est-à-dire un indice que `ky()` ne pose jamais. Lua y verrait nil.

Ce port n'a pas de nil à mettre en chaîne : `pico8_pj_ph_s_str` rend `""`
pour tout indice hors de 1..6. PICO-8, lui, a une représentation dédiée
pour nil dans `tostr` — donc la ligne ne s'affiche pas pareil.

Fixé par `test_scores_box_follows_c_` de `test_pico8_mh`
(`SCORES_SEQUENCE_HASH`).

Fichier : `src/pico8_score_panel_display.c`.

## TROU-14 — Une boucle qui supprime son propre élément en saute un

Même famille que TROU-5, à d'autres endroits et sur d'autres listes :
`ei()` parcourt les particules de `qy` et en supprime au milieu de sa
propre boucle, sans compenser l'indice. La particule qui prend la place de
celle qui vient d'être retirée n'est pas visitée cette frame. Le même
motif existe dans l'affichage des slipways.

Sans conséquence visible : une particule vit une frame de plus que prévu,
et elle sera visitée à la frame suivante. Consigné parce que c'est le
troisième site du même motif, et qu'un futur lecteur qui en corrige un
seul rendra les trois incohérents entre eux.

Fichiers : `src/pico8_particle.c`, `src/pico8_slipway_display.c`,
et `src/pico8_bootstrap.c` pour la variante de TROU-5.

## TROU-15 — Là où Lua planterait, ce port ignore

Convention générale du portage, et non un oubli local : quand le cart
indexe une table absente, Lua lève une erreur. Ce port lit le champ comme
absent et continue. Les sites connus :

- `bd()` écrit dans l'ensemble `hg` d'un slipway qui n'en a pas : écriture
  ignorée.
- `lt`/`he` testent la **présence** du champ `bm`, pas sa véracité Lua :
  un champ posé à faux inscrit quand même l'entité.
- `he` sur une étiquette jamais enregistrée ne fait rien.
- `dc` écarte de toutes les couches une entité sans `fa`, ou dont le `fa`
  sort de 1..15.
- `ck()` saute une planète dont la liste de besoins est absente ou vide.

Aucun de ces cas n'est atteint en partie normale — ils décrivent ce qui
arriverait sur une donnée malformée. Consignés parce qu'un lecteur qui en
« corrige » un seul en le rendant fatal rendrait le port moins tolérant
que le cart à un endroit et pas aux autres.

Fichiers : `src/pico8_economy_tiers.c`, `src/pico8_economy_formulas.c`,
`src/pico8_entity.c`.

## TROU-16 — Le tirage pondéré lit hors bornes si son pool se vide

`gn()` construit un pool de tickets **consommable** : chaque `gn_next()`
retire le ticket tiré, et rien ne le regarnit. Quand il est vide,
`idx0` vaut 0 et le code lit `oc.items[0]` sur une liste vide — `items`
peut être NULL.

Les chiffres, calculés sur le littéral `df` du dépôt avec `og=2` :
**66 tickets**. `bw(90)` place 90 planètes, et `jo:hh` tire un ticket pour
chacune qui ne prend pas la branche fixe (environ 55 % des cas). L'espérance
reste sous 66, mais la queue de distribution la franchit.

Trouvé en reprenant les commentaires : un agent voulait poser
`assert(oc.count > 0)` et a calculé la marge avant de l'écrire — l'assert
aurait sauté. Ni le cart ni ce port ne gardent contre ce cas ; en Lua,
`oc[flr(r()*0+1)]` vaut nil et le `until not gu(nm,e)` boucle
indéfiniment, ce qui n'est pas mieux.

Fichier : `src/pico8_gen.c`.

## TROU-17 — Le générateur pseudo-aléatoire n'est pas celui de PICO-8

`pico8_fixed_rnd` tire d'un générateur maison (`pico8_rnd_next_u32`), pas
de celui de PICO-8. Aucune séquence ne coïncide.

Conséquence directe, et plus forte que TROU-1 : l'objectif acté
« même graine → même galaxie » **ne tient pas** face au vrai cart, et pas
seulement parce que le flottant dérive. Ici la première valeur tirée
diffère déjà.

Les rejeux internes restent déterministes — ce port se compare à
lui-même.

Fichier : `src/pico8_fixed.c`.

## TROU-18 — Le remap de couleurs de `pal()` n'est pas porté

`pico8_spr` lit la couleur d'un pixel de la feuille de sprites, teste le
bit de transparence, puis l'écrit **telle quelle**. Le remap de couleurs
que `pal()` installe dans la palette de dessin n'est appliqué nulle part :
seul le bit 0x80 de l'octet de palette est lu.

Ce qui marche quand même : `dv()` sélectionne un bloc de palette et le
cart s'en sert surtout pour éteindre ou allumer des couleurs, pas pour les
permuter. Ce qui ne marche pas se verrait sur un effet qui permute
réellement deux couleurs.

Fichier : `src/pico8_gfx.c`, marqué par un `TODO` sur place.

## TROU-19 — La police est reconstituée à la main

Les glyphes de PICO-8 appartiennent à son runtime, pas au cart : ils ne
sont dans aucun asset exportable. Ce port les redessine — 3×5 pixels dans
une cellule de 4 — et affiche un bloc plein pour tout caractère non
couvert.

Divergence visuelle permanente, assumée : la seule façon de la refermer
serait de relever les glyphes depuis PICO-8 lui-même.

Fichier : `src/pico8_gfx.c`.

## TROU-20 — Un `nil` explicite comme couleur de `circ` n'est pas vérifié sur PICO-8

Dans `ga:kj`, chaque particule est tracée par
`circ(p.x,p.y,pt.p:qx(),self.k_[flr(pt.l/2)])` (lua.lua:1091). Les
particules à `1 < l < 2` donnent l'indice 0 ; `k_` est indexée à partir de
1, donc `k_[0]` est nil et `circ` reçoit un quatrième argument **nil
explicite**, pas un argument omis. Ce port trace alors en couleur 0 (noir).

Ce n'est probablement pas un écart. Le manuel ne dit rien d'un nil
explicite ; deux émulateurs le convertissent en 0 et posent aussi la plume
à 0 : zepto8 (`lua_isnone` seul marque l'argument absent, nil donne
`lua_tonumber(nil)` = 0) et fake-08 (`lua_tonumber(L,4)` dès qu'il y a
plus de 3 arguments). Le port trace donc comme eux ; il ne pose pas la
plume, sans effet visible ici.

Non vérifié sur le binaire PICO-8. La vérification : `color(8)
circ(64,64,10,nil)` dans une cartouche, et regarder la couleur. Si PICO-8
gardait la plume, il faudrait d'abord un vrai état de plume dans le port
(voir la note sur `pen_color` plus bas) ; seules 22 frames de
`geoharvest_burst` bougeraient.

Note liée : dans les deux émulateurs, **tout** tracé coloré met la plume à
jour, alors que `pen_color` du port n'est écrit que par `pico8_rk`. Un
tracé coloré entre un `rk` et un `rk_default` ferait diverger le port.
Non étudié.

Prémisse réfutée par diagnostic dans les émulateurs.

Fichier : `src/pico8_planet_family.c`.

## TROU-21 — La palette d'un éclat né d'une protoétoile est celle de la classe

`ga:kj` lit `self.k_` (lua.lua:1091), donc le `k_` de l'instance quand elle
en a un. `pm:pq()` en donne un au `ga` qu'il crée,
`k_=o(1,2,8,14,14,14,14,8,8,2,2,1,1)` (lua.lua:1030, 1033), qui masque celui
de la classe, `k_=o(1,2,4,9,9,10,10,7,7,7,7,7)` (lua.lua:1077). Ce port lit
`k_` dans les valeurs par défaut de la classe `ga`, jamais dans l'instance :
l'éclat d'une protoétoile prend la palette de l'éclat ordinaire.

Probable, pas vérifié contre PICO-8. `pico8_pm_pq` est exécuté par l'e2e
`tests/e2e/tech_starbirth_protostar.json` : son empreinte fige le rendu du
port, écart compris.

Fichier : `src/pico8_planet_family.c`.

## TROU-22 — `tostr` sous 0.0001 n'est pas sourcé

`pico8_tostr_fixed` écrit un nombre avec 4 décimales arrondies au pair,
puis retire les zéros de fin et le point nu. Cette règle reproduit 13 des
14 témoins du bloc C de `tools/probe_out.txt`, dont `2/3 → "0.6667"`.

Le 14ᵉ est le brut −6 (`0xffff.fffa`) : PICO-8 affiche `"-0"`, ce port
`"-0.0001"`. Aucune règle unique ne concilie ce témoin avec
`2/3 → "0.6667"`, donc elle n'est pas devinée. Ce port garde sa sortie :
brut −6 → `"-0.0001"`, bruts −1 à −3 → `"-0"`, bruts 1 à 3 → `"0"`. Pour
les autres magnitudes inférieures à 0.0001, la règle de PICO-8 est
inconnue.

Non atteint par le filet : aucune valeur affichée par le jeu n'a une
magnitude inférieure à 0.0001. Se tranche par une ligne de plus dans
`tools/probe.p8`, lancée sur PICO-8.

Fichier : `src/pico8_str.c`.

## TROU-23 — Une ligne de bouton qui porte `h_` ou `jb` à nil a quand même sa pastille ou son infobulle

Dans le cart, `js:pq` fait `self.h_=self.h_ and lq(self.h_)` et
`self.jb=self.jb and jb(self.jb)` : une clé nil ne construit rien. Ce port
construit un `lq` vide, ou un `jb` vide, dès que la ligne porte la clé,
même à nil (TROU-9 : une clé nommée écrite à nil reste présente). Seul
`false` n'en construit pas.

Atteint par le filet : la ligne de `jo:fe` d'un bâtiment dont `b:jb()` est
nil (`hp:jb` quand `ii` est vrai) reçoit une infobulle vide, dessinée au
survol (`events_recorded_1790198294`, `session_events`). La ligne de
`jo:gh` sans `kr` recevrait une pastille vide ; aucun rejeu ne l'atteint.
La struct `js`, où nil vaut absent, garde ce comportement : la présence
d'une clé `h_` ou `jb` nil est portée par `has_h_`/`has_jb` de
`pico8_js_keys_t`, sans `h_row` ni `h_is_false` (sans `jb_row`) ; `js:pq`
construit alors une rangée ou un `jb` vides. Fixé par `test_nil_keys` de
`test_pico8_js`.

Fichier : `src/pico8_ui_button.c`.

## TROU-24 — `trade:kg` donne `qe=false` à la route neuve quand `self.qe` est nil

Dans le cart, `trade:kg` recopie `self.qe` tel quel : nil reste nil, et
aucune route n'a jamais `qe=false`. Ce port pose `false` quand la route
allongée n'a pas de `qe`. Dans la struct `trade`, nil et absent ne font
qu'un : un `qe` nil, passé explicitement ou omis, donne `false`.

`pico8_qe_t` garde les trois états et `pico8_qe_equal` est le `==` de Lua :
l'écart se voit là où une route rencontre un besoin sans `qe`. Dans
`jo:cn`, une route venue de `jo:ib` sans `qe` (nil) répond à un tel besoin,
comme dans le cart ; passée par `kg` (`nr:cn`), elle vaut `false` et n'y
répond plus. Dans `dh`, la garde `has_qe` écarte en plus la route nil que le
cart compterait : une route sans nombre n'y compte jamais.

Non atteint par les e2e : toute route qu'ils construisent a un `qe` non nil.
`jo:ib` passe pourtant un `qe` nil quand la table `kw` de la planète n'a pas
de `qe`. `test_pico8_trade` fixe les deux lectures.

Fichiers : `src/pico8_trade.c`, `src/pico8_planet_trade.c`,
`src/pico8_economy_tiers.c`.

## TROU-25 — La rangée « score 0 » du panneau de fin a un trou, le port en dessine les deux éléments

La ligne `o("",score=0,ra=v(64,21),c=5)` de `bz` (lua.lua:803) n'a qu'un
positionnel, l'indice 1. `pj:ph` (lua.lua:885-891) lui écrit `st[3]` (la
chaîne de TROU-13), puis réécrit `st[1]`, et la passe à `lq`. La rangée
porte donc les indices 1 et 3, sans 2.

`lq:pq` la parcourt par `nv`, donc `all(self)` (lua.lua:289-290), qui
s'arrête à la bordure `#t`. Le port, lui, prend toutes les clés d'indice dans
l'ordre croissant (`pico8_layout_positional_values`) et resserre le trou :
le membre `items` de la rangée garde les deux éléments, `fs` en a deux, et
`w` compte les deux. Le centrage de la rangée (`a=0.5`, par `pi.ld`) en
dépend. Atteint par le filet (`game_end`, 4 constructions).

Vérifié dans la source du cart : la ligne de `bz`, l'ordre des écritures de
`pj:ph` et le parcours par `all` de `lq:pq`. Non vérifié sur PICO-8 : la
valeur de `#t` pour cette table. Dans l'implémentation de Lua 5.2, une
table construite avec `[1]` puis complétée en `[3]` a une bordure de 1, et
`all` ne verrait que l'indice 1 ; PICO-8 peut rendre 3, l'autre
bordure valide (`t[1]` n'étant pas nil, 0 n'en est pas une).

Préexistant : l'instance de `lq` parcourait déjà toutes les clés d'indice.
La conversion en struct préserve l'écart.

Fichiers : `src/pico8_layout.c` (constructeur de `lq`),
`src/pico8_score_panel_display.c`.

## TROU-26 — Le `fo` d'une planète hors écran est coupé quand l'objet tenu est rendu

`jo:kj` (lua.lua) sort avant `self.fo=nil` quand la planète est hors de
`gd()` ou à l'état « ll ». Son `fo` garde alors l'objet tenu (une structure
prise par une icône de recherche, un `jc`) après que celui-ci a quitté la
partie ; si la planète revient à l'écran, la cartouche dessine un halo de
plus avec le `dw` de cet objet mort.

Le port rend l'objet par la file de sortie et recycle son arène : relire
`fo` serait un usage après libération. `oublier` (`pico8_cart.c`) remet donc
à nil le `fo` de toute entité `np` qui vise l'objet rendu, et ce halo
n'est pas dessiné. À l'état « ll », aucune différence : dès t ≥ 60, `kj`
sort toujours avant de lire `fo`. Aucun e2e n'atteint ce cas.

Fichiers : `src/pico8_cart.c` (`oublier`), `src/pico8_planet.c`
(`pico8_jo_oublier_fo`).
