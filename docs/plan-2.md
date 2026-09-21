# Plan 2 — Suppression des dépendances flottant et `malloc`

Document distinct de [`docs/plan.md`](plan.md), qui reste le plan du **portage** (Phases 1 à 5, cible PC/raylib). Celui-ci couvre un chantier différent, postérieur, sur la même base de code : rendre le cœur porté indépendant de la virgule flottante puis de l'allocation dynamique.

Ce document est **le plan** ; aucune implémentation n'est engagée par sa rédaction.

---

## 1. Motivation, et ce qu'elle n'est pas

Deux motivations, données explicitement :

1. **Préparer la reprise PSX.** `docs/plan.md` acte que PSX n'est repris qu'une fois le PC pleinement jouable ; ce chantier prépare le terrain sans engager le portage lui-même.
2. **Contrainte auto-imposée** : viser un cœur *freestanding*, par choix de conception.

**Ce qui n'est PAS une motivation, et doit être dit pour ne pas être relitigé plus tard :** PSn00bSDK **fournit une libc** (`string.h`, `stdlib.h` avec `malloc`/`free` sur heap initialisé par `InitHeap`, `printf`). La disponibilité de la libc n'est donc pas un bloqueur PSX. La contrainte dure de ce chantier est esthétique (motivation 2) ; PSX en est le bénéficiaire, pas la cause. Le budget de ce chantier ne peut pas être justifié par « il le faut pour PSX ».

**Branche close, à ne pas rouvrir :** `stdint.h`, `stddef.h`, `stdbool.h`, `limits.h`, `float.h`, `stdarg.h`, `iso646.h`, `stdalign.h`, `stdnoreturn.h` sont les en-têtes fournis par **le compilateur**, pas par la libc (C99 §4 p6 : ce sont exactement ceux qu'une implémentation *freestanding* doit fournir). Le `mipsel-none-elf-gcc` de PSn00bSDK les livre quel que soit le choix de libc. Leur usage ne demande aucune décision.

---

## 2. Le bloqueur réel, et pourquoi le flottant passe avant `malloc`

Inventaire mesuré sur `src/` (~30 500 lignes, ~65 modules) :

| # | Bloqueur PSX | Ampleur | Réalité |
|---|---|---|---|
| 1 | **Virgule flottante** | 135 sites d'appel, 39 fichiers | Le R3000A **n'a pas de FPU** (COP2 = GTE, fixe). Chaque site devient un appel soft-float `libgcc`. Coût perf **et** taille dans 2 Mo. |
| 2 | **Tas non borné** | 102 sites, 41 fichiers | 2 Mo partagés, fragmentation sur partie longue. |
| 3 | Disponibilité libc | — | **Non-problème** (cf. §1). |

Le titre initial de ce chantier (« supprimer `malloc` et stdlib ») visait les bloqueurs 2 et 3. Le 3 n'existe pas, et le 1 — le plus coûteux et le plus structurant — n'y figurait pas, alors qu'il relève exactement de « dépendance stdlib » (`math.h` + `libgcc`).

**Décision : le flottant passe devant `malloc`.** Justification, à conserver parce qu'elle conditionne tout l'ordre des phases :

- Supprimer le flottant est une transformation **locale et falsifiable** : chaque site a un équivalent entier démontrable, et le comportement observable est préservé par construction sur la majorité des sites.
- Supprimer `malloc` est une transformation **globale** : elle change les durées de vie et les signatures (`pico8_sub()` et `pico8_value_tostr()` retournent aujourd'hui de la mémoire possédée), et peut réintroduire la classe de bugs corrigée en Phase 5 (UAF, commit `f3beb83`).

Faire le second d'abord, c'est déstabiliser la base juste avant d'y appliquer le premier.

---

## 3. Anatomie des 135 sites flottants

```
pico8_fixed_from_double(<littéral>)        61 occurrences, 34 littéraux distincts
(int) pico8_fixed_to_double(x)             76 occurrences
pow(), snprintf("%.4f")                     3 sites (economy_formulas.c:196-197, value.c:83)
sin()/cos() libc dans pico8_fixed          2 sites (fixed.c:254-266)
```

Un seul `from_double` porte sur une expression variable (`economy_formulas.c:197`, qui consomme le `pow`). Ce n'est donc pas 135 problèmes : **2 transformations mécaniques + 5 sites réécrits à la main.**

### 3.1 Les 61 constantes — le piège d'arrondi

Un littéral comme `0.27` dans `pico8/lua.lua` est converti en 16.16 **par PICO-8**, selon *sa* règle. Le C applique la sienne (`pico8_fixed.c:14-18` : `d*65536`, arrondi au plus proche). Les deux ne coïncident pas nécessairement :

```
0.27 × 65536 = 17694.72
  arrondi au plus proche → 17695  (0x0000.451f)   ← ce que fait le C aujourd'hui
  troncature             → 17694  (0x0000.451e)   ← si PICO-8 tronque
```

**28 des 34 littéraux distincts ne sont pas exactement représentables en 16.16** et dépendent donc de cette règle inconnue. Seuls `0.125`, `0.5`, `1.25`, `1.5`, `2.75`, `10.5` sont exacts et ne posent aucune question.

Un LSB d'écart sur `0.27` se propage dans `pico8_bk()` (`economy_formulas.c:211`), qui le multiplie ensuite. Deviner la règle violerait le garde-fou « Traçabilité » de `docs/plan.md`. → Résolu par le relevé de sonde (§5, bloc A) : les constantes seront **recopiées**, pas dérivées.

### 3.2 Les 76 casts — un accident de portage, pas une sémantique

`(int)` sur un `double` **tronque vers zéro** ; `x >> 16` sur un `int32_t` **arrondit vers −∞**. Pour toute valeur négative non entière, les deux diffèrent :

```
x = -98304  (= -1.5 en 16.16)
(int)pico8_fixed_to_double(x)  →  -1
x >> 16                        →  -2
```

Et des valeurs négatives arrivent réellement à ces sites — `src/pico8_layout.c:277-278` passe `pos.x`/`pos.y`, coordonnées écran après caméra, négatives dès qu'un objet sort par la gauche ou le haut. Un remplacement mécanique vers `>> 16` décalerait ces sprites d'un pixel, silencieusement, uniquement en sortie partielle d'écran.

**Constat de fond : ces casts n'encodent aucune sémantique voulue.** Côté Lua, `spr(item.qw, pos.x, pos.y)` ne convertit rien — le nombre reste fractionnaire et c'est `spr()` lui-même, dans le runtime PICO-8, qui décide. De même `t[n]` : en Lua `t[1.5]` n'est pas `t[1]`, c'est une clé distincte. La conversion a été **hissée au site d'appel par le portage C**, qui a choisi `(int)` par défaut sans que ce choix soit tracé nulle part.

`pico8/lua.lua` contient **17 occurrences de `flr(`** pour **76 sites C**. Le Lua ne répond donc pas pour la majorité d'entre eux. D'où la découpe en trois populations, qui est la structure de travail de ce chantier :

| Pop. | Définition | Traitement | Coût |
|---|---|---|---|
| **(a)** | Le Lua dit explicitement `flr(…)` | `flr` entier, sans discussion | ≤ 17 sites, trivial |
| **(b)** | Le Lua est muet, mais la valeur est **prouvablement ≥ 0** (indice issu d'un compteur, `#t`, boucle `for i=1,n`, identifiant de sprite) | Troncature et `flr` coïncident → **aucune conséquence observable**. Bascule mécanique ; il suffit de prouver le **signe**, pas la sémantique. | majorité, peu cher |
| **(c)** | Le Lua est muet **et** la valeur peut être négative (coordonnées écran, deltas, positions) | Vraie question ouverte → tranchée par le relevé de sonde (§5, bloc B) | poignée de sites |

**Posture retenue : audit (posture 2), et ce chantier assume de corriger des bugs de portage**, pas seulement de transformer. Conséquence directe sur le critère de fin (§6) : « terminé » n'est pas « le rendu est identique ».

### 3.3 Vérifications faites en cadrage (à ne pas refaire)

- **`#v` est bien la longueur au carré**, pas la longueur : `km:__len()` retourne `self:rg(self)` (`pico8/lua.lua:119-120`), et `km:qx()` = `sqrt(#self)` est la longueur réelle. Le port (`pico8_vec_len_sq`) est fidèle — pas de bug ici, malgré l'apparence du nom au site `pico8_dj`.
- **`^` apparaît 5 fois** dans `lua.lua` : `-t^3/9` (592), `min(self.gz,3.2)^2` (855), `(…/bq)^0.75` (1260), `0.18*mu^2` (1321), `(#fm.invented*1.2)^2` (1349). Quatre sont des puissances **entières**, déjà portées en multiplications fixes. Seule `^0.75` a fini en `pow()` flottant.
- **Le rejeu est déterministe** : `pico8_rnd_state` est une LCG à graine fixe (`0x2545F491`, `pico8_fixed.c:206-213`), sans API de reseed ni `time()`. Même `events.json` → mêmes frames. Prérequis du filet de non-régression : acquis.
- **Correction à porter dans `docs/plan.md`** : il y est écrit « framebuffer 128×128/**8** couleurs ». C'est faux — `pico8_dump_screen` émet des chiffres hexadécimaux `0`-`f`, donc **16** couleurs.

---

## 4. Périmètre

**Dans le périmètre**

- Le **cœur porté** : tout code atteignable depuis `pico8__init()` / `pico8__update60()` / `pico8__draw()`.
- Suppression du flottant de ce cœur (§3), puis de l'allocation dynamique (§8).

**Hors périmètre, explicitement**

- Les **tests**, `main.c`, `pico8_dump.c`, `pico8_assets.c`, le backend raylib et l'outillage de rejeu. Ils gardent `stdio`/`stdlib` : ce sont des outils d'hôte, et c'est exactement l'outillage de vérification sur lequel repose toute la stratégie de test de `docs/plan.md`. Les amputer détruirait le moyen de prouver que ce chantier n'a rien cassé.
- **`pico8_fixed_sin`/`cos`/`rnd` ne sont pas réécrits par ce chantier** (voir §5 blocs E/F, et §9 Risques). Leur relevé est capturé et stocké ; son exploitation est un chantier distinct.
- Le portage PSX lui-même.

---

## 5. Dispositif de vérification — deux instruments, à mettre en place AVANT toute modification

### Instrument 1 — Relevé de référence PICO-8 (`tools/probe.p8`)

Une cartouche autonome que l'humain charge et lance une fois ; elle écrit un fichier texte versionné dans le dépôt.

**Principe directeur : tout est relevé en bits bruts, jamais en décimal.** PICO-8 est en virgule fixe 16.16 — ses nombres *sont* des entiers. `tostr(x, true)` retourne la représentation hexadécimale exacte (`"0x0001.0000"`). Plus d'aller-retour décimal, plus d'arrondi d'affichage, plus d'ambiguïté sur le nombre de décimales.

**Aucune interprétation visuelle n'est demandée à l'humain.** Pour les questions d'arrondi de coordonnées, la cartouche dessine puis **relit les pixels avec `pget()`** et imprime la colonne trouvée. Les cas de test sont choisis pour que les deux réponses possibles restent à l'écran (p. ex. pixel allumé à l'offset 7 du sprite, dessiné en `x = -0.5` : troncature → colonne 7, `flr` → colonne 6).

| Bloc | Contenu | Résout |
|---|---|---|
| **A** | Les 34 littéraux distincts de `lua.lua`, en hexa | §3.1 — les 61 constantes, **recopiées** et non dérivées |
| **B** | `flr` sur négatifs ; arrondi réel de `spr()` relu par `pget` ; `t[1.5]` vs `t[1]` | §3.2 — population (c), et les sites d'indexation |
| **C** | `tostr` **décimal** de valeurs témoins (`-1.5`, `1/3`, `0.15`, `1`), en regard de leur hexa | `pico8_value.c:83`, dont le commentaire acte déjà « pas les règles exactes de PICO-8 » |
| **D** | `(x/bq)^0.75` sur ~16 entrées témoins, en hexa | `economy_formulas.c:196` — le `pow()` |
| **E** | `sqrt`, `sin(i/64)` et `cos(i/64)` pour `i=0..63`, en hexa | *capturé, non exploité* |
| **F** | `srand(1)` puis 64 tirages `rnd(1)`, en hexa | *capturé, non exploité* |

**A→D servent ce chantier. E et F ne le servent pas** : ils visent le risque ouvert n°1 des Notes de portage (`rnd`/`sin`/`cos` non bit-exacts, objectif « même graine → même galaxie » acté non atteint). Ils sont capturés parce que les six blocs coûtent **la même manipulation unique**, et que ne pas les prendre garantirait de redemander la manip plus tard.

> **Clôture de périmètre, à opposer à toute dérive :** E et F sont **capturés et stockés**, pas exploités. `pico8_fixed_sin`/`cos`/`rnd` ne sont pas touchés par ce chantier.

**Effet de second ordre, qui est le vrai gain de « testable » :** aujourd'hui `tests/test_pico8_fixed.c` n'assertent que des *contrats* (plage, cas zéro, périodicité) et **jamais une valeur numérique**, faute de source — les Notes de portage l'actent explicitement. Le fichier de relevé, versionné, devient une **fixture** : les tests assertent désormais des valeurs exactes relevées sur PICO-8. C'est un changement de nature, pas un ajout de couverture.

*Réserve connue :* la destination exacte du fichier écrit par `printh()` varie selon la version et la plateforme de PICO-8. La cartouche imprime donc **aussi** tout à l'écran, en secours d'une capture d'écran.

### Instrument 2 — Filet de non-régression par empreintes

Un `events.json` enregistré par le mode enregistrement du backend raylib sur une vraie session, couvrant la chaîne sonde → wormhole → colonisation → slipway → commerce.

**Empreintes, pas pixels :**

```
dump texte actuel  : 16 512 octets / frame  (128 lignes de 128 hexa + saut de ligne)
1 000 frames       : 16,5 Mo versionnés — par baseline, et il y aura des re-baselines
empreinte / frame  : 8 octets  →  1 000 frames = 8 Ko
```

→ un fichier `frames.hash` versionné (une empreinte par ligne), et le dumper conserve la capacité d'émettre la frame *N* en entier à la demande. Quand une empreinte diverge, on régénère cette frame des deux côtés et on diffe 128 lignes. Volume maîtrisé, diagnostic intact.

> **Ce filet est une base de non-régression, pas un oracle de fidélité.** Il enregistre ce que produit le port C, pas ce que produit PICO-8. Il répond à « ai-je cassé quelque chose ? », jamais à « était-ce juste ? ». Cette seconde question n'a qu'une source : l'instrument 1.
>
> **Corollaire impératif sur l'ordre :** capturer ce filet *avant* d'avoir tranché la population (c) gèlerait la troncature actuelle dans un fichier versionné — et toute correction ultérieure vers `flr` apparaîtrait alors comme une régression. Un doute serait transformé en apparente certitude, dans le mauvais sens.

---

## 6. Critère de fin — objectivement vérifiable

Aucun de ces points ne demande de jugement ni de relecture :

1. `grep -rnE '\b(double|float)\b' src/` → vide, hors les helpers de test explicitement isolés et nommés comme tels.
2. La cible de lien du cœur perd `-lm`, et le build passe.
3. `nm` sur le binaire → **aucun symbole soft-float de `libgcc`** (`__floatsidf`, `__muldf3`, `__adddf3`, `__fixdfsi`, `__divdf3`…). **C'est le seul critère qui prouve réellement l'objectif PSX** : les points 1 et 2 peuvent passer alors qu'un flottant subsiste dans une constante non repliée par le compilateur.
4. `frames.hash` **identique** avant/après pour les populations (a) et (b). Les seules divergences autorisées relèvent de la population (c) et des 5 sites réécrits, **chacune justifiée par une ligne du relevé de sonde**.

Le point 4 est ce qui empêche ce chantier de devenir une réécriture à l'aveugle : il sépare mécaniquement « j'ai transformé » de « j'ai changé ».

---

## 7. Phases — volet flottant

### Phase A — Relevé PICO-8
Rédiger `tools/probe.p8` (blocs A→F). L'humain la lance et fournit la sortie. Le relevé est versionné dans le dépôt comme donnée de référence.
**Sortie de phase :** le fichier de relevé est dans le dépôt, et les 34 littéraux distincts y ont chacun leur valeur hexa.

### Phase B — Capture du filet
Enregistrer la session via le mode enregistrement raylib, produire `frames.hash` de référence **sur le code actuel, avant toute modification**.
**Sortie de phase :** `frames.hash` versionné, et rejouer deux fois le même `events.json` produit le même fichier (détermination vérifiée, pas supposée).

### Phase C — Populations (a) et (b), strictement préservatrices
Les 34 constantes remplacées par les entiers relevés ; les sites (a) en `flr` entier ; les sites (b) basculés après **preuve du signe** consignée par site.
**Sortie de phase :** `frames.hash` **identique** à celui de la Phase B. Toute divergence est un bug introduit, pas un progrès.

### Phase D — Population (c) et les 5 sites flottants
Les sites à signe potentiellement négatif tranchés par le relevé (bloc B). `pico8_value_tostr` réécrit en entier d'après le bloc C. `pico8_dj`'s `^0.75` réécrit en fixe d'après le bloc D.
**Sortie de phase :** chaque frame divergente est expliquée par une ligne du relevé ; `frames.hash` re-baseliné **volontairement**, avec revue des frames concernées — jamais subi comme une régression.

### Phase E — Fermeture
`-lm` retiré de la cible du cœur, vérification `nm`, entrées consignées en Notes de portage.
**Sortie de phase :** les quatre points du §6 passent.

---

## 8. Volet `malloc` — cadrage NON FAIT

> **Cette section n'est pas un plan. C'est la liste de ce qui reste à trancher.** Elle est écrite maintenant pour que les questions soient posées avant, pas découvertes en cours de route ; les réponses ne sont pas inventées ici.

### Ce qui est déjà mesuré

102 sites d'allocation sur 41 fichiers. L'allocation n'est pas périphérique : c'est **la couche d'émulation Lua elle-même** — `pico8_value_t` (chaînes possédées via `pico8_strdup_owned`), `pico8_list_t`, `pico8_ob_table_t`, `pico8_class_t`/`pico8_instance_t`, `pico8_entity`. Chacune réimplémente indépendamment le même motif « tableau croissant à doublement », déjà signalé trois fois en Notes de portage.

Une structure favorable apparaît à l'inspection du chemin de dessin :

```
pico8_layout.c:212    malloc(sizeof *item)       ← un item de layout, par frame
pico8_ui.c:151,284    malloc(sizeof *heap_le)    ← un pico8_list_t boxé
pico8_msgbox.c:247…   malloc(sizeof *heap_ra)    ← un pico8_vec_t boxé
```

La majorité de ces allocations sont de **petites structures boxées à durée de vie d'une frame**, qui n'existent que parce que Lua manipule des références. Elles ne demandent pas un allocateur, mais une **arène à remise à zéro en début de frame, sans jamais appeler `free`**. C'est une piste, pas une décision.

### Questions ouvertes, à griller avant d'écrire quoi que ce soit

1. **La découpe durée de vie.** Longue durée (état de jeu : entités, registre de classes, galaxie) → pools statiques dimensionnés par les bornes du jeu (`bw(90)`, `pico8_galaxy.c:123`). Courte durée (frame) → arène à remise à zéro. **La découpe tient-elle réellement ?** Chaque allocation supposée « frame-scoped » doit être prouvée telle. Le UAF de `f3beb83` est la preuve que ces durées de vie sont déjà floues — une arène remise à zéro transformerait un tel bug en corruption silencieuse au lieu d'un crash.
2. **Le dimensionnement.** Chaque pool exige une borne maximale. Lua n'en a aucune. D'où viennent ces bornes, et que fait le jeu quand une borne est atteinte ?
3. **Le comportement en épuisement.** Aujourd'hui : `abort()` (`pico8_class.c:229,294`). Sur PSX, `abort()` n'a aucun sens. Épuisement d'un pool = écran noir ? dégradation ? assertion ?
4. **Les signatures.** `pico8_sub()` et `pico8_value_tostr()` retournent de la mémoire possédée par l'appelant. Sans `malloc`, elles doivent changer de forme (tampon fourni par l'appelant ? arène ? pool de chaînes internées ?). Combien de sites d'appel en dépendent ?
5. **La mutualisation.** Faut-il d'abord unifier le motif « tableau croissant » réimplémenté 4+ fois — ce que les Notes de portage réclament déjà indépendamment de ce chantier — avant d'y toucher, pour ne pas faire le travail quatre fois ?
6. **Le filet.** `frames.hash` suffit-il à prouver une transformation qui change les durées de vie ? Probablement pas seul : une corruption mémoire peut ne pas se voir sur une frame. Quel instrument complémentaire (ASan/Valgrind sur le build hôte, que ce chantier **ne doit pas** casser — d'où le §4, les outils gardent la libc) ?

---

## 9. Garde-fous

- **Traçabilité** (hérité de `docs/plan.md`) : aucune valeur numérique non sourcée. Une constante vient du relevé, jamais d'une règle d'arrondi supposée. Une sémantique d'arrondi vient du relevé ou d'un `flr` explicite du Lua, jamais d'un réflexe C. Un site de population (c) non résolu par le relevé est consigné comme risque ouvert, il n'est pas deviné. **Ne pas choisir est aussi un choix** : conserver la troncature actuelle n'est pas plus « neutre » que passer à `flr` — elle n'a pas davantage de source.
- **Le filet est capturé avant**, sinon la Phase C n'est qu'une promesse.
- **Jamais de fenêtre interactive lancée par l'agent** (hérité de `CLAUDE.md`). Ce chantier se vérifie entièrement en headless. L'humain n'intervient que deux fois : lancer la sonde (Phase A), enregistrer la session (Phase B).
- **Les outils gardent la libc** (§4). Les amputer détruirait le moyen de prouver le reste.
- **E/F capturés, pas exploités** (§5).
- **Constats structurels récurrents** : ce chantier hérite de la liste vivante des Notes de portage — motif de tableau croissant, seuil de taille de module, sites de cast REF non gardés.

---

## 10. Risques

| Risque | Effet | Traitement |
|---|---|---|
| Le relevé ne tranche pas la population (c) | Des sites restent sans source | Consignés en risque ouvert, comportement actuel conservé **et documenté comme non sourcé** — pas comme délibéré |
| `printh()` n'écrit pas où attendu | Pas de fichier exploitable | Sortie écran en secours, tenant en une capture |
| `frames.hash` diverge en Phase C | Bug introduit par une transformation censée être neutre | C'est précisément l'objet du critère §6.4 : la divergence est le signal, pas un désagrément |
| Le `pow(^0.75)` en fixe n'est pas reproductible à partir de 16 points témoins | `pico8_dj` reste approximatif | Risque accepté et consigné ; c'est déjà le statut actuel (note « fidélité numérique fractionnaire » de `pico8_economy_formulas.h`) |
| Dérive vers la réécriture de `sin`/`cos`/`rnd` | Ce chantier absorbe un chantier voisin et ne finit pas | Clôture explicite du §5 |
| La découpe de durée de vie du volet `malloc` ne tient pas | Corruption mémoire silencieuse | Volet non engagé tant que le §8 n'est pas grillé |

---

## 11. Décisions actées en cadrage

Figées à l'issue de la session de cadrage, pour ne pas être relitigées :

1. Motivations retenues : préparation PSX + contrainte freestanding auto-imposée. La libc n'est **pas** un bloqueur PSX.
2. Les en-têtes freestanding sont fournis par le compilateur — branche close.
3. Le flottant passe **avant** `malloc`.
4. Les 76 casts font l'objet d'un **audit** (posture 2), et ce chantier **assume de corriger des bugs de portage**.
5. Le découpage en populations (a)/(b)/(c) est la structure de travail ; (b) se prouve par le **signe**, pas par la sémantique.
6. La source de vérité pour (c), les constantes, `tostr` et `^0.75` est un **relevé PICO-8 en bits bruts**, pas une convention ni une décision d'ingénierie.
7. Le relevé couvre **six blocs** en une seule manipulation ; E et F sont stockés sans être exploités.
8. Le filet de non-régression stocke des **empreintes par frame**, pas des pixels, et est capturé **avant** toute modification.
9. Le critère de fin inclut la vérification `nm` de l'absence de symboles soft-float — seule preuve réelle de l'objectif PSX.
10. Le volet `malloc` n'est **pas** cadré ; le §8 liste ses questions ouvertes.
