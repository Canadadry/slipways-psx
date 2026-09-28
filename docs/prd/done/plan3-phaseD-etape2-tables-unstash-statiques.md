---
title: "Plan 3, étape D.2 : les six tables décodées par `unstash` (`r`, `iz`, `hc`, `fd`, `dr`, `nb` avec sa passe `nb_mr`/`gp={\"nk\"}`) deviennent des tables `static const` typées, générées une fois depuis ce que `ob()`/`unstash` produisent sur le cart réel puis versionnées (`pico8_map_tables.c/.h`) ; `r`, `iz`, `hc` lus typés par leurs consommateurs, `fd`, `dr`, `nb` remis aux leurs sous forme de vue ob construite depuis les tables typées ; le test ponctuel `test_pico8_map_tables` naît — un seul commit, aucun pixel changé"
description: "Au démarrage, `pico8_data_init()` et `pico8_stash_init()` décodent six chaînes planquées dans la mémoire de la map (`unstash`), puis les parsent avec `ob()` en tables allouées. Ce sont des constantes du cart : le plan 3 veut qu'elles deviennent des données `static const` versionnées, pour qu'à D.8 `pico8_stash.c` et `pico8_ob.c` puissent disparaître. D.2 génère ces six tables une fois, fait lire `r`, `iz` et `hc` sous forme typée par leurs 13 consommateurs, garde une vue ob pour `fd`, `dr` et `nb` (remises à des constructeurs ou gardées par des entités que des étapes ultérieures convertissent), et prouve par un test ponctuel que chaque table générée est égale, entrée par entrée, à `pico8_ob(pico8_unstash(mem, pd))`."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Phase D — Données », liste écrite après D.1) :

> - D.2 — `stash` : tables décodées par `unstash` — `g_hc`, `g_fd`, `g_dr`, `g_nb` (`data_init.c`, avec la passe `nb_mr` et son routage `gp={"nk"}`), `g_r`, `g_iz` (`pico8_stash.c`, lus par `pico8_r()`/`pico8_iz()`) → `static const` générés (première puce) ; le test ponctuel naît ici : chaque table générée est égale, entrée par entrée, à `pico8_ob(pico8_unstash(mem, pd))`, la mémoire étant chargée par `pico8_load_map_json` (pas de rejeu). `pico8_stash.c/.h` et `test_pico8_stash.c` restent jusqu'à D.8. Empreinte 22 (15, 7). Compte : `stash` = 26 (`pico8_stash.c` 7, `pico8_stash.h` 7, `test_pico8_stash.c` 12) ; `ob` ≤ 2195. §6 : aucun. `class-coverage-net` : seulement si le diff touche un constructeur.

En-tête de la liste de D et règles communes, recopiés :

> Liste de D, écrite après D.1 (HEAD 3816427, agent `guideline`), à partir de `make data-residue` et `sh tools/data_residue.sh --detail`. Comptes à l'écriture (lignes/fichiers) : `valeur` 5989/132, `ob` 2245/128, `stash` 56/14, `type-tag` 445/71, `ref-audit` 70/17, `par-nom` 1081/80, `dispatch` 197/59, `overrides` 449/62, `spec` 939/68, **9678 lignes distinctes dans 143 fichiers** ; ceux de la note de D.0 sont d'avant D.1. Chaque ligne nomme la catégorie qu'elle pousse et son « Compte » après elle : « = » attendu exact, « ≤ » majorant par grep (lignes de la catégorie qui nomment les sites des groupes déjà passés, cumulées dans l'ordre de la liste). Le test ponctuel (deuxième puce de D) est hors compte jusqu'à D.8, où il est supprimé ; il ne nomme aucun motif de `spec` ni `overrides`. Une catégorie tombée à 0 entre dans `DATA_RESIDUE_GUARD` dans le même commit.
>
> Empreinte (mesurée à l'écriture, chaque PRD la remesure) : fichiers de `src/*.[ch]` et `tests/*.[ch]` qui portent une ligne de la catégorie du groupe, ou qui nomment ses tables ou membres (majorant : noms courts partagés, `nu`, `k_`, `lq`). Règles communes : un nom nouveau ne correspond à aucun motif figé de la catégorie qu'il remplace (pas de `g_X_spec`/`g_X_defaults`/`pico8_X_default`, et le paramètre `overrides` typé est renommé) ; un lecteur par nom qui survit reste compté par `par-nom` ; un défaut muté (§5, 1a : `pj.ou`, `pj.ea`, `pj.cr`, `ga.by`, `nf.l`, `nf.r`, `jo.kw`) devient un tableau fixe statique non `const`. Un TU construit son état et ne rejoue jamais de fichier d'événements ; un e2e nouveau ajoute de la couverture de lignes en moins de 180 frames ; aucun `*.sha256` ni fichier de `tests/e2e/` ne change. `make sanitize-net` : à chaque étape qui touche `src/`, puisque toutes font passer des tables ob à des données statiques ou locales (§7, « En D et en 2 »).

Puces de la phase D, recopiées :

> - Les tables constantes parsées au démarrage (`g_es`, `g_ct`, `g_b_`, `g_co`, `g_fb`, `g_ek`, `ci`, menus `g_ix`/`g_ha`, aide `g_bo`, score `g_ef`/`g_bz`, `g_df`) et celles décodées par `unstash` (`g_hc`, `g_fd`, `g_dr`, `g_nb`, `g_r`, `g_iz`) deviennent des tableaux/structs `static const`, **générés une fois** à partir de ce que `ob()`/`unstash` produisent aujourd'hui, puis versionnés.
> - Test ponctuel, supprimé dans le commit qui supprime `pico8_ob.c`/`pico8_stash.c` : chaque table générée est égale, entrée par entrée, à ce que `ob()`/`unstash` produit. Le filet ne voit pas les entrées jamais affichées (aide, score, menus).
> - Les tables ad hoc construites en cours de partie (~68 `ob_table_new()`) deviennent des structs locales ou des tableaux fixes.
> - **Sortie** : `pico8_ob.c`, `pico8_value.c`, `pico8_type_tag.h`, `pico8_ref_audit.h`, `pico8_stash.c` supprimés ; plus aucun `malloc` pour des données. Les entités gardent le leur : c'est Plan 2 §8.

Les lignes voisines, pour la frontière :

> - D.3 — `ob`, tables constantes parsées au démarrage (liste de la première puce) : `g_es`, `g_ct`, `g_b_`, `g_co`, `g_fb` (`game_data.c`), `g_ek`, `g_df` avec sa passe `df_mr` (`data_init.c`), `g_ci` (`entity.c`), `g_ix`/`g_ha` (`menu.c`), `g_bo` (`help.c`), `g_ef`/`g_bz` (`score_panel.c`) → `static const` générés, test ponctuel étendu. […]
> - D.8 — suppression : `pico8_ob.c/.h` (avec `pico8_fi`), `pico8_stash.c/.h`, `test_pico8_ob.c` et `test_pico8_stash.c` (§6, tests de la couche), le test ponctuel, `PICO8_TYPE_TAG_OB_TABLE`, leurs entrées du `Makefile`. Compte : `ob` = 0, `stash` = 0, ajoutés à `DATA_RESIDUE_GUARD`. […]

Règles du §7, recopiées :

> - **Filets depuis la fusion de master (e2e rapide)** : `FRAMES_EVENTS` n'existe plus ; `make e2e` rejoue les 26 e2e de `tests/e2e/` depuis l'écran titre, `make sanitize-net` et `make class-coverage-net` rejouent ce même jeu, `tools/class_coverage_net_expected.txt` y a été reposé à la fusion ; aucun TU ne rejoue de fichier d'événements : un TU construit son état directement, un rejeu est un e2e de `tests/e2e/` (moins de 180 frames, et il doit ajouter de la couverture de lignes). Les mentions de `FRAMES_EVENTS`, `frames-check` et `float-audit` dans les étapes livrées sont historiques.
> - **En D et en 2** (ajouté à la revue de sortie de 1b) : `make class-coverage-net` aux étapes dont le diff touche un constructeur, un appel `pico8_class_coverage_record` ou `KNOWN[]`, et à la sortie de phase ; `make sanitize-net` aux étapes qui changent la durée de vie ou le propriétaire d'une donnée (table ob → locale ou statique, `malloc`/`free` retiré), et à la sortie de phase.

Lecture de ces règles pour D.2 :

- **`make sanitize-net` est exigé** : D.2 change la durée de vie des chaînes (tas, jamais libérées → littéraux statiques) et le propriétaire de `r`, `iz`, `hc` (tables ob du tas → données statiques).
- **`make class-coverage-net` est exigé** : le diff touche le corps du constructeur de `hp`. `pico8_hp_pq_i_step` (le `{qe=r[c]}` de `hp:pq`) lit `r`, et `pico8_ho_apply`, appelé par `pico8_hp_struct_new`, route par `r`. Aucun compte d'instanciation ne doit bouger.

### État de départ

HEAD `3b5af98` (branche `claude/plan-pilot-3-7zdft0`), arbre propre. D.1 est livrée (`baead8e`) et gradée (`3816427`) ; `3b5af98` écrit la liste de D. Décisions de l'utilisateur qui engagent D.2 : **un TU ne rejoue jamais un `events.json`** (il construit son état), **rien de lent**, **aucun `*.sha256` ni fichier de `tests/e2e/` touché**, **un e2e nouveau ajoute de la couverture de lignes en moins de 180 frames**.

**Rien n'a été compilé ni lancé pour ce PRD.** Seuls `make -s data-residue` et `sh tools/data_residue.sh --detail` ont tourné. Le contenu des six tables vient de la commande `jq` de l'annexe A de `docs/design/slipways-mechanics.md` (lecture de `pico8/map.json`), et le reste de `grep`. Tout fait d'exécution est une hypothèse (H1 à H15).

### Inventaire (mesuré sur `3b5af98`)

**`make data-residue`** :

```
valeur	5989	132
ob	2245	128
stash	56	14
type-tag	445	71
ref-audit	70	17
par-nom	1081	80
dispatch	197	59
overrides	449	62
spec	939	68
résidu D : 9678 ligne(s), 143 fichier(s)
```

**`stash` par fichier** (`--detail`) : `pico8_stash.c` 7, `pico8_stash.h` 7, `test_pico8_stash.c` 12 (ces trois restent) ; puis 30 lignes à éteindre :

- `data_init.c` 6 : l'inclusion, `pico8_stash_init(mem)` et les quatre `pico8_unstash` ;
- `data_init.h` 2 : deux commentaires qui nomment `pico8_stash_init`/`pico8_stash_shutdown` ;
- 2 lignes chacun (une inclusion de `pico8_stash.h` et un appel `pico8_r()`) : `collect.c`, `economy.c`, `economy_formulas.c`, `planet_display.c`, `planet_trade.c`, `research.c`, `score_panel.c` ;
- `economy_score.c` 4 (une inclusion, trois `pico8_r()`) ;
- `research_display.c` 4 (une inclusion, deux `pico8_iz()`, un `pico8_r()`).

**Les six tables** (adresse `unstash`, compte d'entrées, forme de ce qu'`ob()` produit) :

| Table | `pd` | Entrées | Forme |
|---|---|---|---|
| `r` | 8192 | 13 | clés nommées d'une lettre (`f`, `o`, `b`, `p`, `g`, `t`, `l`, `w`, `k`, `?`, `a`, `e`, `h`) → entiers 12..54 |
| `iz` | 8259 | 11 | clés nommées `"28"`, `"21"`… → noms de ressource (`"food"`…) |
| `hc` | 8383 | 6 | clés nommées (`f-`, `f+`, `-`, `+`, `qh`, `qh+`) → table de 3 ou 4 entiers positionnels, plus `mc` (chaîne) sur `f-`, `-`, `qh` |
| `fd` | 8494 | 42 | positionnelles : 8 chaînes d'en-tête (`"xenofoods"`…) et 34 enregistrements `o(...)` : positions 1-2 et 4-8 chaînes (7 : chaîne ou `f` = faux), 3 entier (`-20` compris), nommées `ii` (entier), `go`, `cw` (chaînes), `rn` (entier), dans cet ordre quand présentes |
| `dr` | 9807 | 7 | positionnelles : une bulle chacune, liste de rangées ; une rangée = chaînes et entiers positionnels, plus `c` (entier) sur la rangée de titre |
| `nb` | 10457 | 38 | 19 technologies positionnelles `o(px, lv, nk, od, oy)` (`nk` écrit en lettre : `t`, `l`, `o`, `b`, `a`) ; puis la passe `nb_mr` ajoute à chaque enregistrement les nommées `px`, `lv`, `nk` (routé par `r` : entier), `od`, `oy`, et à `nb` 19 nommées `px` → le même enregistrement. `oy` = liste de chaînes, d'entiers et de rangées (mêmes rangées que `dr`) |

**Consommateurs** (hors `pico8_stash.*`, `data_init.*` et leurs TU) :

| Table | Lecteur | Ce qu'il en fait |
|---|---|---|
| `r` | `collect.c` (`pico8_ho_apply`) | route `r[ri[i]]` : index si la clé brute est un nombre, nom si c'est une chaîne |
| `r` | `economy.c`, `economy_score.c` (×3), `planet_trade.c`, `score_panel.c`, `planet_display.c`, `research_display.c`, `research.c` (`pico8_hp_pq_i_step`) | lecture par clé nommée d'une lettre (`k`, `w`, `p`, `h`, `e`, `?`, `r[c]`) ; plusieurs dispatchent ensuite sur le genre (fixe ou chaîne) de la valeur |
| `r` | `economy_formulas.c` (`pico8_cm`) | **parcourt les 13 valeurs dans l'ordre des entrées** |
| `iz` | `research_display.c` (×2) | lecture par `tostr` d'un fixe ; la chaîne va dans une table de bulle |
| `hc` | `research.c` (`pico8_research_lookup`, `pico8_hp_po`) | `hc[ja]`, puis `hc[qp.mc]` ; une clé fixe indexe (sans jamais trouver : `hc` n'a que des clés nommées) |
| `hc` (via `pico8_hp_po`) | `research_display.c`, `economy_tiers.c` (`ob_sequence_length`, index borné), `tests/test_pico8_hp.c:427` (`entry_of(pico8_hc(), po)`, dans un hachage) | `po[1]`, `#po`, `po[min(mi, #po)]` |
| `fd` | `research.c` (`pico8_research_build_gb`) | écrit `rr` **dans chaque enregistrement**, puis le passe à `pico8_hp_new` (clés de construction) ; `g_rr` emprunte la chaîne d'en-tête |
| `dr` | `cart.c:226` | `dr[i+1]` passé à `pico8_jb_new` (clés de construction de `jb`) |
| `nb` | `tech_gate.c` (`pico8_cb`, `pico8_gi`) | **écrit `e_`** dans l'enregistrement ; `menu.c` le garde en `_cr_t` d'une rangée, `js._cr_t` le garde, `hy:kt` le lit |

**Autres faits qui engagent la forme :**

- `pico8_ob_table_get_*` rendent la valeur telle quelle : une chaîne parsée est un `pico8_sub` du tas que personne ne possède. `tests/test_pico8_data_init.c:44`, `:58` la libèrent (`free((void *)…as.string)`), comme `test_pico8_stash.c` (qui garde ses tables à lui).
- `tests/test_pico8_hp.c` lit `pico8_fd()` (l. 614, 624, 775-850) et compare **par pointeur** les chaînes de chaque projet à celles de l'enregistrement de `fd`, dont le `rr` que `build_gb` y a écrit.
- `pico8_cart_load` n'appelle `pico8_data_init(mem)` que si `mem != NULL` ; personne n'appelle `pico8_stash_shutdown` hors `test_pico8_stash.c`.
- `PICO8_FIXED_LIT` est comparé par `make constants-check` à `tools/probe_out.txt` ; `pico8_fixed_from_int` est une fonction (pas une expression constante).
- `CFLAGS` = `-std=c99 -Wall -Wextra -Werror -g` : les trigraphes sont actifs en `-std=c99` et `-Wall` les signale. `fd` contient `"??"`.
- Listes de sources du `Makefile` qui compilent des consommateurs : `ENGINE_SRCS`, `DATA_ENGINE_SRCS`, la règle de `test_pico8_layout_nil_item_draws_nothing` (lie `collect.c`), les deux lignes de `$(TMP_DIR)/main_raylib`. `CORE_SRCS` et `REPLAY_SRCS` dérivent d'`ENGINE_SRCS`.

## Solution

**Un seul commit**, sans aucun pixel changé :

1. **Un module de données nouveau, `pico8_map_tables.c/.h`** (« les tables planquées dans la map du cart »). Le `.c` contient les six tables en `static const` / `const`, émises **une fois** par un générateur jetable qui lit ce qu'`ob()`/`unstash` produisent sur l'arbre de départ, puis versionnées et éditées comme n'importe quel source. Les types sont écrits à la main dans le `.h`. S'y ajoutent trois recherches (`r`, `iz`, `hc`). Le module n'inclut ni `pico8_ob.h`, ni `pico8_value.h`, ni `pico8_stash.h` : il ne contribue à aucune catégorie du compteur.
2. **`r`, `iz`, `hc` sont lus typés** par leurs consommateurs : plus de `pico8_r()`, `pico8_iz()` ni `pico8_hc()` hors de `pico8_stash.*` et de son TU. `pico8_hp_po` rend une entrée typée de `hc`.
3. **`fd`, `dr`, `nb` gardent leur accesseur ob** (`pico8_fd()`, `pico8_dr()`, `pico8_nb()`), mais `pico8_data_init` les construit à partir des tables typées, et non plus par `unstash` + `ob()`. Ces trois **vues ob** sont des ponts : leurs lecteurs sont des constructeurs (`hp`, `jb`) ou des entités (`js._cr_t`, rangées de `menu`) que D.4b, D.6b, D.6c et D.7a convertissent. `pico8_data_init` n'appelle plus `pico8_stash_init` ni `pico8_unstash`, et la passe `nb_mr` disparaît : ses résultats sont dans la table générée.
4. **Le test ponctuel `tests/test_pico8_map_tables.c`** (dans `UNIT_TESTS`) charge la mémoire par `pico8_load_map_json`. Il compare chaque table générée, entrée par entrée, à `pico8_ob(pico8_unstash(mem, pd))`. Pour `nb`, la passe `nb_mr` est refaite par `pico8_ho_apply` dans le test. Pour `fd`, `dr` et `nb`, le test compare aussi la vue ob que `pico8_data_init` a construite.
5. `pico8_stash.c/.h` et `test_pico8_stash.c` ne changent pas (supprimés à D.8). Le glossaire et une note « Découvert à D.2 » du plan suivent.

**Sortie de l'étape**, constatée par commande :

- `make test` vert (dont `test_pico8_map_tables`) ;
- `make e2e` : 26 conformes ;
- `make sanitize-net` : 26 propres ;
- `make class-coverage-net` : identique à la référence ;
- `make data-guard` : code 0 ;
- `make data-residue` : `stash` = 28 lignes dans 4 fichiers (26 + les 2 du test ponctuel) ; les autres comptes sont dans « Effet sur `make data-residue` » ;
- `grep -rnE '\bpico8_(r|iz|hc)\(\)|pico8_stash\.h|pico8_unstash' src tests --include='*.[ch]'` ne rend que `pico8_stash.c`, `pico8_stash.h`, `test_pico8_stash.c` et les 2 lignes de `test_pico8_map_tables.c`.

## User Stories

1. En tant que mainteneur du port, je veux que les six tables décodées par `unstash` soient des données C versionnées, afin de les lire dans un diff sans décoder la mémoire de la map.
2. En tant que mainteneur, je veux que ces tables soient `const` et en stockage statique, afin qu'aucun code ne puisse les muter ni les libérer par erreur.
3. En tant que lecteur de `pico8_cm`, je veux parcourir `r` comme un tableau de 13 paires dans l'ordre du cart, afin de voir que l'ordre compte et d'où il vient.
4. En tant que lecteur d'un site qui lit `r.w` ou `r.k`, je veux un appel qui rend un entier présent ou absent, afin de ne plus dispatcher sur un genre de valeur qu'aucune entrée de `r` n'a.
5. En tant que lecteur de `hp:jb()`, je veux trouver le nom d'une ressource dans `iz` par une recherche qui rend une chaîne ou `NULL`, afin de ne pas manipuler un `pico8_value_t` pour une table de chaînes.
6. En tant que lecteur de `hp:po()`, je veux recevoir une entrée typée de `hc` (ses valeurs positionnelles, leur nombre et son `mc`), afin que `po[1]`, `#po` et `po[i]` se lisent comme des accès à un tableau.
7. En tant que lecteur d'`economy_tiers.c`, je veux que `#po` soit un champ, afin de supprimer l'aide qui recomptait les clés positionnelles d'une table ob.
8. En tant qu'auteur des étapes D.4b, D.6b, D.6c et D.7a, je veux que `fd`, `dr` et `nb` existent déjà sous forme typée, afin que ces étapes n'aient qu'à brancher leurs constructeurs et entités sur ces types et à retirer la vue ob.
9. En tant que lecteur de `pico8_data_init`, je veux que chaque vue ob soit construite par une fonction nommée qui lit la table typée, afin de savoir quelle étape la retirera.
10. En tant que gardien du comportement, je veux que la vue ob de `fd`, `dr` et `nb` soit égale, entrée par entrée et dans l'ordre, à ce que `ob()` + la passe `nb_mr` produisaient, afin que `hp`, `jb`, `menu`, `tech_gate` et `js` voient exactement les mêmes tables.
11. En tant que gardien du comportement, je veux que les enregistrements de `nb` restent des tables partagées entre la clé positionnelle et la clé nommée `px`, afin que l'écriture de `e_` par `tech_gate` reste visible par les deux clés, comme dans le cart.
12. En tant que vérificateur, je veux un TU qui compare chaque table générée à `pico8_ob(pico8_unstash(mem, pd))` sur le vrai `pico8/map.json`, afin de prouver que la génération n'a rien perdu : ni une entrée, ni un ordre, ni un genre, ni un faux.
13. En tant que vérificateur, je veux que ce TU prouve aussi ce que le filet ne voit pas, comme les rangées de bulle jamais affichées par un e2e, afin de ne pas dépendre de la couverture du filet.
14. En tant que vérificateur, je veux que ce TU charge la mémoire par `pico8_load_map_json` et ne rejoue aucun fichier d'événements, afin de respecter la règle des TU.
15. En tant que vérificateur, je veux voir le TU rougir sur une valeur générée changée et sur deux entrées de `r` permutées, afin de savoir qu'il mord sur les valeurs et sur l'ordre.
16. En tant que gardien du §6, je veux qu'aucun des trois tests de régression (`gh_row_lq`, `ga_export_panel_by`, `layout_nil_item_draws_nothing`) ne change, afin que D.2 ne touche à aucune preuve de bug.
17. En tant que gardien de `test_pico8_hp`, je veux que ses constantes de hachage restent identiques, afin que la nouvelle forme de `hc` et la vue de `fd` rendent les mêmes projets, membres et index.
18. En tant que mainteneur, je veux que le générateur soit jetable et que le test ponctuel soit la preuve durable, afin de ne pas maintenir un outil qui lie `pico8_ob.c`, lequel disparaît à D.8.
19. En tant qu'auteur de D.3, je veux trouver dans le message de commit comment le générateur a été écrit (entrées, parcours, assertions), afin d'en écrire un pour les tables constantes parsées au démarrage.
20. En tant qu'auteur de D.8, je veux un test ponctuel dans un fichier à lui, qui ne porte que 2 lignes `stash`, afin de le supprimer d'un bloc avec `pico8_stash.*`.
21. En tant que gardien du compteur, je veux qu'aucun nom nouveau ne corresponde à un motif figé (`pico8_r()`, `pico8_stash_*`, `pico8_ob*`, `*_default`, `*_field`, `*_take_*(`), afin que les comptes mesurent la conversion et non un renommage.
22. En tant que gardien de `make test`, je veux que `constants-check`, `float-grep` et `float-guard` restent verts, afin que le module nouveau, qui est dans le cœur, ne porte ni `PICO8_FIXED_LIT` nouveau ni flottant.
23. En tant que développeur, je veux que les chaînes générées soient à l'abri des trigraphes, afin que `"??"` ne devienne pas une erreur `-Werror` ni un autre caractère.
24. En tant que lecteur de `docs/glossaire-noms.md`, je veux que les lignes `r`, `iz`, `hc`, `fd`, `dr`, `nb` nomment les noms C d'après D.2, afin de retrouver une table depuis son nom de cart.
25. En tant que `plan-pilot`, je veux une note « Découvert à D.2 » qui donne les trois ponts et l'étape qui ferme chacun, et le compte `ob` réel qui décale les majorants suivants, afin que les étapes suivantes partent de faits.
26. En tant que gardien des filets, je veux `make e2e` à 26 conformes, `make sanitize-net` à 26 propres et `make class-coverage-net` identique, afin de prouver qu'aucun pixel, aucune instanciation et aucun accès mémoire n'a changé.
27. En tant que `make raylib` (non lancé), je veux que les deux listes de sources de `main_raylib` nomment le module nouveau, afin que le binaire interactif lie encore.
28. En tant que `plan-pilot`, je veux que toute conversion des tables constantes de D.3, des constructeurs, de `jb`, `menu`, `tech_gate` ou `js._cr_t` reste hors de ce commit, afin que chaque étape reste une étape.

## Implementation Decisions

Pas d'humain dans la boucle. Le skill `to-prd` demande de faire valider par l'utilisateur les modules, les tests et le découpage. Ces choix sont tranchés ici comme le ferait un agent `guideline` (hypothèses explicites, simplicité, changement chirurgical, critères vérifiables), et marqués **(G)**. Les rejets sont notés à chaque décision.

### Règle de forme, table par table (G)

**Une table lue feuille par feuille devient typée jusque chez son lecteur. Une table remise à un constructeur, ou dont une entité garde ou mute un morceau, est typée dans les données, mais son lecteur reçoit une vue ob construite depuis ces données, jusqu'à l'étape qui convertit ce lecteur.**

| Table | Lecteurs | Forme livrée par D.2 | Pont retiré par |
|---|---|---|---|
| `r` | 11 sites de lecture | typée jusqu'au lecteur | — |
| `iz` | 2 sites | typée jusqu'au lecteur | — |
| `hc` | `hp:po` et ses 3 appelants | typée jusqu'au lecteur | — |
| `fd` | `build_gb` → `pico8_hp_new(ri)`, avec `rr` écrit dans `ri` | typée + vue ob `pico8_fd()` | D.6c (clés de construction de `hp`) |
| `dr` | `cart.c` → `pico8_jb_new(dr[i])` | typée + vue ob `pico8_dr()` | D.6b (clés de construction de `jb`) |
| `nb` | `tech_gate` (écrit `e_`), `menu` (`_cr_t`, `oy`), `js._cr_t`, `hy:kt`, `gi` | typée + vue ob `pico8_nb()` | D.4b (`js._cr_t`) puis D.7a (`menu`, `tech_gate`) |

Rejets :

- **Typer aussi les lecteurs de `fd`, `dr`, `nb` dans D.2.** Il faudrait :
  - changer l'entrée des constructeurs de `hp` et de `jb` (D.6b, D.6c) ;
  - retyper l'enregistrement de technologie gardé par `js._cr_t` (D.4b) et par les rangées de `menu`, et muté par `tech_gate` (D.7a) ;
  - trancher le type des éléments de rangée de `lq`, ce que la liste laisse ouvert (arrêt n° 3).

  L'empreinte doublerait, et trois étapes de la liste seraient absorbées.
- **Vue ob pour les six tables**, aucun lecteur ne changeant. Les 11 sites `pico8_r()` doivent de toute façon perdre leur accesseur (`stash` = 26), et un simple renommage laisserait `ob` et `valeur` intacts. C'est un renommage, pas une conversion.
- **Générer les tables sous forme ob statique** (`pico8_ob_entry_t` littéraux). Plusieurs centaines de lignes `ob` et `valeur` en plus, et des données à reconvertir avant D.8.
- **Remplacer seulement `unstash` par la chaîne décodée**, versionnée et encore parsée par `ob()`. La première puce veut des tableaux/structs, générés depuis ce qu'`ob()` produit, et non depuis ce qu'`unstash` produit.

### Les types (G)

Forme retenue (les noms de champs suivent le cart, voir le glossaire) :

```c
typedef struct { const char *key; int qe; } pico8_r_entry_t;          /* r : 13 */
typedef struct { const char *key; const char *name; } pico8_iz_entry_t; /* iz : 11 */
typedef struct { const char *key; int count; int values[4]; const char *mc; } pico8_hc_entry_t; /* hc : 6, mc NULL si absent */

typedef struct pico8_jb_row pico8_jb_row_t;
typedef struct { const char *s; int n; const pico8_jb_row_t *row; } pico8_jb_item_t; /* s : une chaîne ; row : une rangée ; sinon le nombre n */
struct pico8_jb_row { int count; const pico8_jb_item_t *items; int has_c; int c; };

typedef struct {
    const char *header;  /* non NULL : entrée chaîne nue, le rr des enregistrements qui suivent ; le reste est nul */
    int npos;            /* positions 1..npos présentes */
    const char *px, *lr; int fj; const char *ja, *i, *o, *il, *dm;
    int il_false;        /* position 7 écrite `f` */
    int has_ii, ii; const char *go, *cw; int has_rn, rn;
} pico8_fd_entry_t;      /* fd : 42 */

typedef struct { const char *px; int lv; const char *nk_key; int nk; int od; const pico8_jb_row_t *oy; } pico8_nb_entry_t; /* nb : 19 */
/* dr : 7 pico8_jb_row_t, chacune une rangée de rangées */
```

- **Entiers `int`, pas `pico8_fixed_t`.** Toutes les valeurs numériques des six tables sont entières, ce que le générateur vérifie. `pico8_fixed_from_int` n'est pas une expression constante, et un `PICO8_FIXED_LIT` nouveau casserait `constants-check`. Le lecteur convertit par `pico8_fixed_from_int` là où il a besoin d'un fixe.
  - Rejeté : une macro de décalage constante nouvelle. Ce serait un second chemin vers le fixe, pour rien.
- **Tailles** en constantes d'énumération dans le `.h` (`PICO8_R_COUNT` 13, `PICO8_IZ_COUNT` 11, `PICO8_HC_COUNT` 6, `PICO8_FD_COUNT` 42, `PICO8_DR_COUNT` 7, `PICO8_NB_COUNT` 19). Les tables de tête sont déclarées `extern const T pico8_X_table[PICO8_X_COUNT]` : un compte faux ne compile pas. Les tableaux imbriqués (éléments de rangée, rangées) sont `static const`, nommés, dans le `.c`.
  - Rejeté : les littéraux composés à portée de fichier. Ils sont valides en C99, mais moins lisibles dans un diff.
- **`pico8_jb_row_t`/`pico8_jb_item_t`** typent le contenu **constant** d'une bulle (chaînes, entiers, rangées imbriquées, `c`). Ils ne tranchent pas la représentation des `lq.items` d'exécution (fonctions, nil, fixe 0), qui reste l'arrêt n° 3 de la liste. D.6b pourra les réutiliser.
- **`nb` garde les deux formes de `nk`.** `nk_key` est la position 3 telle qu'écrite (`"t"`), `nk` est `r[nk_key]` après la passe. La vue les restitue toutes deux, comme la table actuelle.
- **Ordre des nommées** : `fd` émet `ii`, `go`, `cw`, `rn` dans cet ordre ; `hc` et les rangées émettent leur nommée (`mc`, `c`) après les positionnelles ; `nb` émet ses nommées dans l'ordre de `nb_mr`. C'est l'ordre du cart, et le générateur échoue s'il en voit un autre (H2).
- **Recherches**, écrites à la main à la fin du `.c` :
  - `int pico8_r_lookup(const char *key, int *out_qe)` rend 1 et écrit l'entier si la clé est présente, sinon 0 ;
  - `const char *pico8_iz_lookup(const char *key)` rend `NULL` si la clé est absente ;
  - `const pico8_hc_entry_t *pico8_hc_lookup(const char *key)` rend `NULL` si la clé est absente.

  Ce sont des recherches linéaires par `strcmp`, comme `pico8_ob_table_get_name`. `key` ne doit pas être `NULL` : c'est une assertion avec message.
- **Aucun nom nouveau ne correspond à un motif figé** : ni `pico8_(r|iz)()`, ni `pico8_stash_*`, ni `pico8_ob*`, ni `*_default*`, ni `*_field*`, ni `*_take_*(`. `pico8_map_tables` ne contient pas `stash`. Les commentaires des deux fichiers ne portent que des contrats (ce que rend une recherche, le sens de `header`, `il_false`, `nk_key`), selon les quatre sortes de `CLAUDE.md`. Pas de bandeau « généré, ne pas éditer » : le fichier est un source versionné comme un autre, gardé par le test ponctuel jusqu'à D.8.

### Le générateur (G) : jetable, en C, sur l'arbre de départ

- **Un programme C de l'implémenteur, dans son scratchpad, jamais versionné.** Il est compilé contre les sources de l'arbre de **départ** (`$(DATA_ENGINE_SRCS)` de `3b5af98`), **avant** toute modification. Il fait :
  - `pico8_load_map_json(&mem, "pico8/map.json")` ;
  - pour `r`, `iz`, `hc`, `fd`, `dr` : `pico8_ob(pico8_unstash(&mem, pd), NULL)` ;
  - pour `nb` : `pico8_data_init(&mem)`, puis lecture de `pico8_nb()`, déjà passé par `nb_mr`. `fd` n'est pas lu par `pico8_fd()`, où `build_gb` a déjà écrit `rr` ;
  - il parcourt ces tables ob et émet le C des tableaux dans l'ordre des entrées.
- **Il échoue fort** si une hypothèse de forme tombe (H2) :
  - nombre non entier ;
  - positionnelles non contiguës depuis 1 ;
  - clé nommée hors de l'ensemble attendu de la table ;
  - ordre des nommées différent de celui du tableau des types ;
  - `REF` qui n'est pas une table ob (constructeur enregistré sous `o`) ;
  - `nk` routé qui n'est pas un fixe ;
  - plus de 4 valeurs dans une entrée de `hc`.
- **Échappement** : `\\`, `\"`, `\n`, et **chaque `?` en `\?`** (trigraphes, `"??"` dans `fd`).
- Sa sortie est collée dans `pico8_map_tables.c`, puis remise en forme à la main à la largeur des fichiers voisins (≤ 120 colonnes). Aucune valeur n'est éditée à la main.
- Le message de commit décrit le générateur en ≤ 15 lignes (entrées, parcours, assertions, échappement), pour D.3.
- Rejeté : **un générateur versionné** (`tools/` et une cible `make`). La puce dit « générés une fois ». Il lierait `pico8_ob.c`, qui disparaît à D.8, et il deviendrait un outil mort à maintenir. La preuve durable est le test ponctuel.
- Rejeté : **un générateur en `jq`/shell** depuis le texte décodé. Il réimplémenterait la grammaire d'`ob()` (clé nommée collante, `f` faux, masque des nombres, virgule finale). La puce dit « à partir de ce que `ob()` produit ».

### Conversion des lecteurs typés

| Fichier | Site | Après D.2 |
|---|---|---|
| `collect.c` | `pico8_ho_apply`, `r[ri[i]]` | clé brute chaîne : `pico8_r_lookup`, valeur `pico8_value_fixed(pico8_fixed_from_int(qe))` ; clé brute fixe : **absente** (`r` n'a que des clés nommées : l'ancien `get_index` ne trouvait jamais), donc `new_value = raw` comme avant ; retrait de l'inclusion |
| `economy.c` | `r.k` | `pico8_r_lookup("k", …)`. La branche « valeur chaîne » disparaît : aucune entrée de `r` n'est une chaîne. La branche fixe indexe `self->po` comme avant |
| `economy_formulas.c` | `pico8_cm` | boucle `i < PICO8_R_COUNT` sur `pico8_r_table[i].qe`, **dans l'ordre**, `set_by_value(p, fixe(qe), fixe(jm))` ; la garde `r != NULL` part |
| `economy_score.c` | `r.w`, `r.p`, `r.h`, `r.w` (3 fonctions) | `pico8_r_lookup` ; les valeurs passent en `pico8_value_t` fixe là où `economy_value_equal_content`/`get_by_value` les attendent ; la ligne 207 garde sa lecture `kw.qe` |
| `planet_trade.c` | `r.e`, `r["?"]` | `pico8_r_lookup` ; `have_r_*` gardent leur sens |
| `score_panel.c` | `r.k` | idem |
| `planet_display.c`, `research_display.c:155` | `r.w` | idem, `pico8_value_equal` sur un fixe construit |
| `research.c` | `pico8_hp_pq_i_step` (`{qe=r[c]}`) | `pico8_r_lookup(key, &qe)`, absent → `pico8_value_ref(NULL)` comme avant |
| `research.c` | `pico8_research_lookup`, `pico8_hp_po` | `hc[ja]` : `ja` `NULL` → absent ; sinon `pico8_hc_lookup(ja)`. `mc` : `qp->mc != NULL` → `pico8_hc_lookup(qp->mc)`. `pico8_research_lookup` disparaît (sa branche fixe ne trouvait jamais). `pico8_hp_po` rend `const pico8_hc_entry_t *` |
| `research.h` | `pico8_hp_po` | signature, inclusion de `pico8_map_tables.h` |
| `research_display.c` | `po[1]`, `iz` ×2 | `po->count >= 1 ? po->values[0]`, sinon 1 comme avant ; `pico8_iz_lookup(num_str)`, `pico8_iz_lookup(o_str)` : non `NULL` → `pico8_value_string(name)` à l'index 3 |
| `economy_tiers.c` | `#po`, `po[min(mi, #po)]` | `po->count` ; `ob_sequence_length` supprimée. **Un index < 1 reste « absent » (base 0)**, comme `get_index` sur une clé ≤ 0 |
| `cart.c:50` | commentaire « (stash/research, … ) » | « (research, score-panel display) » : `pico8_data_init` n'initialise plus `stash` |

- Les inclusions de `pico8_stash.h` sortent des 10 fichiers de `src/` qui l'avaient pour `pico8_r()`/`pico8_iz()`, et de `data_init.c`. Le module nouveau entre où il sert.
- **Aucune condition de jeu ne change de sens.** Les gardes `r != NULL` tombent : `r` est présent dès le chargement du programme (H3). La branche morte « `r[x]` est une chaîne » tombe aussi : le générateur et le test prouvent qu'aucune valeur de `r` n'est une chaîne.

### `pico8_data_init` et ses vues ob (G)

- Plus d'inclusion de `pico8_stash.h`, plus de `pico8_stash_init(mem)`, plus de `pico8_unstash`, plus de `decoded`. `nb_mr`, `nb_gp` et l'appel de la passe sur `nb` disparaissent. `pico8_data_init_rename_pass` et `pico8_data_init_positional_values` **restent** pour `df` (D.3).
- `g_hc`, `pico8_hc()` et sa déclaration disparaissent ; `hc` est lu par `pico8_hc_lookup`.
- **Trois constructeurs de vue, statiques dans `data_init.c`**, chacun libre de son nom hors motifs (par exemple `data_init_fd_view`, `data_init_dr_view`, `data_init_nb_view`, et `data_init_row_view`, récursif pour les rangées). Chacun rend une table ob neuve, construite avec `pico8_ob_table_new`/`set_index`/`set_name`, dans **l'ordre exact des entrées** de la table parsée :
  - **`fd`** : une chaîne d'en-tête → `pico8_value_string(header)`. Un enregistrement → une table : positions 1..`npos` (position 7 : `pico8_value_bool(0)` si `il_false`, sinon la chaîne), puis `ii`, `go`, `cw`, `rn` présents.
  - **`dr`** : une rangée → une table : éléments positionnels (chaîne, fixe, ou vue de la rangée imbriquée), puis `c` si `has_c`.
  - **`nb`** : chaque enregistrement → une table. Les positions 1..5 sont `px`, `lv`, `nk_key` (chaîne), `od` et la vue de `oy`. Les nommées `px`, `lv`, `nk` (fixe routé), `od`, `oy` suivent, **la même vue `oy`** (même pointeur) étant rangée aux deux clés. `nb` reçoit les 19 enregistrements en positionnelles, puis 19 nommées `px` → **le même pointeur** d'enregistrement.
- **Propriété inchangée** : `pico8_data_shutdown` libère la table de tête de chaque vue avec `pico8_ob_table_free`, comme aujourd'hui. Les tables imbriquées ne sont pas libérées, comme aujourd'hui (le port fuit par conception, `detect_leaks=0`). Les chaînes sont désormais statiques : **personne ne doit les libérer** (voir les TU).
- `build_gb` écrit toujours `rr` dans les enregistrements de la vue `fd`, et `tech_gate` écrit `e_` dans ceux de la vue `nb` : les vues sont du tas, non `const`, comme les tables parsées. Les données typées ne sont jamais mutées. La règle « défaut muté → tableau fixe statique non `const` » ne s'applique pas : la mutation porte sur la vue.
- **`pico8_data_init(pico8_mem_t *mem)` garde sa signature.** `mem` n'est plus lu (`(void)mem;`), et le contrat du `.h` le dit. Changer la signature toucherait `cart.c` et cinq TU, dont `gh_row_lq` et `ga_export_panel_by` du §6, que la ligne D.2 déclare intouchés. Le retrait du paramètre est laissé à D.7a (`data_init`) : c'est dans la note.
- `data_init.h` : le contrat de tête ne nomme plus `pico8_stash_init` ni `pico8_stash_shutdown`. `pico8_df()`, `pico8_fd()`, `pico8_dr()`, `pico8_nb()` et `pico8_ek()` restent « NULL avant le premier `pico8_data_init()` ».
- Rejeté : **construire les vues dans le module de données.** Il inclurait `pico8_ob.h` et entrerait dans le compteur : le module de données doit rester propre, et `data_init.c` construit déjà ces tables (D.7a le vide).
- Rejeté : **un indicateur « prêt » sur `r`/`iz`/`hc`** pour rendre « absent » avant `pico8_data_init`. Le cart décode `r` au chargement : aucun état du cart n'a `r` absent. Ce serait reproduire un artefact du port (H3).

### `Makefile`, docs

- `src/pico8_map_tables.c` entre dans `ENGINE_SRCS` et `DATA_ENGINE_SRCS` (ordre alphabétique), dans la règle de `test_pico8_layout_nil_item_draws_nothing` (qui lie `collect.c`) et dans les **deux** lignes de `$(TMP_DIR)/main_raylib`. `pico8_stash.c` reste partout où il est : son retrait est à D.8.
- `test_pico8_map_tables` entre dans `UNIT_TESTS`, après `test_pico8_data_init`, avec la règle `$(TMP_DIR)/test_pico8_map_tables: $(call objs,tests/test_pico8_map_tables.c $(DATA_ENGINE_SRCS))`, sur le modèle de `test_pico8_data_init`.
- La légende de tête ne change pas. Aucune cible nouvelle, `DATA_RESIDUE_GUARD` reste vide (aucune catégorie ne tombe à 0).
- `docs/glossaire-noms.md` : les lignes `r`, `iz`, `hc` nomment `pico8_r_table`/`pico8_r_lookup`, `pico8_iz_table`/`pico8_iz_lookup`, `pico8_hc_table`/`pico8_hc_lookup`. Les lignes `fd`, `dr`, `nb` nomment la table typée **et** la vue `pico8_fd()`/`pico8_dr()`/`pico8_nb()`. Une ligne ajoute `pico8_jb_row_t` (contenu constant d'une bulle).
- `docs/plan-3.md` : note « Découvert à D.2 » en fin de ligne D.2 (texte dans Further Notes).

### Ce qui ne change pas

- Aucun pixel, aucune instanciation, aucune condition de jeu.
- Aucun `*.sha256`, aucun fichier de `tests/e2e/`, aucun fichier d'événements.
- `pico8_stash.c`, `pico8_stash.h`, `tests/test_pico8_stash.c`, `pico8_ob.c/.h`, `pico8_collect.h` (signature de `pico8_ho_apply`), `pico8_layout.*`, `pico8_menu.c`, `pico8_tech_gate.c`, `pico8_ui_button.*`.
- Les TU qui lisent les vues `pico8_dr()`/`pico8_nb()` : `test_pico8_js.c`, `test_pico8_jb_ol.c`, `test_pico8_entity_kind.c`, `test_pico8_ie.c`. Les trois tests du §6.
- `tools/data_residue.sh`, `tests/test_data_residue.c`, ses motifs, `DATA_RESIDUE_GUARD`, `tools/class_coverage_*`, `KNOWN[]`.
- **TROU** : aucun écart port/cart n'est touché ni ajouté.

### Empreinte remesurée

- **`src/` : 16 chemins.** Deux nouveaux (`pico8_map_tables.c/.h`) et 14 modifiés : `data_init.c/.h`, `collect.c`, `economy.c`, `economy_formulas.c`, `economy_score.c`, `planet_trade.c`, `score_panel.c`, `planet_display.c`, `research.c/.h`, `research_display.c`, `economy_tiers.c`, `cart.c` (commentaire).
- **`tests/` : 3 chemins.** Un nouveau (`test_pico8_map_tables.c`) et deux modifiés :
  - `test_pico8_data_init.c` : les assertions sur `hc` passent au typé, et les deux `free` de chaînes désormais statiques sont retirés ;
  - `test_pico8_hp.c:427` : `entry_of(pico8_hc(), po)` devient l'index `po - pico8_hc_table`.
- Plus le `Makefile`, `docs/glossaire-noms.md` et `docs/plan-3.md`, soit **22 chemins**.
- Écart avec la ligne du plan (22 : 15, 7) : `tech_gate.c` et les quatre TU lecteurs de `dr`/`nb` ne changent pas (vues). `economy_tiers.c` et `research.h` s'ajoutent (lecteurs de `hp:po`). Le module et le test sont nouveaux.

### Effet sur `make data-residue`

Attendu après D.2 (H11) :

| Catégorie | Avant | Après | Pourquoi |
|---|---|---|---|
| `stash` | 56/14 | **28/4** (exact) | 26 au plan (`pico8_stash.c` 7, `.h` 7, `test_pico8_stash.c` 12) + 2 lignes du test ponctuel (son inclusion de `pico8_stash.h` et l'unique appel `pico8_unstash`, dans une aide `decode(mem, pd)`), hors compte du plan jusqu'à D.8 |
| `ob` | 2245/128 | **< 2245 hors test ponctuel**, attendu entre 2210 et 2235 (H11) ; le test ponctuel en ajoute (≈ 20 à 40) | −≈ 50 lignes qui nomment les sites (les 11 lignes de `data_init.c` pour `hc`, les parses, `nb_mr` et l'accesseur ; celle de `data_init.h` ; ≈ 22 lignes de lecture de `r`/`iz` ; ≈ 13 de `hc`/`po` ; 3 de `test_pico8_data_init.c`), +≈ 25 à 35 lignes de vues ob dans `data_init.c` |
| `type-tag` | 445 | **445** (exact) | aucun site touché n'en porte ; le test ponctuel n'en nomme aucun |
| `ref-audit` | 70 | **70** (exact) | — |
| `par-nom` | 1081 | **1081** (exact) | aucune ligne touchée ne porte un lecteur par nom ; aucun nom nouveau en `*_field*`/`*_take_*(` |
| `dispatch` | 197 | **197** (exact) | — |
| `overrides` | 449 | **449** (exact) | aucune ligne touchée ; ni les vues ni le test ne nomment `overrides` |
| `spec` | 939 | **939** (exact) | le test ponctuel ne nomme aucun motif de `spec` (règle de la liste) |
| `valeur` | 5989/132 | rapporté au commit | les vues et le test ajoutent des `pico8_value_*`, les lecteurs typés en retirent |
| Fichiers | 143 | **144** (H11) | + `test_pico8_map_tables.c` ; le module nouveau ne porte aucun motif, et aucun fichier existant ne tombe à 0 |

- **Le majorant `ob` ≤ 2195 du plan n'est pas atteint (G).** Il compte les ≈ 50 lignes qui nomment les sites, sans les ponts. Les vues ob de `fd`, `dr` et `nb` en ajoutent ≈ 30, qui partent avec D.4b, D.6b, D.6c et D.7a. L'implémenteur écrit le compte exact dans le commit et dans la note ; les majorants `ob` suivants (D.3 ≤ 2105, …) se décalent du même écart. Ce n'est pas un arrêt du §7.
- Aucune catégorie ne tombe à 0 : **`DATA_RESIDUE_GUARD` reste vide.** Aucun motif ne change.

### Découpage (G) : un seul commit

- Le §7 veut un commit par groupe. La découpe « module + test d'abord, lecteurs ensuite » a été évaluée et écartée :
  - le premier commit ajouterait des données qu'aucun code de production ne lit ;
  - le test des vues ne pourrait naître qu'avec le second ;
  - `sanitize-net` et `class-coverage-net` passeraient deux fois pour une seule conversion.
- 22 chemins, sous le seuil de ~40. **Pas de section « Implementation Chunks »**, que `prd-autopilot` traiterait comme autant de commits. L'ordre de travail conseillé est dans Testing Decisions.

### Hypothèses (rien n'a été lancé)

- **H1** : `3b5af98` est vert : `make test`, `make e2e` (26 conformes), `make sanitize-net` (26 propres), `make class-coverage-net` identique, `make data-guard` code 0, `make data-residue` égal à l'inventaire. Si c'est démenti au départ, l'implémenteur s'arrête et note : D.2 ne répare rien.
- **H2** : ce qu'`ob()` produit a la forme de l'inventaire. Toutes les valeurs numériques sont entières. Les positionnelles précèdent les nommées dans chaque table. L'ordre des nommées est celui du tableau des types. Aucun constructeur n'est enregistré sous `o` au moment du parse : tout `o(...)` est une table. Les 19 `nk` routés sont des fixes (`t`, `l`, `o`, `b`, `a` sont dans `r`). `hc` a au plus 4 valeurs. Les comptes sont 13, 11, 6, 42 (8 en-têtes, 34 enregistrements), 7, 38 (19 + 19). Le générateur le vérifie. Si c'est démenti, l'implémenteur adapte le type à ce qu'`ob()` produit (le test ponctuel reste juge) et note l'écart. Ce n'est un arrêt que si un champ ne tombe sous aucune règle.
- **H3** : aucun TU ni e2e ne dépend de `r`, `iz` ou `hc` **absents** avant `pico8_data_init` (ils sont maintenant présents dès le chargement). Un `grep` des TU qui atteignent un lecteur de `r` sans `pico8_data_init`, `pico8_cart_load` ni `pico8_bootstrap` n'en trouve aucun. Si un TU rougit pour cette raison, voir les critères d'échec.
- **H4** : aucun code de `src/` ne libère une chaîne lue dans ces six tables. `research.c` emprunte (`g_rr`, `hp_take_string`), `pico8_research_free_gb_instances` ne libère que les instances. Les seuls `free` de ce genre sont `test_pico8_data_init.c:44`, `:58` (retirés) et ceux de `test_pico8_stash.c` (sur ses tables à lui, inchangés). Un `free` oublié d'une chaîne statique aborterait sous ASan (`gh_row_lq` est un TU ASan qui appelle `pico8_data_init`) et dans `sanitize-net`.
- **H5** : la branche « clé fixe » de `r[ri[i]]` dans `pico8_ho_apply` et celle de `hc[key]` dans `pico8_research_lookup` ne trouvaient jamais rien : `r` et `hc` n'ont que des clés nommées. Les réécrire en « absent » ne change rien.
- **H6** : `make e2e` rend 26 conformes, sans nouvelle référence. L'ordre de `r` dans `pico8_cm` est gardé, les vues sont égales aux tables parsées (test ponctuel), et les chaînes ont le même contenu.
- **H7** : les constantes `*_HASH` de `test_pico8_hp.c` ne changent pas. L'index d'une entrée de `hc` dans `pico8_hc_table` est son index d'entrée dans la table ob (6 nommées, ordre du parse). Les pointeurs de chaîne comparés par `test_hp_members_by_name` viennent de la vue `fd`, où les chaînes des projets sont les chaînes statiques elles-mêmes (`pico8_value_string` ne copie pas).
- **H8** : `-Werror` reste muet. `mem` est marqué `(void)`. Plus aucune fonction statique n'est inutilisée (`ob_sequence_length` et `pico8_research_lookup` supprimées). Les `?` des chaînes sont échappés, et `const char *` reçoit des littéraux.
- **H9** : `make class-coverage-net` est identique à `tools/class_coverage_net_expected.txt` : aucune instanciation n'est ajoutée ni retirée (les vues ne sont pas des classes).
- **H10** : `make sanitize-net` rend 26 propres. Aucun accès hors bornes dans les recherches (bornes par `PICO8_X_COUNT`), aucun `free` de littéral.
- **H11** : `make data-residue` rend les comptes du tableau. `ob` est dans la fourchette, mais l'exact dépend de l'écriture des vues : un écart hors fourchette est expliqué au commit, sans être un échec tant qu'`ob` hors test reste < 2245.
- **H12** : `make constants-check`, `float-grep` et `float-guard` restent verts. Le module nouveau est dans `CORE_SRCS`, mais ne porte que des entiers et des chaînes, sans `PICO8_FIXED_LIT` ni flottant.
- **H13** : les durées sont courtes. `make test` prend quelques secondes, plus une recompilation large (`research.h`, `data_init.h`). `make e2e` prend ~1 s. `sanitize-net` et `class-coverage-net` prennent moins de 2 min chacun, compilation comprise.
- **H14** : la vue `nb` rend les mêmes pointeurs partagés que la table parsée (positionnelle et nommée `px` → même enregistrement ; positionnelle 5 et nommée `oy` → même rangée). `tech_gate` écrit `e_` une fois, et `menu`/`js` la voient.
- **H15** : `make raylib` (non exigé, jamais lancé) lierait avec le module ajouté à ses deux listes.

## Testing Decisions

Un bon test juge ce que le code rend (valeurs, ordres, pixels, instanciations), pas la présence d'une ligne. D.2 remplace une source de données : **la preuve est que les données générées sont égales à ce qu'`ob()`/`unstash` produisent, et que tout ce qui mesure le comportement rend la même chose.**

### Le test ponctuel `tests/test_pico8_map_tables.c` (G)

- **Il construit son état, il ne rejoue rien.** `pico8_mem_init`, puis `pico8_load_map_json(&mem, "pico8/map.json")` (chemin relatif à la racine, comme `test_pico8_data_init`). Une aide `decode(mem, pd)` fait `pico8_ob(pico8_unstash(mem, pd), NULL)`, libère la chaîne et rend la table de référence : c'est **la seule** ligne `pico8_unstash` du fichier.
- **`r`** : `ref->count == PICO8_R_COUNT`. Pour chaque `i`, **dans l'ordre**, l'entrée `i` de la référence est nommée `pico8_r_table[i].key`, et sa valeur est un fixe égal à `pico8_fixed_from_int(pico8_r_table[i].qe)`. `pico8_r_lookup` rend chaque clé et échoue sur une clé absente (`"zz"`).
- **`iz`** : même forme, valeur chaîne `strcmp`. `pico8_iz_lookup` rend `NULL` sur une clé absente.
- **`hc`** : même forme. La valeur est une table dont les `count` premières entrées sont les positionnelles `1..count`, égales à `values[]` ; puis `mc` nommée si et seulement si `mc != NULL`. Le compte d'entrées est `count + (mc != NULL)`. `pico8_hc_lookup` rend le même pointeur que `&pico8_hc_table[i]`.
- **`fd`, `dr`, `nb` : par la vue.** Le test appelle `pico8_data_init(&mem)` et compare chaque vue à la référence par **une égalité profonde générique**, écrite une fois dans le test :
  - même compte ;
  - pour chaque index d'entrée : même genre de clé, même index ou même nom ; même genre de valeur ; fixe égal ; chaîne `strcmp` ; booléen égal ; `REF` égales récursivement (ou toutes deux `NULL`).

  Pour chaque table :
  - **`dr`** : `pico8_dr()` est égale à `decode(mem, 9807)`.
  - **`nb`** : la référence est `decode(mem, 10457)`, suivie de la passe du cart faite dans le test : `mr = pico8_ob("\"px\",\"lv\",\"nk\",\"od\",\"oy\",", NULL)`, `gp = {"nk"}`, `pico8_ho_apply(ref, mr, &gp, rec)` sur chaque positionnelle `REF`, relevée avant la passe. `pico8_nb()` lui est égale. En plus, l'entrée nommée `px` de chaque enregistrement pointe **le même** enregistrement que sa positionnelle, et la nommée `oy` pointe **la même** table que la position 5 (H14).
  - **`fd`** : `pico8_fd()` est lue après `build_gb`, qui a ajouté `rr` à chaque enregistrement. La comparaison porte sur tous les enregistrements de la référence `decode(mem, 8494)`, et chaque enregistrement de la vue a **exactement une** entrée de plus, la dernière, nommée `rr`. Sa valeur est la dernière chaîne d'en-tête rencontrée avant lui, ou `REF NULL` avant la première.
- **Lecture de la puce « chaque table générée est égale, entrée par entrée »** : `r`, `iz` et `hc` sont comparées directement. `fd`, `dr` et `nb` le sont à travers la vue, qui émet **chaque** champ de la table typée dans l'ordre : une valeur générée fausse rend la vue différente. Un test ne peut pas voir une donnée typée qu'aucune vue n'émet, et il n'y en a pas.
- **La passe `nb_mr` du test passe par `pico8_ho_apply`**, qui route après D.2 par `pico8_r_lookup`. La référence de `nb` dépend donc de `r` typé. C'est voulu : la comparaison de `r` précède, dans le même test. Elle couvre aussi la branche routée de `pico8_ho_apply`.
- **Ce que le test ne nomme pas** : aucun motif de `spec` ni d'`overrides` (règle de la liste), aucun `type-tag`, aucun lecteur par nom (`->field`, `*_take_*(`). Ses lignes `stash` sont 2 exactement.
- Le fichier est supprimé à D.8. D.3 et D.5a l'étendent.
- Rejeté : **comparer le texte C émis à une nouvelle sortie du générateur.** Ce serait garder le générateur, et la puce veut une égalité de tables.
- Rejeté : **mettre le test dans `test_pico8_stash.c`**. Son compte (12) est figé par le plan, c'est un test de la couche, et il ne connaît ni `data_init` ni les vues.

### TU touchés, même intention

- **`test_pico8_data_init.c`** :
  - `pico8_hc()->count == 6`, `"f-"`/`"qh+"` présents et `"does-not-exist"` absent deviennent `PICO8_HC_COUNT == 6`, `pico8_hc_lookup("f-") != NULL`, `pico8_hc_lookup("qh+") != NULL`, `pico8_hc_lookup("does-not-exist") == NULL` ;
  - les lectures de `pico8_fd()`/`pico8_dr()`/`pico8_nb()` restent ;
  - les deux `free((void *)….as.string)` sont retirés : les chaînes sont statiques (H4).
- **`test_pico8_hp.c:427`** : `entry_of(pico8_hc(), po)` devient `(int)(po - pico8_hc_table)`. Le type de `po` suit `pico8_hp_po`. **Aucune constante `*_HASH` ne change** (H7).
- Aucun autre TU ne change. Si un TU rougit parce que `r`, `iz` ou `hc` sont présents avant `pico8_data_init` (H3), l'implémenteur **s'arrête** : voir les critères d'échec.

### Pas de nouvel e2e (G)

Le §7 n'admet un e2e que s'il ajoute de la couverture de lignes en moins de 180 frames. Les lignes nouvelles sont couvertes par le chargement du cart (`pico8_data_init`), déjà joué par les 26 e2e, et par les TU. Aucun e2e n'est ajouté.

### Preuve des tests (§7, gcov + mutations)

- **gcov**, dans une copie jetable du scratchpad : `test_pico8_map_tables`, `test_pico8_data_init` et `test_pico8_hp` compilés en `--coverage`. Ils doivent exécuter :
  - chaque ligne des trois recherches ;
  - chaque ligne des quatre constructeurs de vue ;
  - `pico8_hp_po`, `pico8_hp_pq_i_step` et la branche routée de `pico8_ho_apply`.

  Les lignes des autres lecteurs de `r` (économie, score, affichage) ne sont pas exécutées par ces TU. Elles sont justifiées dans le message de commit par le filet e2e (26 conformes, empreintes inchangées).
- **Trois mutations**, chacune doit rougir :
  - **M1** : un entier profond de `nb` (un élément de rangée de `oy`) changé dans `pico8_map_tables.c` → `test_pico8_map_tables` rouge ;
  - **M2** : deux entrées de `pico8_r_table` permutées → `test_pico8_map_tables` rouge (l'ordre de `pico8_cm`) ;
  - **M3** : `pico8_hp_po` ignore `mc` → `test_pico8_hp` rouge (`FUNCTIONS_HASH`).

  Chaque mutation est annulée. Une mutation qui ne rougit pas est un trou, justifié dans le commit ou couvert par une assertion ajoutée au test ponctuel.

### Vérification, budget et tableau « Runs »

**Budget : environ 12 minutes de temps total de tests.**

| Cible | Durée attendue |
|---|---|
| générateur (scratchpad, arbre de départ) | < 1 min, compilation comprise |
| `make test` | quelques secondes, plus une recompilation large |
| `make e2e` | ~1 s |
| `make sanitize-net`, `make class-coverage-net` | < 2 min chacun (H13) |
| gcov + 3 mutations (scratchpad, 3 TU) | < 3 min |
| `make data-guard`, `make data-residue` | < 1 s |

Ordre conseillé :

1. `make data-residue` sur l'arbre de départ (H1) ;
2. le générateur sur l'arbre de départ, **avant toute modification** ;
3. le module, les types et le test ponctuel, puis `make test` (les données sont comparées avant que les lecteurs changent) ;
4. `data_init` et ses vues, puis les lecteurs typés, les TU touchés et le `Makefile`, puis `make test` ;
5. `make e2e`, `make sanitize-net`, `make class-coverage-net` une fois à la fin ;
6. gcov et mutations ;
7. gardes, `grep`, glossaire, note du plan.

Au-delà du budget, l'implémenteur s'arrête et note ce qui manque, plutôt que de relancer. **Jamais l'appli raylib.** `make raylib` n'est pas exigé.

**L'implémenteur tient un tableau « Runs »** dans son journal de progression, une ligne par lancement :

| # | Commande exacte | État de l'arbre (HEAD + « module ajouté » / « lecteurs typés »…) | Code de sortie | Ligne clé | Durée |
|---|---|---|---|---|---|

Exemples de ligne clé : `résidu D : 9678 ligne(s), 143 fichier(s)`, puis le pied final ; `test_pico8_map_tables: all tests passed` ; `26 conforme(s)` ; `sanitize: 26 propre(s)` ; la dernière ligne de `class-coverage-net`. Le vérificateur réutilise ce tableau et ne relance que ce qui suit le dernier changement de l'arbre.

**Cibles finales**, toutes vertes sur l'arbre final :

1. `timeout 5m make test` (dont `test_pico8_map_tables`, `test_pico8_data_init`, `test_pico8_hp`, `test_pico8_stash`) ;
2. `timeout 5m make e2e` : **26 « conforme »** ;
3. `timeout 10m make sanitize-net` : `sanitize: 26 propre(s)` ;
4. `timeout 10m make class-coverage-net` : identique à `tools/class_coverage_net_expected.txt` ;
5. `make data-guard` : code 0 ;
6. `make data-residue` : `stash` 28/4, `type-tag` 445, `ref-audit` 70, `par-nom` 1081, `dispatch` 197, `overrides` 449, `spec` 939, `ob` hors test ponctuel < 2245, 144 fichiers ; écart éventuel expliqué.

### Contrôles du vérificateur

1. **Périmètre du diff** : `git show --stat` liste les 22 chemins de « Empreinte remesurée », et rien d'autre. Aucun `*.sha256`, rien sous `tests/e2e/`, aucun fichier d'événements. `pico8_stash.c/.h` et `test_pico8_stash.c` sont intacts, comme les trois tests du §6.
2. **Module** : `pico8_map_tables.c/.h` n'incluent ni `pico8_ob.h`, ni `pico8_value.h`, ni `pico8_stash.h`, et `sh tools/data_residue.sh --detail` ne les nomme pas. Les tables de tête sont dimensionnées par `PICO8_X_COUNT`. Aucun `PICO8_FIXED_LIT`, aucun flottant, et les `?` sont échappés dans les chaînes.
3. **Lecteurs** : plus aucun `pico8_r()`, `pico8_iz()`, `pico8_hc()` ni inclusion de `pico8_stash.h` hors `pico8_stash.*`, `test_pico8_stash.c` et `test_pico8_map_tables.c` (2 lignes). `pico8_cm` parcourt `pico8_r_table` dans l'ordre. `economy_tiers.c` garde « index < 1 → absent ».
4. **Vues** : `data_init.c` n'a plus ni `pico8_unstash`, ni `nb_mr`, ni `pico8_stash_init`. Chaque vue est construite par une fonction nommée. `pico8_data_shutdown` libère les mêmes tables de tête qu'avant, moins `hc`.
5. **Test ponctuel** : il charge `pico8/map.json` et ne lit aucun `events.json`. Il compare `r`, `iz` et `hc` entrée par entrée et dans l'ordre, et `fd`, `dr`, `nb` par égalité profonde de la vue (avec l'exception `rr` de `fd` et les pointeurs partagés de `nb`). Il ne nomme ni `spec`, ni `overrides`, ni `type-tag`.
6. **TU touchés** : dans `test_pico8_hp.c`, seule la l. 427 et le type de `po` changent, et aucune constante `*_HASH`. Dans `test_pico8_data_init.c`, seules les assertions `hc` (au typé) et les deux `free` changent.
7. **Commentaires** : ceux qui sont ajoutés relèvent des quatre sortes de `CLAUDE.md` (contrats des recherches, sens de `header`, `il_false`, `nk_key`, de `mem` non lu). Aucun récit de chantier, aucun renvoi à D.2 ni au plan dans `src/` ou `tests/`.
8. **Docs** : glossaire à jour pour `r`, `iz`, `hc`, `fd`, `dr`, `nb` et `pico8_jb_row_t` ; note « Découvert à D.2 » en fin de ligne D.2, rien d'autre dans le plan.

### Critères d'échec (traduits en `FAILED:` par le vérificateur de `prd-autopilot`)

**Départ et filet (§7)**

- `FAILED:` si l'arbre de départ n'est pas `3b5af98` (ou un commit de docs au-dessus) propre, ou si `make data-residue` n'y rend pas l'inventaire.
- `FAILED:` si le filet (`make test`, `make e2e`, `make sanitize-net`, `make class-coverage-net`) reste rouge après **3 tentatives**.
- `FAILED:` si un `*.sha256` est modifié ou supprimé, si un fichier de `tests/e2e/` est touché, ou si un e2e est ajouté ou retiré.
- `FAILED:` si `make e2e` ne rend pas **26 « conforme »**, si `make sanitize-net` ne rend pas 26 propres, ou si `make class-coverage-net` diffère de sa référence.
- `FAILED:` si un test de régression du §6 (`gh_row_lq`, `ga_export_panel_by`, `layout_nil_item_draws_nothing`) est modifié.
- `FAILED:` si un TU existant change dans son intention : condition, appel jugé ou valeur attendue, dont une constante `*_HASH`. Sont seuls admis les changements de la section « TU touchés ». Un TU rouge parce que `r`/`iz`/`hc` sont présents avant `pico8_data_init` (H3) est ce cas : l'implémenteur s'arrête et le note.

**Périmètre**

- `FAILED:` si `pico8_stash.c`, `pico8_stash.h` ou `test_pico8_stash.c` changent, ou si `pico8_stash.c` sort d'une liste du `Makefile`.
- `FAILED:` si une ligne `pico8_r()`, `pico8_iz()`, `pico8_hc()`, `pico8_unstash` ou `pico8_stash.h` subsiste hors des trois fichiers de la couche et des 2 lignes du test ponctuel.
- `FAILED:` si le module de données inclut `pico8_ob.h`, `pico8_value.h` ou `pico8_stash.h`, ou porte une ligne d'une catégorie du compteur.
- `FAILED:` si une valeur de `pico8_map_tables.c` a été écrite ou corrigée à la main, au lieu de venir du générateur (le commit décrit le générateur ; le test ponctuel est juge).
- `FAILED:` si un générateur, un script ou une cible `make` de génération est versionné.
- `FAILED:` si le diff convertit une table de D.3 (`df`, `ek`, `es`…), un constructeur (`hp`, `jb`, `js`, `lq`), `menu.c`, `tech_gate.c`, `layout.*` ou `ui_button.*`, ou s'il change la signature de `pico8_data_init` ou de `pico8_ho_apply`.
- `FAILED:` si le test ponctuel rejoue un fichier d'événements, nomme un motif de `spec`, d'`overrides` ou de `type-tag`, ou porte plus de 2 lignes `stash`.
- `FAILED:` si un TROU est ajouté ou modifié, ou si un écart port/cart est corrigé.

**Gardes et comptes**

- `FAILED:` si `make data-guard` n'est pas vert, ou si `DATA_RESIDUE_GUARD`, `tools/data_residue.sh` ou un motif change.
- `FAILED:` si `make data-residue` ne rend pas `stash` 28/4, `type-tag` 445, `ref-audit` 70, `par-nom` 1081, `dispatch` 197, `overrides` 449, `spec` 939 ; ou si `ob` hors test ponctuel n'est pas < 2245.
- `FAILED:` si `make constants-check`, `float-grep` ou `float-guard` rougit, ou si `tools/float_guard.txt` gagne une ligne.

**Preuve, commit et note**

- `FAILED:` si le rapport gcov ou l'une des mutations M1 à M3 manque, ou si une mutation ne rougit pas sans justification au commit.
- `FAILED:` si le travail n'est pas en un seul commit vert.
- `FAILED:` si le budget est dépassé sans note de ce qui n'a pas été lancé, s'il manque le tableau « Runs », ou si le message de commit ne porte pas le pied de `make data-residue` avant et après et la description du générateur.
- `FAILED:` s'il manque la note « Découvert à D.2 », ou si elle omet un pont et son étape, ou le compte `ob` réel.
- `FAILED:` si un commentaire ajouté ou reformulé sort des quatre sortes de `CLAUDE.md`.

## Out of Scope

- **Les tables constantes parsées au démarrage** (D.3) : `df` et sa passe `df_mr`, `ek`, `es`, `ct`, `b_`, `co`, `fb`, `ci`, `ix`/`ha`, `bo`, `ef`/`bz`. `pico8_data_init_rename_pass` reste pour `df`.
- **Convertir les lecteurs des vues** : l'entrée de `hp` (D.6c), celle de `jb` (D.6b), `js._cr_t` (D.4b), `menu` et `tech_gate` (D.7a), et le type des `lq.items` d'exécution (arrêt n° 3 de la liste).
- **Supprimer `pico8_stash.c/.h`, `test_pico8_stash.c`, le test ponctuel, `pico8_r()`/`pico8_iz()`** : D.8.
- **Retirer le paramètre `mem` de `pico8_data_init`** : il touche le §6, laissé à D.7a (dans la note).
- **Libérer les tables imbriquées des vues** : la propriété reste celle d'aujourd'hui (le port fuit par conception, TROU-7).
- `make raylib`, l'appli raylib, l'audio, le rendu, `make e2e-gap` et `tools/line_coverage_reference.txt` (hors suite, décalés par tout diff de `src/`).

## Further Notes

- **Décisions de l'agent à la place de l'utilisateur** (consigne `to-prd` : vérifier modules, tests et découpage avec l'utilisateur), tranchées en `guideline` et marquées (G) plus haut :
  - **modules** : un module de données pur (`pico8_map_tables`), sans dépendance ob ; des vues ob dans `data_init.c`, là où ces tables étaient déjà construites. Règle : « lecteur de feuille → typé ; constructeur ou entité → vue, jusqu'à l'étape qui le convertit ».
  - **générateur** : un programme C jetable dans le scratchpad, compilé contre l'arbre de départ, qui lit ce qu'`ob()` produit. Il est décrit au commit et n'est pas versionné.
  - **forme C** : des entiers `int` (pas de `PICO8_FIXED_LIT`, pas de fixe non constant), les tailles en énumération, un type de rangée de bulle constant (`pico8_jb_row_t`), et les deux formes de `nk` gardées.
  - **tests** : un test ponctuel (données et vues) et deux TU retouchés sans changer leur intention. Pas d'e2e. gcov et trois mutations.
  - **découpage** : un seul commit, sans « Implementation Chunks ».
  - **comptes** : `stash` 28 exact (26 + 2 du test ponctuel) ; `ob` au-dessus du majorant du plan, à cause des ponts ; les autres catégories exactes.
- **Note du plan (G)**, à ajouter en fin de ligne D.2 (le §7 ne garde que ce qui change une étape suivante) :

  > Découvert à D.2 : `pico8_map_tables.c/.h` (données typées générées une fois, sans dépendance ob ; `pico8_r_lookup`, `pico8_iz_lookup`, `pico8_hc_lookup`, `pico8_jb_row_t`). Trois ponts, des vues ob construites par `data_init.c` depuis les tables typées : `pico8_fd()` (fermé par D.6c, entrée de `hp`), `pico8_dr()` (D.6b, entrée de `jb`), `pico8_nb()` (D.4b `js._cr_t`, puis D.7a `menu`/`tech_gate`). `pico8_hp_po` rend `const pico8_hc_entry_t *`. `pico8_data_init(mem)` ne lit plus `mem` (retrait à D.7a). Test ponctuel : `tests/test_pico8_map_tables.c`, 2 lignes `stash` jusqu'à D.8. `ob` après D.2 : N (au lieu de ≤ 2195) ; les majorants `ob` suivants se décalent de N − 2195.

  N est le compte réel, hors test ponctuel.
- **Contrôle d'étape** : un sous-agent frais relit le diff contre ce PRD avant le commit (hors budget de tests).
- **Le message de commit** porte :
  - le périmètre (22 chemins) ;
  - la règle de forme et ses rejets ;
  - le générateur en ≤ 15 lignes ;
  - le pied de `make data-residue` avant et après, et le compte `ob` hors test ;
  - le rapport gcov et les mutations ;
  - le tableau « Runs » résumé ;
  - les réponses à H1 à H15.
- **Surprises pour le pilote** :
  - Le majorant `ob` ≤ 2195 de la ligne D.2 suppose que D.2 convertit aussi les lecteurs de `fd`, `dr` et `nb`. Or ces lecteurs sont des constructeurs et des entités que la liste place à D.4b, D.6b, D.6c et D.7a. Le PRD garde des vues et décale le compte au lieu d'absorber ces étapes.
  - `nb` n'est pas constante à l'exécution : `tech_gate` écrit `e_` dans ses enregistrements. De même, `build_gb` écrit `rr` dans ceux de `fd`. Une table « `static const` » ne peut porter ces écritures qu'à travers la vue.
  - Après D.2, `pico8_stash_init`, `pico8_r()` et `pico8_iz()` ne sont plus appelés que par `test_pico8_stash.c`. Le module reste lié mais mort jusqu'à D.8, comme le prévoit la ligne.
