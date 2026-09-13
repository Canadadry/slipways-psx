# Slipways (PICO-8) — règles de design & mécaniques

Document de référence pour la réimplémentation intégrale de la version PICO-8 de *Slipways* (@krajzeg / @gruber_music), portée sur manette pour PlayStation 1 (Love2D en prototype de référence, voir `docs/plan.md`). Les mécaniques et valeurs ci-dessous sont extraites directement du code source (`lua.lua`) et des données embarquées dans la cartouche (`map.json`) ; le pitch et les instructions officielles viennent de la page itch.io.

## Pitch officiel (itch.io)

> Cette édition PICO-8 (« Slipways Classic ») est disponible en *pay-what-you-want* ; la version PC ajoute un conseil de cinq races aux technologies et philosophies différentes, un mode campagne rejouable, et des graphismes procéduraux revus, en gardant le gameplay rapide de l'original.

Instructions condensées du guide de démarrage rapide du jeu (texte original, anglais) :

> « Drag the map around using the right mouse button or the SDFE keys. Click to drop a wormhole that will become your starting point. Then click and drag to find some planets, which you can colonize with your chosen industry type. Drag from planet to planet to connect them with a slipway. Try to connect planets that make stuff with planets that need it. Buy structures by dragging them from the bottom-right corner. Research technologies from the top-left corner. Click the heart symbol to check your score. Make the biggest, baddest, most prosperous empire possible! »

Points de conception confirmés par le développeur (interview sur le forum PICO-8, lexaloffle) :
- **Une seule unité de ressource par slipway et par échange** : contrainte assumée pour éviter le micro-management, même si plusieurs sources partagent parfois la même icône visuelle. Confirme la mécanique `gc` (nombre de livraisons comptées, pas de quantité) décrite au §9.
- **Approche de la difficulté** : en facile, il suffit de rendre les planètes « content » pour tourner ; en difficile, il faut repérer des planètes « hub » pour concentrer l'activité commerciale et maximiser les profits.
- Le déblocage progressif de certaines technologies (ex. Slipgates) était un choix de rythme assumé, même si des retours de joueurs suggéraient un déblocage plus précoce.
- Contrainte technique : le format PICO-8 (limite de tokens) a forcé une minification agressive du code, avec des compromis de lisibilité/fonctionnalités qui ne s'appliquent pas à la version PC commerciale — à garder en tête en lisant les sections « déduit » de ce document.

## Méthode & fiabilité

- `lua.lua` est le code Lua minifié tel qu'exporté par PICO-8 (noms de variables réduits à 1-2 lettres). Toute la logique de jeu est dedans.
- Une partie des données de contenu (noms de ressources, table des technologies, table des bâtiments/colonisations, courbes de population, descriptions d'infobulles) n'est **pas** dans le code Lua : le jeu les stocke sous forme de texte compressé, planqué dans la zone mémoire de la map non utilisée visuellement (lignes 32-63 de la map PICO-8, adresses `0x1000-0x1FFF`), et les relit au démarrage avec une fonction `unstash()`. C'est une astuce classique de jam PICO-8 pour économiser des tokens de code.
- Ces données ont été retrouvées en décodant `map.json` (le tableau `cells`, indices `4096..8191` = adresses `0x1000..0x1FFF`) via la table de caractères `gt` codée en dur dans `lua.lua` (ligne ~68). Chaque octet 1-51 correspond à un caractère de `gt = "abcdefghijklmnopqrstuvwxyz0123456789 ().,=-+_/\"'?%\n"`, un octet `0` termine la chaîne.
- Statut de confiance :
  - **Confirmé** : valeur/texte lu directement dans les données (noms de ressources, techs, bâtiments, textes d'UI, constantes numériques).
  - **Déduit** : comportement reconstruit en lisant la logique du code autour de la donnée (formules, enchaînements d'états).
  - **Incertain** : signalé explicitement quand une valeur existe mais que son usage exact n'a pas pu être confirmé en suivant le code (ex. la ressource `a`).

Tables brutes décodées : voir [Annexe A](#annexe-a--tables-brutes-décodées).

---

## 1. Concept général

*Slipways* est un jeu de stratégie spatiale au tour... plutôt en temps continu (voir §2) où le joueur :
1. explore une galaxie de planètes majoritairement inconnues avec des **sondes**,
2. colonise les planètes découvertes en choisissant une **spécialisation** adaptée au type de planète,
3. relie les colonies entre elles par des **slipways** (routes commerciales) pour que les ressources circulent automatiquement,
4. gère bonheur, technologie et argent pour faire grossir son score avant la fin de partie.

Il n'y a pas de combat, pas d'ennemis actifs : le jeu est un puzzle d'optimisation économique/logistique sous contrainte de temps.

## 2. Boucle de jeu & temps

- Le temps s'écoule en continu (`_update60` tourne à 60 fps), mais la simulation économique (`hy:lj`, ligne ~1226) est recalculée en continu et un « tick annuel » (`hy:hk`, ligne ~1210) se déclenche tous les *N* mois simulés.
- `hy:nx(hx)` avance l'horloge de `hx` mois (fonction appelée à chaque action qui coûte du temps : sonde lancée, colonisation, construction). Le seuil pour faire avancer une année est `12` mois de base, `15` avec la technologie **time compression** (`ov("time compression",12,15)`, ligne 1204).
- La partie démarre à l'année **3401** (`yr=3401`) et se termine à l'année **3426**, soit **25 années simulées** au total (`hy.hl` passe à `"final score"` quand `yr>=3426`, ligne 1236).
- Défaite anticipée : **banqueroute**. Si le revenu net annuel `ge<=0` **et** la trésorerie `nz<3`, `hy.hl="bankrupt"` et la partie s'arrête (même ligne).
- Chaque action qui « prend du temps » (envoyer une sonde, coloniser, construire une structure) consomme un nombre de mois fixe (`fp`, souvent `1` ou `2`, `3` par défaut pour une colonisation normale via `autoassemblers` qui le réduit à `2`).

**Pour le portage PSX** : le modèle « continu + tick annuel » reste tel quel (aucun changement de modèle de temps décidé) — toutes les formules ci-dessous sont exprimées par année.

## 3. La galaxie

- **Pas de planète de départ imposée** : contrairement à beaucoup de 4X, la galaxie démarre entièrement vide de possessions du joueur. Le premier clic du joueur sert à « lâcher un wormhole » qui devient son point de départ (texte officiel : « Click to drop a wormhole that will become your starting point », confirmé côté code par `bg:ldown`, ligne 376 : tant qu'aucune entité du groupe `g_` n'existe, le clic en crée une plutôt que de lancer une sonde `mz`). Tous les clics suivants sur une zone vide lancent des sondes normalement.
- Génération procédurale au lancement d'une partie (`bw(n)`, ligne 744) : `n` planètes (**90** dans le code, `bw(90)`) sont placées aléatoirement sur une carte carrée, avec un rejet si deux planètes sont trop proches (`ij(p,cg.jo)>0.05`).
- Toutes les planètes démarrent **inconnues** (`me="unknown"`) — aucune information visible tant qu'elles n'ont pas été sondées.
- **Découverte** (`jo:hh`, ligne 407) : quand une sonde révèle une planète, il y a une chance (**~45 %**, tirage `jo.ir()<0.45` avec une marche aléatoire à retour à la moyenne, indépendante de la difficulté) que la planète soit **vide/inutilisable** (`df.ll`, type spécial « aucune ressource, aucune colonisation possible »). Sinon, son type réel est tiré par `jo.oc()`.
- **Type de planète tiré** (`gn`, ligne 732) : le tirage favorise les types les plus « désirables » en fonction de la difficulté — le paramètre `en` de la difficulté (voir §12) biaise une marche aléatoire (`jf`) qui, plus il est négatif, pioche systématiquement plus tôt dans le pool pondéré (construit dans l'ordre du tableau des types, en tête de liste = terrestre). *Forgiving* (`en=-0.99`) tend donc à donner plus souvent de bonnes planètes (terrestres) que *Tough* (`en=-0.2`), qui tire quasi uniformément.
- **Anti-répétition douce** : le générateur évite de retirer 2-3 fois de suite exactement le même type de planète (tableau `nm` dans `gn`).
- **10 types de planètes** (table `df`, confirmé — voir tableau ci-dessous) + le type spécial « inconnue/vide » (`ll`).

| Code | Nom affiché | Rareté relative (`r`, plus haut = plus fréquent) |
|---|---|---|
| `e` | earth-like | 4.5 |
| `f` | forgeworld | 6 |
| `m` | mineral | 4 |
| `o` | ocean | 3 |
| `r` | remnant | 2.5 |
| `x` | xeno | 3 |
| `j` | jungle | 2 |
| `i` | iceball | 2 |
| `s` | barren | 2 |
| `g` | gas giant | 4 |
| `ll` | (inconnu/vide) | 0 — jamais tiré directement, c'est le résultat du jet « planète vide » |

## 4. Sondes & découverte

- Une sonde (`mz`, ligne 752) coûte **3 crédits** (`od=3`) et **3 mois** de temps.
- Portée de lancement limitée : une sonde ne peut viser un point que si ce point est à une distance raisonnable d'une planète déjà colonisée (`self.ox` = planètes « fondées »). Seuil `0.19` (unités normalisées, monde = carré 0..1), étendu à `0.27` avec la technologie **space folding** (`ov("space folding",0.19,0.27)`, ligne 768). Au-delà, la destination est refusée (« -too far- »).
- Une fois arrivée, la sonde balaie une zone circulaire qui grossit sur ~25 « frames » de jeu (`mz:pr`, ligne 776) et révèle (`pt:hh()`) toutes les planètes inconnues dans son rayon final `oi`, dont la taille dépend aussi de la difficulté (`ov("space folding",29,35)`, ligne 772 — même override de tech, mais valeurs différentes de celles du seuil de portée ; à ne pas confondre).

## 5. Slipways (routes commerciales)

Une **slipway** (`jc`, ligne 612) est une route persistante reliant deux planètes/nœuds, tracée en cliquant-glissant d'un point à l'autre. Une fois construite, le commerce entre les deux extrémités devient automatique (§8).

- **Coût** (`dj`, ligne 1259) : `floor(6 * bk() * max(0.5, (distance/0.0977)^0.75))` crédits, où `bk()=max(1, sqrt(nombre_de_slipways_existantes)*0.27-0.1)`. Autrement dit : le coût croît avec la distance (puissance 0.75, donc sous-linéaire) et augmente légèrement avec la taille globale du réseau déjà construit (inflation douce, ex. +25 % de coût à 25 slipways existantes).
- **Portée max** : une slipway ne peut pas dépasser une distance normalisée de `0.5`, étendue à `1.08` (plus du double) avec **space folding** (`ov("space folding",0.5,1.08)`, ligne 642).
- **Règle de croisement** : deux slipways « normales » (`mt=0`) ne peuvent pas se croiser géométriquement — la construction est bloquée si le segment coupe une route existante du même type (`cl()`, test d'intersection de segments). Les **infragates** (voir §6) créent des routes de type `mt=1` qui, elles, peuvent librement passer sous/à travers les routes normales (texte confirmé : « can cross other slipways »).
- **Interdiction de passer par-dessus une planète** : une route ne peut pas non plus être tracée si elle frôle une autre planète existante sur son trajet (tolérance `0.0032`) — il faut alors router explicitement via cette planète (elle devient un relais).
- **Capacité des nœuds** :
  - Une **planète colonisée normale** peut émettre un nombre **illimité** de slipways (confirmé : `jo:ldown` crée directement une nouvelle route sans aucune vérification de quota, ligne 412-417).
  - Un **Slipgate**/**Infragate** (structures dédiées, voir §6) est limité à **3 connexions** (`#self.oe<3`, ligne 1110). Ce sont des relais à construire pour étendre/complexifier le réseau au-delà de la portée d'une planète isolée.
- **Doublon** : si une route existe déjà entre deux mêmes extrémités, en reconstruire une seconde est sans effet (le jeu réutilise la route existante, `jc:ce`).

## 6. Structures spéciales (non-colonisables, posées sur un slipgate/relais)

Sept structures, avec leur description d'infobulle telle qu'affichée dans le jeu (confirmé, table `dr`) :

| Structure | Coût (crédits) | Description en jeu |
|---|---|---|
| **Laboratory** (`qh`) | 15 | « makes science when supplied with [people] and any resource — adding more of that resource increases output » |
| **Food processor** (`os`) | 15 | « converts organics into food » |
| **Slipgate** (`nr`) | 10 | « connects up to 3 slipways » |
| **Infragate** (`fr`) | 25 | « connects up to 3 slipways · routes through infragates can cross other slipways » |
| **Trading hub** (`nl`) | 20 | « generates [joy]/[énergie?] once you connect 2 or 3 different resources » |
| **Synthesizer** (`mx`) | 40 | « can provide food/organics/bots/tech to a planet that needs it » |
| **Ascension gate** (`hj`) | 60 | « accepts people and turns them into joy/happiness » |

Notes :
- Ce ne sont pas des « planètes » à part entière au sens colonisation : ce sont des nœuds de production/logistique posés dans l'espace (probablement sur un point vide sélectionné, comme un Slipgate), débloqués un par un par la table des technologies (§7).
- Le **Synthesizer** est spécial : il n'a pas de production fixe — il regarde dynamiquement de quoi ses partenaires commerciaux ont besoin parmi `{organics, bots, tech, goods}` et fabrique celle-là (`mx:kf`, ligne 1049).
- La **Trading hub** a un comportement de croissance différent des colonies classiques : son niveau dépend du *nombre de ressources différentes* effectivement reçues par le commerce (jusqu'à 3), pas d'une seule ressource (`dh`, ligne 1303).
- Il existe une 8ᵉ entité liée, le **Protostar** (`pm`), mais elle n'est pas achetable directement : c'est le résultat de l'action « collapse » (§7, tech starbirth) qui transforme une planète gazeuse/minérale en étoile productrice d'énergie en continu.

## 7. Ressources

11 ressources nommées + 1 ressource « joker » (confirmé, tables `r`/`iz`) :

| Nom | Rôle typique |
|---|---|
| food | consommée pour faire croître la population |
| ore | matière première industrielle |
| bots | main d'œuvre robotique / intrant industriel |
| people | population, sert d'intrant à plusieurs colonisations et à l'Ascension Gate |
| goods | produit manufacturé, sert de « luxe léger » |
| tech | recherche appliquée locale |
| organics | matière première biologique |
| luxury | ressource de confort, alimente le bonheur (voir §10) |
| science | fait avancer l'arbre technologique |
| energy | ressource universelle produite par le Protostar, comble n'importe quel besoin |
| joy | bonheur produit directement (Ascension Gate), alimente aussi le bonheur global |
| *(joker)* `?` | un « besoin flexible » — se résout automatiquement avec la première ressource compatible livrée (ex. le slot flexible du Laboratory accepte food/bots/organics/ore/tech/goods) |

Ressources vs. affichage « makes » / « exports » : dans l'UI, `luxury`, `joker`, `science` et `joy` sont traitées comme des ressources **abstraites/consommées sur place** (« makes: N ») tandis que les autres s'affichent comme des **exports physiques** comptés (« exports: n/total », table `jo.cq`, ligne 398).

> **Incertain** : le code réserve une 13ᵉ clé de ressource, `a` (valeur brute 33), présente dans la table `r` mais dont aucun usage n'a été retrouvé dans le code parcouru. Peut être un reliquat non utilisé dans cette version. À vérifier si besoin avant de la porter.

## 8. Colonisation : la table des spécialisations

Coloniser une planète = choisir l'une des « jobs » compatibles avec son type. Chaque job définit : les planètes où il est disponible, son coût, ce qu'il consomme (« needs »), ce qu'il produit, et sa courbe de production selon le niveau de la colonie (§9).

Table complète décodée (confirmée), codes de planète et de ressource traduits :

| Job | Planète(s) | Coût | Besoin(s) | Produit | Courbe | Déblocage / effet tech |
|---|---|---|---|---|---|---|
| nanotech | forgeworld | 8 | ore | tech | `-` | dispo dès le début |
| bots | forgeworld | 8 | ore + tech | bots | `+` | dispo dès le début |
| goods | forgeworld | 10 | ore + bots | goods | `+` | dispo dès le début |
| gadgets | forgeworld | 10 | ore + tech | goods | `+` | dispo dès le début |
| hiveworld | earth-like | 5 | food | people | `+` | dispo dès le début |
| colony | ocean | 10 | ore + food | people | `-` | +50 % coût une fois *gene rewriting* recherché |
| colony | xeno | 10 | tech + food | people | `-` | +50 % coût une fois *gene rewriting* recherché |
| mine | mineral | 5 | people | ore | `-` | dispo dès le début |
| strip | remnant | 5 | people + food | ore | `-` | dispo dès le début |
| agroworld | earth-like | 5 | bots | food | `-` | courbe améliorée après *xenofoods* |
| algae | ocean | 8 | bots | food | `-` | courbe améliorée après *xenofoods* |
| breed | ocean | 5 | people | organics | `+` | dispo dès le début |
| breed | xeno | 5 | bots | organics | `-` | dispo dès le début |
| hunt | jungle | 5 | people | organics | `-` | dispo dès le début |
| dismantle | remnant | 5 | people + food | tech | `-` | dispo dès le début |
| scavenge | remnant | 5 | people + food | bots | `-` | dispo dès le début |
| tourism | jungle | 10 | people + goods | luxury | `f-` | dispo dès le début |
| luxury | earth-like | 10 | tech + goods | luxury | `f-` | dispo dès le début |
| farm | xeno / jungle | 10 | bots | food | `-` | débloqué par **xenofoods** |
| habitat | barren / iceball | 12 | organics + food | people | `-` | débloqué par **geneseeds** |
| genefarm | barren / iceball | 12 | organics + bots | food | `-` | débloqué par **geneseeds** |
| excavate | mineral | 12 | bots | ore | `+` | débloqué par **drillbots** |
| excavate | iceball / gas giant | 12 | bots | ore | `-` | débloqué par **drillbots** |
| uplift | forgeworld | 20 | organics | people | `-` | débloqué par **gene rewriting** |
| **mechanize** *(transforme la planète)* | mineral/remnant/barren/ocean/iceball/jungle/xeno | 20 | — | *devient une forgeworld* | — | débloqué par **replication** |
| **terraform** *(transforme la planète)* | xeno/iceball/barren/ocean/jungle | 20 | — | *devient une earth-like* | — | débloqué par **biome hacking** |
| **collapse** *(détruit la planète)* | gas giant / mineral | 30 | — | *devient un Protostar (énergie continue)* | — | débloqué par **starbirth** |
| **harvest** *(détruit la planète)* | tous types | **rembourse 20 crédits** | — | *planète détruite* | — | débloqué par **geoharvesting** |

Structures spéciales (§6), pour mémoire : Laboratory (besoin people + joker parmi food/bots/organics/ore/tech/goods → science, courbe `qh`), Ascension Gate (besoin people → joy, courbe `f+`), Trading Hub (2 besoins flexibles → luxury, courbe `f-`), Synthesizer (aucun besoin fixe → ressource dynamique, courbe `f-`).

Règle de « repli automatique » : au niveau de population 1 (juste colonisée), si le job a un champ « besoin par défaut » (`dm`), ce besoin est ajouté automatiquement à la liste de la colonie (`bb`, ligne 1280) — ça évite qu'une colonie toute neuve n'ait rien à demander avant que le joueur ne configure ses échanges.

## 9. Population, croissance & niveaux

Chaque colonie a un niveau `mi` de 1 à 4, avec nom et couleur d'anneau associés (confirmé, table `jo.dz`) :

| Niveau | Nom | Couleur (palette PICO-8) |
|---|---|---|
| 1 | unhappy | 8 (rouge) |
| 2 | content | 3 (vert foncé) |
| 3 | prosperous | 9 (orange) |
| 4 | rich | 12 (bleu clair) |

**Calcul du niveau cible chaque année** (`dh`, ligne 1291, déduit) :
1. Une colonie non fondée = niveau 0.
2. Pour chaque ressource demandée par la colonie, on compte combien d'unités sont effectivement livrées cette année (`gc`). Si **au moins un** besoin n'est pas satisfait du tout, le niveau cible retombe à **1**.
3. Sinon :
   - Trading Hub (`nl`) : niveau = nombre de ressources différentes effectivement reçues (plafonné à 3).
   - Structures spéciales à un seul besoin (Laboratory, Ascension Gate, Synthesizer, Protostar…) : niveau = quantité reçue du dernier besoin listé + 1, plafonné à 4.
   - Colonie normale : niveau = `1 + médiane(nb. d'envois reçus, nb. de partenaires commerciaux, 1)`, plafonné à 4.
4. **Le niveau ne redescend jamais tout seul** — il ne fait que stagner si les besoins ne sont plus satisfaits (le code ne met à jour `mi` que si la nouvelle valeur calculée est *strictement supérieure* à l'actuelle). Une colonie qui grossit reste acquise à son meilleur niveau, sauf blocage total de ses imports qui la ramène à 1.

**Production annuelle** (`bb`, ligne 1276) : `production = courbe[min(niveau, longueur_courbe)] + besoin_par_défaut_optionnel`. Voir §8 pour les 6 courbes disponibles (`-`, `+`, `f-`, `f+`, `qh`, `qh+`) et leurs valeurs par palier :

| Courbe | Niveau 1 | Niveau 2 | Niveau 3 | Niveau 4 |
|---|---|---|---|---|
| `-` | 1 | 2 | 3 | 3 (plafond) |
| `+` | 2 | 3 | 4 | 4 (plafond) |
| `f-` | 0 | 1 | 2 | 3 |
| `f+` | 0 | 2 | 3 | 4 |
| `qh` (Laboratory) | 0 | 1 | 3 | 5 |
| `qh+` (Laboratory + *superhuman ai*) | 0 | 2 | 5 | 8 |

Deux upgrades de courbe confirmées : **xenofoods** fait passer le Food processor de `f-` à `f+` ; **superhuman ai** fait passer le Laboratory de `qh` à `qh+`. D'autres jobs ont un champ « tech qui augmente son coût de 50 % une fois recherchée » (§8, colonie ocean/xeno via *gene rewriting*) — un levier volontairement différent (pénalité, pas bonus), pour pousser vers de meilleures options une fois débloquées.

## 10. Économie

- **Revenu commercial par colonie** (`db`, ligne 1325) : pour chaque envoi commercial actif vers un partenaire, `+trade_difficulté × sensibilité_locale(hs) × (1 + bonus_niveau(soi) + bonus_niveau(partenaire))`, avec bonus de niveau `{-0.333, 0, 0.25, 0.5}` pour les niveaux 1-4. Si la colonie exporte vers l'extérieur du réseau commercial (`kw.qe==énergie`… en fait la ressource « joker » globale), bonus supplémentaire ×1.5. `trade_difficulté` vaut 7/6/5/4 selon la difficulté (§12).
- **Maintenance par colonie** (`da`, ligne 1312) : les Laboratory coûtent chacun **le nombre total de Laboratory déjà construits** (coût progressif à mesure qu'on en construit). Les colonies normales coûtent selon leur niveau : `{unhappy:1, content:0, prosperous:1, rich:2}` — le niveau « content » est le seul gratuit à entretenir.
- **Frais généraux d'empire** (`cz`, ligne 1319) : `floor(0.18 × (nombre de colonies niveau≥1)²)`, retranché du revenu total chaque année — croissance quadratique, un empire qui s'étend trop vite sans consolider devient cher.
- **Revenu annuel net** : `floor(Σ(revenu - maintenance par colonie) × (1.15 si tech skill implants sinon 1)) - frais_généraux`.
- **Dépense d'argent** (`hy:lu`, ligne 1217) : bloque l'action si la trésorerie est insuffisante (son « -no money- » joué), sinon débite.

## 11. Bonheur

Formule (`bf`, ligne 1365, confirmée) :

```
bonheur% = 100 + 2 × (production_totale_joy + production_totale_luxury)
         + Σ ajustement_par_colonie[niveau]
ajustement_par_niveau = {unhappy: -5, content: 0, prosperous: +1, rich: +1}
```

Le bonheur n'est pas plafonné à 100 % : produire beaucoup de `joy`/`luxury` peut le pousser bien au-dessus, et de nombreuses colonies « unhappy » peut le faire chuter fortement. Il agit comme un **multiplicateur global du score final** (§13), pas seulement un indicateur cosmétique.

## 12. Technologies

20 technologies (confirmé, table `nb`), réparties sur **4 niveaux d'ère** (`lv` 1-4, 5 par niveau). On avance d'ère quand on a inventé un nombre cumulé suffisant de technologies : seuils `{2, 4, 7, 12}` inventions cumulées pour débloquer les ères 2/3/4 (`ek`, ligne 1445 — la 5ᵉ tech de chaque palier de 5 n'ouvre donc pas immédiatement l'ère suivante, il faut atteindre le total cumulé). Chaque tech coûte de la **science**, avec un coût individuel qui grimpe avec le nombre de technologies déjà de ce niveau inventées (`gi`, ligne 1471 — dépend d'un facteur `fu` par niveau non détaillé ici, cf. code).

| Ère | Tech | Effet (texte en jeu) |
|---|---|---|
| 1 | space folding | plus longue portée pour slipways et sondes |
| 1 | geneseeds | débloque colonisation habitat/genefarm sur planètes barren/iceball |
| 1 | nanomaterials | colonisation 15 % moins chère |
| 1 | drillbots | débloque « excavate » (mines) sur mineral/iceball/gas giant |
| 1 | xenofoods | débloque « farm » (xeno/jungle), améliore le rendement du Food processor |
| 2 | slipgates | débloque la structure Slipgate |
| 2 | geoharvesting | débloque « harvest » : détruire une planète vide contre 20 crédits |
| 2 | trade league | débloque la Trading Hub |
| 2 | replication | débloque « mechanize » : transformer une planète en forgeworld |
| 2 | autoassemblers | colonisation prend 2 mois au lieu de 3 |
| 3 | skill implants | +15 % de revenu commercial |
| 3 | starbirth | débloque « collapse » : transformer gas giant/mineral en étoile (énergie) |
| 3 | biome hacking | débloque « terraform » : transformer en earth-like |
| 3 | infraspace | débloque l'Infragate (routes qui traversent les slipways existantes) |
| 4 | superhuman ai | améliore le rendement du Laboratory |
| 4 | void synthesis | débloque le Synthesizer |
| 4 | gene rewriting | +rendement colonies « colony », débloque « uplift » |
| 4 | time compression | ajoute 3 mois à chaque année simulée |
| 4 | ascension | débloque l'Ascension Gate |

## 13. Score & fin de partie

À tout moment, le score courant (`ky`, ligne 1339) se décompose en :

- **Colonies** : `Σ points_par_niveau` avec `{unhappy:0, content:100, prosperous:200, rich:400}`.
- **Population** : `population_totale × 40`.
- **Technologie** : `floor((nb_techs_inventées × 1.2)²) × 10`.
- **Total** : `floor((colonies + population + techno) × 0.01 × bonheur%)`.
- **Étoiles** (0 à 5) : `floor(total / 2000)`.

La partie se termine à l'année 3426 (« final score ») ou en banqueroute (§2). L'écran de fin recalcule ce score une dernière fois et l'affiche avec le détail par catégorie (`pj` classe, lignes ~815-899).

## 14. Difficultés

4 niveaux (confirmé, table `bo`, ligne 1189) :

| Difficulté | Multiplicateur revenu commercial (`trade`) | Biais de tirage des planètes (`en`) |
|---|---|---|
| forgiving | 7 | -0.99 (fort biais vers les bonnes planètes) |
| reasonable | 6 | -0.6 |
| challenging | 5 | -0.4 |
| tough | 4 | -0.2 (tirage quasi uniforme) |

Le coût de construction des slipways et les seuils de portée/technologie restent identiques quelle que soit la difficulté ; seuls le revenu commercial et la qualité des planètes découvertes varient.

## 15. Interface & contrôles (version PICO-8) — points d'attention pour le portage PSX

La version PICO-8 est **pilotée à la souris**, ce qui ne se transpose pas directement sur la manette PSX (pas de pointeur, layout D-pad/✕/○/□/△/L1/L2/R1/R2/Start/Select) :

- Survol d'une planète/bouton = mise en évidence (`ui:gw`, ligne 226).
- Clic gauche maintenu + glisser depuis une planète colonisée jusqu'à une autre = tracer une nouvelle slipway (`ui:bh`, `jo:ldown`).
- Clic droit maintenu + glisser dans le vide = déplacer la caméra (`bg:rdown`/`bg:hq`).
- Clic sur une planète inconnue = envoyer une sonde ; clic sur une planète connue mais non colonisée = ouvrir le choix de colonisation (`jo:fe`, liste des jobs compatibles).
- Il existe malgré tout un vestige de contrôle directionnel : `qb:ph` (ligne 1134) déplace un curseur avec les 4 boutons directionnels PICO-8 (inertie simple, vitesse ×3, freinage ×0.2) — c'est probablement un fallback clavier/manette resté dans le code ; le plan retient toutefois un mode souris simple pour la v1 (curseur en pixels + clic simulé, voir `docs/plan.md`) plutôt que ce fallback à inertie.
- Pas de scroll de zoom : la caméra suit simplement le curseur (`qb:md`).

Règles de blocage (portée, croisement, planète sur le trajet) à garder telles quelles quel que soit le mapping manette retenu — elles ne dépendent pas de la souris. Idées de mapping alternatif au mode souris v1 : voir `docs/after-v1.md`.

---

## Annexe A — tables brutes décodées

Pour reproductibilité (si le code source est mis à jour), extraites de `map.json`, base d'adresse `4096` (= adresse PICO-8 `0x1000`) :

```
r  (unstash 8192) : f=28,o=21,b=18,p=19,g=48,t=22,l=20,w=12,k=24,?=36,a=33,e=54,h=25
iz (unstash 8259) : 28="food",21="ore",18="bots",19="people",48="goods",22="tech",
                     20="organics",12="luxury",24="science",54="energy",25="joy"
hc (unstash 8383) : f-=o(0,1,2,3,mc="f+"), f+=o(0,2,3,4), -=o(1,2,3,mc="+"), +=o(2,3,4),
                     qh=o(0,1,3,5,mc="qh+"), qh+=o(0,2,5,8)
```

`fd` (unstash 8494), `dr` (unstash 9807, descriptions des structures) et `nb` (unstash 10457, arbre technologique) : voir le détail déjà digéré aux §6-9 et §12 ; contenu brut disponible en relançant la commande de décodage suivante (jq, sans dépendance externe) depuis la racine du projet :

```
jq -r 'def gt: "abcdefghijklmnopqrstuvwxyz0123456789 ().,=-+_/\"'"'"'?%\n";
def unstash(addr): .cells[(addr-4096):(addr-4096+3500)] | .[0:index(0)]
  | map(gt[(.-1):.]) | join("");
unstash(8494)' map.json
```
(remplacer `8494` par `9807` ou `10457` pour les deux autres tables.)

---

*Document basé sur une analyse statique complète du code source (`lua.lua` + données cachées dans `map.json`), complétée par le pitch et les instructions officielles de la page itch.io (https://krajzeg.itch.io/slipways) et une interview du développeur sur le forum PICO-8 (lexaloffle, https://www.lexaloffle.com/bbs/?tid=30978). Le jeu n'a pas de scénario/lore à proprement parler au-delà de ce pitch.*
