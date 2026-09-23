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

Les numéros de ligne de cette section datent du cadrage et dérivent ; la liste à jour est `tools/float_inventory.txt` (`make float-inventory`, Phase B2).

Un seul `from_double` porte sur une expression variable (`economy_formulas.c:197`, qui consomme le `pow`). Ce n'est donc pas 135 problèmes : **2 transformations mécaniques + 5 sites réécrits à la main.**

### 3.1 Les 61 constantes — le piège d'arrondi

Un littéral comme `0.27` dans `pico8/lua.lua` est converti en 16.16 **par PICO-8**, selon *sa* règle. Le C applique la sienne (`pico8_fixed.c:14-18` : `d*65536`, arrondi au plus proche). Les deux ne coïncident pas nécessairement :

```
0.27 × 65536 = 17694.72
  arrondi au plus proche → 17695  (0x0000.451f)   ← ce que fait le C aujourd'hui
  troncature             → 17694  (0x0000.451e)   ← si PICO-8 tronque
```

**28 des 34 littéraux distincts ne sont pas exactement représentables en 16.16** et dépendent donc de cette règle inconnue. Seuls `0.125`, `0.5`, `1.25`, `1.5`, `2.75`, `10.5` sont exacts et ne posent aucune question.

> **RÉSOLU par le relevé** (`tools/probe_out.txt`, PICO-8 0.2.6b, bloc A) : **PICO-8 tronque**, `floor(v × 65536)`, vérifié sans exception sur les 54 littéraux de `lua.lua`.
>
> ```
> 0.27  → 0x0000.451e = 17694     le C produit 17695
> 0.1   → 0x0000.1999 =  6553     le C produit  6554
> 0.001 → 0x0000.0041 =    65     le C produit    66
> ```
>
> **Conséquence : 16 des 34 constantes du C sont aujourd'hui décalées d'un LSB.**
> `0.001` `0.0032` `0.005` `0.031` `0.05` `0.08` `0.0977` `0.1` `0.19` `0.27` `0.55` `0.6` `0.8` `0.85` `0.99` `1.08`.
> Les 18 autres coïncident par hasard (la valeur exacte ou un `.x` où troncature et arrondi tombent pareil).
>
> C'est le premier bug trouvé par ce chantier, et il était invisible autrement : aucun test ne portait sur une valeur numérique. Les constantes sont désormais **recopiées** du relevé, jamais dérivées.

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
| **(c)** | Le Lua est muet **et** la valeur peut être négative (coordonnées écran, deltas, positions) | **`flr`** — tranché par le relevé, voir ci-dessous | poignée de sites |

> **RÉSOLU par le relevé** (bloc B) : **`spr()` applique `flr`, pas une troncature.** Le pixel témoin est à l'offset local (7,0) du sprite, donc `colonne − 7 = origine` :
>
> ```
> spr(-0.5, 4)  → col 6    origine -1      troncature aurait donné 0 → col 7
> spr(-0.9, 8)  → col 6    origine -1      troncature → 0
> spr(-1.5,12)  → col 5    origine -2      troncature → -1 → col 6
> spr(4, -0.5)  → none     origine -1, hors écran ; troncature → 0, visible
> ```
>
> `flr` lui-même est bien un arrondi vers −∞ (`flr(-1.5) → -2`, `flr(-0.5) → -1`).
>
> **Les 76 casts sont donc faux pour toute coordonnée négative** — `pico8_layout.c:277-278` en tête. Deuxième bug confirmé, avec source.
>
> **Découverte non anticipée, même bloc : les clés de table fractionnaires sont des clés distinctes.** `t[1.5]="a"` puis `t[1]` → `nil`, et `t[2]="b"` puis `t[2.5]` → `nil`. Le C, lui, convertit la clé en `int` *avant* la recherche (`pico8_score_panel.c:110`, `pico8_planet.c:309`, `economy_formulas.c:55,66`…) : sur une clé fractionnaire il trouverait une entrée là où Lua ne trouve rien. À vérifier site par site pendant l'audit — une clé toujours entière par construction rend le point sans objet, mais ça doit être établi, pas supposé.

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

**A→D servent ce chantier. E et F ne le servent pas** : ils visent le premier trou de portage (`rnd`/`sin`/`cos` non bit-exacts, objectif « même graine → même galaxie » acté non atteint — [TROU-1](trous-de-portage.md)). Ils sont capturés parce que les six blocs coûtent **la même manipulation unique**, et que ne pas les prendre garantirait de redemander la manip plus tard.

> **Clôture de périmètre, à opposer à toute dérive :** E et F sont **capturés et stockés**, pas exploités. `pico8_fixed_sin`/`cos`/`rnd` ne sont pas touchés par ce chantier.

**Effet de second ordre, qui est le vrai gain de « testable » :** aujourd'hui `tests/test_pico8_fixed.c` n'assertent que des *contrats* (plage, cas zéro, périodicité) et **jamais une valeur numérique**, faute de source. Le fichier de relevé, versionné, devient une **fixture** : les tests assertent désormais des valeurs exactes relevées sur PICO-8. C'est un changement de nature, pas un ajout de couverture.

*Réserve connue :* la destination exacte du fichier écrit par `printh()` varie selon la version et la plateforme de PICO-8. La cartouche imprime donc **aussi** tout à l'écran, en secours d'une capture d'écran.

### État : relevé effectué

`tools/probe_out.txt` — PICO-8 **0.2.6b**, macOS, 342 lignes. Résultats par bloc :

| Bloc | Résultat | Effet |
|---|---|---|
| **A** | PICO-8 **tronque** les littéraux | §3.1 — 16 constantes du C à corriger |
| **B** | `spr()` applique **`flr`** ; clés fractionnaires distinctes | §3.2 — population (c) tranchée, 2ᵉ bug confirmé |
| **C** | 13 échantillons sur 14 **correspondent déjà** au C actuel | voir ci-dessous — un seul cas reste ouvert |
| **D** | `x^0.75 = sqrt(x) · sqrt(sqrt(x))` | §7 Phase D — plus de `pow` à écrire |
| **E/F** | capturés | non exploités, cf. clôture ci-dessus |

**Bloc C — le seul point encore ouvert.** `pico8_value_tostr` (`%.4f`, trim des zéros, trim du point) reproduit exactement PICO-8 sur 13 des 14 témoins, `2/3 → "0.6667"` et `1/3 → "0.3333"` compris. Le seul écart est `-0.0001` (brut −6) : PICO-8 affiche **`"-0"`**, le C afficherait `"-0.0001"`. Aucune règle unique ne réconcilie ce cas avec `2/3 → "0.6667"` — donc **il n'est pas deviné**. Il est consigné comme question ouverte, à trancher par une relance d'une ligne de la sonde. Il ne bloque pas : aucune valeur affichée par le jeu (crédits, scores, comptes) n'a une magnitude inférieure à 0.0001.

**Bloc D — la formule.** Vérifiée sur les vecteurs : `x=2 → 0x0001.ae87 = 1.68178`, et `sqrt(2)·sqrt(sqrt(2)) = 1.414214 × 1.189207 = 1.681793` ; `x=0.25 → 0.353546 = 0.5 × 0.70711` ; `x=16 → 8` exact. Deux `sqrt` et une multiplication, tout en fixe. Les 16 vecteurs servent de test.

**Bloc F, en passant :** `rnd(0)` **n'avance pas** le PRNG (`rnd(10)` juste après redonne le premier tirage), et `rnd(x)` se comporte comme `r mod (x·65536)`. Le C matche déjà le premier point (`pico8_fixed.c:229`, retour avant `next_u32`). Consigné, **pas exploité**.

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
3. Les objets du cœur **compilent avec `-mgeneral-regs-only`** (erreur du compilateur sur tout flottant à l'exécution, constantes repliées tolérées). **C'est le seul critère qui prouve réellement l'objectif PSX** : les points 1 et 2 peuvent passer alors qu'un flottant subsiste dans une constante non repliée par le compilateur.
   *Angle mort sur x86_64, mesuré en B2 :* la garde rattrape les retours et opérations `double`, pas un argument `double` passé à une fonction externe — GCC le passe sur la pile sans erreur. `pico8_fixed_from_double(<littéral>)` compile donc sous la garde : voir « État de B2 » au §7.
   *Pourquoi pas `nm` :* sur l'hôte x86_64 le FPU est matériel, donc aucun symbole soft-float de `libgcc` (`__muldf3`, `__adddf3`…) n'est jamais émis — `nm` était **déjà vide avant tout changement** (mesuré : 0 symbole, alors que `pow`/`sin`/`cos` sont importés de la glibc). Un critère qui passe sur le code de départ ne prouve rien. Alternative équivalente : `nm` sur les `.o` du cœur compilés par le `mipsel` PSn00bSDK en soft-float.
4. `frames.hash` **identique** avant/après pour les populations (a) et (b). Les seules divergences autorisées relèvent de la population (c) et des 5 sites réécrits, **chacune justifiée par une ligne du relevé de sonde**.

Le point 4 est ce qui empêche ce chantier de devenir une réécriture à l'aveugle : il sépare mécaniquement « j'ai transformé » de « j'ai changé ».

---

## 7. Phases — volet flottant

### Phase A — Relevé PICO-8 — ✅ FAITE
`tools/probe.p8` rédigée, lancée sur PICO-8 0.2.6b (macOS), sortie versionnée en `tools/probe_out.txt`. Les 54 littéraux de `lua.lua` y ont leur valeur hexa. Résultats : §5, « État : relevé effectué ».
**Reliquat :** le cas `tostr` de magnitude < 0.0001 (bloc C) — une relance d'une ligne, non bloquante.

### Phase B — Capture du filet
Enregistrer la session via le mode enregistrement raylib, produire `frames.hash` de référence **sur le code actuel, avant toute modification**.

Méthode, en deux étapes :

- **B1 — Empreintes.** Le filet n'est pas une seule session : la session humaine (`events.json` racine) n'atteint ni wormhole, ni slipway, ni commerce — donc pas le `^0.75`. Une empreinte par frame est prise **pour chaque fichier d'événements de jeu existant** (l'`events.json` racine et les fixtures de jeu non malformées de `tests/fixtures/`), un fichier d'empreintes par fichier d'événements.
- **B2 — Inventaire, couverture, garde.** (i) Un script versionné produit l'inventaire des sites flottants du cœur par catégorie — (a), (b), (c), constantes, les 5 sites réécrits — au lieu de numéros de ligne cités, qui dérivent. (ii) Un build de couverture (`--coverage`/`gcov`) reproductible liste chaque site avec son nombre de passages sous le filet. (iii) La garde `-mgeneral-regs-only` du §6.3 est posée sur les objets du cœur, et on consigne qu'elle **échoue** sur le code actuel, avec la liste des fichiers en échec.

**État de B2** (mesuré avec `cc` GCC 13.3, x86_64). Les trois cibles sont hors de `make test`, qui ne lance que leurs tests sur des entrées fabriquées. Toutes partent de `CORE_SRCS` du `Makefile` : `ENGINE_SRCS` moins `pico8_assets.c` et `pico8_dump.c`, 58 fichiers `.c`.

- **Inventaire** — `make float-inventory` → `tools/float_inventory.txt`, une ligne par jeton, identité (fichier, fonction, catégorie, rang) indépendante du numéro de ligne. Comptes : 61 constantes, 76 casts, 5 réécrits, 23 autres (à auditer), 165 au total. Les populations (a), (b), (c) valent 0 : les 76 casts portent « à auditer », faute de règle déductible du code seul ; leur classement est le travail de la Phase C. L'inventaire ne lit que les `.c` du cœur : le grep du §6.1 sur `src/` voit en plus les prototypes de `pico8_fixed.h` (réservés aux tests), des mots de commentaire d'en-têtes (« double-free »…), le backend raylib et le visualiseur de screencast. À l'inverse, l'inventaire compte aussi les mots `double` des commentaires des `.c` du cœur, en « autre ».
- **Couverture** — `make float-coverage` → `tools/float_coverage.txt`, passages par site après rejeu de chaque fichier de `FRAMES_EVENTS` ; granularité à la ligne, pas à l'expression. 122 sites couverts, 43 `NON COUVERT` (constantes 38/23, casts 60/16, réécrits 5/0, autres 19/4). C'est ce rapport que cite la règle pour C et D ci-dessous.
- **Garde** — `make float-guard` → `tools/float_guard.txt`, la liste des fichiers du cœur qui ne compilent pas avec `-mgeneral-regs-only`, sans édition de liens. **Elle échoue sur le code actuel : 32 fichiers sur 58**, tous sur « SSE register return with SSE disabled ». La cible sort en échec tant que la liste n'est pas vide ; le fichier versionné est réécrit même en échec, et sa décroissance se lit dans l'historique.
- **Angle mort de la garde.** Six fichiers compilent sous la garde alors qu'ils contiennent des constantes `pico8_fixed_from_double(<littéral>)` (20 au total) : `pico8_entity_geom.c`, `pico8_galaxy.c`, `pico8_game_data.c`, `pico8_geom.c`, `pico8_slipway.c`, `pico8_wormhole.c`. La liste de la garde n'est donc pas l'inventaire : la source de vérité sur les sites restants est `tools/float_inventory.txt`, et §6.3 ne prouve rien de plus sur cet hôte que les fichiers qu'il voit. La garde `mipsel` soft-float du §6.3 fermerait cet angle ; elle n'est pas posée.

**Sortie de phase :** fichiers d'empreintes versionnés, et rejouer deux fois le même fichier d'événements produit le même fichier d'empreintes (détermination vérifiée, pas supposée) ; inventaire et rapport de couverture reproductibles ; garde `-mgeneral-regs-only` en place et en échec constaté.

**Règle pour C et D, issue de B2 :** chaque étape liste les sites qu'elle touche avec leur statut de couverture. Un site non atteint par le filet reçoit soit un test unitaire au niveau valeur, soit une fixture scriptée qui l'atteint, soit une entrée explicite « non protégé par le filet » dans ce plan — jamais un passage silencieux. La couverture et la liste des fichiers encore en échec sous la garde sont relancées en début de chaque phase ; la sortie de chaque phase s'énonce aussi en « compte de la catégorie X = 0 » d'après l'inventaire.

### Phase C — Populations (a) et (b), et les constantes
Les sites (a) en `flr` entier ; les sites (b) basculés après **preuve du signe** consignée par site. Les 34 constantes remplacées par les entiers de `tools/probe_out.txt`.

**C0 — Classement, propriété, fraîcheur (avant toute modification de site).**
- **Table de propriété** : chaque catégorie ou sous-groupe de l'inventaire est affecté à **exactement une** étape (C1, C2b, D1–D4, E) ; la somme des comptes par étape égale le total de l'inventaire. Un site sans propriétaire devient une **décision ouverte consignée dans ce plan**, jamais un oubli découvert en Phase E.
- **Fichier d'annotation versionné** des 76 casts : population + une ligne de justification par cast, joint à l'inventaire comme l'est la couverture. Une annotation orpheline ou un cast non annoté est une erreur. La clé doit survivre à la disparition d'un site voisin (fonction + extrait ; le rang ne sert qu'à départager), sinon convertir un cast décale les annotations de ses frères.
- **Observation** (facultative mais recommandée) : le build d'audit de C1 lancé d'abord en mode relevé, qui consigne la valeur minimale vue à chaque cast sur tout le filet — tri empirique « vu négatif » / « jamais négatif » avant d'écrire les preuves. Les casts NON COUVERT restent « à auditer » jusqu'à test ou fixture (règle de B2).
- **Fraîcheur** : `make test` régénère l'inventaire dans `tmp/` et le compare à la copie versionnée ; un rapport périmé fait échouer la suite. Éventuellement, cliquet sur la garde : tout fichier du cœur absent de `float_guard.txt` doit encore compiler sous `-mgeneral-regs-only` (la liste ne peut que rétrécir). La couverture reste manuelle, citée par chaque PRD d'étape.

**Table de propriété** (tirée de `tools/float_inventory.txt` ; les comptes de casts viennent de ses lignes `compte etape`, pas d'un recopiage). Une ligne marquée *déduction* affecte un sous-groupe que le plan ne nomme pas mot pour mot : la phrase dont elle se déduit est citée, et l'humain peut la contester.

| Sous-groupe de l'inventaire | Jetons | Étape | Source |
|---|---|---|---|
| constantes `pico8_fixed_from_double(<littéral>)` | 61 | C2b | *Déduction.* Phase C : « les 34 constantes remplacées par les entiers de `tools/probe_out.txt` », et C1 garde les « constantes inchangées » : C2 est la seule étape de C qui touche aux constantes. Les 18 valeurs qui ne changent pas y sont remplacées sans divergence. |
| casts, étape C1 | 37 | C1 — ✅ faite | `compte etape C1` : (b) 36, et (a) à signe prouvé 1 (`pico8_gn_next`) ; convertis, voir « État de C1 » |
| casts, étape D1 | 22 | D1 | `compte etape D1` : (c) 21, et (a) à signe non prouvé 1 (`pico8_mz_kj`, `pico8_fixed_flr(ratio)`) |
| casts « à auditer » | 17 | aucune — **bloqués** | `compte etape a auditer` ; voir « Casts bloqués » ci-dessous |
| `%.4f`, et `double` + `pico8_fixed_to_double` de `pico8_value_tostr` | 3 | D2 | Phase D, `tostr` |
| `pow`, `pico8_fixed_from_double(ec_d)`, et `double` + 2 `pico8_fixed_to_double` de `pico8_dj` | 5 | D3 | Phase D, `^0.75` |
| revue des clés de table | 0 | D4 | *Déduction* (PRD de C0) : la conversion du jeton d'un cast-clé suit sa population, et le plan route (b) vers C1 sans exception ; D4 ne retire aucun jeton. D4 hérite de la liste des 28 casts dont la justification porte « cle de table » dans `tools/float_annotation.txt` (16 en C1, 1 en D1, 11 à auditer). |
| mots de commentaire : `float` (`pico8_fixed.c`), `double-free` (`pico8_bw`), `pico8_fixed_from_double` (`pico8_ob.c`, hors fonction), `double comma` (`pico8_ob_step`) | 4 | E | *Déduction* : le grep du §6.1, sortie de E, les voit. |
| `sin`/`cos` libc (2 réécrits), et `double` ×2, `pico8_fixed_to_double`, `pico8_fixed_from_double(…)` de chacune de `pico8_fixed_cos`/`_sin` (8 autres) | 10 | **sans propriétaire — décision ouverte** | §4/§5 contre §6/E ; [TROU-1](trous-de-portage.md) |
| définitions de `pico8_fixed_from_double`/`_to_double` (3 + 3 autres) | 6 | **sans propriétaire — décision ouverte** | idem |

Somme : 61 + 37 + 22 + 17 + 3 + 5 + 0 + 4 + 10 + 6 = **165**, le `compte total` de l'inventaire (61 constantes, 76 casts, 5 réécrits, 23 autres).

- **Ligne « décision ouverte »** — 16 jetons. Les deux options du plan (note de la Phase E) : ajuster le §6/E, ou lever la clôture du §5 pour ces sites. C0 la compte et ne la tranche pas ; elle reste à l'humain.
- **Titre de la Phase D.** Il annonce « les 5 sites flottants », c'est-à-dire les 5 réécrits de l'inventaire. D2 et D3 n'en possèdent que 3 (`%.4f`, `pow`, `from_double(ec_d)`) ; les 2 autres sont le `sin`/`cos` de la décision ouverte. Constat seulement : le titre n'est pas corrigé.
- **Casts bloqués** — les 17 « à auditer » n'appartiennent à aucune étape tant qu'ils ne sont pas classés, et C1 ne peut pas les prendre. Celle qui en convertira un — C1 s'il est reclassé (b) ou (a) à signe prouvé, D1 s'il est reclassé (c) ou (a) à signe non prouvé — doit d'abord l'atteindre, par la règle de B2 : un test unitaire au niveau valeur, une fixture scriptée qui l'atteint, ou une entrée explicite « non protégé par le filet » dans ce plan ; son annotation change dans le même commit. *Déduction* : D1 étant la dernière étape qui convertit des casts, c'est au plus tard à la sortie de D1 que les 17 doivent avoir été atteints et classés, faute de quoi E ne ferme pas. C0 ne fixe pas lequel des deux les prend.

**État de C0** (mesuré avec `cc` GCC 13.3, x86_64).

- **Fichiers.** `tools/float_annotation.txt` : une ligne par cast, clé (fichier, fonction, extrait, occurrence), population, étape, justification ; le seul des rapports flottants édité à la main. `tools/float_inventory.txt` le lit (`make float-inventory`). `tools/float_observe.txt` : `make float-observe`, build `-DPICO8_FLOAT_OBSERVE`, hors suite. `make test` lance `float-inventory-check`, qui régénère l'inventaire dans `tmp/` et échoue s'il diffère de la copie versionnée. Couverture et garde restent hors suite.
- **Populations** des 76 casts : (a) 2, (b) 36, (c) 21, à auditer 17. **Étapes** : C1 37, D1 22, à auditer 17.
- **Les 17 « à auditer »** : les 16 casts NON COUVERT de `tools/float_coverage.txt` (`pico8_ho_apply`, `pico8_hy_kt`, `set_by_value` de `economy_tiers`, `pico8_cr` ×2, `pico8_ha2_my_dispatch`, `pico8_jo_hu`, `pico8_ga_kj` ×5, `pico8_research_lookup`, `pico8_pj_ph_map` ×2, `pico8_jc_display_get_by_value`), plus `pico8_pj_hk` `yr`, couvert (8 passages) mais dont la preuve de signe est tombée. Plusieurs portent un candidat dans leur justification ; aucun candidat ne classe.
- **Débordement de `qc.t`.** `qc.t` croît de 1 par frame sans borne (`lua.lua:179`) et repasse à -32768 après 32768 frames (~9 min) ; l'état de placement `kz` n'a pas de délai. À t = -1, `oi` ≈ -1.48 (troncature -1, `flr` -2). Ce constat a réfuté deux preuves (b) de `pico8_mz_kj` (`oi`, `sr`), passées en (c)/D1, et retiré la même prémisse de `pico8_pj_hk` `yr` (à auditer : `yr` part de 3401, +1 par année, sans borne établie).
- **Observation** sur le filet (`FRAMES_EVENTS`), un indice, pas une preuve : 4 vus négatifs (`pico8_rn`, `pico8_bg_to_int`, `pico8_hud_to_int`, `pico8_bl`, tous (c)), 56 jamais négatifs, 16 NON COUVERT, 6 sur une ligne partagée, **0 contradiction** ((b) vu négatif). Total 76.

**État de C1** (mesuré avec `cc` GCC 13.3, x86_64).

- **Conversion.** Les 37 casts d'étape C1 appellent `pico8_fixed_flr_int` (module fixe, `flr` entier sans flottant) ; leurs lignes d'annotation sont retirées, et `tools/float_annotation.txt` n'a plus de ligne `| C1 |`.
- **Inventaire** (`tools/float_inventory.txt`) : constantes 61, casts 39 (76 → 39), réécrits 5, autres 23, **total 128** (165 → 128). Populations : (a) 1, (b) 0, (c) 21, à auditer 17. **Étapes** : C1 0, D1 22, à auditer 17. La table de propriété ci-dessus reste la photographie de C0 (165) ; sans la ligne C1, sa somme donne 128.
- **Porte 1, le filet** : `make frames-check` identique sur les 8 fichiers de `FRAMES_EVENTS` à chaque commit de conversion, sans `frames-baseline`.
- **Porte 2, l'audit** (`make float-audit`, hors suite, sans rapport versionné) : 37 sites, 37 atteints, 0 non atteint, 0 écart, 0 négatif, 0 ligne partagée, 1 413 641 passages. Limite du filet : certains sites ne sont atteints que 1 à 4 fois (`pico8_jo_gh` 1, `pico8_jo_pq` 2, `pico8_jo_gh_jp_step` 2, `pico8_db` 4).
- **Garde** (`tools/float_guard.txt`) : 32 → **24** fichiers en échec sur 58. Sortis : `pico8_economy_score.c`, `pico8_entity.c`, `pico8_gen.c`, `pico8_msgbox.c`, `pico8_research_display.c`, `pico8_research_icon.c`, `pico8_score_panel.c`, `pico8_tech_gate.c` — exactement la liste prévue par le PRD de C1.
- **Observation** (`tools/float_observe.txt`) : vu négatif 4, jamais négatif 19 (56 → 19), non couvert 16, ligne partagée 4 (6 → 4), **contradiction 0**, total 39.
- **Couverture** (`tools/float_coverage.txt`) : casts couverts 23, non couverts 16 ; total couverts 85, non couverts 43. Les passages de `pico8_fixed_to_double` tombent de 5 070 600 à 3 656 959 : l'écart, 1 413 641, est exactement le nombre de passages de l'audit.
- **Constat.** La copie versionnée de `tools/float_coverage.txt` datait d'avant l'annotation de C0 : tous ses casts y portaient « a auditer | - ». Régénérée, elle porte les populations et étapes de l'annotation (par exemple `pico8_rn`, `pico8_bg_to_int` en (c)/D1). La couverture n'a pas de contrôle de fraîcheur dans `make test`.

**Attention, la Phase C n'est plus intégralement préservatrice :** 16 des 34 constantes changent de valeur (§3.1), donc `frames.hash` **peut légitimement diverger** de la Phase B. La découpe est donc :

- **C1 — strictement préservatrice** : les sites (a) et (b) **dont le signe est prouvé ≥ 0**, constantes inchangées. Un site (a) pouvant recevoir un négatif change de comportement en passant de la troncature à `flr` — c'est une correction, pas une transformation neutre : il part en **D1** avec la population (c). `frames.hash` doit être **identique**. Toute divergence est un bug introduit. Un site atteint mais sans effet sur un pixel échappe au hash : C1 a donc une seconde porte, un **build d'audit** (sur le modèle de `-DPICO8_REF_AUDIT` / `make ref-audit`) qui asserte à chaque site (a)/(b) l'égalité de l'ancienne et de la nouvelle expression — pour (b), `x >= 0` — rejoué sur tout le filet. La preuve du signe devient une vérification exécutée, pas seulement consignée.
- **C2 — correction des 16 constantes**, en un commit séparé. Divergences attendues, à revoir frame par frame. Pour rattacher chaque frame divergente à **une** constante, chaque constante est d'abord appliquée **seule** (boucle scriptée de rejeux), et l'ensemble des frames divergentes est consigné par constante avant le commit. Deux étapes :
  - **C2a — l'outillage.** (i) Le harnais d'attribution : un répertoire de sortie par variante, aucun chemin fixe partagé (`tmp/pico8_replay_dump.txt` est réécrit à chaque rejeu), testé sur un cas fabriqué où une constante connue fait basculer une frame connue. (ii) Un **rapport de divergence** complet par fichier d'événements — première frame divergente, nombre, plages — écrit dans un fichier **avant** tout `frames-baseline`, qui sinon efface la seule trace (`frames_check.sh` n'en liste que 10). (iii) Un test unitaire, piloté par l'inventaire, qui asserte que chaque constante du C égale sa valeur hexa de `tools/probe_out.txt` : seule protection des constantes NON COUVERT par le filet, et réalisation de la promesse du §5 (le relevé devient une fixture).
  - **C2b — l'application** des 16 constantes, par le harnais de C2a.

Séparer C1 de C2 est ce qui garde le critère §6.4 falsifiable : mélangées, une régression de C1 se cacherait derrière une divergence légitime de C2.

**Sortie de phase :** `frames.hash` identique après C1 ; après C2, pour chaque fichier d'événements, la **première** frame divergente est rattachée à une constante précise du relevé, et la suite est étiquetée cascade (un changement d'état se propage à toutes les frames suivantes) ; le rapport de divergence est versionné avec le commit.

### Phase D — Population (c) et les 5 sites flottants
Quatre changements divergents indépendants, donc quatre étapes, chacune avec son re-baseline et son rapport de divergences — sans quoi la même confusion que C1/C2 se reproduirait : **D1** population (c), **D2** `tostr`, **D3** `^0.75`, **D4** clés de table.

Les 76 casts à signe potentiellement négatif passent en **`flr`** (bloc B — plus une question ouverte, une correction de bug sourcée). `pico8_value_tostr` réécrit en entier d'après le bloc C, en laissant le cas `< 0.0001` sur le comportement actuel **et documenté comme non sourcé**. `pico8_dj`'s `^0.75` réécrit en `sqrt(x) · sqrt(sqrt(x))`, validé contre les 16 vecteurs du bloc D. Les sites d'indexation de table revus contre la sémantique des clés fractionnaires (§3.2).
**Sortie de phase :** pour chaque étape D et chaque fichier d'événements, la première frame divergente est expliquée par une ligne du relevé, la suite étiquetée cascade (un changement d'affichage seul, D1, ne doit pas cascader), et le rapport de divergence de C2a est versionné avant le re-baseline ; `frames.hash` re-baseliné **volontairement**, avec revue des frames concernées — jamais subi comme une régression.

### Phase E — Fermeture
`-lm` retiré de la cible du cœur, garde `-mgeneral-regs-only` vide (§6.3), [TROU-1](trous-de-portage.md) refermé.

> **Décision ouverte, relevée en fin de Phase B (à trancher en C0, par l'humain) :** les 2 sites `sin`/`cos` libc de `pico8_fixed_cos`/`_sin` n'appartiennent à aucune étape — le §4 et le §5 interdisent de réécrire ces fonctions, tandis que le §6.1-6.3 et la Phase E exigent zéro flottant et le retrait de `-lm`, et TROU-1 porte précisément sur eux. Tels quels, les deux ne peuvent être satisfaits ensemble. Idem pour les définitions `pico8_fixed_from_double`/`to_double` elles-mêmes (catégorie « autre »). Ce plan ne choisit pas : ajuster le §6/E, ou lever la clôture du §5 pour ces sites.
> C0 l'a comptée sans la trancher : 16 jetons (10 pour `pico8_fixed_cos`/`_sin`, 6 pour les définitions), sur la ligne « sans propriétaire — décision ouverte » de la table de propriété (Phase C, C0).

**Sortie de phase :** les quatre points du §6 passent.

---

## 8. Volet `malloc` — cadrage NON FAIT

> **Cette section n'est pas un plan. C'est la liste de ce qui reste à trancher.** Elle est écrite maintenant pour que les questions soient posées avant, pas découvertes en cours de route ; les réponses ne sont pas inventées ici.

### Ce qui est déjà mesuré

102 sites d'allocation sur 41 fichiers. L'allocation n'est pas périphérique : c'est **la couche d'émulation Lua elle-même** — `pico8_value_t` (chaînes possédées via `pico8_strdup_owned`), `pico8_list_t`, `pico8_ob_table_t`, `pico8_class_t`/`pico8_instance_t`, `pico8_entity`. Chacune réimplémente indépendamment le même motif « tableau croissant à doublement », déjà signalé trois fois.

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
5. **La mutualisation.** Faut-il d'abord unifier le motif « tableau croissant » réimplémenté 4+ fois — ce que les garde-fous de `plan.md` réclament déjà indépendamment de ce chantier — avant d'y toucher, pour ne pas faire le travail quatre fois ?
6. **Le filet.** `frames.hash` suffit-il à prouver une transformation qui change les durées de vie ? Probablement pas seul : une corruption mémoire peut ne pas se voir sur une frame. Quel instrument complémentaire (ASan/Valgrind sur le build hôte, que ce chantier **ne doit pas** casser — d'où le §4, les outils gardent la libc) ?

---

## 9. Garde-fous

- **Traçabilité** (hérité de `docs/plan.md`) : aucune valeur numérique non sourcée. Une constante vient du relevé, jamais d'une règle d'arrondi supposée. Une sémantique d'arrondi vient du relevé ou d'un `flr` explicite du Lua, jamais d'un réflexe C. Un site de population (c) non résolu par le relevé est consigné comme risque ouvert, il n'est pas deviné. **Ne pas choisir est aussi un choix** : conserver la troncature actuelle n'est pas plus « neutre » que passer à `flr` — elle n'a pas davantage de source.
- **Le filet est capturé avant**, sinon la Phase C n'est qu'une promesse.
- **Jamais de fenêtre interactive lancée par l'agent** (hérité de `CLAUDE.md`). Ce chantier se vérifie entièrement en headless. L'humain n'intervient que deux fois : lancer la sonde (Phase A), enregistrer la session (Phase B).
- **Les outils gardent la libc** (§4). Les amputer détruirait le moyen de prouver le reste.
- **E/F capturés, pas exploités** (§5).
- **Constats structurels récurrents** : ce chantier hérite de la liste des garde-fous de `plan.md` — motif de tableau croissant, seuil de taille de module, sites de cast REF non gardés.

---

## 10. Risques

| Risque | Effet | Traitement |
|---|---|---|
| ~~Le relevé ne tranche pas la population (c)~~ | — | **Levé** : le bloc B tranche, c'est `flr` |
| ~~`printh()` n'écrit pas où attendu~~ | — | **Levé** : le fichier a été produit |
| `tostr` sous 0.0001 non sourcé | Un cas limite d'affichage reste deviné | Comportement actuel conservé **et documenté comme non sourcé** ; relance d'une ligne de la sonde quand ce sera utile |
| Clés de table fractionnaires (§3.2) | Le C trouverait une entrée là où Lua renvoie `nil` | À établir site par site pendant l'audit — « toujours entier par construction » doit être démontré, pas supposé |
| Phase C mélangeant (a)/(b) et les 16 constantes | Une régression se cacherait derrière une divergence légitime | Découpe C1/C2 en commits séparés (§7) |
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

Ajoutées après le relevé (`tools/probe_out.txt`, PICO-8 0.2.6b) :

11. PICO-8 **tronque** les littéraux décimaux : `floor(v × 65536)`, sans exception sur les 54 relevés. Les constantes du C sont recopiées du relevé.
12. **16 des 34 constantes du C sont décalées d'un LSB** — bug confirmé, corrigé en Phase C2, dans un commit séparé de C1.
13. `spr()` applique **`flr`** à ses coordonnées. Les 76 casts `(int)pico8_fixed_to_double` sont **faux pour tout négatif** — bug confirmé, corrigé en Phase D.
14. Les **clés de table fractionnaires sont des clés distinctes** en Lua/PICO-8 ; le C convertit avant la recherche. À établir site par site.
15. `pico8_value_tostr` est **déjà correct** sur 13 témoins sur 14 ; seul le cas de magnitude < 0.0001 reste non sourcé et est conservé tel quel, documenté comme tel.
16. `x^0.75` s'écrit `sqrt(x) · sqrt(sqrt(x))` en fixe pur — aucun `pow` à écrire, 16 vecteurs de test disponibles.
17. `rnd(0)` n'avance pas le PRNG, ce que le C fait déjà. Consigné, **non exploité** (bloc F).
