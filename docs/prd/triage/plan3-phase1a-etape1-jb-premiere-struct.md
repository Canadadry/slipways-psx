---
title: "Plan 3, étape 1a.1 : `jb` devient la première struct, et la coexistence struct/instance s'installe"
description: "Aucune entité n'existe encore sous forme de struct : la machinerie de coexistence du §5 (tag STRUCT, table de méthodes, délégation de pico8_class_instantiate) n'a jamais été construite ni vue par le filet, et `jb` est la racine sans fille choisie pour la prouver."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Étapes de la phase 1a ») :

> 1a.1 — `jb` : racine sans fille (`layout.c:357`), champ `ls`. Tests : aucun.

§5, règles qui s'appliquent à ce commit :

> Chaque classe devient un `struct` dont le premier membre est la struct de sa classe parente ; chaque champ de sa chaîne (défauts et champs écrits sur instance) devient un membre de type `pico8_value_t`, **nom minifié conservé**. Nil reste `pico8_value_ref(NULL)`, comme aujourd'hui.
>
> **Méthodes** : une table `static const` de pointeurs de fonction par classe ; une surcharge remplace l'entrée héritée. Les méthodes appelées par nom de chaîne (`mv(held,"hq")`, `jz(rm,…)`, `dc(nt)`) passent par un enum de méthode qui indexe cette table.
>
> **Coexistence** : une instance convertie porte un nouveau tag (`PICO8_TYPE_TAG_STRUCT`) et un pointeur vers sa table de méthodes, dans le même premier emplacement qu'aujourd'hui. `hv`/`cv`/`cg`, `dc`, `mv` traitent les deux formes pendant la migration ; l'ordre d'enregistrement est préservé.
>
> **Premier commit** : une vraie racine sans parent ni fille (`jb`), qui prouve la coexistence.
>
> **Héritage entre formes et classes-valeurs** : une classe convertie garde sa `pico8_class_t` enregistrée (registres `fz` et nommé compris) jusqu'à la dernière étape de 1a […]. Dans le commit de conversion, sa `pico8_class_t` reçoit un pointeur vers le constructeur de struct, et `pico8_class_instantiate` délègue à ce constructeur quand il est posé […].
>
> **Accès directs aux défauts** : tout site hors `pico8_class.c` qui lit ou écrit `<classe>->defaults` […] migre dans le commit de cette classe.

§7 : chaque commit de 1a passe `make class-coverage-net` (sortie identique à `tools/class_coverage_net_expected.txt`, versionnée à l'étape 1a.0) ; le crochet de couverture migre dans le constructeur de struct et enregistre par nom (`pico8_class_coverage_record`, livré à 1a.0).

**Emprise réelle de `jb`, mesurée en cadrant ce PRD (sur c3a1d94).**

- **Cart** : `jb=kv:jr()` (racine, pas de spec, pas de `fz`) ; `jb:pq()` pose `self.ls=nv(self, …)` — un `lq` par élément positionnel (chaîne enveloppée en `{ln}`, sinon la table telle quelle) ; `jb:ol(om,a)` dessine un cadre de hauteur `#self*8+2` puis chaque `lq` de `ls`.
- **Déclaration** : module de mise en page, juste après `lq` (`g_lq_class` puis `g_jb_class`, tous deux dans l'emplacement `""` du registre `fz`, absents du registre nommé). Méthode `pq` = `pico8_jb_pq` ; `ol` = `pico8_jb_ol`, appel direct, **pas** installé dans les défauts. Aucun accès `->defaults` sur la classe `jb` (grep : les seuls `->defaults` du module sont ceux de `lq`).
- **Champs** : `ls` seulement (REF vers une `pico8_list_t` allouée, TROU-7), plus les entrées positionnelles recopiées des overrides, lues **uniquement** par `pq`. `#self` dans `ol` est déjà porté comme `ls->count` (égalité documentée dans le module).
- **Overrides reçus** (mesuré par dump des données décodées) : les 7 entrées de `dr` (`cart.c:244`), `hp:jb()` (`{{it,13,…}}`), `nb[i].oy`, `fm.invented` (liste convertie). **Tous positionnels au niveau de `jb`** : les clés nommées (`c=`) n'apparaissent qu'un niveau plus bas, dans les lignes que `lq` reçoit.
- **Constructions** : `cart.c:244` (7 `ie`, via `pico8_jb_new(dr_entry)`) et `ui_button.c:184` (`js:pq` : `self.jb=self.jb and jb(self.jb)`, avec assert « spec table »). Aucun `pico8_class_instantiate` générique n'atteint `jb` (pas dans `gx[]`, pas de `fz`, pas nommé, pas `ie.ke`).
- **Lecteurs** : `ui_button.c:218` (`js:ev`, `self.jb:ol(127,1)`) et `research_icon.c:248` (`ie`, `self.jb:ol(0,0)`) — chacun lit le champ `jb` d'une instance **non convertie** (`js`, `ie`) puis caste le REF en instance.
- **Hors d'atteinte** : `jb` ne descend pas de `ke` → jamais dans `hv`/`cv`/`cg` ; aucun `mv`/`dc` ne reçoit un `jb`. `dc` et le parcours de `bootstrap` assertent déjà `PICO8_TYPE_TAG_INSTANCE` sur ce qu'ils lisent.
- **REF-désambiguïsation** : `pico8_type_tag_of` ne connaît que LIST/OB_TABLE/INSTANCE. `ie_self_ke_class` asserte qu'un `ie.ke` est UNKNOWN (une `pico8_class_t`, sans tag) — une `pico8_class_t` doit donc rester sans tag. `pico8_js_lq_arg_table` asserte sur un REF non taggé : il ne reçoit jamais `jb`. Aucun site `PICO8_REF_AS_LIST*` ne reçoit un `jb` (le seul, dans `jb:ol`, lit `ls`).
- **Couverture** : `jb 831 15/15` dans `tools/class_coverage_net_expected.txt` — à reproduire exactement.
- **Tests** : aucun fichier de `tests/` ne nomme `jb`.

## Solution

1. **Un nouveau tag** `PICO8_TYPE_TAG_STRUCT` (même famille de constantes magiques que les trois autres), reconnu par `pico8_type_tag_of`.
2. **Un en-tête commun des entités-structs** : le tag, puis le pointeur vers la table de méthodes de la classe — le « premier emplacement » du §5. Un enum de méthode indexe la table ; il ne contient que `ol` (nom réellement appelé par chaîne dans le port : `mv(…,"ol")` sur `lq`, et méthode de `jb`). La table est un tableau `static const` de pointeurs au type de méthode déjà en usage (`pico8_value_function_t`), pour que `mv` puisse l'appeler le jour où une struct l'atteint.
3. **`pico8_class_t` gagne un pointeur de constructeur de struct**, NULL par défaut. `pico8_class_instantiate` délègue au constructeur quand il est posé (sans lancer de `pq`, sans enregistrer de couverture lui-même : le constructeur le fait) ; sinon, comportement inchangé.
4. **`jb` devient une struct** : en-tête, puis un membre `pico8_value_t ls`. Son constructeur (celui que `pico8_jb_new` appelle et que la `pico8_class_t` de `jb` reçoit) alloue la struct, pose tag et table, enregistre `"jb"` au crochet de couverture, puis exécute le corps de `jb:pq` sur les éléments positionnels des overrides. `pico8_jb_ol` prend la struct.
5. **Les deux constructeurs et les deux lecteurs** passent à la struct : `cart.c` et `js:pq` stockent un REF vers la struct dans le champ `jb` de l'`ie`/`js` non converti ; `js:ev` et l'icône de recherche assertent le tag STRUCT avant de caster. C'est la coexistence que ce commit prouve : une instance à chaîne de prototypes porte, dans un champ ordinaire, un REF vers une struct, et le filet voit les deux.
6. `hv`/`cv`/`cg`, `dc`, `mv` **ne changent pas** : aucune struct ne les atteint dans ce commit (voir décisions).

## User Stories

1. En tant qu'autopilote de la phase 1a, je veux qu'une première entité existe sous forme de struct, afin que la coexistence du §5 soit prouvée par le filet avant d'être répétée 40 fois.
2. En tant qu'autopilote, je veux que cette première conversion soit une racine sans parent ni fille, afin que l'embarquement de la struct parente et la chaîne `parent` ne soient pas en jeu en même temps que la coexistence.
3. En tant que lecteur d'un REF, je veux que `pico8_type_tag_of` distingue une struct d'une instance, d'une table et d'une liste, afin qu'une struct castée au mauvais type soit une assertion et non un dessin faux.
4. En tant que lecteur d'un REF, je veux qu'une `pico8_class_t` reste non taggée (UNKNOWN), afin que l'assert « `ie.ke` tient une classe » reste vrai.
5. En tant qu'auteur des étapes suivantes, je veux un en-tête commun (tag puis table de méthodes) en tête de chaque struct d'entité, afin qu'une struct fille qui enchâsse sa parente en premier membre garde ce même en-tête à l'offset 0.
6. En tant qu'auteur des étapes suivantes, je veux que la table de méthodes soit indexée par un enum et typée comme les méthodes que `mv` appelle déjà, afin de pouvoir y brancher `mv`/`dc` sans changer sa forme.
7. En tant qu'auteur des étapes suivantes, je veux que l'enum ne contienne que les noms réellement appelés, afin que chaque ajout ultérieur soit tracé à une étape.
8. En tant qu'auteur des étapes suivantes, je veux que `pico8_class_instantiate` délègue au constructeur de struct quand la classe en a un, afin que `gx[]`, `ie.ke`, `lookup_named` et `kv[hp.cw]` construisent la forme struct sans être modifiés avant la conversion de leur propre classe.
9. En tant que mainteneur, je veux que la délégation n'enregistre la couverture qu'une fois (dans le constructeur), afin que le compte de `jb` reste 831 et non 1 662.
10. En tant que mainteneur, je veux que la délégation ne lance pas les `pq` de la chaîne de classes, afin que le corps de `jb:pq` ne s'exécute qu'une fois, dans le constructeur.
11. En tant que mainteneur, je veux qu'une classe sans constructeur de struct s'instancie exactement comme avant, afin que les 29 classes non converties ne bougent pas.
12. En tant que mainteneur de la couverture, je veux que la `pico8_class_t` de `jb` reste déclarée, nommée `"jb"` et enregistrée à la même place dans l'ordre de déclaration, afin que le registre `fz` (emplacement `""`) soit dans le même état qu'avant et que `KNOWN[]` n'ait pas à changer.
13. En tant que gardien du filet, je veux que les tooltips `jb` dessinés par `js` et par les icônes de recherche soient identiques pixel pour pixel, afin que l'invariant du §3 tienne.
14. En tant que gardien du filet, je veux un test qui dessine un `jb` construit depuis une vraie entrée de `dr` et compare l'image à celle du code d'avant, afin que le dessin de `jb:ol` soit protégé même si aucun rejeu du filet ne survole un tooltip.
15. En tant que lecteur du code, je veux que la struct `jb` n'ait que le membre `ls`, nom minifié conservé, afin que la traçabilité vers `pico8/lua.lua` tienne.
16. En tant que lecteur du code, je veux que les éléments positionnels des overrides soient lus par le constructeur et non stockés, afin de ne pas porter un membre que rien ne lit après `pq`.
17. En tant que mainteneur, je veux qu'un override de `jb` portant une clé nommée fasse échouer une assertion, afin qu'un champ nommé jamais mesuré ne soit pas perdu en silence (ce serait un champ sous aucune règle, §7).
18. En tant que mainteneur, je veux que `js:ev` et l'icône de recherche assertent le tag STRUCT avant de caster le champ `jb`, afin qu'un `jb` resté instance (ou une table de spec non convertie) soit détecté.
19. En tant que mainteneur, je veux que l'assert de `js:pq` (« le champ `jb` tient une table de spec ») reste en place, afin que la double construction reste impossible.
20. En tant que mainteneur, je veux que `dc` et le parcours de `hv` continuent d'asserter INSTANCE, afin qu'une struct qui les atteindrait avant l'étape qui les adapte soit un arrêt et non un comportement indéfini.
21. En tant que gardien du filet, je veux qu'aucun `.sha256`, aucune entrée de `FRAMES_EVENTS`/`RECORDINGS` et aucune ligne des références de couverture ne bouge, afin que l'étape ne se valide pas en déplaçant sa cible.
22. En tant que lecteur du code, je veux que les commentaires ajoutés ne portent qu'un contrat C ou un piège (règle de CLAUDE.md), afin de ne pas réintroduire du récit de chantier.
23. En tant que mainteneur de la mémoire, je veux que la struct `jb` et son `ls` ne soient jamais libérés, comme l'instance d'avant, afin que TROU-7 reste la seule convention de fuite.

## Implementation Decisions

Aucun agent frais n'était disponible pour les points où `to-prd` demande de consulter l'utilisateur (pas d'outil d'agent dans cette session) : j'ai chargé et appliqué moi-même le skill `guideline` (simplicité, changement chirurgical, hypothèses explicites, critères vérifiables). Les décisions tranchées ainsi sont marquées (G).

- **Tag.** `PICO8_TYPE_TAG_STRUCT`, constante pleine largeur au préfixe `0x9E5C` comme les trois autres, différente d'elles dans la moitié basse ; `pico8_type_tag_of` la rend. Le commentaire d'en-tête qui parle de « trois formes » est corrigé en conséquence (il deviendrait faux).
- **(G) En-tête de struct, table, enum — module d'en-tête dédié à la forme struct, petit.** Contenu :
  - l'en-tête : `tag` puis `methods` (pointeur vers la table `const`) ;
  - l'enum de méthode avec une seule valeur utile, `ol`, et le compte ;
  - la table : tableau de `pico8_value_function_t` de taille « compte » (le type que `mv` appelle déjà sur un champ fonction).
  Rejeté : une table vide (C n'a pas de tableau de taille 0) ; une struct de pointeurs typés par classe (une fille ne pourrait pas surcharger l'entrée de sa parente dans une table de même type) ; ajouter d'avance les noms de `mv`/`dc`/`jz` (spéculatif : chaque étape ajoute les siens).
- **Table de `jb`** : `static const`, entrée `ol` = un pont de dispatch vers `pico8_jb_ol` (même forme que le pont `ol` de `lq`). Les deux appelants gardent l'appel direct : `self.jb:ol(…)` vise statiquement un `jb`. L'entrée est lue par le test de coexistence.
- **`pq` hors de la table** (G) : pour une racine, le constructeur appelle le corps de `pq` directement. La règle de déduplication des `pq` le long de la chaîne ne concerne que les classes à parent (1a.2 et suivantes).
- **Constructeur dans `pico8_class_t`** : un membre « constructeur de struct » (prend les overrides `const`, rend un `void *` possédé par l'appelant), NULL à la déclaration, posé par un setter à côté de `pico8_class_set_pq`. `pico8_class_instantiate` : si posé, rend son résultat (casté en `pico8_instance_t *`) **avant** d'allouer quoi que ce soit, d'enregistrer la couverture ou de lancer un `pq`. Contrat à écrire dans l'en-tête : le pointeur rendu porte alors `PICO8_TYPE_TAG_STRUCT` et ne doit être lu qu'après vérification du tag. Signature de `pico8_class_instantiate` inchangée. Le membre est placé après les membres existants : `pico8_class_t` reste non taggée.
- **`pico8_jb_t`** : `{ en-tête ; pico8_value_t ls; }`. `ls` vaut `pico8_value_ref(NULL)` avant `pq`, puis un REF vers la liste allouée (TROU-7, jamais libérée ; la struct non plus — contrat à noter à la déclaration).
- **(G) Positionnels non stockés.** Le constructeur lit les éléments positionnels **dans les overrides** (même helper qu'aujourd'hui, même ordre croissant d'index) et n'en garde rien. Justification : aucun lecteur hors `pq` (grep), `#self` est déjà lu comme `ls->count`. Contrairement à `lq` (1a.34), où les positionnels sont relus par les méthodes, et pour lequel le plan prévoit un membre liste. Assertion porteuse de message dans le constructeur : aucun override de `jb` n'a de clé nommée (mesuré : aucun) — si un jour c'est faux, c'est un champ sous aucune règle, donc un arrêt §7, pas une adaptation.
- **Overrides NULL** : `jb` vide, `ls` liste vide (comportement actuel quand `dr[i]` manque).
- **API du module de mise en page** : `pico8_jb_new` rend `pico8_jb_t *` ; `pico8_jb_ol` prend `const pico8_jb_t *` (NULL ou `ls` absent → ne dessine rien, contrat actuel conservé) ; `pico8_jb_class()` reste (rend la classe, constructeur posé). La déclaration de la classe reste à la même place (après `lq`), avec son `PICO8_CLASS_COVERAGE_NAME`, sans `pq` de classe (le constructeur de struct le remplace).
- **Couverture** : le constructeur de `jb` appelle `pico8_class_coverage_record("jb")` sous `PICO8_CLASS_COVERAGE`, une fois par construction. `KNOWN[]` et les deux fichiers de `tools/` ne changent pas.
- **Sites convertis** : `cart.c` (7 `ie`) et `js:pq` stockent `pico8_value_ref(struct)` ; `js:ev` et le dessin de l'icône de recherche assertent `PICO8_TYPE_TAG_STRUCT` (assert porteur de message) puis castent en `pico8_jb_t`. L'assert « spec table » de `js:pq` reste.
- **(G) `hv`/`cv`/`cg`, `dc`, `mv` inchangés.** Mesuré : aucun `jb` ne les atteint (pas de `ke`, donc pas de `lt` ; aucun `mv`/`dc` sur un champ `jb`). Leur traitement des deux formes arrive avec la première struct qui s'y enregistre (1a.2, `ke`), là où le filet peut le voir ; l'écrire ici serait du code sans chemin d'exécution. Les asserts INSTANCE existants de `dc` et de `bootstrap` servent de garde d'ici là. « L'ordre d'enregistrement est préservé » est trivialement vrai : `jb` ne s'enregistre pas.
- **`pico8_ref_audit.h` inchangé** : aucun site de cast vers liste ne reçoit un `jb` ; une struct y serait comptée « unknown » en diagnostic, et rejetée par l'assert du build ordinaire, ce qui est le comportement voulu.
- **Accès directs aux défauts** : aucun pour `jb` (grep `->defaults` du module : `lq` seulement) — rien à migrer.
- **Commentaires** (CLAUDE.md) : seuls ajouts permis — contrat du constructeur (qui libère : personne, TROU-7), contrat de la délégation (tag du pointeur rendu), l'assert des overrides positionnels. Les commentaires du module de mise en page et de l'en-tête de classes que ce changement rendrait faux (« instance », « n'est pas une struct ») sont corrigés, pas complétés.

## Implementation Chunks

Un seul commit (§5 « Premier commit » ; §7 un commit par classe). Les morceaux sont des points d'arrêt TDD dans ce commit, chacun laissant `make test`, `make frames-check` et `make class-coverage-net` au vert.

1. **Machinerie, sans classe convertie.** Tag STRUCT reconnu ; en-tête/enum/table ; membre et setter de constructeur dans `pico8_class_t` ; délégation dans `pico8_class_instantiate`. Tests unitaires de la couche (voir Testing). Aucun site du jeu ne change : filet et couverture identiques trivialement.
2. **Test de caractérisation de `jb:ol`**, écrit et passé **sur le code d'avant la conversion** (image de référence obtenue ainsi).
3. **`jb` converti** : struct, constructeur, pont `ol` dans la table, classe reliée au constructeur, deux constructions et deux lecteurs. Le test du morceau 2 passe sans changer son image ; test de coexistence ajouté.

## Testing Decisions

- **Un bon test observe le comportement** : tag lu par `pico8_type_tag_of`, ce que rend `pico8_class_instantiate`, compte de couverture, pixels dessinés — jamais l'agencement mémoire au-delà du contrat « tag à l'offset 0 ».
- **Aucun test de régression de bug n'est concerné** (ligne de l'étape : « Tests : aucun » ; grep : aucun test ne nomme `jb`).
- **Couche de classes** (dans le test de la couche existant, `test_pico8_entity_smoke`, supprimé en 1a.42 avec elle) :
  - une classe avec constructeur de struct posé : `pico8_class_instantiate` rend exactement ce que rend le constructeur, le constructeur reçoit les overrides passés, aucun `pq` de la classe n'est appelé ;
  - une classe sans constructeur : comportement inchangé (tag INSTANCE, `pq` appelé) ;
  - `pico8_type_tag_of` sur un objet dont les 4 premiers octets sont `PICO8_TYPE_TAG_STRUCT` rend STRUCT ; sur une `pico8_class_t`, rend UNKNOWN.
- **`jb`, nouveau test unitaire** (dans `make unit`, lié comme les tests qui chargent les données réelles) :
  - **caractérisation** : construit un `jb` depuis une vraie entrée de `dr` (données du cart) et depuis une spec positionnelle à chaîne nue, le dessine avec `pico8_jb_ol` sur un framebuffer neuf, compare au hash (ou aux octets) obtenu sur le code d'avant la conversion. Le test est d'abord vert sur l'ancien code, puis doit rester vert, image inchangée ;
  - **coexistence** : `pico8_jb_new` rend un objet taggé STRUCT dont la table de méthodes est celle de `jb` et dont l'entrée `ol` est posée ; `pico8_class_instantiate(pico8_jb_class(), …)` rend aussi une struct taggée STRUCT ; overrides NULL → `ls` liste vide, rien dessiné hormis le cadre (contrat actuel, à vérifier sur l'ancien code d'abord).
- **Couverture** : `make class-coverage-net` code 0, sortie identique à `tools/class_coverage_net_expected.txt` (donc `jb 831 15/15`) — c'est aussi le test que la délégation ne double pas le compte.
- **Prior art** : `test_pico8_layout_nil_item_draws_nothing` (construit un `lq` réel et dessine sur un framebuffer), `test_pico8_data_init` (charge `pico8/map.json` et décode `dr`), `test_pico8_entity_smoke` (déclare et instancie des classes dans un test).

## Critères du vérificateur

Chacun se traduit en `FAILED:` par le vérificateur de `prd-autopilot` (§7 du plan ; seul arrêt que `plan-pilot` honore).

- `FAILED:` si `make test` ou `make frames-check` est encore rouge après **3 tentatives** de correction sur ce commit.
- `FAILED:` si le diff modifie ou supprime un `*.sha256` du filet, ou retire une entrée de `FRAMES_EVENTS` ou de `RECORDINGS`.
- `FAILED:` si `make class-coverage-net` (relancé sur le code final) ne rend pas 0, ou si sa sortie n'est pas identique à `tools/class_coverage_net_expected.txt` — en particulier si la ligne de `jb` n'est pas `jb  831  15/15` (compte doublé par la délégation, ou perdu par un constructeur qui n'enregistre pas).
- `FAILED:` si `tools/class_coverage_net_expected.txt`, `tools/class_coverage_exempt.txt` ou `KNOWN[]` est modifié.
- `FAILED:` si un champ ou une clé dynamique de `jb` ne tombe sous aucune règle du §5 — notamment si un override de `jb` s'avère porter une clé nommée (l'assertion du constructeur se déclenche sur un rejeu ou un test) : arrêt, pas d'ajout de membre improvisé.
- `FAILED:` si un test de régression de bug devrait être modifié dans son intention pour passer.
- `FAILED:` si une autre classe que `jb` est convertie, si `pico8_class_instantiate` change de signature, ou si la `pico8_class_t` de `jb` n'est plus déclarée/enregistrée à la même place.
- `FAILED:` si l'image du test de caractérisation de `jb:ol` a été changée après la conversion (elle doit être celle obtenue sur l'ancien code).

Critères de réussite (en plus) : grep montre `PICO8_TYPE_TAG_STRUCT` reconnu par `pico8_type_tag_of` ; `pico8_jb_t` n'a que l'en-tête et `ls` ; les deux lecteurs assertent STRUCT avant de caster ; aucune modification de `pico8_entity.c` ni de `pico8_ref_audit.h`.

## Out of Scope

- Toute autre classe, `ke` compris (1a.2), et l'adaptation de `hv`/`cv`/`cg`, `dc`, `mv` à la forme struct (arrive avec la première struct qui les atteint).
- Convertir `js` ou `ie`, qui gardent un champ `jb` ordinaire tenant un REF vers la struct.
- Libérer `jb` ou son `ls` (TROU-7 ; Plan 2 §8).
- Changer le rapport de `make ref-audit`.
- Types C réels pour `ls` (phase 1b).
- Lancer l'appli raylib : jamais.

## Further Notes

- `make class-coverage-net` prend ~4 min (15 rejeux) ; le vérificateur le relance au moins une fois sur le code final.
- `jb` est construit dans 15/15 rejeux, mais le dessin de son tooltip n'apparaît que si un rejeu survole un `js` ou une icône de recherche ; le test de caractérisation couvre ce que le filet pourrait ne pas voir.
- Mesure d'emprise faite par un dump jetable de `dr`/`nb` (scratchpad), non versionné.
