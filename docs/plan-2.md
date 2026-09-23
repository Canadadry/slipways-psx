# Plan 2 — Suppression des dépendances flottant et `malloc`

Document distinct de [`docs/plan.md`](plan.md), qui reste le plan du **portage** (Phases 1 à 5, cible PC/raylib). Celui-ci couvre un chantier différent, postérieur, sur la même base de code : rendre le cœur porté indépendant de la virgule flottante puis de l'allocation dynamique.

Ce document est **le plan** ; aucune implémentation n'est engagée par sa rédaction.

**État : le volet flottant (§7) est terminé** — les quatre points du §6 passent, voir « Sortie de phase E ». Le volet `malloc` (§8) n'est pas cadré.

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

Les numéros de ligne de cette section datent du cadrage et dérivent ; la liste à jour était `tools/float_inventory.txt` (`make float-inventory`, Phase B2), retiré depuis (« Après la Phase E — outils retirés », §7).

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

Aucun de ces points ne demande de jugement ni de relecture. Les points 1, 2 et 3 sont tenus par `make test` ; le point 4, propriété de l'historique git et non de l'arbre, par la cible explicite `make frames-ledger`.

1. **`make float-grep`** → aucune ligne. Le grep `\b(double|float)\b` porte sur le **cœur** (§4) : les `CORE_SRCS` et les en-têtes qu'ils incluent par défaut (calculés par `cc -MM`, sans les définitions `PICO8_FLOAT_*`). Le reste de `src/` (points d'entrée, outils d'hôte, backend raylib, visualiseur, builds d'audit, aide de test `pico8_fixed_test_double.{h,c}`) est exclu par `tools/float_grep_exclusions.txt`, une justification par ligne ; un fichier du cœur qui y figurerait fait échouer la cible.
2. **La cible de lien du cœur est `$(TMP_DIR)/pico8_replay`** (sources `REPLAY_SRCS`) : elle est liée sans `-lm`, et `make test` la construit. `make float-link` refait ce lien dans un binaire jetable et liste les symboles non résolus ; c'est un diagnostic, hors suite.
3. **`make float-guard`** → liste vide : les objets du cœur (`CORE_SRCS`) **compilent avec `-mgeneral-regs-only`** sur l'hôte (erreur du compilateur sur tout flottant à l'exécution, constantes repliées tolérées). C'est une **garde hôte** : elle ne prouve pas l'objectif PSX à elle seule.
   *Angle mort sur x86_64, mesuré en B2 :* la garde rattrape les retours et opérations `double`, pas un argument `double` passé à une fonction externe — GCC le passe sur la pile sans erreur. Il est fermé depuis E1 par un autre moyen : le cœur n'a plus aucun prototype flottant, donc un tel appel y est une déclaration implicite, erreur sous `-Werror` (« État de E1 »).
   *Pourquoi pas `nm` :* sur l'hôte x86_64 le FPU est matériel, donc aucun symbole soft-float de `libgcc` n'y est émis ; `nm` était déjà vide avant tout changement et ne prouve rien. *Décision humaine (Phase D) :* la vérification PSX (`nm` sur un build `mipsel` soft-float) est **hors périmètre** ; ce chantier reste sur du C pour PC, et §6.3 se juge sur la garde hôte seule.
4. **`make frames-ledger`** → 0 erreur : toute divergence de `*.frames.hash` depuis la base `30dfd6c` est **justifiée par une ligne du relevé de sonde**, citée dans `tools/frames_ledger.txt`, et a son rapport et son attribution dans l'arbre de son commit ; un commit qui ne fait que poser des références n'en réécrit aucune. La cible exige un clone qui atteint la base (sinon elle rend 2 et nomme le remède) : elle est lancée à la clôture d'E4 et à chaque re-baseline, pas par `make test`.

Le point 4 est ce qui empêche ce chantier de devenir une réécriture à l'aveugle : il sépare mécaniquement « j'ai transformé » de « j'ai changé ».

---

## 7. Phases — volet flottant — ✅ TERMINÉ

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
| casts, étape D1 | 22 | D1 — ✅ faite | `compte etape D1` : (c) 21, et (a) à signe non prouvé 1 (`pico8_mz_kj`, `pico8_fixed_flr(ratio)`) ; D1 a converti **29** casts (ces 22 de C0 et 7 venus de D0b), `compte etape D1` 29 → 0, voir « État de D1 » |
| casts « à auditer » | 17 | D0b — ✅ faite | `compte etape a auditer` (17 en C0, 0 après D0b) : 10 convertis et ajoutés à la liste C1, 7 en D1 ; voir « Casts bloqués » ci-dessous et « État de D0b » |
| `%.4f`, et `double` + `pico8_fixed_to_double` de `pico8_value_tostr` | 3 | D2 — ✅ faite (« État de D2 ») | Phase D, `tostr` |
| `pow`, `pico8_fixed_from_double(ec_d)`, et `double` + 2 `pico8_fixed_to_double` de `pico8_dj` | 5 | D3 — ✅ faite (« État de D3 ») | Phase D, `^0.75` |
| revue des clés de table | 0 | D4 — ✅ faite (« État de D4 » : 33 sites clé, aucun fractionnaire possible) | *Déduction* (PRD de C0) : la conversion du jeton d'un cast-clé suit sa population, et le plan route (b) vers C1 sans exception ; D4 ne retire aucun jeton. D4 hérite de la liste des 28 casts dont la justification porte « cle de table » dans `tools/float_annotation.txt` (16 en C1, 1 en D1, 11 à auditer). |
| mots de commentaire : `float` (`pico8_fixed.c`), `double-free` (`pico8_bw`), `pico8_fixed_from_double` (`pico8_ob.c`, hors fonction), `double comma` (`pico8_ob_step`) | 4 | E | *Déduction* : le grep du §6.1, sortie de E, les voit. |
| `sin`/`cos` libc (2 réécrits), et `double` ×2, `pico8_fixed_to_double`, `pico8_fixed_from_double(…)` de chacune de `pico8_fixed_cos`/`_sin` (8 autres) | 10 | **D5** (décision humaine, Phase D) — ✅ faite (« État de D5 ») | [TROU-1](trous-de-portage.md) |
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
- **D1 — population (c)** — ✅ faite, voir « État de D1 ». Divergente, avec re-baseline.
- **D2 — `tostr`.** ✅ faite, voir « État de D2 ». **Préservatrice, pas divergente** : le `tostr` actuel reproduit déjà 13 témoins sur 14 et le 14ᵉ est conservé tel quel. Porte : (i) un test d'hôte qui compare ancien et nouveau sur les 65 536 parties fractionnaires, des deux signes, et quelques parties entières dont ±32767/−32768 — seule différence admise, le cas `< 0.0001` ; attention aux 16 égalités exactes où `%.4f` arrondit au pair ; (ii) `frames-check` identique, **sans** `frames-baseline`.

  **État de D2.** `pico8_value_tostr` (branche `FIXED`) calcule ses décimales en entiers : magnitude non signée, `f·625` sur 4096, arrondi au pair sur les égalités exactes (comme `printf`), retenue à 10000 ; la troncature des zéros est inchangée. Porte (i) : `tests/test_pico8_value.c` compare l'ancienne implantation (copie isolée dans le test, qui calcule `raw/65536.0` elle-même) à la nouvelle sur 1 245 183 entrées (18 parties entières × 65 536 fractions, plus les bruts −1 à −65 535), 304 égalités exactes, **0 différence** ; un arrondi au-dessus échoue sur `0x0000.0800` (« 0.0313 » au lieu de « 0.0312 »). Les 13 témoins du bloc C passent ; le cas `< 0.0001` garde la sortie du port et devient [TROU-22](trous-de-portage.md). Porte (ii) : `frames-check` identique sur les 12 fichiers, **sans** `frames-baseline`. Pas de paire d'audit : le balayage exhaustif est une preuve plus forte que ce que les rejeux affichent. Inventaire : 28 → 25 jetons (réécrit 4, autre 21) ; `pico8_value.c` n'y figure plus. Garde `-mgeneral-regs-only` : 3 → **2** fichiers en échec (`pico8_economy_formulas.c`, `pico8_fixed.c`). Couverture : 21 sites couverts, 4 non couverts.
- **D3 — `^0.75`.** ✅ faite, voir « État de D3 ». Divergente si le nouveau diffère de l'ancien. Constat fait en revue de fin de Phase C : `pico8_fixed_mul(s, pico8_fixed_sqrt(s))` avec `s = pico8_fixed_sqrt(x)` reproduit les 16 vecteurs du bloc D au bit près, et `pico8_fixed_sqrt` les 16 vecteurs `sqrt` du bloc E.

  **État de D3.** Nouvelle `pico8_fixed_pow075(x) = mul(s, sqrt(s))`, `s = sqrt(x)` : 16/16 vecteurs du bloc D et 16/16 vecteurs `sqrt` du bloc E au bit près (`tests/test_pico8_fixed.c`). `pico8_dj` calcule `ec = pow075(len_sq / bq)` en 16.16 — la division `double` était un accident de portage. Paire d'audit sur la valeur rendue par `pico8_dj` : 23 appels sur le filet (phase4 9, built_planet_panel 14), 14 à argument fractionnaire, **0 ancien ≠ nouveau** ; rien ne diverge (`tools/frames_reports/D3.txt` : 12/12 conformes), donc S = ∅, pas de re-baseline. `pico8_dj` est une **correction non vue par le filet** (la valeur rendue ne change que sur ~0,05 % des entrées), protégée par les vecteurs des blocs D et E. Enregistrements C1 identiques sur 12/12 (seules deux lignes renumérotées par le retrait de `#include <math.h>`). TROU-1 ne cite plus `^0.75`. Inventaire : 25 → 20 jetons (réécrit 2, autre 18), `pico8_economy_formulas.c` n'y figure plus. Garde : 2 → **1** fichier en échec (`pico8_fixed.c`, qui relève de D5 et E). Détail de l'attribution : `tools/frames_reports/D3-attribution.txt`.
- **D5 — `sin`/`cos` sur table** (décision humaine). ✅ faite, voir « État de D5 ». `pico8_fixed_sin`/`cos` lisent une table de valeurs 16.16 indexée par la fraction de tour, sans `sin()`/`cos()` libc. Modèle donné par l'humain : le portage de Celeste Classic sur GBA (JeffRuLz/Celeste-Classic-GBA, `src/sine.h` + `main.c:331-332` : 360 entrées `float`, index `(u16)(x*360) % 360`, sans interpolation) — le principe est repris, pas ses `float`. Porte : les 64 vecteurs `sin(i/64)`/`cos(i/64)` du bloc E reproduits **au bit près** ; hors de ces points, la table est une approximation **non sourcée**, documentée comme telle dans TROU-1 (taille, construction, interpolation éventuelle : à trancher par la PRD de l'étape). Divergente, avec re-baseline et attribution par l'audit avant/après (paire ancien libc / nouvelle table).

  **État de D5.** Table choisie avec l'humain : **quart d'onde de 16 385 entrées** (`src/pico8_quarter_wave.h`, 65 540 octets, générée par `tools/gen_quarter_wave.c` via `make sin-table`, qui recopie l'ancienne expression libc mot pour mot), repliée par quadrant, indexée par la fraction de tour masquée, sans interpolation ; `cos` lit la même table décalée d'un quart de tour, `sin` garde l'inversion de signe de PICO-8. Le cœur n'inclut plus `math.h`. Constat de cadrage : le calcul libc reproduisait déjà les 128 vecteurs du bloc E au bit près ; les tables plus petites (360 à la GBA, 257 interpolée) changeaient des valeurs sans source PICO-8 et ont été écartées. Tests : 64 + 64 vecteurs du bloc E, période, parité, balayage des 65 536 fractions contre l'ancien calcul — **0 différence**. Audit : 3 sites (`pico8_vec_qm` ligne partagée cos+sin, 323 526 appels ; `pico8_mz_kz` 1 503 ; `pico8_na` 1), 0 ancien ≠ nouveau ; rien ne diverge (`tools/frames_reports/D5.txt` 12/12), S = ∅, pas de re-baseline ; enregistrements d'audit antérieurs identiques sur 12/12. TROU-1 réécrit : exact aux 64 points du relevé, arrondi au plus proche ailleurs, sans source PICO-8. Inventaire : 20 → **10** jetons (0 réécrit, 10 autres : définitions `from_double`/`to_double` et mots de commentaires, tous à E). Garde : 1 fichier en échec (`pico8_fixed.c`, uniquement la définition de `to_double`), attendu jusqu'à E. Coût à revoir à la reprise PSX : 64 Ko de données en lecture seule.
- **D4 — clés de table.** ✅ faite, voir « État de D4 ». Réduite aux clés déjà converties (16 en C1, 1 en D1) ; lue principalement dans le compte d'arguments fractionnaires de D0a (un site sans argument fractionnaire sur tout le filet est entier de fait, pas seulement par argument).

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

**État de D1** (mesuré avec `cc` GCC 13.3, x86_64, 4 cœurs). Trois commits : `c6272b6` remet en ligne l'appel `pico8_spr_default` de `pico8_lq_ol_item_draw` (neutre, `frames-check` identique), `d57e758` convertit, attribue et re-baseline, puis celui-ci consigne. `float-inventory-check`, `constants-check` et `float-audit` sont verts, et `make test` aussi.

- **Conversion.** Les **29** casts `(int)pico8_fixed_to_double(x)` affectés à D1 (22 de C0 et 7 venus de D0b) deviennent `pico8_fixed_flr_int(x)`, rien d'autre ne change ; formes composées `-pico8_fixed_flr_int(s)` (`pico8_ha2_my_dispatch`) et `pico8_fixed_flr_int(pico8_fixed_flr(ratio))` (`pico8_mz_kj`). Les 29 lignes d'annotation sont retirées. Aucun site converti n'entre dans `tools/float_audit_c1.txt` : l'audit les classe D.
- **Audit** (`make float-audit`) : **76 sites**, **47 C1** et **29 D** ; 71 atteints, 5 non atteints (les 5 de G5), 0 écart, 0 négatif (C1), **0 nouveau ≠ `flr`**, **28 corrections non vues**, 0 erreur de liste, **4 lignes partagées** (les deux lignes `cp.x`/`cp.y` annoncées en D0a, `pico8_scroll.c` 139 et `pico8_wormhole_display.c` 87, chacune comptée deux fois). **5 226 116 passages** (1 515 540 en D0b) : l'essentiel vient des sites D les plus appelés, `pico8_particle_to_int` (2 094 431) et `pico8_hud_to_int` (763 679). Aux sites D, un seul a un ancien ≠ nouveau : `pico8_bl` (`src/pico8_pi.c` ligne 169), **1 021 sur 2 445 appels**, 1 077 négatifs, 2 052 fractionnaires, premier fautif `0xFF90E38F`. Négatifs vus ailleurs, tous entiers (0 ancien ≠ nouveau) : `pico8_rn` 15, `pico8_bg_to_int` 5 227, `pico8_hud_to_int` 22 038.
- **Attribution** (`tools/frames_reports/D1.txt`, `tools/frames_reports/D1-attribution.txt`). S = {`pico8_bl` `nc`}. 6 fichiers sur 12 divergent, **691 frames divergentes** :

  | Fichier | F | A (`pico8_bl`) | A→F | Frames divergentes |
  |---|---|---|---|---|
  | `phase4_playthrough` | 6 | 0 | 6 | 172 |
  | `built_planet_panel` | 6 | 0 | 6 | 219 |
  | `slipway_drag` | 8 | 2 | 6 | 171 |
  | `tech_screen_invent` | 29 | 14 | 15 | 28 |
  | `game_end` | 63 | 14 | 49 | 10 |
  | `geoharvest_burst` | 94 | 43 | 51 | 91 |

  Contrôle 1 : A < F partout, consigné « effet différé de A à F » ; aucun F sans A, aucun A sans F (les 6 autres fichiers sont conformes et sans A). Contrôle 2 (complément neutre, `pico8_bl` seul ramené à l'ancienne forme) : **12/12** conformes aux références d'avant D1. Contrôle 3 (variante suffisante, arbre d'avant avec le seul `pico8_bl` converti) : **12/12** empreintes identiques à l'arbre D1, toutes frames. Les 691 frames sont attribuées à `pico8_bl`, puis `make frames-baseline ETAPE=D1`.
- **Absence de cascade.** Mesure : les lignes `AUD` des 47 sites C1 de l'arbre d'avant se retrouvent toutes à l'identique dans l'arbre D1, sur 12/12 fichiers ; les lignes nouvelles sont toutes des sites D. Lecture : chaque site D1 ne nourrit que `camera`, une primitive de dessin, `music`/`sfx` sans effet ([TROU-10](trous-de-portage.md)), un choix d'icône ou de couleur, ou une branche morte ; les appelants hors dessin des aides `*_to_int` ne reçoivent que des valeurs où `flr` = troncature (détail dans `D1-attribution.txt`).
- **Revue sans fenêtre à F**, en confirmation : sur les 6 fichiers, la capture d'après est celle de la variante octet pour octet ; l'écart tient en 2 à 8 lignes sur 128 et 5 à 26 pixels, dans des boîtes à droite de l'écran (colonnes 100-127). C'est la boîte de message tracée sous `camera(nc,0)` par `pico8_bl`, décalée d'une colonne vers la droite à son entrée à l'écran (`flr(nc)` = `trunc(nc) − 1` pour `nc` négatif fractionnaire).
- **Inventaire** (`tools/float_inventory.txt`) : constantes 0, **casts 0** (29 → 0), réécrits 5, autres 23, **total 28** (57 → 28). Populations : toutes à 0. **Étapes** : C1 0, **D1 0**, à auditer 0. Restent, par fichier : `pico8_fixed.c` 15 autres et 2 réécrits (entre autres les définitions `from_double`/`to_double` → E et `sin`/`cos` → D5), `pico8_economy_formulas.c` 3 autres et 2 réécrits (`pico8_dj`, `^0.75` → D3), `pico8_value.c` 2 autres et 1 réécrit (`tostr` → D2), `pico8_ob.c` 2 autres et `pico8_galaxy.c` 1 autre (lignes sans code, voir la couverture).
- **Couverture** (`tools/float_coverage.txt`) : casts couverts **0**, non couverts **0** (24 et 5 en D0b) ; total couverts 24, non couverts 4 (les 4 « lignes sans code » de `pico8_fixed.c`, `pico8_galaxy.c` et `pico8_ob.c`). `pico8_fixed_to_double` passe de 4 061 051 à 350 475 passages.
- **Observation** : `make float-observe` s'arrête sur « aucun cast dans tmp/observe/inventaire.txt » (**code 2**), et ne réécrit pas la copie versionnée. `tools/float_observe.txt` reste la photographie de D0b ; l'outil n'est ni adapté ni retiré (constat pour E ou pour l'humain).
- **Garde** (`tools/float_guard.txt`, `make float-guard`, en échec comme prévu) : **22 → 3** fichiers en échec sur 58. Restent `pico8_economy_formulas.c` (D3), `pico8_fixed.c` (D5, E) et `pico8_value.c` (D2). Sortent les 19 fichiers qui ne gardaient qu'un cast D1 : `pico8_audio.c`, `pico8_bg.c`, `pico8_collect.c`, `pico8_economy_tiers.c`, `pico8_hud.c`, `pico8_layout.c`, `pico8_menu.c`, `pico8_particle.c`, `pico8_pi.c`, `pico8_planet.c`, `pico8_planet_display.c`, `pico8_probe.c`, `pico8_research.c`, `pico8_score_panel_display.c`, `pico8_scroll.c`, `pico8_slipway_display.c`, `pico8_ui.c`, `pico8_ui_button.c`, `pico8_wormhole_display.c`.
- **Coût.** `make test` : **59,4 s** après effacement de `tmp/obj`, `tmp/obj-asan`, `tmp/obj-ubsan` et `tmp/obj-audit`, puis **34,6 s et 34,6 s** à arbres d'objets à jour (33,6 s et 34,2 s en D0b), soit environ +0,7 s. `make float-audit` seul : **16,2 s** en recompilant `tmp/obj-audit/`, **14,2 s** à jour (13,7 s en D0b).

**Corrections non vues par le filet (D1).** Les 28 sites D1 à zéro ancien ≠ nouveau dans l'audit. Protection commune, au niveau valeur : l'expression convertie est `pico8_fixed_flr_int`, assertée contre le bloc B par `test_flr_int_matches_probe_block_b`, et par `test_flr_int_extremes` et `test_flr_int_raw_minus_one_is_minus_one_not_zero` (`tests/test_pico8_fixed.c`). Les formes composées s'y ramènent : `-flr(s)` garde la négation hors de la conversion, et `flr(flr(r)) = flr(r)`. Sur le filet, la porte D (0 nouveau ≠ `flr`) et le complément neutre (aucun pixel changé par ces 28) complètent. Pas de test par site.

- *Non atteints, branche « clé nombre » morte (G5, voir « Non protégés par le filet (D0b, G5) »)* : `pico8_ho_apply` (`pico8_collect.c` 208), `set_by_value` (`pico8_economy_tiers.c` 55), `pico8_jc_display_get_by_value` (`pico8_slipway_display.c` 82), `pico8_research_lookup` (`pico8_research.c` 229), `pico8_jo_hu` (`pico8_planet.c` 359). Clé chaîne par construction ; en Lua comme en C, `flr` ne rendrait de toute façon pas une clé fractionnaire fidèle (§3.2).
- *Négatifs vus, mais jamais négatifs et fractionnaires à la fois (troncature = `flr`)* : `pico8_rn` `no` (`pico8_audio.c` 28 ; 254 appels, 15 négatifs, 0 fractionnaire ; `sfx` sans effet), `pico8_bg_to_int` (`pico8_bg.c` 16 ; 24 274 appels, 5 227 négatifs, 60 fractionnaires), `pico8_hud_to_int` (`pico8_hud.c` 20 ; 763 679 appels, 22 038 négatifs, 12 304 fractionnaires).
- *Jamais négatifs sur le filet, fractionnaires positifs* : `pico8_lq_ol_item_draw` `pos.x`, `pos.y` (`pico8_layout.c` 204, 205), `pico8_particle_to_int` (`pico8_particle.c` 56), `pico8_jo_kj` `adjusted.x`, `.y` (`pico8_planet_display.c` 503, 504), `pico8_qb_md` `cp.x`/`cp.y` (`pico8_scroll.c` 139, ligne partagée), `pico8_jc_display_to_int` (`pico8_slipway_display.c` 91), `pico8_ui_to_int` (`pico8_ui_button.c` 44), `pico8_mz_kj` `p.x`, `p.y` (`pico8_wormhole_display.c` 67, 68) et `cp.x`/`cp.y` (87, ligne partagée).
- *Jamais négatifs, et (c) seulement après le repli de `qc.t` à 32 768 frames (le plus long fichier en fait 9 729)* : `pico8_mz_kj` `oi` (79 ; 1 187 appels, 1 058 fractionnaires) et `sr` (120 ; 3 080 appels, 3 040 fractionnaires) ; `pico8_fixed_flr(ratio)` (108 ; 3 080 appels, 0 fractionnaire), entier par construction, où la conversion ne peut rien changer.
- *Jamais négatifs, entiers seulement* : `pico8_ha2_my_dispatch` `s` (`pico8_menu.c` 358 ; 1 appel, `stat(20)` rendu 0 par le port, [TROU-10](trous-de-portage.md), `music` sans effet), `pico8_pj_ph_map` `s6_val` (`pico8_score_panel_display.c` 257 ; 5 appels), `pico8_probe_to_int` (`pico8_probe.c` 26 ; 42 appels), `pico8_ui_to_int` (`pico8_ui.c` 53 ; 46 614 appels).

**État de D4** (mesuré avec `cc` GCC 13.3, x86_64, 4 cœurs). Revue sans code : aucune ligne de `src/` n'a changé. `make frames-check` est identique sur les 12 fichiers de `FRAMES_EVENTS`, sans `frames-baseline`, et `make test` est vert.

- **Audit de référence** (`make float-audit` sur l'arbre de D4) : 80 sites, 47 C1 et 33 D, 75 atteints, 5 non atteints (les 5 de G5), 0 écart, 0 négatif, 0 nouveau ≠ `flr`, 32 corrections non vues, 4 lignes partagées, **5 551 169 passages**. Les comptes « appels / fractionnaires » de la table viennent de cette passe.
- **Balayage.** Il porte sur les 76 sites d'audit de `pico8_fixed_flr_int` (74 lignes, dont 8 corps d'aides `*_to_int`) et sur les 44 appels de ces aides. Critère : le `int` rendu, décalé ou non de ±1, sert d'indice à `pico8_ob_table_get_index`/`set_index`/`remove_index`, à `pico8_ft_index` ou à un `pico8_list_t`. Résultat : **33 sites clé**. 29 sont des appels directs : les 28 étiquetés « cle de table » (16 C1, 1 D1, 11 D0b) et `pico8_gn_next`. Les 4 autres passent par une aide. Aucun site clé n'est sur une ligne partagée.
- **Pas une clé**, une ligne par groupe :
  - *Appels directs (39)* :
    - coordonnées ou caméra : `pico8_pi.c` 106, 107, 169 ; `pico8_layout.c` 204, 205 ; `pico8_planet_display.c` 503, 504 ; `pico8_planet_family.c` 240, 241 ; `pico8_scroll.c` 139 ×2 ; `pico8_wormhole_display.c` 67, 68, 87 ×2 ;
    - rayon : `pico8_planet_display.c` 576, `pico8_planet_family.c` 277, `pico8_wormhole_display.c` 79, 120 ;
    - couleur lue dans une table, mais qui n'indexe rien : `pico8_pi.c` 64, 82 ; `pico8_layout.c` 199 ; `pico8_msgbox.c` 239 ; `pico8_planet_display.c` 536 ; `pico8_planet_family.c` 275 ; `pico8_wormhole_display.c` 115 ;
    - sprite ou taille : `pico8_pi.c` 105, 109, 110 ; `pico8_layout.c` 203 ; `pico8_planet_display.c` 519, 521 ;
    - borne de boucle : `pico8_gen.c` 72 (`for n=1,e.r*og`, lua.lua:734), `pico8_planet_display.c` 142 (`for j=1,i.gc`, 431), `pico8_research_display.c` 248 ;
    - adresse `poke` : `pico8_score_panel.c` 168 (`0x3101+4*ny`, 1175) ;
    - comparaison : `pico8_score_panel_display.c` 257 (`i<=s[6]`) ;
    - `sfx`/`music` : `pico8_audio.c` 28, `pico8_menu.c` 358.
  - *Appels d'aide (40)* :
    - `pico8_hud_to_int` ×9 : coordonnées, et l'adresse source de `dv` (`0x5000+…`, `pico8_hud.c` 41) ;
    - `pico8_probe_to_int` ×2, `ie_to_int` ×3, `pico8_ui_to_int` de `pico8_ui_button.c` ×7 : coordonnées, sprite, couleur ;
    - `pico8_bg_to_int` 135 (`122+rnd(5)` vers `mset`) et 144 (tuile lue) ;
    - `pico8_particle_to_int` 191, 192 (coordonnées) et 198 (couleur) ;
    - `pico8_jc_display_to_int` ×9 (coordonnées, couleurs) ;
    - `pico8_ui_to_int` de `pico8_ui.c` 231, 234, 239, 296 (sprite, coordonnées, valeur `poke`).
- **Contrôle hors `pico8_fixed_flr_int`.** Le grep de `>> 16`, `65536`, des `(int)` et des fonctions `*_int(` ne trouve que deux conversions, et aucune n'indexe une table. `pico8_value.c` 83 (`tostr`) va de 16.16 vers `int`. `pico8_ob.c` 231 va dans l'autre sens : c'est l'analyse d'un littéral du `ob`. Le cœur n'a pas d'autre conversion 16.16 → `int`.
- **Verdicts.** Aucune des 33 clés ne peut être fractionnaire. 28 sont entières par construction et 5 sont des chaînes par construction. Aucun verdict ne repose sur l'audit seul. Aucune clé n'est « fractionnaire possible » ni « non établi », et **aucun trou de portage n'est ouvert**. Les 28 sites clé atteints ont **0 fractionnaire** sur le filet. Ce compte ne vaut que comme complément : quatre de ces sites sont des aides partagées, et les 5 de G5 ont 0 appel. Les preuves de C0 portaient sur le signe (≥ 0). Elles sont reformulées ici en preuves d'intégralité et revérifiées contre `lua.lua`. Une correction en chemin : `t.nk` peut aussi valoir `r.a` = 33 (lettre `"a"` de `nb`, voir « Non protégés par le filet (D0b, G5) »), ce qui ne change pas le verdict.

  Sources communes :
  - **(r)** Les valeurs de `r` (unstash 8192, annexe A de `docs/design/slipways-mechanics.md`) sont les entiers 12..54. Une valeur `qe` ne vient que de `r[c]` (1407), de `r[ri[i]]` via `ho` (17-19, `gp` de `hp` 1401), ou de copies d'autres `qe` (463, 488, 492, 504, 607, 1053, 1280). Donc `qe` est un entier de 12..54, une chaîne quand `r` manque la lettre, ou `nil`.
  - **(mi)** `mi` vaut 1 à la création (912) ou `dh(pt)` (1278-1283). `dh` rend 0, 1, 2, `min(#…,3)`, `min(li.gc+1,4)` avec `gc=#jp(…)`, ou `min(4,1+mid(#pt.by,#pt.cc,1))` (1291-1310). Ce sont des littéraux, et des `min`/`mid` de compteurs `#` : la chaîne ne contient ni division, ni facteur décimal.

  | Site C (ligne) | Table[clé] Lua (lua.lua) | Prov. | L/É | Verdict | Source | Audit (appels / fract.) |
  |---|---|---|---|---|---|---|
  | `pico8_economy.c` `pico8_hy_hk` (131) | `self.po[r.k]` (1212) | C1 | L | entière par construction | `r.k` = 24, constante du cart (r) | 61 / 0 |
  | `pico8_economy_formulas.c` `set_by_value` (54) | `p[qe]=jm` de `cm`, `p[pt.kw.qe]+=` de `bx`, `d[….qe]-=0.15` de `ck` (1241, 1247, 1253) | C1 | É | entière par construction | `cm` : valeurs de `r` ; `bx`/`ck` : `qe` (r) | 3 465 / 0 (aide, mais ses 3 appelants indexent tous) |
  | `pico8_economy_formulas.c` `get_by_value` (65) | lectures de `p[…]` de `bx` et de `d[…]` de `ck` (1247, 1253) | C1 | L | entière par construction | `qe` (r) | 33 / 0 (2 appelants, tous deux clés) |
  | `pico8_economy_score.c` `get_by_value` (66) | `fm.po[r.p]`, `[r.h]`, `[r.w]` (1347, 1367-1368) | C1 | L | entière par construction | 19, 25, 12, constantes (r) | 265 / 0 (3 appelants, tous clés) |
  | `pico8_economy_score.c` `pico8_da` (111) | `es[pt.mi]` (1316) | C1 | L | entière par construction | (mi) | 31 / 0 |
  | `pico8_economy_score.c` `pico8_db` (180) | `ct[pt.mi]` (1330) | C1 | L | entière par construction | (mi) | 4 / 0 |
  | `pico8_economy_score.c` `pico8_db` (188) | `ct[oz.mi]` (1330) | C1 | L | entière par construction | (mi) | 4 / 0 |
  | `pico8_economy_score.c` `sum_by_mi_visit` (241) | `b_[pt.mi]` (1343), `co[pt.mi]` (1372) | C1 | L | entière par construction | (mi) | 31 / 0 (2 appelants, tous deux clés) |
  | `pico8_economy_tiers.c` `bb_visit` (218) | `po[min(pt.mi,#po)]` (1286) | C1 | L | entière par construction | `min` de (mi) et d'un `#` ; le `min` est fait en C après la conversion | 44 / 0 |
  | `pico8_entity.c` `pico8_dc` (261) | `ft(fl,qc.fa,qc)` puis `fl[o]` (191-195) ; C : `layers[layer-1]`, base 0 | C1 | É | entière par construction | `fa` : littéraux de classe 1..15 (214 à 1144) ; `2` ou `4` (630-631) ; `13` (879) ; `p.ca` (283-284), dont les littéraux valent 9..14 (400, 828, 992, 1146) | 1 211 558 / 0 |
  | `pico8_planet.c` `pico8_jo_pq` (234) | `gb[self.hp]` (405) | C1 | L | entière par construction | `hp` : défauts de classe 1..6 (907, 1001, 1020, 1039, 1060, 1097), seules affectations | 2 / 0 |
  | `pico8_planet_display.c` `pico8_jo_gh` (187) | `jo.dz[self.mi]` (426) | C1 | L | entière par construction | (mi) | 1 / 0 |
  | `pico8_planet_display.c` `pico8_jo_kj` (527) | `jo.ka[self.mi]`, sprite `mi+1` (547) | C1 | L | entière par construction | (mi) | 4 333 / 0 |
  | `pico8_score_panel.c` `get_by_value` (78) | `fm.po[r.k]` (847) | C1 | L | entière par construction | 24, constante (r) | 132 / 0 |
  | `pico8_score_panel.c` `pico8_pj_hb` (229) | `ef[fm.lo]` (847) | C1 | L | entière par construction | `lo` = 1 (979, 1202). `+=hx` (1205), où `hx` ne vient que de `2`, de `ov(…,3,2)` (451, 459, 1467-1470), de `self.fp or 0` avec `fp` littéral `f`, 1 ou 2 (571, 616, 712, 758, 910, 1103), ou de `1` (640). `-=kx` avec `kx=ov(…,12,15)` (1204-1207) | 132 / 0 |
  | `pico8_tech_gate.c` `pico8_gi` (88) | `fm.fu[t.nk]` (1473) | C1 | L | entière par construction | `nk = r[…]` posé par `ho` sur `nb` (1442-1444) ; lettres `t`/`l`/`o`/`b`/`a` → 22, 20, 21, 18, 33 (r) | 2 604 / 0 |
  | `pico8_wormhole_display.c` `pico8_mz_kj` (108) | `self.kk[flr(sr/self.oi*8)]` (790) | D1 | L | entière par construction | `flr` explicite en Lua, `pico8_fixed_flr` en C avant la conversion | 3 080 / 0 |
  | 6 sites G1, G2, G4 : `pico8_hy_kt` (`pico8_economy.c` 311), `pico8_cr` (`pico8_menu.c` 172 et 216), `pico8_pj_hk` (`pico8_score_panel_display.c` 92), `pico8_pj_ph_map` `score` (214), `pico8_ga_kj` (`pico8_planet_family.c` 270) | `ek[self.ew]`, `ek[tl-1]`, `ek[tl]`, `gs[3426-yr]`, `s[st.score]`, `k_[flr(pt.l/2)]` | D0b | L | entière par construction | voir la table de l'« État de D0b ». Décalage : en 172, le C calcule `flr(tl)-1`, soit la clé Lua `tl-1` ; en 92, `3426-flr(yr)` | 4, 4, 4, 61, 7, 251 / 0 |
  | 5 sites G5 : `pico8_ho_apply`, `set_by_value` (`economy_tiers`), `pico8_jc_display_get_by_value`, `pico8_research_lookup`, `pico8_jo_hu` | `r[ri[i]]`, `w.hg[….id]`, `self.hg[t.id]`, `hc[…]`, `df[hp.go]` | D0b | L, et É pour `economy_tiers` | chaîne par construction | voir « Non protégés par le filet (D0b, G5) » | 0 / 0 : non atteint, le compte ne prouve rien |
  | `pico8_gen.c` `pico8_gn_next` (92) | `oc[flr(r()*#oc+1)]` (739) ; C : `items[idx-1]`, base 0 | balayage | L | entière par construction | `flr` explicite en Lua, `pico8_fixed_flr` en C | 47 / 0 |
  | `pico8_bg.c`, appel de `pico8_bg_to_int` (138) | `bg.pl[flr(vh+rnd(4))]` (370) | balayage | L | entière par construction | `flr` explicite en Lua, `pico8_fixed_flr` en C | aide partagée (ligne 16) : 24 274 / 60 pour 3 appelants, ne dit rien de celui-ci |
  | `pico8_particle.c`, appel de `pico8_particle_to_int` (195) | `cs[flr(p.l)]` (705) | balayage | L | entière par construction | `flr` explicite en Lua, `pico8_fixed_flr(new_l)` en C | aide partagée (ligne 56) : 2 094 431 / 1 157 455 pour 4 appelants, ne dit rien de celui-ci |
  | `pico8_slipway_display.c`, appel de `pico8_jc_display_to_int` (184) | `self.k_[self.nh]` (669) | balayage | L | entière par construction | `nh` : 20 par défaut (619) ; 1 (639) ; `+=1` sous `nh<20` (670) | aide partagée (ligne 91) : 4 349 / 3 948 pour 10 appelants, ne dit rien de celui-ci |
  | `pico8_ui.c`, appel de `pico8_ui_to_int` (277) | `ft(self.ic,e.fa,e)` (229) | balayage | É | entière par construction | `fa`, mêmes sources que `pico8_dc` | aide partagée (ligne 53) : 46 614 / 0 pour 6 appelants. Ce 0 couvre aussi cet appel, mais l'audit ne dit pas s'il est atteint |

- **Comptes.** 33 sites clé revus : 16 C1, 1 D1, 11 D0b, 5 trouvés par le balayage. Par verdict : 28 entiers par construction, 5 chaînes par construction, 0 sur le seul audit, 0 fractionnaire possible, 0 non établi. Les écritures sont `set_by_value` de `economy_formulas`, `pico8_dc`, `pico8_ui.c` 277 et `set_by_value` de `economy_tiers` (G5). 79 appels balayés ne sont pas des clés : 39 directs et 40 d'aide. Le risque « Clés de table fractionnaires » du §10 est **levé**.

**Règle d'attribution d'une étape D.** Soit S l'ensemble des sites de l'étape ayant au moins un ancien ≠ nouveau sur le filet (audit), A la plus petite frame de premier ancien ≠ nouveau, F la première frame divergente de `tools/frames_reports/<étape>.txt`.
1. **A > F, ou F sans A : échec.** Un pixel a bougé avant toute valeur auditée.
2. **Complément neutre :** l'arbre de l'étape, les seuls sites de S ramenés à l'ancienne forme, donne `frames-check` identique à la référence d'avant l'étape sur tous les fichiers, sans `frames-baseline`. Cela prouve que les sites hors S (dont les corrections non vues) ne changent aucun pixel — y compris un glissement d'argument que l'audit ne peut pas voir.
3. **Variante suffisante :** l'arbre d'avant l'étape, les seuls sites de S convertis, donne des empreintes identiques à celles de l'arbre de l'étape, sur toutes les frames. Cela prouve l'absence d'interaction.
4. Si 2 et 3 passent, toutes les frames divergentes sont attribuées à S, nommé. A < F est permis et consigné (« effet différé de A à F ») ; la revue sans fenêtre à F est une confirmation, pas la porte.
5. Si S a plus d'un site et qu'on veut les départager, on répète 2 et 3 site par site ; sinon on consigne « attribution partagée : S ».

Le contrôle d'absence de cascade sur l'état (enregistrements C1 de l'audit identiques avant/après, plus lecture du code) est maintenu : il répond à une autre question.

*Source du changement (D1, chunk 2) :* l'ancienne règle « F == A » a échoué sur les 6 fichiers divergents, A < F partout (écarts 6/6/6/15/49/51), tous à `pico8_bl` — l'offset caméra de la boîte de message change alors qu'elle est encore hors écran, l'effet sur les pixels vient quand elle entre. `pico8_bl` est le seul site D1 à ancien ≠ nouveau sur tout le filet ; aucune cascade sur l'état. La règle stricte ne gardait que la première frame ; le complément neutre garde toutes les frames.

**Corrections invisibles au filet :** pour chaque étape D, les sites à zéro appel ancien ≠ nouveau sur le filet sont listés « correction non vue par le filet » (comme les 13 constantes de C2b). Chacun reçoit un test au niveau valeur de l'expression convertie (par exemple une entrée négative fractionnaire qui doit donner `flr`) ou une entrée explicite dans ce plan. Critère de couverture, pas de ligne `gcov` : C a montré que NON COUVERT ne prouve rien dans un sens comme dans l'autre. Structurel pour D1 : `oi`, `sr` et `flr(ratio)` ne sont (c) que par le repli de `qc.t` à 32 768 frames, alors que le plus long fichier du filet en fait 9 729.

**Sortie de phase :** pour chaque étape D divergente, toutes les frames divergentes sont attribuées à S par les contrôles complément/variante de la règle ci-dessus ; F, A et l'écart A→F consignés dans `<étape>-attribution.txt` ; aucune cascade sur l'état (enregistrements C1 de l'audit identiques avant/après), et le rapport de divergence est versionné avant le re-baseline ; `frames.hash` re-baseliné **volontairement**, avec revue des frames concernées — jamais subi comme une régression. D2 ne re-baseline pas.

### Phase E — Fermeture — ✅ FAITE
Seule porte de E : les quatre points du §6. Décisions humaines prises en fin de Phase D :
- **§6.1 porte sur le cœur seul**, conformément au §4 : `CORE_SRCS` et les en-têtes qu'ils incluent par défaut (sans les définitions `PICO8_FLOAT_*`). Le reste de `src/` (backend raylib, visualiseur, builds d'audit) est exclu par une liste versionnée, une ligne de justification par fichier.
- **§6.4 se lit « toute divergence justifiée par une ligne du relevé »** : C2b (bloc A) et D1 (bloc B) sont admis.
- **Le build d'observation est retiré** (`make float-observe`, `src/pico8_float_observe.*`) : plus aucun cast à observer, l'audit avant/après le remplace. ✅ fait en E1.
- **TROU-1 est restreint, pas fermé** : il ne décrit plus que `sin`/`cos` hors des 64 points du relevé.

Étapes :
- **E0 — Mesurer.** ✅ faite, voir « État de E0 ». Une cible `make` par point du §6, lancée sur l'arbre actuel **avant toute modification**, résultat consigné : 6.1 (grep sur l'ensemble défini ci-dessus, via `cc -MM`, plus la liste d'exclusion), 6.2 (`$(TMP_DIR)/pico8_replay` lié sans `-lm` — c'est « la cible de lien du cœur »), 6.3 (`float-guard`), 6.4 (registre : chaque commit depuis `30dfd6c` qui touche un `*.frames.hash` a son `tools/frames_reports/<étape>.txt` et sa ligne de relevé). L'écart entre « inventaire = 0 » et « §6 passe » y apparaît (prototypes et commentaire de `pico8_fixed.h`, mots de commentaires d'en-têtes).
- **E1 — Sortir `from_double`/`to_double` du cœur.** ✅ faite, voir « État de E1 ». Déclarations **et** définitions dans un module d'aides de test/hôte, hors `ENGINE_SRCS`/`CORE_SRCS`, lié par les tests et l'audit. Sans prototype dans l'en-tête du cœur, tout nouvel appel flottant dans le cœur devient une erreur de compilation (`-Werror`, déclaration implicite) : c'est ce qui referme l'angle mort x86_64 de la garde. Retrait du build d'observation. Neutre : `frames-check` identique sans `frames-baseline`.
- **E2 — Mots de commentaires.** ✅ faite, voir « État de E2 ». Les 4 de l'inventaire et les 3 d'en-têtes, plus la mention devenue périmée de `pico8_ob.c`. TROU-1 restreint.
- **E3 — `-lm` retiré de `$(TMP_DIR)/pico8_replay`** ✅ faite, voir « État de E3 ». Seulement ; il reste sur les variantes couverture/audit, les tests qui incluent `math.h`, le générateur et `main_raylib`.

  **État de E3.** La ligne de lien de `$(TMP_DIR)/pico8_replay` n'a plus `-lm` ; il reste sur les variantes couverture/audit, les tests, le générateur `gen_quarter_wave` et `main_raylib`. Le binaire ne dépend plus que de `libc.so.6` (`readelf -d`), `nm -u` ne montre aucun symbole de libm. Les quatre cibles du §6 : `float-grep` 0 (119 fichiers du cœur, 30 exclus, 0 ligne), `float-link` 0 (62 objets, 0 non résolu), `float-guard` 0 (58 fichiers, 0 en échec), `frames-ledger` 0 (6 commits, 4 poses, 2 divergences, 0 erreur). `frames-check` 12/12 identique, `make test` vert. Désormais 6.2 est tenu par le lien de `pico8_replay` que `make test` construit : un appel à libm réintroduit dans le cœur casserait ce lien.
- **E4 — Verrouiller** ✅ faite, voir « État de E4 ». Les cibles 6.1, 6.3 (liste vide) et 6.4 entrent dans `make test` ; 6.2 est verrouillé par le lien de `pico8_replay` que `make test` construit déjà. *Écart décidé à l'exécution :* 6.4 (`make frames-ledger`) reste une cible explicite, hors suite.

**État de E0** (mesuré avec `cc` GCC 13.3, x86_64, 4 cœurs). Aucune ligne de `src/` n'a changé ; `make frames-check` est identique sur les 12 fichiers de `FRAMES_EVENTS`, sans `frames-baseline`, et `make test` est vert. Trois cibles nouvelles, toutes hors de `make test`, chacune portée par un script de `tools/` testé sur des entrées fabriquées dans `make unit` (`test_float_grep`, `test_float_link`, `test_frames_ledger`) ; la 6.3 reste `make float-guard`, inchangée. La liste de sources de `$(TMP_DIR)/pico8_replay` devient `REPLAY_SRCS`, partagée avec `float-link` ; sa ligne de lien garde `-lm` jusqu'à E3.

| § | Cible | Code | Résultat sur l'arbre d'E0 |
|---|---|---|---|
| 6.1 | `make float-grep` | 1 (échoue) | `float-grep coeur 119 exclus 30 lignes 13` : 13 lignes dans 8 fichiers, liste ci-dessous |
| 6.2 | `make float-link` | 0 (passe) | `float-link objets 62 non-resolus 0` : les 62 objets de `pico8_replay` se lient sans `-lm` |
| 6.3 | `make float-guard` | 1 (échoue) | `garde fichiers 58 en-echec 1` : `src/pico8_fixed.c` ; `tools/float_guard.txt` inchangé |
| 6.4 | `make frames-ledger` | 0 (passe) | `frames-ledger commits 6 poses 4 divergences 2 erreurs 0` |

- **6.1.** Ensemble du cœur calculé par `cc -MM` sur les 58 `CORE_SRCS`, sans `PICO8_FLOAT_*` : 119 fichiers (58 `.c`, 61 en-têtes, dont `pico8_quarter_wave.h`). `src/` en compte 149 ; les 30 autres sont dans `tools/float_grep_exclusions.txt`, une justification par ligne : 4 points d'entrée (`main*.c`), les outils d'hôte du §4 (`pico8_assets.*`, `pico8_dump.*` : leurs en-têtes ne sont inclus par aucun fichier du cœur), les builds d'audit et d'observation (`pico8_float_audit.*`, `pico8_float_observe.*`), et backend raylib, palette, visualiseur, enregistrement d'écran, rejeu et empreintes (`pico8_raylib_backend*`, `pico8_palette.*`, `pico8_screencast*`, `pico8_replay_*`, `pico8_session_recorder.*`, `pico8_frame_hash.*`). Les 13 lignes :
  - `pico8_fixed.c` 9, 10, 15, 16 (définitions de `from_double`/`to_double`) et 128 (« no host float ») ;
  - `pico8_fixed.h` 29 et 32 (les deux prototypes), 31 (« representable as a double ») ;
  - `pico8_galaxy.c` 133 (« double-frees »), `pico8_ob.c` 347 (« double comma ») ;
  - `pico8_geom.h` 9 (« double duty »), `pico8_research.h` 21 (« double-free »), `pico8_type_tag.h` 15 (« double stride »).
- **6.2.** Le lien d'essai (`tmp/float-link/pico8_replay`, jamais lancé) passe : aucun symbole de `libm` n'est plus appelé par la cible de lien du cœur. E3 n'a que `-lm` à retirer de la ligne de lien.
- **6.4.** Registre `tools/frames_ledger.txt`, base `30dfd6c` : `2b1151d` C2b divergence (bloc `a`, `0.001 0x0000.0041`), `d57e758` D1 divergence (bloc `b`, `spr(-0.5,4) -> 6,4`), et 4 poses de D0b (`6bb2265`, `457991c`, `4c40a1a`, `66d3c98`), qui n'ajoutent que des références neuves. Les deux divergences ont leur rapport et leur attribution dans l'arbre de leur commit, et leurs fichiers `DIVERGENT` sont exactement les références réécrites (5 pour C2b, 6 pour D1). `D3.txt` et `D5.txt` n'ont pas de commit de re-baseline ; le registre ne les exige pas. Le clone de mesure est superficiel (`git rev-parse --is-shallow-repository` : `true`) mais la base y est joignable ; sur un clone qui ne l'atteint pas, la cible rend 2.
- **Écart entre inventaire et §6.1**, pour E1 et E2 :
  - vus par le grep, pas par l'inventaire (qui ne lit que les `.c` du cœur) : 6 lignes dans 4 en-têtes, `pico8_fixed.h` 29, 31, 32, `pico8_geom.h` 9, `pico8_research.h` 21, `pico8_type_tag.h` 15 ;
  - vus par l'inventaire, jamais par le grep (le `_` est un caractère de mot) : 3 jetons d'identifiant sur 10, le nom `pico8_fixed_from_double` de la ligne 9 et `pico8_fixed_to_double` de la ligne 15 de `pico8_fixed.c` (lignes que le grep voit quand même, par leur `double`), et la mention `pico8_fixed_from_double` de `pico8_ob.c` 201, seule ligne de l'inventaire absente du grep. §6.1 passerait avec elle ; seul E1 (plus de prototype dans l'en-tête du cœur) ferme cet angle, et E2 la retire comme périmée ;
  - compte d'E2 : « les 3 d'en-têtes » est **confirmé** (`pico8_geom.h`, `pico8_research.h`, `pico8_type_tag.h`) à condition que le commentaire de `pico8_fixed.h` 31, qui décrit `to_double`, sorte avec les prototypes en E1. S'il reste, E2 en a 4.
- **Coût dans `make unit`** : les trois tests prennent 0,5 s (`test_float_grep`), 0,3 s (`test_float_link`) et 1,6 s (`test_frames_ledger`, un dépôt git fabriqué par cas), soit environ 2,4 s. `make test` : 37,4 s (arbres d'objets à jour).

**État de E1** (mesuré avec `cc` GCC 13.3, x86_64, 4 cœurs). `pico8_fixed_from_double`/`to_double` vivent dans `src/pico8_fixed_test_double.{h,c}`, à l'identique (noms, arrondi `±0.5` puis troncature `int64_t`, division par 65536) ; `pico8_fixed.h` a perdu les deux prototypes, leurs commentaires (dont « representable as a double ») et le `/* TROU-1 */` de `from_double`, `pico8_fixed.c` les deux définitions. L'aide est liée par les huit tests appelants, `test_pico8_fixed_ubsan`, les trois `_refaudit` correspondants, `pico8_replay_audit` (depuis l'arbre par défaut) et le programme engendré par `tools/constants_compare.sh` (source d'aide voisine du `pico8_fixed.c` reçu) ; `test_constant_variants` la copie dans sa racine fabriquée. Pas par `pico8_replay` : `nm tmp/pico8_replay` ne contient ni `from_double` ni `to_double`. `tools/float_grep_exclusions.txt` : 30 − 2 (observation) + 2 (aide) = 30. Le build d'observation est retiré en entier (sources, arbre `obj-observe`, règle, cible, légende, `-include`, `tools/float_observe.{sh,txt}`, `test_float_observe`) ; les objets restent en six arbres. `make frames-check` est identique sur les 12 fichiers de `FRAMES_EVENTS`, sans `frames-baseline` ; `make test` est vert ; `make ref-audit` construit ses 8 binaires.

| § | Cible | Code | Résultat sur l'arbre d'E1 |
|---|---|---|---|
| 6.1 | `make float-grep` | 1 (échoue) | `float-grep coeur 119 exclus 30 lignes 6` : 6 lignes dans 6 fichiers, plus aucune de `pico8_fixed.h` |
| 6.2 | `make float-link` | 0 (passe) | `float-link objets 62 non-resolus 0` |
| 6.3 | `make float-guard` | 0 (passe) | `garde fichiers 58 en-echec 0` ; `tools/float_guard.txt` sans ligne de fichier |
| 6.4 | `make frames-ledger` | 0 (passe) | `frames-ledger commits 6 poses 4 divergences 2 erreurs 0` ; aucun `*.frames.hash` touché |

- **6.1.** Les 6 lignes, reliquat exact d'E2 : `pico8_fixed.c` 117 (« no host float », ex-128), `pico8_galaxy.c` 133, `pico8_ob.c` 347, `pico8_geom.h` 9, `pico8_research.h` 21, `pico8_type_tag.h` 15. Un fichier du cœur qui inclurait `pico8_fixed_test_double.h` entrerait dans le cœur par `-MM` tout en étant exclu : `float_grep.sh` rendrait 2.
- **Inventaire** : `compte autre 4`, `compte total 4` (`pico8_fixed.c` « float », `pico8_galaxy.c` 133, `pico8_ob.c` 201 et 347) ; les 6 jetons des définitions ont quitté le cœur. `tools/float_coverage.txt` régénéré : `couverture total couverts 0 non-couverts 4`.
- **Compte d'E2 confirmé** : « les 4 de l'inventaire et les 3 d'en-têtes » (`pico8_geom.h`, `pico8_research.h`, `pico8_type_tag.h`), le commentaire de `pico8_fixed.h` étant sorti avec les prototypes. La mention `pico8_fixed_from_double` de `pico8_ob.c` 201 reste pour E2, comme le texte de TROU-1 dans `docs/trous-de-portage.md`.
- **Angle mort de la garde, fermé et mesuré** : sur une copie jetable de `src/pico8_vec.c` terminée par `return pico8_fixed_from_double(0.5);`, `cc -std=c99 -Wall -Wextra -Werror -g -I src -c` échoue (code 1), avec ou sans `-mgeneral-regs-only` : `error: implicit declaration of function 'pico8_fixed_from_double'; did you mean 'pico8_fixed_from_int'? [-Werror=implicit-function-declaration]`.
- **Constat pour l'humain, pré-existant, non traité** : `make constant-variants` (hors suite) échoue comme avant E1 — `constants_compare: aucune constante dans tools/float_inventory.txt ni dans les sources`, code 1 —, sa recette appelant le comparateur sans sources et l'inventaire n'ayant plus de constante depuis C2b.
- **Durée** : `make test` 38,4 s (arbres d'objets à jour), contre 37,4 s en E0.

**État de E2** (mesuré avec `cc` GCC 13.3, x86_64, 4 cœurs). Seuls des commentaires changent dans `src/` (`git diff` : aucune ligne de code) : `pico8_fixed.c` (« no host float » retiré de la doc de `pico8_fixed_sqrt`, redondant avec la garde), `pico8_galaxy.c` (« frees the pair twice »), `pico8_ob.c` (première phrase du commentaire de `pico8_ob_parse_number`, périmée depuis E1, retirée ; « two commas in a row »), `pico8_geom.h` (« where 0 would have to mean both »), `pico8_research.h` (« free them twice »), `pico8_type_tag.h` (« twice the stride »). Aucune reformulation ne contient les sous-chaînes `double`/`float`. TROU-1 restreint : titré « `sin`/`cos` ne sont pas bit-exacts hors des 64 points relevés », sans le paragraphe des littéraux décimaux ni le renvoi à ce plan ; les `/* TROU-1 */` de `pico8_fixed_sin`/`_cos` restent. `make frames-check` est identique sur les 12 fichiers de `FRAMES_EVENTS`, sans `frames-baseline` ; aucun `*.frames.hash` touché ; `make test` est vert ; `make float-audit` passe.

| § | Cible | Code | Résultat sur l'arbre d'E2 |
|---|---|---|---|
| 6.1 | `make float-grep` | 0 (passe) | `float-grep coeur 119 exclus 30 lignes 0` |
| 6.2 | `make float-link` | 0 (passe) | `float-link objets 62 non-resolus 0` |
| 6.3 | `make float-guard` | 0 (passe) | `garde fichiers 58 en-echec 0` |
| 6.4 | `make frames-ledger` | 0 (passe) | `frames-ledger commits 6 poses 4 divergences 2 erreurs 0` |

- **Inventaire** régénéré : `compte total 0`, toutes catégories à 0 ; `make float-inventory-check` vert. Le grep du §6.1 et l'inventaire valent tous deux zéro : l'écart décrit dans l'« État de E0 » est fermé.
- **Constat pour l'humain, par conception, non traité** : `make float-coverage` (hors suite) rend désormais 2 — `float_coverage: aucun site dans tmp/coverage/inventaire.txt` —, l'outil refusant une jointure vide pour qu'elle ne passe pas pour une couverture complète. `tools/float_coverage.txt` reste la photographie d'E1 (4 sites non couverts, tous des commentaires, disparus en E2) : il n'est plus à jour. Idem, toujours, pour `make constant-variants` (constat d'E1).

**État de E4** (mesuré avec `cc` GCC 13.3, x86_64, 4 cœurs). Aucune ligne de `src/` n'a changé, aucun `*.frames.hash` touché ; `make frames-check` est conforme sur les 12 fichiers de `FRAMES_EVENTS`, sans `frames-baseline`. `make test` devient `unit replay screencast-check frames-check float-inventory-check constants-check float-audit float-grep float-guard` ; la légende du `Makefile` le dit, et dit pourquoi `float-link` et `frames-ledger` restent hors suite. L'en-tête que `tools/float_guard.sh` écrit dans `tools/float_guard.txt` (régénéré) dit que la liste doit être vide et que l'angle mort x86_64 est tenu par l'absence de prototype flottant dans le cœur ; le commentaire du `Makefile` au-dessus de `float-guard` ne garde que l'aide à la lecture. Le message « base injoignable » de `tools/frames_ledger.sh` nomme le remède (`git fetch --unshallow`, ou `git fetch --deepen=<n>`), code 2 inchangé ; `test_frames_ledger` l'asserte désormais, et `make unit` est vert.

| § | Cible | Code | Résultat sur l'arbre d'E4 | Tenu par |
|---|---|---|---|---|
| 6.1 | `make float-grep` | 0 (passe) | `float-grep coeur 119 exclus 30 lignes 0` | `make test` |
| 6.2 | lien de `tmp/pico8_replay` sans `-lm` ; `make float-link` | 0 (passe) | `float-link objets 62 non-resolus 0` | `make test` (cible `replay`) |
| 6.3 | `make float-guard` | 0 (passe) | `garde fichiers 58 en-echec 0` | `make test` |
| 6.4 | `make frames-ledger` | 0 (passe) | `frames-ledger commits 6 poses 4 divergences 2 erreurs 0` | cible explicite, lancée à la clôture |

- **`float-link` hors suite** : il refait le lien des mêmes 62 objets que `tmp/pico8_replay`, que `make test` lie déjà sans `-lm` ; il reste un diagnostic (liste des non-résolus), et `test_float_link` reste dans `make unit`.
- **`frames-ledger` hors suite** (décision de l'orchestrateur, écart à la puce E4) : §6.4 est une propriété de l'historique, et la cible rend 2 sur tout clone qui n'atteint pas la base — une suite rouge par environnement finirait ignorée. Lancée à la clôture d'E4 sur un clone superficiel qui atteint la base (`is-shallow-repository` : `true`, base à 96 commits de la tête) : code 0, ligne ci-dessus. Sur un clone de profondeur 50 fabriqué pour la mesure : code 2, `frames_ledger: base injoignable : 30dfd6c (clone superficiel ? git fetch --unshallow, ou git fetch --deepen=<n> jusqu'a l'atteindre)` ; après `git fetch --deepen=60`, code 0 et la même ligne. `make test` n'en dépend pas.
- **Preuves de verrou**, une par cible, sur des changements jetables annulés ensuite (non versionnées) :
  - `/* double */` ajouté en fin de `src/pico8_fixed.h` → `make test` rouge sur `float-grep` : `src/pico8_fixed.h:110:/* double */`, `float-grep coeur 119 exclus 30 lignes 1`, `make: *** [Makefile:757: float-grep] Error 1` ;
  - `__typeof__(0.5) pico8_e4_probe(void) { return 0.5; }` (prototype compris) ajouté en fin de `src/pico8_vec.c`, sans le mot `double` pour passer le grep → `make test` rouge sur `float-guard` : `error: SSE register return with SSE disabled`, `garde fichiers 58 en-echec 1`, `make: *** [Makefile:747: float-guard] Error 1` ; `tools/float_guard.txt` régénéré ensuite ;
  - ligne de C2b falsifiée dans `tools/frames_ledger.txt` (`0.001 0x0000.0042`) → `make frames-ledger` : le script rend 1, `2b1151d C2b erreur : ligne de releve introuvable dans le bloc a : 0.001 0x0000.0042`, `erreurs 1`.
- **Durée** (arbres d'objets à jour, deux mesures) : l'ancienne composition 38,6 s et 38,4 s, `make test` 41,3 s et 40,7 s, soit ≈ 2,5 s de plus (≈ 6,5 %) ; `float-grep` 0,7 s, `float-guard` 2,2 s seuls.

**Sortie de phase E.** Les quatre points du §6 passent, chacun verrouillé :

| § | Point | Résultat final | Verrou |
|---|---|---|---|
| 6.1 | aucun `double`/`float` dans le cœur | `float-grep coeur 119 exclus 30 lignes 0` | `make test` (`float-grep`) |
| 6.2 | la cible de lien du cœur sans `-lm` | `tmp/pico8_replay` lié sans `-lm` ; `float-link objets 62 non-resolus 0` | `make test` (`replay`) |
| 6.3 | cœur compilé sous `-mgeneral-regs-only` (garde hôte) | `garde fichiers 58 en-echec 0` | `make test` (`float-guard`) |
| 6.4 | toute divergence justifiée par une ligne du relevé | `frames-ledger commits 6 poses 4 divergences 2 erreurs 0` (C2b bloc A, D1 bloc B) | `make frames-ledger`, lancé à la clôture d'E4 |

Constats laissés à l'humain, non traités par ce volet :
- `make constant-variants` (hors suite) échoue depuis C2b : l'inventaire n'a plus de constante et la recette appelle le comparateur sans sources (« État de E1 ») ;
- `make float-coverage` (hors suite) rend 2 sur inventaire vide, par conception ; `tools/float_coverage.txt` est la photographie d'E1 (« État de E2 ») ;
- reliquat de la Phase A : le cas `tostr` de magnitude < 0.0001 n'est pas sourcé (§10) ;
- hors périmètre par décision : la vérification `mipsel`/soft-float, `pico8_fixed_rnd` (TROU-17), et TROU-1 restreint à `sin`/`cos` hors des 64 points relevés.

> **Décision tranchée par l'humain en Phase D :** `sin`/`cos` sur table (D5) ; la vérification PSX hors périmètre. Historique de la question :
>
> **Décision ouverte, relevée en fin de Phase B (à trancher en C0, par l'humain) :** les 2 sites `sin`/`cos` libc de `pico8_fixed_cos`/`_sin` n'appartiennent à aucune étape — le §4 et le §5 interdisent de réécrire ces fonctions, tandis que le §6.1-6.3 et la Phase E exigent zéro flottant et le retrait de `-lm`, et TROU-1 porte précisément sur eux. Tels quels, les deux ne peuvent être satisfaits ensemble. Idem pour les définitions `pico8_fixed_from_double`/`to_double` elles-mêmes (catégorie « autre »). Ce plan ne choisit pas : ajuster le §6/E, ou lever la clôture du §5 pour ces sites.
> C0 l'a comptée sans la trancher : 16 jetons (10 pour `pico8_fixed_cos`/`_sin`, 6 pour les définitions), sur la ligne « sans propriétaire — décision ouverte » de la table de propriété (Phase C, C0).

**Sortie de phase :** les quatre points du §6 passent. ✅ Atteinte, voir « Sortie de phase E ».

### Après la Phase E — outils retirés

Trois outils du volet flottant n'avaient plus d'objet ; ils sont retirés en entier (scripts, tests de `make unit`, rapports versionnés, règles, cibles et lignes de légende du `Makefile`, arbre d'objets `obj-coverage`) :
- **`make constant-variants`** (`tools/constant_variants.sh`, `test_constant_variants`) : il servait l'attribution de C2b et échouait depuis, l'inventaire n'ayant plus de constante. `tools/frames_reports/C2b-variantes.txt` reste, comme trace de cette attribution.
- **`make float-coverage`** (`tools/float_coverage.{sh,txt}`, `test_float_coverage`, `tmp/pico8_replay_coverage`) : il joignait la couverture du filet à l'inventaire, vide depuis E2, et rendait 2 par conception.
- **L'inventaire** : `make float-inventory` et `float-inventory-check` (qui sort de `make test`), `tools/float_inventory{,_check}.sh`, `tools/float_inventory.txt`, `tools/float_annotation.txt` (plus aucun cast à annoter), `test_float_inventory{,_check}`. Il valait zéro depuis E2. La propriété qu'il tenait est tenue plus strictement par `float-grep` (§6.1 : cœur complet, en-têtes compris, par `cc -MM`) et `float-guard` (§6.3 : aucun flottant à l'exécution, et aucun prototype flottant, donc aucun appel), tous deux dans `make test`.

`make constants-check` ne lit plus que les appels `PICO8_FIXED_LIT` des sources du cœur : `tools/constants_compare.sh` et `tools/constants_check.sh` perdent l'argument `<inventaire>`, le programme engendré n'a plus besoin de `pico8_fixed_test_double.c` ni de `-lm`, et `test_constants_check` fabrique des sources au lieu d'un inventaire. Résultat inchangé : `litteraux 34 | sites 61 | ok 34 | ecart 0`. `make test` devient `unit replay screencast-check frames-check constants-check float-audit float-grep float-guard` ; vert, `frames-check` conforme sur les 12 fichiers sans `frames-baseline`, 35,3 s (arbres d'objets à jour).

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
| ~~Clés de table fractionnaires (§3.2)~~ | — | **Levé en D4** : les 33 sites où une valeur 16.16 devient une clé sont entiers (28) ou chaînes (5) par construction, sources dans l'« État de D4 » ; aucun trou ouvert |
| Phase C mélangeant (a)/(b) et les 16 constantes | Une régression se cacherait derrière une divergence légitime | Découpe C1/C2 en commits séparés (§7) |
| `frames.hash` diverge en Phase C | Bug introduit par une transformation censée être neutre | C'est précisément l'objet du critère §6.4 : la divergence est le signal, pas un désagrément |
| ~~Le `pow(^0.75)` en fixe n'est pas reproductible à partir de 16 points témoins~~ | — | **Levé en D3** : `sqrt(x)·sqrt(sqrt(x))` reproduit les 16 vecteurs du bloc D au bit près |
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
9. Le critère de fin inclut la vérification `nm` de l'absence de symboles soft-float — seule preuve réelle de l'objectif PSX. *Levée par décision humaine en Phase D : vérification `mipsel`/`nm` hors périmètre, §6.3 se juge sur la garde hôte.*
10. Le volet `malloc` n'est **pas** cadré ; le §8 liste ses questions ouvertes.

Ajoutées après le relevé (`tools/probe_out.txt`, PICO-8 0.2.6b) :

11. PICO-8 **tronque** les littéraux décimaux : `floor(v × 65536)`, sans exception sur les 54 relevés. Les constantes du C sont recopiées du relevé.
12. **16 des 34 constantes du C sont décalées d'un LSB** — bug confirmé, corrigé en Phase C2, dans un commit séparé de C1.
13. `spr()` applique **`flr`** à ses coordonnées. Les 76 casts `(int)pico8_fixed_to_double` sont **faux pour tout négatif** — bug confirmé, corrigé en Phase D.
14. Les **clés de table fractionnaires sont des clés distinctes** en Lua/PICO-8 ; le C convertit avant la recherche. À établir site par site.
15. `pico8_value_tostr` est **déjà correct** sur 13 témoins sur 14 ; seul le cas de magnitude < 0.0001 reste non sourcé et est conservé tel quel, documenté comme tel.
16. `x^0.75` s'écrit `sqrt(x) · sqrt(sqrt(x))` en fixe pur — aucun `pow` à écrire, 16 vecteurs de test disponibles.
17. `rnd(0)` n'avance pas le PRNG, ce que le C fait déjà. Consigné, **non exploité** (bloc F).
