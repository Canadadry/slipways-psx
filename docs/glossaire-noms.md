# Glossaire des noms

Le cart d'origine (`pico8/lua.lua`) est minifié : ses identifiants sont des
digrammes sans signification (`jo`, `bv`, `cg`). Le port C les translittère
littéralement (`pico8_jo_*`, `pico8_bv`, `pico8_cg`), donc le nom se retrouve
dans le code — mais pas le sens. C'est ce que donne ce fichier, et lui seul :
une ligne par symbole. Le code ne le répète pas.

Conventions de lecture : « classe » = classe `ke`-descendante du système
`kv:jr`. Les méthodes sont notées `classe:methode`.

## Noyau : classes, entités, dispatch

| Nom | Symbole C | Sens |
|---|---|---|
| `kv` | `pico8_class_*` | le système de classes lui-même ; `kv:jr(spec)` déclare une classe |
| `jr` | `pico8_class_declare` | déclare une sous-classe à partir d'une spec `[[...]]` |
| `ke` | `pico8_ke_class` | classe racine : toute entité du jeu en descend |
| `ke:ks(me)` | `pico8_ke_ks` | réinitialise l'état `me` de l'instance et remet son timer `t` à 0 |
| `pq` | `pico8_class_set_pq` | constructeur supplémentaire, appelé à l'instanciation |
| `lt` | `pico8_lt` | enregistre une instance fraîche dans `hv`/`cv`/`cg` ; c'est le `pq` de `ke` |
| `me` | champ `"me"` | nom de l'état/icône courant d'une entité |
| `t` | champ `"t"` | compteur de frames de l'état courant |
| `mv(e,rm,...)` | `pico8_mv` | dispatch : résout `e[rm]`, l'appelle si c'est une fonction, sinon rend la valeur |
| `dc(nt)` | `pico8_dc` | envoie le message `nt` à toutes les entités inscrites sous ce nom ; la branche `kj` dessine couche par couche via `ci` |
| `ft(rc,nt,pu)` | `pico8_ft_index`, `pico8_ft_name` | « crée la liste `rc[nt]` si besoin, puis y ajoute `pu` ». La clé polymorphe de Lua devient deux points d'entrée ; seul `pico8_ft_index` a un appelant (`ui:gw`) |
| `bm` | — | les cinq noms de message que `dc` sait router : `kj`, `ev`, `hd`, `hk`, `hb` |
| `cv` | `pico8_cv` | entités indexées par nom de message `bm` : `kj` (dessin), `ev`, `hd` (survol), `hk`, `hb` |
| `cg` | `pico8_cg` | entités indexées par étiquette (`nu`), d'où `cg("jo")` = toutes les planètes |
| `hv` | `pico8_hv` | liste de toutes les entités vivantes, dans l'ordre d'enregistrement |
| `he(e)` | `pico8_he` | retire une entité de `hv`/`cv`/`cg` |
| `ci` | `pico8_ci` | facteur de mélange de palette par couche, lu par la branche `kj` de `dc` |
| `jl()` | `pico8_jl` | (re)construit les singletons du jeu : `mo=qb()`, `je=nf()`, `ok=ui()` |
| `nu` | champ `"nu"` | étiquettes d'une classe, ce par quoi `cg` l'indexe |

## Mémoire cart et données encodées

| Nom | Symbole C | Sens |
|---|---|---|
| `gt` | `PICO8_GT` | alphabet de 51 caractères de `unstash` |
| `unstash(pd)` | `pico8_unstash` | décode une chaîne depuis la mémoire cart, un octet = un index dans `gt`, l'octet 0 termine |
| `r` | `pico8_r` | table décodée depuis l'adresse 8192 |
| `iz` | `pico8_iz` | table décodée depuis l'adresse 8259 |
| `es` | `pico8_es` | table de correspondance de `da()` |
| `ct` | `pico8_ct` | bonus par palier du partenaire commercial, lu par `db()` |
| `b_` | `pico8_b_` | table de score de `ky()` |
| `co` | `pico8_co` | modificateur de bonheur de `bf()` |
| `fb` | `pico8_fb` | décalages d'adresse de palette de `eg()` |
| `bn`, `bq` | `pico8_bn`, `pico8_bq` | constantes de réglage de `dj()` |

## Entrée et son

| Nom | Symbole C | Sens |
|---|---|---|
| `bu(qn,oa)` | `pico8_bu` | détection de front d'un bouton souris : remplit `on`/`jn`/`hm` (tenu / vient d'être pressé / vient d'être relâché) depuis `stat(34)` |
| `on`, `jn`, `hm` | champs | bouton tenu / front descendant / front montant |
| `ih` | `pico8_ih` | drapeau « son coupé », basculé par la rangée « toggle sfx » du **menu pause** |
| `rn(no)` | `pico8_rn` | joue le sfx `no` si `no` est vrai et `ih` faux |
| `sfx`, `music` | `pico8_sfx`, `pico8_music` | stubs inertes : l'audio est hors périmètre du port |
| `sub(s,i,j)` | `pico8_sub` | sous-chaîne PICO-8, indices 1-based et bornes incluses |

## Géométrie et vecteurs

| Nom | Symbole C | Sens |
|---|---|---|
| `km` | `pico8_vec_t` | métatable vecteur : `{x,y}` en virgule fixe |
| `v(x,y)` | `pico8_vec_v` | construit un vecteur |
| `rg` | `pico8_vec_rg` | produit scalaire |
| `#v` (`__len`) | `pico8_vec_len_sq` | norme **au carré** (utilisée telle quelle pour comparer des distances) |
| `qx` | `pico8_vec_qx` | norme |
| `nn` | `pico8_vec_nn` | normalise |
| `pa` | `pico8_vec_pa` | perpendiculaire (rotation d'un quart de tour) |
| `qm(fy,lx)` | `pico8_vec_qm` | polaire → cartésien : vecteur de longueur `fy` à l'angle `lx` (en tours) |
| `qz` | `pico8_qz_box_t` | prédicat « ce point est-il dans la boîte » |

## Champs d'entité récurrents

| Nom | Sens |
|---|---|
| `ra` | position écran de l'entité |
| `p` | position monde (planètes, sondes) — distinct de `ra` |
| `kq` | boîte de collision, un prédicat `qz` |
| `fa` | couche de rendu ; `fa >= 10` = interface, testé contre la position écran plutôt que monde |
| `ma` | rayon du halo de particules, décroissant |

## Helpers géométriques sur entités vivantes

| Nom | Symbole C | Sens |
|---|---|---|
| `gk(e,mp,rp)` | `pico8_gk` | test de survol : la boîte `kq` de `e` contient-elle le curseur ; c'est le `hd` de `js` |
| `gd(e)` | `pico8_gd` | `e` est-elle dans les 128px visibles de la caméra |
| `na(self,t)` | `pico8_na` | flottement vertical, mute `ra.y` en place |
| `ma(p,e)` | `pico8_ma` | fait décroître le halo de `e` et décale `p` d'un vecteur polaire |
| `ij(p,j_)` | `pico8_ij` | plus proche voisin : rend la distance au carré et l'entité |
| `mo` | `pico8_mo` | la caméra (instance `qb`) |

## Générateurs procéduraux

| Nom | Symbole C | Sens |
|---|---|---|
| `jf(n_,b)` | `pico8_jf`/`_next` | marche aléatoire bornée, un pas par appel |
| `gn(hr,og,r)` | `pico8_gn`/`_next` | tirage pondéré (par le champ `.r`) avec fenêtre anti-répétition de 3 |

## Entrée : classes lectrices

| Nom | Symbole C | Sens |
|---|---|---|
| `nf` | `pico8_nf_class` | classe lectrice de la souris uniquement (pas du D-pad) |
| `nf:pq` | `pico8_nf_pq` | `poke(0x5f2d,1)` : active le rapport souris devkit de PICO-8 |
| `nf:ph` | `pico8_nf_ph` | lit la position souris et les fronts des deux boutons |
| `je` | — | le singleton `nf` |
| `qb` | `pico8_qb_*` | classe caméra / lecteur D-pad (défilement) |

## Tables de données du cart

| Nom | Symbole C | Sens |
|---|---|---|
| `df` | `pico8_df` | catalogue des types de planète, tiré au sort par `bw()` |
| `hc` | `pico8_hc` | données lues par `hp:po()` |
| `fd` | `pico8_fd` | données des projets de recherche, source de `gb` |
| `gb` | `pico8_gb` | les instances `hp` construites depuis `fd` |
| `dr` | `pico8_dr` | les specs d'infobulle des icônes d'amélioration : `eu()` lit `dr[i]` pour la i-ème classe de planète qui reçoit une icône `ie` |
| `nb` | `pico8_nb` | liste des technologies, lue et mutée par `cb()`/`gi()` |
| `ek` | `pico8_ek` | seuils de palier comparés à `#self.invented` par `hy:kt()` |

## Commerce

| Nom | Symbole C | Sens |
|---|---|---|
| `trade` | `pico8_trade_class` | une route commerciale : chaîne de planètes `lp`, slipways `rl`, ressource `qe` |
| `lp`, `rl`, `qe` | champs | planètes de la route / slipways / ressource transportée |
| `trade:to()` | `pico8_trade_to` | dernière planète de la route |
| `trade:kg(np,jc)` | `pico8_trade_kg` | rend une **nouvelle** route allongée d'une planète et d'un slipway |
| `trade.__eq` | `pico8_trade_eq` | deux routes sont égales si mêmes planètes de départ et d'arrivée |

## Géométrie pure

| Nom | Symbole C | Sens |
|---|---|---|
| `ee(a,b,c)` | `pico8_geom_ee` | de quel côté de la droite `a→b` se trouve `c` ; trois états, « sur la droite » compris |
| `cl(p1,p2,q1,q2)` | `pico8_geom_cl` | les deux segments se croisent-ils franchement |
| `be(a,b,c,fh)` | `pico8_geom_be` | `c` est-il à moins de `fh` du segment `a→b`, extrémités exclues ; `fh` est une distance **au carré** |
| `qz(xl,yt,xr,yb)` | `pico8_qz` | construit la boîte ; `pico8_qz_call` teste un point |

## Caméra et défilement

| Nom | Symbole C | Sens |
|---|---|---|
| `qb:ph` | `pico8_qb_ph` | défilement au D-pad avec inertie, position bornée à [-64, 320] |
| `qb:md(fy)` | `pico8_qb_md` | applique la caméra, `ra` multiplié par le facteur de parallaxe `fy` |
| `pc` | champ | les 4 vecteurs directions unitaires du D-pad |
| `v` (champ de `qb`) | — | vélocité de la caméra, distinct du constructeur `v(x,y)` |

## Composition graphique

| Nom | Symbole C | Sens |
|---|---|---|
| `cu()` | `pico8_cu` | construit en 0x5000 la table de remap de palette 16×16, encodée dans les pixels de la feuille de sprites |
| `dv(no)` | `pico8_dv` | active le bloc de palette `no` parmi les 16 que `cu` a construits |
| `rk(y1,y2,col)` | `pico8_rk` | barre horizontale pleine largeur ; `col` omis reprend la couleur de plume courante |
| `eq(x1,y1,x2)` | `pico8_eq` | cadre d'un bouton d'aide : contour noir, fond gris, deux demi-icônes |
| `io(t,x,y,c,a)` | `pico8_io` | texte avec contour noir ; `a` centre horizontalement |
| `er` | — | les 8 décalages du contour de `io`, plus le centre |

## Recherche et déblocages

| Nom | Symbole C | Sens |
|---|---|---|
| `oj(px)` | `pico8_oj` | le prérequis `px` est-il débloqué (`px` absent = pas de prérequis, donc vrai) |
| `ov(px,lk,mc)` | `pico8_ov` | rend `mc` si `px` est débloqué et `mc` non faux, sinon `lk` |
| `hp` | `pico8_hp_class` | classe d'un projet de recherche |
| `fm` | `pico8_fm` | le singleton économie (instance `hy`) |
| `invented` | champ de `fm` | liste des technologies déjà débloquées |

## Économie : la classe `hy` et son singleton `fm`

| Nom | Symbole C | Sens |
|---|---|---|
| `hy` | `pico8_hy_class` | classe économie ; `fm = hy()` est son unique instance, construite par `eu()` |
| `hy:nx(hx)` | `pico8_hy_nx` | avance l'horloge de `hx` ticks, recalcule, et déclenche une « année » au seuil `ov("time compression",12,15)` |
| `hy:hk()` | `pico8_hy_hk` | tick d'année : revenu → crédits, points de recherche, année +1, puis `:hk(année)` à toutes les entités inscrites sous `hk` |
| `hy:lu(jt)` | `pico8_hy_lu` | dépense `jt` si les crédits suffisent, sinon affiche « -no money- » et échoue |
| `hy:lj()` | `pico8_hy_lj` | recalcul complet : paliers jusqu'au point fixe, tables production/demande, revenu, bonheur, statut de fin |
| `hy:kt(t)` | `pico8_hy_kt` | valide une techno recherchée : paie, l'ajoute à `invented`, fait éventuellement avancer l'ère |

### Champs de `fm`

| Champ | Sens |
|---|---|
| `nz` | crédits |
| `ge` | revenu par année |
| `jk` | points de recherche accumulés |
| `yr` | année courante ; 3426 déclenche le score final |
| `lo` | compteur de ticks depuis la dernière année |
| `po` | production totale par ressource (résultat de `bx()`) |
| `fu` | demande par ressource (résultat de `ck()`) |
| `happiness` | score de bonheur (résultat de `bf()`) |
| `hl` | statut de fin de partie : `"final score"`, `"bankrupt"`, ou faux |
| `invented` | technologies débloquées |
| `ew` | ère technologique, 0 à 4 |

## Économie : les formules

| Nom | Symbole C | Sens |
|---|---|---|
| `cm(jm)` | `pico8_cm` | table indexée par ressource, chaque entrée initialisée à `jm` |
| `bx()` | `pico8_bx` | production totale par ressource, toutes planètes confondues |
| `ck()` | `pico8_ck` | pénalité d'encombrement : -0.15 par planète colonisée demandant la ressource |
| `dj(pb,to)` | `pico8_dj` | coût de base d'une route entre deux planètes, croissant avec la distance |
| `bk()` | `pico8_bk` | facteur d'échelle global du coût, croissant avec le nombre de slipways |
| `bd()` | `pico8_bd` | reconstruit la comptabilité des routes entrantes de chaque planète |
| `bb()` | `pico8_bb` | recalcule les paliers ; rend vrai si au moins un palier a bougé |
| `dh(pt)` | `pico8_dh` | palier de développement d'une planète, 0 à 4 |
| `da(pt)` | `pico8_da` | charge d'une planète, déduite de son revenu |
| `cz()` | `pico8_cz` | entretien global, quadratique en nombre de planètes |
| `db(pt)` | `pico8_db` | revenu commercial d'une planète |
| `ky()` | `pico8_ky` | détail du score final : développement, recherche, technologies, total, palier |
| `bf()` | `pico8_bf` | score de bonheur |
| `dk` | `pico8_dk` | la table de difficulté choisie au lancement ; `dk.trade` module tous les revenus |

### Champs de planète lus par l'économie

| Champ | Sens |
|---|---|
| `mi` | palier de développement |
| `jy` | besoins de la planète, une liste de `{qe=ressource}` |
| `cc` | routes commerciales arrivant sur la planète |
| `by` | routes partant de la planète (résultat de `jo:ib()`) |
| `kw` | production : `{qe=ressource, mu=quantité}` |
| `hf` | la planète est un hub |
| `hs` | multiplicateur de revenu commercial ; 1 par défaut, 0,5 sur une planète colonisée |
| `jh` | bonus de production ajouté à `kw.mu`, posé une seule fois par `jo:cn` |
| `hn` | le projet de recherche installé (instance `hp`) |
| `kr` | revenu net de la planète |
| `gg()` | la planète compte-t-elle dans les scores (colonisée) |
| `qh`, `nl` | marqueurs de sous-type de planète |
| `hg` | ensemble des planètes desservies par un slipway |

## Le mini-langage de données `ob()`

| Nom | Symbole C | Sens |
|---|---|---|
| `ob(qs,mr)` | `pico8_ob` | analyse une spec compacte (`"x=-1,y=-1,c=0,"`) en table, puis y fusionne `mr` ; **la virgule finale est obligatoire**, sans elle le dernier champ est perdu |
| `fi(qs,fn)` | `pico8_fi` | applique `fn` à chaque caractère et collecte les résultats non nuls |
| `pi` | `pico8_ob_register_constructor` | table des constructeurs appelables depuis une spec : `b`→`qz`, `v`→`v`, `br`→`rk`, `qr`→`io`, `rf`→`eq`, `s`→`spr`, `ld`→`lq:ol`. Depuis une spec, seule la première lettre est atteignable ; les noms longs ne le sont que par `bl()` |
| `oo(fn,a)` | `pico8_oo` | appelle le constructeur `fn` avec les 5 premières entrées positionnelles, ou rend `a` tel quel |
| `bl(rb,nc)` | `pico8_bl` | dessine chaque rangée de `rb` via le constructeur `pi` que nomme son champ `fn`, avec une secousse de caméra `nc` alternée. C'est le second consommateur de `pi` |
| `fn`, `rb`, `nc` | — | le nom de constructeur d'une rangée, la liste de rangées, le décalage de secousse |
| `oa` | — | masque de bouton passé à `bu()` : 1 pour le gauche, 6 pour le droit |

## Le système de classes `kv:jr`

| Nom | Symbole C | Sens |
|---|---|---|
| `fz` | `cls->name` | nom de la classe ; les classes sans `fz` se marchent dessus dans le registre, comme dans l'original |
| `iq` | `cls->parent` | classe parente ; absente pour une racine |
| `ns` | — | métatable d'instance de l'original ; sans équivalent ici, remplacée par la remontée de chaîne à chaque lecture de champ |
| `pq` | `cls->pq` | constructeur supplémentaire ; chaque `pq` distinct de la chaîne est appelé une fois |
| `cw` | champ | nom de classe lu à l'exécution, pour le re-classement dynamique de `jo:hu` |

## Helpers de listes et de tables

| Nom | Symbole C | Sens |
|---|---|---|
| `qj(o,mr)` | `pico8_qj` | fusionne `mr` dans `o` ; `mr` gagne sur les clés partagées |
| `mw(o)` | `pico8_mw` | copie superficielle |
| `gu(qk,kb)` | `pico8_gu` | `kb` est-il dans la liste `qk` (identité de pointeur) |
| — | `pico8_gu_content` | même chose, mais les chaînes sont comparées par contenu ; nécessaire parce que ce port n'interne pas les chaînes, contrairement à Lua |
| `jp(qk,ow)` | `pico8_jp` | filtre |
| `nv(qk,kp)` | `pico8_nv` | applique `kp`, en ignorant les résultats faux |
| `la(a,b)` | `pico8_la` | concatène deux listes |
| `ne(d,c,ku)` | `pico8_ne` | formate un nombre signé : `+` devant les positifs, couleur 8 (rouge) forcée pour les négatifs, suffixe `ku` optionnel |
| `ho(rc,mr,gp)` | `pico8_ho_apply` | renomme les champs positionnels d'un enregistrement selon `mr` et l'indexe dans `rc` par son `.px` |
| `tostr(v)` | `pico8_value_tostr` | conversion en chaîne ; 4 décimales puis zéros de fin élagués |

## Fond de carte et caméra à la souris

| Nom | Symbole C | Sens |
|---|---|---|
| `bg` | `pico8_bg_class` | le fond : champ d'étoiles et glisser-caméra |
| `bg:pq` | `pico8_bg_pq` | peint les 32×32 tuiles du fond avec un bruit lissé ; 0,4 % de chances d'un amas brillant par case |
| `bg:kj` | `pico8_bg_kj` | dessine la carte, une grille pointillée tous les 32 px, puis une bordure |
| `bg:ldown` | `pico8_bg_ldown` | clic gauche sur le vide : lance une sonde `g_`, ou un trou de ver `mz` si une sonde est déjà en vol |
| `bg:rdown`, `bg:hq` | `pico8_bg_rdown`, `pico8_bg_hq` | glisser-caméra au clic droit : le déplacement de la souris devient la vélocité `mo.v` |
| `pl` | champ de `bg` | les 15 tuiles parmi lesquelles le fond est tiré |
| `mq` | champ de `bg` | dernière position souris mémorisée pendant un glissé |

## La sonde `g_`

| Nom | Symbole C | Sens |
|---|---|---|
| `g_` | `pico8_g__class` | la sonde que le joueur lance depuis le vide |
| `bv(self,mp,rp)` | `pico8_bv` | suit la souris et choisit la cible `et` : planète proche, sinon slipway sous le curseur, sinon rien |
| `dl(self)` | `pico8_dl` | info-bulle près du curseur — générique, aussi appelée par `jc:hh` et `qh:ph` |
| `d_(self)` | `pico8_d_` | valide la cible : paie via `fm:lu`, avance l'horloge, sinon marque la sonde pour destruction |
| `g_:mm` | `pico8_g__mm` | engendre une particule de la traînée, en orbite plutôt qu'en ligne droite |
| `et` | champ | cible courante du glissé |
| `fo` | champ | « accrochée par » : la cible pointe en retour vers la sonde |
| `p_` | champ | drapeau d'auto-destruction, relevé par `dg()` à la fin de la frame |
| `od` | champ | coût du projet visé |
| `dw` | champ | seuil de distance au carré pour s'accrocher à une planète |
| `qs` | champ | texte d'état d'une cible, affiché par `dl` |

## L'interface `ui`

| Nom | Symbole C | Sens |
|---|---|---|
| `ui` | `pico8_ui_class` | le répartiteur de survol et de clics ; `ok` est son singleton |
| `ui:lj` | `pico8_ui_lj` | une frame d'interface : position monde de la souris, survol, puis les deux boutons |
| `ui:ev` | `pico8_ui_ev` | dessine le curseur : sprite de l'entité survolée, ou celui par défaut |
| `ui:gw` | `pico8_ui_gw` | teste le survol de toutes les entités inscrites sous `hd` et les range par couche |
| `ui:bh(bt)` | `pico8_ui_bh` | un bouton : à l'appui `ldown`/`rdown`, pendant le maintien `hq`, au relâchement `gv` puis `ey` |
| `ui:jz(rm,…)` | `pico8_ui_jz` | envoie `rm` aux entités survolées, de la couche 15 vers 0, et rend la première réponse vraie |
| `ui:cj` | `pico8_ui_cj` | détruit toutes les entités éphémères de `le` |
| `qa` | champ | position de la souris en coordonnées monde (`je.ra + mo.ra`) |
| `o_`, `gj` | champs | entité survolée cette frame / la frame précédente |
| `m_` | champ | drapeau « je suis survolée », posé et retiré par `gw` |
| `ic` | champ | seaux d'entités survolées, indexés par couche `fa` |
| `le` | champ | entités éphémères, détruites en bloc par `cj` |
| `hw` | champ | boîte du panneau principal, `b(10,10,118,118)` |
| `ql` | champ | sprite de curseur |
| `lheld`, `rheld` | champs | entité tenue par le bouton gauche / droit |
| `ok` | `pico8_ok` | le singleton `ui` |

### Champs de bouton et de rangée

| Nom | Sens |
|---|---|
| `h_` | seconde rangée d'icônes d'un bouton : l'infobulle dessinée à côté |
| `mb` | largeur du bouton, dérivée de celle de sa rangée `lq` |
| `qf` | position cible vers laquelle `ph` fait glisser `ra` |
| `ep` | champ de planète effacé par `jo:lz` et reposé par `jo:hh` ; l'effacement est un effet de bord observable, pas un oubli |

## La planète `jo` et sa famille

| Nom | Symbole C | Sens |
|---|---|---|
| `jo` | `pico8_jo_class` | la planète de base : toute planète de la carte en est une, ou une sous-classe |
| `jo:lz(hp)` | `pico8_jo_lz` | installe un projet de recherche : fixe `hn`, **efface `ep`**, dérive les besoins `jy` et la production `kw` |
| `jo:gg()` | `pico8_jo_gg` | la planète est-elle colonisée et prospère (`hf` vrai et état `ly`) |
| `jo:kd()` | `pico8_jo_kd` | relance le halo de particules à son maximum |
| `jo:ll(t)` | `pico8_jo_ll` | état « en train de disparaître » : au-delà de 60 ticks, marque la planète pour destruction |
| `jo:hu(hp)` | `pico8_jo_hu` | applique un projet terminé : reclasse la planète en une autre classe, ou fixe son prochain type, ou la colonise |
| `jo:hh()` | `pico8_jo_hh` | tire le prochain aléa `nk` : 45 % depuis l'entrée fixe de `df`, sinon depuis le générateur de classe `jo.oc` |
| `jo:hk()` | `pico8_jo_hk` | tick d'année : affiche un toast de revenu si `kr` est non nul et la planète visible |
| `jo:ldown()` | `pico8_jo_ldown` | clic gauche : ouvre le panneau de recherche si la planète est connue mais non colonisée, sinon amorce un slipway |
| `jo:hb()` | `pico8_jo_hb` | reconstruit la rangée d'icônes flottantes `lq` au-dessus de la planète |
| `jo:gh()` | `pico8_jo_gh` | panneau d'inspection de la planète |
| `jo:gv(s)` | `pico8_jo_gv` | une extrémité de slipway peut-elle se poser ici |
| `jo:fe()` | `pico8_jo_fe` | panneau des projets de recherche disponibles |
| `jo:kj(p)` | `pico8_jo_kj` | dessine la planète |
| `jo:kf(sw)` | `pico8_jo_kf` | enregistre un slipway fraîchement construit dans `oe` |
| `jo:cn(lw)` | `pico8_jo_cn` | la planète a-t-elle encore de la place pour la ressource proposée |
| `jo:ib()` | `pico8_jo_ib` | reconstruit la liste des routes commerciales acceptées |
| `jo.ex(…)` | `pico8_jo_ex` | fonction **statique**, pas une méthode : construit une rangée d'icônes de ressources |
| `jo.oc`, `jo.ir` | `pico8_jo_set_oc/_ir` | générateurs partagés par toute la classe, posés une fois par `bw()` |
| `nk` | champ | aléa courant de la planète |
| `oe` | champ | slipways branchés sur la planète |
| `px` | champ | nom affiché |
| `rr` | champ | technologie requise pour débloquer cette classe de planète |

### Les sous-classes de planète

| Nom | Nom affiché | Sens |
|---|---|---|
| `qh` | laboratory | planète colonisée, la base de toutes les autres |
| `os` | processor | variante de `qh`, sans méthode propre |
| `nr` | slipgate | relaie les routes au lieu de les consommer |
| `fr` | infraspace | |
| `nl` | trading hub | débloquée par `trade league` |
| `mx` | synthesizer | débloquée par `void synthesis` ; synthétise les ressources de son champ `ht` |
| `hj` | ascension | émettrice de particules |
| `pm` | protostar | une des deux seules classes que `jo:hu` sait résoudre par son nom `fz` |
| `ga` | | éclat de particules ; **pas purement visuel** : son `pq` écrit `mo.ma=3` sur la caméra |

## Le trou de ver `mz`

| Nom | Symbole C | Sens |
|---|---|---|
| `mz` | `pico8_mz_class` | le pliage d'espace : révèle les planètes autour de son trajet |
| `mz:pq` | `pico8_mz_pq` | à la création, liste dans `ox` toutes les entités dont le champ `fx` est vrai, c'est-à-dire les points d'ancrage possibles |
| `mz:hq` | `pico8_mz_hq` | redéfinit `bv` : suit la souris et cherche un ancrage assez proche |
| `mz:kz` | `pico8_mz_kz` | état d'approche, avant validation |
| `mz:pr` | `pico8_mz_pr` | état après validation : fait grandir le front de révélation |
| `mz:ej(t)` | `pico8_mz_ej` | rayon du front de révélation au tick `t` |
| `mz:ey` | `pico8_mz_ey` | valide le pliage |
| `fx` | champ | l'entité peut servir de point d'ancrage à un pliage d'espace |
| `ox` | champ | les ancrages candidats retenus à la création |

## Le slipway `jc`

| Nom | Symbole C | Sens |
|---|---|---|
| `jc` | `pico8_jc_class` | un slipway : le lien commercial entre deux planètes |
| `jc:pq` | `pico8_jc_pq` | initialise `ts` (points lumineux en transit) et `hg` (planètes desservies) |
| `jc:hq` | `pico8_jc_hq` | glisse l'extrémité libre à la souris |
| `jc:ey` | `pico8_jc_ey` | valide la construction et la paie |
| `jc:ce()` | `pico8_jc_ce` | qu'est-ce qui interdit ce slipway : trop long, croise un autre, ou existe déjà |
| `jc:eb(p)` | `pico8_jc_eb` | la planète à l'autre bout depuis `p` |
| `jc:ch(f,t)` | `pico8_jc_ch` | 8 % de chances par appel d'engendrer un point de transit de `f` vers `t` |
| `jc:kj` | `pico8_jc_kj` | dessine la ligne, avec une légère ondulation temporelle aux deux bouts |
| `qu`, `re` | champs | les deux extrémités du slipway |
| `ts` | champ | points lumineux en transit |
| `hg` | champ | ensemble des planètes que ce slipway dessert |

## Boîtes, boutons et menus

| Nom | Symbole C | Sens |
|---|---|---|
| `mh` | `pico8_mh_class` | boîte de message |
| `iv` | `pico8_iv_class` | toast flottant : monte lentement et s'efface en 50 ticks par changement de bloc de palette |
| `pz(self,bs)` | `pico8_pz` | construit une boîte à partir d'une liste de rangées et remplit `ok.le` avec ses icônes cliquables |
| `lq` | `pico8_lq_class` | une rangée d'icônes mise en page |
| `jb` | `pico8_jb_class` | une bulle d'info |
| `ol` | `pico8_lq_ol`, `pico8_jb_ol` | dessine la rangée ou la bulle |
| `js` | `pico8_js_class` | un bouton cliquable |
| `fq` | `pico8_fq_class` | fond modal : attrape tous les clics derrière une boîte |
| `my` | champ | callback de clic d'un bouton |
| `bo` | `pico8_bo` | les options de difficulté de l'écran-titre |
| `ix` | — | les 4 boutons du menu de la vue galaxie |
| `ha` | — | le menu pause : recommencer, musique, bruitages, aide |
| `fv` | `pico8_fv_class` | l'écran d'aide ; étend `g_` |
| `pj` | `pico8_pj_class` | le panneau de score ; `ju` est son singleton |
| `eg()` | `pico8_eg` | recalcule la palette du panneau de score selon l'avancement |
| `ie` | `pico8_ie_class` | icône d'amélioration de planète ; `ie:hb` la révèle quand sa techno est acquise, `ie:ldown` construit la classe correspondante |

## Recherche

| Nom | Symbole C | Sens |
|---|---|---|
| `hp` | `pico8_hp_class` | un projet de recherche |
| `hp:po()` | `pico8_hp_po` | table de production du projet, par palier |
| `hp:od()` | `pico8_hp_od` | coût du projet, réduit par les technologies acquises |
| `bj(jo)` | `pico8_bj` | les projets proposables sur une planète donnée |
| `gb` | `pico8_gb` | tous les projets construits depuis `fd` |
| `cb(t)` | `pico8_cb` | les technologies affichables ; `t` vrai ignore le coût |
| `gi(t)` | `pico8_gi` | coût en points de recherche d'une technologie |

## Particules

| Nom | Symbole C | Sens |
|---|---|---|
| `f_(self)` | `pico8_f_` | initialise la liste de particules `qy` ; sert de `pq` à plusieurs classes |
| `ez(self,n)` | `pico8_ez` | engendre des particules en appelant `self:mm()`, avec une probabilité décroissante |
| `ei(self)` | `pico8_ei` | avance et dessine les particules, retire celles dont la durée de vie est épuisée |
| `qy` | champ | liste de particules |
| `mm` | méthode | forme d'une nouvelle particule ; chaque classe émettrice a la sienne |

## Amorçage et boucle de jeu

| Nom | Symbole C | Sens |
|---|---|---|
| `jl()` | `pico8_jl` | (re)construit les registres d'entités et les singletons `mo`, `je`, `ok` |
| `dg()` | `pico8_dg` | une frame de logique : appelle sur chaque entité la méthode qui porte le nom de son état `me`, puis détruit celles dont `p_` est levé |
| `bw(n)` | `pico8_bw` | peuple la galaxie de `n` planètes à des positions tirées au sort, puis installe les générateurs `jo.oc`/`jo.ir` |
| `eu(nj)` | `pico8_eu` | démarre une partie avec l'option de difficulté choisie : construit `dk`, `fm` et `ju` |
| `bl(…)` | `pico8_bl` | met en page et dessine une rangée d'icônes |
| `_init`, `_update60`, `_draw` | `pico8__init`, `pico8__update60`, `pico8__draw` | les trois points d'entrée PICO-8 du cart |

## Le vocabulaire des noms de méthode

Ces noms reviennent sur presque toutes les classes. Les cinq premiers sont
ceux de `bm` : `dc(nom)` les envoie à toutes les entités inscrites. Les
autres sont atteints par `mv()` (lookup par nom, sans restriction) ou par
`dg()`, qui appelle chaque frame la méthode portant le nom de l'état `me`.

| Nom | Rôle |
|---|---|
| `pq` | constructeur supplémentaire, appelé une fois à l'instanciation |
| `kj` | dessiner l'entité *(nom `bm`)* |
| `ev` | dessiner par-dessus, après toutes les couches *(nom `bm`)* |
| `hd` | test de survol : le curseur est-il sur moi *(nom `bm`)* |
| `hk` | tick d'année *(nom `bm`)* |
| `hb` | recalculer mon affichage après un changement d'économie *(nom `bm`)* |
| `ldown`, `rdown` | appui du bouton gauche / droit sur moi |
| `hq` | appelé chaque frame tant que le bouton reste enfoncé sur moi |
| `ey` | relâchement : valider l'action |
| `gv` | puis-je être la cible du glissé en cours |
| `ph` | tick ordinaire ; c'est l'état `me` par défaut de `ke` |
| `dq` | l'état dans lequel `d_` fait basculer l'entité après validation, via `ks(self.dq or "ph")`. **C'est le seul chemin qui mène à `ly`** : aucun site n'écrit `:ly(`, ces méthodes ne sont atteintes que parce que `dq="ly"` devient `me`, et que `dg()` appelle chaque frame la méthode qui porte le nom de l'état. Un `grep ly` conclura à tort à du code mort |
| `ly`, `kz`, `pr`, `ll` | autres états, appelés par `dg()` quand `me` vaut ce nom |
| `mm` | forme d'une nouvelle particule |
| `ol` | dessiner une rangée d'icônes ou une bulle |
| `lj` | recalculer tout (l'économie pour `hy`, une frame d'interface pour `ui`) |

## Divers

| Nom | Symbole C | Sens |
|---|---|---|
| `kc` | `pico8_kc` | la boîte de message actuellement affichée, ou rien |
| `ju` | `pico8_ju` | le singleton du panneau de score, instance de `pj` |
| `cr()` | `pico8_cr` | l'écran de l'arbre technologique |
| `ef` | `pico8_ef` | les 15 abréviations de période (`JAN`…`TER`), indexées par le compteur de ticks `fm.lo` |
| `bz` | `pico8_bz` | les rangées du détail de score |
| `de` | `pico8_de` | **pas une table de mise en page** : la table de rappel partagée entre `pj:ph` et `ix[3]`, dont le `my` bascule la visibilité du panneau. Il ne doit y en avoir qu'une |
| `ix` | `pico8_ix` | les 4 boutons de la vue galaxie |
| `ha` | `pico8_ha` | le menu pause |
| `jc:lg` | `pico8_jc_lg` | dessine une des lignes du slipway |
| `je` | `pico8_je` | le singleton lecteur de souris, instance de `nf` |

## Champs de classe et d'instance, par thème

Les noms qui reviennent d'un module à l'autre. Un même digramme peut avoir
deux sens dans deux fichiers voisins — c'est signalé quand c'est le cas.

### Animation, particules, rendu

| Nom | Sens |
|---|---|
| `oi` | rayon du cercle en expansion d'un trou de ver |
| `kk` | rampe de 9 couleurs indexée par le rayon relatif (voir TROU-12) |
| `qy` | liste de particules vivantes |
| `pn`, `pk`, `du`, `py`, `ki` | forme d'une particule : tranches angulaires, rotation d'un quart de tour, rayon de base, facteur de vélocité, durée de vie |
| `k_` | palette de la traînée de particules |
| `l` (sur `iv`) | avancement du fondu, 0 → 5 ; sert d'indice de bloc de palette et déclenche `p_` à 5 |
| `qw` | index de bloc de palette du sprite ; posé par `nl`, absent = 0 |
| `s` | identifiant de sprite |
| `ka` | couleur de l'anneau d'une planète colonisée, indexée par `mi` |
| `dz` | les quatre tuples d'état `unhappy`/`content`/`prosperous`/`rich`, indexés par `mi` |
| `ng` | décalage de position du `ga` engendré par `pm` |

### Boîtes, rangées, mise en page

| Nom | Sens |
|---|---|
| `rh` | échelle de durée d'animation d'une boîte : `t/rh - 10` donne ouverture, plateau, fermeture |
| `lh` | prédicat « garder la boîte ouverte » ; tant qu'il rend vrai, le compte à rebours de fermeture est gelé |
| `iw` | la liste de rangées passée à `mh(iw)` |
| `hi` | position de pose, prioritaire sur `ra` |
| `gq` | pas de progression d'une rangée à la suivante |
| `ca` | couleur de fond d'une boîte, ou couche `fa` imposée aux boutons créés par `pz` |
| `ff` | vider `ok.le` juste après l'avoir rempli |
| `eo` | numéro de sfx joué par `pz` (62 par défaut) |
| `h_` | seconde rangée d'un bouton : l'infobulle dessinée à côté |
| `mb` | largeur du bouton, dérivée de celle de sa rangée `lq` |
| `qf` | position cible vers laquelle `ph` fait glisser `ra` |

### Économie, commerce, recherche

| Nom | Sens |
|---|---|
| `gc` | combien de routes arrivées satisfont déjà ce besoin ; posé par `dh`, absent = 0 |
| `cq` | les quatre ressources qu'une planète « makes: » au lieu d'« exports: » |
| `ba` | ressources qu'une sous-classe accepte, lues par `jo:cn` |
| `ht` | ressources qu'un `mx` sait synthétiser |
| `hz` | sprite choisi selon le nombre de connexions (`nr`, `fr`) |
| `oy` | contenu de l'infobulle d'une technologie |
| `e_` | « finançable », posé par `cb()` ; toujours un booléen |
| `lv` | ère d'une technologie, comparée à `fm.ew` |
| `en` | champ de réglage d'une entrée de `bo`, à côté de `trade` |

### Panneau de score

| Nom | Sens |
|---|---|
| `ea` | la rangée du bandeau d'année, réécrite par `pj:hk` |
| `gs` | messages de compte à rebours, indexés par `3426 - yr` |
| `i_`, `ou` | rangées du cadre / les 7 afficheurs réécrits par `pj:hb` |
| `ed` | la dernière boîte construite par le panneau, comparée à `kc` |
| `gz` | compteur du rideau de fin, testé par **présence** et non par valeur |
| `c_` | « panneau visible », basculé par le `my` de `de` |
| `gy`, `eh` | sur la classe `ie` : `eh` est la valeur de repos, `gy` la valeur courante partagée par toute la classe. `_init()` fait `gy = eh` à chaque démarrage, `ie:hb` réécrit `gy` |
| `nj` | le bouton de difficulté cliqué, dont la table de champs **devient** `dk` |

### Sonde et validation

| Nom | Sens |
|---|---|
| `np` | étiquette `nu` portée par toutes les planètes ; `bv` cherche la plus proche dans `cg("np")` |
| `fp` | nombre de ticks que `d_` transmet à `fm:nx` ; `g_` le pose à faux, d'où le repli 0 |
| `ik` | sprite d'état lu par `dl`, et numéro de sfx à la validation réussie |
| `jj` | la pseudo-cible « -too far- » posée dans `et` quand aucun ancrage n'est assez proche |
| `ij` (champ) | l'ancrage retenu par `mz:hq` — homonyme de la fonction `ij`, qui cherche le plus proche voisin |
| `spawn_snd` | posé à 52 dans la spec de `mz`, aucun lecteur (audio hors périmètre) |

### Deux noms, deux sens

| Nom | Selon le fichier |
|---|---|
| `hp` | la **classe** d'un projet de recherche — mais sur une sous-classe de planète, un **nombre** de 1 à 6 : l'index du projet correspondant, lu par `jo:hu` |
| `nk` | l'aléa courant d'une planète — mais sur une entrée de `nb`, l'**icône** de la technologie |
| `cr` | l'écran de l'arbre technologique — mais `pj.cr` est un bloc de mise en page |
| `ij` | la fonction « plus proche voisin » — mais sur `mz`, le champ qui retient l'ancrage choisi |
| `l` | la durée de vie d'une particule — mais sur `iv`, l'avancement du fondu |

### Projets de recherche et icônes d'amélioration

| Nom | Sens |
|---|---|
| `fj` | coût de base d'un projet, avant les modificateurs de `hp:od` |
| `ja` | clé du projet dans `hc`, qui donne sa table de production |
| `il` | technologie qui **améliore** le projet : coût ×1,5 et production prise sur l'entrée améliorée |
| `mc` | sur une entrée de `hc`, le nom de l'entrée améliorée que `hp:po` substitue |
| `ii` | marqueur « ce projet est une technologie » : supprime la bulle, fait écrire un `lq` **donnée** au lieu d'une méthode |
| `lr` | codes de type de planète qu'un projet accepte, comparés à `jo.nk.px` |
| `i` | ressources requises, développées en liste de `{qe=…}` |
| `o` | ressource produite |
| `dm` | routé par `gp` comme `o` ; aucun lecteur trouvé |
| `fk` | l'entrée `{false,2}` que `hp:lq` ajoute en fin de rangée |
| `mr`, `gp` | carte de renommage positionnel lue par `ho`, et le sous-ensemble dont la valeur est routée à travers `r` — c'est `gp` qui rend `o`/`dm` identiques au pointeur de `r`, donc qui justifie l'exception de TROU-3 |
| `ke` (sur `ie`) | la **classe** d'amélioration que l'icône construit, pas une instance |
| `kh` | drapeau d'instance « icône déjà révélée » |
| `id` (sur `ie`) | compteur de classe, source de l'override de sprite |
| `jo.em` | l'entrée d'espacement que `jo.ex` ajoute après chaque élément |

### Locales du parseur et de la boucle d'entités

Elles n'existent pas hors de leur fonction, mais le port a gardé leurs noms.

| Nom | Sens |
|---|---|
| `rj` | dans `kv:jr`, la table de spec qui **devient** la classe |
| `ko`, `iy` | remontée de chaîne à l'instanciation : la classe courante, et le dernier `pq` appelé. `iy` porte toute la règle de déduplication |
| `qc`, `fl` | dans `dc()` : l'entité visitée, et les 15 seaux par couche |
| `kn`, `s`, `n`, `lm`, `sc`, `rq` | dans `ob()` : table résultat, début du token, clé courante, profondeur de parenthèses, premier caractère du token, position suivante |
| `f` | dans une spec, l'identifiant nu `f` vaut le **faux** littéral — c'est ce qui rend `fp=f` différent de `fp=0` |

## Conventions de propriété et de représentation

Transversales, et trois trous en dépendent. Aucune n'est portée par un nom.

| Convention | Règle |
|---|---|
| Emprunt de pointeur | un `pico8_value_t` ne possède **jamais** ce que pointent ses genres `STRING` et `REF`. `pico8_list_free` ne libère que son tableau, `pico8_ob_table_free` que ses clés |
| L'inverse, pour les chaînes construites | `tostr`, `sub`, `unstash` et `ne` rendent une chaîne **possédée par l'appelant**, à libérer |
| `pico8_value_ref(NULL)` | l'emplissage « valeur absente ». C'est lui qui fait exister TROU-11 et TROU-13 : il traverse les dispatches et les conversions sans se distinguer d'une vraie valeur |
| Le tag de type | `PICO8_TYPE_TAG_*` en premier membre des trois formes qui modélisent la table Lua ; `PICO8_REF_AS_LIST` le vérifie. Quatre débordements de tas en sont nés, d'où ce mot de 4 octets qu'aucun code de jeu ne lit |
| Les primitives de liste | `add` → `pico8_list_add`, `del` → `pico8_list_del` (supprime par **valeur**, pas par index), `all` → `pico8_list_all` + `_iter_next` (tolère la suppression de l'élément courant, pas d'un précédent), `foreach` → `pico8_list_foreach`. Ce sont celles de PICO-8, pas celles de Lua |
