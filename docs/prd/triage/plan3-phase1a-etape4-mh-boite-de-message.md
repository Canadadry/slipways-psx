---
title: "Plan 3, étape 1a.4 : `mh` (boîte de message) devient une struct, construite à partir du défaut muté `pj.ea` d'un `pj` qui reste une instance"
description: "La boîte de message `mh` est encore une instance de la couche objet : elle passe en struct fille de `ke`, avec son global `kc` typé, sans toucher à `pj` dont le défaut muté `pj.ea` lui sert de table de construction."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Étapes de la phase 1a ») :

> 1a.4 — `mh` (`msgbox.c:146`), construite avec le défaut muté `pj.ea` (`score_panel_display.c:115`). Tests : aucun.

Règles du §5 qui s'appliquent à ce commit :

> Chaque classe devient un `struct` dont le premier membre est la struct de sa classe parente ; chaque champ de sa chaîne (défauts et champs écrits sur instance) devient un membre de type `pico8_value_t`, **nom minifié conservé**. Nil reste `pico8_value_ref(NULL)`.
>
> **Méthodes** : une table `static const` de pointeurs de fonction par classe ; une surcharge remplace l'entrée héritée. Les méthodes appelées par nom de chaîne […] passent par un enum de méthode qui indexe cette table.
>
> **Défauts mutés sur la classe** : […] deviennent un état statique partagé de la classe, jamais un champ d'instance. Même traitement pour tout autre défaut réaffecté après l'initialisation **ou dont la table pointée est modifiée en place** (connus : `pj.ou`, `pj.ea`, `pj.cr`, `ga.by`) ; la liste des étapes les nomme.
>
> **Coexistence** : […] `hv`/`cv`/`cg`, `dc`, `mv` traitent les deux formes pendant la migration ; l'ordre d'enregistrement est préservé.
>
> **Héritage entre formes et classes-valeurs** : une classe convertie garde sa `pico8_class_t` enregistrée (registres `fz` et nommé compris) jusqu'à la dernière étape de 1a […]. `pico8_class_instantiate` délègue à ce constructeur quand il est posé.
>
> **Accès directs aux défauts** : tout site hors `pico8_class.c` qui lit ou écrit `<classe>->defaults` […] migre dans le commit de cette classe.

§6 : un test de régression est réécrit pour lire la struct dans le commit qui convertit la classe qu'il touche. §7 : chaque commit de 1a passe `make class-coverage-net`, sortie identique à `tools/class_coverage_net_expected.txt`.

### Ce que 1a.1 et 1a.2 ont posé (commits 9ce8bfa, 42fdf50), réutilisé tel quel

- `PICO8_TYPE_TAG_STRUCT` ; en-tête commun (tag, pointeur vers le descripteur `static const` de la classe).
- Descripteur de classe struct : tableau de méthodes indexé par l'enum (`OL`, `PH`, `KJ` aujourd'hui) + lecteur de champ par nom ; table de noms parallèle (`pico8_method_of`). Un nom absent de l'enum est cherché comme champ.
- `pico8_ke_t` (en-tête, `me`, `t`), son initialisation, son lecteur de champ, son `pq` struct (`lt`), appelé une fois après le `pq` de la fille.
- `pico8_class_t.struct_new` : `pico8_class_instantiate` délègue avant toute allocation, tout `pq` et tout comptage ; c'est le constructeur qui enregistre la couverture.
- `pico8_entity_get_field`, lecteur à double forme utilisé par `lt`/`he`, `dc`, `mv`, `dg` et le scan `fx` de `mz:pq`. `dg` écrit `t` directement dans `pico8_ke_t` pour une struct. `pico8_instance_get_field` asserte ne jamais recevoir de struct.
- Modèle de test : `test_pico8_iv` (caractérisation figée sur l'ancien code, mutations notées dans le commit).

### Emprise réelle de `mh` (mesurée sur 02a402d)

**Cart** (`pico8/lua.lua:580-596`) :

```
mh=ke:jr([[ nu=o("mh"), fa=14, rh=5, ]])
function mh:pq() if (kc)kc.p_=true  kc=self end
function mh:ev() local t=self.t/self.rh-10
  if t>=0 and mv(self,"lh") then self.t,t=self.rh*10,0 end
  local nc=-t^3/9  bl(self,nc) camera()
  if t>10.5 then self.p_,kc=true end end
```

L'appel de classe fait `setmetatable(mw(ob), …)` (`lua.lua:88`) : **copie superficielle** de la table reçue. Les sous-tables positionnelles restent partagées avec l'appelant ; la table externe, elle, n'est pas gardée.

| Aspect | Mesure |
|---|---|
| Constructions | 2, toutes dans `score_panel_display.c` (module de `pj`) : `pj:hk` (`:115`, table = le défaut muté `pj.ea` lui-même) et `pj:ph` (`:340`, table fraîche bâtie de `bz`, libérée juste après). Valeur gardée : `pj:ph` la range dans le champ `ed` de l'instance `pj` (`:345`). `pj:hk` l'ignore. |
| Défauts | `nu` (table `o("mh")`, partagée par toutes les `mh`, jamais modifiée), `fa`=14, `rh`=5. Hérités de `ke` : `me`="ph", `t`=0. Aucun défaut de `mh` n'est réaffecté ni modifié en place : `mh` n'a pas de défaut muté à elle. |
| Clés reçues | positionnels `[1..n]`, contigus, de longueur variable (2 ou 3 pour `pj.ea`, le nombre de lignes de `bz` pour `pj:ph`) ; `lh` (fonction, les deux sites) ; `rh` (`pj:ph` seulement, = 3). Aucune autre (spec de `pj.ea` lue : positionnels seulement, plus `lh` ajouté par `pj:hk`). |
| Champs écrits sur l'instance | `p_` (par `mh:pq` sur la `mh` précédente, par `mh:ev`), `t` (par `mh:ev`, et par `dg`). |
| Méthodes appelées par nom | `ev` (bm, via `dc("ev")`) ; `lh` via `mv(self,"lh")` — **champ** fonction propre à chaque instance, pas une méthode de classe ; `ph` via `dg` (`me`) — absente, `mv` rend 0. `pq` non appelée par nom. |
| `->defaults` | 1 site (`msgbox.c`, installation de `ev`), plus `pico8_class_set_pq(g_mh_class, …)`. |
| `bm` / `nu` | `ev` / `mh` → `hv`, `cv.ev`, `cg("mh")`. Aucun lecteur de `cg("mh")` dans `src/` ni `tests/`. |
| Global | `kc` (`msgbox.c`) : n'a jamais tenu qu'une `mh` ou NULL. Écrit par `mh:pq`, `mh:ev`, `pico8__init` (`cart.c:160`, NULL) et `pico8_mh_shutdown` ; lu par `pj:ph` (`score_panel_display.c:319`, comparaison d'identité avec `pj.ed`). |
| Écritures `->fields` | 3, toutes dans `msgbox.c` et sur des `mh` : `p_` de la précédente (`:91`), `t` (`:121`), `p_` (`:139`). |
| Consommateurs génériques | `dg` (via `hv`) ; `dc("ev")` (`fa`, `ra` absent) ; scan `fx` de `mz:pq` (`wormhole.c:83`, `fx` absent). **Non atteints** : boucle de survol de `ui` et `jz` (`cv.hd`, `mh` n'a pas `hd`), `economy` (`cv.hk`/`cv.hb`), lecteurs de `cg`. Aucune macro de `pico8_ref_audit.h` ne reçoit une `mh`. |
| Tests qui lisent `->cls` d'une entrée de `hv` sans tester le tag | `score_panel_close` (`fq_alive`, `:53`) — atteint : ouvrir le panneau fait `pj:ph` → `mh` ; `geoharvest_burst` (`live_ga`, `:84`) — y voit déjà des structs `iv` depuis 1a.2. Lire `e->cls` sur une struct lit les octets de `ke.me` ; la comparaison avec un pointeur de classe est toujours fausse, jamais déréférencée. Voir décisions. |
| Tests qui nomment `mh`/`kc` | aucun (hors `KNOWN[]` de `main_class_coverage.c`). |
| Couverture | `mh  117  10/15` ; `pj  19  15/15` (inchangés). |

### Comment `mh` consomme `pj.ea` pendant que `pj` reste une instance

`pj.ea` est un défaut muté **de `pj`** (§5), et `pj` n'est converti qu'en 1a.29. Dans ce commit :

- `pico8_pj_hk` **ne change pas de logique** : il continue d'aller chercher `pj.ea` par le `->defaults` de la classe `pj` (site de `pj`, qui migre en 1a.29), de modifier en place `pj.ea[2][1]`, `pj.ea[3]` et `pj.ea.lh`, puis de passer la table `pj.ea` au constructeur de `mh`.
- Le constructeur de `mh` lit la table une fois et **n'en garde aucun pointeur** : il recopie la valeur de chaque positionnel (des REF vers les sous-tables, qui restent partagées — c'est `mw`), et la valeur de `lh`. Une mutation ultérieure de `pj.ea[2][1]` par un `pj:hk` suivant se voit donc sur une `mh` encore vivante, exactement comme dans le cart ; une mutation des clés de `pj.ea` elle-même (`[3]`, `lh`) ne s'y voit pas.
- Conséquence : quand 1a.29 fera de `pj.ea` un état statique de `pj`, le constructeur de `mh` n'aura pas à bouger — il reçoit toujours une table.
- La persistance de `pj.ea.lh` d'une année à l'autre (une année sans `pp` recopie la fermeture d'une année précédente, avec son compteur `g_pj_hk_lh_t` déjà entamé) est préservée telle quelle.

## Solution

1. **Struct `mh`** : premier membre `pico8_ke_t`, puis `nu`, `fa`, `rh`, `lh`, `p_` (`pico8_value_t`) et un membre liste pour les positionnels `self[1..n]` (éléments `pico8_value_t`, dans l'ordre de la table reçue).
2. **Constructeur** (posé comme `struct_new` de la `pico8_class_t` de `mh`), dans l'ordre :
   1. alloue ;
   2. initialise la partie `ke` avec le descripteur de `mh` ;
   3. pose les défauts : `nu` = REF vers une seule table `o("mh")` du module, `fa`=14, `rh`=5, `lh`/`p_` nil, liste vide ;
   4. recopie les clés reçues : positionnels ajoutés à la liste dans l'ordre de stockage de la table, `rh`, `lh` ; toute autre clé nommée → assertion porteuse de message (arrêt §7) ;
   5. enregistre `"mh"` au crochet de couverture ;
   6. `mh:pq` (marque `p_` sur l'ancien `kc`, pose `kc`), puis `pico8_ke_struct_pq` (`lt`).
3. **Descripteur de `mh`** : méthode `EV` (nouvelle entrée de l'enum, nom `"ev"`) et lecteur de champ qui connaît `nu`, `fa`, `rh`, `lh`, `p_`, et délègue à `pico8_ke_field`. Les positionnels ne sont pas lus par nom.
4. **`kc` typé** : l'accesseur et le setter prennent/rendent une `mh` struct. `mh:pq` et `mh:ev` écrivent `p_`/`t` directement dans les membres. `pj:ph` compare `kc` à la REF de `pj.ed` par identité de pointeur, sans cast vers `pico8_instance_t`.
5. **`pj` inchangé**, à part les types aux deux sites de construction et à la comparaison `kc`/`ed` : aucune logique, aucun `->defaults` de `pj` déplacé.
6. **`pico8_class_t` de `mh`** : même déclaration, même place, même nom de couverture, reçoit le constructeur ; perd `set_pq` et l'installation `->defaults` de `ev`.

## User Stories

1. En tant qu'autopilote de la phase 1a, je veux que `mh` soit la seule classe convertie par ce commit, afin de respecter « une classe par commit » (§7).
2. En tant qu'autopilote, je veux que l'emprise réelle de `mh` (constructions, champs, méthodes, `->defaults`, consommateurs génériques, tests) soit mesurée dans le PRD, afin que chaque site touché soit vérifiable et pas estimé.
3. En tant qu'auteur de 1a.29, je veux que le constructeur de `mh` ne garde aucun pointeur sur la table reçue, afin que la conversion de `pj.ea` en état statique de `pj` ne demande aucun changement dans `mh`.
4. En tant que gardien du filet, je veux que les sous-tables positionnelles reçues restent partagées (REF recopiée, pas de copie profonde), afin qu'un `pj:hk` qui réécrit `pj.ea[2][1]` change le texte d'une `mh` encore vivante comme dans le cart.
5. En tant que gardien du filet, je veux que la valeur `lh` soit recopiée telle quelle, y compris quand c'est la fermeture d'une année précédente restée sur `pj.ea`, afin que la durée d'affichage d'un message d'année soit identique.
6. En tant que mainteneur, je veux que `pj` reste une instance et que ses `->defaults` (`ea`, `gs`, `ou`, `cr`) restent où ils sont, afin que le défaut muté `pj.ea` soit traité par l'étape de `pj` (1a.29) et pas ici.
7. En tant que lecteur du code, je veux que la struct `mh` ait exactement les membres de sa chaîne (`nu`, `fa`, `rh`, `lh`, `p_`) plus la liste des positionnels, noms minifiés conservés, afin que la traçabilité vers `pico8/lua.lua` tienne.
8. En tant que mainteneur, je veux qu'une clé nommée reçue autre que `rh` ou `lh` déclenche une assertion porteuse de message, afin qu'un champ jamais mesuré ne soit pas perdu en silence (champ sous aucune règle, §7).
9. En tant que gardien du filet, je veux que les défauts `fa`=14, `rh`=5 et le contenu de `nu` soient égaux à ce que le parseur de spec produit sur la chaîne de spec de `mh`, afin que la couche de dessin (14) et la durée d'ouverture ne bougent pas.
10. En tant que mainteneur, je veux que `nu` soit une seule table partagée par toutes les `mh`, vivant de `pico8_mh_init` à `pico8_mh_shutdown`, afin que `lt`/`he` lisent le même tag à l'enregistrement et au retrait, comme avec la table de classe.
11. En tant que gardien du filet, je veux que l'ordre de construction reste `mh:pq` puis `lt`, afin que `kc` et le `p_` de la boîte précédente soient posés avant l'enregistrement, comme dans la marche de chaîne.
12. En tant que gardien du filet, je veux que `lt` enregistre une `mh` dans `hv`, `cv.ev` et `cg("mh")` et nulle part ailleurs, dans le même ordre qu'avant, afin que l'ordre de dessin et de `dg` soit inchangé.
13. En tant que gardien du filet, je veux que `he` retire une `mh` des trois mêmes listes, afin qu'une boîte fermée disparaisse comme avant.
14. En tant que gardien du filet, je veux que `dc("ev")` place la `mh` en couche 14 et appelle son `ev` par la table, afin que la boîte glisse et se dessine à l'identique.
15. En tant que gardien du filet, je veux que `mv(self,"lh")` sur une struct trouve `lh` comme champ fonction et l'appelle, afin que « tenir la boîte ouverte tant que `lh` est vrai » marche sans entrée d'enum pour `lh`.
16. En tant qu'auteur des étapes suivantes, je veux que l'enum de méthodes ne gagne que `EV`, afin que chaque ajout reste attaché à une étape et qu'un champ fonction par instance (`lh`) ne soit pas pris pour une méthode de classe.
17. En tant que gardien du filet, je veux que `dg` appelle `me`="ph" sur une `mh` sans rien trouver, incrémente `t` dans la struct `ke` et retire la boîte quand `p_` est vrai, afin que le temps d'animation soit identique.
18. En tant que gardien du filet, je veux que `mh:ev` réécrive `t` à `rh*10` dans la struct quand `lh` est vrai, afin que la boîte reste ouverte exactement le même nombre de ticks.
19. En tant que gardien du filet, je veux que `self.p_,kc=true` reste « `p_` vrai, `kc` à nil », avec son assertion existante, afin que le piège Lua documenté ne soit pas réintroduit.
20. En tant que mainteneur, je veux que `kc` soit typé comme une `mh` struct, afin qu'aucune écriture `->fields` ne puisse plus viser la boîte courante.
21. En tant que gardien du filet, je veux que `pj:ph` compare `kc` et `pj.ed` par identité de pointeur, afin que le panneau des scores ne reconstruise pas sa boîte tant qu'elle est la courante.
22. En tant que gardien du filet, je veux que `pj.ed` garde la REF vers la struct rendue par le constructeur, afin que cette comparaison porte sur le même objet que `kc`.
23. En tant que mainteneur, je veux que la `pico8_class_t` de `mh` reste déclarée à la même place, avec son nom de couverture et le constructeur de struct, afin que `pico8_class_instantiate(pico8_mh_class(), …)`, le registre et `KNOWN[]` ne changent pas, et que l'assertion de `cart.c` sur son `parent` tienne.
24. En tant que gardien de la couverture, je veux que le constructeur enregistre `"mh"` une fois, jamais `"ke"`, afin que `mh 117 10/15` et `ke 0 0/15` restent identiques.
25. En tant que mainteneur, je veux que l'installation `->defaults` de `ev` et le `set_pq` de `mh` disparaissent, remplacés par la table et le constructeur, afin d'appliquer la règle des accès directs aux défauts.
26. En tant que gardien du filet, je veux que le scan `fx` de `mz:pq` ignore une `mh` vivante (pas de `fx`), afin qu'un pli d'espace construit pendant qu'une boîte est ouverte ne plante pas.
27. En tant que mainteneur de la mémoire, je veux que la struct `mh` et sa liste de positionnels ne soient jamais libérées, comme l'instance avant, afin que TROU-7 reste la seule convention de fuite.
28. En tant que gardien des tests de régression, je veux que `score_panel_close`, `game_end` et `geoharvest_burst` passent sans que leur intention change, afin que la conversion ne se valide pas en affaiblissant un test.
29. En tant que gardien du filet, je veux qu'aucun `.sha256`, aucune entrée de `FRAMES_EVENTS`/`RECORDINGS` ni aucune ligne des références de couverture ne bouge, afin que l'étape ne se valide pas en déplaçant sa cible.
30. En tant que lecteur du code, je veux que les commentaires ajoutés ne portent qu'un contrat C ou un piège (CLAUDE.md), et que ceux que ce changement rend faux (« `kc` est un emprunt dans une entité », « positionnels deviennent `self[1..n]` de l'instance ») soient corrigés, afin de ne pas laisser de contrat mensonger.

## Implementation Decisions

Pas d'outil d'agent dans cette session pour lancer un agent `guideline` frais : j'ai chargé et appliqué moi-même le skill `guideline` (simplicité, changement chirurgical, hypothèses explicites, critères vérifiables). Les décisions tranchées ainsi sont marquées (G).

- **Struct `mh`** : `{ pico8_ke_t ke ; nu ; fa ; rh ; lh ; p_ ; positionnels }`, déclarée dans le module de la boîte de message à côté de `iv`.
- **(G) Positionnels → un membre liste**, pas N membres. L'ensemble des index est `1..n` contigu mais `n` varie selon l'appelant (2, 3 ou `#bz`). C'est le traitement que le plan prescrit déjà pour les positionnels de `lq` (1a.34), donc une règle existante, pas une clé dynamique non finie. Seul `mh:ev` les lit (via `bl`), en entier et dans l'ordre. Ordre de remplissage : ordre de stockage des entrées à clé index de la table reçue — c'est ce que `pico8_class_instantiate` recopiait puis ce que `ev` relisait, donc le rendu est identique par construction. Si le vérificateur juge que ce n'est couvert par aucune règle du §5, c'est un arrêt, pas une improvisation.
- **(G) `lh` est un champ, pas une méthode.** C'est une fermeture propre à chaque boîte, fournie par l'appelant. Le lecteur de `mh` la rend ; `mv` l'appelle parce que la valeur est de genre fonction. L'enum ne gagne que `EV`.
- **`nu`** : REF vers une table `o("mh")` construite une fois dans `pico8_mh_init` (par le parseur de spec, pour qu'elle ait la même forme que celle de la classe) et libérée dans `pico8_mh_shutdown`. `lt` et `he` la lisent comme aujourd'hui (table de positionnels chaîne). Sa durée de vie est celle qu'avait la table de défauts de la classe.
- **`pico8_class_t` de `mh`** : garde sa chaîne de spec (copie en lecture, `mh` n'a pas de descendant), son nom de couverture, reçoit `struct_new`. Perd `set_pq` (le `pq` prend une instance et n'est plus jamais atteint, la délégation passe avant la marche) et l'installation `->defaults` de `ev`. Grep attendu : plus aucun `g_mh_class->defaults`.
- **Constructeur** : ordre de la Solution §2. Signature publique : rend la struct ; les deux appelants de `score_panel_display.c` changent seulement le type de la variable (et `pj:ph` range toujours `pico8_value_ref(made)` dans `pj.ed`).
- **`mh:pq`, `mh:ev`** : prennent la struct. `pq` écrit `p_` dans la `mh` précédente par le membre (plus d'écriture `->fields`). `ev` lit `t` (membre de `ke`), `rh`, appelle `lh` par `pico8_mv` (double forme déjà en place), réécrit `t` dans `pico8_ke_t`, passe la liste des positionnels à `bl` sans la reconstruire (plus de liste temporaire à libérer), pose `p_` et remet `kc` à NULL. Les lecteurs de champ locaux de `msgbox.c` qui ne servent plus qu'à `iv`/`pz` restent tels quels.
- **`kc`** : accesseur et setter typés sur la struct `mh`. Contrat de l'en-tête corrigé : « la boîte ouverte, ou NULL ; emprunt d'une struct enregistrée par `lt`, jamais libérée ici ». `cart.c` (remise à NULL) inchangé à part ce que le type impose.
- **`pj:ph`** : la comparaison devient une égalité de pointeurs entre `kc` et `ed_val.as.ref`, sans cast vers `pico8_instance_t`. Rien d'autre dans `pj` ne bouge. Le commentaire existant sur `ed` (défaut 1, jamais REF) reste vrai.
- **`pico8_pj_hk` et `pj.ea`** : aucun changement de logique. Le site `->defaults` de `pj` (`pj_class_field_ref_table`) appartient à `pj` et migre en 1a.29 ; le déplacer ici serait convertir une autre classe (arrêt).
- **Consommateurs génériques** : aucun changement de noyau attendu (`lt`/`he`/`dc`/`mv`/`dg`/scan `fx` lisent déjà les deux formes). Si un site oublié lit une `mh` par la couche, l'assertion de `pico8_instance_get_field` le signale au premier rejeu : il est traité par la règle de coexistence et nommé dans le message de commit (« découvert à 1a.4 »).
- **(G) Tests qui lisent `e->cls` sur `hv` sans tester le tag** (`score_panel_close:fq_alive`, `geoharvest_burst:live_ga`) : **laissés tels quels**. La lecture est une comparaison toujours fausse, jamais déréférencée, et `geoharvest_burst` y voit déjà des `iv` depuis 1a.2 sans effet. Leur réécriture « lire la struct » appartient aux étapes qui les listent (`score_panel_close` en 1a.5/1a.29, `geoharvest_burst` en 1a.26), avec la preuve rouge du §6. Y toucher ici ferait une réécriture sans objet de conversion ni preuve rouge possible. Signalé dans le message de commit.
- **`pico8_ref_audit.h`, `KNOWN[]`, fichiers de référence de couverture** : inchangés.
- **Commentaires** (CLAUDE.md) : seulement le contrat de la struct (positionnels = `self[1..n]` du cart, sous-tables partagées avec l'appelant, rien n'est libéré), le contrat de `kc`, l'assertion des clés reçues. Corriger le commentaire de `pico8_mh_new` (« ses positionnels deviennent ceux de l'instance ») et celui de `kc`. Le reste va dans le message de commit.

## Implementation Chunks

**Un seul commit** (§7 : une classe par commit). Chaque morceau est un point d'arrêt TDD ; chacun laisse `make test` et `make frames-check` verts. `make class-coverage-net` est lancé au moins sur le code final (~4 min).

1. **Caractérisation de `mh` sur l'ancien code.** Nouveau test (voir Testing) : scénarios réels par `pico8_pj_hk` et `pico8_pj_ph`, relevé par tick, empreintes figées **avant** toute modification de `msgbox.c`.
2. **Enum `EV`.** Ajout de l'entrée et de son nom ; aucune classe ne la porte encore : filet et couverture trivialement identiques.
3. **`mh` convertie.** Struct, table `nu`, descripteur, constructeur, `pq`/`ev` sur la struct, `kc` typé, types aux deux sites de `pj`, classe reliée, `->defaults`/`set_pq` retirés. Le test du morceau 1 passe **sans changer ses valeurs figées**.

## Testing Decisions

- **Un bon test observe le comportement** : appartenance à `hv`/`cv.ev`/`cg("mh")` et son ordre, identité de `kc`, champs lus par `pico8_entity_get_field`, ce que rend `mv`, pixels dessinés par `dc("ev")`, compte de couverture. Jamais l'agencement mémoire au-delà de « en-tête à l'offset 0 ».
- **Nouveau test `test_pico8_mh`** (dans `make unit`, lié comme `test_pico8_iv`, sur le cart réel chargé) :
  - **Caractérisation, figée sur l'ancien code** — pour chaque scénario, à chaque tick `dg` puis `dc("ev")` sur un framebuffer neuf jusqu'au retrait : `t`, `p_`, `kc == la boîte`, présence dans `hv`/`cv.ev`/`cg("mh")`, empreinte d'écran ; le tout replié en une empreinte de séquence + nombre de ticks, figés :
    1. `pj:hk` sur une année avec message (`pp` non nil, `lh` à 90) ;
    2. puis `pj:hk` sur une année sans message : `[3]` retiré de `pj.ea`, `lh` d'avant recopié (compteur entamé) ;
    3. deux `pj:hk` successifs : le second met `p_` sur le premier (`kc`), et le texte `pj.ea[2][1]` réécrit se voit sur la première boîte tant qu'elle est dessinée (sous-table partagée) ;
    4. `pj:ph` sur le `pj` réel avec `c_` vrai : boîte de `rh`=3 tenue ouverte tant que `c_`, puis fermée quand `c_` passe à faux ; un second `pj:ph` pendant que `ed == kc` ne construit rien (compte de `cg("mh")` inchangé).
  - **Défauts** : `fa`, `rh` égaux aux valeurs parsées de la chaîne de spec de `mh` ; `nu` a une seule entrée, la chaîne `"mh"`.
  - **Forme** : le constructeur et `pico8_class_instantiate(pico8_mh_class(), …)` rendent un objet tagué `STRUCT`, présent dans `hv`, `cv.ev`, `cg("mh")`, absent de `cv.kj`/`hd`/`hk`/`hb` ; `mv(mh,"lh")` appelle la fonction reçue ; `mv(mh,"ph")` rend 0.
  - **Mutations** : l'implémenteur remet localement au moins 5 erreurs plausibles et note dans le commit celles qui sont attrapées (et pourquoi une serait équivalente) : `p_` de l'ancien `kc` non posé ; `t` non réécrit à `rh*10` ; `rh` à 4 ; `lh` non recopié ; positionnels copiés en profondeur (scénario 3) ; `nu` absent ; `EV` retiré de la table.
- **Tests de régression** : aucun nommé par l'étape. `score_panel_close` et `game_end` (panneau des scores, donc `pj:ph` → `mh`) et `geoharvest_burst` sont exécutés par `make unit` et doivent passer **avec un diff vide**.
- **Couche d'entités** : pas de nouveau test dans `test_pico8_entity_smoke` — `mh` n'ajoute aucune capacité au noyau, et le champ fonction appelé par `mv` est couvert par `test_pico8_mh`.
- **Couverture** : `make class-coverage-net` code 0, sortie identique à la référence (`mh  117  10/15`, `pj  19  15/15`, `ke  0  0/15  exemptée …`).
- **Prior art** : `test_pico8_iv` (caractérisation figée, `dg`/`dc` réels, empreintes FNV) ; `test_pico8_jb_ol` ; `test_pico8_score_panel_close` (pilotage de `ju.c_`).

## Critères du vérificateur

Chacun se traduit en `FAILED:` par le vérificateur de `prd-autopilot` (§7 ; seul arrêt que `plan-pilot` honore).

- `FAILED:` si `make test` ou `make frames-check` est encore rouge après **3 tentatives** de correction sur ce commit.
- `FAILED:` si le diff modifie ou supprime un `*.sha256` du filet, ou retire une entrée de `FRAMES_EVENTS` ou de `RECORDINGS`.
- `FAILED:` si `make class-coverage-net`, relancé sur le code final, ne rend pas 0 ou si sa sortie n'est pas identique à `tools/class_coverage_net_expected.txt` — en particulier si `mh` n'est pas `mh  117  10/15`.
- `FAILED:` si `tools/class_coverage_net_expected.txt`, `tools/class_coverage_exempt.txt` ou `KNOWN[]` est modifié.
- `FAILED:` si un champ ou une clé de `mh` ne tombe sous aucune règle du §5 — notamment si l'assertion des clés reçues (nommées hors `rh`, `lh`) se déclenche sur un rejeu ou un test : arrêt, pas de membre improvisé.
- `FAILED:` si un test de régression de bug devrait être modifié dans son intention pour passer, ou si `tests/test_pico8_score_panel_close.c`, `tests/test_pico8_game_end.c` ou `tests/test_pico8_geoharvest_burst.c` a un diff non vide.
- `FAILED:` si une autre classe que `mh` est convertie — en particulier si `pj` reçoit un constructeur de struct, si un `->defaults` de `pj` est déplacé ou supprimé, ou si `pj.ea`/`pj.gs`/`pj.ou`/`pj.cr` changent de représentation.
- `FAILED:` si la `pico8_class_t` de `mh` n'est plus déclarée ou enregistrée à la même place, ou si `ke`/`iv`/`jb` changent de forme.
- `FAILED:` si les valeurs figées du test de caractérisation de `mh` ont été changées après la conversion (elles doivent être celles relevées sur l'ancien code).

Critères de réussite (en plus) :

- grep : plus aucun `g_mh_class->defaults`, plus de `set_pq` sur `g_mh_class`, plus aucune écriture `->fields` dans `pico8_mh_pq`/`pico8_mh_ev` ;
- la struct `mh` n'a que `pico8_ke_t` et les 6 membres listés ;
- l'enum de méthodes vaut exactement `OL`, `PH`, `KJ`, `EV` ;
- `kc` n'est plus typé `pico8_instance_t *` ;
- dans `score_panel_display.c`, le diff se limite aux types des deux constructions et à la comparaison `kc`/`ed`.

## Out of Scope

- `pj` et ses défauts mutés `ea`/`gs`/`ou`/`cr` (1a.29), l'enveloppe de `pz()` sur `pj.cr` (1a.30).
- Toute autre feuille de `ke` ; la suite de la liste (1a.5 `fq`…).
- La réécriture des helpers de test qui lisent `e->cls` sur `hv` (1a.5/1a.26/1a.29).
- Une écriture générique de champ par nom sur une struct ; toute extension du noyau que `mh` n'exige pas.
- Types C réels (1b) ; libération des entités (TROU-7, Plan 2 §8) ; rapport de `make ref-audit`.
- Lancer l'appli raylib : jamais.

## Further Notes

- `mh` n'a **pas** de défaut muté à elle : le « défaut muté » de la ligne d'étape est celui de `pj`, que `mh` reçoit en table de construction. Le §5 s'applique à `pj.ea` en 1a.29 ; ce commit garantit seulement que `mh` n'en dépend pas au-delà de la lecture unique à la construction.
- Le compte de `mh` (117) inclut les boîtes d'année (`pj:hk`) et celles du panneau des scores (`pj:ph`) : les deux sites passent par le même constructeur.
- Mesures faites par grep sur `src/` et `tests/` à 02a402d, et en lisant `pico8/lua.lua:85-94` (`jr`/`mw`), `:580-596` (`mh`) et `:815-900` (`pj`).
