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
- **`pico8_fixed_rnd` n'est pas réécrit par ce chantier** (voir §5 bloc F, et §9 Risques). Son relevé est capturé et stocké ; son exploitation est un chantier distinct. **`sin`/`cos` : clôture levée par décision humaine (Phase D)**, voir D5.
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

> **Clôture de périmètre, à opposer à toute dérive :** F est **capturé et stocké**, pas exploité ; `pico8_fixed_rnd` n'est pas touché par ce chantier. *Levée partielle, décision humaine en Phase D :* `pico8_fixed_sin`/`cos` passent sur une table (D5), et les vecteurs `sin`/`cos` du bloc E en deviennent les points de contrôle.

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
   *Pourquoi pas `nm` :* sur l'hôte x86_64 le FPU est matériel, donc aucun symbole soft-float de `libgcc` (`__muldf3`, `__adddf3`…) n'est jamais émis — `nm` était **déjà vide avant tout changement** (mesuré : 0 symbole, alors que `pow`/`sin`/`cos` sont importés de la glibc). Un critère qui passe sur le code de départ ne prouve rien. *Décision humaine (Phase D) :* la vérification PSX (`nm` sur un build `mipsel` soft-float) est **hors périmètre** ; ce chantier reste sur du C pour PC, et §6.3 se juge sur la garde hôte seule, angle mort compris.
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
- **Angle mort de la garde.** Six fichiers compilent sous la garde alors qu'ils contiennent des constantes `pico8_fixed_from_double(<littéral>)` (20 au total) : `pico8_entity_geom.c`, `pico8_galaxy.c`, `pico8_game_data.c`, `pico8_geom.c`, `pico8_slipway.c`, `pico8_wormhole.c`. La liste de la garde n'est donc pas l'inventaire : la source de vérité sur les sites restants est `tools/float_inventory.txt`, et §6.3 ne prouve rien de plus sur cet hôte que les fichiers qu'il voit. La garde `mipsel` soft-float fermerait cet angle ; elle est hors périmètre (décision humaine, Phase D : on reste sur du C pour PC).

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
| constantes `pico8_fixed_from_double(<littéral>)` | 61 | C2b — ✅ faite | *Déduction.* Phase C : « les 34 constantes remplacées par les entiers de `tools/probe_out.txt` », et C1 garde les « constantes inchangées » : C2 est la seule étape de C qui touche aux constantes. Les 18 valeurs qui ne changent pas y sont remplacées sans divergence. Converties, voir « État de C2b ». |
| casts, étape C1 | 37 | C1 — ✅ faite | `compte etape C1` : (b) 36, et (a) à signe prouvé 1 (`pico8_gn_next`) ; convertis, voir « État de C1 » |
| casts, étape D1 | 22 | D1 | `compte etape D1` : (c) 21, et (a) à signe non prouvé 1 (`pico8_mz_kj`, `pico8_fixed_flr(ratio)`) |
| casts « à auditer » | 17 | D0b — ✅ faite | `compte etape a auditer` (17 en C0, 0 après D0b) : 10 convertis et ajoutés à la liste C1, 7 en D1 ; voir « Casts bloqués » ci-dessous et « État de D0b » |
| `%.4f`, et `double` + `pico8_fixed_to_double` de `pico8_value_tostr` | 3 | D2 | Phase D, `tostr` |
| `pow`, `pico8_fixed_from_double(ec_d)`, et `double` + 2 `pico8_fixed_to_double` de `pico8_dj` | 5 | D3 | Phase D, `^0.75` |
| revue des clés de table | 0 | D4 | *Déduction* (PRD de C0) : la conversion du jeton d'un cast-clé suit sa population, et le plan route (b) vers C1 sans exception ; D4 ne retire aucun jeton. D4 hérite de la liste des 28 casts dont la justification porte « cle de table » dans `tools/float_annotation.txt` (16 en C1, 1 en D1, 11 à auditer). |
| mots de commentaire : `float` (`pico8_fixed.c`), `double-free` (`pico8_bw`), `pico8_fixed_from_double` (`pico8_ob.c`, hors fonction), `double comma` (`pico8_ob_step`) | 4 | E | *Déduction* : le grep du §6.1, sortie de E, les voit. |
| `sin`/`cos` libc (2 réécrits), et `double` ×2, `pico8_fixed_to_double`, `pico8_fixed_from_double(…)` de chacune de `pico8_fixed_cos`/`_sin` (8 autres) | 10 | **D5** (décision humaine, Phase D) | [TROU-1](trous-de-portage.md) |
| définitions de `pico8_fixed_from_double`/`_to_double` (3 + 3 autres) | 6 | **E** (*déduction* : après D1–D3 et D5 le cœur ne les appelle plus ; §6.1 admet des helpers de test isolés) | idem |

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

**État de C2a** (mesuré avec `cc` GCC 13.3, x86_64). Aucune constante de `src/` n'a changé ; `make frames-check` est identique sur les 8 fichiers de `FRAMES_EVENTS`, sans `frames-baseline`.

- **(iii) Comparateur et contrôle.** `tools/constants_compare.sh` lit les jetons `constante` de `tools/float_inventory.txt` et le bloc A de `tools/probe_out.txt`, compile chaque littéral tel qu'écrit dans la source contre le vrai `pico8_fixed.c`, et compare les valeurs brutes. `make constants-check` (`tools/constants_check.sh`, dans `make test`) : **34 littéraux, 61 sites, 18 ok, 16 écart**. Les 16 écarts sont exactement ceux du §3.1, listés à la main dans `tools/constants_expected_ecarts.txt` ; le contrôle échoue si l'ensemble des écarts diffère de la liste (« périmée » ou « régression »), si un littéral manque au bloc A, si la liste nomme un littéral absent de l'inventaire, ou si aucune constante n'est vérifiée.
- **Contrat pour C2b.** Le commit qui corrige les constantes vide `tools/constants_expected_ecarts.txt` et rebranche le comparateur sur la forme retenue pour les constantes corrigées : les appels `pico8_fixed_from_double(<littéral>)` disparaissant, le contrôle échouerait sinon sur « zéro constante ». C2a ne choisit pas cette forme.
- **(ii) Rapport de divergence.** `tools/frames_report.sh` : par fichier d'événements, `conforme (N frames)`, `aucune reference`, ou première frame divergente, nombre, **toutes** les plages et écart de longueur ; il rend 0, c'est un rapport. `make frames-report ETAPE=<id>` l'écrit dans `tools/frames_reports/<id>.txt` (répertoire versionné, un fichier par étape, vide aujourd'hui hors `.gitkeep`). `make frames-baseline` refuse sans rien copier quand une référence diverge et qu'`ETAPE=` manque ; avec `ETAPE=`, il écrit le rapport puis copie. `tools/frames_check.sh` et `MAX_LISTED` sont inchangés.
- **Pilote de rejeu.** `src/main.c` accepte un troisième argument, le chemin du dump texte ; absent, le dump reste `tmp/pico8_replay_dump.txt`.
- **(i) Harnais d'attribution.** `make constant-variants [LITTERAL=<l>]` (`tools/constant_variants.sh`, hors suite) : une variante par littéral en `ECART` du comparateur, copie de `src/` sous `tmp/constant_variants/lit-<l>/` où seuls les appels `pico8_fixed_from_double(<l>)` reçoivent la valeur brute du relevé, nombre de remplacements vérifié contre l'inventaire, rejeu des 8 fichiers de `FRAMES_EVENTS` contre les références versionnées. Synthèse : `tmp/constant_variants/synthese.txt`, **non versionnée** (C2b décide de ce qu'il en consigne). Tests sur entrées fabriquées dans `make test` : `test_constants_check`, `test_frames_report`, `test_constant_variants`, et `test_pico8_headless_capture` pour le chemin de dump.
- **Passe réelle** (`make constant-variants`, environ 4 minutes) : 16 variantes, aucun échec de construction ni de rejeu, `src/` inchangé. Première frame divergente et nombre de frames divergentes, lus dans la synthèse (fichiers absents : 0 frame) :

  | Littéral | Sites | `events.json` | `phase4_playthrough` | `built_planet_panel` | `empty_planet_cleared` | `slipway_drag` | 3 autres |
  |---|---|---|---|---|---|---|---|
  | `0.1` | 5 : `pico8_bg_pq`, `pico8_bk`, `pico8_ma`, `pico8_iv_ph`, `pico8_mz_kz` | 457, 21 | 75, 4 | 75, 7 | 75, 1 | 77, 1 | 0 |
  | `0.05` | 1 : `pico8_bw` (`pico8_galaxy.c`) | 2373, 7302 | 0 | 0 | 0 | 0 | 0 |
  | `0.001` | 1 : `pico8_g__mm` (`pico8_probe.c`) | 291, 7631 | 41, 103 | 41, 150 | 39, 111 | 38, 106 | 0 |
  | les 13 autres | | 0 | 0 | 0 | 0 | 0 | 0 |

  Les « 3 autres » sont `shop_button_hover`, `screencast_capture` et `top_menubar`. `events.json` compte 9 729 frames.
- **13 des 16 constantes corrigées ne font diverger aucune frame du filet** : `0.0032`, `0.005`, `0.031`, `0.08`, `0.0977`, `0.19`, `0.27`, `0.55`, `0.6`, `0.8`, `0.85`, `0.99`, `1.08`. D'après `tools/float_coverage.txt`, 7 ont tous leurs sites couverts (`0.005`, `0.08`, `0.0977`, `0.19`, `0.55`, `0.8`, `0.99`), 2 en ont un sur deux (`0.0032`, `0.27`), 4 n'en ont aucun (`0.031`, `0.6`, `0.85`, `1.08`). Pour ces 13, le filet ne voit pas la correction, et `make constants-check` (niveau valeur, contre le relevé) est la seule protection : c'est le rôle que §7 C2a (iii) lui donne.
- **Constat.** Le site de `0.001` (`pico8_g__mm`, `pico8_probe.c`, rang 2) est `NON COUVERT` dans `tools/float_coverage.txt` (« ligne sans code », granularité à la ligne), alors que sa variante fait diverger les 5 fichiers de jeu. `NON COUVERT` ne prouve donc pas qu'un site est hors d'atteinte du filet.

**État de C2b** (mesuré avec `cc` GCC 13.3, x86_64).

- **Forme.** `PICO8_FIXED_LIT(raw, lua)` (`src/pico8_fixed.h`) : `raw` est l'hexa à 8 chiffres du bloc A de `tools/probe_out.txt`, point retiré ; `lua` est le littéral du cart, jamais évalué. Les 61 sites du cœur ont cette forme ; `pico8_fixed_from_double(` n'y sert plus qu'à `sin`/`cos` et à `ec_d` (`^0.75`). Les 34 valeurs viennent du relevé, pas d'une règle d'arrondi (§9).
- **Commit de forme, neutre deux fois** (`af4e53e`) : table du comparateur identique (`litteraux 34 | sites 61 | ok 18 | ecart 16`), liste des défaillances attendues inchangée, et `make frames-check` identique sur les 8 fichiers de `FRAMES_EVENTS`, sans `frames-baseline`.
- **Comparateur.** `tools/constants_compare.sh` reçoit en plus `CORE_SRCS` et lit les sites de macro dans les `.c` du cœur ; il compile chaque appel tel qu'écrit, contre le vrai en-tête. Après le commit de valeurs (`2b1151d`), `make constants-check` imprime **`litteraux 34 | sites 61 | ok 34 | ecart 0`**, et `tools/constants_expected_ecarts.txt` ne garde que son en-tête (`# TROU-1` compris). Les 16 corrections touchent 22 sites.
- **Attribution** (`tools/frames_reports/C2b-variantes.txt`, synthèse de `make constant-variants` sur l'arbre d'avant correction, `c6f16a4`, aucun chemin sous `tmp/` ; mêmes chiffres que la table de l'« État de C2a »). Règle : F = première frame divergente de `tools/frames_reports/C2b.txt`, rattachée à L si exactement une variante a F pour première frame et aucune n'a moins. Détail dans `tools/frames_reports/C2b-attribution.txt`.

  | Fichier | F | Constante | Frames divergentes (combiné) | `0.001` seule | `0.1` seule | `0.05` seule |
  |---|---|---|---|---|---|---|
  | `events.json` | 291 | `0.001` | 9 174 | 7 631 | 21 | 7 302 |
  | `phase4_playthrough` | 41 | `0.001` | 107 | 103 | 4 | 0 |
  | `built_planet_panel` | 41 | `0.001` | 155 | 150 | 7 | 0 |
  | `empty_planet_cleared` | 39 | `0.001` | 112 | 111 | 1 | 0 |
  | `slipway_drag` | 38 | `0.001` | 107 | 106 | 1 | 0 |

  Toutes les frames après F sont **cascade** ; les colonnes par variante sont données pour information. `shop_button_hover`, `screencast_capture` et `top_menubar` restent conformes. La règle n'a échoué sur aucun fichier : aucune interaction entre constantes à la première frame.
- **Revue sans fenêtre**, à F, par `pico8_frames frame` (capture d'avant, d'après, et variante `lit-0.001`) : sur les 5 fichiers, « après » est identique octet pour octet à la variante. Écart « avant »/« après » : `events.json` 2 lignes sur 128, 2 pixels, lignes 64–65, colonne 49 ; `phase4_playthrough`, `built_planet_panel`, `empty_planet_cleared` 1 ligne, 1 pixel, ligne 12, colonne 109 ; `slipway_drag` 1 ligne, 1 pixel, ligne 10, colonne 104. Aucun jugement de justesse du pixel : la justesse vient du relevé, vérifiée au niveau valeur.
- **Re-baseline** `make frames-baseline ETAPE=C2b` dans le commit de valeurs : `tools/frames_reports/C2b.txt` versionné, 5 fichiers d'empreintes réécrits. `events.json.sha256` n'existe pas ; `screencast-sign` n'a pas été lancé.
- **13 des 16 corrections ne sont vues par aucune frame du filet** (`0.0032`, `0.005`, `0.031`, `0.08`, `0.0977`, `0.19`, `0.27`, `0.55`, `0.6`, `0.8`, `0.85`, `0.99`, `1.08`) : `make constants-check` est leur seule protection.
- **Inventaire** (`tools/float_inventory.txt`) : constantes 0 (61 → 0), casts 39, réécrits 5, autres 23, **total 67** (128 → 67).
- **Couverture** (`tools/float_coverage.txt`) : constantes couverts 0, non couverts 0 (38/23 → 0/0) ; casts 23/16, réécrits 5/0, autres 19/4 inchangés ; total couverts 47, non couverts 20 (85/43 → 47/20). Les passages de `pico8_fixed_from_double` tombent de 1 191 880 à 311 652, soit exactement `pico8_fixed_cos` 155 071 + `pico8_fixed_sin` 156 558 + `pico8_dj` 23. Les autres passages bougent parce que 5 rejeux divergent.
- **Garde** (`tools/float_guard.txt`) : **24 → 24** fichiers en échec sur 58, liste identique. C'est l'angle mort de B2 : un argument `double` passé à `pico8_fixed_from_double` ne faisait échouer aucun fichier sous `-mgeneral-regs-only` sur x86_64, donc retirer ces appels n'en fait sortir aucun.
- **Observation** (`tools/float_observe.txt`) : vu négatif 4, jamais négatif 19, non couvert 16, ligne partagée 4, contradiction 0, total 39, comptes inchangés. Les passages et certains minimums par site bougent, pour la même raison que la couverture.
- **Limite connue, non corrigée : le harnais d'attribution est gelé sur l'ancienne forme.** Il appelle le comparateur sans `CORE_SRCS`, donc ne voit plus de constante. Mesuré après C2b : `make constant-variants` échoue (« aucune constante dans tools/float_inventory.txt ni dans les sources », code 2), au lieu de ne rien sélectionner. Son test sur arbre fabriqué reste vert.
- **TROU-1** reste ouvert : seuls les littéraux décimaux sont maintenant bit-exacts ; `sin`/`cos` et `^0.75` ne le sont pas, et `pico8_fixed_from_double` reste défini (décision ouverte).

**Sortie de phase :** `frames.hash` identique après C1 ; après C2, pour chaque fichier d'événements, la **première** frame divergente est rattachée à une constante précise du relevé, et la suite est étiquetée cascade (un changement d'état se propage à toutes les frames suivantes) ; le rapport de divergence est versionné avec le commit.

**État de sortie de la Phase C** (d'après les états C0, C1, C2a et C2b).

- *`frames.hash` identique après C1* : oui, `make frames-check` identique sur les 8 fichiers à chaque commit de conversion de C1, sans `frames-baseline` (« État de C1 », porte 1). C2a et le commit de forme de C2b l'ont aussi laissé identique.
- *Après C2, première frame divergente rattachée à une constante du relevé, suite étiquetée cascade* : oui, 5 fichiers divergents, tous rattachés à `0.001` (frames 291, 41, 41, 39, 38), le reste en cascade ; 3 fichiers conformes (« État de C2b »).
- *Rapport de divergence versionné avec le commit* : oui, `tools/frames_reports/C2b.txt` dans le commit de valeurs, à côté de `C2b-variantes.txt` et `C2b-attribution.txt`.
- *Comptes d'inventaire* (règle de B2) : constantes 0, population (b) 0, étape C1 0. Restent 67 jetons : casts 39 ((a) 1 et (c) 21 en D1, 17 à auditer), réécrits 5, autres 23, tous affectés par la table de propriété à D1–D3, E ou à la décision ouverte.
- *Garde* : 24 fichiers sur 58 en échec, liste inchangée depuis C1.

### Phase D — Population (c) et les 5 sites flottants
Périmètre, d'après la table de propriété (Phase C) : **22 casts D1** (21 (c) + 1 (a) de signe non prouvé), **les 17 casts « à auditer »** (dont 11 clés de table), **3 jetons D2** (`tostr`), **5 jetons D3** (`^0.75`), et la revue des clés de table (D4). Les 2 sites `sin`/`cos` du titre sont en **D5** (décision humaine). La sortie de chaque étape s'énonce en « compte d'étape = 0 » dans l'inventaire annoté.

Sources, inchangées : population (c) → **`flr`** (bloc B, correction de bug sourcée) ; `pico8_value_tostr` réécrit en entier d'après le bloc C, le cas `< 0.0001` gardé sur le comportement actuel **et documenté comme non sourcé** ; `^0.75` → `sqrt(x) · sqrt(sqrt(x))`, validé contre les 16 vecteurs du bloc D ; clés de table revues contre la sémantique des clés fractionnaires (§3.2).

Étapes, dans l'ordre :

- **D0a — Outillage de D** — ✅ faite, voir « État de D0a ». (i) Le build d'audit de C1 devient un build avant/après pour chaque étape D : par site modifié, nombre d'appels, appels où ancien ≠ nouveau, appels à argument fractionnaire, et **frame du premier ancien ≠ nouveau**. Même motif pour D2 (ancien `%.4f` contre nouveau) et D3 (ancien `pow` contre nouveau). C'est l'outil d'attribution de D — `constant-variants` ne remplace que des littéraux et n'a plus rien à remplacer. (ii) La garde de C1 est préservée : les 37 sites C1 sont figés dans une liste versionnée, la vérification se fait **par classe de site** (C1 : aucun négatif et ancien == nouveau ; D1 : nouveau == `flr`), et `float-audit` entre dans `make test`.
- **D0b — Les 17 casts bloqués** — ✅ faite, voir « État de D0b ». Sortie : « à auditer » = 0. Chacun est atteint par une fixture scriptée ou un test au niveau valeur, ou reçoit une entrée explicite « non protégé par le filet », puis est classé ; la revue de clé de ses 11 sites clé de table se fait ici, appuyée par le compte d'arguments fractionnaires de D0a. Un cast reclassé (b) est converti comme en C1 (et rejoint la liste figée) ; un cast reclassé (c) rejoint D1, dont le périmètre est ainsi fixé **avant** D1.
- ~~**D0c**~~ — sans objet : (i) la vérification PSX est hors périmètre (décision humaine : on reste sur du C pour PC) ; (ii) la décision ouverte est tranchée (D5), la mesure n'a plus à la chiffrer.
- **D1 — population (c).** Divergente, avec re-baseline.
- **D2 — `tostr`.** **Préservatrice, pas divergente** : le `tostr` actuel reproduit déjà 13 témoins sur 14 et le 14ᵉ est conservé tel quel. Porte : (i) un test d'hôte qui compare ancien et nouveau sur les 65 536 parties fractionnaires, des deux signes, et quelques parties entières dont ±32767/−32768 — seule différence admise, le cas `< 0.0001` ; attention aux 16 égalités exactes où `%.4f` arrondit au pair ; (ii) `frames-check` identique, **sans** `frames-baseline`.
- **D3 — `^0.75`.** Divergente si le nouveau diffère de l'ancien. Constat fait en revue de fin de Phase C : `pico8_fixed_mul(s, pico8_fixed_sqrt(s))` avec `s = pico8_fixed_sqrt(x)` reproduit les 16 vecteurs du bloc D au bit près, et `pico8_fixed_sqrt` les 16 vecteurs `sqrt` du bloc E.
- **D5 — `sin`/`cos` sur table** (décision humaine). `pico8_fixed_sin`/`cos` lisent une table de valeurs 16.16 indexée par la fraction de tour, sans `sin()`/`cos()` libc. Modèle donné par l'humain : le portage de Celeste Classic sur GBA (JeffRuLz/Celeste-Classic-GBA, `src/sine.h` + `main.c:331-332` : 360 entrées `float`, index `(u16)(x*360) % 360`, sans interpolation) — le principe est repris, pas ses `float`. Porte : les 64 vecteurs `sin(i/64)`/`cos(i/64)` du bloc E reproduits **au bit près** ; hors de ces points, la table est une approximation **non sourcée**, documentée comme telle dans TROU-1 (taille, construction, interpolation éventuelle : à trancher par la PRD de l'étape). Divergente, avec re-baseline et attribution par l'audit avant/après (paire ancien libc / nouvelle table).
- **D4 — clés de table.** Réduite aux clés déjà converties (16 en C1, 1 en D1) ; lue principalement dans le compte d'arguments fractionnaires de D0a (un site sans argument fractionnaire sur tout le filet est entier de fait, pas seulement par argument).

**État de D0a** (mesuré avec `cc` GCC 13.3, x86_64, 4 cœurs). Aucune ligne du cœur n'a changé ; `make frames-check` est identique sur les 8 fichiers de `FRAMES_EVENTS`, sans `frames-baseline` ; `float-inventory-check` et `constants-check` inchangés.

- **Outils.** `tools/float_audit_c1.txt` : la liste C1 figée, une ligne par site, clé (fichier, fonction, extrait, occurrence) comme l'annotation ; produite depuis les sources (37 lignes, répartition par fichier égale à celle de l'« État de C1 »), éditée à la main ensuite. `src/pico8_float_audit.c` (build `-DPICO8_FLOAT_AUDIT` seulement) : point d'entrée générique `pico8_float_audit_record(fichier, fonction, ligne, argument 16.16, ancien ≠ nouveau, nouveau ≠ référence)`, et la seule paire livrée, celle de `pico8_fixed_flr_int` (ancien `(int)pico8_fixed_to_double(x)`, référence `floor()` de la libc). D2 et D3 branchent leur propre paire sur le même point d'entrée. `tools/float_audit.sh` : le rapport, testé par `tests/test_float_audit.c` sur sources et enregistrements fabriqués.
- **Format d'enregistrement**, une ligne par (fichier, ligne) et par fichier d'événements (`tmp/audit/<events>.aud`, effacés avant la passe) : `AUD`, fichier, fonction, ligne, appels, négatifs, ancien ≠ nouveau, premier argument fautif en hexa brut ou `-`, fractionnaires (16 bits bas non nuls), nouveau ≠ référence, frame du premier ancien ≠ nouveau ou `-`. La frame est `current_frame` de l'entrée du contexte de dispatch : numérotation de `frames_check.sh`/`frames_report.sh`, 0 pendant l'initialisation, −1 si aucune entrée n'est posée. Un enregistrement à l'ancien format fait échouer le rapport (code 6).
- **Règles par classe.** C1 = clé dans la liste ; D sinon. C1 échoue sur `ECART` (ancien ≠ nouveau), `NEGATIF` et `NON ATTEINT` — la porte de C1, inchangée. D échoue sur `NOUVEAU != FLR` seulement ; négatifs, fractionnaires et ancien ≠ nouveau y sont des mesures, et zéro ancien ≠ nouveau (non atteint compris) sort « correction non vue par le filet », sans échec. Une entrée de liste sans site ou une clé dupliquée échoue (code 5). Par fichier d'événements, le rapport donne la plus petite frame de premier ancien ≠ nouveau et **tous** les sites qui l'atteignent : l'entrée de la règle d'attribution.
- **Sortie réelle** (`make float-audit`) : **37 sites C1, 0 site D**, 37 atteints, 0 non atteint, 0 écart, 0 négatif, 0 nouveau ≠ `flr`, 0 correction non vue, 0 erreur de liste, 0 ligne partagée ; « premier ancien ≠ nouveau : aucun » pour les 8 fichiers ; **1 444 884 passages**. L'« État de C1 » en comptait 1 413 641 : il était mesuré avant C2b, dont les corrections de constantes font diverger 5 des 8 rejeux ; les chemins parcourus changent, et les passages avec eux (+31 243). Les sites C1 restent tous vérifiés, sur les rejeux d'aujourd'hui.
- **Fractionnaires aux sites C1** : un seul site, `pico8_jo_kj` ligne 576 (`radius_f`, `src/pico8_planet_display.c`), **103 appels, 103 fractionnaires**, tous positifs et sans écart. Cela confirme l'« État de C1 ». Les 36 autres sites C1 n'ont vu aucun argument fractionnaire sur le filet.
- **Lignes partagées.** Le PRD de D0a annonçait que `pico8_qb_md` (`cp.x`, `cp.y`, `src/pico8_scroll.c` ligne 139) et `pico8_mz_kj` (`cp.x`, `cp.y`, `src/pico8_wormhole_display.c` ligne 87) partageraient chacun une ligne dans l'audit. Ce sont des casts D1 **pas encore convertis** : ils n'appellent pas `pico8_fixed_flr_int`, et l'audit d'aujourd'hui ne les voit pas (0 ligne partagée). Ils apparaîtront quand D1 les convertira ; alors, pour chacune de ces deux lignes, la frame du premier écart désignera la ligne, pas l'appel.
- **Coût dans `make test`** (`float-audit` en est la dernière cible). `make test` passe de **20,1 s à 33,2 s** (arbres d'objets à jour, deux passes identiques), soit **+13,1 s**. `make float-audit` seul : **13,2 s** à jour, **15,7 s** en recompilant l'arbre `tmp/obj-audit/` ; `make test` avec cet arbre effacé : 35,4 s. Dans l'ordre de la demande (environ 15 s) ; la cible reste dans la suite.

**État de D0b** (mesuré avec `cc` GCC 13.3, x86_64, 4 cœurs). Aucune référence `frames.hash` existante n'a changé ; celles des 4 nouvelles fixtures ont été posées sur le code d'avant conversion, chacune dans son commit d'ajout. `float-inventory-check` et `constants-check` sont verts, et `make test` aussi.

- **Fixtures.** Quatre fichiers scriptés entrent dans `FRAMES_EVENTS`, qui en compte maintenant **12** : `tech_screen_invent` (58 frames, G1), `game_end` (74 frames, G2), `pause_menu_music` (32 frames, G3) et `geoharvest_burst` (293 frames, G4). Cela fait 457 frames de plus. Chacune a son test de scénario dans `make unit`. G5 n'a pas de fixture : voir « Non protégés par le filet » ci-dessous.
- **Sort des 17**, par groupe :

  | Groupe | Site(s) | Population → étape | Atteinte | Clé de table |
  |---|---|---|---|---|
  | G1 | `pico8_hy_kt` `ew`, `pico8_cr` `tl` ×2 | (b) → C1, convertis | `tech_screen_invent` | `ek[…]` entière par construction : `ew` vaut 1 à la création (lua.lua:1200), puis seulement `+=1` sous `ew<4` (1449-1451) ; 0 fractionnaire à l'audit |
  | G2 | `pico8_pj_hk` `yr` | (b) → C1, converti | `game_end` | `gs[3426-yr]` entière par construction : `yr` part de 3401 (1202), ne fait que `+=1` (1213). Signe : à 3426, l'overlay `fq` de `pj:ph` intercepte tout clic avant `ix[1]` (324-327, 870-871), donc `yr <= 3426` ; 0 fractionnaire |
  | G2 | `pico8_pj_ph_map` `score_val` | (b) → C1, converti | `game_end` | entière par construction : littéraux 0..6 des lignes `bz` (803-814) ; 0 fractionnaire |
  | G2 | `pico8_pj_ph_map` `s6_val` | (a) → D1, signe non prouvé | `game_end` | pas une clé (comparaison `i<=s[6]`) ; `s[6]=min(flr(total/2000),5)` (1361), mais `total` peut être négatif via `happiness` |
  | G3 | `pico8_ha2_my_dispatch` `s` | (c) → D1 | `pause_menu_music` | pas une clé ; `music(-sgn(stat(20)))` (993) : négatif possible |
  | G4 | `pico8_ga_kj` ×5 : `p.x`, `p.y`, `flr(pt.l/2)`, couleur `k_[…]`, rayon `qx()` | (b) ×4 et (a) à signe prouvé ×1 → C1, convertis | `geoharvest_burst` | `k_[flr(pt.l/2)]` entière par construction (`flr`, 1091) ; 0 fractionnaire sur 251 appels |
  | G5 | `pico8_ho_apply`, `set_by_value` (`economy_tiers`), `pico8_jc_display_get_by_value`, `pico8_research_lookup`, `pico8_jo_hu` | (c) → D1 | **non protégés par le filet** | clé chaîne par construction, sources ci-dessous |

  Soit **10 convertis** (G1 3, G2 2, G4 5) et **7 en D1** (G2 `s6`, G3, G5 5). Aucune clé des 11 sites clé de table n'est fractionnaire possible : les 6 converties sont entières par construction, et les 5 de G5 sont des chaînes. Aucun trou de portage n'est donc ouvert pour une clé.
- **Inventaire** (`tools/float_inventory.txt`) : constantes 0, casts **29** (39 → 29), réécrits 5, autres 23, **total 57**. Populations : (a) 2, (b) 0, (c) 27, **à auditer 0**. **Étapes** : C1 0, **D1 29** (22 + 7), **à auditer 0**.
- **Audit** (`make float-audit`) : **47 sites C1** (37 + 10), 0 site D, 47 atteints, 0 non atteint, 0 écart, 0 négatif, 0 nouveau ≠ `flr`, 0 correction non vue, 0 erreur de liste, 0 ligne partagée. « premier ancien ≠ nouveau : aucun » pour les 12 fichiers ; **1 515 540 passages** (1 444 884 en D0a). Les sites convertis sont peu atteints : `pico8_hy_kt` 4 appels, `pico8_cr` 4 et 4, `pico8_pj_hk` 61, `pico8_pj_ph_map` 7. Fractionnaires aux convertis : 0 aux 6 clés et à la couleur de `pico8_ga_kj`. En dehors des clés, `p.x` et `p.y` de `pico8_ga_kj` en ont 84 sur 84 et le rayon 250 sur 251, tous positifs et sans écart.
- **Couverture** (`tools/float_coverage.txt`) : casts couverts **24**, non couverts **5**, qui sont exactement les 5 de G5 ; total couverts 48, non couverts 9.
- **Observation** (`tools/float_observe.txt`) : vu négatif 4, jamais négatif 20, non couvert 5, ligne partagée 4, **contradiction 0**, total 29. Les deux D1 atteints de D0b ne sont jamais vus négatifs, ce qui reste un indice : `ha2` `s` minimum `0x00010000` (1 passage), parce que le port rend 0 pour `stat(20)` ([TROU-10](trous-de-portage.md)) ; `s6_val` minimum `0x00000000` (5 passages).
- **Garde** (`tools/float_guard.txt`, `make float-guard`, en échec comme prévu) : **24 → 22** fichiers en échec sur 58. Sortent `pico8_economy.c` et `pico8_planet_family.c`. `pico8_menu.c` et `pico8_score_panel_display.c` y restent : ils gardent un cast D1 (`ha2` `s`, `s6_val`).
- **Coût dans `make test`** : **33,6 s et 34,2 s** (arbres d'objets à jour, deux passes), contre 33,2 s à l'« État de D0a », soit environ +1 s. `make float-audit` seul : 13,7 s (13,2 s en D0a).
- **Trouvé en chemin, hors plan** : un plantage du panneau de score. `de.my` rendait nil là où le cart rend `true` (lua.lua:901-903), et le clic qui referme le panneau ouvert par `ix[3]` faisait avorter le port dans `pico8_dg_truthy`. C'est corrigé dans `fe6473d`, avec le test de scénario `score_panel_close`, et aucune empreinte du filet n'a bougé. La fixture `game_end` avait d'abord pris le chemin par `ix[3]`, puis l'a abandonné.
- **Écart connu, à aligner plus tard : le chemin des fixtures.** Le pilote de rejeu (`frames-check`, `float-audit`, couverture, observation) part de `pico8_eu` et saute l'écran titre. Les tests de scénario de `tech_screen_invent`, `game_end` et `pause_menu_music` partent de l'écran titre (`pico8__init`, choix de la difficulté à la frame 10). Celui de `geoharvest_burst` part de `pico8_eu` comme le pilote. Sous le pilote, le filet atteint bien les mêmes états : les enregistrements d'audit de chaque fichier montrent son site (`pico8_hy_kt` et `pico8_cr` dans `tech_screen_invent`, `pico8_pj_hk` et `pico8_pj_ph_map` dans `game_end`, `pico8_ga_kj` dans `geoharvest_burst`), et l'observation voit `ha2` `s` une fois. Mais les trois tests assertent ces états par un autre chemin que celui du filet.
- **Deux écarts de rendu probables dans `pico8_ga_kj`**, trouvés en le convertissant et consignés en trous de portage, sans correction ici : `k_[0]` nil donne la plume courante en Lua et la couleur 0 en C ([TROU-20](trous-de-portage.md), atteint par le filet) ; `k_` est lu dans les valeurs par défaut de la classe au lieu de `self`, donc un `ga` né de `pm:pq()` prend la mauvaise palette ([TROU-21](trous-de-portage.md), non atteint).

**Non protégés par le filet (D0b, G5).** Cinq casts de dispatch par type de clé ont une branche « clé nombre » qu'aucune fixture ne peut prendre : avec les données du cart, la clé est toujours une chaîne ou `nil`. Ils sont classés (c)/D1 : une preuve de signe sur une valeur qui n'arrive jamais n'est pas exécutable. La branche reste en place, c'est le contrat de la fonction.

- `pico8_ho_apply` `raw` (`r[ri[i]]`, lua.lua:17-19). Seuls les champs listés dans `gp` passent par `r`. `df` n'a pas de `gp` (lua.lua:1389), donc `gu` est faux. Pour `hp`, `gp=o("o","dm")` (lua.lua:1401) : dans le `fd` (unstash 8494), les positions 6 et 8 sont des chaînes ou absentes (le `f` nu, `false`, n'y apparaît qu'en position 7). Pour `nb`, `gp={"nk"}` (lua.lua:1443) : la position 3 (unstash 10457) vaut `"t"`, `"l"`, `"o"`, `"b"` ou `"a"`. Clé : `r` (unstash 8192) n'a que des clés chaîne. Une clé nombre y manquerait en Lua comme en C.
- `set_by_value` (`w.hg[e.lp[i+1].id]`, lua.lua:1271) et `pico8_jc_display_get_by_value` (`self.hg[t.id]`, lua.lua:657). Le cart ne crée que trois sortes d'`id`, toutes des chaînes : `"p"..id` (lua.lua:749), `"s"..ie.id` (lua.lua:934), et l'`id` hérité par `jo:hu` (lua.lua:454). Le `re` provisoire d'un tracé, `{ra=mp,p=mp/128}` (lua.lua:622), n'a pas d'`id` (`nil`). Clé chaîne par construction.
- `pico8_research_lookup` (`hc[self.ja]`, `hc[qp.mc]`, lua.lua:1422-1423). `ja` est la position 4 d'une entrée du `fd` (unstash 8494). `mc` est un champ nommé de `hc` (unstash 8383). Les deux sont des chaînes ou `nil`, et `hc` n'a aucune clé nombre. Clé chaîne par construction.
- `pico8_jo_hu` `go` (`df[hp.go]`, lua.lua:456). `hp` vient de `gb` (lua.lua:418-420, 1434-1440). `go` ne vaut que `"f"` ou `"e"` dans le `fd` (unstash 8494). `df` a des clés entières 1..11, mais aucun `go` n'est un nombre. Clé chaîne par construction.

**Règle d'attribution d'une étape D :** la première frame divergente F de `tools/frames_reports/<étape>.txt` doit égaler la plus petite « frame du premier ancien ≠ nouveau » parmi les sites de l'étape ; ce site est désigné. Même règle qu'en C2b, lue dans un seul rejeu.

**Corrections invisibles au filet :** pour chaque étape D, les sites à zéro appel ancien ≠ nouveau sur le filet sont listés « correction non vue par le filet » (comme les 13 constantes de C2b). Chacun reçoit un test au niveau valeur de l'expression convertie (par exemple une entrée négative fractionnaire qui doit donner `flr`) ou une entrée explicite dans ce plan. Critère de couverture, pas de ligne `gcov` : C a montré que NON COUVERT ne prouve rien dans un sens comme dans l'autre. Structurel pour D1 : `oi`, `sr` et `flr(ratio)` ne sont (c) que par le repli de `qc.t` à 32 768 frames, alors que le plus long fichier du filet en fait 9 729.

**Sortie de phase :** pour chaque étape D divergente et chaque fichier d'événements, la première frame divergente est attribuée à un site par la règle ci-dessus, la suite étiquetée cascade (un changement d'affichage seul, D1, ne doit pas cascader), et le rapport de divergence est versionné avant le re-baseline ; `frames.hash` re-baseliné **volontairement**, avec revue des frames concernées — jamais subi comme une régression. D2 ne re-baseline pas.

### Phase E — Fermeture
`-lm` retiré de la cible du cœur, garde `-mgeneral-regs-only` vide (§6.3), [TROU-1](trous-de-portage.md) refermé.

> **Décision tranchée par l'humain en Phase D :** `sin`/`cos` sur table (D5) ; la vérification PSX hors périmètre. Historique de la question :
>
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
