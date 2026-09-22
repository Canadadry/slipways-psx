# Trous de portage

Les écarts connus entre ce port C et le cart PICO-8 d'origine, et les
morceaux volontairement non faits. Le code y renvoie par un commentaire
`/* TROU-n */` à l'endroit concerné, sans répéter l'explication.

Chaque entrée a été revérifiée contre le code au moment de la reprise des
commentaires. Une entrée supprimée est une entrée refermée : voir git.

---

## TROU-1 — L'arithmétique flottante n'est pas bit-exacte

`pico8_fixed_cos`/`_sin` passent par le `cos()`/`sin()` de l'hôte en
`double`, puis reconvertissent en virgule fixe. Idem pour
`pico8_fixed_from_double`, qui sert aux littéraux décimaux hors spec `ob()`
(`bq=0.0977`), et pour l'exposant fractionnaire `^0.75` de `dj()`
(`pico8_economy_formulas.c`).

Conséquence : une partie rejouée ici peut diverger d'une partie rejouée
dans le vrai PICO-8, d'autant plus vite que la divergence se propage par le
générateur pseudo-aléatoire. Les rejeux internes restent déterministes,
puisqu'ils comparent ce port à lui-même.

`docs/plan-2.md` a l'élimination du flottant à son programme.

Fichiers : `src/pico8_fixed.c`, `src/pico8_economy_formulas.c`,
`src/pico8_game_data.c`.

## TROU-2 — La deuxième manette est câblée à « rien »

`qb:ph()` teste `btn(b) or btn(b,1)`. Ce port n'a aucune représentation
d'un deuxième joueur, ni dans `pico8_input_t` ni dans `events.json`, et
n'en aura pas : `btn(b,1)` rend toujours faux.

Sans effet tant que personne ne branche une deuxième manette.

Fichier : `src/pico8_scroll.c`.

## TROU-3 — Les chaînes ne sont pas internées

`pico8_value_equal` compare deux `PICO8_VALUE_STRING` **par pointeur**,
alors que Lua interne les littéraux : deux chaînes identiques y sont le
même objet. Comme le parseur `ob()` de ce port alloue une chaîne neuve par
token, deux chaînes égales ne sont jamais le même pointeur.

Contourné au cas par cas : `pico8_gu_content` compare par contenu, et
plusieurs modules ont leur propre comparaison par contenu (`jo:gg`, `dh`,
`db`). Toute nouvelle comparaison de chaînes doit choisir explicitement,
`pico8_value_equal` ne fera pas ce qu'on croit.

Fichiers : `src/pico8_value.c`, `src/pico8_collect.c`.

## TROU-4 — `oo()` ne retombe pas sur la table quand le constructeur rend faux

Lua : `oo(fn,a)` vaut `fn and fn(...) or a` — si `fn(...)` rend une valeur
fausse, c'est `a`, la table analysée, qui est la valeur du champ. Ce port
ne teste que `fn == NULL` et rend sinon le résultat du constructeur tel
quel.

Devenu atteignable : `'s'` (le constructeur `spr`) est bien enregistré
depuis `pico8_pi_init()`, et `pico8_pi_spr_constructor` rend
`pico8_value_ref(NULL)`, donc une valeur fausse. Aucune spec du cart
n'écrit `s(...)` aujourd'hui — vérifié sur `pico8/lua.lua` — donc le trou
est dormant, pas actif. Il s'ouvrira le jour où une spec en écrira une.

Fichiers : `src/pico8_collect.c` (`pico8_oo`), `src/pico8_ob.c` (la même
logique, dupliquée en ligne dans le parseur).

## TROU-5 — `dg()` saute une entité quand une entité s'auto-supprime

`dg()` parcourt `hv` par index. Quand une entité se retire pendant sa
propre visite, les suivantes glissent d'un cran et celle qui prend sa place
n'est pas visitée cette frame. Lua fait autrement.

Divergence assumée : une entité sautée est simplement traitée à la frame
suivante.

Fichier : `src/pico8_bootstrap.c`.

## TROU-6 — L'entité tenue par la souris est castée sans vérification

`ui:bh()` range dans `lheld`/`rheld` ce que rend le `ldown`/`rdown` de
l'entité cliquée, puis le relit comme un `pico8_instance_t*` pour lui
envoyer `hq`/`ey`. Rien ne garantit que c'en soit une : en Lua c'est
n'importe quelle table.

Aujourd'hui sans danger — toutes les classes qui définissent `ldown` rendent
soit une instance, soit un booléen — mais c'est la même famille de
confusion de représentation que celle qui a produit trois segfaults (voir
le tag de type, `src/pico8_type_tag.h`).

Fichier : `src/pico8_ui.c`.

## TROU-7 — Fuites mémoire assumées

Ce port n'a pas de ramasse-miettes là où Lua en a un. Deux conventions
fuient volontairement :

- les constructeurs de rangées (`jo.ex`, `hp:lq`, `pz`…) allouent une table
  que personne ne libère jamais ;
- une réaffectation de champ (`self.kw = {…}`) abandonne l'ancienne valeur.

Une partie longue croît donc lentement en mémoire. `docs/plan-2.md` a
l'élimination de `malloc` à son programme, ce qui referme les deux.

## TROU-8 — Le tag `prober` n'a aucun lecteur

`g_` et toute la famille `jo` s'enregistrent sous le tag `prober`, mais
rien dans le cart ne fait jamais `cg("prober")`. Conservé par fidélité, pas
par utilité.

## TROU-9 — Pas de suppression de champ nommé

`pico8_ob_table_remove_index` existe, son pendant nommé non : aucun site
d'appel n'en a eu besoin. `t[nom]=nil` n'a donc pas d'équivalent exact —
écrire une valeur fausse n'est pas la même chose qu'un champ absent.

Fichier : `src/pico8_ob.c`.

## TROU-10 — L'audio est hors périmètre

`sfx()` et `music()` sont des fonctions vides, et le drapeau de sourdine
`ih` n'est jamais posé. Ce n'est pas un report : le port n'aura pas de son.

Fichier : `src/pico8_audio.c`.

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

Fichiers : `src/pico8_ui_button.c`, `src/pico8_entity.c`.

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

Fichier : `src/pico8_wormhole_display.c`.
