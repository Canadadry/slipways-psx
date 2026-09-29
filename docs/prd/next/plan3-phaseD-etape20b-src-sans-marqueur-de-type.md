---
title: "Plan 3, étape D.20b : `src/` sans marqueur de type — les 23 tests d'étiquette deviennent le test de classe qui les suivait, ou partent devant un pointeur typé ; le membre `tag` de l'en-tête d'entité, ses 17 écritures, les 15 includes et `pico8_type_tag.h` sont retirés ; `mo` et `ui` écrivent `{&cls}` ; `type-tag` = 0 et gardée, résidu D vide, Sortie de D"
description: "Dernière étape de la phase D. Après D.20a, `type-tag` n'a plus que 76 lignes (71 dans `src/`, 5 dans `mo` et `ui`). D.20b1 retire les lecteurs de `src/` (23 tests, 7 includes morts), D.20b2 retire le membre, ses écritures, l'en-tête et le reste des includes, et met `type-tag` dans `DATA_RESIDUE_GUARD`. `type-tag` 76 / 25 → 46 / 18 → 0 ; aucune image, aucun `*.sha256`, aucune empreinte de TU ne bouge."
status: needs-triage
---

## Problem Statement

**Écrit pendant que D.20a2 est implémentée**, sur l'arbre commis **`ca68736`** (D.20a1 faite). Seuls `grep`, `sed`, `git log/show` ont tourné. **Rien n'a été compilé ni lancé.** Tout ce que ce PRD suppose de l'issue de D.20a est une hypothèse `H-D20a-n` (plus bas), à vérifier sur l'arbre gradué de D.20a2 avant la mise en file.

**Numéros de ligne.** Ceux de `ca68736` pour `src/`. D.20a ne touche aucun fichier de `src/` (H-D20a-2), ils restent donc valables au départ de D.20b1. Pour `mo` et `ui`, ce sont les numéros prévus après D.20a2 (`mo:23`, `:96`, `:97`, `ui:54`, `:762`), à remesurer par contenu. Chaque sous-étape donne les numéros de son parent. L'implémenteur remesure par contenu et explique tout écart.

Le problème, vu du mainteneur du port : la Sortie de la phase D exige que `pico8_type_tag.h` disparaisse, et l'humain a tranché **Q13 = A** : « Le retirer ; on vérifie seulement la catégorie de l'objet ». Après D.20a, plus aucun TU ne **lit** le marqueur (`0x9E5C57C7`, premier mot de chaque entité). Il reste :

- dans `src/`, **71 lignes / 23 fichiers / 95 occurrences** : 15 includes, les 12 lignes de `pico8_type_tag.h`, 17 écritures, 23 tests, le membre `tag` et 3 lignes de commentaire de `pico8_struct.h` ;
- dans `tests/`, **5 lignes** : les initialiseurs de fausses entités de `mo` et `ui` et leurs deux includes. Ils écrivent le marqueur tant que `src/` le vérifie, et ne peuvent donc changer que dans le commit qui retire le membre.

**Fait capital pour « le jeu reste identique »** (vérifiable par grep, Annexe B) : dans `src/`, les 23 appels de `pico8_type_tag_of` sont **tous dans des `assert`**. Aucun `if` ne choisit un chemin selon l'étiquette, et `PICO8_TYPE_TAG_UNKNOWN` n'est lu nulle part dans `src/` hors de sa définition. Les écritures ne servent qu'à ces `assert`. Retirer l'étiquette ne change donc que deux choses : (1) ce que vérifient les `assert`, et (2) la disposition mémoire des entités (8 octets de moins en tête, voir « Taille des structs »). Le rendu, l'ordre des appels et les valeurs calculées ne dépendent ni de l'un ni de l'autre.

Ligne de l'étape, recopiée de `docs/plan-3.md` à `ca68736` (l. 295) :

> - D.20b — `type-tag`, `src/` : membre d'étiquette retiré (Q13 = A), `assert` d'étiquette → contrôles de classe, `pico8_type_tag.h` supprimé. Compte : `type-tag` = 0, gardé ; résidu D vide, Sortie de D atteinte. `class-coverage-net` : oui.

Ligne de l'étude D.14 (§3), recopiée :

> | **D.20b** | `src/` : membre `tag` retiré de `pico8_struct_header_t`, 17 écritures, 26 `assert` en tests de classe (ou retirés si le pointeur est typé), 15 includes, `pico8_type_tag.h` supprimé | ≈ 25 fichiers | `type-tag` = 0, dans `DATA_RESIDUE_GUARD` ; résidu D vide |

Écart à l'étude : **23** tests et non 26. Trois lecteurs sont partis depuis l'étude, dont les deux qu'elle liait à `valeur` : `ui_button.c:20` (`pico8_js_is_lq`, D.18b) et `probe.c:210` (B3). L'inventaire ci-dessous, mesuré, fait foi. 76 lignes et non « ≈ 72 » : les 5 lignes de `mo` et `ui` (D.20a, « Lecture » 6).

Legs reçus, relus ici :

- **D.14, §1.4** : « Aucune ligne de `src/` ne discrimine réellement […] Les 26 `pico8_type_tag_of` sont des `assert` qui gardent un cast de `void *` — éléments d'un conteneur de pointeurs […] ou `self` d'une méthode. »
- **Q13** : « (A) Le retirer ; on vérifie seulement la catégorie de l'objet, un peu moins robuste si la mémoire est abîmée. […] les vérifications mémoire automatiques couvrent ce risque. » Voir « Taille des structs » : cette dernière phrase n'est plus tout à fait vraie après le retrait (question ouverte 1).
- **D.20a** (PRD, « Pour D.20b ») : « `src/` 71 / 23 / 95 : 15 includes ; `pico8_type_tag.h` (12) ; 17 écritures […] ; 23 `assert`/tests […] ; le membre et 3 commentaires de `pico8_struct.h` (`:43`, `:44`, `:46`, `:125`). Dans `tests/`, `mo:23`, `:96`, `:97` et `ui:54`, `:762` (initialiseurs `{&cls}`, includes retirés). Les `FROZEN_STRUCT_TAG` des TU restent : ce sont des octets d'empreinte. `class-coverage-net` : oui. […] L'ensemble `L` de D.20b contiendra tout `pico8_type_tag.h`, supprimé. »
- **D.20a1** (note) : `L` = {`pico8_type_tag.h:51`} ; `fold_untagged` dans `g_` et `ga`. Rien de cela ne touche `src/`.
- **D.19c2** (note, précédent de suppression d'un module) : garde qui mord (M1), `tools/line_coverage_reference.txt` perd les lignes du fichier supprimé, `nm` pour la règle de lien, préprocesseur trié.
- **Règle des includes** (D.15b, D.18b, D.19a/b, D.20a) : un include part **avec le dernier usage du fichier**, dans le même commit.

### Hypothèses `H-D20a-n` (« D.20a faite »)

Chacune se vérifie par la commande donnée, sur l'arbre gradué `<g>` de D.20a2. `$T` est le motif `type-tag` de `tools/data_residue.sh:32` (recette plus bas).

| Hypothèse | Énoncé | Vérification | Effet si elle tombe |
|---|---|---|---|
| H-D20a-1 | `type-tag` **76 / 25 ; 100 occ.** : `src/` **71 / 23 / 95**, `tests/` **5 / 2 / 5** (`mo` 3, `ui` 2) ; résidu D **76 / 25** | `make -s data-residue` ; `sh tools/data_residue.sh src` et `… tests` ; recette des occurrences | départs, arrivées absolues, lignes 1–3 des « Runs », `FAILED:` de départ ; les deltas (− 30 / − 53 occ. ; − 46 / − 47 occ.) tiennent si H-D20a-2 et H-D20a-4 tiennent |
| H-D20a-2 | D.20a n'a touché aucun fichier de `src/`, ni le `Makefile`, ni `tools/` | `git diff --stat ca68736 <g> -- src Makefile tools` → vide | inventaire de `src/` à remesurer, écart rapporté |
| H-D20a-3 | Plus aucun TU ne lit le marqueur ni le membre | `git grep -nE 'pico8_type_tag_of\|PICO8_TYPE_TAG_UNKNOWN\|(->\|\.)tag\b' -- tests` → vide | arrêt : un lecteur de TU casserait à D.20b2 ; le rapporter, pas le corriger ici |
| H-D20a-4 | Les 5 lignes de `tests/` sont `mo` : include, `ra_only_t near_e = {{PICO8_TYPE_TAG_STRUCT, &g_ra_only_class}, NULL};`, idem `far_e` ; `ui` : include, `static giver_t g_giver = {{{PICO8_TYPE_TAG_STRUCT, &g_giver_class}, NULL, 0}};` | `git grep -nE "$T" -- tests` → 5 lignes | inventaire de `tests/` à remesurer |
| H-D20a-5 | `FROZEN_STRUCT_TAG` n'est défini que par `#define FROZEN_STRUCT_TAG 0x9E5C57C7u` et n'est lu que dans des plis ; aucun TU n'écrit `0x9E5C57C7` ailleurs | `git grep -hE '#define FROZEN_STRUCT_TAG' -- tests \| sort -u` → une ligne ; `git grep -n 0x9E5C57C7 -- tests \| grep -v '#define FROZEN_STRUCT_TAG 0x9E5C57C7u'` → vide | un alias du marqueur vivant disparaîtrait à D.20b2 et ferait bouger des empreintes : arrêt, rapporté |
| H-D20a-6 | `DATA_RESIDUE_GUARD` = `spec,overrides,stash,ob,dispatch,par-nom,liste,ref-audit,valeur` | `grep -n '^DATA_RESIDUE_GUARD' Makefile` | seule la ligne attendue du grep 8 de D.20b2 change |
| H-D20a-7 | 66 TU (`tests/test_pico8_*.c`), 26 e2e, aucun `*.sha256` ni fichier de `tests/e2e/` changé depuis `ca68736` | `ls tests/test_pico8_*.c \| wc -l` ; `git diff --stat ca68736 <g> -- tests/e2e` → vide | budget et balayage à recompter |
| H-D20a-8 | La note « Découvert à D.20a2 » existe, et son reste pour D.20b est celui de ce PRD | `grep -n '^- D.20a' docs/plan-3.md` | Annexe A et inventaire à reprendre |

### Sections à revérifier à la graduation de D.20a2

1. « État de départ », `FAILED:` de départ, lignes 1–3 des « Runs » (H-D20a-1).
2. Inventaire de `tests/` et numéros de `mo`/`ui` (H-D20a-4).
3. Grep 6 de D.20b2 (H-D20a-5).

### État de départ

| Mesure | À `ca68736` (D.20a1 faite) | Départ de D.20b1 (H-D20a-1) |
|---|---|---|
| `type-tag` (lignes / fichiers ; occ.) | 161 / 45 ; 241 | **76 / 25 ; 100** |
| `type-tag` `src/` | 71 / 23 ; 95 | **71 / 23 ; 95** |
| `type-tag` `tests/` | 90 / 22 ; 146 | **5 / 2 ; 5** |
| résidu D | 161 / 45 | **76 / 25** |
| `DATA_RESIDUE_GUARD` | `…,ref-audit,valeur` | inchangé |

Recette des occurrences (celle de D.12 à D.20a) :

```sh
T='\bpico8_type_tag(_of|_t)?\b|\bPICO8_TYPE_TAG_[A-Z0-9_]+|pico8_type_tag\.h'
F=$(find src tests \( -name '*.c' -o -name '*.h' \) -type f | sort)
printf 'type-tag\t%s lignes\t%s fichiers\t%s occ.\n' "$(cat $F | grep -cE "$T")" \
  "$(grep -lE "$T" $F | wc -l)" "$(cat $F | grep -oE "$T" | wc -l)"
for f in $F; do n=$(grep -cE "$T" $f); [ $n -gt 0 ] && \
  printf '%s\t%s\t%s\n' $f $n "$(grep -oE "$T" $f | wc -l)"; done
```

Par fichier à `ca68736` (lignes / occ.) : `bootstrap` 2/3, `economy` 3/4, `entity` 3/5, `entity_geom` 4/7, `help` 1/1, `ke` 3/4, `layout` 3/4, `menu` 1/1, `particle` 4/7, `planet` 1/1, `planet_display` 1/1, `planet_family` 1/1, `planet_family_trade` 2/2, `probe` 7/12, `research` 3/4, `research_icon` 1/1, `score_panel` 3/4, `slipway` 5/8, `struct.h` 5/5, `trade` 3/4, `type_tag.h` 12/13, `ui` 2/2, `wormhole` 1/1.

### Inventaire (71 lignes de `src/`, 5 de `tests/`)

Classes du message de commit :

- **(a1)** réécrite au même site, sans nom compté : moitié d'étiquette retirée d'un `assert` qui garde son test de classe ; initialiseur `{PICO8_TYPE_TAG_STRUCT, &cls}` → `{&cls}` ; commentaire réécrit ;
- **(a2)** retirée sans remplaçant : include ; `assert` d'étiquette entier ; écriture `header.tag = …` ; membre ;
- **(d)** partie avec le fichier supprimé (`pico8_type_tag.h`) ;
- **(b)** et **(c)** : **aucune**. Aucune signature ne change, et aucun mot d'étiquette n'est renommé : les `FROZEN_STRUCT_TAG` des TU ne sont pas comptés et ne bougent pas.

#### Les 23 tests d'étiquette — ce qui les remplace et pourquoi c'est sûr (D.20b1)

Règle (G, Q13 = A) :

- **R1 — le test de classe reste** : quand le même `assert` teste déjà la classe, seule la moitié d'étiquette part. La ligne garde son contrôle de classe, qui lit `->cls` ;
- **R2 — le type statique suffit** : quand le pointeur est déclaré `pico8_struct_header_t *` (ou un type d'entité), ou vient d'un registre que **seul** `pico8_lt(pico8_ke_t *)` remplit (`hv`, `cv`, `cg`, et ce qui en est filtré), ou est le `self` d'une méthode appelée par `e->cls->m(e, …)`, l'`assert` d'étiquette entier part. Il ne teste plus que « non NULL », et la ligne d'après lit de toute façon `->cls`. Aucun accesseur de classe générique n'existe dans `src/` (pas de registre de classes hors de `main_class_coverage.c`) : en créer un toucherait l'API, et c'est rejeté ;
- **R3 — NULL** : un `assert((x == NULL || tag(x)) && …)` sur un pointeur typé devient vrai quel que soit `x`. Il part entier (R2), sauf s'il porte aussi une classe (R1).

| Site (`ca68736`) | Pointeur, et d'où il vient | Règle | Après | Pourquoi c'est sûr |
|---|---|---|---|---|
| `bootstrap.c:61` | `qc` = `hv->items[i]`, `pico8_struct_header_t *` | R3 | `assert` retiré (2 lignes) | `hv` n'est rempli que par `pico8_lt(pico8_ke_t *)` (`entity.c:195`) |
| `economy.c:63` | `self`, `const pico8_hy_t *` | R1 | `assert((self == NULL \|\|` / `self->header.cls == &g_hy_struct_class) && …` | classe testée |
| `entity.c:160` | `e`, `const pico8_struct_header_t *` (`pico8_bm_has`, statique) | R2 | retiré (2 lignes) | seul appelant : `pico8_lt`, avec `&ke->header` |
| `entity.c:264` | `qc` = élément de `cv`, `pico8_struct_header_t *` | R2 | retiré (1 ligne) | `cv` n'est rempli que par `pico8_lt` (`entity.c:199`) |
| `entity_geom.c:31` | `e`, `const pico8_struct_header_t *` (`pico8_gk`) | R3 | retiré (2 lignes) | ses appelants (`help.c:36`, `planet.c:37`, `research_icon.c:203`, `ui_button.c:38`) passent le `self` d'une méthode `hd` appelée par `e->cls->hd(e, …)` |
| `entity_geom.c:65` | `self`, `pico8_struct_header_t *` (`pico8_na`) | R3 | retiré (2 lignes) | la ligne suivante asserte déjà la classe (`pico8_jo_is_family`) |
| `entity_geom.c:109` | `e` = élément de `j_`, puis `o`, `pico8_struct_header_t *` | R3 | retiré (2 lignes) | appelants : `cg("jo")` (`galaxy.c:67`), `cg("np")` (`probe.c:167`), `mz.ox` = `jp_ptrs(hv, …)` (`wormhole.c:51`, `:67`), tous remplis par `pico8_lt` |
| `ke.c:31` | `self`, `pico8_struct_header_t *`, NULL déjà rendu | R2 | retiré (2 lignes) | type statique |
| `layout.c:188` | `self`, `const pico8_lq_t *`, NULL déjà rendu | R1 | `assert(self->header.cls == &g_lq_struct_class &&` | classe testée |
| `particle.c:46` | `self`, `pico8_struct_header_t *`, NULL déjà rendu | R2 | retiré (2 lignes) | la ligne suivante asserte `self->cls->emitter_of != NULL` (catégorie « porte un émetteur ») |
| `particle.c:63` | idem | R2 | retiré (2 lignes) | type statique ; la ligne suivante lit `self->cls->emitter_of` |
| `particle.c:94` | idem | R2 | retiré (2 lignes) | ligne suivante : `emitter_of != NULL` asserté |
| `probe.c:154` | `self`, `pico8_struct_header_t *`, NULL déjà rendu | R2 | retiré (2 lignes) | ligne suivante : `self->cls->probe_of != NULL` asserté |
| `probe.c:200` | `et`, `pico8_struct_header_t *` | R1 | `assert((et == NULL \|\| g__is_target_class(et->cls)) &&` | classe testée |
| `probe.c:231` | `self`, comme `:154` | R2 | retiré (2 lignes) | `probe_of != NULL` asserté |
| `probe.c:326` | idem | R2 | retiré (2 lignes) | `probe_of != NULL` asserté |
| `probe.c:370` | `self` = `self_ptr` de `pico8_g__mm`, méthode `mm` | R3 | retiré (2 lignes) | appelée seulement par `self->cls->mm(self)` (`particle.c:71`) ; `emitter_of != NULL` asserté plus bas |
| `research.c:257` | `self`, `const pico8_hp_t *` | R1 | `(self->header.cls == &g_hp_struct_class)) &&` | classe testée |
| `score_panel.c:117` | `e` = élément de `cg("jo")`, `void *` | R2 | retiré (1 ligne) | `cg` n'est rempli que par `pico8_lt` (`entity.c:209`) |
| `slipway.c:150` | `self->probe->et` = `pico8_jc_ce(self)`, `pico8_struct_header_t *` | R2 | retiré (2 lignes) | type de retour ; `pico8_probe_et_hang_fo` choisit ensuite selon la classe |
| `slipway.c:282` | `w_val` = élément de `qu.oe`, `void *` | R1 | `assert(((const pico8_struct_header_t *)w_val)->cls == pico8_jc_struct_class() &&` | classe testée ; `oe` n'est rempli que par `pico8_jo_kf(…, pico8_struct_header_t *sw)` (`planet_trade.c:54`). Le cast était déjà sur la ligne suivante : il remonte, il n'est pas nouveau |
| `slipway.c:438` | `keys->qu`, `pico8_struct_header_t *` | R1 | `assert(jc_is_endpoint_class(keys->qu->cls) &&` | classe testée |
| `trade.c:56` | `self`, `const pico8_trade_t *` | R1 | `assert((self == NULL \|\|` / `self->header.cls == &g_trade_struct_class) && …` | classe testée |

**Bilan** : R1 × 7 (réécrites, (a1)), R2 × 11 et R3 × 5 (retirées, (a2)). **Aucun site ne reçoit un pointeur dont il ne peut pas connaître la nature** : chacun a un type statique d'entité, ou un seul écrivain typé. Il n'y a donc pas d'arrêt « site sans règle ». Les 16 `assert` retirés emportent 14 lignes de message non comptées (`entity.c:264` et `score_panel.c:117` tiennent sur une ligne), et parfois la ligne vide qui les suit.

**Forme des R1 (anti-tassement)** : on retire la moitié d'étiquette **sur sa ligne**, et la ligne de classe qui suit garde sa place. Elle ne perd qu'une parenthèse si le groupement l'exige. **Aucune ligne n'est jointe à une autre.** Les lignes non comptées modifiées sont déclarées (attendu ≈ 4 : `economy:64`, `research:257-258`, `trade:57`, `slipway:283`).

**Messages d'`assert`** : un message qui ne dit que « no class instance reaches g_'s functions any more » part avec son `assert`. C'est du récit de chantier (`CLAUDE.md`, « Commentaires »). Un message d'`assert` R1 garde son texte.

#### Includes (15 dans `src/`, 2 dans `tests/`)

| Fichier | Dernier usage après D.20b1 | Sous-étape |
|---|---|---|
| `bootstrap.c:10`, `entity_geom.c:5`, `particle.c:15` | leurs `assert` (D.20b1) | **D.20b1** |
| `help.c:17`, `planet.c:25`, `planet_display.c:24`, `planet_family.c:20` | **aucun** (include seul, déjà sans usage) | **D.20b1** |
| `economy.c:17`, `ke.c:10`, `planet_family_trade.c:18`, `research.c:12`, `score_panel.c:24`, `trade.c:9`, `ui.c:16` | une écriture | D.20b2 |
| `pico8_struct.h:8` | le membre | D.20b2 |
| `tests/test_pico8_mo.c:23`, `tests/test_pico8_ui.c:54` | l'initialiseur | D.20b2 |

**Piège** (legs de D.20a) : `pico8_struct.h` inclut `pico8_type_tag.h` jusqu'à D.20b2. Un fichier sans son include **voit encore** le module, et le compilateur ne signale pas un reste : seule la grep le fait. `pico8_type_tag.h` apporte aussi `<string.h>`. Vérifié à `ca68736` : les 14 `.c` qui l'incluent et qui appellent `mem*`/`str*` (`help`, `planet`, `planet_display`, `research`, `score_panel`, `ui`) incluent `<string.h>` eux-mêmes, et `pico8_struct.h` l'inclut aussi. **Aucun include n'est attendu en gain.** Tout include gagné est déclaré au message, avec l'erreur du compilateur qui l'exige.

#### Écritures (17) et membre (D.20b2)

- **8 affectations** `…header.tag = PICO8_TYPE_TAG_STRUCT;` retirées (a2) : `economy.c:27`, `ke.c:38`, `layout.c:305`, `layout.c:413`, `research.c:158`, `research_icon.c:55` (`view.header.tag`), `slipway.c:132` (`point->header.tag`), `trade.c:34`. Chacune est suivie de `…header.cls = …;`, qui reste.
- **9 initialiseurs** `{PICO8_TYPE_TAG_STRUCT, &g_X_struct_class}` → `{&g_X_struct_class}` (a1) : `entity.c:47`, `menu.c:52`, `planet_family_trade.c:56`, `probe.c:31`, `probe.c:41`, `score_panel.c:67`, `slipway.c:31`, `ui.c:40`, `wormhole.c:26`. Même forme dans `tests/` : `mo:96`, `mo:97` → `{{&g_ra_only_class}, NULL}` ; `ui:762` → `{{{&g_giver_class}, NULL, 0}}`.
- **`pico8_struct.h`** : `:46` `pico8_type_tag_t tag;` retiré (a2). Le commentaire `:42-44` devient une phrase sans nom compté (« First member of every entity struct, and of the parent struct a child embeds first. ») : `:43` réécrite (a1), `:44` retirée (a2). C'est un contrat C que le nom ne dit pas, puisque tout cast `pico8_X_t *` ↔ `pico8_struct_header_t *` en dépend. `:124-125` perd la proposition « whose first member is no tag, so pico8_type_tag_of() reads it UNKNOWN » : `:125` réécrite (a1), sans nom compté.
- **`pico8_type_tag.h`** supprimé : 12 lignes (d).

**Pourquoi c'est sûr** : le compilateur tient les 17 écritures. Avec le membre retiré, une affectation oubliée ne compile pas (`no member named 'tag'`). Un initialiseur oublié `{PICO8_TYPE_TAG_STRUCT, &cls}` mettrait un entier dans le pointeur `cls` et un pointeur en trop, deux erreurs sous `-Werror`. Un initialiseur réécrit `{0, &cls}` serait aussi rejeté. La seule faute que le compilateur ne verrait pas serait un `{&mauvaise_classe}` : la mutation Mc2 la couvre.

#### Totaux

| Sous-étape | Lignes sorties (occ.) | (a1) | (a2) | dont includes | (d) | (b) | (c) |
|---|---|---|---|---|---|---|---|
| D.20b1 | 30 (53) | 7 | 23 (16 `assert`, 7 includes) | 7 | 0 | 0 | 0 |
| D.20b2 `src/` | 41 (42) | 11 (9 initialiseurs, `struct.h:43`, `:125`) | 18 (8 includes, 8 affectations, `struct.h:44`, `:46`) | 8 | 12 (13 occ.) | 0 | 0 |
| D.20b2 `tests/` | 5 (5) | 3 | 2 | 2 | 0 | 0 | 0 |
| **D.20b** | **76 (100)** | **21** | **43** | **17** | **12** | **0** | **0** |

Chaque ligne sort de `type-tag`, et aucune n'en gagne. **Δlignes > Δocc.** : aucun fichier. Les tests portent 2 occurrences, `type_tag.h:41` en porte 2, les autres lignes 1. **Lignes comptées modifiées qui gardent le motif** : aucune.

## Solution

**Deux commits** `refactor(D.20b1)` puis `refactor(D.20b2)`, chacun vert seul, vérifié et poussé, dans cet ordre. b2 dépend de b1 : un lecteur d'étiquette restant lirait, sans le membre, la moitié basse du pointeur `cls`.

- **D.20b1 — plus aucun lecteur** : les 23 tests deviennent le test de classe qu'ils précédaient (R1) ou partent (R2, R3). Les 7 includes sans autre usage partent. Aucune écriture, aucun membre, aucun TU, aucun `Makefile` ne change. Dès ce commit, `pico8_type_tag_of` n'a plus d'appelant : `nm` le montre absent de tout objet.
- **D.20b2 — plus de marqueur** : le membre `tag` et ses 17 écritures, les 8 derniers includes de `src/`, `pico8_type_tag.h`, les 5 lignes de `mo` et `ui`, les commentaires de `pico8_struct.h`. `type-tag` entre dans `DATA_RESIDUE_GUARD`, et `tools/line_coverage_reference.txt` perd ses lignes de `pico8_type_tag.h`. Le glossaire, TROU-6 et le plan sont mis à jour.

Du point de vue du joueur : **rien**. Même image, frame par frame, sur les 26 e2e.

**Sortie de l'étape**, constatée par commande sur chaque commit :

- `make test` vert (dont `data-guard`, `constants-check`, `float-grep`, `float-guard` ; `tools/float_guard.txt` inchangé, `garde fichiers 57`) ; `make e2e` **26 conformes** (les `*.json.sha256` sont les empreintes des framebuffers : conformes = images identiques) ; `make sanitize-net` et `make class-coverage-net` verts, attendus inchangés ; balayage ASan+UBSan des 66 TU propre ;
- `make data-residue` :

| Mesure | Départ (H-D20a-1) | Après D.20b1 | Après D.20b2 |
|---|---|---|---|
| `type-tag` | 76 / 25 ; 100 occ. | **46 / 18 ; 47** | **0** (plus de ligne) |
| `type-tag` `src/` | 71 / 23 ; 95 | 41 / 16 ; 42 | 0 |
| `type-tag` `tests/` | 5 / 2 ; 5 | 5 / 2 ; 5 | 0 |
| résidu D | 76 / 25 | **46 / 18** | **0** : seule ligne `résidu D : 0 ligne(s), 0 fichier(s)` |
| `DATA_RESIDUE_GUARD` | `…,valeur` | inchangé | **`…,valeur,type-tag`** |

  Si le départ réel diffère : départ réel − 30 / − 53 occ., puis tout le reste.

- **Greps de D.20b1** (`<parent>` = le parent du commit). Chaque sortie « vide » exclut **nommément** ce que l'étape garde :

  ```sh
  T='\bpico8_type_tag(_of|_t)?\b|\bPICO8_TYPE_TAG_[A-Z0-9_]+|pico8_type_tag\.h'
  # 1. plus aucun appel de pico8_type_tag_of : restent sa définition et les deux commentaires de struct.h
  git grep -n 'pico8_type_tag_of' -- src tests | grep -vE '^src/pico8_type_tag\.h:41:|^src/pico8_struct\.h:(43|125):'   # vide
  # 2. restent exactement les écritures, les includes de b2, le membre et l'en-tête
  git grep -nE "$T" -- src | grep -vE '^src/pico8_type_tag\.h:|^src/pico8_struct\.h:(8|43|44|46|125):|header\.tag = PICO8_TYPE_TAG_STRUCT;|\{PICO8_TYPE_TAG_STRUCT, &g_[a-z0-9_]+_struct_class\}|^src/pico8_(economy|ke|planet_family_trade|research|score_panel|trade|ui)\.c:[0-9]+:#include "pico8_type_tag\.h"$'   # vide
  # 3. périmètre : src/ seulement (et le plan)
  git diff --stat <parent> -- Makefile tools tests      # vide
  git diff --name-only <parent> | grep -vE '^src/pico8_(bootstrap|economy|entity|entity_geom|help|ke|layout|particle|planet|planet_display|planet_family|probe|research|score_panel|slipway|trade)\.c$|^docs/plan-3\.md$'   # vide
  # 4. aucune ligne ajoutée ne nomme le module, aucun cast nouveau hors du cast remonté de slipway
  git diff <parent> -- src | grep -E '^\+[^+]' | grep -E "$T"        # vide
  git diff <parent> -- src | grep -E '^\+[^+]' | grep -E '\(\s*(const\s+)?[a-z0-9_]+_t\s*\*\s*\)' | grep -vF '((const pico8_struct_header_t *)w_val)->cls == pico8_jc_struct_class()'   # vide
  # 5. includes : 7 retirés, aucun ajouté
  git diff <parent> -- src | grep -E '^[-+]#include'   # 7 lignes '-#include "pico8_type_tag.h"', aucune '+'
  # 6. comptes
  make -s data-residue; sh tools/data_residue.sh --detail src | grep '^type-tag'
  ```

- **Greps de D.20b2** :

  ```sh
  # 7. plus aucune trace du module dans les fichiers suivis, hors motif du compteur et historique de docs/
  git grep -nE "$T|type_tag|0x9E5C57C7" -- . ':!docs' ':!tools/data_residue.sh' | grep -vE '^tests/test_pico8_[a-z0-9_]+\.c:[0-9]+:#define FROZEN_STRUCT_TAG 0x9E5C57C7u$'   # vide
  git grep -nE '(->|\.)tag\b' -- src tests | grep -vE '^src/pico8_entity\.c:[0-9]+:.*g_cg\[[a-z_]+\]\.tag\b'   # vide (g_cg[i].tag est le nom d'un sac de cg, pas le marqueur)
  ls src/pico8_type_tag.h                              # « No such file »
  # 8. Makefile : la seule ligne de garde change
  git diff <parent> -- Makefile | grep -E '^[-+][^-+]'
  #   -DATA_RESIDUE_GUARD := spec,overrides,stash,ob,dispatch,par-nom,liste,ref-audit,valeur
  #   +DATA_RESIDUE_GUARD := spec,overrides,stash,ob,dispatch,par-nom,liste,ref-audit,valeur,type-tag
  # 9. l'en-tête d'entité n'a plus qu'un membre
  sed -n '/^typedef struct {$/,/} pico8_struct_header_t;/p' src/pico8_struct.h | grep -cE '^\s+[a-z].*;'   # 1 (const pico8_struct_class_t *cls;)
  # 10. tests/ : seuls mo et ui changent, 2 lignes retirées, 3 réécrites ; aucune constante figée ne bouge
  git diff --stat <parent> -- tests                    # tests/test_pico8_mo.c, tests/test_pico8_ui.c
  git diff <parent> -- tests | grep -E '^[-+].*(_HASH|identity\[|FROZEN_|VIEW_[A-Z_]+ =|TABLE_INDEX_KEY =|_KIND = )'   # vide
  # 11. référence de couverture : seules partent les lignes du fichier supprimé
  git diff <parent> -- tools/line_coverage_reference.txt | grep '^-[^-]' | grep -vc '^-src/pico8_type_tag\.h:'   # 0
  git diff <parent> -- tools/line_coverage_reference.txt | grep -c '^+[^+]'   # 0
  cut -d: -f1 tools/line_coverage_reference.txt | sort -u | while read f; do [ -f "$f" ] || echo "$f"; done   # vide
  # 12. périmètre
  git diff --name-only <parent> -- tests/e2e; git diff --name-only <parent> | grep -E '\.sha256$'   # vide
  git diff --name-only <parent> | grep -vE '^src/|^tests/test_pico8_(mo|ui)\.c$|^Makefile$|^tools/line_coverage_reference\.txt$|^docs/(plan-3|glossaire-noms|trous-de-portage)\.md$'   # vide
  # 13. docs vivantes : plus de nom du module
  git grep -nE "$T" -- docs/glossaire-noms.md docs/trous-de-portage.md   # vide
  # 14. comptes : la Sortie de D
  make -s data-residue                                 # résidu D : 0 ligne(s), 0 fichier(s)
  ```

  (Grep 7 : `docs/` est exclu parce que `plan-2.md`, `plan-3.md` et les PRD racontent l'historique du module ; `tools/data_residue.sh:32` porte le motif du compteur. La légende du `Makefile` ne nomme pas le module, vérifié à `ca68736` : aucune exclusion du `Makefile` n'est donc nécessaire. Si une ligne de légende ou de commentaire apparaît quand même, c'est un écart **rapporté** et non une exclusion ajoutée en silence.)

## User Stories

1. En tant que mainteneur du port, je veux que `pico8_type_tag.h` n'existe plus, afin que la Sortie de la phase D soit atteinte.
2. En tant que mainteneur, je veux que l'en-tête d'entité ne porte plus que sa classe, afin qu'il n'y ait plus qu'un seul contrôle, celui de la catégorie (Q13 = A).
3. En tant que gardien du jeu, je veux que les 26 e2e rendent les mêmes images, afin de prouver que retirer le marqueur ne change rien au jeu.
4. En tant que gardien du jeu, je veux la preuve écrite que le marqueur n'était lu que dans des `assert`, afin que « le jeu reste identique » soit un fait vérifiable et non une supposition.
5. En tant que gardien des `assert`, je veux, pour chacun des 23 tests d'étiquette, la règle qui le remplace et la raison de sa sûreté (type statique, NULL, classe), afin qu'aucun contrôle utile ne soit perdu.
6. En tant que gardien des `assert`, je veux qu'un test de classe qui suivait l'étiquette dans le même `assert` reste en place, afin que Q13 = A soit appliquée à la lettre.
7. En tant que gardien des `assert`, je veux une mutation par test de classe restant, avec la règle du parent, afin de prouver que chacun s'exécute et mord encore.
8. En tant que gardien de la mémoire, je veux savoir ce que le retrait du membre change à la taille des structs et ce qui en dépend, afin qu'aucune lecture brute d'une entité ne soit cassée.
9. En tant que gardien de la mémoire, je veux ASan+UBSan sur les 66 TU et `sanitize-net` sur les 26 e2e, afin qu'aucun `->cls` ne soit lu hors d'un objet.
10. En tant que gardien des constructeurs, je veux `class-coverage-net` inchangé, afin de prouver que les constructeurs dont l'en-tête change instancient toujours les mêmes classes.
11. En tant que vérificateur, je veux `gcov` sur les 26 e2e avec un ensemble `L` défini par appariement de lignes, afin de voir qu'aucune ligne exécutée hors de celles retirées ne cesse de l'être.
12. En tant que vérificateur, je veux `nm` (et `nm -u`) avant et après, afin que la règle de lien de `DATA_ENGINE_SRCS` et des autres listes soit mesurée, et non supposée.
13. En tant que vérificateur, je veux le préprocesseur trié sur chaque `.c`, afin de prouver que les includes retirés n'emportent que le module.
14. En tant que gardien du compteur, je veux que `type-tag` entre dans `DATA_RESIDUE_GUARD` et que la garde morde, afin que le marqueur ne revienne pas.
15. En tant que gardien des empreintes, je veux qu'aucune constante figée de TU ne change et que les `FROZEN_STRUCT_TAG` restent, afin que les empreintes ne dépendent plus d'aucun marqueur vivant.
16. En tant que gardien du compteur, je veux au message de chaque commit les comptes par catégorie et par fichier, en lignes et en occurrences, et la classification ligne à ligne, afin qu'aucune baisse ne soit un tassement.
17. En tant que `plan-pilot`, je veux deux sous-étapes d'environ 40 à 50 minutes, un plafond et un repli écrit, afin de finir la phase sans commit trop gros.
18. En tant que lecteur du plan, je veux que la note de D.20b2 ferme D (Sortie constatée) et que le bloc « Reste de D » perde sa ligne `type-tag`, afin que la revue de sortie de phase parte d'un état exact.
19. En tant qu'utilisateur, je veux qu'on me propose la mise à jour de `CLAUDE.md` sans l'appliquer, afin de rester maître des consignes du projet.

## Implementation Decisions

Pas d'humain dans la boucle. Le skill `to-prd` demande de faire valider modules, tests et découpage. Ces choix sont tranchés ici comme le ferait un agent `guideline` (hypothèses explicites, simplicité, changement chirurgical, orphelins retirés, critères vérifiables) et marqués **(G)**. Q13 (A) est une décision humaine. L'étude D.14 fait foi, sauf écart déclaré : 23 tests et non 26, 5 lignes de TU, deux sous-étapes, la référence de couverture.

### Modules

- **D.20b1** : `src/pico8_{bootstrap,economy,entity,entity_geom,help,ke,layout,particle,planet,planet_display,planet_family,probe,research,score_panel,slipway,trade}.c` (16 fichiers) ; `docs/plan-3.md` (note).
- **D.20b2** : `src/pico8_struct.h`, `src/pico8_type_tag.h` (supprimé), `src/pico8_{economy,entity,ke,layout,menu,planet_family_trade,probe,research,research_icon,score_panel,slipway,trade,ui,wormhole}.c` (14) ; `tests/test_pico8_{mo,ui}.c` ; `Makefile` (une ligne) ; `tools/line_coverage_reference.txt` ; `docs/glossaire-noms.md` (entrée « Le tag de type ») ; `docs/trous-de-portage.md` (TROU-6, dernière phrase) ; `docs/plan-3.md` (note, bloc « Reste de D »).
- **Inchangés** : les 64 autres TU, `tests/e2e/`, tout `*.sha256`, `tools/` hors de la référence de couverture (dont `tools/data_residue.sh`, `tools/class_coverage_net_expected.txt`, `tools/float_guard.txt`), les listes de sources du `Makefile`, `CLAUDE.md`.

### Représentation (G)

- `typedef struct { const pico8_struct_class_t *cls; } pico8_struct_header_t;`. Le type et son nom restent : 124 lignes de `src/` lisent `.cls` ou `->cls`, et le renommer serait hors sujet.
- Un initialiseur positionnel d'en-tête s'écrit `{&g_X_struct_class}`. Aucun passage aux initialiseurs désignés : ce serait une réécriture sans objet.
- Pas de remplaçant du marqueur (Q13 = A) : ni champ, ni mot de garde, ni `static_assert` sur un premier mot.
- `FROZEN_STRUCT_TAG` reste dans les TU qui le définissent (octet d'empreinte gelé par D.20a). Rien à faire.

### Taille des structs (G) — ce que le retrait du membre change

- **Avant** : `{uint32 tag; [4 octets de remplissage]; const cls *cls;}` = **16 octets** sur x86_64 (LP64), `cls` à l'offset **8**. **Après** : `{const cls *cls;}` = **8 octets**, `cls` à l'offset **0**. Toute struct d'entité commence par l'en-tête : 17 structs de `src/` le déclarent en premier membre (`git grep -nE '^\s*pico8_struct_header_t header;' -- src`), une de `mo`, et les autres l'ont par `pico8_ke_t ke;` ou la struct parente. Chacune **perd exactement 8 octets**, puisque l'alignement de l'en-tête (8) ne change pas. Sur une cible 32 bits (PSX, en pause), l'en-tête passerait de 8 à 4 octets.
- **Ce qui en dépend**, relevé par grep à `ca68736` et à reconfirmer :
  - les `malloc(sizeof *self)` des constructeurs suivent seuls ;
  - les 9 vues statiques et les vues sur la pile (`pico8_ie_ke_view`, rendue par valeur) sont réécrites ou suivent seules ;
  - les casts `pico8_X_t *` ↔ `pico8_struct_header_t *` (l'en-tête reste à l'offset 0) ne changent pas ;
  - **aucune lecture brute** d'une entité ne reste une fois `pico8_type_tag_of` parti (`memcpy` du premier mot). Aucun `memcmp` ni pli d'octets sur une entité dans `src/` ou `tests/` : le seul `memcmp` de struct des TU (`hp:685`) porte sur `pico8_jy_t`, qui n'est pas une entité ; aucun `offsetof` dans `src/` (`data-guard`) ;
  - les empreintes des TU plient des valeurs de membres, et le mot d'étiquette y est gelé depuis D.20a. Les `*.sha256` sont des empreintes de framebuffer. **Aucune empreinte ne dépend de la disposition.**
- **Mesure exigée** (D.20b2) : une sonde jetable dans le scratchpad (hors de `src/` et `tests/`, jamais commise) affiche `sizeof(pico8_struct_header_t)`, `offsetof(pico8_struct_header_t, cls)` et le `sizeof` de chaque type de `git grep -nE 'pico8_struct_header_t header;|pico8_ke_t ke;' -- src/*.h`, au parent et à l'arrivée. Attendu : 16 → 8 ; 8 → 0 ; chaque entité − 8. Un autre écart est rapporté et expliqué.
- **Conséquence pour les filets** (question ouverte 1) : avant, lire `->cls` sur un petit objet pris à tort pour une entité (un `pico8_vec_t` de 8 octets) débordait à l'offset 8, et ASan le voyait. Après, `cls` est à l'offset 0, **dans** le petit objet. ASan ne voit plus cette confusion-là : on lit un faux pointeur de classe, et le plantage vient plus tard. Le type C et les écrivains uniques (table des 23 sites) sont désormais la garde. Déclaré au message de D.20b2 et dans l'entrée du glossaire.

### Commentaires (G)

- `pico8_struct.h:42-44` et `:124-125` : voir « Écritures et membre ». Rien d'autre.
- Aucun commentaire neuf ne nomme D.20, le plan, Q13, ni le marqueur disparu : la raison va au message de commit (`CLAUDE.md`).

### `Makefile`, `tools/` (G)

- `DATA_RESIDUE_GUARD` gagne `,type-tag` (D.20b2). Toutes les catégories de `tools/data_residue.sh` sont alors gardées.
- **Aucune liste de sources ne change** : `pico8_type_tag.h` est un en-tête sans `.c`, et aucune cible ne le nomme. **Règle de lien** : aucun objet d'une liste de lien (`ENGINE_SRCS`, `DATA_ENGINE_SRCS`, listes explicites des TU, `main_raylib`) ne référence un symbole défini hors de sa liste. Elle tient par construction, puisque `pico8_type_tag_of` est `static inline` et qu'aucun symbole externe n'est ajouté ni retiré. Elle est **mesurée** par `nm` (Testing Decisions).
- `tools/line_coverage_reference.txt` perd ses lignes `src/pico8_type_tag.h:*` (6 à `ca68736`), et elles seules, comme D.19c2. La référence n'est pas recalculée : ses numéros d'autres fichiers sont historiques.
- `tools/float_guard.txt` : réécrit par `make test`, attendu **inchangé** (`garde fichiers 57`, aucun `.c` supprimé). Un écart est rapporté.

### Docs (G)

- **`docs/glossaire-noms.md`**, entrée « Le tag de type » : réécrite pour dire l'état. L'en-tête d'entité ne porte que sa classe, et les `assert` d'entrée testent la classe (Q13). Le piège reste : une confusion de représentation a produit quatre débordements de tas, et, sans le mot de tête, lire `->cls` sur un non-entité ne déborde plus.
- **`docs/trous-de-portage.md`**, TROU-6 : la parenthèse « (voir le tag de type, `src/pico8_type_tag.h`) » renvoie à l'entrée du glossaire, sans le chemin supprimé.
- **`docs/plan-3.md`** : note « Découvert à D.20b1 », puis « Découvert à D.20b2 (clôt D) » en fin de ligne D.20b. Le bloc « Reste de D » perd sa puce `type-tag` ; les autres puces y restent pour la revue de sortie. Rien d'autre.
- `docs/plan-2.md` et les PRD (historique) ne changent pas.

### Anti-tassement et message de commit (liant)

- **Interdit** : joindre des lignes (comptées ou non) ; renommer un identifiant vivant pour échapper au motif ; alias (`#define`, constante ou champ qui redonne le marqueur ou le lit sous un autre nom) ; cast nouveau (hors du cast **remonté** de `slipway.c:283` → `:282`) ; lire le premier mot d'une entité par un autre chemin (`memcpy`, `*(const uint32_t *)`) ; accesseur de classe nouveau ; toucher une constante figée de TU, `tests/e2e/`, un `*.sha256`, une liste de sources ; toucher un TU autre que `mo` et `ui` ; toucher `CLAUDE.md`.
- **Classification exigée** (recette : `git diff -U0 <parent> -- <fichier>`, lignes `-` qui portent le motif, en regard des `+`) : D.20b1 **(a1) 7, (a2) 23** ; D.20b2 **(a1) 14, (a2) 20, (d) 12**. Recomptée au parent, ligne à ligne, avec les numéros du parent, et avec la règle (R1/R2/R3) de chaque test.
- **Mesure exigée**, avant et après, par catégorie (`type-tag`, et les neuf gardées à 0) et par fichier du diff, en lignes (`--detail`) **et** en occurrences.
- **Lignes qui gagnent une catégorie** : attendu aucune. **Lignes non comptées retirées ou modifiées** : nombre par fichier (b1 : les 14 lignes de message, les lignes vides, les ≈ 4 continuations R1).
- **Includes** : retirés, gagnés (attendu aucun), avec la preuve du préprocesseur.
- **Δlignes > Δocc.** : attendu aucun fichier.

### Empreinte et découpage (G)

| Commit | Chemins | `type-tag` retirées (l. / occ.) | Includes | Coût attendu |
|---|---|---|---|---|
| **D.20b1** | 16 `.c` + plan | 30 / 53 | 7 | ≈ 35–40 min |
| **D.20b2** | 14 `.c`, `struct.h`, `type_tag.h`, 2 TU, `Makefile`, référence, 3 docs | 46 / 47 | 10 | ≈ 45–50 min |

- **Pourquoi deux, et dans cet ordre** : le membre, ses 17 écritures et les 3 initialiseurs de TU doivent partir **ensemble**, puisque le compilateur refuse tout état intermédiaire. Les lecteurs, eux, se retirent seuls. b1 retire ce qui peut être prouvé à part (les `assert`, avec leurs mutations). b2 retire ce que le compilateur tient (le membre), avec la preuve de disposition. **Rejeté** : un seul commit (76 lignes dans 25 fichiers, plus la sonde de taille, `gcov` apparié, `class-coverage-net`, `sanitize-net` et le balayage : au-delà de 50 min) ; trois sous-étapes (b2 ne se coupe proprement qu'en repli).
- **Rythme de b1** : lecture des 23 sites ≈ 5 min, éditions ≈ 8, préprocesseur et `nm` ≈ 3, mutations Mb ≈ 6, `gcov` départ et arrivée ≈ 5, filets ≈ 8, mesures, classification, message et note ≈ 5.
- **Rythme de b2** : éditions ≈ 8, sonde de taille ≈ 4, préprocesseur et `nm` ≈ 3, mutations Mc ≈ 5, `gcov` ≈ 5, filets (`make test`, `e2e`, `sanitize-net`, `class-coverage-net`, balayage) ≈ 10, docs ≈ 5, mesures, message et note ≈ 5.
- **Plafond 50 min par sous-étape. Repli écrit** : au plafond sans commit, rien n'est commis, et `plan-pilot` relance la sous-étape coupée en deux, sans pont. **D.20b1** devient (`probe`, `particle`, `entity_geom`, `bootstrap` : 16 lignes, R2/R3, avec les 4 includes de ces fichiers et des 4 fichiers à include seul) puis (le reste : 14 lignes, dont les 7 R1). **D.20b2** devient **b2a** (membre, 17 écritures, `mo:96-97`, `ui:762`, commentaires de `struct.h` ; l'en-tête et ses includes restent, sans lecteur ni écrivain) puis **b2b** (les 10 includes, `pico8_type_tag.h`, garde, référence de couverture, docs). Les comptes sont recomptés par commit.

### Ce qui ne change pas

- Le comportement du jeu : aucun appel, aucun ordre, aucune valeur. Seuls changent des `assert` (plus faibles d'un test de NULL ou d'étiquette, jamais d'un test de classe) et la taille des entités.
- `tests/e2e/`, tout `*.sha256`, toute constante `*_HASH`/`identity[]`, `FROZEN_*`, `VIEW_*`, `TABLE_INDEX_KEY`, `*_KIND` ; les 64 TU autres que `mo` et `ui`.
- Les listes de sources du `Makefile`, `tools/data_residue.sh` et ses motifs figés.

### Hypothèses d'exécution (rien n'a été compilé ni lancé)

- **H1** : `hv`, `cv`, `cg` ne sont remplis que par `pico8_lt` (`entity.c:195`, `:199`, `:209`), et `jo.oe` que par `pico8_jo_kf` (`planet_trade.c:54`). Vérification : `git grep -n 'pico8_ptrs_add(&g_hv\|g_cv\[\|bucket->list\|pico8_ptrs_add(oe' -- src`.
- **H2** : aucun `.c` ne tire `<string.h>` du seul `pico8_type_tag.h` (vérifié par lecture des includes à `ca68736`) ; le préprocesseur et le compilateur le confirment.
- **H3** : sans usage, les variables locales ne deviennent pas orphelines. `qc` (`bootstrap`, `entity`), `e`/`o` (`entity_geom`) et `w_val` sont lus après l'`assert`. Un orphelin sous `-Werror` est retiré et déclaré.
- **H4** : `pico8_type_tag.h:41-51` n'est exécuté par les e2e qu'au travers des 23 `assert` de `src/` (D.20a a retiré les appels des TU). Donc `L` de b1 ⊆ `pico8_type_tag.h`, et `L` de b2 = ∅.
- **H5** : aucun e2e n'atteint un `assert` d'étiquette avec un non-entité (sinon il échouerait aujourd'hui). Retirer l'étiquette ne peut donc faire passer aucun e2e de rouge à vert ni l'inverse.

## Implementation Chunks

**D.20b1** (un commit)

1. Départ : graduation de D.20a2 ou un descendant qui ne touche que `docs/`, arbre propre (H-D20a-1 à H-D20a-8). Recette, `--detail`, et les 30 lignes remesurées par contenu.
2. Préprocesseur (départ), `nm` (départ), `gcov` e2e (départ).
3. Éditions, fichier par fichier : `probe`, `particle`, `entity_geom`, `bootstrap`, `entity`, `ke`, `score_panel`, `slipway` (R2/R3 et R1), puis `economy`, `layout`, `research`, `trade` (R1), puis les includes de `help`, `planet`, `planet_display`, `planet_family`. Après chaque fichier, `make tmp/obj/src/pico8_<f>.o`.
4. `make test`, `make e2e` ; préprocesseur, `nm`, `gcov` (arrivée) ; mutations Mb1–Mb2 ; `sanitize-net`, `class-coverage-net`, balayage ASan+UBSan ; mesures, classification ; note ; **commit**, vérification, push.

**D.20b2** (un commit)

1. Départ : D.20b1, arbre propre ; `type-tag` 46 / 18 / 47. Sonde de taille (parent), préprocesseur, `nm`, `gcov` (départ).
2. `pico8_struct.h` (membre, include, commentaires), puis les 8 affectations, les 9 initialiseurs, `mo` et `ui`, les 7 includes restants, la suppression de `pico8_type_tag.h`. `make test` doit compiler dès ce point : une écriture oubliée est une erreur de compilation.
3. `DATA_RESIDUE_GUARD`, référence de couverture, glossaire, TROU-6.
4. Sonde de taille (arrivée), préprocesseur, `nm`, `gcov` ; mutations Mc1–Mc3 ; filets ; mesures ; note « clôt D » et bloc « Reste de D » ; **commit**, vérification, push.

## Testing Decisions

Un bon test juge ce que le code rend ou fait, pas la présence d'une ligne. D.20b ne change aucun chemin exécuté du jeu. La preuve est donc : **mêmes images** (26 e2e), **mêmes lignes exécutées** hors de celles retirées (`gcov`), **mêmes symboles** (`nm`), **même texte compilé** hors du module (préprocesseur), **mémoire propre** (ASan+UBSan, `sanitize-net`), **mêmes instanciations** (`class-coverage-net`). Les mutations prouvent que chaque contrôle de classe restant mord, que les vues statiques lisent bien leur classe et que la garde mord.

### TU touchés

- D.20b1 : **aucun**. Tous les TU sont reconstruits contre le nouveau `src/`, et tous passent.
- D.20b2 : `mo`, `ui` (initialiseurs et includes). Leurs assertions et leurs empreintes ne changent pas.
- Aucun TU n'est ajouté. Une assertion n'est ajoutée que si une mutation exigée ne rougit rien alors qu'elle rougit au parent (chemin perdu).
- Aucun test du §6 n'est touché (`planet_gh_row_lq`, `ga_export_panel_by`, `layout_nil_item_draws_nothing`). Il n'y a donc pas de preuve rouge à refaire, mais ils doivent passer.
- Prior art : suppression d'un module avec sa garde (D.8, D.12d, D.13, D.19c2) ; `assert` de classe (D.11d) ; sonde de disposition jetable (la sonde d'étiquette de D.20a).

### Préprocesseur (b1 et b2)

Dans le scratchpad, pour chaque `src/*.c` et `tests/*.c` présent au départ et à l'arrivée : `cc -std=c99 -E -P <f> | sed -E 's/[0-9]+/N/g' | grep -v '^[[:space:]]*$' | sort`.

- b1 : (i) `comm -13 départ arrivée` ne contient que les 7 lignes R1 réécrites (forme développée par `assert`) ; (ii) `comm -23` ne contient que les lignes des 23 `assert` d'origine. Les 4 fichiers à include seul (`help`, `planet`, `planet_display`, `planet_family`) ont une sortie triée **identique**.
- b2 : (i) `comm -13` ne contient que les 12 initialiseurs réécrits (9 de `src/`, 3 des TU) ; (ii) chaque ligne de `comm -23` est une ligne de `cc -std=c99 -E -P src/pico8_type_tag.h` au parent, le membre `pico8_type_tag_t tag;`, une affectation `header.tag = …` ou un initialiseur d'origine.
- Nombre de fichiers dont la sortie change, et lignes perdues ou gagnées, au message. Toute autre différence : arrêt, rapporté.

### `nm` — règle de lien (b1 et b2)

Après `make test`, sur les objets dont la source existe (`for s in src/*.c tests/*.c; do o=tmp/obj/${s%.c}.o; [ -f "$o" ] && nm -A "$o"; done`, parce que des `.o` périmés de sources supprimées traînent dans `tmp/obj`, legs de D.19c1) :

- **Départ de b1** : les objets qui contiennent un `t pico8_type_tag_of` local (inline émis à `-O0`) sont listés. Attendu : les `.o` des fichiers de `src/` à test d'étiquette, et aucun TU (D.20a).
- **Arrivée de b1 et de b2** : `… | grep -F type_tag` → **vide**.
- **`nm -u`** : `… nm -A -u …| sort` au parent et à l'arrivée → **identiques** (b1 et b2). Aucun symbole externe n'est gagné ni perdu. C'est la règle de lien de `DATA_ENGINE_SRCS` et des autres listes, mesurée. Le lien de chaque binaire par `make test` (66 TU et `tmp/pico8_replay`), puis par `make sanitize-net` et `make class-coverage-net`, la confirme.

### `gcov` e2e et ensemble `L` (b1 et b2)

`make line-coverage EVENTS="$(ls tests/e2e/*.json)"` (pilote instrumenté, arbre `obj-gcov`), au départ puis à l'arrivée de chaque commit.

- **Appariement** : `src/` change, donc on n'apparie pas par numéro seul. Pour chaque fichier, on lit les hunks de `git diff -U0 <parent> -- <f>`. Une ligne hors hunk va de `n` à `n + décalage cumulé`. Une ligne `-` d'un hunk n'a **pas d'image**. Une ligne R1 réécrite a pour image sa ligne `+`, apparentée dans l'ordre. Un fichier supprimé n'a aucune image.
- **`R`** = lignes exécutées au départ **sans image** (retirées). Attendu b1 : ⊆ premières lignes des 16 `assert` retirés (et leurs lignes de message, selon `gcov`). Attendu b2 : ⊆ les 8 affectations `header.tag = …` et les lignes de `pico8_type_tag.h` (aucune n'est plus exécutée depuis b1 : attendu ∅ pour ce fichier).
- **`L`** = lignes exécutées au départ **avec image**, et dont l'image n'est plus exécutée à l'arrivée. Attendu b1 : **⊆ `src/pico8_type_tag.h:*`** (le corps de `pico8_type_tag_of`, qui n'a plus d'appelant ; la suppression vient en b2, H4). Attendu b2 : **∅**.
- Totaux : arrivée = départ − |`R`| − |`L`|, au message. Une ligne de `L` hors de ces ensembles est un comportement perdu : arrêt, rapporté.

### Mutations (règle du parent)

Une à la fois, **dans une copie jetable**, ou dans l'arbre avec le fichier sauvegardé par `cp` et restauré par `cp` simple puis `touch`, jamais `git checkout` ni `stash`. Les binaires sont lancés exactement comme le `Makefile` les lance. « Rougit » : `make test` ou `make e2e` échoue, avec le TU ou l'e2e nommé au message.

**Règle du parent** : une mutation qui ne rougit rien est rejouée **au parent**, sur la même moitié de classe de la ligne d'origine. Si le parent reste vert lui aussi, le contrôle n'était jamais atteint avec un objet non NULL par les tests et les e2e : c'est déclaré, et il n'y a rien à ajouter. Si le parent rougit et que l'enfant reste vert, un chemin est perdu : `FAILED:`.

**D.20b1** (exigées : Mb1, Mb2)

- **Mb1** — chaque contrôle de classe R1, tourné vers une autre classe ou nié, un à la fois (7 mutations) : `economy:63-64` (`&g_hy_struct_class` → `&g_trade_struct_class`), `layout:188` (`&g_lq_struct_class` → `&g_jb_struct_class`), `research:257` (`&g_hp_struct_class` → `&g_trade_struct_class`), `trade:56-57` (→ `&g_hy_struct_class`), `probe:200` (`g__is_target_class(et->cls)` → `!g__is_target_class(et->cls)`), `slipway:282` (`pico8_jc_struct_class()` → `pico8_bg_struct_class()`), `slipway:438` (`jc_is_endpoint_class(…)` → `!jc_is_endpoint_class(…)`). Rouge attendu (l'`assert` saute) ; sinon règle du parent.
- **Mb2** — un contrôle R2 pris pour témoin : `particle.c:48` (`emitter_of != NULL` → `== NULL`) et `probe.c:156` (`probe_of != NULL` → `== NULL`). Rouge attendu. Cela prouve que le contrôle de catégorie sur lequel s'appuie la règle R2 s'exécute encore.

**D.20b2** (exigées : Mc1, Mc2, Mc3)

- **Mc1** — la garde mord : `tests/test_pico8_ptrs.c` est sauvegardé par `cp`, puis on y ajoute en fin de fichier la ligne `/* pico8_type_tag.h */`. `make data-guard` doit échouer, avec `type-tag: tests/test_pico8_ptrs.c:<n>:/* pico8_type_tag.h */` et `résidu gardé (spec,overrides,stash,ob,dispatch,par-nom,liste,ref-audit,valeur,type-tag) : 1 ligne(s)`. Le fichier est restauré par `cp` puis `touch`, et `make data-guard` redevient vert.
- **Mc2** — une vue statique lit sa classe : `probe.c:41` `{&g_bv_et_struct_class}` → `{&g_jc_too_far_struct_class}`. Rouge attendu (l'`assert` de `probe:200` ou le branchement de `pico8_probe_et_hang_fo`). Puis `wormhole.c:26` et `menu.c:52`, chacun vers une autre classe : rouge, ou règle du parent (au parent, la même mutation sur la moitié `&cls`).
- **Mc3** — le compilateur tient les écritures : dans une copie jetable, on remet `self->header.tag = 0;` dans `trade.c` ; `make tmp/obj/src/pico8_trade.o` doit échouer (`no member named 'tag'`, erreur citée au message). On remet ensuite `{PICO8_TYPE_TAG_STRUCT, &g_X_struct_class}` dans `menu.c:52`, avec `#define PICO8_TYPE_TAG_STRUCT 0x9E5C57C7u` local ; la compilation doit échouer (conversion entier → pointeur sous `-Werror`, ou initialiseur en trop).

**Si une mutation exigée ne rougit pas sans que la règle du parent l'explique : `FAILED:`.** Aucune autre mutation : il n'y a aucune logique nouvelle, et §7 demande des mutations sur ce que `gcov` ne prouve pas.

### Filets

- `make test`, `make e2e` (**26 conformes**), à chaque commit. Aucun e2e n'est ajouté.
- **`make sanitize-net`**, à chaque commit : 26 propres. b2 change la disposition de toutes les entités (§7 : « à la sortie de phase » aussi).
- **`make class-coverage-net`** : **exigé à b2** (7 constructeurs et 9 vues statiques changent d'en-tête) et identique à `tools/class_coverage_net_expected.txt` ; rapporté à b1.
- **Balayage ASan+UBSan des 66 TU**, à chaque commit : dans une copie jetable, flags du `Makefile` plus `-fsanitize=address,undefined -fno-sanitize-recover=undefined -fno-omit-frame-pointer`, lancés comme le `Makefile` avec `ASAN_OPTIONS=detect_leaks=0` (TROU-7).
- `make raylib` : seulement si `third_party/raylib/src` est présent, compilation et lien, **jamais d'exécution**. **Jamais l'appli raylib.**
- Aucun TU ne rejoue un fichier d'événements ; `make e2e-gap` n'est pas lancé ; rien de lent (pas d'union `gcov` des TU : aucun TU ne lit plus le module, `nm` le prouve).

### Vérification, budget et tableau « Runs »

**Budget : ≈ 35–40 min (b1) et ≈ 45–50 min (b2) de temps réel jusqu'au commit**, preuves comprises ; **plafond 50 min par sous-étape** (au-delà, repli). La relecture par un sous-agent frais est hors budget.

| Cible | Durée attendue |
|---|---|
| `make test` | de l'ordre de la minute |
| `make e2e` | ~1 s |
| `make sanitize-net`, `make class-coverage-net` | quelques minutes |
| `make line-coverage` (26 e2e), départ + arrivée | ≈ 4 min |
| préprocesseur trié (tous les `.c`) | < 1 min |
| `nm` | secondes |
| balayage ASan+UBSan (66 TU) | 2 à 5 min |
| mutations (b1 : 9 ; b2 : ≈ 5), chacune un `make` ciblé | < 6 min |
| sonde de taille (b2), départ + arrivée | < 2 min |

**L'implémenteur tient un tableau « Runs »** par sous-étape, une ligne par lancement :

| # | Commande exacte | État de l'arbre (HEAD + état) | Code de sortie | Ligne clé | Durée |
|---|---|---|---|---|---|
| 1 | `make -s data-residue` | graduation de D.20a2, avant D.20b1 | 0 | `type-tag 76 25` ; résidu 76 / 25 (H-D20a-1) | … |
| 2 | recette `type-tag` | idem | 0 | `type-tag 76 lignes 25 fichiers 100 occ.` | … |
| 3 | `sh tools/data_residue.sh --detail src \| grep '^type-tag'` | idem | 0 | 23 fichiers, comptes de « État de départ » | … |
| 4 | `nm` (départ) | idem | 0 | objets à `pico8_type_tag_of` local | … |
| … | … | … | … | … | … |

Pour D.20b2, les lignes 1–3 partent de 46 / 18 / 47 (`src/` 41 / 16 / 42). Le vérificateur réutilise le tableau et ne relance que ce qui suit le dernier changement de l'arbre.

### Contrôles du vérificateur

1. **Périmètre** : `git show --stat` de chaque commit ne liste que les chemins de « Modules » de sa sous-étape.
2. **Greps** de la Sortie, exactement, avec leurs exclusions nommées.
3. **Tests d'étiquette** : pour chacun des 23, la règle appliquée est celle de la table ; aucun R1 n'a perdu sa moitié de classe ; aucune ligne jointe ; le seul cast déplacé est celui de `slipway:282`.
4. **Membre** : l'en-tête n'a plus que `cls` ; les 9 + 3 initialiseurs ont la forme `{&cls}` ; aucune affectation `header.tag`.
5. **Preuve** : préprocesseur (conditions i/ii) ; `nm` vide et `nm -u` identique ; `R` et `L` dans les ensembles attendus ; mutations rapportées et restaurées, règle du parent appliquée ; sonde de taille (16 → 8, − 8 partout) ; `sanitize-net`, `class-coverage-net` et balayage propres ; 26 e2e conformes.
6. **Anti-tassement** : comptes avant/après en lignes ET en occurrences, par catégorie et par fichier ; classification juste, avec numéros du parent.
7. **Commentaires** : les quatre sortes de `CLAUDE.md` ; aucun renvoi à D.20, au plan ni à Q13 ; aucun commentaire neuf ne nomme le module.
8. **Docs** : notes « Découvert à D.20b1/b2 » ; « Reste de D » sans puce `type-tag` ; glossaire et TROU-6 réécrits ; `CLAUDE.md` **non modifié**.

### Critères d'échec (traduits en `FAILED:` par le vérificateur de `prd-autopilot`)

**Départ et filet (§7)**

- `FAILED:` si l'arbre de départ de D.20b1 n'est pas la graduation de D.20a2 (ou un descendant qui ne touche que `docs/`), propre ; ou si `type-tag` n'y vaut pas 76 / 25 / 100 occ. et le résidu D 76 / 25, sans explication. De même pour D.20b2 (46 / 18 / 47).
- `FAILED:` si H-D20a-3 tombe (un TU lit encore le marqueur ou le membre).
- `FAILED:` si le filet (`make test`, `make e2e`) reste rouge après **3 tentatives** sur le commit.
- `FAILED:` si un `*.sha256` est modifié ou supprimé, si un fichier de `tests/e2e/` est touché, si un e2e est ajouté ou retiré, ou si un TU rejoue un fichier d'événements.
- `FAILED:` si un test de régression du §6 échoue ou est modifié ; si un site ne tombe sous aucune règle de ce PRD ; si `sanitize-net`, `class-coverage-net` ou le balayage ASan+UBSan n'est pas propre.

**Périmètre et forme**

- `FAILED:` si un fichier hors de « Modules » change ; si une liste de sources du `Makefile` change ; si le `Makefile` change ailleurs que sur `DATA_RESIDUE_GUARD` ; si `CLAUDE.md` change.
- `FAILED:` si une grep de la Sortie ne rend pas ce qui y est écrit, ou si une exclusion y est ajoutée sans être rapportée.
- `FAILED:` si un contrôle de classe qui suivait une étiquette est retiré ou affaibli ; si un accesseur de classe, un alias, un champ ou un mot remplace le marqueur ; si le premier mot d'une entité est relu par un autre chemin ; si un cast nouveau est ajouté ; si des lignes sont jointes.
- `FAILED:` si un include est gagné sans l'erreur du compilateur qui l'exige, ou si un include de `pico8_type_tag.h` part avant ou après le dernier usage de son fichier.

**Gardes, comptes et anti-tassement**

- `FAILED:` si `make data-guard` n'est pas vert, si `DATA_RESIDUE_GUARD` change à b1, ou s'il n'est pas `…,valeur,type-tag` à b2 ; si Mc1 ne mord pas.
- `FAILED:` si `type-tag` ne baisse pas exactement de **30 / 53 occ.** (b1), puis jusqu'à **0** (b2), depuis le départ réel, sans explication chiffrée ligne à ligne ; si `make -s data-residue` ne rend pas, à b2, la seule ligne `résidu D : 0 ligne(s), 0 fichier(s)`.
- `FAILED:` **si un compte baisse par tassement** : lignes jointes, alias, renommage, commentaire compté réécrit hors de son sujet, pont.
- `FAILED:` si le **message du commit** omet l'un de ces éléments : les comptes avant/après par catégorie et par fichier, **en lignes et en occurrences** ; la classification (a1)/(a2)/(d) ligne à ligne, avec numéros du parent et règle R1/R2/R3 ; les lignes qui gagnent une catégorie (« aucune » compris) ; Δlignes/Δocc. ; les includes retirés ou gagnés ; le rapport du préprocesseur ; le rapport `nm` ; `R`, `L` et les totaux `gcov` ; les mutations et la règle du parent ; la sonde de taille (b2) et ce qui en dépend ; la perte de détection par ASan (b2) ; les filets ; le tableau « Runs » (ou son renvoi au journal). Un commit de rattrapage ne compense pas un message incomplet.
- `FAILED:` si `make constants-check`, `float-grep` ou `float-guard` rougit, ou si `tools/float_guard.txt` diffère après `make test` sans explication.

**Preuve, commit et note**

- `FAILED:` si une constante `*_HASH`, `identity[]`, `FROZEN_*`, `VIEW_*`, `TABLE_INDEX_KEY` ou `*_KIND` change.
- `FAILED:` si une mutation exigée manque, ne se restaure pas, ou ne rougit pas sans que la règle du parent l'explique.
- `FAILED:` si `L` sort de `pico8_type_tag.h` (b1) ou n'est pas vide (b2) ; si `R` contient une ligne qui n'est pas une ligne retirée par le diff ; si `nm` montre `pico8_type_tag_of` après b1 ou un `nm -u` différent.
- `FAILED:` si la sonde de taille manque à b2, ou si un écart à « − 8 octets » n'est pas expliqué.
- `FAILED:` si le plafond est dépassé sans note (et sans repli), ou s'il manque un tableau « Runs ».
- `FAILED:` s'il manque la note « Découvert à D.20b1 » ou « Découvert à D.20b2 », si la dernière ne constate pas la Sortie de D, ou si le bloc « Reste de D » garde sa puce `type-tag`.
- `FAILED:` si un commentaire ajouté ou reformulé sort des quatre sortes de `CLAUDE.md`.

## Out of Scope

- Renommer `pico8_struct_header_t` ou `header`, passer aux initialiseurs désignés, créer un registre ou un accesseur de classes générique.
- Ajouter une vérification de catégorie aux 11 sites R2 qui n'en ont pas de générique (question ouverte 2).
- Toucher les `FROZEN_STRUCT_TAG`, `fold_untagged`, `fold_entity` des TU : ce sont des octets d'empreinte gelés par D.20a.
- Recalculer `tools/line_coverage_reference.txt` au-delà des lignes du fichier supprimé.
- Les points du bloc « Reste de D » autres que `type-tag` (`hp_pose`, `pk_is_bool`/`fp_is_bool`, `pj:hb`, reports vers la phase 2) : ils vont à la revue de sortie.
- La revue de sortie de phase elle-même, et la modification de `CLAUDE.md` (voir « Further Notes »).
- Tout e2e ; `make raylib` exécuté, l'appli raylib, l'audio, le rendu ; la PSX.

## Further Notes

- **Décisions de l'agent à la place de l'utilisateur** (consigne `to-prd`), tranchées en `guideline` et marquées (G) :
  - **Découpage** : deux commits, b1 (lecteurs) puis b2 (membre, en-tête, garde) ; repli écrit pour chacun.
  - **Tests** : R1 (la classe reste), R2 (type statique ou écrivain unique), R3 (NULL) ; aucune vérification ajoutée.
  - **Preuve** : préprocesseur, `nm`, `gcov` apparié, sonde de taille, mutations sur les contrôles restants, sur une vue statique et sur la garde.
  - **Docs** : glossaire et TROU-6 réécrits ; `line_coverage_reference.txt` comme à D.19c2.
- **Questions ouvertes (pour l'humain ; les défauts s'appliquent s'il ne répond pas)** :
  1. *Une vérification mémoire devient moins sensible.* Aujourd'hui, chaque objet du jeu commence par un nombre de contrôle, et sa catégorie vient juste après. Si le code prenait par erreur un petit objet (une simple position à l'écran, par exemple) pour un objet du jeu, il irait lire la catégorie au-delà de la fin du petit objet, et l'outil de vérification mémoire l'attraperait tout de suite. Sans le nombre de contrôle, la catégorie passe au tout début de l'objet : la même erreur lirait n'importe quoi **à l'intérieur** du petit objet, et l'outil ne dirait rien. Le programme planterait plus tard, ou se tromperait. Aujourd'hui, aucun endroit du code ne peut faire cette erreur : chacun reçoit un objet dont le type C dit que c'est un objet du jeu, ce qui est vérifié endroit par endroit. **Défaut : on accepte, c'est ce que disait votre choix A** (« un peu moins robuste si la mémoire est abîmée »). L'autre choix serait de garder un mot vide en tête de chaque objet, sans rien y vérifier : cela reviendrait au choix B sous un autre nom.
  2. *Onze endroits ne vérifieront plus rien à l'entrée.* À onze endroits, la vérification retirée ne testait que « c'est bien un objet du jeu », sans regarder sa catégorie. Le type C le garantit déjà, et six d'entre eux vérifient juste après que l'objet a la capacité attendue (des particules, une sonde). **Défaut : on n'ajoute rien.** L'autre choix serait d'ajouter une liste de toutes les catégories connues pour la vérifier à ces endroits, ce qui ajoute du code au jeu pour une vérification que le type C fait déjà.
- **Surprises de mesure, pour `plan-pilot`** :
  1. **Les 23 lecteurs de `src/` sont tous des `assert`** : aucun ne choisit un chemin. Le jeu est identique par construction, et la preuve le constate.
  2. **23 et non 26** : trois lecteurs sont partis depuis l'étude D.14, dont `ui_button.c:20` (D.18b) et `probe.c:210` (B3).
  3. **Les includes de `help`, `planet`, `planet_display`, `planet_family` sont déjà morts** : ils partent en b1.
  4. **`g_cg[i].tag`** (`entity.c:60`, `:86`, `:110`) est le nom d'un sac de `cg`, pas le marqueur : c'est l'exclusion nommée du grep 7, la leçon de D.19c (légende `Makefile:59`).
  5. **La référence de couverture nomme encore `pico8_type_tag.h`** (6 lignes, numéros d'avant D.13) : elles partent en b2, comme les 58 de `pico8_value.c` à D.19c2.
  6. **Retirer le membre rend ASan aveugle à une confusion de représentation** (question ouverte 1).

### Après D.20b : fin de la phase D

D.20b2 est la dernière étape de D. Sa note constate la **Sortie de D** : `pico8_ob.c`, `pico8_value.c`, `pico8_type_tag.h`, `pico8_ref_audit.h`, `pico8_stash.c` absents ; `make -s data-residue` rend la seule ligne vide ; toutes les catégories sont dans `DATA_RESIDUE_GUARD` ; plus aucune allocation de données (les entités gardent la leur, Plan 2 §8). Ensuite, **hors de ce PRD** et hors de ses commits :

1. **Revue de sortie de phase** (`plan-pilot`, « End of phase ») : `warn-review` sur le code touché pendant D, et ses avertissements, bruts, donnés à une consultation `plan-review` ; relecture du bloc « Reste de D et report vers la phase 2 » (chaque puce rattachée à une étape de la phase 2 ou fermée) ; §7 : `make class-coverage-net` identique à `tools/class_coverage_net_expected.txt` et `make sanitize-net` propre sur l'arbre de sortie (déjà constatés à D.20b2 ; la revue les cite). La phase 2 démarre ensuite par **2.0 — mesure seule**.
2. **Mise à jour de `CLAUDE.md` à proposer à l'utilisateur, sans l'appliquer.** La phrase « La couche objet Lua (`pico8_value`/`ob`/`class`/`entity`) est en cours de remplacement par des structs plates — voir `docs/plan-3.md`. » n'est plus exacte : `pico8_value` et `ob` n'existent plus. Proposition de texte : « La couche objet Lua est remplacée par des structs plates (phases 1a, 1b et D de `docs/plan-3.md` faites) ; la phase 2 (composition : noyau, archétypes, composants) est en cours — voir `docs/plan-3.md`. » L'agent qui fait la revue la présente à l'humain, et c'est l'humain qui l'applique ou non.

## Annexe A — textes recopiés (`ca68736`, par extraction de lignes)

### Ligne D.20b (`docs/plan-3.md`, l. 295)

> - D.20b — `type-tag`, `src/` : membre d'étiquette retiré (Q13 = A), `assert` d'étiquette → contrôles de classe, `pico8_type_tag.h` supprimé. Compte : `type-tag` = 0, gardé ; résidu D vide, Sortie de D atteinte. `class-coverage-net` : oui.

### Bloc « Reste de D », première puce (l. 299)

> - `type-tag` (239) : D.20a, D.20b.

### `src/pico8_struct.h:42-47`

```c
/* First member of every entity struct, and of the parent struct a child
 * embeds first: the tag stays at offset 0 for pico8_type_tag_of(). `tag` is
 * always PICO8_TYPE_TAG_STRUCT. */
typedef struct {
    pico8_type_tag_t tag;
    const pico8_struct_class_t *cls;
} pico8_struct_header_t;
```

### Sortie de la phase D (`docs/plan-3.md`, §5, Phase D)

> - **Sortie** : `pico8_ob.c`, `pico8_value.c`, `pico8_type_tag.h`, `pico8_ref_audit.h`, `pico8_stash.c` supprimés ; plus aucun `malloc` pour des données. Les entités gardent le leur : c'est Plan 2 §8.

## Annexe B — reproduire (lecture seule, sans build)

```sh
T='\bpico8_type_tag(_of|_t)?\b|\bPICO8_TYPE_TAG_[A-Z0-9_]+|pico8_type_tag\.h'
grep -nE "$T" src/*.c src/*.h                                          # les 71 lignes
grep -nE -B2 'pico8_type_tag_of' src/*.c                               # chaque appel est dans un assert ouvert sur sa ligne ou juste avant
grep -n 'TYPE_TAG_UNKNOWN\|!= PICO8_TYPE_TAG_STRUCT' src/*.c           # vide : aucune discrimination
grep -nE '(->|\.)tag\b' src/*.c src/*.h tests/*.c                       # header.tag ×8, g_cg[i].tag ×3 (et js:1610 avant D.20a2)
grep -nE '^\s*pico8_struct_header_t header;' src/*.h src/*.c tests/*.c  # 17 + 1
grep -nE 'pico8_ptrs_add\(&g_hv|g_cv\[i\]\.list, e|bucket->list, e|pico8_ptrs_add\(oe' src/*.c   # écrivains uniques (H1)
grep -nE 'pico8_ij\(|pico8_gk\(|->mm\(' src/*.c                         # appelants des sites R2/R3
grep -n 'type_tag' tools/line_coverage_reference.txt                    # 6 lignes
for f in bootstrap economy entity_geom help ke particle planet planet_display planet_family planet_family_trade research score_panel trade ui; do
  printf '%s: ' $f; grep -cE '^#include <string\.h>' src/pico8_$f.c; done   # H2
```
