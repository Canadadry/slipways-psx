---
title: "Plan 3, étape D.3 : les tables constantes parsées au démarrage (`es`, `ct`, `b_`, `co`, `fb`, `ek`, `ci`, `ef`, `df` avec sa passe `df_mr`, `ix`/`ha`, `bo`, `bz`) deviennent des tables `static const` typées, générées une fois depuis ce que `ob()` produit sur l'arbre de départ puis versionnées dans `pico8_game_data.c/.h` ; `es`, `ct`, `b_`, `co`, `fb`, `ek`, `ci`, `ef` lus typés par leurs consommateurs, `df`, `ix`, `ha`, `bo`, `bz` remis aux leurs sous forme de vue ob construite par `data_init.c` depuis les tables typées ; le test ponctuel `test_pico8_map_tables` est étendu — un seul commit, aucun pixel changé"
description: "Au démarrage, `pico8_game_data_init()`, `pico8_data_init()`, `pico8_entity_init()`, `pico8_menu_init()`, `pico8_help_init()` et `pico8_score_panel_init()` parsent avec `ob()` treize chaînes littérales du cart en tables allouées. Ce sont des constantes du cart : le plan 3 veut qu'elles deviennent des données `static const` versionnées, pour qu'à D.8 `pico8_ob.c` puisse disparaître. D.3 génère ces treize tables une fois, fait lire les huit tables à feuilles sous forme typée, garde une vue ob pour les cinq tables dont les rangées vont à un constructeur ou à une entité (`df` → `jo.nk`, `ix`/`ha`/`bo` → boutons `js`, `bz` → `mh`), et prouve par le test ponctuel que chaque table générée est égale, entrée par entrée, à ce que `pico8_ob()` produit des chaînes de l'arbre de départ."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Phase D — Données », liste écrite après D.1) :

> - D.3 — `ob`, tables constantes parsées au démarrage (liste de la première puce) : `g_es`, `g_ct`, `g_b_`, `g_co`, `g_fb` (`game_data.c`), `g_ek`, `g_df` avec sa passe `df_mr` (`data_init.c`), `g_ci` (`entity.c`), `g_ix`/`g_ha` (`menu.c`), `g_bo` (`help.c`), `g_ef`/`g_bz` (`score_panel.c`) → `static const` générés, test ponctuel étendu. Empreinte 21 (17, 4). Compte : `ob` ≤ 2105. §6 : aucun. `class-coverage-net` : seulement si le diff touche un constructeur.

En-tête de la liste de D et règles communes, recopiés :

> Liste de D, écrite après D.1 (HEAD 3816427, agent `guideline`), à partir de `make data-residue` et `sh tools/data_residue.sh --detail`. Comptes à l'écriture (lignes/fichiers) : `valeur` 5989/132, `ob` 2245/128, `stash` 56/14, `type-tag` 445/71, `ref-audit` 70/17, `par-nom` 1081/80, `dispatch` 197/59, `overrides` 449/62, `spec` 939/68, **9678 lignes distinctes dans 143 fichiers** ; ceux de la note de D.0 sont d'avant D.1. Chaque ligne nomme la catégorie qu'elle pousse et son « Compte » après elle : « = » attendu exact, « ≤ » majorant par grep (lignes de la catégorie qui nomment les sites des groupes déjà passés, cumulées dans l'ordre de la liste). Le test ponctuel (deuxième puce de D) est hors compte jusqu'à D.8, où il est supprimé ; il ne nomme aucun motif de `spec` ni `overrides`. Une catégorie tombée à 0 entre dans `DATA_RESIDUE_GUARD` dans le même commit.
>
> Empreinte (mesurée à l'écriture, chaque PRD la remesure) : fichiers de `src/*.[ch]` et `tests/*.[ch]` qui portent une ligne de la catégorie du groupe, ou qui nomment ses tables ou membres (majorant : noms courts partagés, `nu`, `k_`, `lq`). Règles communes : un nom nouveau ne correspond à aucun motif figé de la catégorie qu'il remplace (pas de `g_X_spec`/`g_X_defaults`/`pico8_X_default`, et le paramètre `overrides` typé est renommé) ; un lecteur par nom qui survit reste compté par `par-nom` ; un défaut muté (§5, 1a : `pj.ou`, `pj.ea`, `pj.cr`, `ga.by`, `nf.l`, `nf.r`, `jo.kw`) devient un tableau fixe statique non `const`. Un TU construit son état et ne rejoue jamais de fichier d'événements ; un e2e nouveau ajoute de la couverture de lignes en moins de 180 frames ; aucun `*.sha256` ni fichier de `tests/e2e/` ne change. `make sanitize-net` : à chaque étape qui touche `src/`, puisque toutes font passer des tables ob à des données statiques ou locales (§7, « En D et en 2 »).

Note de l'étape précédente, recopiée (fin de la ligne D.2) :

> Découvert à D.2 : `pico8_map_tables.c/.h` (données typées générées une fois, sans dépendance ob ; `pico8_r_lookup`, `pico8_iz_lookup`, `pico8_hc_lookup`, `pico8_jb_row_t`). Trois ponts, des vues ob construites par `data_init.c` depuis les tables typées : `pico8_fd()` (fermé par D.6c, entrée de `hp`), `pico8_dr()` (D.6b, entrée de `jb`), `pico8_nb()` (D.4b `js._cr_t`, puis D.7a `menu`/`tech_gate`). `pico8_hp_po` rend `const pico8_hc_entry_t *`. `pico8_data_init(mem)` ne lit plus `mem` (retrait à D.7a). Test ponctuel : `tests/test_pico8_map_tables.c`, 2 lignes `stash` jusqu'à D.8. `ob` après D.2 : 2227 (au lieu de ≤ 2195) ; les majorants `ob` suivants se décalent de 32.

Le majorant de D.3 devient donc **`ob` ≤ 2137** (2105 + 32).

Puces de la phase D, recopiées :

> - Les tables constantes parsées au démarrage (`g_es`, `g_ct`, `g_b_`, `g_co`, `g_fb`, `g_ek`, `ci`, menus `g_ix`/`g_ha`, aide `g_bo`, score `g_ef`/`g_bz`, `g_df`) et celles décodées par `unstash` (`g_hc`, `g_fd`, `g_dr`, `g_nb`, `g_r`, `g_iz`) deviennent des tableaux/structs `static const`, **générés une fois** à partir de ce que `ob()`/`unstash` produisent aujourd'hui, puis versionnés.
> - Test ponctuel, supprimé dans le commit qui supprime `pico8_ob.c`/`pico8_stash.c` : chaque table générée est égale, entrée par entrée, à ce que `ob()`/`unstash` produit. Le filet ne voit pas les entrées jamais affichées (aide, score, menus).
> - Les tables ad hoc construites en cours de partie (~68 `ob_table_new()`) deviennent des structs locales ou des tableaux fixes.
> - **Sortie** : `pico8_ob.c`, `pico8_value.c`, `pico8_type_tag.h`, `pico8_ref_audit.h`, `pico8_stash.c` supprimés ; plus aucun `malloc` pour des données. Les entités gardent le leur : c'est Plan 2 §8.

Les lignes voisines, pour la frontière :

> - D.4a — `ob`, membres table de l'arbre `jo` : `jo.nu`/`ka`/`dz`/`cq`/`kw`/`em`/`nk`, `qh.px`/`ba`, `hj.k_`, `ga.k_`, `nr.hz`, `mx.ht` → struct ou tableau fixe par membre (troisième puce) […]. Compte : `ob` ≤ 2033. […]
> - D.6b — `overrides`, mise en page et entités d'écran : `lq`/`jb` (`layout`), `js`/`fq` (`ui_button`), `mh`/`iv` (`msgbox`), `ui`, `qb` (`scroll`), `nf` (`mouse_reader`), `bg`, `mz` (`wormhole`), `jc` (`slipway`), appelants de `menu.c`, même règle. […]
> - D.7a — `ob`, tables ad hoc de l'affichage, de l'économie, de la recherche et des données (troisième puce) : tout `ob` restant de `planet_display`, `research_display`, `score_panel_display` (`g_de`, `spec1`/`spec2`), […] `menu`, […] `galaxy`, `cart`, `data_init`, `game_data`, `mouse` → structs locales ou tableaux fixes […].

Règles du §7, recopiées :

> - **Filets depuis la fusion de master (e2e rapide)** : `FRAMES_EVENTS` n'existe plus ; `make e2e` rejoue les 26 e2e de `tests/e2e/` depuis l'écran titre, `make sanitize-net` et `make class-coverage-net` rejouent ce même jeu, `tools/class_coverage_net_expected.txt` y a été reposé à la fusion ; aucun TU ne rejoue de fichier d'événements : un TU construit son état directement, un rejeu est un e2e de `tests/e2e/` (moins de 180 frames, et il doit ajouter de la couverture de lignes). Les mentions de `FRAMES_EVENTS`, `frames-check` et `float-audit` dans les étapes livrées sont historiques.
> - **En D et en 2** (ajouté à la revue de sortie de 1b) : `make class-coverage-net` aux étapes dont le diff touche un constructeur, un appel `pico8_class_coverage_record` ou `KNOWN[]`, et à la sortie de phase ; `make sanitize-net` aux étapes qui changent la durée de vie ou le propriétaire d'une donnée (table ob → locale ou statique, `malloc`/`free` retiré), et à la sortie de phase.

Lecture de ces règles pour D.3 :

- **`make sanitize-net` est exigé** : D.3 change la durée de vie et le propriétaire de treize tables (tas → données statiques), de leurs chaînes (sous-chaînes du tas → littéraux) et des vecteurs `v(...)` de `ix`, `ha` et `bz` (alloués par le constructeur `v` d'`ob()` → alloués par les constructeurs de vue).
- **`make class-coverage-net` n'est pas exigé** : le diff ne touche aucun constructeur (`pico8_*_new`, `pico8_*_struct_new`), aucun appel `pico8_class_coverage_record` ni `KNOWN[]`. Les fonctions touchées sont des `*_init`/`*_shutdown` de module, des méthodes et des lecteurs (`da`, `db`, `ky`, `bf`, `hy:kt`, `cr`, `dc`, `eg`, `pj:hb`). Les vues sont remises aux constructeurs `js` et `mh` **inchangées** (égalité prouvée par le test ponctuel). Si l'implémentation touche malgré tout un constructeur, la cible devient exigée (critère d'échec).

### État de départ

HEAD `c631538` (branche `claude/plan-pilot-3-7zdft0`), arbre propre. D.2 est livrée (`c2cc8a7`) et gradée (`c631538`). Décisions de l'utilisateur qui engagent D.3 : **un TU ne rejoue jamais un `events.json`** (il construit son état), **rien de lent**, **aucun `*.sha256` ni fichier de `tests/e2e/` touché**, **un e2e nouveau ajoute de la couverture de lignes en moins de 180 frames**.

**Rien n'a été compilé ni lancé pour ce PRD.** Seuls `make -s data-residue` et `sh tools/data_residue.sh --detail` ont tourné. Le contenu des tables vient des littéraux de l'arbre de départ (identiques à ceux de `pico8/lua.lua`, l. 187, 797, 965-990, 1171, 1189, 1311, 1338, 1364, 1377-1392, 1445 et la ligne `ct` de `game_data.c`), et le reste de `grep`. Tout fait d'exécution est une hypothèse (H1 à H16).

### Inventaire (mesuré sur `c631538`)

**`make data-residue`** :

```
valeur	6012	133
ob	2264	129
stash	28	4
type-tag	445	71
ref-audit	70	17
par-nom	1081	80
dispatch	197	59
overrides	449	62
spec	939	68
résidu D : 9691 ligne(s), 144 fichier(s)
```

`ob` 2264 = **2227 hors test ponctuel** + 37 lignes de `tests/test_pico8_map_tables.c`. C'est le chiffre de la note de D.2.

**`ob` par fichier du groupe** (`--detail`) : `game_data.c` 20, `game_data.h` 6 (ces deux fichiers ne portent **que** de l'`ob`), `data_init.c` 64, `data_init.h` 6, `entity.c` 7, `entity.h` 2, `menu.c` 71, `menu.h` 3, `help.c` 19, `score_panel.c` 34, `score_panel.h` 10, `economy_score.c` 27, `economy.c` 24, `cart.c` 12.

**Les treize tables** (source, entrées, forme de ce qu'`ob()` produit) :

| Table | Chaîne (départ) | Entrées | Forme |
|---|---|---|---|
| `es` | `game_data.c` | 4 | positionnelles, entiers `1,0,1,2` |
| `ct` | `game_data.c` | 4 | positionnelles `-0.333,0,0.25,0.5` : **non entières** |
| `b_` | `game_data.c` | 4 | positionnelles, entiers `0,100,200,400` |
| `co` | `game_data.c` | 4 | positionnelles, entiers `-5,0,1,1` |
| `fb` | `game_data.c` | 14 | positionnelles, entiers |
| `ek` | `data_init.c` | 4 | positionnelles, entiers `2,4,7,12` |
| `ci` | `entity.c` | 15 | positionnelles `0.5,1,…,0` : **la première non entière** |
| `ef` | `score_panel.c` | 15 | positionnelles, chaînes `"JAN"`…`"TER"` |
| `df` | `data_init.c` | 11 (+ 11 après la passe) | enregistrements `o(px, s, r[, d])` : `px`, `d` chaînes, `s` entier, `r` **non entier** sur `e` (4.5) et `r` (2.5) ; le premier (`"ll"`) n'a que 3 positionnelles et une nommée `me="ll"`. La passe `df_mr` = `"px","s","r","d"` (sans `gp`) ajoute à chaque enregistrement les nommées `px`, `s`, `r`, `d` (`d` absent sur `"ll"`), et à `df` 11 nommées `px` → le même enregistrement |
| `ix` | `menu.c` | 4 | rangées nommées seulement : `ra=v(x,y)` (**vecteur construit par `v`**), puis `lq=o(o(n,c))` |
| `ha` | `menu.c` | 4 + 3 | 4 rangées `o(lq=o(n,"texte"))` (la 4e avec `fg=5`), puis les nommées `hi=v(0,107)`, `gq=v(0,-11)` (**vecteurs**), `ca=14` |
| `bo` | `help.c` | 4 | rangées `o(lq=o(n,"texte"), px="…", trade=n, en=…)`, `en` **non entier** (`-0.99`, `-0.6`, `-0.4`, `-0.2`) |
| `bz` | `score_panel.c` | 12 | rangées de mise en page : positionnelles (entiers, chaînes, rangées imbriquées `o(27,5)`, **`f` = faux**, y compris dans `o(f,0)`), puis dans cet ordre les nommées présentes `fn` (chaîne), `score` (entier), `ra=v(x,y)` (**vecteur**), `c` (entier) |

**Consommateurs** (hors module propriétaire) :

| Table | Lecteur | Ce qu'il en fait |
|---|---|---|
| `es` | `economy_score.c` (`pico8_da`) | `es[flr(mi)]`, absent → 0 |
| `ct` | `economy_score.c` (`pico8_db`, ×2) | `ct[flr(mi)]`, absent → 0 |
| `b_`, `co` | `economy_score.c` (`pico8_ky`, `pico8_bf`, via `sum_by_mi_ctx_t.table`) | `t[flr(mi)]` sommé sur les `jo` |
| `fb` | `score_panel.c` (`pico8_eg`) | parcourt les 14 valeurs dans l'ordre, `poke` |
| `ek` | `menu.c` (`pico8_cr`, ×2 : `ek[tl-1]`, `ek[tl]`), `economy.c` (`pico8_hy_kt`) | index, absent → 0 / pas de palier |
| `ci` | `entity.c` (`pico8_dc`) | `ci[o]`, `o` = 1..15, passé à `pico8_mv(mo, "md", …)` |
| `ef` | `score_panel.c` (`pico8_pj_hb`) | `ef[flr(fm.lo)]` → chaîne ou nil dans `ou[5]` |
| `df` | `planet.c` (`jo:hu` `df[hp.go]`, `jo:hh` `df.ll`), `galaxy.c` (`bw` : positionnelles → `gn`), `tests/test_pico8_ui.c:1661`, `tests/test_pico8_hp.c:521` | **l'enregistrement est gardé tel quel dans `jo.nk`** (membre table, D.4a) ; les TU comparent des pointeurs d'enregistrement |
| `ix` | `menu.c` (`menu_init` écrit `my`/`hb` dans les rangées), `cart.c:196-208` (`nv(ix, js)`), `tests/test_pico8_ui.c`, `tests/test_pico8_pj.c:842` | **rangées mutées et remises au constructeur `js`** |
| `ha` | `menu.c` (`ix[4].my` écrit `my` dans les rangées, `pz(ha,ha)` ; `g_ha_pz` emprunte `hi`/`gq`/`ca`), `tests/test_pico8_js.c:1755`, `:1806` (même pointeur `hi`/`gq` dans `g_ha_pz` et dans `ha`) | **rangées mutées, remises à `pz` → `js`** |
| `bo` | `help.c` (`fv:pq` écrit `my`, puis `pz(self, bo)`) ; `pico8_bo()` n'existe plus, la table reste pour `fv_pq`/`pz` | **rangées mutées, remises à `pz` → `js`** |
| `bz` | `score_panel_display.c:303` (`pj:ph` : `pj_ph_map` **réécrit `st[3]` dans chaque rangée**, puis la liste va à `pico8_mh_new`) | **rangées mutées en place d'un appel à l'autre, remises au constructeur `mh`** |

**Autres faits qui engagent la forme :**

- **`ob()` masque chaque nombre par `0xffff.fffe`** (`PICO8_OB_NUMBER_MASK`, comme le `band(…,0xffff.fffe)` du cart, `lua.lua:66`). Un non-entier parsé n'est donc ni le relevé de `tools/probe_out.txt` ni un simple `-PICO8_FIXED_LIT` : `-0.333` donne `-(333·65536/1000)` = `0xffffaac1`, masqué en `0xffffaac0` = `-0x5540` (H13).
- `PICO8_FIXED_LIT` n'admet que la forme stricte `PICO8_FIXED_LIT(0x<8 hexa>, <littéral positif>)` et `make constants-check` compare le littéral au relevé : un négatif est hors forme, et un masqué n'a pas de relevé.
- `DATA_ENGINE_SRCS` (lié par `test_pico8_map_tables`) contient `data_init.c`, `game_data.c`, `map_tables.c`, `pi.c`, `vec.c`, `collect.c`, mais **ni `menu.c`, ni `help.c`, ni `score_panel.c`, ni `cart.c`** : le test ne peut pas appeler `pico8_ix()`, `pico8_ha()` ou `pico8_bz()`, et `bo` n'a plus d'accesseur.
- `pico8_ob_table_free` ne libère que la table de tête et ses noms de clé : tables imbriquées, chaînes et vecteurs ne sont jamais libérés aujourd'hui.
- `pico8_ci()` et `pico8_ef()` n'ont **aucun lecteur** (seulement leur définition et leur déclaration). `pico8_es()`…`pico8_fb()` et `pico8_ek()` n'ont que les lecteurs ci-dessus. `pico8_game_data_init/shutdown` ne sont appelés que par `cart.c:64`, `:121`.
- `game_data.h` est inclus par `game_data.c`, `score_panel.c`, `cart.c`, `economy_score.c`, `economy_formulas.c` (ce dernier pour `pico8_bn`/`pico8_bq`).
- Le type `pico8_jb_item_t` de D.2 (chaîne / nombre / rangée) n'a pas de faux ; `pico8_map_tables.c` l'initialise par désignateurs (`{.s = …}`, `{.n = …}`).

## Solution

**Un seul commit**, sans aucun pixel changé :

1. **`pico8_game_data.c/.h` devient le module de données des tables constantes du cart.** Le `.c` contient les treize tables en `static const` / `const`, émises **une fois** par un générateur jetable qui lit ce qu'`ob()` produit sur l'arbre de départ, puis versionnées et éditées comme n'importe quel source. Les types sont écrits à la main dans le `.h`, avec deux aides d'indexation 1-based. Le module n'inclut plus `pico8_ob.h` : `g_es`…`g_fb`, `pico8_es()`…`pico8_fb()` et `pico8_game_data_init/shutdown` disparaissent, `pico8_bn`/`pico8_bq` restent. Les deux fichiers sortent du compteur.
2. **`es`, `ct`, `b_`, `co`, `fb`, `ek`, `ci`, `ef` sont lus typés** par leurs consommateurs : `g_ek`/`pico8_ek()`, `g_ci`/`pico8_ci()`, `g_ef`/`pico8_ef()` disparaissent.
3. **`df`, `ix`, `ha`, `bo`, `bz` gardent une vue ob** (`pico8_df()`, `pico8_ix()`, `pico8_ha()`, `g_bo`, `pico8_bz()`), construite par `data_init.c` depuis les tables typées au lieu d'`ob()`. Ces cinq **vues** sont des ponts : leurs rangées vont à un constructeur (`js`, `mh`) ou à une entité (`jo.nk`), et sont mutées (`my`, `hb`, `st[3]`), comme dans le cart. La passe `df_mr` disparaît : ses résultats sont dans la table générée.
4. **Le test ponctuel `tests/test_pico8_map_tables.c` est étendu** : chaque table générée est comparée, entrée par entrée, à `pico8_ob(<chaîne de l'arbre de départ>, NULL)` (après `pico8_pi_init()`, pour que `v(...)` construise ses vecteurs), la passe `df_mr` étant refaite par `pico8_ho_apply`.
5. Le glossaire et une note « Découvert à D.3 » du plan suivent.

**Sortie de l'étape**, constatée par commande :

- `make test` vert (dont `test_pico8_map_tables`) ;
- `make e2e` : 26 conformes ;
- `make sanitize-net` : 26 propres ;
- `make data-guard` : code 0 ;
- `make data-residue` : `stash` 28/4, `type-tag` 445, `ref-audit` 70, `par-nom` 1081, `dispatch` 197, `overrides` 449, `spec` 939 ; `ob` hors test < 2227 ; 142 fichiers (voir « Effet sur `make data-residue` ») ;
- `grep -rnE '\bpico8_(es|ct|b_|co|fb|ek|ci|ef)\(|pico8_game_data_(init|shutdown)|\bg_(es|ct|b_|co|fb|ek|ci|ef)\b|df_mr' src tests --include='*.[ch]'` ne rend rien ;
- `grep -nE 'pico8_ob\(' src/pico8_menu.c src/pico8_help.c src/pico8_score_panel.c src/pico8_entity.c src/pico8_data_init.c` ne rend que les deux parses de spécification (`g_fv_spec`, `g_pj_spec`) de D.5b.

## User Stories

1. En tant que mainteneur du port, je veux que les treize tables constantes parsées au démarrage soient des données C versionnées, afin de les lire dans un diff sans exécuter `ob()`.
2. En tant que mainteneur, je veux que ces tables soient `const` et en stockage statique, afin qu'aucun code ne puisse les muter ni les libérer par erreur.
3. En tant que mainteneur, je veux qu'elles vivent toutes dans `pico8_game_data.c/.h`, qui ne dépend plus d'`ob`, afin qu'un seul fichier réponde à « où sont les constantes du cart hors map ».
4. En tant que lecteur de `pico8_da`, `pico8_db`, `pico8_ky` et `pico8_bf`, je veux une indexation 1-based qui rend « présent + valeur » ou « absent », afin que `es[mi]`, `ct[mi]`, `b_[mi]` et `co[mi]` gardent le sens Lua d'un index hors de la séquence.
5. En tant que lecteur de `pico8_eg`, je veux parcourir `fb` comme un tableau de 14 entiers dans l'ordre du cart, afin de voir l'ordre des `poke`.
6. En tant que lecteur de `pico8_cr` et de `hy:kt`, je veux lire `ek[tl-1]` et `ek[tl]` par la même aide, afin que `tl = 1` et `tl = 5` restent « absent ».
7. En tant que lecteur de `pico8_dc`, je veux que `ci[o]` soit un fixe typé, afin que seul l'appel `mo:md` reste en `pico8_value_t` (dispatch, hors D.3).
8. En tant que lecteur de `pj:hb`, je veux que `ef[lo]` rende une chaîne ou rien, afin de ne plus traverser une table ob pour 15 abréviations.
9. En tant que mainteneur, je veux que les valeurs non entières (`ct`, `ci`, `df.r`, `bo.en`) soient écrites telles qu'`ob()` les produisait, masque compris, afin qu'aucun bit ne change dans aucun calcul.
10. En tant que gardien de `make constants-check`, je veux qu'aucun `PICO8_FIXED_LIT` ne soit ajouté pour ces valeurs, afin que le relevé de `tools/probe_out.txt` ne juge pas des valeurs qui ne viennent pas de lui.
11. En tant qu'auteur de D.4a, je veux que `df` existe déjà sous forme typée, afin que D.4a n'ait qu'à typer `jo.nk` et retirer la vue.
12. En tant qu'auteur de D.6b, je veux que `ix`, `ha`, `bo` et `bz` existent déjà sous forme typée, afin que D.6b n'ait qu'à brancher les clés de construction de `js` et `mh` sur ces types et retirer les vues.
13. En tant que lecteur de `data_init.c`, je veux que chaque vue soit construite par une fonction nommée qui lit la table typée, afin de savoir quelle étape la retirera.
14. En tant que gardien du comportement, je veux que chaque vue soit égale, entrée par entrée et dans l'ordre, à ce que `ob()` (et la passe `df_mr`) produisait, afin que `jo`, `js`, `mh`, `menu`, `help` et `galaxy` voient exactement les mêmes tables.
15. En tant que gardien du comportement, je veux que chaque enregistrement de `df` reste une table partagée entre sa clé positionnelle et sa clé nommée `px`, afin que `jo.nk` pointe le même enregistrement par `df[go]`, `df.ll` et le tirage de `bw`.
16. En tant que gardien du comportement, je veux que les vecteurs `ra`, `hi`, `gq` des vues soient des vecteurs du tas neufs à chaque construction, comme ceux que `v(...)` allouait, afin que `g_ha_pz` emprunte encore les `hi`/`gq` de `ha` (même pointeur) et qu'un bouton qui garde son `ra` ne partage rien de `const`.
17. En tant que gardien du comportement, je veux que les vues de `ix`, `ha`, `bo`, `bz` soient construites une fois par `*_init`, au même moment qu'aujourd'hui, afin que les mutations (`my`, `hb`, `st[3]`) persistent d'un appel à l'autre comme dans le cart.
18. En tant que vérificateur, je veux un test qui compare chaque table générée à `pico8_ob()` sur les chaînes exactes de l'arbre de départ, afin de prouver que la génération n'a perdu ni une entrée, ni un ordre, ni un genre, ni un faux, ni un bit d'un non-entier.
19. En tant que vérificateur, je veux que ce test couvre aussi ce que le filet ne voit pas (aide, score, menus, entrées jamais affichées), afin de ne pas dépendre de la couverture des e2e.
20. En tant que vérificateur, je veux que ce test appelle directement les constructeurs de vue, afin qu'il ne dépende ni de `menu.c`, ni de `help.c`, ni de `score_panel.c`, absents de son lien.
21. En tant que vérificateur, je veux voir le test rougir sur un faux changé en 0 dans `bz`, sur un non-entier démasqué dans `ct`, sur un index 0 accepté et sur un champ omis d'une vue, afin de savoir qu'il mord.
22. En tant que vérificateur, je veux que le test ne rejoue aucun fichier d'événements et ne nomme ni `spec`, ni `overrides`, ni `type-tag`, afin de respecter la règle des TU et celle de la liste.
23. En tant que gardien du §6, je veux qu'aucun des trois tests de régression (`gh_row_lq`, `ga_export_panel_by`, `layout_nil_item_draws_nothing`) ne change, afin que D.3 ne touche à aucune preuve de bug.
24. En tant que gardien des TU, je veux que `test_pico8_js`, `test_pico8_ui`, `test_pico8_pj` et `test_pico8_hp` passent sans modification, afin de prouver que les vues rendent les mêmes rangées, les mêmes pointeurs et les mêmes index.
25. En tant que mainteneur, je veux que le générateur soit jetable et que le test ponctuel soit la preuve durable, afin de ne pas maintenir un outil qui lie `pico8_ob.c`, lequel disparaît à D.8.
26. En tant qu'auteur de D.4a et de D.5a, je veux trouver dans le message de commit comment le générateur a été écrit, afin de le réutiliser pour les défauts table et les spécifications.
27. En tant que gardien du compteur, je veux qu'aucun nom nouveau ne corresponde à un motif figé (`pico8_ob*`, `*_default*`, `*_field*`, `*_take_*(`, `g_X_spec`), afin que les comptes mesurent la conversion et non un renommage.
28. En tant que gardien de `make test`, je veux que `constants-check`, `float-grep` et `float-guard` restent verts, afin que `game_data.c`, qui est dans le cœur, ne porte ni flottant ni littéral fixe hors relevé.
29. En tant que développeur, je veux que les chaînes générées soient à l'abri des trigraphes, afin que `"how to play?"` et toute suite de `?` compilent sous `-Werror`.
30. En tant que lecteur de `docs/glossaire-noms.md`, je veux que les lignes `es`, `ct`, `b_`, `co`, `fb`, `ek`, `ci`, `ef`, `df`, `ix`, `ha`, `bo`, `bz` nomment les noms C d'après D.3, afin de retrouver une table depuis son nom de cart.
31. En tant que `plan-pilot`, je veux une note « Découvert à D.3 » qui donne les cinq ponts et l'étape qui ferme chacun, et le compte `ob` réel qui décale les majorants suivants, afin que les étapes suivantes partent de faits.
32. En tant que gardien des filets, je veux `make e2e` à 26 conformes et `make sanitize-net` à 26 propres, afin de prouver qu'aucun pixel et aucun accès mémoire n'a changé.
33. En tant que `plan-pilot`, je veux que toute conversion des clés de construction (`js`, `mh`), de `jo.nk`, de `pz`, de `pj:ph` ou des spécifications (`g_fv_spec`, `g_pj_spec`) reste hors de ce commit, afin que chaque étape reste une étape.

## Implementation Decisions

Pas d'humain dans la boucle. Le skill `to-prd` demande de faire valider par l'utilisateur les modules, les tests et le découpage. Ces choix sont tranchés ici comme le ferait un agent `guideline` (hypothèses explicites, simplicité, changement chirurgical, critères vérifiables), et marqués **(G)**. Les rejets sont notés à chaque décision.

### Règle de forme, table par table (G) — celle de D.2

**Une table lue feuille par feuille devient typée jusque chez son lecteur. Une table remise à un constructeur, ou dont une entité garde ou mute un morceau, est typée dans les données, mais son lecteur reçoit une vue ob construite depuis ces données, jusqu'à l'étape qui convertit ce lecteur.**

| Table | Lecteurs | Forme livrée par D.3 | Pont retiré par |
|---|---|---|---|
| `es`, `ct`, `b_`, `co` | `economy_score.c` (5 sites) | typée jusqu'au lecteur | — |
| `fb` | `score_panel.c` (`eg`) | typée jusqu'au lecteur | — |
| `ek` | `menu.c` (2), `economy.c` (1) | typée jusqu'au lecteur | — |
| `ci` | `entity.c` (`dc`) | typée jusqu'au lecteur (la valeur part ensuite en argument de dispatch, inchangé) | — |
| `ef` | `score_panel.c` (`pj:hb`) | typée jusqu'au lecteur | — |
| `df` | `planet.c` (`jo:hu`, `jo:hh`), `galaxy.c` (`bw`) : l'enregistrement devient `jo.nk` | typée + vue ob `pico8_df()` | **D.4a** (`jo.nk`) ; si `bw` garde une lecture ob de `df` après D.4a, D.7a (`galaxy`) |
| `ix` | `menu.c` (écrit `my`/`hb`), `cart.c` (`nv(ix, js)`) | typée + vue ob `pico8_ix()` | **D.6b** (clés de construction de `js`, appelants de `menu.c`) ; `cart.c` à D.7a s'il reste une lecture ob |
| `ha` | `menu.c` (écrit `my`, `pz(ha,ha)`, `g_ha_pz`) | typée + vue ob `pico8_ha()` | **D.6b** (`pz` → `js`, appelants de `menu.c`) |
| `bo` | `help.c` (écrit `my`, `pz(self, bo)`) | typée + vue ob `g_bo` | **D.6b** (`pz` → `js`) |
| `bz` | `score_panel_display.c` (`pj_ph_map` réécrit `st[3]`, puis `pico8_mh_new`) | typée + vue ob `pico8_bz()` | **D.6b** (entrée de `mh`) puis **D.7a** (`score_panel_display`) |

Rejets :

- **Typer aussi les lecteurs de `df`, `ix`, `ha`, `bo`, `bz` dans D.3.** Il faudrait typer `jo.nk` (D.4a), les clés de construction de `js` et de `mh` et la liste `bs` de `pz` (D.6b), la réécriture `st[3]` de `pj:ph` (D.7a). Trois étapes de la liste seraient absorbées et l'empreinte doublerait.
- **Vue ob pour les treize tables**, aucun lecteur ne changeant. Les huit tables à feuilles ont des lecteurs d'une ligne ; les garder en ob laisserait les sites `ob` de `economy_score.c`, `economy.c`, `menu.c`, `entity.c`, `score_panel.c` intacts : un renommage, pas une conversion.
- **Générer les tables sous forme ob statique** (`pico8_ob_entry_t` littéraux). Des centaines de lignes `ob`/`valeur`, à reconvertir avant D.8 (rejet de D.2).

### Où vivent les données (G) : `pico8_game_data.c/.h`

- Le module existe, porte déjà `es`…`fb` et les scalaires `bn`/`bq`, et ne sert qu'à cela. Il devient **pur** : il n'inclut ni `pico8_ob.h`, ni `pico8_value.h`, ni `pico8_stash.h`. Il inclut `pico8_fixed.h` et `pico8_map_tables.h` (pour `pico8_jb_row_t`).
- `pico8_game_data_init/shutdown` disparaissent (plus rien à construire) ; `cart.c` perd leurs deux appels et son inclusion de `game_data.h`, s'il n'en lit plus rien.
- `game_data.c` et `game_data.h` sortent du compteur (ils ne portaient que de l'`ob`).
- Aucune liste de sources du `Makefile` ne change : `game_data.c` est déjà dans `ENGINE_SRCS`, `DATA_ENGINE_SRCS` et les deux lignes de `main_raylib`.
- Rejeté : **étendre `pico8_map_tables.c/.h`**. Son nom dit « tables planquées dans la map » (`unstash`) : ces treize-là sont des littéraux du code.
- Rejeté : **garder chaque table `static const` dans son module propriétaire.** Le test ponctuel ne pourrait comparer ni `bo` (sans accesseur) ni `ix`/`ha`/`bz` (modules hors de son lien), et D.8 aurait six fichiers à fouiller au lieu d'un.

### Les types (G)

Forme retenue (les noms de champs suivent le cart, voir le glossaire) :

```c
enum {
    PICO8_ES_COUNT = 4, PICO8_CT_COUNT = 4, PICO8_B__COUNT = 4, PICO8_CO_COUNT = 4, PICO8_FB_COUNT = 14,
    PICO8_EK_COUNT = 4, PICO8_CI_COUNT = 15, PICO8_EF_COUNT = 15, PICO8_DF_COUNT = 11,
    PICO8_IX_COUNT = 4, PICO8_HA_COUNT = 4, PICO8_BO_COUNT = 4, PICO8_BZ_COUNT = 12
};

extern const int pico8_es_table[PICO8_ES_COUNT];          /* idem b_, co, fb, ek */
extern const pico8_fixed_t pico8_ct_table[PICO8_CT_COUNT]; /* idem ci : bits 16.16 tels qu'ob() les rendait */
extern const char *const pico8_ef_table[PICO8_EF_COUNT];

typedef struct { const char *px; int s; pico8_fixed_t r; const char *d; const char *me; } pico8_df_entry_t;
                                                  /* d NULL : 3 positionnelles ; me NULL si absent */
typedef struct { int ra_x, ra_y; const pico8_jb_row_t *lq; } pico8_ix_entry_t;          /* ra puis lq */
typedef struct { const pico8_jb_row_t *lq; int has_fg; int fg; } pico8_ha_entry_t;
typedef struct { int hi_x, hi_y, gq_x, gq_y, ca; } pico8_ha_tail_t;                     /* hi, gq, ca */
typedef struct { const pico8_jb_row_t *lq; const char *px; int trade; pico8_fixed_t en; } pico8_bo_entry_t;
typedef struct {
    pico8_jb_row_t row;          /* positionnelles ; son c est émis en dernier */
    const char *fn;              /* NULL si absent */
    int has_score, score;
    int has_ra, ra_x, ra_y;
} pico8_bz_entry_t;

int pico8_int_seq_at(const int *seq, int count, int i, int *out);
int pico8_fixed_seq_at(const pico8_fixed_t *seq, int count, int i, pico8_fixed_t *out);
```

- **Un champ numérique est `int` si toutes ses valeurs parsées sont entières, `pico8_fixed_t` sinon** (règle du générateur, par champ ou par table). C'est la règle de D.2, qui n'avait que des entiers, étendue : `ct`, `ci`, `df.r`, `bo.en` sont `pico8_fixed_t`. Le lecteur d'un `int` convertit par `pico8_fixed_from_int` là où il veut un fixe.
- **Un `pico8_fixed_t` généré s'écrit en hexadécimal signé des bits qu'`ob()` rendait** (par exemple `-0x5540` pour `-0.333`, `0x8000` pour `0.5`, `0x48000` pour `4.5`), masque compris. C'est une constante entière : aucune macro, aucun calcul.
  - Rejeté : **`PICO8_FIXED_LIT`**. Sa forme stricte refuse un négatif, et `constants-check` jugerait la valeur contre le relevé de PICO-8, alors qu'elle doit égaler ce qu'`ob()` produisait (le test ponctuel est juge).
  - Rejeté : **`pico8_fixed_from_int` + un littéral décimal**. Non constant, et un décimal réveillerait `float-grep`.
  - Rejeté : **une macro nouvelle de bits bruts**. Un second chemin vers le fixe pour ce que l'hexadécimal écrit déjà.
- **`pico8_jb_item_t` (D.2) gagne `int is_false`** : un élément est la chaîne `s` si non `NULL`, la rangée `row` si non `NULL`, le faux si `is_false`, le nombre `n` sinon. Les initialiseurs désignés de `pico8_map_tables.c` ne changent pas (le champ vaut 0). C'est le seul changement de D.2 : `bz` contient `f` et `o(f,0)`.
- **`pico8_jb_row_t` sert aux `lq` de `ix`, `ha`, `bo` et aux rangées de `bz`** : c'est le même contenu constant de mise en page (chaînes, nombres, rangées, faux, `c`). Il ne tranche toujours pas la représentation des `lq.items` d'exécution (arrêt n° 3 de la liste).
- **Tailles** en constantes d'énumération ; les tables de tête sont déclarées `extern const T pico8_X_table[PICO8_X_COUNT]` : un compte faux ne compile pas. Les rangées et éléments imbriqués sont `static const` nommés dans le `.c`, comme à D.2.
- **Ordre des nommées** : `df` émet `me` avant la passe, puis `px`, `s`, `r`, `d` ; `ix` émet `ra` puis `lq` ; `ha` émet `lq` puis `fg` ; `bo` émet `lq`, `px`, `trade`, `en` ; `bz` émet `fn`, `score`, `ra`, `c`. C'est l'ordre du cart, et le générateur échoue s'il en voit un autre (H2).
- **Aides d'indexation**, écrites à la main à la fin du `.c` : `pico8_int_seq_at`/`pico8_fixed_seq_at` rendent 1 et écrivent `*out` si `1 <= i <= count`, sinon 0 sans toucher `*out` : c'est `t[i]` de Lua sur une séquence. `seq` non `NULL` et `count >= 0` : assertion avec message.
  - Rejeté : **une recherche par table** (`pico8_es_at`…), sur le modèle de `pico8_r_lookup`. Huit fonctions identiques ; D.2 en avait besoin pour des clés chaînes, pas pour des index.
  - Rejeté : **l'indexation nue `t[i-1]` aux sites**. Chaque site referait la borne ; `es[mi]`, `ct[mi]`, `ek[tl-1]` et `ef[lo]` ont des index hors séquence réels.
- **Aucun nom nouveau ne correspond à un motif figé** : ni `pico8_ob*`, ni `*_default*`, ni `*_field*`, ni `*_take_*(`, ni `g_X_spec`. Les commentaires du module ne portent que des contrats (ce que rendent les aides, le sens des bits d'un fixe généré, `d`/`me`/`fn` `NULL`, `is_false`, l'ordre d'émission des nommées), selon les quatre sortes de `CLAUDE.md`. Pas de bandeau « généré, ne pas éditer ».

### Le générateur (G) : jetable, en C, sur l'arbre de départ

- **Un programme C de l'implémenteur, dans son scratchpad, jamais versionné**, compilé contre `$(DATA_ENGINE_SRCS)` de `c631538`, **avant** toute modification. Il fait :
  - `pico8_pi_init()` (pour que `v(...)` construise ses vecteurs) ;
  - `pico8_ob(s, NULL)` sur chacune des treize chaînes, **copiées octet pour octet des littéraux de l'arbre de départ** (`game_data.c`, `data_init.c`, `entity.c`, `menu.c`, `help.c`, `score_panel.c`) ;
  - pour `df`, la passe du cart : `pico8_ho_apply(df, pico8_ob("\"px\",\"s\",\"r\",\"d\",", NULL), NULL, rec)` sur chaque positionnelle relevée avant la passe ;
  - il parcourt ces tables et émet le C dans l'ordre des entrées ; un `REF` rangé sous `ra`, `hi` ou `gq` est lu comme un `pico8_vec_t`.
- **Il échoue fort** si une hypothèse de forme tombe (H2) : positionnelles non contiguës depuis 1 ; clé nommée hors de l'ensemble attendu de la table ; ordre des nommées différent de celui des types ; valeur non entière dans un champ `int` ; coordonnée de vecteur non entière ; `REF` hors `ra`/`hi`/`gq` qui n'est pas une table ; compte d'entrées différent de l'énumération.
- **Échappement** : `\\`, `\"`, `\n`, et **chaque `?` en `\?`** (trigraphes ; `"how to play?"`).
- Sa sortie est collée dans `pico8_game_data.c`, puis remise en forme à la main à la largeur des fichiers voisins (≤ 120 colonnes). Aucune valeur n'est éditée à la main.
- Le message de commit décrit le générateur en ≤ 15 lignes (entrées, parcours, assertions, échappement, écriture des fixes).
- Rejeté : **un générateur versionné** (`tools/` et une cible `make`). La puce dit « générés une fois » ; il lierait `pico8_ob.c` (rejet de D.2).
- Rejeté : **lire les tables par les accesseurs de départ** (`pico8_es()`, `pico8_ix()`…). `ix` et `ha` y portent déjà `my`/`hb` écrits par `menu_init`, `bo` n'a pas d'accesseur, et deux chemins pour treize tables compliquent un outil jetable. Les chaînes copiées sont vérifiées contre le départ (contrôle du vérificateur n° 5).

### Conversion des lecteurs typés

| Fichier | Site | Après D.3 |
|---|---|---|
| `economy_score.c` | `pico8_da` (`es[flr(mi)]`) | `pico8_int_seq_at(pico8_es_table, PICO8_ES_COUNT, flr(mi), &v)` → `pico8_fixed_from_int(v)` ; la garde `es != NULL` part |
| `economy_score.c` | `pico8_db` (`ct[flr(pt.mi)]`, `ct[flr(oz.mi)]`) | `pico8_fixed_seq_at(pico8_ct_table, …)` ; les gardes `ct != NULL` partent, les lectures `pico8_entity_get_field(…, "mi", …)` restent sur leur ligne |
| `economy_score.c` | `sum_by_mi_ctx_t`, `pico8_ky` (`b_`), `pico8_bf` (`co`) | le champ `table` devient `const int *seq` + `int count` ; `sum_by_mi_visit` lit par `pico8_int_seq_at` ; les gardes `table == NULL` partent |
| `economy.c` | `pico8_hy_kt` (`ek[flr(ew)]`) | `pico8_int_seq_at(pico8_ek_table, PICO8_EK_COUNT, …)`, comparaison inchangée |
| `menu.c` | `pico8_cr` (`ek[tl-1]`, `ek[tl]`) | idem, deux sites ; `qi` et `ek_tl` gardent leur défaut 0 |
| `entity.c` | `pico8_dc` (`ci[o]`) | `pico8_fixed_seq_at(pico8_ci_table, PICO8_CI_COUNT, o, &ci)` → `pico8_mv(g_mo, "md", 1, pico8_value_fixed(ci), …)` ; `g_ci`, son parse, son `free` et `pico8_ci()` partent |
| `entity.h` | `pico8_ci()` | déclaration retirée ; les contrats d'`init`/`shutdown` ne nomment plus `ci` |
| `score_panel.c` | `pico8_eg` (`fb`) | `for (i = 1; i <= PICO8_FB_COUNT; i++)` sur `pico8_fb_table[i - 1]` ; le retour `mem == NULL` reste, `fb == NULL` part |
| `score_panel.c` | `pico8_pj_hb` (`ef[flr(lo)]`) | `1 <= lo <= PICO8_EF_COUNT` → `pico8_value_string(pico8_ef_table[lo - 1])`, sinon `pico8_value_ref(NULL)` comme avant ; `g_ef`, son parse, son `free` et `pico8_ef()` partent |
| `score_panel.h` | `pico8_ef()`, contrat d'`init`/`shutdown` | déclaration retirée ; le contrat ne dit plus que `bz` est parsé ni qu'`ef` est construit |
| `cart.c` | `pico8_cart_load`, `pico8_cart_shutdown` | appels de `pico8_game_data_init/shutdown` retirés |

- **Aucune condition de jeu ne change de sens.** Les gardes « table `NULL` » tombent : les tables sont présentes dès le chargement du programme, comme dans le cart (H3). Un index hors séquence reste « absent » par les aides.
- Aucune ligne portant un lecteur par nom (`pico8_entity_get_field`, `*_take_*(`) n'est ajoutée, retirée ni coupée en deux : `par-nom` reste exact.

### Les vues (G) : des constructeurs publics dans `data_init.c`

- **Cinq constructeurs de vue, dans `data_init.c`**, chacun rend une table ob neuve construite avec `pico8_ob_table_new`/`set_index`/`set_name`, **dans l'ordre exact des entrées** de la table parsée ; l'appelant possède la table de tête, jamais les tables imbriquées, les chaînes ni les vecteurs :
  - `df` : statique (`data_init_df_view`, sur le modèle de `data_init_nb_view`). Chaque enregistrement : positions 1..3 (`px`, `s`, `r`) et 4 (`d`) si présent, puis `me` si présent, puis `px`, `s`, `r`, et `d` si présent. `df` reçoit les 11 enregistrements en positionnelles, puis 11 nommées `px` → **le même pointeur** d'enregistrement.
  - `ix`, `ha`, `bo`, `bz` : **publics**, déclarés dans `data_init.h` (par exemple `pico8_ix_view`, `pico8_ha_view`, `pico8_bo_view`, `pico8_bz_view`), appelés par `pico8_menu_init`, `pico8_help_init`, `pico8_score_panel_init` à la place de `pico8_ob(...)`, et par le test ponctuel.
  - `ix` : chaque rangée → `ra` (vecteur neuf), `lq` (vue de la rangée). `ha` : 4 rangées (`lq`, puis `fg` si présent), puis `hi`, `gq` (vecteurs neufs), `ca`. `bo` : `lq`, `px`, `trade`, `en`. `bz` : positionnelles de `row` (chaîne, fixe, faux ou vue de rangée), puis `fn`, `score`, `ra` (vecteur neuf), `c` présents.
  - La vue de rangée existante (`data_init_row_view`) émet `pico8_value_bool(0)` pour un élément `is_false`. Aucun changement pour `dr` et `nb`, qui n'en ont pas.
- **Un vecteur de vue** est alloué comme le faisait le constructeur `v` de `pico8_pi.c` (`malloc` + `pico8_vec_v(pico8_fixed_from_int(x), pico8_fixed_from_int(y))`), un par occurrence et par construction, jamais libéré (comme aujourd'hui).
- **Moment et propriété inchangés** : `menu_init`, `help_init`, `score_panel_init` construisent leur vue au même endroit qu'ils parsaient, et leur `*_shutdown` la libère par `pico8_ob_table_free` comme avant. `menu_init` écrit toujours `my`/`hb` dans les rangées de `ix` et lit toujours `hi`/`gq`/`ca` dans la vue `ha` pour `g_ha_pz` (même pointeur, H5). `fv:pq`, `ix[4].my` et `pj:ph` mutent toujours les rangées de la vue.
- `pico8_data_init` : plus de `pico8_ob` pour `df` ni pour `ek`, plus de `df_mr`. `pico8_data_init_rename_pass` et `pico8_data_init_positional_values` n'ont plus d'appelant et sont supprimées ; l'inclusion de `pico8_collect.h` part si plus rien ne l'utilise. `g_ek`, `pico8_ek()` et leur `free` partent.
- `data_init.h` : `pico8_ek()` retiré, les quatre constructeurs publics déclarés avec leur contrat de propriété. `pico8_df()`, `pico8_fd()`, `pico8_dr()`, `pico8_nb()` restent « NULL avant le premier `pico8_data_init()` ».
- `pico8_pi_init()` reste appelé par `menu_init` et `score_panel_init`, et `help_init` garde son assertion sur `v` : les spécifications `g_pj_spec`/`g_fv_spec` (D.5b) contiennent encore `v(...)`.
- `menu.h` : le contrat « An init frees the previous ix/ha and reparses both » devient « … and rebuilds both ». `score_panel.h` : le contrat d'`init` ne parle plus que de la spécification `pj` pour `v(...)`.
- Rejeté : **construire les vues dans `menu.c`, `help.c`, `score_panel.c`**. Le test ponctuel, lié sur `DATA_ENGINE_SRCS`, ne pourrait pas les appeler, et `bo` n'a pas d'accesseur. `data_init.c` construit déjà les vues de D.2, et D.7a le vide.
- Rejeté : **rendre un accesseur `pico8_bo()`**. Il a été supprimé sans lecteur ; le test appelle le constructeur de vue.
- Rejeté : **des vecteurs `static const` partagés par les vues**. Un bouton garde son `ra` et `g_ha_pz` emprunte `hi`/`gq` en non-`const` ; chaque parse en allouait de neufs.
- Rejeté : **envelopper les appels `pico8_ob_table_*` des vues dans des aides à nom neutre.** Ce serait cacher des lignes `ob` au compteur.

### `Makefile`, docs

- **Le `Makefile` ne change pas** : aucun fichier nouveau, `test_pico8_map_tables` est déjà dans `UNIT_TESTS` et lie `DATA_ENGINE_SRCS` (qui contient `data_init.c`, `game_data.c`, `pi.c`, `vec.c`, `collect.c`). `DATA_RESIDUE_GUARD` reste vide (aucune catégorie ne tombe à 0).
- `docs/glossaire-noms.md` : les lignes `es`, `ct`, `b_`, `co`, `fb`, `ek`, `ci`, `ef` nomment `pico8_X_table` ; `df`, `ix`, `ha`, `bo`, `bz` nomment la table typée **et** la vue (`pico8_df()`, `pico8_ix()`, `pico8_ha()`, `g_bo`, `pico8_bz()`) ; la ligne `pico8_jb_row_t`/`pico8_jb_item_t` ajoute le faux et les `lq` de `ix`, `ha`, `bo` et les rangées de `bz`.
- `docs/plan-3.md` : note « Découvert à D.3 » en fin de ligne D.3 (texte dans Further Notes).

### Ce qui ne change pas

- Aucun pixel, aucune instanciation, aucune condition de jeu.
- Aucun `*.sha256`, aucun fichier de `tests/e2e/`, aucun fichier d'événements.
- `pico8_ob.c/.h`, `pico8_stash.*`, `pico8_map_tables.c`, `pico8_collect.*`, `pico8_pi.*`, `pico8_msgbox.*`, `pico8_ui_button.*`, `pico8_score_panel_display.c`, `pico8_planet.c`, `pico8_galaxy.c`, `pico8_economy_formulas.c`, `pico8_help.h`.
- Les spécifications `g_fv_spec` (`help.c`) et `g_pj_spec` (`score_panel.c`) et leurs `*_default` (D.5b).
- Les TU qui lisent les vues : `test_pico8_js.c`, `test_pico8_ui.c`, `test_pico8_pj.c`, `test_pico8_hp.c`. Les trois tests du §6.
- `tools/data_residue.sh`, ses motifs, `DATA_RESIDUE_GUARD`, `tools/class_coverage_*`, `KNOWN[]`.
- **TROU** : aucun écart port/cart n'est touché ni ajouté.

### Empreinte remesurée

- **`src/` : 15 chemins modifiés**, aucun nouveau : `game_data.c/.h`, `map_tables.h` (`is_false`), `data_init.c/.h`, `economy_score.c`, `economy.c`, `menu.c/.h` (`.h` : contrat), `entity.c/.h`, `help.c`, `score_panel.c/.h`, `cart.c`.
- **`tests/` : 1 chemin** : `test_pico8_map_tables.c`.
- Plus `docs/glossaire-noms.md` et `docs/plan-3.md`, soit **18 chemins**.
- Écart avec la ligne du plan (21 : 17, 4) : les quatre TU lecteurs de `pico8_ix()`/`pico8_ha()`/`pico8_df()` ne changent pas (vues) ; `planet.c`, `galaxy.c`, `score_panel_display.c` ne changent pas (vues) ; `map_tables.h` et `cart.c` s'ajoutent.

### Effet sur `make data-residue`

Attendu après D.3 (H10) :

| Catégorie | Avant | Après | Pourquoi |
|---|---|---|---|
| `stash` | 28/4 | **28/4** (exact) | le test ponctuel garde ses 2 lignes |
| `ob` | 2227 hors test (2264) | **< 2227 hors test**, attendu entre 2190 et 2215 (H10) ; le test ponctuel en ajoute (≈ 40 à 70) | −≈ 70 lignes qui nomment les sites (`game_data.c` 20, `game_data.h` 6, `economy_score.c` ≈ 8, `economy.c` 2, `menu.c` ≈ 5, `entity.c` 5, `entity.h` 1, `help.c` 1, `score_panel.c` ≈ 9, `score_panel.h` 1, `data_init.c` ≈ 12, `data_init.h` 1), +≈ 35 à 55 lignes de vues (`data_init.c`) et 4 déclarations (`data_init.h`) |
| `type-tag` | 445 | **445** (exact) | aucun site touché n'en porte (l'`assert` de tag de `fv:pq` et celui de `pj_ph_map` restent) ; le test n'en nomme aucun |
| `ref-audit` | 70 | **70** (exact) | — |
| `par-nom` | 1081 | **1081** (exact) | aucune ligne de lecteur par nom ajoutée, retirée ni coupée ; aucun nom nouveau en `*_field*`/`*_take_*(` |
| `dispatch` | 197 | **197** (exact) | l'appel `pico8_mv(g_mo, "md", …)` de `dc` et les `pico8_value_function` de `menu.c` restent ; les vues n'en créent pas |
| `overrides` | 449 | **449** (exact) | aucune ligne touchée |
| `spec` | 939 | **939** (exact) | `g_fv_spec`, `g_pj_spec` et leurs lignes sont intacts ; le test ne nomme aucun motif de `spec` |
| `valeur` | 6012/133 | rapporté au commit | les vues et le test ajoutent des `pico8_value_*`, les lecteurs typés en retirent |
| Fichiers | 144 | **142** (H10) | `game_data.c` et `game_data.h` ne portent plus aucune ligne ; aucun fichier n'entre |

- **Le majorant `ob` ≤ 2137 (2105 + 32) n'est pas atteint (G).** Il compte les lignes qui nomment les sites, sans les ponts. Les vues de `df`, `ix`, `ha`, `bo`, `bz` en ajoutent ≈ 40 à 60, qui partent avec D.4a, D.6b et D.7a. L'implémenteur écrit le compte exact N dans le commit et dans la note ; les majorants `ob` suivants se décalent de N − 2105 par rapport à la liste (et non plus de 32). Ce n'est pas un arrêt du §7.
- Aucune catégorie ne tombe à 0 : **`DATA_RESIDUE_GUARD` reste vide.** Aucun motif ne change.

### Découpage (G) : un seul commit

- Le §7 veut un commit par groupe. La découpe « données + test d'abord, lecteurs et vues ensuite » a été évaluée et écartée : le premier commit ajouterait des données qu'aucun code ne lit, le test des vues ne pourrait naître qu'avec le second, et `sanitize-net` passerait deux fois pour une seule conversion.
- 18 chemins, sous le seuil de ~40. **Pas de section « Implementation Chunks »**, que `prd-autopilot` traiterait comme autant de commits. L'ordre de travail conseillé est dans Testing Decisions.

### Hypothèses (rien n'a été lancé)

- **H1** : `c631538` est vert : `make test`, `make e2e` (26 conformes), `make sanitize-net` (26 propres), `make data-guard` code 0, `make data-residue` égal à l'inventaire. Si c'est démenti au départ, l'implémenteur s'arrête et note : D.3 ne répare rien.
- **H2** : ce qu'`ob()` produit a la forme de l'inventaire : comptes (4, 4, 4, 4, 14, 4, 15, 15, 11 → 22 après la passe, 4, 4 + 3, 4, 12), entiers là où l'inventaire le dit, positionnelles avant nommées sauf `me` de `df` (nommée après les positionnelles, avant la passe), ordre des nommées de la section « Les types », vecteurs sous `ra`/`hi`/`gq` seulement, faux seulement dans `bz`. Le générateur le vérifie. Si c'est démenti, l'implémenteur adapte le type à ce qu'`ob()` produit (le test ponctuel reste juge) et note l'écart. Ce n'est un arrêt que si un champ ne tombe sous aucune règle.
- **H3** : aucun TU ni e2e ne dépend d'`es`, `ct`, `b_`, `co`, `fb` absents avant `pico8_game_data_init` ou après `pico8_game_data_shutdown`, d'`ek` avant `pico8_data_init`, de `ci` avant `pico8_entity_init`, ni d'`ef` avant `pico8_score_panel_init`. Ils sont maintenant présents dès le chargement. Les TU qui atteignent `da`, `db`, `ky`, `bf`, `eg`, `pj:hb`, `hy:kt`, `cr` ou `dc` (`test_pico8_js`, `fv`, `fr`, `mx`, `hy`, `mh`, `os`, `ga`, `pm`, `nr`, `iv`, `g_`, `pj`, `bg`, `hj`, `hp`, `nl`, `planet_kj_dispatch`) passent tous par `pico8_cart_load`, `pico8_bootstrap` ou un `start_game`, mais ce n'est pas prouvé ligne à ligne. Si un TU rougit pour cette raison, voir les critères d'échec.
- **H4** : aucun code ne libère une chaîne ni un vecteur lus dans ces tables ; les chaînes sont désormais des littéraux. `grep` ne trouve de `free(… .as.string)` que dans `test_pico8_ob.c` et `test_pico8_stash.c`, sur leurs tables à elles. Un `free` d'une chaîne statique aborterait sous ASan et dans `sanitize-net`.
- **H5** : les pointeurs partagés sont préservés : `df[i]` et `df[px]` pointent le même enregistrement ; `g_ha_pz.hi`/`gq` pointent les vecteurs de la vue `ha` (`same_as_ha`, `ha_pz_vec_as_ha` de `test_pico8_js`) ; `entry_of(pico8_df(), nk)` de `test_pico8_hp:521` rend le même index.
- **H6** : `make e2e` rend 26 conformes, sans nouvelle référence : les bits des fixes sont ceux d'`ob()`, l'ordre de `fb` est gardé, les vues sont égales aux tables parsées (test ponctuel), les chaînes ont le même contenu.
- **H7** : `-Werror` reste muet : les `?` sont échappés, plus aucune fonction statique n'est inutilisée (`pico8_data_init_rename_pass`, `pico8_data_init_positional_values` supprimées), et aucun fichier ne dépendait de l'inclusion transitive de `pico8_ob.h` par `game_data.h` (`score_panel.c`, `economy_score.c`, `economy_formulas.c`, `cart.c` l'ont par d'autres en-têtes). Si l'un en dépendait, l'inclusion est ajoutée dans ce fichier, pas dans `game_data.h`.
- **H8** : le diff ne touche aucun constructeur, aucun `pico8_class_coverage_record` ni `KNOWN[]` : `make class-coverage-net` n'est pas exigé. Les vues remises à `js`/`mh` sont égales aux tables parsées : aucune instanciation ne change.
- **H9** : `make sanitize-net` rend 26 propres : aucun accès hors bornes (aides bornées par `count`, boucles par `PICO8_X_COUNT`), aucun `free` de littéral.
- **H10** : `make data-residue` rend les comptes du tableau. `ob` est dans la fourchette, mais l'exact dépend de l'écriture des vues : un écart hors fourchette est expliqué au commit, sans être un échec tant qu'`ob` hors test reste < 2227.
- **H11** : `make constants-check`, `float-grep` et `float-guard` restent verts : `game_data.c` (cœur) ne porte que des entiers, de l'hexadécimal et des chaînes, sans `PICO8_FIXED_LIT` nouveau ni flottant ; `pico8_bq` garde le sien.
- **H12** : les durées sont courtes : `make test` en quelques secondes plus une recompilation large (`map_tables.h`, `data_init.h`, `entity.h`, `game_data.h`), `make e2e` ~1 s, `sanitize-net` < 2 min compilation comprise.
- **H13** : les fixes générés valent `ct[1]` = `-0x5540`, `ci[1]` = `0x8000`, `df` `e.r` = `0x48000`, `r.r` = `0x28000`, et pour `bo.en` les bits masqués d'`-0.99`, `-0.6`, `-0.4`, `-0.2` (attendus `-0xfd70`, `-0x999a`, `-0x6666`, `-0x3334`). Le générateur donne l'exact ; le test ponctuel est juge.
- **H14** : dans le test, `pico8_pi_init()` suffit pour que les chaînes de référence construisent leurs vecteurs, et aucun constructeur autre que `v` n'est appelé par ces treize chaînes (pas de `s(...)`, `b(...)`).
- **H15** : `make raylib` (non exigé, jamais lancé) lierait : aucun fichier nouveau, `game_data.c` et `data_init.c` sont déjà dans ses deux listes.
- **H16** : les TU qui lisent les vues (`test_pico8_js`, `test_pico8_ui`, `test_pico8_pj`, `test_pico8_hp`) passent sans modification, constantes de hachage comprises.

## Testing Decisions

Un bon test juge ce que le code rend (valeurs, ordres, pixels, pointeurs), pas la présence d'une ligne. D.3 remplace une source de données : **la preuve est que les données générées sont égales à ce qu'`ob()` produit des chaînes de départ, et que tout ce qui mesure le comportement rend la même chose.**

### Le test ponctuel `tests/test_pico8_map_tables.c`, étendu (G)

- **Il construit son état, il ne rejoue rien.** Il appelle `pico8_pi_init()` avant ses nouvelles références. Une aide `parse(s)` rend `pico8_ob(s, NULL)`. Les treize chaînes de référence sont **les littéraux de l'arbre de départ, octet pour octet**. Ses 2 lignes `stash` ne changent pas.
- **Tables à feuilles** (`es`, `b_`, `co`, `fb`, `ek` en `int` ; `ct`, `ci` en fixe ; `ef` en chaînes) : `ref->count == PICO8_X_COUNT` ; pour chaque `i`, **dans l'ordre**, l'entrée `i` de la référence est la positionnelle `i + 1`, et sa valeur est un fixe égal à `pico8_fixed_from_int(table[i])` (ou à `table[i]` pour un fixe), ou une chaîne `strcmp`-égale. Les aides rendent chaque index `1..count` et échouent sur `0` et `count + 1` sans toucher `*out`.
- **Vues** (`ix`, `ha`, `bo`, `bz` par leurs constructeurs publics ; `df` par `pico8_df()` après le `pico8_data_init(&mem)` déjà présent) : égalité profonde avec la référence par l'aide générique existante, étendue d'une règle : **un `REF` rangé sous le nom `ra`, `hi` ou `gq` est un vecteur**, comparé par `x` et `y` (et non récursivement comme une table). Aucun `type-tag` n'est nommé.
  - **`df`** : la référence est `parse(df)` suivie de la passe du cart faite dans le test (`mr` = `"px","s","r","d"`, `gp` `NULL`, `pico8_ho_apply` sur chaque positionnelle relevée avant la passe). `pico8_df()` lui est égale ; en plus, la nommée `px` de chaque enregistrement pointe **le même** enregistrement que sa positionnelle (H5).
  - **`ha`** : la vue est égale à la référence, y compris `hi`, `gq`, `ca` ; `hi` et `gq` sont deux vecteurs distincts.
  - **`bz`** : la vue est égale à la référence, y compris les faux de la rangée 10.
- **Lecture de la puce « chaque table générée est égale, entrée par entrée »** : les huit tables à feuilles sont comparées directement ; les cinq autres à travers la vue, qui émet **chaque** champ de la table typée dans l'ordre. Une donnée typée qu'aucune vue n'émet n'existe pas.
- **Ce que le test ne nomme pas** : aucun motif de `spec` ni d'`overrides` (règle de la liste), aucun `type-tag`, aucun lecteur par nom (`->field`, `*_take_*(`).
- Le fichier est supprimé à D.8. D.4a et D.5a l'étendent encore.
- Rejeté : **comparer à `pico8_ix()`/`pico8_bz()` après `menu_init`/`score_panel_init`.** Modules hors du lien du test, et `ix` y porte déjà `my`/`hb`.
- Rejeté : **lire les chaînes de référence dans `pico8/lua.lua`.** Un analyseur de plus pour rien : les littéraux de départ sont ceux que le port parsait, et le vérificateur les compare au départ.

### TU touchés

- **Aucun autre TU ne change.** Si un TU rougit parce qu'une table est présente avant son `init` (H3), ou parce qu'un pointeur partagé a changé (H5), l'implémenteur **s'arrête** : voir les critères d'échec.

### Pas de nouvel e2e (G)

Le §7 n'admet un e2e que s'il ajoute de la couverture de lignes en moins de 180 frames. Les lignes nouvelles sont couvertes par le chargement du cart, déjà joué par les 26 e2e, et par le test ponctuel. Aucun e2e n'est ajouté.

### Preuve des tests (§7, gcov + mutations)

- **gcov**, dans une copie jetable du scratchpad : `test_pico8_map_tables` compilé en `--coverage`. Il doit exécuter chaque ligne de `pico8_game_data.c` (les deux aides comprises) et chaque ligne des constructeurs de vue de `data_init.c` (dont la branche `is_false` de la vue de rangée). Les lignes des lecteurs typés (`economy_score.c`, `economy.c`, `menu.c`, `entity.c`, `score_panel.c`) ne sont pas exécutées par ce TU ; elles sont justifiées dans le message de commit par le filet e2e (26 conformes, empreintes inchangées) et par les TU qui les atteignent.
- **Quatre mutations**, chacune doit rougir :
  - **M1** : le faux de `o(f,0)` dans `bz` écrit `{.n = 0}` (sans `is_false`) → `test_pico8_map_tables` rouge ;
  - **M2** : `pico8_ct_table[0]` écrit `-0x553f` (la valeur non masquée) → rouge ;
  - **M3** : `pico8_int_seq_at` accepte `i == 0` → rouge ;
  - **M4** : la vue `bo` omet `en` → rouge.

  Chaque mutation est annulée. Une mutation qui ne rougit pas est un trou, justifié dans le commit ou couvert par une assertion ajoutée au test ponctuel.

### Vérification, budget et tableau « Runs »

**Budget : environ 8 minutes de temps total de tests.**

| Cible | Durée attendue |
|---|---|
| générateur (scratchpad, arbre de départ) | < 1 min, compilation comprise |
| `make test` | quelques secondes, plus une recompilation large |
| `make e2e` | ~1 s |
| `make sanitize-net` | < 2 min (H12) |
| gcov + 4 mutations (scratchpad, 1 TU) | < 3 min |
| `make data-guard`, `make data-residue` | < 1 s |

`make class-coverage-net` n'est pas lancé (H8) ; il le devient si le diff touche un constructeur.

Ordre conseillé :

1. `make data-residue` sur l'arbre de départ (H1) ;
2. le générateur sur l'arbre de départ, **avant toute modification** ;
3. les données, les types, les aides et le test ponctuel des tables à feuilles, puis `make test` ;
4. `is_false`, les constructeurs de vue et leur test, puis `make test` ;
5. les lecteurs typés, les modules propriétaires, `cart.c`, puis `make test` ;
6. `make e2e`, `make sanitize-net` une fois à la fin ;
7. gcov et mutations ;
8. gardes, `grep`, glossaire, note du plan.

Au-delà du budget, l'implémenteur s'arrête et note ce qui manque, plutôt que de relancer. **Jamais l'appli raylib.** `make raylib` n'est pas exigé.

**L'implémenteur tient un tableau « Runs »** dans son journal de progression, une ligne par lancement :

| # | Commande exacte | État de l'arbre (HEAD + « données ajoutées » / « vues » / « lecteurs typés »…) | Code de sortie | Ligne clé | Durée |
|---|---|---|---|---|---|

Exemples de ligne clé : `résidu D : 9691 ligne(s), 144 fichier(s)`, puis le pied final ; `test_pico8_map_tables: all tests passed` ; `26 conforme(s)` ; `sanitize: 26 propre(s)`. Le vérificateur réutilise ce tableau et ne relance que ce qui suit le dernier changement de l'arbre.

**Cibles finales**, toutes vertes sur l'arbre final :

1. `timeout 5m make test` (dont `test_pico8_map_tables`, `test_pico8_js`, `test_pico8_ui`, `test_pico8_pj`, `test_pico8_hp`) ;
2. `timeout 5m make e2e` : **26 « conforme »** ;
3. `timeout 10m make sanitize-net` : `sanitize: 26 propre(s)` ;
4. `make data-guard` : code 0 ;
5. `make data-residue` : `stash` 28/4, `type-tag` 445, `ref-audit` 70, `par-nom` 1081, `dispatch` 197, `overrides` 449, `spec` 939, `ob` hors test ponctuel < 2227, 142 fichiers ; écart éventuel expliqué.

### Contrôles du vérificateur

1. **Périmètre du diff** : `git show --stat` liste les 18 chemins de « Empreinte remesurée », et rien d'autre. Aucun `*.sha256`, rien sous `tests/e2e/`, aucun fichier d'événements, aucun changement du `Makefile`. Les trois tests du §6 et les quatre TU lecteurs des vues sont intacts.
2. **Constructeurs** : aucun hunk ne touche le corps d'un `pico8_*_new`/`pico8_*_struct_new`, un `pico8_class_coverage_record` ni `KNOWN[]` ; sinon `make class-coverage-net` doit figurer, identique à sa référence.
3. **Module** : `pico8_game_data.c/.h` n'incluent ni `pico8_ob.h`, ni `pico8_value.h`, ni `pico8_stash.h`, et `sh tools/data_residue.sh --detail` ne les nomme pas. Les tables de tête sont dimensionnées par `PICO8_X_COUNT`. Aucun `PICO8_FIXED_LIT` nouveau, aucun flottant, les `?` sont échappés.
4. **Lecteurs** : le `grep` de la « Sortie de l'étape » est vide ; `pico8_eg` parcourt `pico8_fb_table` dans l'ordre ; les index hors séquence restent « absent » (aides).
5. **Chaînes de référence** : chacune des treize chaînes du test est identique, octet pour octet, au littéral correspondant de `git show c631538:src/…`.
6. **Vues** : `data_init.c` n'a plus ni `df_mr`, ni `pico8_data_init_rename_pass`, ni parse de `df`/`ek` ; chaque vue est construite par une fonction nommée ; `menu_init`, `help_init`, `score_panel_init` appellent leur constructeur de vue là où ils parsaient, et leurs `*_shutdown` libèrent les mêmes tables de tête.
7. **Test ponctuel** : il ne lit aucun `events.json` ; il compare les huit tables à feuilles entrée par entrée et dans l'ordre, les cinq vues par égalité profonde (vecteurs par `x`/`y`), et les pointeurs partagés de `df`. Il ne nomme ni `spec`, ni `overrides`, ni `type-tag`, et garde 2 lignes `stash`.
8. **Commentaires** : ceux qui sont ajoutés relèvent des quatre sortes de `CLAUDE.md` (contrats des aides, sens des bits d'un fixe généré, `is_false`, propriété des vues). Aucun récit de chantier, aucun renvoi à D.3 ni au plan dans `src/` ou `tests/`.
9. **Docs** : glossaire à jour pour les treize noms et `pico8_jb_row_t`/`pico8_jb_item_t` ; note « Découvert à D.3 » en fin de ligne D.3, rien d'autre dans le plan.

### Critères d'échec (traduits en `FAILED:` par le vérificateur de `prd-autopilot`)

**Départ et filet (§7)**

- `FAILED:` si l'arbre de départ n'est pas `c631538` (ou un commit de docs au-dessus) propre, ou si `make data-residue` n'y rend pas l'inventaire.
- `FAILED:` si le filet (`make test`, `make e2e`, `make sanitize-net`) reste rouge après **3 tentatives**.
- `FAILED:` si un `*.sha256` est modifié ou supprimé, si un fichier de `tests/e2e/` est touché, ou si un e2e est ajouté ou retiré.
- `FAILED:` si `make e2e` ne rend pas **26 « conforme »** ou si `make sanitize-net` ne rend pas 26 propres.
- `FAILED:` si le diff touche un constructeur, un `pico8_class_coverage_record` ou `KNOWN[]` sans que `make class-coverage-net` soit lancé et identique à sa référence.
- `FAILED:` si un test de régression du §6 (`gh_row_lq`, `ga_export_panel_by`, `layout_nil_item_draws_nothing`) est modifié.
- `FAILED:` si un TU existant autre que `test_pico8_map_tables.c` change. Un TU rouge parce qu'une table est présente avant son `init` (H3) ou parce qu'un pointeur partagé a changé (H5) est ce cas : l'implémenteur s'arrête et le note.

**Périmètre**

- `FAILED:` si une ligne de `pico8_(es|ct|b_|co|fb|ek|ci|ef)()`, `pico8_game_data_(init|shutdown)`, `g_(es|ct|b_|co|fb|ek|ci|ef)` ou `df_mr` subsiste dans `src/` ou `tests/`.
- `FAILED:` si `pico8_game_data.c/.h` incluent `pico8_ob.h`, `pico8_value.h` ou `pico8_stash.h`, ou portent une ligne d'une catégorie du compteur.
- `FAILED:` si une valeur de `pico8_game_data.c` a été écrite ou corrigée à la main au lieu de venir du générateur, ou si un non-entier y est écrit par `PICO8_FIXED_LIT` ou en décimal.
- `FAILED:` si un générateur, un script ou une cible `make` de génération est versionné.
- `FAILED:` si le diff convertit une clé de construction (`js`, `mh`, `jo`), `jo.nk`, `pz`, `pj:ph`, une spécification (`g_fv_spec`, `g_pj_spec`) ou un `*_default`, ou s'il change la signature de `pico8_data_init`, `pico8_ho_apply` ou `pico8_pz`.
- `FAILED:` si une vue n'est plus construite au même moment (`*_init`) ou n'est plus libérée par le même `*_shutdown`, ou si un vecteur de vue est partagé ou `const`.
- `FAILED:` si le test ponctuel rejoue un fichier d'événements, nomme un motif de `spec`, d'`overrides` ou de `type-tag`, ou porte plus de 2 lignes `stash`.
- `FAILED:` si un TROU est ajouté ou modifié, ou si un écart port/cart est corrigé.

**Gardes et comptes**

- `FAILED:` si `make data-guard` n'est pas vert, ou si `DATA_RESIDUE_GUARD`, `tools/data_residue.sh` ou un motif change.
- `FAILED:` si `make data-residue` ne rend pas `stash` 28/4, `type-tag` 445, `ref-audit` 70, `par-nom` 1081, `dispatch` 197, `overrides` 449, `spec` 939 ; ou si `ob` hors test ponctuel n'est pas < 2227.
- `FAILED:` si `make constants-check`, `float-grep` ou `float-guard` rougit, ou si `tools/float_guard.txt` gagne une ligne.

**Preuve, commit et note**

- `FAILED:` si le rapport gcov ou l'une des mutations M1 à M4 manque, ou si une mutation ne rougit pas sans justification au commit.
- `FAILED:` si le travail n'est pas en un seul commit vert.
- `FAILED:` si le budget est dépassé sans note de ce qui n'a pas été lancé, s'il manque le tableau « Runs », ou si le message de commit ne porte pas le pied de `make data-residue` avant et après et la description du générateur.
- `FAILED:` s'il manque la note « Découvert à D.3 », ou si elle omet un pont et son étape, ou le compte `ob` réel.
- `FAILED:` si un commentaire ajouté ou reformulé sort des quatre sortes de `CLAUDE.md`.

## Out of Scope

- **Convertir les lecteurs des vues** : `jo.nk` et le tirage de `bw` (D.4a, puis D.7a pour `galaxy`), les clés de construction de `js` via `nv(ix, js)` et `pz` et celles de `mh` (D.6b), la réécriture `st[3]` de `pj:ph` (D.7a), `cart.c` (D.7a).
- **Les spécifications de classe** qui vivent à côté de ces tables : `g_fv_spec` (`help.c`), `g_pj_spec` (`score_panel.c`) et leurs `*_default` (D.5b), avec les `v(...)` qui obligent encore à `pico8_pi_init()`.
- **Les tables ad hoc** de `menu.c` (`pico8_cr_step`, `extra`/`lq2`), `score_panel.c` (`ou`), `economy_score.c` (`pico8_ky`) : D.7a.
- **Supprimer `pico8_ob.c/.h`, le test ponctuel** : D.8.
- **Libérer les tables imbriquées et les vecteurs des vues** : la propriété reste celle d'aujourd'hui (le port fuit par conception, TROU-7).
- `make raylib`, l'appli raylib, l'audio, le rendu, `make e2e-gap` et `tools/line_coverage_reference.txt` (hors suite, décalés par tout diff de `src/`).

## Further Notes

- **Décisions de l'agent à la place de l'utilisateur** (consigne `to-prd` : vérifier modules, tests et découpage avec l'utilisateur), tranchées en `guideline` et marquées (G) plus haut :
  - **modules** : `pico8_game_data.c/.h` devient le module pur des treize tables (sans dépendance ob, sans `init`) ; les vues ob sont construites dans `data_init.c`, publiques pour `ix`, `ha`, `bo`, `bz` afin que le test ponctuel les atteigne. Règle de D.2 inchangée : « lecteur de feuille → typé ; constructeur ou entité → vue, jusqu'à l'étape qui le convertit ».
  - **forme C** : `int` pour un champ entier, `pico8_fixed_t` en hexadécimal signé des bits d'`ob()` sinon (jamais `PICO8_FIXED_LIT`) ; tailles en énumération ; `pico8_jb_row_t` réutilisé pour les `lq` et les rangées de `bz`, `pico8_jb_item_t` gagne `is_false` ; deux aides d'indexation 1-based.
  - **générateur** : un programme C jetable qui parse les treize chaînes de départ après `pico8_pi_init()` et refait la passe `df_mr`, décrit au commit, non versionné.
  - **tests** : le test ponctuel étendu (huit tables directement, cinq par leur vue, vecteurs par `x`/`y`), aucun autre TU touché, pas d'e2e, gcov et quatre mutations. `class-coverage-net` non exigé (aucun constructeur touché).
  - **découpage** : un seul commit, sans « Implementation Chunks ».
  - **comptes** : `stash` 28 exact ; `ob` au-dessus du majorant du plan, à cause des ponts ; les autres catégories exactes ; 142 fichiers.
- **Note du plan (G)**, à ajouter en fin de ligne D.3 (le §7 ne garde que ce qui change une étape suivante) :

  > Découvert à D.3 : `pico8_game_data.c/.h` devient le module pur des tables constantes (générées une fois, sans dépendance ob ni `init` ; `pico8_int_seq_at`, `pico8_fixed_seq_at` ; un non-entier est l'hexadécimal signé des bits qu'`ob()` rendait, masque `0xffff.fffe` compris, jamais `PICO8_FIXED_LIT`) ; `pico8_jb_item_t` gagne `is_false`. Huit tables lues typées (`es`, `ct`, `b_`, `co`, `fb`, `ek`, `ci`, `ef`). Cinq ponts, des vues ob construites par `data_init.c` (publiques `pico8_ix_view`, `pico8_ha_view`, `pico8_bo_view`, `pico8_bz_view`, appelées par les `*_init`) : `pico8_df()` (fermé par D.4a, `jo.nk` ; `galaxy` à D.7a s'il reste), `pico8_ix()` et `pico8_ha()` (D.6b, clés de `js` et appelants de `menu.c`), `g_bo` (D.6b, `pz` → `js`), `pico8_bz()` (D.6b, entrée de `mh`, puis D.7a `score_panel_display`). `ob` après D.3 : N (au lieu de ≤ 2137) ; les majorants `ob` suivants se décalent de N − 2105.

  N est le compte réel, hors test ponctuel. Les noms des constructeurs de vue sont ceux de l'implémentation.
- **Contrôle d'étape** : un sous-agent frais relit le diff contre ce PRD avant le commit (hors budget de tests).
- **Le message de commit** porte : le périmètre (18 chemins) ; la règle de forme et ses rejets ; le générateur en ≤ 15 lignes ; le pied de `make data-residue` avant et après, et le compte `ob` hors test ; le rapport gcov et les mutations ; le tableau « Runs » résumé ; les réponses à H1 à H16.
- **Surprises pour le pilote** :
  - Le majorant `ob` ≤ 2105 (≤ 2137 après D.2) suppose que D.3 convertit aussi les lecteurs de `df`, `ix`, `ha`, `bo`, `bz`. Or ces lecteurs sont des entités (`jo.nk`) et des constructeurs (`js`, `mh`) que la liste place à D.4a, D.6b et D.7a. Comme à D.2, le PRD garde des vues et décale le compte au lieu d'absorber ces étapes ; l'écart attendu est plus grand qu'à D.2 (cinq ponts au lieu de trois).
  - Quatre tables portent des **non-entiers** (`ct`, `ci`, `df.r`, `bo.en`), que la forme « entiers `int` » de D.2 ne couvre pas, et `ob()` les **masque** (`-0.333` → `-0x5540`, ni le relevé ni `-PICO8_FIXED_LIT(0x553f)`).
  - `ix`, `ha` et `bz` contiennent des `v(...)` : `ob()` y appelle un constructeur et range des vecteurs du tas ; les vues doivent en allouer, et le test doit les comparer par clé (`ra`, `hi`, `gq`) sans `type-tag`.
  - `bz` contient des **faux** : le type de rangée de D.2 doit s'étendre.
  - `DATA_ENGINE_SRCS`, lié par le test ponctuel, n'a ni `menu.c`, ni `help.c`, ni `score_panel.c` : d'où les constructeurs de vue publics dans `data_init.c`.
  - `pico8_ci()` et `pico8_ef()` n'avaient aucun lecteur ; `pico8_game_data_init/shutdown` n'ont plus rien à faire : tous disparaissent, et `game_data.c/.h` sortent du compteur (144 → 142 fichiers).
