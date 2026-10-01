---
title: "Plan 3, étape D.16b : genres mêlés B3–B7, en deux sous-étapes — b1 : `et` en pointeur (NULL pour nil et `false`, `has_et` retiré, `pico8_mz_et` retiré), `pico8_rn` prend un nombre ; b2 : `pico8_oj(const char *)`, `pico8_hp_il` en `const char *`, `hp_pose` par position, `pico8_lq_bg` et `pico8_pj_ed` retirés ; quatre copies `truthy` parties ; aucune constante ne change"
description: "Cinq petites valeurs à genres mêlés (la cible `et` d'une sonde, l'argument de `oj`, le `il` d'un projet, l'argument de `rn`, le `bg` d'une rangée et le `ed` du panneau) sont encore des valeurs Lua alors qu'aucun lecteur ne sépare nil de `false` ; D.16b les type en deux commits (b1 : B3 + B5, b2 : B4 + B6 + B7 + `hp_pose`), absorbe les deux ponts laissés par D.15a et D.15b, retire quatre copies `truthy` et fait suivre 14 TU sans changer une constante : `valeur` baisse de 80 lignes (85 occ.), ≈ 825 → ≈ 745."
status: needs-triage
---

## Problem Statement

**Écrit pendant que D.16a était implémentée** (`plan-pilot`, étape 2b) sur l'arbre commis `c77124e`, puis **revérifié après la graduation de D.16a** (`6fcb370` `refactor(D.16a)`, gradé par `ab8839d`) sur **`0b8ea0d`** (HEAD de `claude/plan-pilot-3-7zdft0`, qui ne fait qu'ajouter ce PRD), extrait par `git archive HEAD` dans le scratchpad. Seuls `grep`, `sed`, `awk` et `sh tools/data_residue.sh` ont tourné ; **rien n'a été compilé ni lancé**. Les hypothèses `H-D16a-n` ont reçu leur verdict (plus bas). **Les numéros de ligne de ce PRD sont ceux de `0b8ea0d`** : identiques à ceux de `c77124e` dans les fichiers que D.16a n'a pas touchés, remesurés par contenu dans ceux qu'elle a touchés (`planet.c`, `planet_family.c`, `score_panel.c` et les TU `fr`, `mx`, `os`, `pm`, `nr`, `hj`, `nl`, `qh`, `ga`, `lq`). L'implémenteur remesure encore par contenu au parent de chaque commit et explique tout écart.

Mesuré à `c77124e` : `valeur` 879 / 99 ; 971 occ., `type-tag` 243 / 58, résidu D 1120 / 108. **Mesuré à `0b8ea0d` (départ de b1) : `valeur` 825 / 97 ; 916 occ. (`src/` 387 / 59 / 438, `tests/` 438 / 38 / 478), `type-tag` 242 / 58, résidu D 1065 / 107.**

Le problème, vu du mainteneur du port : la Sortie de la phase D exige que `pico8_value.c/.h` disparaissent (D.19c). L'étude D.14 (`docs/prd/done/plan3-phaseD-etape14-etude-genres-meles.md`) range en B3–B7 cinq valeurs que le port tient encore comme des valeurs Lua, alors que **aucun lecteur, ni du cart ni du port, ne sépare nil de `false`** :

- `et` (cible d'une sonde) : le membre est déjà un pointeur, mais `bv` le calcule dans une `pico8_value_t` locale et le port garde un drapeau `has_et` que personne ne lit sans `et != NULL` ; `pico8_mz_et` le rend en valeur pour une copie `truthy` ;
- l'argument de `pico8_oj` (`pico8_value_t px`) : ses appelants lui passent des chaînes enveloppées (`pico8_value_string(...)`), y compris `pico8_ov`, qui depuis D.15a prend un `const char *` et le réenveloppe (pont déclaré de D.15a, `tech_gate.c:45`) ;
- `pico8_hp_il` rend `REF NULL`, `BOOL 0` ou une `STRING` que deux lecteurs redéballent par une copie `truthy` ;
- `pico8_rn(pico8_value_t)` : onze appels enveloppent un nombre, un seul passe `false` ;
- `pico8_lq_bg` et `pico8_pj_ed` : deux accesseurs à valeur que seuls des TU appellent.

La ligne `bv` garde en outre le pont déclaré de D.15b : `probe.c:182` `et = pico8_value_ref(c);` (`c` est déjà un pointeur).

Ligne de l'étape, recopiée de `docs/plan-3.md` à `c77124e` (§5, l. 287) :

> - D.16b — genres mêlés B3–B7 (pointeur NULL pour nil/faux, chaînes, `hp_pose` en affectation par position, `rn`, `bg`, `pj_ed`), en deux sous-étapes b1/b2. `class-coverage-net` : oui. Compte : ≈ 765.

Ligne de l'étude D.14 (§3), recopiée :

> | **D.16b** | B3 (`et`, `has_et` retiré), B4 (`oj`, `il`) et `hp_pose` par position, B5 (`rn`), B6 (`bg`, `bg_is_bool` retiré), B7 (`pico8_pj_ed` retiré) ; copies `truthy` restantes de ces modules ; TU `hp`, `mz`, `g_`, `ga`, `spec_defaults`, `lq` (bg) | ≈ 25 fichiers : **découper** b1 (B3, B5) / b2 (B4, B6, B7, `hp_pose`) | ≈ 765 |

Points B3–B7 et G7 de l'étude (§1.3, §4), recopiés :

> **B3. `et` de la sonde — 16 lignes.** […] Règle (G) : **pointeur, NULL pour nil et `false`** ; `has_et` supprimé ; `pico8_mz_et` rend `const void *` (ou disparaît au profit de `self->probe->et`). Raison : aucun lecteur ne sépare les deux absences.
> **B4. `il` d'un projet et l'argument de `oj` — 14 lignes.** […] Règle (G) : **pointeur de chaîne, NULL pour nil et `false`** : `int pico8_oj(const char *px)`, `const char *pico8_hp_il(…)`. Le cas « nombre » de `test_pico8_hp` (`pico8_oj(fixed 1)`) est un genre que `src/` ne produit pas (G4). `hp_pose` […] suit dans la même étape, sans compte : `mr` est la constante de classe `hp.mr` […] ; l'aiguillage par `strcmp` devient une affectation par position (`switch (p)`), les `assert` de genre restent.
> **B5. `rn(no)` — 16 lignes.** […] Règle (G) : **`void pico8_rn(pico8_fixed_t n)`**, et l'unique appelant à `false` appelle sous condition (`if (has_my) pico8_rn(60)`). Raison : `rn(false)` ne fait rien ; l'audio est hors périmètre (TROU-10), `pico8_sfx` est vide.
> **B6. `bg` d'une rangée — 4 lignes (+ 3 de `tests/`).** […] Règle (G) : **drapeau + nombre**, `false` rangé en absent (`has_bg = 0`), `bg_is_bool` supprimé ; `pico8_lq_bg` supprimé (appelants : deux TU seulement, `ga`, `hp`).
> **B7. `ed` du panneau de score — 5 lignes.** […] Règle (G) : **pointeur, NULL = défaut de classe** (déjà en place) ; `pico8_pj_ed` supprimé (un seul appelant, `test_pico8_spec_defaults`).
> **G1.** Aucune constante figée ne change dans D.15–D.20. […] Une empreinte qui bouge est une régression, pas une mise à jour.
> **G2.** nil et `false` confondus seulement là où aucun lecteur du cart ne les sépare (preuve par grep des lecteurs dans le PRD) : B1, B3, B4, B6.
> **G4.** Un cas de TU qui passe un genre qu'aucun appelant de `src/` n'écrit […] est supprimé avec le type qui le permettait ; le message de commit le nomme.
> **G7. `hp_pose`.** Positionnel en D.16b, sans compte : `mr` est une constante de classe.

Règles communes de l'étude (§3) : un TU qui nomme un symbole changé le suit dans le même commit ; aucune empreinte figée ne change (G1 : un pli typé reproduit l'octet de genre et le contenu de l'ancien pli) ; aucun `*.sha256` ni e2e ne change ; `class-coverage-net` si l'étape touche un constructeur.

### Hypothèses `H-D16a-n` — verdicts (D.16a gradée, mesurés à `0b8ea0d`)

Source des hypothèses : le PRD de D.16a à `c77124e`. Verdicts : `git archive HEAD` (`0b8ea0d`), `git diff --stat c77124e HEAD -- src tests Makefile tools`, la note « Découvert à D.16a » (`docs/plan-3.md:286`).

| Hypothèse | Énoncé (résumé) | Verdict | Mesure à `0b8ea0d` ; effet sur ce PRD |
|---|---|---|---|
| H-D16a-1 | `valeur` 825 / 97 ; 916 occ. (`src/` 387 / 59 / 438, `tests/` 438 / 38 / 478) ; `type-tag` 243 / 58 ; résidu D 1066 / 107 ; garde inchangée | **fausse en partie** | `valeur` exact ; **`type-tag` 242 / 58, résidu D 1065 / 107** : `fold_ref` de `test_pico8_pm` n'avait que `fold_value` pour appelant et est partie avec elle (une ligne `type-tag`). Départs, arrivées et critères `type-tag`/résidu D corrigés (− 1 ligne) ; les deltas de D.16b ne changent pas |
| H-D16a-2 | `planet.c` 2 / 2, ses deux lignes `rn` intactes, ≈ `:422`, `:424` | **tient** | `:421`, `:423` (décalage − 24) |
| H-D16a-3 | `score_panel.c` 19 / 23, `pico8_pj_ed` intacte, ≈ − 7 | **tient** | 19 / 23 ; `pico8_pj_ed` en `:291-299` (décalage − 8) |
| H-D16a-4 | constructeur `qh` garde `et = NULL` / `has_et = 0`, décalés | **tient** | `planet_family.c:79-80` |
| H-D16a-5 | aucune ligne `has_et`/`et_state`/`fold_et`/`et_kind`/`mz_et`/`lq_bg`/`pj_ed`/`oj` touchée dans les TU ; `ga` 30 / 30 ; `mx`, `os`, `pm`, `hj`, `nl` remontent d'≈ 15 | **tient, décalages différents** | contenu intact, `ga` 30 / 30. Décalages : `mx`, `os`, `hj`, `nl` − 15 ; **`pm` − 25** (`fold_value` et `fold_ref`) ; **`qh` + 1** (include de `economy_score.h` ajouté par D.16a, hors de son empreinte) ; `lq` + 1 à partir de `:239` (test et include ajoutés par D.16a ; `lq` n'est pas touché ici) ; `fr`, `nr`, `ga` inchangés aux sites de ce PRD. Tous les numéros sont remesurés ci-dessous |
| H-D16a-6 | 7 copies `truthy` | **tient** | `layout`, `probe`, `research`, `research_display`, `tech_gate`, `ui_button`, `wormhole_display` |
| H-D16a-7 | `fold_absent`, `fold_bool_member`, `fold_ref` restent utilisées dans `os`, `hj`, `nl`, `qh` | **tient** | `fold_absent` 6 / 6 / 6 / 7 lignes, `fold_ref` 3 / 3 / 3 / 4, `fold_bool_member` 4 / 5 / 4 / 4. Le retrait de `fold_ref` de `pm` ne touche pas b1 : le seul site `has_et` de `pm` (pli « built », `:358`) plie `fnv(h, …)` sans `fold_ref`, et `pm` n'a ni `et_state` ni `fold_et` |
| H-D16a-8 | ni `Makefile`, ni `tools/`, ni aucun fichier B3–B7 hors ceux de H-2 à H-5 | **tient** | `git diff --stat c77124e HEAD -- Makefile tools` vide ; les 23 chemins de D.16a (dont `test_pico8_pj`, `test_pico8_lq`, hors de son empreinte) n'incluent aucun fichier de `src/` de D.16b hors `planet.c`, `planet_family.c`, `score_panel.c` |
| H-D16a-9 | ligne D.16b inchangée ; D.16a entière | **tient** | un commit ; ligne D.16b identique (Annexe A) ; aucun site B3–B7 déplacé |

**Erreur du brouillon, corrigée (sans lien avec D.16a)** : le TU de B7 est `tests/test_pico8_spec_defaults.c` (qui appelle `pico8_pj_ed` à `:233`), pas `test_pico8_spec_defaults.c` (TU de D.16a, qui ne nomme pas `ed`). Le brouillon écrivait `spec_defaults` : corrigé partout en `spec_defaults`. `test_pico8_spec_defaults.c` est inchangé depuis `c77124e`.

### Revérification faite à la graduation de D.16a

- « État de départ », critères `FAILED:` de départ, « Effet sur `make data-residue` », Runs 1-2 : remis sur `0b8ea0d` (`type-tag` 242, résidu D 1065, puis 1025 et 985).
- Numéros de `planet.c`, `planet_family.c`, `score_panel.c`, des TU `mx`, `os`, `pm`, `hj`, `nl`, `qh` et de `lq` (preuve B6) : remesurés par contenu. `fr:293`, `nr:308`, `ga:497/504/514`, `hp`, `mz`, `g_`, `fv`, `spec_defaults:233` : inchangés.
- Découpage b1/b2 : **gardé** (aucune mesure ne le change ; D.16a n'a touché aucun fichier de b2 hors `score_panel.c`, en place).
- Annexe A : lignes D.16a–D.17 réextraites de `0b8ea0d`.

### Preuve B3 (`et`), par le cart et par le port

**Écritures du cart** (`grep -nE '\.et\s*=|self\.et=' pico8/lua.lua`, trois sites) :

| Ligne | Site | Valeur écrite |
|---|---|---|
| 552-557 | `bv` | `nq<self.dw and c` (une planète) `or jp(cg.jc,…)[1]` (un `jc`) `or not ui.hw(rp) and {}` (la jetable) — sinon `false` (`not ui.hw(rp)` faux) |
| 626 | `jc:hq` | `self:ce()` : nil, la vue « -too far- » ou une entité |
| 769 | `mz:hq` | `mz.jj` |

Avant tout `bv`, `et` est nil (champ absent).

**Lecteurs du cart** : `if (self.et)self.et.fo=self` (558), `if self.et then` (`dl`, 562), `if not self.et` (`d_`, 569), `(self.et or self.fo)and 2` (`jc:kj`, 672), `if not self.et then` (`mz:kj`, 784). **Tous en vérité seule** : aucun ne compare `et` par `==`/`~=`, ne le passe à `tostr` ni ne le range. nil et `false` ne sont distingués nulle part (G2 : B3 confirmé).

**Écritures du port** (`grep -rn 'has_et\|->et = ' src`) : `bv` (`probe.c:215-216`, `has_et = 1` toujours), `jc:hq` (`slipway.c:149-150`, `has_et = et != NULL`), `mz:hq` (`wormhole.c:73-74`), constructeurs `g_` (`probe.c:475-476`), `qh` (`planet_family.c:90-91`), `mz` (`wormhole.c:281-282`), `jc` (`slipway.c:435-436`), tous `et = NULL; has_et = 0;`. **Invariant** : `has_et == 0` implique `et == NULL` à chaque écriture.

**Lecteurs du port** — tous en `has_et && et != NULL` ou en `et != NULL` seul :

| Site (à `c77124e`) | Lecture | Après b1 |
|---|---|---|
| `probe.c:218` (`bv`) | `probe->et != NULL` | inchangé |
| `probe.c:263` (`dl`) | `pb->has_et && pb->et != NULL` | `pb->et != NULL` |
| `probe.c:343` (`d_`) | `has_et = probe->has_et && probe->et != NULL;` | `has_et = probe->et != NULL;` (le local garde son nom) |
| `slipway.c:151`, `slipway_display.c:120` | `et != NULL` | inchangés |
| `wormhole_display.c:45` (`mz:kj`) | `!pico8_mzd_truthy(pico8_mz_et(self))` : `REF NULL` et `BOOL 0` faux | `if (self->probe->et == NULL) {` |

Par l'invariant, `has_et && et != NULL` vaut `et != NULL` partout : **retirer `has_et` ne change aucune lecture de `src/`.**

**Ce que les TU plient** (surprise, déclarée) : `has_et` est plié ou lu dans **11 TU** (`fr`, `fv`, `g_`, `hj`, `mx`, `mz`, `nl`, `nr`, `os`, `pm`, `qh`), hors motif `valeur`. Deux contextes seulement, prouvés par lecture :

1. **Empreintes « built »** (`fnv(h, (uint32_t)qh->probe->has_et)`, `fr:293`, `mx:354`, `os:325`, `pm:358`, `nr:308`, `hj:361`, `nl:298`, `qh:298`) : les fonctions `fold_gate`/`fold_synthesizer`/`fold_processor`/`fold_protostar`/`fold_slipgate`/`fold_ascension`/`fold_plate`/`fold_lab` ne sont appelées que par `test_built` sur des planètes faites par `ie:ldown` (`build()` : `cls_of(icon)->ldown(icon, NULL, NULL)`), jamais passées par `hq` : `has_et == 0` et `et == NULL`. Le pli typé `qh->probe->et != NULL ? 1u : 0u` écrit le même octet (0).
2. **Après un `hq`** (`et_state`/`fold_et` de `os`, `hj`, `nl`, `qh` ; `et_kind`/`fold_et` de `g_` ; `et_kind` de `mz`) : chaque lecture suit un `hq` qui est `bv` (`qh.hq=bv`, `g_.hq=bv`, lua.lua:915, 722 ; `pico8_bv_dispatch` dans `planet_family.c:52,146,207,284,391`, `probe.c:455`) ou `mz:hq` (qui appelle `bv`), et `bv` pose `has_et = 1` sans condition. La branche « absente » (`!has_et`) n'y est donc jamais prise, et les `assert` des TU le confirment (aucun n'attend `ET_ABSENT`/`0` ; `qh:434` attend l'état 1, `false`). Retirer cette branche laisse les mêmes octets.

Aucune empreinte ne plie donc un état « nil après construction » **et** un état « `false` après `bv` » par le seul membre : G1 tient sans drapeau. **Hypothèse vérifiée par l'exécution** : si une constante rougit, c'est qu'un pli lit un état que cette analyse manque — arrêt, rapporté (pas de mise à jour de constante).

### Preuve B4 (`oj`, `il`)

**Cart** : `function oj(px) return not px or gu(fm.invented,px) end` (1464-1466) ; appelants `ov(px,…)` (1468, `px` littéral), `cb` (`not oj(t.px)`, 1459), `ie:hb` (`oj(self.ke.rr)`, 926), `bj` (`oj(b.rr)`, 1392), `hp:po`/`hp:od` (`self.il and oj(self.il)`, 1423, 1427), `oj("nanomaterials")` (1430). `il` : position 7 de `mr` (`ho`, l. 17-22 : `ri[p]=gu(gp,p) and r[ri[i]] or ri[i]`), chaîne, `f` (false) ou absente. `rr` : chaîne ou nil. **Lecteurs de `px` et de `il` : vérité seule** (`not px`, `self.il and …`).

**Port** : `pico8_oj(pico8_value_t)` rend 1 sur `REF NULL`/`BOOL 0`, 0 sur un non-`STRING` (le cas « nombre », G4), sinon `gu`. Appelants de `src/` : `tech_gate.c:45` (`ov`, `px` littéral), `:98` (`cb`, `t->px`), `research.c:357`, `:377` (`il`), `:381` (littéral), `research_icon.c:93`, `research_display.c:325` (`rr`, déjà traduit `NULL` → `REF NULL`). **Les 19 entrées de `pico8_nb_table` (`map_tables.c:254-273`) ont toutes un `px` chaîne** : `pico8_oj(t->px)` ne reçoit jamais `NULL`. (Aujourd'hui, `pico8_value_string(NULL)` serait une `STRING` à pointeur nul, lue vraie, et `gu` ferait `strcmp(e, NULL)` : chemin latent qui disparaît avec le type.) `hp.il` : `has_il` + `il` (`research.h:46-47`), trois états ; `pico8_hp_il` n'a que deux lecteurs (`hp_po`, `hp_od`), en vérité seule.

### Preuve B5 (`rn`)

`function rn(no) if (no and not ih)sfx(no,3) end` (330-332). Appelants : `rn(self.my and 60)` (277, le seul qui peut passer `false`), `rn(p.eo or 62)` (282), `rn(hp.rn or 55)` (460), `rn(self.ik or 54)` (571), `rn(57)` (574), `rn(50)` (620), `rn(-1)` (635), `rn(53)` (764) : **un nombre partout, sauf `false` au seul `js:ldown`**. Port : onze lignes `pico8_rn(pico8_value_fixed(…))` et `ui_button.c:342`. `pico8_sfx` est vide (TROU-10) : **aucun TU ni e2e ne peut observer `rn`** (voir « Preuve des tests »).

### Preuve B6 (`bg`) et écart à la règle de l'étude

**Cart** : `bg=` n'est écrit qu'à la ligne 429 (`{ne(self.kr).."$",bg=self.kr>=0 and 1}`, rangée `h_` de `jo:gh`) ; lecteur unique `c.bg or 2` (275). **Port** : écriture unique `planet_display.c:198-200` (`has_bg = 1`, `bg_is_bool = kr < 0`, `bg = kr >= 0 ? 1 : 0`) et copie `layout.c:401-403` ; lecteur unique `ui_button.c:325` (`has_bg && !bg_is_bool` → nombre, sinon 2). `pico8_lq_bg` n'a que deux appelants, les TU `ga:504` et `hp:277`.

**Conflit avec G1, tranché (G)** : les empreintes figées de `test_pico8_lq` **séparent** `false` de nil : `fold_bg` (`lq:239-244`) plie un `BOOL 0` quand `has_bg && bg_is_bool` et un absent (`0xFFFFFFFF`) sinon ; `CONSTRUCTIONS_HASH` plie les constructions « hole »/« disorder » (sans `bg`) **et** « bg false » (`with_bg(…, -1)`, `lq:457`) ; `HP_LQ_HASH` plie la rangée `h_` à `bg` faux (`lq:541`). Ranger `false` en absent (`has_bg = 0`) rendrait ces états identiques : deux constantes bougeraient. G1 l'emporte : **`bg_is_bool` est gardé** (membre déjà typé, hors motif `valeur`), la représentation à trois états reste, et seul `pico8_lq_bg` part (4 lignes de `src/`). `test_pico8_lq`, `planet_display.c` et `ui_button.c:325` ne changent pas. `bg_is_bool` rejoint `pk_is_bool` et `fp_is_bool` dans la note du plan (forme « 1 ou faux » sans étape nommée).

**`ga` et `hp`** : leurs plis de rangée (`ga:fold_lq`, `hp:fold_lq_instance`) portent sur `jo.lq` (fait par `jo.ex`, lua.lua:539) et `hp:lq` ; aucune n'a de `bg` (cart : `bg=` seulement en 429 ; port : seules `h_keys` de `planet_display.c` et les `with_bg` du TU `lq` posent `has_bg`). La branche `bg` de ces plis n'est jamais prise (`pico8_lq_bg` rend `REF NULL`, le terme `!(bg…)` de `hp:280` vaut 0). Elle devient `assert(!lq->has_bg);` : mêmes octets, et l'invariant est vérifié à l'exécution au lieu d'être plié mort.

### Preuve B7 (`ed`)

`ed=1` (défaut de classe, 846), `self.ed=mh(iw)` (899) ; lecteur unique `kc~=self.ed` (882), identités. Port : membre pointeur, NULL = défaut (`score_panel.c:331`), lecteur `score_panel_display.c:318` (`self->ed != NULL && pico8_kc() == self->ed`). `pico8_pj_ed` : un seul appelant, `test_pico8_spec_defaults:233` (ligne hors motif : `pico8_pj_ed` ne correspond pas à `\bpico8_value_`).

### Lecture de la ligne pour D.16b

- **« en deux sous-étapes b1/b2 »** : **Tranché (G)** : la coupe de l'étude, **b1 = B3 + B5**, **b2 = B4 + B6 + B7 + `hp_pose`**. Raison mesurée : B3 et B5 partagent `probe.c`, `wormhole.c`, `slipway.c` (les séparer éditerait trois fichiers deux fois) ; b2 ne partage **aucun** chemin avec b1. b1 : 12 fichiers de `src/`, 11 TU ; b2 : 10 fichiers de `src/`, 3 TU. Chacune est un commit, vert seul, vérifié et poussé, comptée à part ; b1 d'abord (ordre de l'étude ; les deux sont indépendantes, l'ordre n'est pas porteur). Rejeté : un commit unique (≈ 36 chemins, deux familles sans lien, repli plus coûteux) ; b1 = B3 seul et B5 avec b2 (trois fichiers édités deux fois).
- **« pointeur NULL pour nil/faux »** (B3) : `pico8_struct_header_t *et` local dans `bv` ; `has_et` **retiré** de `pico8_probe_t` et de ses sept écritures ; **`pico8_mz_et` retirée** (option « disparaît » de l'étude : son seul lecteur de `src/` lit le membre ; précédent `pico8_jo_hf` de D.16a) ; `pico8_probe_truthy` et `pico8_mzd_truthy` partent (sans appel, fatales sous `-Werror`).
- **« chaînes »** (B4) : `int pico8_oj(const char *px)` (NULL : vrai) ; `const char *pico8_hp_il(const pico8_hp_t *)` (NULL pour nil, `false` ou `self` NULL) ; **`has_il` gardé** (déjà typé, hors motif ; `test_pico8_hp:189-194` plie nil ≠ `false` dans une empreinte figée, et l'étude ne le retire pas) ; `pico8_research_truthy`, `pico8_tech_gate_truthy` partent.
- **« `hp_pose` en affectation par position »** : `hp_pose(self, p, s, is_fixed, fixed, i_src)`, trois `assert` de genre gardés mot pour mot (message du troisième réécrit : le cas « `mr` nomme un membre inconnu » n'existe plus), puis `switch (p)` ; `hp_ho` garde `name` pour `pico8_ho_gp_has`. Sans compte (G7).
- **« `rn` »** (B5) : `void pico8_rn(pico8_fixed_t no)` ; `js:ldown` appelle `pico8_rn(pico8_fixed_from_int(60))` **dans le bloc `if (has_my)` existant**, avant `self->my(self)` (l'ordre du cart `rn(…)mv(self,"my")` est gardé).
- **« `bg` »** (B6) : `pico8_lq_bg` retiré ; `bg_is_bool` **gardé** (écart à l'étude, G1, ci-dessus).
- **« `pj_ed` »** (B7) : `pico8_pj_ed` retiré ; `spec_defaults:233` devient `assert(pj->ed == NULL);`.
- **Ponts absorbés** : `tech_gate.c:45` (D.15a) → `if (pico8_oj(px)) {` ; `probe.c:182` (D.15b) → `et = c;`.
- **« Compte : ≈ 765 »** : estimation sur une base de 845 (− 80). Mesuré à `c77124e`, remesuré identique à `0b8ea0d` : **b1 − 40 lignes / − 42 occ., b2 − 40 / − 43, total − 80 / − 85** — le delta de l'étude exactement ; l'arrivée est **745** parce que D.16a a fini à 825 et non à ≈ 845. « ≈ » n'est pas un critère ; les deltas exacts le sont.
- **« `class-coverage-net` : oui »** : b1 touche les constructeurs `g_`, `qh`, `mz`, `jc` (ligne `has_et`) ; b2 le constructeur `hp` (`hp_pose` via `hp_ho`). Exigé aux deux commits, **identique** à `tools/class_coverage_net_expected.txt`. `sanitize-net` : **non** (aucune durée de vie ni propriétaire ne change : `et` reste emprunté, aucune allocation touchée). Balayage ASan+UBSan des TU exigé aux deux commits (consigne).
- **§6** : aucun des trois tests de régression (`layout_nil_item_draws_nothing`, `planet_gh_row_lq`, `ga_export_panel_by`) ne nomme un symbole de l'étape ; recompilés, relancés.
- **Consignes de l'utilisateur, liantes** : un TU ne rejoue jamais un `events.json` ; rien de lent ; aucun `*.sha256` ni fichier de `tests/e2e/` touché ; interdits : jonction de lignes comptées, alias, renommage pour échapper aux greps, cast nouveau, cale valeur ↔ typé ; mutations restaurées par `cp` simple puis `touch` ; binaires lancés exactement comme le `Makefile` ; **jamais l'appli raylib**.

### État de départ

| Mesure | À `c77124e` (D.16a non commencée) | Départ de b1, mesuré à `0b8ea0d` (D.16a gradée) |
|---|---|---|
| `valeur` (lignes / fichiers ; occ.) | 879 / 99 ; 971 occ. | **825 / 97 ; 916 occ.** (`src/` 387 / 59 / 438, `tests/` 438 / 38 / 478) |
| `type-tag` | 243 / 58 | **242 / 58** |
| résidu D | 1120 / 108 | **1065 / 107** |
| fonctions publiques qui nomment `pico8_value_t` (recette de l'Annexe B) | 16 | 13 |
| copies `*_truthy(pico8_value_t)` | 11 | 7 |
| `DATA_RESIDUE_GUARD` | `spec,overrides,stash,ob,dispatch,par-nom,liste,ref-audit` | inchangé |

Recette des occurrences (celle de D.12–D.16a, motif `valeur` de `tools/data_residue.sh:29`) :

```sh
P='\bpico8_value_[A-Za-z0-9_]+|\bPICO8_VALUE_[A-Z0-9_]+|pico8_value\.h'
F=$(find src tests \( -name '*.c' -o -name '*.h' \) -type f | sort)
printf 'valeur\t%s lignes\t%s fichiers\t%s occ.\n' "$(cat $F | grep -cE "$P")" \
  "$(grep -lE "$P" $F | wc -l)" "$(cat $F | grep -oE "$P" | wc -l)"
for f in $F; do n=$(grep -cE "$P" $f); [ $n -gt 0 ] && \
  printf '%s\t%s\t%s\n' $f $n "$(grep -oE "$P" $f | wc -l)"; done
```

### Inventaire (numéros à `0b8ea0d`, remesurés par contenu après D.16a)

Classes du message de commit : **(a1)** même site, type C à la place de la valeur ; **(a2)** retirée sans remplaçant (enveloppe, copie `truthy`, local, include d'un `.c` vidé, cas G4, terme de pli mort) ; **(b)** signature publique dont le type change ; **(c)** même donnée, autre nom : **attendu 0**. `occ.` : occurrences du motif sur la ligne.

#### b1 — API

| Symbole | Aujourd'hui | Après b1 | Appelants | TU |
|---|---|---|---|---|
| `pico8_rn` (`audio.c:24`, `audio.h:19`) | `void (pico8_value_t no)` | **`void pico8_rn(pico8_fixed_t no)`** — (b) | 11 lignes (table ci-dessous) | aucun |
| `pico8_mz_et` (`wormhole.c:199-207`, `wormhole.h:46-49`) | rend `pico8_value_t` | **retirée** — (a2) | `wormhole_display.c:45` → `self->probe->et == NULL` | `mz:337` (retirée) |
| membre `pico8_probe_t.has_et` (`probe.h:27`) | `int` | **retiré** (hors motif) | sept écritures, deux lectures | 11 TU (table « Les TU suivent ») |

#### b1 — sites de `src/` (39 lignes, 41 occ. retirées ; aucune gagnée)

| Fichier | Ligne(s) `valeur` (occ.) | Remplacement | Classe |
|---|---|---|---|
| `probe.c` | `:47`, `:48`, `:51` (1 chacune) | `pico8_probe_truthy` retirée entière (`:47-55` et la blanche) : son seul appelant `:197` devient `first != NULL` | (a2) |
| | `:159` (1) | `pico8_struct_header_t *et;` | (a1) |
| | `:179` (1) | `et = NULL;` | (a1) |
| | `:182` (1) | `et = c;` (pont de D.15b) | (a1) |
| | `:187` (2) | `pico8_struct_header_t *first = NULL;` | (a1) |
| | `:192` (1) | `first = filtered.items[0];` (`void *` → pointeur d'objet, conversion implicite de C, pas de cast) | (a1) |
| | `:197` (0) | `if (have_first && first != NULL) {` | — |
| | `:202` (1) | `et = &g_bv_et.header;` | (a1) |
| | `:204` (1) | `et = NULL;` | (a1) |
| | `:209` (1) | `assert((et == NULL \|\|` | (a1) |
| | `:210` (1) | retirée : `et.as.ref != NULL` est l'autre moitié du `\|\|` | (a2) |
| | `:211` (0 ; **`type-tag` 1**) | `(pico8_type_tag_of(et) == PICO8_TYPE_TAG_STRUCT &&` — ligne `type-tag` **modifiée, non retirée** (argument seul) | — |
| | `:212` (0) | `g__is_target_class(et->cls))) &&` (un cast **retiré**) ; message `:213-214` inchangé (« false » reste vrai : NULL) | — |
| | `:215` (1) | `probe->et = et;` | (a1) |
| | `:216`, `:263`, `:343`, `:476` (0) | `has_et = 1` retirée ; `if (pb->et != NULL) {` ; `has_et = probe->et != NULL;` ; `has_et = 0` retirée (constructeur `g_`) | — |
| | `:358` (1) | `pico8_rn(probe->has_ik ? probe->ik : pico8_fixed_from_int(54));` | (a1) |
| | `:363` (1) | `pico8_rn(pico8_fixed_from_int(57));` | (a1) |
| `probe.h` | `:22-23`, `:27` (0) | commentaire réécrit (« `et` is borrowed; NULL is nil or false, which no reader tells apart. ») ; `int has_et;` retiré | — |
| `wormhole.c` | `:47` (1) | `pico8_rn(pico8_fixed_from_int(53));` | (a1) |
| | `:199`, `:201`, `:204`, `:206` (1 chacune) | `pico8_mz_et` retirée entière (`:199-207` et la blanche) | (a2) |
| | `:74`, `:282` (0) | `has_et = 1` (`mz:hq`) et `has_et = 0` (constructeur `mz`) retirées | — |
| `wormhole.h` | `:49` (1) | prototype retiré avec son contrat `:46-48` | (a2) |
| `wormhole_display.c` | `:9` (1) | `#include "pico8_value.h"` retiré (le `.c` ne nomme plus le module ; le type lui vient encore de `wormhole.h:12`) | (a2) |
| | `:12`, `:13`, `:16` (1 chacune) | `pico8_mzd_truthy` retirée entière (`:12-19` et la blanche) | (a2) |
| | `:45` (0) | `if (self->probe->et == NULL) {` | — |
| `slipway.c` | `:21` (1) | `#include "pico8_value.h"` retiré (le `.c` se vide ; `wormhole.h:12`, `probe.h:11` l'incluent encore) | (a2) |
| | `:107` (1) | `pico8_rn(pico8_fixed_from_int(50));` | (a1) |
| | `:176` (1) | `pico8_rn(pico8_fixed_from_int(-1));` | (a1) |
| | `:150`, `:436` (0) | `has_et = et != NULL` (`jc:hq`) et `has_et = 0` (constructeur `jc`) retirées | — |
| `planet_family.c` | `:80` (0) | `self->probe->has_et = 0;` retirée (constructeur `qh` ; `:91` à `c77124e`) | — |
| `audio.c` | `:24` (1) | `void pico8_rn(pico8_fixed_t no) {` | (b) |
| | `:25` (1) | local `truthy` retiré | (a2) |
| | `:27` (0) | `if (!pico8_ih()) {` | — |
| | `:28` (1) | `int n = pico8_fixed_flr_int(no);` (la ligne reste, le repli « pas un nombre → 0 » part : aucun appelant n'en passe) | (a1) |
| `audio.h` | `:17` (1) | contrat `:16-18` retiré (il décrivait le `false` que le type n'admet plus) | (a2) |
| | `:19` (1) | `void pico8_rn(pico8_fixed_t no);` | (b) |
| `planet.c` | `:421`, `:423` (1 chacune ; `:445`, `:447` à `c77124e`) | `pico8_rn(hp_instance->rn);`, `pico8_rn(pico8_fixed_from_int(55));` | (a1) |
| `msgbox.c` | `:339` (1) | `pico8_fixed_t rn_arg;` | (a1) |
| | `:361` (1) | `rn_arg = frame->eo;` | (a1) |
| | `:363` (1) | `rn_arg = pico8_fixed_from_int(62);` | (a1) |
| `ui_button.c` | `:342` (2) | retirée ; `pico8_rn(pico8_fixed_from_int(60));` entre en tête du bloc `if (has_my) {` (`:343-345`), avant `self->my(self);` | (a1) |

Totaux `src/` b1 (lignes / occ. retirées) : `probe.c` 15 / 16, `wormhole.c` 5 / 5, `wormhole.h` 1 / 1, `wormhole_display.c` 4 / 4, `slipway.c` 3 / 3, `audio.c` 3 / 3, `audio.h` 2 / 2, `planet.c` 2 / 2, `msgbox.c` 3 / 3, `ui_button.c` 1 / 2 : **39 lignes, 41 occ.** Classes : (a1) 21, (a2) 16, (b) 2.

#### b1 — Les TU suivent (1 ligne, 1 occ. retirée ; le reste hors motif)

| TU | Ligne(s) | Nouveau texte | Classe |
|---|---|---|---|
| `mz` | `:337` (1) | retirée : elle testait l'accesseur retiré ; `:336` teste le membre | (a2) |
| `mz` | `:151-153` | branche `ET_ABSENT` d'`et_kind` retirée ; **l'`enum` `:135` est gardé tel quel** (`ET_FALSE` = 1 … : les valeurs sont pliées par `fold_hq`) | — |
| `mz` | `:336` | `… && mz->probe->et == NULL && …` à la place de `!mz->probe->has_et` | — |
| `mz` | `:426`, `:444`, `:766` | `assert(et != NULL);` ; `if (et != NULL) {` ; `assert(mz->probe->et != NULL && other->probe->et != NULL);` | — |
| `g_` | `:426-428`, `:461-463` | branches `!has_et` d'`et_kind` et de `fold_et` retirées (valeurs de retour 1–4 inchangées) ; commentaires `:419-420`, `:457-458` sans « absent » | — |
| `g_` | `:366`, `:448`, `:778` | `pr->et == NULL` ; `assert(et != NULL);` ; `assert(a->probe->et != NULL);` | — |
| `fv` | `:358` | `pr->et == NULL` à la place de `!pr->has_et` | — |
| `os:168`, `hj:164`, `nl:154`, `qh:126` | `et_state` | `return qh->probe->et == NULL ? 1u : 3u;` ; commentaire au-dessus : « 1 no target, 3 a target: the bytes the frozen hashes fold. » | — |
| `os:174-175`, `hj:170-171`, `nl:160-161`, `qh:159-160` | `fold_et` | `case 0u: return fold_absent(h);` retiré ; commentaire sans « absent » | — |
| `fr:293`, `mx:354`, `os:325`, `pm:358`, `nr:308`, `hj:361`, `nl:298`, `qh:298` | pli « built » | `h = fnv(h, qh->probe->et != NULL ? 1u : 0u);` | — |
| `qh` | `:427` | `assert(as_qh(out_lab)->probe->et == planet &&` (la moitié `has_et &&` part) | — |

(Remesuré : à `c77124e`, `mx:369`, `os:183/189-190/340`, `pm:383`, `hj:179/185-186/376`, `nl:169/175-176/313`, `qh:125/158-159/297/426`.) **Pourquoi `? 1u : 0u`** : aucun cast nouveau (l'ancien `(uint32_t)…has_et` part avec sa ligne), et une mutation qui ferait valoir 2 au test ne passerait pas pour un pli juste. **Rejeté** : garder `has_et` pour les seuls TU (état que `src/` ne lit plus) ; plier `0u` en constante (le pli ne lirait plus rien) ; `jo.p != NULL` comme témoin de « `bv` a tourné » (faux pour les planètes placées par la galaxie).

Totaux `tests/` b1 : `mz` 1 / 1. **Total b1 : 40 lignes, 42 occ.** ; (a1) 21, (a2) 17, (b) 2, (c) 0.

#### b2 — API

| Symbole | Aujourd'hui | Après b2 | Appelants (`src/`) | TU |
|---|---|---|---|---|
| `pico8_oj` (`tech_gate.c:27`, `.h:10`) | `int (pico8_value_t px)` | **`int pico8_oj(const char *px)`** (NULL : 1) — (b) | `tech_gate.c:45`, `:98`, `research.c:357`, `:377`, `:381`, `research_icon.c:93`, `research_display.c:325` | `hp:526`, `:532`, `:534`, `:535` (G4, retirée), `:541` |
| `pico8_hp_il` (`research.c:251`, `.h:84`) | rend `pico8_value_t` | **`const char *pico8_hp_il(const pico8_hp_t *self)`** — (b) | `research.c:356`, `:373` | aucun |
| `pico8_lq_bg` (`layout.c:301-306`, `.h:127-130`) | rend `pico8_value_t` | **retirée** — (a2) | aucun | `ga:504`, `hp:277` |
| `pico8_pj_ed` (`score_panel.c:291-299`, `.h:105-106`) | rend `pico8_value_t` | **retirée** — (a2) | aucun | `spec_defaults:233` |

#### b2 — sites de `src/` (30 lignes, 33 occ. retirées ; aucune gagnée)

| Fichier | Ligne(s) `valeur` (occ.) | Remplacement | Classe |
|---|---|---|---|
| `tech_gate.c` | `:14`, `:15`, `:18` (1 chacune) | `pico8_tech_gate_truthy` retirée entière avec son commentaire (`:12-22`) | (a2) |
| | `:27` (1) | `int pico8_oj(const char *px) {` | (b) |
| | `:31` (0) | `if (px == NULL) {` | — |
| | `:34` (1) | retirée avec `:35-36` : le cas « pas une chaîne » (G4) | (a2) |
| | `:42` (0) | `return pico8_gu_content_ptrs(invented_list, px);` (`/* TROU-3 */` gardé) | — |
| | `:45` (1) | `if (pico8_oj(px)) {` (pont de D.15a) | (a1) |
| | `:98` (1) | `if (pico8_oj(t->px)) {` | (a1) |
| `tech_gate.h` | `:10` (1) | `int pico8_oj(const char *px);` ; contrat `:8-9` réécrit : « A NULL `px` (the cart's nil or false) answers 1: no prerequisite is an unlocked one. » | (b) |
| `research.c` | `:34`, `:35`, `:38` (1 chacune) | `pico8_research_truthy` retirée entière avec son commentaire (`:32-42`) | (a2) |
| | `:251` (1) | `const char *pico8_hp_il(const pico8_hp_t *self) {` | (b) |
| | `:253` (1) | `return NULL;` | (a1) |
| | `:256` (1) | retirée avec `:255-257` : `il` NULL se rend lui-même | (a2) |
| | `:258` (1) | `return self->il;` | (a1) |
| | `:345`, `:369` (1 chacune) | `const char *il;` | (a1) |
| | `:357`, `:377` (0) | `il != NULL && pico8_oj(il)` à la place de `pico8_research_truthy(il) && pico8_oj(il)` (`oj(NULL)` vaut 1 : le test `il != NULL` est le `self.il and` du cart, pas une redondance) | — |
| | `:381` (1) | `pico8_oj("nanomaterials")` | (a1) |
| | `:73-107`, `:133` (0) | `hp_pose` par position (voir « `hp_pose` ») ; appel `hp_pose(self, p, s, is_fixed, fixed, i_src);` | — |
| `research.h` | `:84` (1) | `const char *pico8_hp_il(const pico8_hp_t *self);` ; contrat `:83` : « `il` as the cart reads it: NULL when nil, false or `self` is NULL. » | (b) |
| `research_icon.c` | `:93` (2) | `if (!pico8_oj(rr)) {` (commentaire `:88-89` reste vrai) | (a1) |
| `research_display.c` | `:325` (2) | `if (!pico8_oj(b->rr)) {` | (a1) |
| `layout.c` | `:301`, `:303`, `:305` (1, 1, 2) | `pico8_lq_bg` retirée entière (`:301-306` et la blanche) | (a2) |
| `layout.h` | `:130` (1) | prototype retiré avec son contrat `:127-129` | (a2) |
| `score_panel.c` | `:291`, `:293`, `:296`, `:298` (1 chacune) | `pico8_pj_ed` retirée entière (`:291-299` et la blanche) | (a2) |
| `score_panel.h` | `:106` (1) | prototype retiré avec son contrat `:105` | (a2) |

Totaux `src/` b2 : `tech_gate.c` 7 / 7, `tech_gate.h` 1 / 1, `research.c` 10 / 10, `research.h` 1 / 1, `research_icon.c` 1 / 2, `research_display.c` 1 / 2, `layout.c` 3 / 4, `layout.h` 1 / 1, `score_panel.c` 4 / 4, `score_panel.h` 1 / 1 : **30 lignes, 33 occ.** Classes : (a1) 9, (a2) 17, (b) 4.

#### b2 — Les TU suivent (10 lignes, 10 occ. retirées)

| TU | Ligne(s) `valeur` (occ.) | Remplacement | Classe |
|---|---|---|---|
| `hp` | `:526`, `:532`, `:534`, `:541` (1 chacune) | `pico8_oj(px)`, `pico8_oj("nanomaterials")` ×2, `pico8_oj(b->il)` | (a1) |
| | `:535` (1) | retirée : `pico8_oj` d'un nombre, genre que `src/` ne produit pas (G4, nommé au message) | (a2) |
| | `:269` (1) | local `bg` retiré | (a2) |
| | `:277` (0) | `assert(!lq->has_bg);` à la place de `bg = pico8_lq_bg(lq);` | — |
| | `:280` (1) | le terme `+ !(bg.kind == PICO8_VALUE_REF && bg.as.ref == NULL)` part ; la ligne reste (`… + (lq->has_c != 0) +`), `:281` inchangée — **ligne comptée qui sort de la catégorie sans être retirée** | (a2) |
| | `:288` (1) | branche `bg` retirée avec `:289-290` | (a2) |
| `ga` | `:497` (1) | local `bg` retiré | (a2) |
| | `:504` (0) | `assert(!lq->has_bg);` à la place de `bg = pico8_lq_bg(lq);` | — |
| | `:514` (1) | branche `bg` retirée avec `:515-517` | (a2) |
| `spec_defaults` | `:233` (0) | `assert(pj->ed == NULL);` (commentaire `:224` reste vrai) | — |

Totaux `tests/` b2 : `hp` 8 / 8, `ga` 2 / 2. **Total b2 : 40 lignes, 43 occ.** ; (a1) 13, (a2) 23, (b) 4, (c) 0. `test_pico8_lq` : **inchangé** (`bg_is_bool` gardé).

#### `hp_pose` (G7, sans compte)

Forme décidée ici (les trois `assert` de genre gardés, le choix du membre par position ; `p` borné par l'`assert(ri->npos <= self->mr->count && ri->npos <= 8)` de `hp_ho`) :

```c
static void hp_pose(pico8_hp_t *self, int p, const char *s, int is_fixed, pico8_fixed_t fixed,
                    const char **i_src) {
    if (p == 0 || p == 1 || p == 3 || p == 4) {
        assert(s != NULL && "hp.px/lr/ja/i: …");      /* message inchangé */
    } else if (p == 2 || p == 5 || p == 7) {
        assert(is_fixed && "hp.fj/o/dm: …");          /* message inchangé */
    } else {
        assert(!is_fixed && "hp: `il` gets a number -- stop, do not add one");
    }
    switch (p) {
    case 0: self->px = s; break;
    case 1: self->lr = s; break;
    case 2: self->fj = fixed; self->has_fj = 1; break;
    case 3: self->ja = s; break;
    case 4: *i_src = s; break;
    case 5: self->o = fixed; self->has_o = 1; break;
    case 6: self->il = s; self->has_il = 1; break;
    default: self->dm = fixed; self->has_dm = 1; break;
    }
}
```

(Une instruction par ligne à l'écriture, style du fichier.) Positions : `pico8_hp_mr_names` = `px, lr, fj, ja, i, o, il, dm` (`game_data.c:270`), le même ordre que `strings[]` de `hp_ho` (`research.c:111`) et que `mr=o(…)` du cart (1400). Le commentaire de tête garde son contrat (« `s` NULL with `is_fixed` 0 is false… ») et gagne « position `p` is `hp.mr[p]`, the class's constant ». Rejeté : un `assert` répété dans chaque `case` (quatre copies d'un message) ; garder `name` et le `strcmp` (G7).

#### Copies `truthy`

b1 retire `pico8_probe_truthy`, `pico8_mzd_truthy` ; b2 `pico8_research_truthy`, `pico8_tech_gate_truthy`. **7 → 5 → 3** : restent `layout` (B8, D.18a), `research_display` (argument `o` de `jo_ex`, D.17), `ui_button` (B9, D.18b).

#### Includes (G, règle de D.15b)

Un `.c` qui ne nomme plus rien du module perd son `#include "pico8_value.h"` ; un en-tête le garde jusqu'à D.19c. b1 : **`wormhole_display.c:9` et `slipway.c:21` retirés** (comptés ci-dessus) ; `probe.c`, `wormhole.c`, `audio.c`, `planet.c` se vident sans include propre. En-têtes gardés : `audio.h:4` (seule ligne restante ; il fournit aussi `pico8_fixed_t` à `pico8_rn` — pour D.19c : le remplacer par `pico8_fixed.h`), `wormhole.h:12`, `probe.h:11`. b2 : aucun include touché (`tech_gate.c`, `research_icon.c` n'en ont pas ; `tech_gate.h:6`, `research.h:9`, `layout.h:9`, `score_panel.h:12` gardés). Aucun include ajouté ; si la compilation en exige un, il est ajouté et déclaré avec l'erreur du compilateur.

#### `Makefile` et liens

- Aucun fichier nouveau ; **aucune ligne du `Makefile` ne change**.
- Règle de lien : aucun fichier de `DATA_ENGINE_SRCS` (`Makefile:229-279`) ne nomme un symbole défini hors de cette liste. D.16b **n'ajoute aucun symbole** et en retire trois (`pico8_mz_et`, `pico8_lq_bg`, `pico8_pj_ed`) : grep 1 des Sorties prouve qu'aucun appelant ne reste. `research_icon.c` (hors liste) appelle `pico8_oj` (`tech_gate.c`, dans la liste) : permis. Les binaires liés sur `DATA_ENGINE_SRCS` seule (`data_init`, `map_tables`, `jb_ol`, `planet_kj_dispatch`, `bg_kj`), sous ASan (`planet_gh_row_lq`, `Makefile:550`) ou avec `planet_family.c` (`ga_export_panel_by`, `:553`) doivent se lier aux deux commits (grep 6).

## Solution

**Deux commits, même PRD** (`refactor(D.16b1)`, `refactor(D.16b2)`), chacun vert seul, vérifié, poussé, avec sa note :

**b1** (24 chemins : 12 de `src/`, 11 TU, `docs/plan-3.md`)
1. La cible `et` d'une sonde est un pointeur : NULL pour nil et pour `false` ; `has_et` disparaît ; `pico8_mz_et`, `pico8_probe_truthy`, `pico8_mzd_truthy` disparaissent ; le pont `probe.c:182` part.
2. `pico8_rn` prend un nombre ; `js:ldown` l'appelle sous `has_my`.
3. Onze TU plient `et` aux mêmes octets sans le drapeau.

**b2** (14 chemins : 10 de `src/`, 3 TU, `docs/plan-3.md`)
1. `pico8_oj` prend un `const char *`, `pico8_hp_il` en rend un ; le pont `tech_gate.c:45` part ; `pico8_research_truthy`, `pico8_tech_gate_truthy` disparaissent.
2. `hp_pose` pose par position.
3. `pico8_lq_bg` et `pico8_pj_ed` disparaissent ; `bg_is_bool` reste (G1).
4. `hp`, `ga`, `spec_defaults` suivent. Aucune constante ne change.

**Sortie de b1**, constatée par commande :

- `make test` vert (dont `data-guard`, `constants-check`, `float-grep`, `float-guard`) ; `make e2e` **26 conformes** ; `make class-coverage-net` **identique** à l'attendu ; balayage ASan+UBSan des TU propre ;
- `make data-residue` : `valeur` **825 → 785** lignes (**916 → 874** occ.), fichiers **97 → 91** (sortent : `probe.c`, `wormhole.c`, `wormhole_display.c`, `audio.c`, `slipway.c`, `planet.c`) ; `type-tag` 242 / 58 inchangé ; résidu D **1065 / 107 → 1025 / 105** (`wormhole_display.c`, `audio.c` sortent : aucune ligne `type-tag`) ; départs mesurés à `0b8ea0d` (si un commit `docs/` s'intercale, départ réel − 40 / − 42 occ.) ;
- ces commandes rendent exactement ce qui est écrit :

  ```sh
  P='\bpico8_value_[A-Za-z0-9_]+|\bPICO8_VALUE_[A-Z0-9_]+|pico8_value\.h'
  # 1. drapeau et accesseur partis : vide
  grep -rnE 'has_et|pico8_mz_et|pico8_probe_truthy|pico8_mzd_truthy' src tests
  # 2. rn typé : une ligne, 'void pico8_rn(pico8_fixed_t no);'
  grep -nE 'pico8_rn\(' src/*.h
  # 3. copies truthy : 5 fichiers (layout, research, research_display, tech_gate, ui_button)
  grep -lE 'static int [a-z0-9_]*truthy\(pico8_value_t' src/*.c
  # 4. plus aucun rn enveloppé : vide
  grep -rnE 'pico8_rn\(.*pico8_value_' src
  # 5. fichiers vidés : 0 partout
  grep -cE "$P" src/pico8_probe.c src/pico8_wormhole.c src/pico8_wormhole_display.c \
       src/pico8_audio.c src/pico8_slipway.c src/pico8_planet.c
  # 6. règle de lien
  make tmp/test_pico8_data_init tmp/test_pico8_map_tables tmp/test_pico8_jb_ol \
       tmp/test_pico8_planet_kj_dispatch tmp/test_pico8_bg_kj tmp/test_pico8_planet_gh_row_lq \
       tmp/test_pico8_ga_export_panel_by
  # 7. aucune constante de TU changée : vide
  git diff <parent> -- tests | grep -E '^[-+].*(_HASH|identity\[)' | grep -vE '^[-+]\s*(/\*|\*)'
  # 8. includes : exactement deux lignes '-' (wormhole_display.c, slipway.c), aucune '+'
  git diff <parent> | grep -E '^[-+]\s*#include "(\.\./src/)?pico8_value\.h"'
  # 9. comptes ; 10. filet de classes
  make -s data-residue; sh tools/data_residue.sh --detail | grep -E '^valeur'
  make class-coverage-net
  ```

**Sortie de b2** :

- mêmes filets verts ;
- `valeur` **785 → 745** lignes (**874 → 831** occ.), fichiers **91 → 89** (`tech_gate.c`, `research_icon.c` sortent) ; `type-tag` inchangé ; résidu D **1025 / 105 → 985 / 104** (`tech_gate.c` sort) ;
- commandes :

  ```sh
  # 1. accesseurs et copies partis : vide
  grep -rnE 'pico8_lq_bg|pico8_pj_ed|pico8_research_truthy|pico8_tech_gate_truthy' src tests
  # 2. signatures : 'int pico8_oj(const char *px);' et 'const char *pico8_hp_il(const pico8_hp_t *self);'
  grep -nE 'pico8_oj\(|pico8_hp_il\(' src/*.h
  # 3. copies truthy : 3 fichiers (layout, research_display, ui_button)
  grep -lE 'static int [a-z0-9_]*truthy\(pico8_value_t' src/*.c
  # 4. plus aucun oj enveloppé ni d'un nombre : vide
  grep -rnE 'pico8_oj\([^)]*pico8_value_' src tests
  # 5. hp_pose par position : aucun strcmp de nom ; bg_is_bool gardé (lecteur ui_button.c:325)
  sed -n '/^static void hp_pose/,/^}/p' src/pico8_research.c | grep -c strcmp        # 0
  grep -c bg_is_bool src/pico8_layout.h                                               # 2
  # 6. à 9. : comme b1 (liens ; constantes ; includes : vide ; comptes, class-coverage-net)
  ```

## User Stories

1. En tant que mainteneur du port, je veux que la cible `et` d'une sonde soit calculée comme un pointeur dans `bv`, afin que planète, slipway, vue jetable et absence se lisent sans déballer une valeur.
2. En tant que mainteneur du port, je veux que nil et `false` de `et` soient un même NULL, afin que le type dise ce que les cinq lecteurs du cart en lisent : sa vérité.
3. En tant que mainteneur du port, je veux que `has_et` disparaisse, afin qu'aucun drapeau ne reste que `src/` ne lit jamais sans `et != NULL`.
4. En tant que mainteneur du port, je veux que `pico8_mz_et` disparaisse, afin que l'anneau de `mz:kj` teste le membre.
5. En tant que mainteneur du port, je veux que `pico8_rn` prenne un nombre et que `js:ldown` ne l'appelle qu'avec une action, afin qu'aucun appel ne passe `false` pour dire « rien ».
6. En tant que mainteneur du port, je veux que `pico8_oj` prenne une chaîne (NULL : pas de prérequis), afin que `ov`, `cb`, `ie:hb`, `bj`, `hp:po` et `hp:od` lui passent ce qu'ils ont.
7. En tant que mainteneur du port, je veux que `pico8_hp_il` rende la chaîne ou NULL, afin que `hp:po` et `hp:od` écrivent `self.il and oj(self.il)` en C.
8. En tant que mainteneur du port, je veux que `hp_pose` pose par position, afin que la construction d'un projet n'aiguille plus sur des noms qu'une constante de classe fixe.
9. En tant que mainteneur du port, je veux que `pico8_lq_bg` et `pico8_pj_ed`, que seuls des TU appelaient, disparaissent, afin qu'aucune API à valeur ne survive pour les tests.
10. En tant que mainteneur du port, je veux que les deux ponts déclarés (`ov` → `oj`, `bv` → `et`) partent avec leur objet, afin qu'aucune valeur ne soit fabriquée pour être aussitôt défaite.
11. En tant que mainteneur du port, je veux que quatre copies privées de `*_truthy` partent avec leur dernier argument, afin qu'il n'en reste que trois, rangées dans leurs étapes.
12. En tant que gardien des tests, je veux que les onze TU qui plient `has_et` plient `et` aux mêmes octets, avec la preuve écrite du contexte (« built » avant `bv`, lectures après `bv`), afin qu'aucune constante ne change.
13. En tant que gardien des tests, je veux que `bg_is_bool` reste tant qu'une empreinte figée sépare `false` de nil, afin que G1 l'emporte sur une représentation plus courte.
14. En tant que gardien des tests, je veux que les branches `bg` mortes de `ga` et `hp` deviennent `assert(!lq->has_bg)`, afin que l'invariant soit vérifié plutôt que plié.
15. En tant que gardien des tests, je veux que le cas `pico8_oj` d'un nombre soit retiré et nommé au message (G4), afin qu'aucun test ne garde un genre que `src/` n'écrit pas.
16. En tant que gardien des filets, je veux `make test`, `make e2e`, `make class-coverage-net` identique et le balayage ASan+UBSan verts à chaque commit, afin de prouver qu'aucun constructeur, pixel ou accès mémoire ne change.
17. En tant que gardien des tests, je veux `gcov` avant/après et des mutations restaurées par `cp`, afin que chaque ligne modifiée soit exécutée et qu'aucune ligne survivante ne cesse de l'être.
18. En tant que gardien du compteur, je veux au message de chaque commit les comptes par catégorie et par fichier en lignes et occurrences, la classification (a1)/(a2)/(b)/(c), les lignes qui gagnent une catégorie (« aucune » compris), la ligne `type-tag` modifiée et la ligne `hp:280` sortie de catégorie, avec les numéros du parent, afin qu'aucune baisse ne soit un tassement.
19. En tant que `plan-pilot`, je veux deux sous-étapes d'environ quinze minutes chacune, un plafond, un repli et une note « Découvert à D.16b1 / b2 », afin que D.17 et la suite partent d'un état exact.

## Implementation Decisions

Pas d'humain dans la boucle. Le skill `to-prd` demande de faire valider modules, tests et découpage. Ces choix sont tranchés ici comme le ferait un agent `guideline` (hypothèses explicites, simplicité, changement chirurgical, critères vérifiables) et marqués **(G)** ; les rejets sont notés. Q12 (oui) et Q13 (A) sont des décisions humaines ; l'étude D.14 (G1–G7) fait foi sauf écart déclaré ici (B6 : `bg_is_bool` gardé ; B4 : `has_il` gardé ; `probe.c:211` `type-tag` gardé).

### Modules

- **Sonde** (b1) : `pico8_probe.c/.h` (`bv`, `dl`, `d_`, constructeur `g_`), `pico8_wormhole.c/.h`, `pico8_wormhole_display.c`, `pico8_slipway.c` (`jc:hq`, constructeur `jc`), `pico8_planet_family.c` (constructeur `qh`).
- **Audio** (b1) : `pico8_audio.c/.h` ; appelants `planet.c`, `probe.c`, `wormhole.c`, `slipway.c`, `msgbox.c`, `ui_button.c`.
- **Recherche** (b2) : `pico8_tech_gate.c/.h`, `pico8_research.c/.h` (`hp_il`, `hp_po`, `hp_od`, `hp_pose`), `pico8_research_icon.c`, `pico8_research_display.c`.
- **Rangée et panneau** (b2) : `pico8_layout.c/.h`, `pico8_score_panel.c/.h`.
- **TU** : b1 `fr`, `fv`, `g_`, `hj`, `mx`, `mz`, `nl`, `nr`, `os`, `pm`, `qh` ; b2 `hp`, `ga`, `spec_defaults`. **Docs** : `plan-3.md` (deux notes).
- Inchangés : `Makefile`, `tools/`, `pico8_value.c/.h`, `pico8_type_tag.h`, `planet_display.c`, `ui_button.h`, `test_pico8_lq`, `test_pico8_jc`, `docs/glossaire-noms.md` (`rn`, `oj` y décrivent le cart, toujours vrai), `docs/trous-de-portage.md` (TROU-3, TROU-10 restent vrais).

### Représentation (G)

- `et` : `pico8_struct_header_t *` (membre inchangé), NULL = nil ou `false`. Aucun nom neuf. `ET_ABSENT` reste dans l'`enum` de `test_pico8_mz` (valeurs pliées).
- `oj` : `const char *px`, NULL → 1 ; `hp_il` : `const char *`, NULL pour nil, `false`, `self` NULL ; `has_il` gardé.
- `rn` : `pico8_fixed_t no` (même nom de paramètre, glossaire).
- `bg` : `has_bg`, `bg_is_bool`, `bg` gardés tels quels.
- **Piège de motif** : aucun nom neuf ne doit correspondre à un motif gardé (`spec`, `par-nom`…) ni à `valeur`.

### Anti-tassement et message de commit (liant, pour chacun des deux commits)

- **Interdit** : joindre des lignes (la condition de l'`assert` de `bv` garde ses lignes ; les deux `if` de `hp_il` ne sont pas fondus au-delà du retrait décrit) ; renommer pour échapper au motif ; alias (`#define`, `typedef`) ; cast nouveau (les plis écrivent `? 1u : 0u` ; `first = filtered.items[0]` et `et = &g_bv_et.header` sans cast) ; **cale valeur ↔ typé** (`pico8_value_bool(lq->bg_is_bool …)` dans un TU, `pico8_oj_value`, une `pico8_mz_et` gardée pour un seul TU) ; toucher une constante de hachage, `tests/e2e/`, un `*.sha256`, `tools/`, le `Makefile`, un include autre que les deux de b1.
- **Classification exigée** (recette : `git diff -U0 <parent>` par fichier, lignes `-` qui portent le motif, en regard des `+`) : **b1 (a1) 21, (a2) 17, (b) 2, (c) 0** ; **b2 (a1) 13, (a2) 23, (b) 4, (c) 0** — à `c77124e`, recomptées au parent.
- **Mesure exigée**, avant et après, par catégorie (`valeur`, `type-tag` ; les huit gardées à 0 constatées) et par fichier du diff, en lignes (`--detail`) **et** occurrences.
- **Lignes qui gagnent une catégorie**, par fichier et catégorie, « aucune » compris : attendu **aucune**.
- **Lignes comptées modifiées, non retirées** : b1 **une**, `probe.c:211` (`type-tag`, argument `et.as.ref` → `et`) ; b2 **une**, `hp:280` (sort de `valeur`, la ligne reste). Toute autre est à justifier.
- **Δlignes > Δoccurrences** : attendu aucun fichier (Δocc ≥ Δlignes partout : `probe.c` 15 / 16, `ui_button.c` 1 / 2, `research_icon.c` 1 / 2, `research_display.c` 1 / 2, `layout.c` 3 / 4).
- **Numéros de ligne** : ceux du parent de chaque commit.

### Empreinte et découpage (G)

| Commit | `src/` | `tests/` | autres | Chemins |
|---|---|---|---|---|
| **D.16b1** | `probe.c`, `probe.h`, `wormhole.c`, `wormhole.h`, `wormhole_display.c`, `slipway.c`, `planet_family.c`, `audio.c`, `audio.h`, `planet.c`, `msgbox.c`, `ui_button.c` (12) | `fr`, `fv`, `g_`, `hj`, `mx`, `mz`, `nl`, `nr`, `os`, `pm`, `qh` (11) | `plan-3.md` | 24 |
| **D.16b2** | `tech_gate.c`, `tech_gate.h`, `research.c`, `research.h`, `research_icon.c`, `research_display.c`, `layout.c`, `layout.h`, `score_panel.c`, `score_panel.h` (10) | `hp`, `ga`, `spec_defaults` (3) | `plan-3.md` | 14 |

(L'étude disait ≈ 25 fichiers en tout et nommait les TU `hp`, `mz`, `g_`, `ga`, `spec_defaults`, `lq` : mesuré, 36 chemins de code ; `lq` sort (G1), neuf TU entrent pour `has_et`.)

- **Rythme mesuré** : D.15a–D.15c ≈ 7 à 12 min chacune de la mise en file à la graduation. **b1** : éditions ≈ 6 min (11 TU mécaniques) ; `gcov` départ/arrivée ≈ 4 ; 10 mutations ≈ 4 ; filets ≈ 3 ; mesures, message, note ≈ 3 : **budget ≈ 20 min, plafond 45 min**. **b2** : éditions ≈ 5 ; `gcov` ≈ 4 ; 12 mutations ≈ 5 ; filets ≈ 3 ; mesures ≈ 3 : **budget ≈ 20 min, plafond 45 min**. Chaque sous-étape ≤ 50 min.
- **Repli écrit** (G) : si b1 atteint son plafond sans commit, rien n'est commis et `plan-pilot` relance en **b1a** (B5 seul : `rn`, 8 fichiers de `src/`, aucun TU, − 16 lignes / − 17 occ. ; `probe.c`, `wormhole.c`, `slipway.c` n'y changent que leurs lignes `rn` et `slipway.c` y perd son include) puis **b1b** (B3). Si b2 atteint son plafond : **b2a** (B4 + `hp_pose`) puis **b2b** (B6, B7). Comptes recomptés par commit. Rejeté comme plan principal : quatre coûts fixes au lieu de deux.

### Effet sur `make data-residue`

| Catégorie | À `c77124e` | Départ (mesuré à `0b8ea0d`) | Après b1 | Après b2 |
|---|---|---|---|---|
| `valeur` | 879 / 99 ; 971 occ. | 825 / 97 ; 916 occ. | **785 / 91 ; 874 occ.** (− 40, − 42) | **745 / 89 ; 831 occ.** (− 40, − 43) |
| `valeur` `src/` | 415 / 61 ; 466 occ. | 387 / 59 ; 438 occ. | 348 / 53 ; 397 occ. | 318 / 51 ; 364 occ. |
| `valeur` `tests/` | 464 / 38 ; 505 occ. | 438 / 38 ; 478 occ. | 437 / 38 ; 477 occ. | 427 / 38 ; 467 occ. |
| `type-tag` | 243 / 58 | 242 / 58 | inchangé | inchangé |
| résidu D | 1120 / 108 | 1065 / 107 | **1025 / 105** | **985 / 104** |
| `DATA_RESIDUE_GUARD` | 8 catégories | inchangé | inchangé | inchangé |

Par fichier (lignes / occ. ; départ attendu → après) — b1 : `probe.c` 15/16 → 0/0 ; `probe.h` 1/1 → 1/1 ; `wormhole.c` 5/5 → 0/0 ; `wormhole.h` 2/2 → 1/1 ; `wormhole_display.c` 4/4 → 0/0 ; `slipway.c` 3/3 → 0/0 ; `audio.c` 3/3 → 0/0 ; `audio.h` 3/3 → 1/1 ; `planet.c` 2/2 → 0/0 ; `msgbox.c` 6/6 → 3/3 ; `ui_button.c` 17/23 → 16/21 ; `test_pico8_mz` 7/8 → 6/7 ; `planet_family.c`, les dix autres TU : 0 ligne `valeur` touchée. b2 : `tech_gate.c` 7/7 → 0/0 ; `tech_gate.h` 2/2 → 1/1 ; `research.c` 12/13 → 2/3 ; `research.h` 3/3 → 2/2 ; `research_icon.c` 1/2 → 0/0 ; `research_display.c` 27/33 → 26/31 ; `layout.c` 31/32 → 28/28 ; `layout.h` 5/5 → 4/4 ; `score_panel.c` 19/23 → 15/19 ; `score_panel.h` 3/3 → 2/2 ; `test_pico8_hp` 43/44 → 35/36 ; `test_pico8_ga` 30/30 → 28/28 ; `spec_defaults` 0/0.

Fonctions publiques qui nomment `pico8_value_t` (Annexe B) : 13 → **11** (b1 : `rn`, `mz_et`) → **7** (b2 : `oj`, `hp_il`, `lq_bg`, `pj_ed`).

« − 40 / − 42 » et « − 40 / − 43 » (ou départ réel moins ces deltas), « inchangé » sont des critères (écart = explication chiffrée au message).

### Documentation (G)

- `docs/plan-3.md` : une note « Découvert à D.16b1 », puis « Découvert à D.16b2 », en fin de ligne D.16b, rien d'autre. Elles disent : le commit ; les comptes réels et l'écart à ≈ 765 (base 825) ; les lectures (`pico8_mz_et` retirée ; `has_et` et ses plis de TU ; `has_il` gardé ; `bg_is_bool` gardé par G1, ajouté à la puce « `pk`/`pk_is_bool` … `fp_is_bool` » de l'l. 302 **dans la note**, pas par édition de cette puce) ; les copies `truthy` restantes ; **pour D.17** : `research_display` garde sa copie (`o` de `jo_ex`) ; **pour D.19c** : `audio.h`, `wormhole.h`, `probe.h`, `tech_gate.h` n'ont plus que leur include ; **pour D.20b** : `probe.c:211` porte encore l'`assert` d'étiquette sur `et`.
- Commentaires : quatre sortes de `CLAUDE.md` ; contrats réécrits (`probe.h:20-24`, `tech_gate.h:8-9`, `research.h:83`, tête de `hp_pose`) ; contrats et commentaires retirés avec leur objet (`audio.h:16-18`, `wormhole.h:46-48`, `layout.h:127-129`, `score_panel.h:105`, commentaires des copies `truthy`) ; commentaires de TU (`et_state`, `fold_et`, `et_kind`) sans « absent » ; aucun renvoi à D.16b ni au plan dans `src/` ou `tests/`.

### Ce qui ne change pas

- `tests/e2e/`, tout `*.sha256`, `tools/`, le `Makefile`, tout include hors les deux de b1 ; aucun e2e ni TU ajouté ou supprimé ; toute constante `*_HASH`/`identity[]` ; `tools/class_coverage_net_expected.txt`.
- Le comportement : mêmes cibles de sonde, même anneau de `mz:kj`, même `dl`, même `d_`, même porte de techs (`ov`, `cb`, `ie:hb`, `bj`), mêmes coûts de projet (`po`, `od`), même `bg` dessiné, mêmes durées de vie ; `make e2e` conforme au bit près.
- `pico8_jy_qe`, `pico8_trade_qe`, `economy_value_equal_content`, `jo_ex` (B2, D.17) ; cases, sources de bouton, `pi`, `s[i]` (B8–B11, D.18) ; `pk`/`fp_is_bool`/`bg_is_bool` ; les `assert` d'étiquette (D.20).

### Hypothèses d'exécution (rien n'a été compilé ni lancé)

- **H1** : sous `-Wall -Wextra -Werror`, les quatre copies `truthy` deviennent fatales une fois leurs appels partis (retirées dans le même commit) ; dans `os`, `hj`, `nl`, `qh`, `g_`, `mz`, aucune aide ne devient sans appel (H-D16a-7 ; `fold_absent` garde ≥ 3 appels) ; `ET_ABSENT` inutilisé ne déclenche aucun avertissement (constante d'`enum`).
- **H2** : aucun autre lecteur ou écrivain de `has_et`, `pico8_mz_et`, `pico8_rn`, `pico8_oj`, `pico8_hp_il`, `pico8_lq_bg`, `pico8_pj_ed` n'existe hors ceux de l'Inventaire (grep à `c77124e`, `src/`, `tests/`, `tools/`) ; un oublié se révèle à la compilation et suit (listé).
- **H3** : toutes les lectures d'état de `et` des TU sont soit « built » (avant tout `bv`, `et` NULL), soit après un `hq` qui passe par `bv` (preuve B3) ; une constante qui rougit invalide H3 : arrêt, rapporté.
- **H4** : `jo.ex` et `hp:lq` ne posent jamais `bg` (preuve B6) ; un `assert(!lq->has_bg)` qui tombe invalide H4 : arrêt, rapporté.
- **H5** : `pico8_rn` est exécuté par des TU (`d_`, `jc:pq`, `mz:pq`, `pz`, `js:ldown`), mais aucun n'observe son effet (`pico8_sfx` vide).
- **H6** : `wormhole_display.c` et `slipway.c` compilent sans leur include propre (`wormhole.h`, `probe.h` l'incluent).

## Implementation Chunks

**b1** (un commit)
1. Mesure de départ (`0b8ea0d` ou un descendant `docs/` seul : 825 / 97 / 916 occ., `type-tag` 242 / 58, résidu D 1065 / 107) ; remesure par contenu ; `gcov` de départ.
2. `probe.h` (membre, commentaire) ; `probe.c` (`bv`, `dl`, `d_`, constructeur `g_`, copie `truthy`, `rn`) ; `wormhole.c/.h`, `wormhole_display.c` ; `slipway.c` ; `planet_family.c`.
3. `audio.c/.h` ; appelants `planet.c`, `msgbox.c`, `ui_button.c` (et les lignes `rn` déjà ouvertes).
4. TU : `mz`, `g_`, `fv`, puis `et_state`/`fold_et`/plis « built » de `os`, `hj`, `nl`, `qh`, `fr`, `mx`, `pm`, `nr` ; `make tmp/test_pico8_<nom>` et lancement de chacun.
5. `make test`, `make e2e`, `make class-coverage-net`, balayage ASan+UBSan ; `gcov` d'arrivée ; mutations M1–M10 ; mesures, classification ; note « Découvert à D.16b1 » ; **commit**, vérification, push.

**b2** (un commit, sur b1 gradée)
1. Mesure de départ (785 / 91 / 874 occ. attendus) ; `gcov` de départ.
2. `tech_gate.c/.h` (`oj`, `ov`, `cb`) ; `research.c/.h` (`hp_il`, `po`, `od`, `hp_pose`) ; `research_icon.c`, `research_display.c`.
3. `layout.c/.h`, `score_panel.c/.h`.
4. TU `hp`, `ga`, `spec_defaults`.
5. Filets, `gcov`, mutations M11–M22, mesures, note « Découvert à D.16b2 », **commit**, vérification, push.

## Testing Decisions

Un bon test juge ce que le code rend ou fait (cible d'une sonde, anneau dessiné, coût d'un projet, techs proposées, membres d'un projet construit), pas la présence d'une ligne. D.16b change des représentations et des signatures sans changer un résultat. La preuve : les TU existants, qui plient ces états dans des constantes inchangées ; les 26 e2e au bit près ; `class-coverage-net` identique ; `gcov` et mutations ; le balayage ASan+UBSan.

### TU touchés (G)

1. b1 : `fr`, `fv`, `g_`, `hj`, `mx`, `mz`, `nl`, `nr`, `os`, `pm`, `qh`. b2 : `hp`, `ga`, `spec_defaults`.
2. Prouvent sans suivre : `jc` (`et` par `jc:hq`, `fold_ref`), `js` (`js:ldown` → `rn` ; `bj`), `fm` (`ov`, `cb`), `ie` (`ie:hb` → `oj(rr)`), `mh`/`ui` (`pz` → `rn`), `lq` (`bg` à trois états, inchangé).
3. **Constantes** : aucune `*_HASH`/`identity[]` ne change ; une constante qui rougit est un bug de conversion (ou H3/H4 fausse : arrêt).
4. **Assertion nouvelle** : seulement `assert(!lq->has_bg)` (`ga`, `hp`, décision B6), ou pour couvrir une ligne modifiée non exécutée, ou pour faire rougir une mutation qui ne rougit rien, dans un test du module, listée au message ; rien de lent, aucun fichier d'événements.
5. **Prior art** : plis typés aux mêmes octets (`? 1u : 0u` de D.16a ; `pico8_d_(…) ? fnv(fnv(h, 0xB0u), 1u) : …` de D.15b) ; accesseurs retirés et lecteurs sur le membre (`pico8_jo_hf`, `pico8_qh_qh`, D.16a ; `pf/mz_value_vec`, D.15b) ; cas G4 retirés (`hg` d'un nombre, D.15b).

### Preuve des tests (§7 : `gcov` et mutations)

**`gcov`**, à chaque sous-étape : dans une copie jetable du scratchpad, tous les TU compilés en `--coverage` (flags du `Makefile` + `GCOV_CFLAGS`, `Makefile:118`), **les TU qui forkent** (enfants de `must_abort`, sortie par `_exit`, dont `hp`) compilés en plus avec **`-D_exit=exit`** ; lancés comme le `Makefile` (`ASAN_OPTIONS=detect_leaks=0` pour `test_pico8_planet_gh_row_lq`) ; union des lignes de `src/` exécutées, départ et arrivée.

- Chaque ligne **ajoutée ou modifiée** de `src/` est exécutée par l'union des TU, sinon par les e2e (`make line-coverage EVENTS="tests/e2e/*.json"` → `tmp/line_coverage.txt`), sinon couverte par une assertion ajoutée, sinon justifiée au message. Attendus b1 : `bv` (les quatre écritures de `et` : `g_` kinds 2/3/1/4, `qh`, `mz`), l'`assert` de `bv`, `dl` (`fv`), `d_` (`qh`, `g_`, `os`), `mz:kj` (`mz`, sinon e2e), `rn` (tout TU qui passe par `d_`, `jc:pq`, `mz:pq`) et ses onze appelants (`pz` par `mh`/`ui`, `js:ldown` par `js`, `hp.rn` de `jo:hb` par e2e au moins). Attendus b2 : `oj` (NULL : `ie`, `hp` ; chaîne : `fm`, `hp`), `ov`, `cb` (`fm`, `js`), `hp_il` (`hp`), `po`/`od` avec `il` NULL et chaîne (`hp`), `hp_pose` — **les huit `case` et les trois `assert`** (`hp` : projets de `gb` ; `build_with_no_px_string`, `build_with_no_o_number` pour les abandons), `research_icon.c:93` (`ie`, sinon e2e), `bj` (`hp`/`js`, sinon e2e).
- **`L`** = lignes **survivantes** de `src/` exécutées au départ et plus à l'arrivée (appariées par contenu et fonction) : **attendu vide** aux deux commits. Partantes, hors `L` : corps des quatre copies `truthy`, de `pico8_mz_et`, `pico8_lq_bg`, `pico8_pj_ed` ; les écritures `has_et` ; `local truthy` de `rn` ; le cas « pas une chaîne » de `oj` ; la branche `il == NULL` de `hp_il` ; les `strcmp` de `hp_pose`. Une ligne de `L` est un comportement perdu : arrêt, rapporté.

**Mutations**, une à la fois, dans l'arbre de travail : fichier sauvegardé par `cp` avant, restauré par `cp` simple puis `touch` après (jamais `git checkout`/`stash`) ; chaque TU lancé exactement comme le `Makefile` le lance ; chacune doit rougir au moins le TU nommé, sinon trou couvert par une assertion ou justifié (mutation équivalente démontrée). **Astuces** : retirer un appel ou une affectation peut échouer sous `-Werror` (fonction `static` ou variable inutilisée) : neutraliser par `0 ? x : …`, `if (0)` ou une condition toujours fausse plutôt que supprimer ; si un `assert` de `src/` attrape la mutation avant le TU (abandon), relancer avec cet `assert` relâché dans la copie mutée, pour montrer que le TU l'attrape aussi, et rapporter les deux.

b1 :
- **M1** `bv` : `et = c;` → `et = 0 ? c : NULL;` → `test_pico8_g_` (kind 2 attendu), `test_pico8_qh` (`fold_held` près d'une planète) ;
- **M2** `bv` : branche `ui:hw` vraie écrit `&g_bv_et.header` au lieu de `NULL` → `g_` (kind 1), `qh` (`:434`), `mz` (`ET_FALSE`) ; l'`assert` de `bv` ne l'attrape pas (la jetable y est admise) ;
- **M3** `bv` : `if (have_first && 0 && first != NULL)` → `g_` (kind 3), `mz` (`ET_JC`) ;
- **M4** `d_` : `has_et = 1;` → `qh` (`d_` sur `poor_lab`), `os`/`hj` (`test_life`) ;
- **M5** `d_` : `has_et = 0;` → l'`assert` `(!has_et || !lu_ok)` de `d_` attrape d'abord ; relancée avec l'`assert` relâché → `os`/`hj`/`nl` (crédits, `ks`) ;
- **M6** `dl` : `if (0 && pb->et != NULL)` → `test_pico8_fv` (écran de `dl`), sinon e2e ;
- **M7** `mz:kj` : `if (self->probe->et != NULL)` → `test_pico8_mz` s'il plie l'écran de `kj`, sinon `make e2e`, sinon trou justifié ;
- **M8** `mz:hq` : `if (0 && nq > threshold)` → `mz` (`ET_JJ`) — le voisin de la ligne `has_et` retirée ;
- **M9** (preuve du pli, TU) `test_pico8_qh` : `et_state` rend `3u` toujours → `qh` rouge (`:434` et `fold_held`), restauré, vert ;
- **M10** (preuve du pli, TU) `test_pico8_fr` : pli « built » `? 0u : 1u` → `fr` rouge (`BUILT_HASH`), restauré, vert.
- **B5 sans mutation** : `pico8_sfx` est vide (TROU-10) ; toute mutation de `rn` ou de ses appels est **équivalente par construction** (aucun observateur, ni TU ni e2e). Preuve : compilation (types), `gcov` (lignes exécutées), e2e au bit près. Déclaré au message.

b2 :
- **M11** `oj` : `if (px == NULL) { return 0; }` → `test_pico8_ie` (icône à `rr` absent, `qh`/`os`), `test_pico8_hp` (`bj`, `po`/`od` ne l'atteignent pas : `il != NULL` d'abord), sinon e2e ;
- **M12** `oj` : `return 1;` à la place du `gu` → `hp:532`/`:534` (`!oj("nanomaterials")`), `fm` ;
- **M13** `ov` : `if (!pico8_oj(px))` → `fm`, `mz` (seuil « space folding ») ;
- **M14** `cb` : `if (0 && pico8_oj(t->px))` → `fm`, `js` ;
- **M15** `hp_il` : `return 0 ? self->il : NULL;` → `hp` (coûts `od` ×1.5, `po` → `mc`) ;
- **M16** `hp_po` : `il != NULL` → `1` (`oj(NULL)` vaut 1) → `hp` (projets sans `il`) ;
- **M17** `hp_od` : même mutation → `hp` ;
- **M18** `hp_od` : `!has_ii && !pico8_oj("nanomaterials")` → `hp` ;
- **M19** `hp_pose` : `case 1` pose `ja`, `case 3` pose `lr` (échange) → `hp` (plis des membres), `spec_defaults` s'il lit un projet ;
- **M20** `hp_pose` : `case 6` pose `has_il = 0` → `hp` (`fold_il` : `false` ≠ nil) ;
- **M21** `research_icon.c:93` : `pico8_oj(0 ? rr : NULL)` → `ie`, sinon e2e ;
- **M22** (preuve de l'invariant, TU) `test_pico8_ga` : `assert(lq->has_bg);` → `ga` abandonne (l'`assert` est exécuté), restauré, vert.

Cibles (numéros à `0b8ea0d`) : `probe.c:150-220`, `:255-270`, `:330-365`, `wormhole.c:60-76`, `wormhole_display.c:40-50`, `tech_gate.c:25-50`, `:95-100`, `research.c:70-125`, `:250-260`, `:340-385`, `research_icon.c:88-95`. Une mutation dont la cible n'existe plus sous cette forme est remplacée par une mutation équivalente sur le même comportement, déclarée.

### Filets

- `make test`, `make e2e` (26 conformes) à chaque commit. Aucun e2e ajouté.
- **`make class-coverage-net`** (exigé) : sortie **identique** à `tools/class_coverage_net_expected.txt` (`g_ 18`, `mz 20`, `jc 26`, `qh 12`, `hp 884`…), aux deux commits.
- **Balayage ASan+UBSan des TU** : tous les TU compilés sous `-fsanitize=address,undefined` dans une copie jetable (`Makefile` inchangé), lancés avec `ASAN_OPTIONS=detect_leaks=0`, aux deux commits.
- `make sanitize-net` : non exigé ; s'il est lancé, résultat rapporté.
- Aucun TU ne rejoue un fichier d'événements.

### Vérification, budget et tableau « Runs »

**Budget : environ 20 min de temps réel par sous-étape**, mesures, `gcov`, mutations et filets compris ; **plafond 45 min chacune** (au-delà : repli b1a/b1b ou b2a/b2b). La relecture par un sous-agent frais est hors budget.

| Cible | Durée attendue |
|---|---|
| `make tmp/test_pico8_<nom>` + lancement, par TU | quelques secondes |
| `make test` | de l'ordre de la minute (`probe.h`, `research.h`, `layout.h` changent : recompilation large) |
| `make e2e` | ~1 s |
| `make class-coverage-net` | < 1 min |
| `make line-coverage EVENTS="tests/e2e/*.json"` | < 1 min |
| `gcov` de tous les TU (départ et arrivée) | quelques minutes |
| mutations (10 en b1, 12 en b2) | < 5 min chacune des deux séries |
| balayage ASan+UBSan des TU | quelques minutes |
| `make data-residue`, recette, classification, greps | < 2 min |

Au-delà du plafond, l'implémenteur s'arrête et note ce qui n'a pas été fait ; rien d'inachevé n'est commis. **Jamais l'appli raylib** ; `make raylib` n'est pas exigé.

**L'implémenteur tient un tableau « Runs »** par sous-étape, une ligne par lancement :

| # | Commande exacte | État de l'arbre (HEAD + sous-étape) | Code de sortie | Ligne clé | Durée |
|---|---|---|---|---|---|
| 1 | `make -s data-residue` | HEAD de graduation de D.16a, avant b1 | 0 | `valeur 825 97` (mesuré à `0b8ea0d`) | … |
| 2 | recette `valeur` | idem | 0 | `valeur 825 lignes 97 fichiers 916 occ.` | … |
| 3 | `grep -rn has_et src tests \| wc -l` | idem | 0 | 40 (16 de `src/`, 24 de `tests/`, inchangé depuis `c77124e`) | … |
| … | … | … | … | … | … |

Le vérificateur le réutilise et ne relance que ce qui suit le dernier changement de l'arbre.

### Contrôles du vérificateur (à chaque commit)

1. **Périmètre** : `git show --stat` ne liste que les chemins de « Empreinte » de la sous-étape (plus un TU suiveur listé au message). Ni `tests/e2e/`, ni `*.sha256`, ni `tools/`, ni `Makefile`.
2. **Greps** de la Sortie de la sous-étape, exactement.
3. **Formes** : b1 — `et` local pointeur, `has_et` absent partout, `pico8_mz_et` absente, `rn(pico8_fixed_t)`, `rn` de `js:ldown` dans le bloc `has_my` ; b2 — `oj(const char *)`, `hp_il` en `const char *`, `has_il` et `bg_is_bool` présents, `hp_pose` sans `strcmp`, `pico8_lq_bg`/`pico8_pj_ed` absentes.
4. **Casts** : aucun nouveau (un retiré à `probe.c:212`).
5. **API** : b1 — seule `pico8_rn` change de signature, seule `pico8_mz_et` disparaît ; b2 — seules `pico8_oj`, `pico8_hp_il` changent, seules `pico8_lq_bg`, `pico8_pj_ed` disparaissent (plus les copies `truthy` et `hp_pose`, `static`).
6. **Liens** : grep 6.
7. **Constantes** : grep 7 vide ; M9, M10, M22 rapportées.
8. **Anti-tassement** : comptes avant/après en lignes ET occurrences, par catégorie et par fichier ; classification présente et juste ; aucune ligne gagnée ; `probe.c:211` et `hp:280` déclarées ; includes (grep 8).
9. **Écarts déclarés** : `bg_is_bool` gardé (G1) ; `has_il` gardé ; `probe.c:211` gardée ; `pico8_mz_et` retirée ; neuf TU de plus que l'étude, `lq` en moins ; B5 sans mutation (TROU-10).
10. **Preuve** : `gcov` (lignes modifiées exécutées ou justifiées ; `L` vide), mutations restaurées par `cp`, `class-coverage-net` et ASan+UBSan rapportés.
11. **Commentaires** : quatre sortes de `CLAUDE.md` ; aucun renvoi à D.16b ni au plan dans `src/` ou `tests/`.
12. **Docs** : note « Découvert à D.16b1 » (puis « b2 ») en fin de ligne D.16b ; rien d'autre dans le plan.

### Critères d'échec (traduits en `FAILED:` par le vérificateur de `prd-autopilot`)

**Départ et filet (§7)**

- `FAILED:` si l'arbre de départ de b1 n'est pas `0b8ea0d` (ou un descendant qui ne touche que `docs/`), propre ; ou si `valeur` n'y vaut pas 825 / 97 / 916 occ., `type-tag` 242 / 58, résidu D 1065 / 107 (mesurés à `0b8ea0d`), sans explication. Idem pour b2 : départ = b1 gradée, 785 / 91 / 874 occ.
- `FAILED:` si le filet (`make test`, `make e2e`) reste rouge après **3 tentatives** sur un commit.
- `FAILED:` si un `*.sha256` est modifié ou supprimé, si un fichier de `tests/e2e/` est touché, si un e2e est ajouté ou retiré, ou si un TU rejoue un fichier d'événements.
- `FAILED:` si un test de régression du §6 change.
- `FAILED:` si un site ne tombe sous aucune règle de ce PRD ni de l'étude (arrêt « champ sans règle »), si un lecteur de `et`, `px`, `il`, `no` ou `ed` qui distingue nil de `false` est découvert (il sort de son groupe : arrêt, rapporté), ou si un écart port/cart découvert n'est pas préservé et décrit par une entrée `TROU-N`.
- `FAILED:` si `make class-coverage-net` diffère de l'attendu, ou si le balayage ASan+UBSan des TU n'est pas propre.

**Périmètre et forme**

- `FAILED:` si `tools/`, le `Makefile` ou un `#include "pico8_value.h"` autre que `wormhole_display.c:9` et `slipway.c:21` (b1) change ; si un commit touche un chemin hors de son « Empreinte » sans le lister comme TU suiveur.
- `FAILED:` si une grep de la Sortie ne rend pas ce qui y est écrit.
- `FAILED:` si une signature ou une fonction hors de la liste du contrôle 5 change ou disparaît ; si `qe`, `jo_ex`, les cases, les sources de bouton, `pk`/`fp_is_bool` ou `bg_is_bool` changent ; si une durée de vie change ; si b1 touche un fichier de b2 ou l'inverse.
- `FAILED:` si un cast nouveau est ajouté, si des lignes comptées sont jointes, si `hp_pose` garde un `strcmp` de nom, ou si les `assert` de genre de `hp_pose` disparaissent.

**Gardes, comptes et anti-tassement**

- `FAILED:` si `make data-guard` n'est pas vert, ou si `DATA_RESIDUE_GUARD` change.
- `FAILED:` si `valeur` ne baisse pas de **40 lignes / 42 occ. (b1)** et **40 lignes / 43 occ. (b2)** exactement depuis le départ réel, sans explication chiffrée ligne à ligne ; si `type-tag` change de compte.
- `FAILED:` **si un compte baisse par tassement** : lignes jointes, alias, renommage d'un identifiant vivant, commentaire compté réécrit hors de son sujet, cale valeur ↔ typé, (c) non nulle, (b) hors de `pico8_rn` (b1) ou de `pico8_oj`/`pico8_hp_il` (b2).
- `FAILED:` si le **message d'un commit** omet l'un de ces éléments : par catégorie et par fichier, les comptes avant/après **en lignes et en occurrences** ; la classification (a1)/(a2)/(b)/(c) avec numéros du parent ; les lignes qui gagnent une catégorie (« aucune » compris) ; les lignes comptées modifiées non retirées (`probe.c:211`, `hp:280`) ; la justification de chaque Δlignes > Δoccurrences ; les fonctions retirées et leurs lecteurs ; les copies `truthy` retirées (raison `-Werror`) ; la preuve du groupe (écritures et lecteurs, cart et port ; pour b1 le contexte des plis `has_et`, pour b2 le conflit G1 de `bg` et le cas G4) ; le rapport `gcov` ; les mutations (et B5 équivalente) ; `class-coverage-net` ; le tableau « Runs » (ou son renvoi au journal). Un commit de rattrapage ne compense pas un message incomplet.
- `FAILED:` si `make constants-check`, `float-grep` ou `float-guard` rougit, ou si `tools/float_guard.txt` diffère après `make test`.

**Preuve, commit et note**

- `FAILED:` si une constante `*_HASH` ou `identity[]` change.
- `FAILED:` si une ligne modifiée n'est exécutée ni par un TU ni par les e2e sans justification ; si `L` n'est pas vide.
- `FAILED:` si une mutation manque, ne se restaure pas par `cp`, ou ne rougit rien sans justification.
- `FAILED:` si un plafond est dépassé sans note (et sans repli), ou s'il manque un tableau « Runs ».
- `FAILED:` s'il manque une note « Découvert à D.16b1/b2 », ou si elle omet : les comptes réels et l'écart à ≈ 765, les lectures, les copies `truthy` restantes, `bg_is_bool` gardé.
- `FAILED:` si un commentaire ajouté ou reformulé sort des quatre sortes de `CLAUDE.md`.

## Out of Scope

- B2 (`qe`, `pico8_jy_qe`, `pico8_trade_qe`, `economy_value_equal_content`, l'argument `o` de `pico8_jo_ex` et la copie `truthy` de `research_display.c`) : D.17.
- B8–B11 (cases de `lq`, sources de bouton et la copie `truthy` de `ui_button.c`, `pi`, `s[i]`, `ou[]`) et la copie de `layout.c` : D.18.
- `bg_is_bool` (gardé ici par G1), `pk`/`pk_is_bool`, `fp_is_bool`, `has_il` : forme « deux absences ou un nombre » sans étape nommée (note du plan).
- Les autres aides d'empreinte des TU (`fold_value`, `vec_ref`, `fold_particle`, octets `PICO8_VALUE_*` de `lq`/`hp`…) : D.19a/b. Les includes des en-têtes vidés (`audio.h`, `wormhole.h`, `probe.h`, `tech_gate.h`) : D.19c. L'`assert` d'étiquette de `bv` (`probe.c:211`) et les autres : D.20.
- Tout e2e ; `make raylib`, l'appli raylib, l'audio (TROU-10), le rendu.

## Further Notes

- **Décisions de l'agent à la place de l'utilisateur** (consigne `to-prd`), tranchées en `guideline` et marquées (G) :
  - **Découpage** : b1 (B3 + B5) / b2 (B4 + B6 + B7 + `hp_pose`), la coupe de l'étude, justifiée par les fichiers partagés ; replis b1a/b1b, b2a/b2b écrits.
  - **Types** : `et` pointeur, NULL pour nil et `false` ; `oj(const char *)` ; `hp_il` → `const char *` ; `rn(pico8_fixed_t)`.
  - **Portée** : `has_et`, `pico8_mz_et`, `pico8_lq_bg`, `pico8_pj_ed` et quatre copies `truthy` retirés ; `has_il`, `bg_is_bool`, l'`assert` d'étiquette de `bv` gardés (écarts à l'étude déclarés, G1 ou hors groupe).
  - **TU** : plis de `et` aux mêmes octets par contexte prouvé (« built » : `et != NULL ? 1u : 0u` ; après `bv` : branche absente retirée) ; branches `bg` mortes de `ga`/`hp` en `assert` ; cas G4 de `oj` retiré.
  - **Drapeaux** : `class-coverage-net` aux deux commits ; balayage ASan+UBSan ; pas de `sanitize-net`.
- **Surprises de mesure, pour `plan-pilot`** : (1) `has_et` est plié dans les empreintes de **onze** TU (l'étude en nommait deux) : G1 tient sans drapeau parce que chaque lecture est soit avant tout `bv`, soit après un `bv` qui pose `has_et = 1` ; (2) **B6 contredit G1** : `test_pico8_lq` sépare `bg` faux de `bg` absent dans deux empreintes figées — `bg_is_bool` reste, seul `pico8_lq_bg` part, `lq` sort de l'empreinte ; (3) les branches `bg` de `ga`/`hp` étaient mortes (le cart n'écrit `bg` qu'en 429) ; (4) `has_il` est plié nil ≠ `false` par `test_pico8_hp` : gardé ; (5) `pico8_value_string(NULL)` passé à l'ancien `oj` aurait fait `strcmp(e, NULL)` — chemin latent jamais pris (les 19 `px` de `nb` sont des chaînes), fermé par le type ; (6) `rn` n'a aucun observateur (TROU-10) : B5 se prouve par `gcov` et types, pas par mutation ; (7) `spec_defaults:233` n'est pas comptée (`pico8_pj_ed` hors motif) mais doit suivre ; (8) `probe.c:211` (`type-tag`) est modifiée sur place, pas retirée comme l'étude l'annonçait ; (9) le delta mesuré (− 80 lignes, − 85 occ.) est exactement celui de l'étude ; l'arrivée ≈ 745 et non ≈ 765 vient de la base de D.16a (825, et non 845) ; (10) deux `.c` perdent leur include (`wormhole_display.c`, `slipway.c`), trois fichiers sortent du résidu D (`wormhole_display.c`, `audio.c`, `tech_gate.c`) ; (11) copies `truthy` 7 → 3 ; fonctions publiques à `pico8_value_t` 13 → 7.
- **Révision après la graduation de D.16a** (`0b8ea0d`) : H-D16a-1 fausse sur `type-tag` (242, non 243) et le résidu D (1065, non 1066), par le retrait de `fold_ref` de `test_pico8_pm` ; toutes les autres hypothèses tiennent ; numéros remesurés (`planet.c`, `planet_family.c`, `score_panel.c`, TU `mx`, `os`, `pm`, `hj`, `nl`, `qh`, `lq`) ; nom du TU de B7 corrigé (`test_pico8_spec_defaults.c`) ; découpage, inventaire, deltas et classification inchangés.
- **Pour D.17** : la copie `truthy` de `research_display.c` n'a plus que `o` de `jo_ex` pour appelant ; `research.c` garde `pico8_jy_qe` (`:137-138`).
- **Pour D.19a/b** : les plis de `et` sont typés dans les onze TU ; `hp` et `ga` n'ont plus de local `bg`.
- **Pour D.19c** : `audio.h:4` (seule ligne, fournit `pico8_fixed_t` à `rn`), `wormhole.h:12`, `probe.h:11`, `tech_gate.h:6` n'ont plus que leur include.

## Annexe A — textes recopiés (`docs/plan-3.md` à `0b8ea0d`, par extraction de lignes)

### Lignes D.16a à D.17 (l. 286-288)

> - D.16a — genres mêlés B1 (valeurs vraies/fausses devenues entiers). `class-coverage-net` : oui. Compte : ≈ 845. Découvert à D.16a : un commit (`refactor(D.16a)`) ; `valeur` 879 / 99 (971 occ.) → **825 / 97 (916 occ.)**, − 54 lignes, − 55 occ., et non ≈ 845 : l'écart vient des 25 lignes de `fold_value` retirée de `mx`, `os`, `pm`, `hj`, `nl` (sans appel sous `-Werror`) et des 10 lignes des quatre copies `truthy` (`planet`, `planet_display`, `economy_score`, `economy_tiers`) ; `type-tag` 243 / 58 → **242 / 58** (surprise : `fold_ref` de `test_pico8_pm` n'avait que `fold_value` pour appelant et part avec elle, `-Werror`) ; résidu D 1120 / 108 → **1065 / 107** (`economy_score.c` sort de `valeur` et du résidu D ; `planet_family.c` sort de `valeur`, reste au résidu par `type-tag`). Lectures : `int hf`, `int qh` (1 / 0), `hf_is_bool`/`qh_is_bool` retirés des structs et des données ; `pico8_jo_hf` et `pico8_qh_qh` retirées, leurs lecteurs lisent le membre ; `pico8_jo_gg` rend `int`, 0 sur `self` NULL, deux tests non joints, TROU-3 gardé ; aucun champ sorti de B1. Trois tests ajoutés pour trois mutations sans rouge : `da` d'un laboratoire (`test_pico8_qh`), `eg` sans planète `ly` (`test_pico8_pj`), « wants more » d'un laboratoire (`test_pico8_lq`). Pour D.16b : 7 copies `truthy` restantes (`layout`, `probe`, `research`, `research_display`, `tech_gate`, `ui_button`, `wormhole_display`). Pour D.19a : `fold_value` déjà absente de `mx`, `os`, `pm`, `hj`, `nl` (et `qh` depuis D.15b), `fold_ref` absente de `pm`. Pour D.19c : `planet.h` et `planet_family.h` gardent leur include de `pico8_value.h`.
> - D.16b — genres mêlés B3–B7 (pointeur NULL pour nil/faux, chaînes, `hp_pose` en affectation par position, `rn`, `bg`, `pj_ed`), en deux sous-étapes b1/b2. `class-coverage-net` : oui. Compte : ≈ 765.
> - D.17 — genres mêlés B2 : `pico8_qe_t` et `pico8_qe_equal` (nil ≠ faux conservé). `class-coverage-net` : oui. Compte : ≈ 720.

(Réextraites de `0b8ea0d` : la ligne D.16a porte sa note « Découvert à D.16a » ; les lignes D.16b et D.17 sont identiques à celles de `c77124e`, H-D16a-9.)

### Ponts déclarés (notes de D.15a et D.15b, l. 283-284, extraits)

> `ov` passe `pico8_value_string(px)` à `oj`, ligne qui part à D.16b avec `oj` ; `pico8_tech_gate_truthy` n'a plus que `oj` pour appelant (B12, D.16b)
> Ligne gagnée : `probe.c:181` `et = pico8_value_ref(c);` (`bv`, `c` en pointeur), rendue par D.16b (B3). […] `pico8_probe_truthy` n'a plus que `first` (`probe.c:196`).

(À `c77124e`, ces deux lignes de `probe.c` sont `:182` et `:197` : D.15c a ajouté une ligne au-dessus.)

### §7, extrait (« En D et en 2 »)

> - **En D et en 2** : `make class-coverage-net` aux étapes dont le diff touche un constructeur, un appel `pico8_class_coverage_record` ou `KNOWN[]`, et à la sortie de phase ; `make sanitize-net` aux étapes qui changent la durée de vie ou le propriétaire d'une donnée […], et à la sortie de phase.

## Annexe B — reproduire

```sh
grep -nE 'self\.et\s*=|\.et\b' pico8/lua.lua                     # écritures et lecteurs de et
grep -nE 'oj\(|\.il\b|rr=' pico8/lua.lua                         # oj, il, rr
grep -nE 'rn\(' pico8/lua.lua                                    # rn et ses appelants
grep -nE 'bg=|c\.bg|\.ed\b|ed=' pico8/lua.lua                    # bg (429, 275), ed (846, 882, 899)
grep -rn 'has_et' src tests                                      # 40 lignes à 0b8ea0d (16 src, 24 tests)
grep -rnE 'et_state|fold_et|et_kind' tests                       # plis d'état de et
grep -rnE 'pico8_mz_et|pico8_lq_bg|pico8_pj_ed|pico8_hp_il|pico8_oj\(|pico8_rn\(' src tests
grep -nE 'static int [a-z0-9_]*truthy\(pico8_value_t' src/*.c    # 7 à 0b8ea0d (11 à c77124e)
grep -nE 'pico8_value_t' src/*.h | grep -vE ':\s*(\*|/\*)' | grep -v 'src/pico8_value.h' \
  | grep -vE '^\S+:[0-9]+:\s+(const )?pico8_value_t \*?[a-z_]+;|^\S+:[0-9]+:\s+const pico8_lq_keys_t'  # fonctions : 13 à 0b8ea0d (16 à c77124e)
awk 'NR>=254 && NR<=273' src/pico8_map_tables.c | grep -c '{"'  # 19 entrées de nb, px chaîne
```
