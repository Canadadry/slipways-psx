---
title: "Plan 3, étape 1a.2 : `ke` devient une struct avec sa feuille `iv`, et le noyau d'entités traite les deux formes"
description: "Aucune struct n'atteint encore `hv`/`cv`/`cg`, `dg`, `dc` ni `mv` : le noyau `ke` et sa feuille de plus petite emprise, `iv`, passent en struct dans un même commit pour que la double forme du noyau soit construite et vue par le filet dès son introduction."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Étapes de la phase 1a ») :

> 1a.2 — `ke` (`ke.c:19`) **avec** la feuille directe de `ke` de plus petite emprise (mesurée ; candidates : `iv`, `mh`, `fq`, `nf`, `qb`, `mz`, `bg`, `jc`, `ie`, `js`), qui sort alors de la suite de la liste. Tests : `built_planet_panel_nil_row`, `empty_planet_cleared` (`e.me`).

Règles du §5 qui s'appliquent à ce commit :

> Chaque classe devient un `struct` dont le premier membre est la struct de sa classe parente ; chaque champ de sa chaîne (défauts et champs écrits sur instance) devient un membre de type `pico8_value_t`, **nom minifié conservé**. Nil reste `pico8_value_ref(NULL)`.
>
> **Méthodes** : une table `static const` de pointeurs de fonction par classe ; une surcharge remplace l'entrée héritée. Les méthodes appelées par nom de chaîne (`mv(held,"hq")`, `jz(rm,…)`, `dc(nt)`) passent par un enum de méthode qui indexe cette table.
>
> **Coexistence** : […] `hv`/`cv`/`cg`, `dc`, `mv` traitent les deux formes pendant la migration ; l'ordre d'enregistrement est préservé.
>
> **Deuxième** [commit] : `ke` **dans le même commit** que sa fille directe sans descendance de plus petite emprise (mesurée par le PRD de l'étape), pour que la struct de `ke` soit construite et vue par le filet dès son introduction.
>
> **Héritage entre formes et classes-valeurs** : une classe convertie garde sa `pico8_class_t` enregistrée (registres `fz` et nommé compris) jusqu'à la dernière étape de 1a […]. Les descendants non convertis trouvent leurs champs hérités en remontant `parent`, qui reste valide.
>
> **Accès directs aux défauts** : tout site hors `pico8_class.c` qui lit ou écrit `<classe>->defaults` migre dans le commit de cette classe […].

§6 : un test de régression est réécrit pour lire la struct dans le commit qui convertit la classe qu'il touche, et prouvé rouge avec le bug remis. §7 : chaque commit de 1a passe `make class-coverage-net`, sortie identique à `tools/class_coverage_net_expected.txt`.

### Ce que 1a.1 a posé (commit 9ce8bfa), réutilisé ici

- `PICO8_TYPE_TAG_STRUCT`, reconnu par `pico8_type_tag_of`.
- L'en-tête commun des entités-structs (tag, puis pointeur vers la table `static const` de la classe) et l'enum de méthodes, qui ne contient que `OL`.
- `pico8_class_t.struct_new` et son setter ; `pico8_class_instantiate` délègue au constructeur de struct **avant** toute allocation, tout `pq` et tout comptage de couverture. C'est le constructeur qui enregistre la couverture.
- `jb` = en-tête + `ls`. Comme `jb` n'est pas une fille de `ke`, `hv`/`cv`/`cg`, `dg`, `dc` et `mv` n'ont pas bougé et assertent toujours `INSTANCE`. Le PRD de 1a.1 avait renvoyé leur double forme à cette étape-ci.

### Emprise mesurée des candidates (sur e5aa9d1)

Mesures faites par grep sur `src/` et `tests/`, et en lisant les définitions du cart. « Consommateurs génériques » désigne le code qui reçoit l'entité sans connaître sa classe, en dehors de `lt`/`he`/`dg`/`dc`/`mv`. Compte = ligne de `tools/class_coverage_net_expected.txt`.

| Feuille | Constructions (C) | Clés reçues à la construction | Méthodes appelées par nom | `->defaults` | `bm` / `nu` | Consommateurs génériques, globaux | Tests | Compte |
|---|---|---|---|---|---|---|---|---|
| **`iv`** | 3 : `jo:hk` (`planet.c`), `hy:lu` ×2 (`economy.c`) ; valeur rendue jamais gardée | `ra`, `[1]`, `[2]` | `ph` (via `me`, `dg`), `kj` (`dc`) ; `pq` | 2 | `kj` / — | scan `fx` de `mz:pq` sur `hv` ; aucun global | aucun | 474, 9/15 |
| `mh` | 2 (`score_panel_display.c`) | positionnels de longueur variable (`bl(self)`) + `lh` issu du défaut muté `pj.ea` | `ev` (`dc`), `lh` (`mv`) ; `pq` réécrit le global `kc` | 1 | `ev` / `mh` | global `kc` (écriture `p_` par `->fields`) | aucun | 117, 10/15 |
| `fq` | 2 (`menu.c`, `score_panel_display.c`) | `my` (fonction) | `ldown` (`jz`), `hd=1` (valeur, pas une fonction), `my` (`mv`) | 1 | `hd` / — | boucle de survol `ui` (`fa` lu, `m_` écrit par `->fields`, `fg` lu), `jz` | `score_panel_close` | 3, 2/15 |
| `nf` | 1 (`je`, `bootstrap.c`) | — | `ph` ; `pq` | 1 | — | global `je` : environ 18 lectures, clé dynamique `je[bt]` | aucun | 38 |
| `qb` | 1 (`mo`, `bootstrap.c`) | — | `ph`, `md` (`dc` via `mv(mo,…)`) | 2 | — | global `mo` : environ 16 lectures, `mo.v` écrit par `bg` | `planet_kj_dispatch` | 38 |
| `mz` | via `bg:ldown` | nombreuses | `hq`, `ev`, `ey`, `kz`, `kj` | 7 | `ev`, `kj` / `mz` | `jj` lu sur la classe, `lookup_named` | aucun | 146 |
| `bg` | 1 | — | `kj`, `ldown`, `rdown`, `hq`, `hd=1` | 5 | `kj`, `hd` / — | `pl` lu sur la classe, `lookup_named("mz"\|"g_")` | `bg_kj` | 19 |
| `jc` | `lookup_named` ×4, `probe.c` | nombreuses | nombreuses | 5 | `nu=jc` → 4 fichiers lisent `cg("jc")` | — | aucun | 152 |
| `ie` | `cart.c` ×7 | — | `hd=gk`, `hb`, `kj`, `ldown` | 9 | `hd`, `hb`, `kj` / — | défauts mutés `gy`/`id`, `ie.ke` | aucun | 133 |
| `js` | nombreuses (`pz()`, …) | nombreuses | `hd`, `ev`, `kj`, `ldown`, `lq` | 5 | `hd`, `ev`, `kj` / — | `jb`, `lq` | 13 tests | 1 286 |

**Choix : `iv`.** C'est la seule candidate qui cumule les cinq propriétés suivantes :

- aucun global ne la tient ;
- aucune de ses valeurs n'est gardée par un appelant ;
- aucun test ne la nomme ;
- aucun défaut muté, aucune clé dynamique ;
- un seul `bm` (`kj`) et aucun `nu`.

De plus, elle traverse tout le noyau : `lt`, `dg` qui lit `me`/`t`/`p_`, écrit `t` et appelle `he` sur `p_`, et `dc("kj")` qui lit `fa`/`ra`. Elle est construite 474 fois dans 9 rejeux sur 15 et **dessine** (toast flottant) : le filet la voit. `fq` est la seule autre candidate avec une emprise d'enregistrement aussi petite. Mais elle passe par la boucle de survol de `ui`, qui écrit dans `->fields`, et par `jz` ; elle porte un `hd` qui n'est pas une fonction ; un test de régression la lit ; et elle n'apparaît que dans 2 rejeux sur 15. `iv` sort donc de la suite de la liste : 1a.3 devient sans objet (voir Further Notes).

### Emprise réelle de `ke` et `iv`

- **Cart** : `ke=kv:jr("me=\"ph\",t=0")`, `ke.pq=lt`, `ke:ks(me)`. `iv=ke:jr("fa=9,l=0,v=v(0,-0.34)")`. `iv:pq` pose `self.ra=self.ra or je.ra+mo.ra-v(0,5)`. `iv:ph` avance `ra` de `v`, `l` de 0,1, et pose `p_` quand `l>=5`. `iv:kj(p)` dessine `self[1]` en couleur `self[2]`, à la palette `l`.
- **Ordre des `pq`** (règle de la couche, de la classe dérivée vers la base, avec dédoublonnage) : `iv:pq`, puis `lt`. L'enregistrement dans `hv` a donc lieu après que `ra` est posé.
- **Champs d'`iv`** : défauts `fa`, `l`, `v` ; champs écrits sur l'instance `ra`, `l`, `p_` ; positionnels `[1]` (chaîne possédée par l'instance, jamais libérée) et `[2]` (fixe). Aucun autre champ n'est reçu ni écrit. `fx`, que lit le scan de `mz:pq`, n'est pas dans la chaîne : il est absent.
- **Champs de `ke`** : `me` (défaut `"ph"`), `t` (défaut 0, incrémenté par `dg`). `ke:ks` n'est appelée que sur `jo` et `g_` (`planet.c:370,431`, `probe.c:292`), qui restent des instances.
- **`->defaults`** : 2 sites, dans `pico8_iv_init` (installation de `ph` et `kj`). Aucun pour `ke`.
- **Lecteurs de `me`/`t` hors noyau** : environ 20 sites (`planet*`, `probe`, `wormhole*`, `economy_tiers`). Tous lisent des instances non converties (`jo`, `g_`, `mz`, `mh`) et trouvent `me`/`t` par la `pico8_class_t` de `ke`, qui reste. Aucun n'est touché.
- **Instanciations directes de `ke`** : aucune dans le jeu (`ke 0 0/15 exemptée`). `test_pico8_entity_smoke` instancie `ke` et une fille déclarée sur `ke` sous forme d'instances : ces tests de la couche restent valides.

## Solution

1. **Struct `ke`** : en-tête, puis `me` et `t` (`pico8_value_t`). Le module `ke` fournit :
   - une initialisation de la partie `ke` d'une struct fille : en-tête avec la table de la fille, `me="ph"`, `t=0` ;
   - un lecteur de champs par nom qui connaît `me`/`t` ;
   - le `pq` de `ke` sous forme struct (appel de `lt`).

   La `pico8_class_t` de `ke` **ne change pas** : mêmes défauts, même `pq=lt`, **pas** de constructeur de struct (voir décisions). Les 20 filles et petites-filles non converties continuent à la remonter.
2. **Struct `iv`** : premier membre la struct `ke`, puis `fa`, `l`, `v`, `ra`, `p_` et deux membres pour les positionnels `[1]`/`[2]`. Son constructeur fait, dans l'ordre :
   1. alloue ;
   2. initialise la partie `ke` ;
   3. pose les défauts d'`iv` ;
   4. recopie les clés reçues (`ra`, `[1]`, `[2]`, rien d'autre : assertion) ;
   5. enregistre `"iv"` au crochet de couverture ;
   6. exécute `iv:pq`, puis le `pq` de `ke` (`lt`).

   La `pico8_class_t` d'`iv` reste déclarée à la même place, avec son nom de couverture, reçoit le constructeur et perd ses deux installations `->defaults`.
3. **Table de méthodes étendue en descripteur de classe struct** : la table `static const` de chaque classe convertie porte, en plus des méthodes indexées par l'enum, le lecteur de champs par nom de la classe. Celui d'`iv` connaît ses membres et délègue à celui de `ke` pour le reste. L'enum gagne `PH` et `KJ`. Une table de noms parallèle permet de passer d'un nom de méthode à son index.
4. **Noyau à double forme**, dans le module d'entités : un seul lecteur de champ par nom qui aiguille sur le tag. `INSTANCE` passe par la lecture de la couche ; `STRUCT` passe par le lecteur de la classe. Il est utilisé par :
   - `lt`/`he` : présence des cinq `bm`, lecture de `nu` ;
   - `dc` : `fa`, `ra` ;
   - `mv` : une méthode de la table si le nom en est une, sinon la valeur du champ telle quelle, sinon absent ;
   - `dg` : `me`, `t`, `p_`. `dg` écrit `t` directement dans la struct `ke` pour la forme struct ;
   - le scan `fx` de `mz:pq`.

   Les asserts `INSTANCE` de `dc` et de `dg` deviennent « `INSTANCE` ou `STRUCT` ». La lecture de champ de la couche asserte qu'on ne lui passe jamais une struct, pour qu'un site oublié soit une assertion et pas une lecture hors bornes.
5. **Tests `built_planet_panel_nil_row` et `empty_planet_cleared` inchangés** : ils ne lisent `me` que sur des planètes (`cg("jo")`), qui restent des instances non converties. La conversion de `jo` (1a.17) est le commit qui les réécrira.

## User Stories

1. En tant qu'autopilote de la phase 1a, je veux que `ke` soit converti dans le même commit qu'une feuille réellement construite, afin que la struct `ke` et la double forme du noyau soient vues par le filet dès leur introduction.
2. En tant qu'autopilote, je veux que la feuille choisie soit celle de plus petite emprise mesurée, avec le tableau de mesure dans le PRD, afin que le choix soit vérifiable et non estimé.
3. En tant qu'autopilote, je veux que la feuille retenue (`iv`) sorte de la suite de la liste des étapes, afin que 1a.3 ne tente pas une seconde conversion.
4. En tant qu'auteur des étapes suivantes, je veux une struct `ke` (en-tête, `me`, `t`) que chaque fille enchâsse en premier membre, afin que l'en-tête reste à l'offset 0 et que `me`/`t` soient au même endroit pour toute struct enregistrée.
5. En tant qu'auteur des étapes suivantes, je veux une seule fonction d'initialisation de la partie `ke`, afin qu'aucune fille convertie n'oublie `me="ph"` ou `t=0`.
6. En tant qu'auteur des étapes suivantes, je veux que le `pq` de `ke` sous forme struct soit un appel explicite, fait par le constructeur de la fille après son propre `pq`, afin de reproduire l'ordre de la couche (de la dérivée vers la base) sans la marche de chaîne.
7. En tant qu'auteur des étapes suivantes, je veux que la règle de dédoublonnage soit écrite en contrat : une fille sans `pq` propre n'appelle que celui de `ke`, et une fille dont le `pq` est hérité de `ke` ne l'appelle qu'une fois. Ainsi `fq`, qui n'a pas de `pq`, sera bien traitée.
8. En tant que mainteneur, je veux que les 20 descendants non convertis de `ke` (arbre `jo`, `g_`→`fv`, `pj`, `ui`, les autres feuilles) s'instancient exactement comme avant, afin que le filet ne bouge pas.
9. En tant que mainteneur, je veux que la `pico8_class_t` de `ke` garde ses défauts `me`/`t` et son `pq=lt`, afin que les instances non converties trouvent `me`/`t` et s'enregistrent comme avant.
10. En tant que mainteneur, je veux que `ke` ne reçoive pas de constructeur de struct, afin qu'une instanciation directe de `ke` (tests de la couche seulement) garde la forme instance et que le compte de `ke` reste 0.
11. En tant que mainteneur, je veux que la `pico8_class_t` d'`iv` reste déclarée à la même place, avec son nom de couverture et un constructeur de struct, afin que le registre et `KNOWN[]` ne changent pas.
12. En tant que gardien de la couverture, je veux que le constructeur d'`iv` enregistre `"iv"` une fois et jamais `"ke"`, afin que les lignes `iv 474 9/15` et `ke 0 0/15` restent identiques.
13. En tant que lecteur du code, je veux que la struct `iv` ait exactement les membres de sa chaîne (`fa`, `l`, `v`, `ra`, `p_`) plus les deux positionnels, noms minifiés conservés, afin que la traçabilité vers `pico8/lua.lua` tienne.
14. En tant que mainteneur, je veux qu'une clé reçue autre que `ra`, `[1]`, `[2]` fasse échouer une assertion porteuse de message, afin qu'un champ jamais mesuré ne soit pas perdu en silence (champ sous aucune règle, §7).
15. En tant que gardien du filet, je veux que les défauts d'`iv` sous forme struct soient identiques au bit près à ceux que le parseur de spec produit (en particulier `v=v(0,-0.34)`), afin que le toast monte exactement comme avant.
16. En tant que mainteneur, je veux que le défaut vecteur `v` soit un seul objet partagé par toutes les `iv`, jamais modifié en place, comme la table de classe du cart, afin de ne pas allouer un vecteur par toast.
17. En tant que gardien du filet, je veux que `lt` enregistre une struct dans `hv` et dans chaque `cv[p]` dont le nom est présent (méthode posée ou champ non nil), et dans `cg` selon son `nu`, afin qu'`iv` tombe dans `hv` et `cv.kj` et nulle part ailleurs.
18. En tant que gardien du filet, je veux que l'ordre d'enregistrement dans `hv` et `cv` soit celui d'avant, afin que l'ordre de dessin par couche et l'ordre de `dg` ne bougent pas.
19. En tant que gardien du filet, je veux que `he` retire une struct des mêmes listes que `lt`, afin qu'un toast arrivé à `p_` disparaisse comme avant.
20. En tant que gardien du filet, je veux que `dg` appelle sur une struct la méthode nommée par `me` avec `t`, relève `p_` et incrémente `t`, afin que les 50 ticks de vie du toast soient identiques.
21. En tant que mainteneur, je veux que TROU-5 (entité sautée quand une entité se retire pendant `dg`) reste exactement tel quel pour une struct, afin que la divergence documentée ne change pas de forme.
22. En tant que gardien du filet, je veux que `dc("kj")` lise `fa` et `ra` d'une struct et l'appelle par sa table, afin que le toast soit dessiné dans la même couche (9), à la même position.
23. En tant que mainteneur, je veux que `mv` sur une struct rende 0 pour un nom qui n'est ni une méthode de sa table ni un de ses champs, afin que le contrat « champ absent → 0 » tienne pour les deux formes.
24. En tant que mainteneur, je veux que `mv` sur une struct rende la valeur d'un champ non fonction telle quelle, afin que les futurs `hd=1` (`fq`, `bg`) trouvent déjà le bon comportement.
25. En tant que gardien du filet, je veux que le scan `fx` de `mz:pq` lise une struct par le lecteur à double forme, afin qu'un toast vivant au moment où un pli d'espace est créé ne fasse pas planter et ne soit pas retenu comme ancre.
26. En tant que mainteneur, je veux que la lecture de champ de la couche asserte qu'elle ne reçoit jamais une struct, afin que tout site oublié qui lit une entité générique soit une assertion au premier rejeu.
27. En tant que mainteneur, je veux que `dc` et `dg` assertent « `INSTANCE` ou `STRUCT` » et plus seulement `INSTANCE`, afin que la garde contre une table ou une liste reste en place.
28. En tant qu'auteur des étapes suivantes, je veux que l'enum de méthodes ne gagne que `PH` et `KJ` (noms réellement appelés sur `iv`), afin que chaque ajout reste attaché à une étape.
29. En tant que lecteur du code, je veux que `iv:ph` et `iv:kj` prennent la struct `iv` et soient appelés par la table, afin que les deux installations `->defaults` disparaissent (règle des accès directs aux défauts).
30. En tant que mainteneur, je veux que les trois sites de construction d'`iv` n'aient pas à changer au-delà du type rendu, qu'ils ignorent, afin que le diff reste concentré sur le noyau et les modules `ke`/`iv`.
31. En tant que mainteneur de la mémoire, je veux que la struct `iv`, ses vecteurs `ra` et sa chaîne `[1]` ne soient jamais libérés, comme avant, afin que TROU-7 reste la seule convention de fuite.
32. En tant que gardien des tests de régression, je veux que `built_planet_panel_nil_row` et `empty_planet_cleared` restent intacts et verts, puisqu'ils ne lisent aucune struct, afin que leur réécriture arrive avec `jo` (1a.17), qui est la classe qu'ils touchent vraiment.
33. En tant que gardien du filet, je veux qu'aucun `.sha256`, aucune entrée de `FRAMES_EVENTS`/`RECORDINGS` et aucune ligne des références de couverture ne bouge, afin que l'étape ne se valide pas en déplaçant sa cible.
34. En tant que lecteur du code, je veux que les commentaires ajoutés ne portent qu'un contrat C ou un piège (CLAUDE.md), et que ceux que ce changement rend faux (« assert INSTANCE », « `.as.ref` est un `pico8_instance_t*` ») soient corrigés, afin de ne pas laisser de contrat mensonger.

## Implementation Decisions

Je n'avais pas d'outil d'agent dans cette session, donc pas d'agent `guideline` frais pour les points où `to-prd` demande de consulter l'utilisateur. J'ai chargé et appliqué moi-même le skill `guideline` : simplicité, changement chirurgical, hypothèses explicites, critères vérifiables. Les décisions tranchées ainsi sont marquées (G).

- **(G) Feuille : `iv`**, d'après le tableau ci-dessus. Rejetée : `fq`. Son emprise d'enregistrement est comparable, mais elle ajoute la boucle de survol (écriture `m_` dans `->fields`), `jz`, un `hd` qui n'est pas une fonction, un test de régression, et le filet ne la voit que dans 2 rejeux.
- **Struct `ke`** : `{ en-tête ; me ; t }`, dans le module `ke`. Contrat à écrire à la déclaration : premier membre de toute struct fille de `ke` ; seules des structs qui la contiennent sont jamais passées à `lt`, parce que seul le `pq` de `ke` appelle `lt` hors tests.
- **(G) Pas de constructeur de struct pour `ke`.** `ke` est abstraite (jamais instanciée dans le jeu, exemptée). Un constructeur seul serait du code sans chemin d'exécution, et changerait la forme de ce que construisent les tests de la couche qui l'instancient directement. La struct `ke` est « construite » chaque fois qu'une `iv` l'est, ce qui est ce que demande le §5.
- **(G) Pas de `ks` sous forme struct.** Aucun appelant ne passe une struct (mesuré). `pico8_ke_ks` garde sa signature d'instance.
- **(G) Descripteur de classe struct.** Le pointeur de l'en-tête vise désormais une struct `static const` par classe : le tableau de méthodes indexé par l'enum, plus un lecteur de champ par nom (`self`, nom → présent/absent + valeur). La table de `jb` est adaptée, avec un lecteur NULL : `jb` n'atteint jamais le lecteur générique, qui asserte le lecteur non NULL.
  - Rejeté : lecteur dans l'en-tête, qui ferait un pointeur par instance.
  - Rejeté : membres « génériques » `p_`/`fa`/`ra`/`nu` remontés dans `ke`. Ce sont des champs que la chaîne de `ke` n'a pas, ce qui violerait la règle des membres.
  - Rejeté : lecture par offset connu. Chaque feuille a d'autres membres.

  Justification : le noyau (`dg`, `dc`, `lt`/`he`, scan de `mz`) lit par nom des champs de classes qu'il ne connaît pas. C'est exactement `e[k]` dans le cart. Le lecteur par classe est la traduction littérale de la remontée de chaîne (le lecteur d'`iv` délègue à celui de `ke`). Il se retire en 1b, quand les membres seront typés.
- **Enum de méthodes** : `OL`, `PH`, `KJ`, et une table de noms parallèle pour passer du nom à l'index. Un nom absent de l'enum n'est la méthode d'aucune struct : il est cherché comme champ.
- **Lecteur de champ à double forme dans le module d'entités** (utilisé partout où le noyau lisait un champ par la couche) : `INSTANCE` → lecture de la couche, sémantique de présence inchangée ; `STRUCT` → lecteur du descripteur. Pour un membre `pico8_value_t`, « absent » = `pico8_value_ref(NULL)` : c'est la sémantique Lua (nil = absent), et la seule qui existe pour une struct.
- **Présence d'un `bm` sur une struct** (`lt`/`he`) : entrée de méthode non NULL pour ce nom, ou champ présent. `iv` → `kj` seulement.
- **`nu` sur une struct** : lu par le lecteur. `iv` n'en a pas, donc aucune entrée `cg`. Les futures feuilles à `nu` (`mh`, `mz`, `jc`) s'enregistrent sans toucher au noyau.
- **`mv` sur une struct** : méthode de la table si le nom en est une (appel avec `self` = la struct), sinon valeur du champ si présent, sinon 0. Signature de `pico8_mv` inchangée : le pointeur est lu d'après son tag, contrat à mettre à jour dans l'en-tête. Idem `pico8_lt`/`pico8_he`.
- **`dg`** : lit `me`, `t`, `p_` par le lecteur à double forme. L'écriture de `t` se fait sur la struct `ke` pour la forme struct, et reste une écriture d'instance sinon. Le parcours par index (TROU-5) est inchangé.
- **`dc`** : `fa`/`ra` par le lecteur. L'assert devient « `INSTANCE` ou `STRUCT` ». `mo:md` inchangé (`mo` est un `qb`, non converti).
- **Scan `fx` de `mz:pq`** : lecteur à double forme. C'est un site couvert par la règle de coexistence (consommateur de `hv`), pas un arrêt. S'il en apparaît d'autres en cours d'étape (même règle), ils sont traités de même et nommés dans le message de commit (« découvert à 1a.2 »).
- **(G) Garde dans la lecture de champ de la couche** : assert « jamais une struct » (tag différent de `STRUCT`), avec message. Assert ciblé sur le nouveau risque, et pas « doit être `INSTANCE` », pour ne pas transformer un défaut ancien sans rapport en arrêt de cette étape.
- **Struct `iv`** : `{ struct ke ; fa ; l ; v ; ra ; p_ ; deux membres pour [1]/[2] }`. Le nom des deux positionnels est laissé à l'implémentation, avec une courte mention à la déclaration qu'ils sont `self[1]`/`self[2]` du cart (contrat que le nom ne dit pas).
  - (G) Deux membres plutôt qu'un membre liste : l'ensemble des index est fini et fixe (mesuré sur les 3 sites), et seul `kj` les lit, par index constant.
- **Défauts d'`iv`** : `fa` = 9, `l` = 0, `v` = REF vers un seul vecteur statique du module. La valeur fixe de -0,34 doit être celle que produit le parseur de spec : vérifiée par un test (voir Testing), pas supposée. La `pico8_class_t` d'`iv` garde sa chaîne de spec (copie en lecture, sans descendant) et perd ses deux installations `->defaults`, remplacées par la table (`PH`, `KJ`).
- **Constructeur d'`iv`** : ordre fixé en Solution §2. `ra` reçu vaut tel quel ; `iv:pq` le remplace s'il est nil. L'assertion « clés ⊂ {`ra`, 1, 2} » porte un message qui nomme l'arrêt §7. `pico8_iv_new` rend la struct (les trois appelants ignorent la valeur) ; `pico8_iv_pq`/`_ph`/`_kj` prennent la struct.
- **Couverture** : `"iv"` enregistré dans le constructeur, sous `PICO8_CLASS_COVERAGE`. `KNOWN[]`, `tools/class_coverage_net_expected.txt` et `tools/class_coverage_exempt.txt` sont inchangés.
- **`pico8_ref_audit.h` inchangé** : aucun cast vers liste ne reçoit une `iv`.
- **Commentaires** (CLAUDE.md) : on ajoute seulement :
  - le contrat de la struct `ke` (premier membre, seule forme passée à `lt`) ;
  - le contrat du lecteur de champ (valeur rendue si le champ manque) ;
  - les deux asserts porteurs ;
  - la mention des positionnels.

  On corrige, sans les compléter, les commentaires d'en-tête des entités qui disent « `.as.ref` est un `pico8_instance_t*` » et ceux de `ke.h` sur la marche de `pq`. Tout le reste va dans le message de commit, dont la ligne « `iv` sort de la liste : 1a.3 sans objet ».

## Implementation Chunks

**Un seul commit** (§5 : `ke` dans le même commit que sa feuille ; §7 : un commit par classe). Chaque morceau ci-dessous est un point d'arrêt TDD dans ce commit. Chacun laisse `make test` et `make frames-check` au vert. `make class-coverage-net` est lancé au moins sur le code final.

1. **Caractérisation d'`iv` sur l'ancien code.** Nouveau test unitaire : construire une `iv` réelle (spec positionnelle comme `hy:lu`, et avec `ra` comme `jo:hk`), la faire vivre par `dg` jusqu'à `p_`, et relever à chaque tick `ra`, `l`, `t`, sa présence dans `hv`/`cv.kj`, plus l'empreinte du dessin `dc("kj")` sur un framebuffer neuf. Les empreintes et la séquence sont relevées sur l'ancien code, puis figées.
2. **Machinerie, sans classe convertie.** Descripteur (table + lecteur) avec `jb` adapté, lecteur NULL. Enum `PH`/`KJ` et table de noms. Lecteur à double forme dans le module d'entités, branché dans `lt`/`he`/`dc`/`mv`/`dg` et dans le scan de `mz`. Asserts élargis et garde de la couche. Tests de la couche (voir Testing) avec une struct factice fille de `ke`. Aucune classe du jeu ne change de forme : filet et couverture trivialement identiques.
3. **`ke` et `iv` convertis.** Struct `ke` et son initialisation ; struct `iv`, constructeur, table, classe reliée ; installations `->defaults` retirées. Le test du morceau 1 passe **sans changer ses valeurs figées**.

## Testing Decisions

- **Un bon test observe le comportement** : appartenance aux listes `hv`/`cv`/`cg` et leur ordre, ce que rend `mv`, champs lus par le lecteur public, pixels dessinés, compte de couverture. Jamais l'agencement mémoire au-delà de « en-tête à l'offset 0 ».
- **Tests de régression nommés par l'étape.** `built_planet_panel_nil_row` et `empty_planet_cleared` lisent `me` par la couche, sur des instances de `cg("jo")`, qui restent des instances à la fin de ce commit. Ils ne touchent ni la struct `ke` ni une `iv`. Ils **restent tels quels** (diff vide sur ces deux fichiers) et doivent passer. Pas de preuve rouge à refaire : leur code et le bug qu'ils gardent sont intacts. Leur réécriture et leur preuve rouge appartiennent à 1a.17 (`jo`), qui les liste aussi. Ils restent néanmoins une couverture d'intégration utile : leurs parties achètent (`hy:lu` → toasts `iv`), donc ils font passer des structs `iv` par `dg`/`dc`.
- **Couche d'entités** (dans `test_pico8_entity_smoke`, supprimé en 1a.42), avec une struct factice fille de `ke` construite par l'initialisation de `ke` :
  - `lt` l'enregistre dans `hv`, et dans `cv[p]` exactement pour les `bm` qu'elle porte (méthode posée ou champ non nil), et dans `cg` selon un `nu` rendu par son lecteur ; `he` l'en retire ;
  - l'ordre est préservé quand instances et structs sont mêlées (`lt` alterné, lecture de `hv`/`cv`) ;
  - `mv` : méthode de la table appelée avec la struct ; champ non fonction rendu tel quel ; nom inconnu → 0, sortie non touchée ;
  - `dc("ev")` mêlant une instance et une struct : ordre par `fa` croissant, `ra` passé ;
  - `dg` : méthode nommée par `me` appelée avec `t`, `t` incrémenté sur la struct, `p_` vrai → retirée ;
  - les tests existants d'instanciation directe de `ke` et d'une fille de `ke` sous forme d'instance passent sans modification.
- **`iv`, nouveau test unitaire** (dans `make unit`, lié comme `test_pico8_jb_ol`, avec les données réelles) :
  - caractérisation du morceau 1 : valeurs et empreintes figées sur l'ancien code, inchangées après conversion ;
  - défauts : le `v` de la struct est égal au bit près au vecteur que donne le parseur de spec sur la chaîne de spec d'`iv`, et `fa`/`l` sont égaux aux défauts parsés ;
  - forme : `pico8_iv_new` et `pico8_class_instantiate(pico8_iv_class(), …)` rendent un objet tagué `STRUCT`, présent dans `hv` et `cv.kj`, absent des autres `cv` ;
  - mutations : l'implémenteur remet localement au moins 4 erreurs plausibles et note dans le commit que chacune est attrapée : `lt` avant `iv:pq`, `t` non incrémenté, `v` à -0,33, `kj` absent de la table.
- **Couverture** : `make class-coverage-net` code 0, sortie identique à la référence (`iv 474 9/15`, `ke 0 0/15`).
- **Prior art** : `test_pico8_jb_ol` (caractérisation figée sur l'ancien code, mutations notées dans le commit) ; `test_pico8_entity_smoke` (`lt`/`he`/`dc`/`mv` sur des classes de test) ; `test_pico8_planet_kj_dispatch` (`pico8_jl` réel puis `dc("kj")` avec lecture du framebuffer).

## Critères du vérificateur

Chacun se traduit en `FAILED:` par le vérificateur de `prd-autopilot` (§7 du plan ; seul arrêt que `plan-pilot` honore).

- `FAILED:` si `make test` ou `make frames-check` est encore rouge après **3 tentatives** de correction sur ce commit.
- `FAILED:` si le diff modifie ou supprime un `*.sha256` du filet, ou retire une entrée de `FRAMES_EVENTS` ou de `RECORDINGS`.
- `FAILED:` si `make class-coverage-net`, relancé sur le code final, ne rend pas 0 ou si sa sortie n'est pas identique à `tools/class_coverage_net_expected.txt`. En particulier si `iv` n'est pas `iv  474  9/15` ou si `ke` n'est plus `ke  0  0/15  exemptée …`.
- `FAILED:` si `tools/class_coverage_net_expected.txt`, `tools/class_coverage_exempt.txt` ou `KNOWN[]` est modifié.
- `FAILED:` si un champ ou une clé de `ke`/`iv` ne tombe sous aucune règle du §5. C'est le cas notamment si l'assertion des clés reçues par `iv` (hors `ra`, 1, 2) se déclenche sur un rejeu ou un test : arrêt, pas de membre improvisé.
- `FAILED:` si un test de régression de bug devrait être modifié dans son intention pour passer. Aussi si `tests/test_pico8_built_planet_panel_nil_row.c` ou `tests/test_pico8_empty_planet_cleared.c` est modifié sans que la modification soit une réécriture « lire la struct » d'un accès à une struct, accompagnée de la preuve rouge du §6 notée dans le commit (attendu : aucune modification).
- `FAILED:` si une autre classe que `ke` et `iv` est convertie, si `ke` reçoit un constructeur de struct, si la `pico8_class_t` de `ke` perd ses défauts `me`/`t` ou son `pq=lt`, ou si la `pico8_class_t` d'`iv` n'est plus déclarée ou enregistrée à la même place.
- `FAILED:` si les valeurs figées du test de caractérisation d'`iv` ont été changées après la conversion (elles doivent être celles relevées sur l'ancien code).

Critères de réussite (en plus) :

- grep : plus aucun `g_iv_class->defaults` ;
- `pico8_iv_t` n'a que la struct `ke` et les 7 membres listés ;
- `dc` et `dg` acceptent `STRUCT` ;
- la lecture de champ de la couche asserte l'absence de `STRUCT` ;
- l'enum de méthodes vaut exactement `OL`, `PH`, `KJ` ;
- les deux tests de régression nommés ont un diff vide.

## Out of Scope

- Toute autre feuille de `ke`, l'arbre `jo`, `g_`, `pj`, `ui` ; la réécriture des tests de régression qui lisent des planètes (1a.17).
- `ke:ks` sous forme struct (aucun appelant struct) ; un constructeur autonome de `ke`.
- Adapter les consommateurs génériques qu'`iv` n'atteint pas : boucle de survol et `jz` de `ui`, `hk`/`hb` d'`economy` au-delà du `mv` déjà à double forme, lecteurs de `cg`. Chaque étape suivante les étend quand sa classe les atteint. La garde de la couche les signale.
- Écriture générique de champ par nom sur une struct : seul `t` est écrit par le noyau, et il est écrit directement.
- Types C réels (1b), libération des entités (TROU-7, Plan 2 §8), rapport de `make ref-audit`.
- Lancer l'appli raylib : jamais.

## Further Notes

- **Liste des étapes** : ce commit met à jour `docs/plan-3.md` pour que 1a.3 (`iv`) soit marquée faite dans 1a.2. La numérotation n'est pas décalée, pour que les renvois existants tiennent. Si le vérificateur préfère ne pas toucher au plan, le message de commit le dit, et le PRD de l'étape suivante constate que 1a.3 est sans objet.
- `make class-coverage-net` prend environ 4 minutes (15 rejeux).
- Mesures faites par grep sur e5aa9d1 ; elles sont qualitatives pour les grosses candidates (`mz`, `jc`, `ie`, `js`), dont l'écart avec `iv` n'est pas discutable.
- Le vecteur de défaut `v` partagé est sûr parce que `iv:ph` fait `self.ra+=self.v`, ce qui crée un nouveau vecteur : aucun site ne modifie `v` en place (mesuré).
