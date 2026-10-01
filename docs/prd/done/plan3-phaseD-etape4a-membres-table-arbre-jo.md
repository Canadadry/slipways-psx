---
title: "Plan 3, étape D.4a : les membres table de l'arbre `jo` (`jo.nu`/`ka`/`dz`/`cq`/`kw`/`em`/`nk`, `qh.px`/`ba`, `hj.k_`, `ga.k_`, `nr.hz`, `mx.ht`) deviennent des pointeurs vers des données typées — séquences d'entiers, listes d'étiquettes, rangées `pico8_jb_row_t`, enregistrement `pico8_kw_t`, entrée `pico8_df_entry_t` — et leurs défauts table sont générés une fois en `static const` dans `pico8_game_data.c` ; `kw` garde un défaut partagé statique non `const` ; la vue `pico8_df()` de D.3 est fermée (`bw`/`gn` lisent `df` typé) ; `lt`/`he` et `f_` lisent à double forme jusqu'à D.4b — un seul commit, aucun pixel changé"
description: "Après D.3, les treize membres table de l'arbre `jo` pointent encore les tables ob que `ob()` a parsées dans les spécifications de classe (`jo_default_table`, `family_default_table`, `family_trade_default_table`), `kw` est une table ob que `lz` alloue et que `bb`/`mx:kf` mutent, `nk` pointe un enregistrement de la vue `pico8_df()`. D.4a donne à chaque membre une forme C tranchée, génère ses défauts en `static const`, ferme le pont `pico8_df()`, réécrit les deux tests de régression du §6 qui construisent `kw` (preuve rouge), et garde tous les comportements par nom dont un lecteur de `src/` a encore besoin."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Phase D — Données », liste écrite après D.1) :

> - D.4a — `ob`, membres table de l'arbre `jo` : `jo.nu`/`ka`/`dz`/`cq`/`kw`/`em`/`nk`, `qh.px`/`ba`, `hj.k_`, `ga.k_`, `nr.hz`, `mx.ht` → struct ou tableau fixe par membre (troisième puce), forme tranchée par le PRD (le masque de bits de `nu` est la phase 2) ; leurs défauts table (`jo_default_table`, `family_default_table`, `family_trade_default_table`) sont générés `static const` dans la même étape (première puce). Empreinte 34 (22, 12). Compte : `ob` ≤ 2033. §6 : `gh_row_lq` (sa table `kw`, l. 102-120) et `ga_export_panel_by` (sa table `kw`, l. 98-117) construisent le nouveau `kw`, preuve rouge dans ce commit. `class-coverage-net` : oui (init des constructeurs).

En-tête de la liste de D et règles communes, recopiés :

> Liste de D, écrite après D.1 (HEAD 3816427, agent `guideline`), à partir de `make data-residue` et `sh tools/data_residue.sh --detail`. Comptes à l'écriture (lignes/fichiers) : `valeur` 5989/132, `ob` 2245/128, `stash` 56/14, `type-tag` 445/71, `ref-audit` 70/17, `par-nom` 1081/80, `dispatch` 197/59, `overrides` 449/62, `spec` 939/68, **9678 lignes distinctes dans 143 fichiers** ; ceux de la note de D.0 sont d'avant D.1. Chaque ligne nomme la catégorie qu'elle pousse et son « Compte » après elle : « = » attendu exact, « ≤ » majorant par grep (lignes de la catégorie qui nomment les sites des groupes déjà passés, cumulées dans l'ordre de la liste). Le test ponctuel (deuxième puce de D) est hors compte jusqu'à D.8, où il est supprimé ; il ne nomme aucun motif de `spec` ni `overrides`. Une catégorie tombée à 0 entre dans `DATA_RESIDUE_GUARD` dans le même commit.
>
> Empreinte (mesurée à l'écriture, chaque PRD la remesure) : fichiers de `src/*.[ch]` et `tests/*.[ch]` qui portent une ligne de la catégorie du groupe, ou qui nomment ses tables ou membres (majorant : noms courts partagés, `nu`, `k_`, `lq`). Règles communes : un nom nouveau ne correspond à aucun motif figé de la catégorie qu'il remplace (pas de `g_X_spec`/`g_X_defaults`/`pico8_X_default`, et le paramètre `overrides` typé est renommé) ; un lecteur par nom qui survit reste compté par `par-nom` ; un défaut muté (§5, 1a : `pj.ou`, `pj.ea`, `pj.cr`, `ga.by`, `nf.l`, `nf.r`, `jo.kw`) devient un tableau fixe statique non `const`. Un TU construit son état et ne rejoue jamais de fichier d'événements ; un e2e nouveau ajoute de la couverture de lignes en moins de 180 frames ; aucun `*.sha256` ni fichier de `tests/e2e/` ne change. `make sanitize-net` : à chaque étape qui touche `src/`, puisque toutes font passer des tables ob à des données statiques ou locales (§7, « En D et en 2 »).

Notes des deux étapes précédentes, recopiées (fins des lignes D.2 et D.3) :

> Découvert à D.2 : `pico8_map_tables.c/.h` (données typées générées une fois, sans dépendance ob ; `pico8_r_lookup`, `pico8_iz_lookup`, `pico8_hc_lookup`, `pico8_jb_row_t`). Trois ponts, des vues ob construites par `data_init.c` depuis les tables typées : `pico8_fd()` (fermé par D.6c, entrée de `hp`), `pico8_dr()` (D.6b, entrée de `jb`), `pico8_nb()` (D.4b `js._cr_t`, puis D.7a `menu`/`tech_gate`). `pico8_hp_po` rend `const pico8_hc_entry_t *`. `pico8_data_init(mem)` ne lit plus `mem` (retrait à D.7a). Test ponctuel : `tests/test_pico8_map_tables.c`, 2 lignes `stash` jusqu'à D.8. `ob` après D.2 : 2227 (au lieu de ≤ 2195) ; les majorants `ob` suivants se décalent de 32.
>
> Découvert à D.3 : `pico8_game_data.c/.h` devient le module pur des tables constantes (générées une fois, sans dépendance ob ni `init` ; `pico8_int_seq_at`, `pico8_fixed_seq_at` ; un non-entier est l'hexadécimal signé des bits qu'`ob()` rendait, masque `0xffff.fffe` compris, jamais `PICO8_FIXED_LIT`) ; `pico8_jb_item_t` gagne `is_false`. Huit tables lues typées (`es`, `ct`, `b_`, `co`, `fb`, `ek`, `ci`, `ef`). Cinq ponts, des vues ob construites par `data_init.c` (publiques `pico8_ix_view`, `pico8_ha_view`, `pico8_bo_view`, `pico8_bz_view`, appelées par les `*_init`) : `pico8_df()` (fermé par D.4a, `jo.nk` ; `galaxy` à D.7a s'il reste), `pico8_ix()` et `pico8_ha()` (D.6b, clés de `js` et appelants de `menu.c`), `g_bo` (D.6b, `pz` → `js`), `pico8_bz()` (D.6b, entrée de `mh`, puis D.7a `score_panel_display`). `ob` après D.3 : 2208 (au lieu de ≤ 2137) ; les majorants `ob` suivants se décalent de 103 (2208 − 2105).

Le majorant de D.4a devient donc **`ob` ≤ 2136** (2033 + 103), hors test ponctuel. La note de D.3 attribue à D.4a la fermeture du pont `pico8_df()` (« fermé par D.4a, `jo.nk` ; `galaxy` à D.7a s'il reste ») : ce PRD le ferme entièrement, `galaxy` compris (décision plus bas).

Puces de la phase D, recopiées :

> - Les tables constantes parsées au démarrage (`g_es`, `g_ct`, `g_b_`, `g_co`, `g_fb`, `g_ek`, `ci`, menus `g_ix`/`g_ha`, aide `g_bo`, score `g_ef`/`g_bz`, `g_df`) et celles décodées par `unstash` (`g_hc`, `g_fd`, `g_dr`, `g_nb`, `g_r`, `g_iz`) deviennent des tableaux/structs `static const`, **générés une fois** à partir de ce que `ob()`/`unstash` produisent aujourd'hui, puis versionnés.
> - Test ponctuel, supprimé dans le commit qui supprime `pico8_ob.c`/`pico8_stash.c` : chaque table générée est égale, entrée par entrée, à ce que `ob()`/`unstash` produit. Le filet ne voit pas les entrées jamais affichées (aide, score, menus).
> - Les tables ad hoc construites en cours de partie (~68 `ob_table_new()`) deviennent des structs locales ou des tableaux fixes.
> - **Sortie** : `pico8_ob.c`, `pico8_value.c`, `pico8_type_tag.h`, `pico8_ref_audit.h`, `pico8_stash.c` supprimés ; plus aucun `malloc` pour des données. Les entités gardent le leur : c'est Plan 2 §8.

§6, recopié :

> - Les tests **de la couche** (`test_pico8_entity_smoke`, `test_pico8_ob`, `test_pico8_value`, et `test_pico8_list`/`test_pico8_stash` s'ils perdent leur objet) sont supprimés dans le commit qui supprime le code testé.
> - Les tests **de régression de bugs** (17 fichiers : `nil_row`, `gh_row_lq`, `ga_export_panel_by`, `game_end`, …) ne sont **jamais supprimés** par ce plan. Depuis la fusion de master (e2e rapide, #32), seuls `gh_row_lq`, `ga_export_panel_by` et `layout_nil_item_draws_nothing` restent ; les autres ont été remplacés par les e2e de `tests/e2e/`, qui en portent la garde. Chacun est réécrit pour lire la struct, dans le commit qui convertit la classe qu'il touche, et doit toujours échouer si le bug d'origine est réintroduit. Vérifié dans ce commit : bug remis localement, test rouge, correctif restauré.

Règles du §7, recopiées :

> - **Filets depuis la fusion de master (e2e rapide)** : `FRAMES_EVENTS` n'existe plus ; `make e2e` rejoue les 26 e2e de `tests/e2e/` depuis l'écran titre, `make sanitize-net` et `make class-coverage-net` rejouent ce même jeu, `tools/class_coverage_net_expected.txt` y a été reposé à la fusion ; aucun TU ne rejoue de fichier d'événements : un TU construit son état directement, un rejeu est un e2e de `tests/e2e/` (moins de 180 frames, et il doit ajouter de la couverture de lignes). Les mentions de `FRAMES_EVENTS`, `frames-check` et `float-audit` dans les étapes livrées sont historiques.
> - **En D et en 2** (ajouté à la revue de sortie de 1b) : `make class-coverage-net` aux étapes dont le diff touche un constructeur, un appel `pico8_class_coverage_record` ou `KNOWN[]`, et à la sortie de phase ; `make sanitize-net` aux étapes qui changent la durée de vie ou le propriétaire d'une donnée (table ob → locale ou statique, `malloc`/`free` retiré), et à la sortie de phase.

Lecture de ces règles pour D.4a :

- **`make sanitize-net` est exigé** : D.4a change la durée de vie et le propriétaire de toutes les tables du groupe (tables ob du tas, empruntées aux spécifications → données `static const` ; `kw` alloué par `lz` → stockage dans la struct ; vue `df` du tas → table typée statique).
- **`make class-coverage-net` est exigé** : le diff touche les constructeurs (`pico8_jo_struct_init` et les `*_struct_init`/`*_struct_new` des neuf filles, qui posent les défauts ; la clé de construction `k_` de `ga` ; `pm:pq`, qui construit un `ga`). Sortie identique à `tools/class_coverage_net_expected.txt`.
- **§6** : `gh_row_lq` et `ga_export_panel_by` construisent le nouveau `kw` (typé), chacun avec sa preuve rouge dans ce commit. `layout_nil_item_draws_nothing` ne change pas (D.6b).

### État de départ

HEAD `e24a5e5` (branche `claude/plan-pilot-3-7zdft0`), arbre propre. D.3 est livrée (`2ea8951`) et gradée (`e24a5e5`). Décisions de l'utilisateur qui engagent D.4a : **un TU ne rejoue jamais un `events.json`** (il construit son état), **rien de lent**, **aucun `*.sha256` ni fichier de `tests/e2e/` touché**, **un e2e nouveau ajoute de la couverture de lignes en moins de 180 frames**.

**Rien n'a été compilé ni lancé pour ce PRD.** Seuls `make -s data-residue` et `sh tools/data_residue.sh --detail` ont tourné. Le reste vient de `grep` et de la lecture des sources de `e24a5e5`. Tout fait d'exécution est une hypothèse (H1 à H20).

### Inventaire (mesuré sur `e24a5e5`)

**`make data-residue`** :

```
valeur	6028	133
ob	2266	127
stash	28	4
type-tag	445	71
ref-audit	70	17
par-nom	1081	80
dispatch	197	59
overrides	449	62
spec	939	68
résidu D : 9685 ligne(s), 142 fichier(s)
```

`ob` 2266 = **2208 hors test ponctuel** + 58 lignes de `tests/test_pico8_map_tables.c` : c'est le chiffre de la note de D.3.

**`ob` par fichier du groupe** (`--detail`) : `planet.c` 49, `planet.h` 13, `planet_family.c` 93, `planet_family.h` 11, `planet_family_trade.c` 52, `planet_family_trade.h` 6, `planet_display.c` 41, `planet_trade.c` 24, `research_display.c` 48, `economy_score.c` 19, `economy_formulas.c` 22, `economy_tiers.c` 16, `data_init.c` 98, `data_init.h` 10, `galaxy.c` 9, `gen.c` 3, `gen.h` 2, `entity.c` 2, `particle.c` 13 ; TU : `test_pico8_ui` 114, `test_pico8_ga` 44, `test_pico8_hp` 36, `test_pico8_mx` 21, `test_pico8_jo` 18, `test_pico8_hj` 15, `test_pico8_pm` 15, `test_pico8_qh` 12, `test_pico8_nr` 11, `test_pico8_fr` 10, `test_pico8_os` 10, `test_pico8_nl` 9, `test_pico8_ga_export_panel_by` 9, `test_pico8_planet_gh_row_lq` 6, `test_pico8_jo_spec_defaults` 2.

**Ce que les spécifications donnent aux membres** (chaînes de `e24a5e5`, identiques au cart) :

| Membre | Classe(s) | Valeur parsée | Genre des entrées |
|---|---|---|---|
| `nu` | `jo`, `nl`, `os`, `pm`, `mx` | `o("jo","np","prober")` (une table par spécification) | chaînes |
| `nu` | `qh` / `hj` / `nr` / `ga` | `o("qh","jo","np","prober")` / `o("hj","jo","np","prober")` / `o("nr","np","prober")` / `o()` vide | chaînes |
| `ka` | `jo` | `o(2,3,9,12)` | entiers |
| `dz` | `jo` | `o(o("unhappy",c=8), o("content",c=3), o("prosperous",c=9), o("rich",c=12))` | 4 rangées « chaîne + `c` » |
| `cq` | `jo` | `o(12,36,24,25)` | entiers |
| `kw` | `jo` | `o(mu=0)`, **muté en place** (`bb` écrit `mu`, `mx:kf` écrit `qe`) jusqu'à ce que `lz` en pose un neuf `{qe=hp.o si présent, mu=0}` | fixes nommés |
| `em` | `jo` | `o(f,3,0,-1)` | faux puis entiers |
| `nk` | `jo` | aucun défaut ; un enregistrement de `df` posé par `jo:hu` (`df[hp.go]`) ou `jo:hh` (`df.ll` ou le tirage `jo.oc()`) | enregistrement |
| `px` | `qh`, `nl`, `os`, `hj`, `pm`, `mx` | `o("laboratory",c=13)`, `o("trading hub",c=13)`, `o("processor",c=13)`, `o("ascension nr",c=13)`, `o("protostar",c=13)`, `o("synthesizer",c=13)` | une rangée « chaîne + `c` » |
| `ba` | `qh`, `nl` | `o(28,18,20,21,22,48)`, `o(28,48,18,20,21,22)` | entiers |
| `k_` | `hj`, `pm`, `mx`, `ga` | `o(1,13,12,12,12,13,13,5,1,1,1)`, `o(1,2,4,9,10,7)`, `o(1,1,5,5,3,3,3,5,5,1)`, `o(1,2,4,9,9,10,10,7,7,7,7,7)` ; clé de construction de `pm:pq` pour son `ga` : `o(1,2,8,14,14,14,14,8,8,2,2,1,1)` | entiers |
| `hz` | `nr`, `fr` | `o(170,170,170,168)`, `o(78,78,78,76)` | entiers |
| `ht` | `mx` | `o(28,20,21,22)`, remplacé par `o()` neuf quand `mx:kf` accepte une ressource | entiers |

**Lecteurs et écrivains** (hors module propriétaire ; `grep -rnE '(->|\.)X\b|"X"'`, noms partagés filtrés à la main) :

| Membre | Écrit par | Lu par (`src/`) | Lu **par nom** dans `src/` |
|---|---|---|---|
| `nu` | `*_struct_init` seulement | — | `pico8_lt`, `pico8_he` (`entity.c:215`, `:245`), **pour toutes les entités**, dont `mh`, `g_`, `jc`, `mz` (D.4b) |
| `ka` | init | `planet_display.c:490` (`ka[mi]` par `pico8_jo_ka()`) | aucun |
| `dz` | init | `planet_display.c:156-157` (`jo:gh`, `dz[mi]` par `pico8_jo_dz()`, remis à `pz` → clé `lq` d'un `js`) | aucun |
| `cq` | init | `planet_display.c:261-262` (`jo:gh`, `gu(cq, kw.qe)`) | aucun |
| `kw` | init, `lz` (`planet.c:863-868`), `bb` (`economy_tiers.c:231-234`, écrit `mu` à travers la lecture par nom), `mx:kf` (`planet_family_trade.c:490-493`, écrit `qe`) | `jo:hb` (`planet_display.c:83-91`), `jo:gh` (`:201-207`, `:243-252`, **poussé tel quel comme item de rangée `lq`** `:275`), `jo:ib` (`planet_trade.c:241-250`) | `economy_score.c:190`, `economy_formulas.c:109`, `economy_tiers.c:231` |
| `em` | init | `research_display.c:250-251` (`jo.ex`, **poussé comme item de rangée** par `pico8_jo_em()`) | aucun |
| `nk` | `jo:hu` (`planet.c:966-969`), `jo:hh` (`:1013-1021`) | `planet_display.c:477` (`nk.s`), `jo:hh` (`nk.d` → `ep`, `nk.me` → `ks`) | `research_display.c:325` (`bj` : `nk.px`) |
| `px` | init | — | `planet_display.c:153` (`jo:gh` : icône, remise à `pz` → clé `lq` d'un `js`) |
| `ba` | init | — | `planet_trade.c:196` (`jo:cn` : `gu(ba, t.qe)`) |
| `k_` | init ; clé `k_` de `ga` (`planet_family.c:1209-1210`) depuis `pm:pq` (`:753-757`) | `ga:kj` lit la classe `g_ga_k_` (`:1108`, TROU-21) | `particle.c:125` (`f_`), **aussi pour `g_`/`fv`** (D.4b) |
| `hz` | init | `nr:hb` (`planet_family_trade.c:143-147`) | aucun |
| `ht` | init, `mx:kf` (`:496`, `pico8_ob_table_new()`) | `mx:kf` (`:484-487`, `gu(ht, qe)`) | aucun |

Aucune écriture par nom de ces membres n'existe dans `src/` (`grep 'set_field(' src` : aucun appel à nom non littéral, aucun littéral de ces noms). Les branches `kw`/`nk` de `pico8_jo_set_field` et `ht` de `pico8_mx_set_field` ne sont atteintes que par les TU (`test_pico8_ui` `write_by_name`, `test_pico8_ga:535`, `test_pico8_mx:674`, `test_pico8_ui:3477`).

**Autres faits qui engagent la forme :**

- Les défauts table ne sont lus que par `jo_default_table` (`planet.c:69`), `family_default_table` (`planet_family.c:1232`), `family_trade_default_table` (`planet_family_trade.c:657`) : 29 paires de lignes `found = pico8_X_default("…", &v); g_X_… = X_default_table(found, v);` (jo 6, qh 3, nl 3, os 2, hj 3, pm 3, ga 2, nr 2, fr 1, mx 4).
- `kw` : la table de la spécification est **partagée** par toutes les planètes nues et mutée en place (`test_pico8_jo:491-504` le fige) ; chaque `pico8_jo_init()` reparse la spécification, donc repart de `{mu=0}`.
- `jo:gh` pousse `kw` **lui-même** comme item de la rangée « exports:/makes: » ; `lq:mn` (`layout.c:90-128`) lit un item REF comme une table ob : `qe` vrai → `{qe,5}`, sinon index 1..4. `pico8_js_lq_arg_table` (`ui_button.c:28-46`) **avorte** sur un REF non tagué : `px` et `dz[mi]` doivent arriver à `js` en table ob.
- `pico8_gn` (`gen.c:47-82`) lit le poids `r` de chaque entrée de `hr` par `pico8_ob_table_get_name` ; son seul appelant est `pico8_bw` (`galaxy.c:118-129`), qui lui passe les positionnelles de la vue `pico8_df()`. Les autres lecteurs de `pico8_df()` : `planet.c:966`, `:1013`, `tests/test_pico8_ui.c:1661`, `tests/test_pico8_hp.c:521`, `tests/test_pico8_map_tables.c:444`.
- `pico8_type_tag_of` lit les 4 premiers octets ; une struct typée dont le premier membre est un compte (≤ 13) lit `UNKNOWN`.
- Les TU de classe (`test_pico8_jo`, `qh`, `nl`, `os`, `hj`, `pm`, `ga`, `nr`, `fr`, `mx`) comparent les membres table par nom à `class_ref(pico8_X_default, nom)` (identité avec la table de la spécification) et plient ces identités, la présence et le contenu de `kw` dans leurs constantes `*_HASH`. `test_pico8_ui` fait de même (`JO_MEMBERS_HASH`, `JO_WRITTEN_HASH`) ; `test_pico8_jo_spec_defaults` plie `same_value(spec, struct)` (identité des REF) dans `STRUCT_HASH`, « figé, à ne pas reprendre ».
- `pico8_game_data.c/.h` est pur depuis D.3 (ni `pico8_ob.h`, ni `pico8_value.h`) et est lié par le test ponctuel (`DATA_ENGINE_SRCS`), comme `data_init.c`.

## Solution

**Un seul commit**, sans aucun pixel changé :

1. **Chaque membre table de l'arbre `jo` devient un pointeur vers une donnée typée** (forme tranchée membre par membre plus bas) : séquence d'entiers `pico8_int_seq_t`, liste d'étiquettes `pico8_str_seq_t`, rangée `pico8_jb_row_t` (de D.2), enregistrement `pico8_kw_t`, entrée `pico8_df_entry_t` (de D.3).
2. **Les défauts table sont générés une fois en `static const`** dans `pico8_game_data.c/.h`, un objet nommé par table et par classe, par un générateur jetable qui lit ce qu'`ob()` produit des spécifications de départ. `jo_default_table`, `family_default_table`, `family_trade_default_table`, les 29 paires de lectures et les statiques `g_X_nu`/`px`/`ba`/`k_`/`hz`/`ht`/`ka`/`dz`/`cq`/`kw`/`em` disparaissent. Les spécifications restent parsées pour leurs autres clés (D.5c).
3. **`kw` garde son défaut partagé et muté** : un `pico8_kw_t` statique **non `const`** de `planet.c` (règle du §5, 1a), remis à la valeur générée par chaque `pico8_jo_init()` ; `lz` pose le `kw` propre de la planète dans la struct, sans allocation.
4. **Le pont `pico8_df()` est fermé** : `jo:hu`, `jo:hh`, `bw` et `gn` lisent `pico8_df_table` ; la vue et son constructeur disparaissent de `data_init.c/.h`.
5. **Pendant la coexistence**, la lecture par nom d'un membre converti rend un REF vers la donnée typée ; ses lecteurs par nom de `src/` sont convertis à cette forme, sauf `lt`/`he` (`nu`) et `f_` (`k_`), partagés avec les classes de D.4b, qui lisent **à double forme** jusqu'à D.4b. Les consommateurs qui exigent une table ob (rangée `lq` de `layout`, clé `lq` de `js`) reçoivent une vue ob construite à l'appel depuis la donnée typée (pont jusqu'à D.6b/D.7a).
6. **`gh_row_lq` et `ga_export_panel_by` construisent le `kw` typé**, preuve rouge dans ce commit ; les TU touchés lisent les structs, **constantes de hachage inchangées**.
7. **Le test ponctuel est étendu** (chaque défaut généré contre `pico8_ob(<spécification de départ>)`, `df` typé contre la table parsée après sa passe) ; le glossaire et une note « Découvert à D.4a » du plan suivent.

**Sortie de l'étape**, constatée par commande :

- `make test` vert ;
- `make e2e` : 26 conformes ;
- `make sanitize-net` : 26 propres ;
- `make class-coverage-net` : identique à sa référence ;
- `make data-guard` : code 0 ;
- `make data-residue` : `stash` 28/4, `overrides` 450, `ob` hors test ponctuel < 2208 et attendu ≤ 2136 ; les autres selon « Effet sur `make data-residue` » ;
- `grep -rnE '\b(jo|family|family_trade)_default_table\b|\bpico8_df\(\)|data_init_df_view|\bg_(jo|qh|nl|os|hj|pm|ga|nr|fr|mx)_(nu|px|ba|k_|hz|ht|ka|dz|cq|kw|em)\b' src tests --include='*.[ch]'` ne rend rien ;
- `grep -nE 'pico8_ob_table_t \*(nu|ka|dz|cq|kw|em|nk|px|ba|k_|hz|ht);' src/pico8_planet.h src/pico8_planet_family.h src/pico8_planet_family_trade.h` ne rend rien.

## User Stories

1. En tant que mainteneur du port, je veux que les membres table de l'arbre `jo` aient un type C qui dit ce qu'ils contiennent, afin de lire `jo.ka[mi]` ou `mx.ht` sans passer par une table ob.
2. En tant que mainteneur, je veux que les défauts table de chaque classe soient des données `static const` versionnées dans `pico8_game_data.c`, afin de les relire dans un diff sans exécuter `ob()`.
3. En tant que mainteneur, je veux que chaque classe garde sa propre donnée de défaut, même quand son contenu égale celle d'une autre (`nu` de `nl`, `os`, `pm`, `mx`), afin que « le membre est le défaut de telle classe » reste une question à réponse exacte, comme avec les tables de spécification.
4. En tant que lecteur de `pico8_lt`, je veux que les étiquettes `nu` d'une planète soient une séquence ordonnée de chaînes, afin que `cg` soit rempli dans le même ordre qu'avant.
5. En tant qu'auteur de la phase 2, je veux que `nu` ne soit pas encore un masque de bits, afin que ce choix reste celui de la composition.
6. En tant que lecteur de `jo:gh`, je veux tester `kw.qe ∈ cq` par une aide typée, afin que « makes: » et « exports: » gardent leur choix.
7. En tant que gardien du comportement, je veux que les planètes nues partagent toujours un seul `kw` muté en place par `bb` et `mx:kf`, afin que l'aliasing du cart soit préservé.
8. En tant que gardien du comportement, je veux que ce `kw` partagé reparte de `{mu=0}` à chaque `pico8_jo_init()`, afin qu'une nouvelle partie ne voie pas le `mu` de la précédente, comme le reparse de la spécification le garantissait.
9. En tant que gardien de la mémoire, je veux que `lz` pose le `kw` propre d'une planète sans allocation, afin de ne pas remplacer une table ob par un autre `malloc` de données.
10. En tant que lecteur de `jo:hu` et `jo:hh`, je veux trouver un enregistrement de `df` par son nom `px` dans la table typée, afin que `df[hp.go]` et `df.ll` ne passent plus par une vue.
11. En tant que lecteur de `bw`, je veux que le générateur `gn` tire des entrées typées de `df` pondérées par leur `r`, afin que le pont `pico8_df()` disparaisse entièrement.
12. En tant que gardien du comportement, je veux que le tirage `gn` reçoive les mêmes entrées, dans le même ordre, avec les mêmes poids, afin que la galaxie générée et les aléas soient identiques.
13. En tant que lecteur de `planet_display.c` et `research_display.c`, je veux lire `nk.s` et `nk.px` sur un `pico8_df_entry_t`, afin de ne plus traverser une table ob pour un champ.
14. En tant que lecteur des trois modules d'économie, je veux lire `kw.qe` et `kw.mu` sur un `pico8_kw_t` obtenu par la même lecture par nom qu'avant, afin que la conversion ne change ni l'ordre ni le nombre des lectures par nom.
15. En tant que lecteur de `jo:cn`, je veux tester `t.qe ∈ ba` sur une séquence typée, afin de garder l'acceptation des ressources par `qh` et `nl`.
16. En tant que lecteur de `nr:hb`, je veux lire `hz[#oe+1]` par l'aide 1-based de D.3, afin qu'un index hors séquence reste « absent ».
17. En tant que lecteur de `mx:kf`, je veux que l'acceptation d'une ressource remplace `ht` par une séquence vide statique, afin que « `ht` n'accepte plus rien » ne demande pas d'allocation.
18. En tant que gardien de TROU-21, je veux que `ga:kj` lise encore la palette de la classe `ga`, jamais celle de l'instance, afin que l'écart connu reste tel quel.
19. En tant que lecteur de `pm:pq`, je veux que la palette de l'éclat soit une donnée générée passée par référence typée dans la clé `k_`, afin que le membre `ga.k_` puisse être typé sans convertir toutes les clés de construction (D.6a).
20. En tant qu'auteur de D.4b, je veux que `lt`/`he` et `f_` lisent à double forme (table ob ou donnée typée), afin de n'avoir qu'à retirer la branche ob quand `mh.nu`, `g_.nu`/`k_`, `jc.nu` et `mz.nu` seront typés.
21. En tant que gardien du comportement, je veux que `layout` et `js` reçoivent toujours des tables ob de même contenu pour `px`, `dz[mi]`, `em` et l'item `kw`, afin que les rangées affichées soient identiques.
22. En tant qu'auteur de D.6b et de D.7a, je veux que ces vues soient construites par une seule fonction publique nommée (`pico8_jb_row_view`) et par une aide locale pour `kw`, afin de savoir quoi retirer.
23. En tant que gardien du §6, je veux que `gh_row_lq` et `ga_export_panel_by` construisent le `kw` typé et échouent toujours si leur bug d'origine revient, afin qu'aucune garde de régression ne se perde.
24. En tant que vérificateur, je veux un test ponctuel qui compare chaque défaut généré à `pico8_ob()` sur les spécifications exactes de départ, et `df` typé à la table parsée après sa passe, afin de prouver que la génération n'a perdu ni une entrée, ni un ordre, ni un genre.
25. En tant que vérificateur, je veux que les aides nouvelles (`pico8_int_seq_has_fixed`, `pico8_df_lookup`) soient testées sur leurs bornes, afin de savoir qu'elles gardent le sens Lua.
26. En tant que gardien des TU, je veux que les TU de classe, `test_pico8_ui`, `test_pico8_hp` et `test_pico8_jo_spec_defaults` lisent les structs mais gardent leurs constantes de hachage, afin de prouver que les identités, présences et contenus qu'ils figeaient n'ont pas bougé.
27. En tant que gardien du compteur, je veux qu'aucun nom nouveau ne corresponde à un motif figé (`*_default*`, `g_X_spec`, `*_field*`, `pico8_ob*`), afin que les comptes mesurent la conversion.
28. En tant que gardien des filets, je veux `make e2e` à 26 conformes, `make sanitize-net` à 26 propres et `make class-coverage-net` identique, afin de prouver qu'aucun pixel, aucun accès mémoire ni aucune instanciation n'a changé.
29. En tant que lecteur de `docs/glossaire-noms.md`, je veux que les lignes des treize membres, de `df` et des types nouveaux nomment les noms C d'après D.4a, afin de retrouver un membre depuis son nom de cart.
30. En tant que `plan-pilot`, je veux une note « Découvert à D.4a » qui donne les ponts restants et l'étape qui ferme chacun, et le compte `ob` réel, afin que D.4b et les suivantes partent de faits.
31. En tant que `plan-pilot`, je veux que les spécifications (D.5c), les autres clés de construction (D.6a), les consommateurs ob de mise en page (D.6b, D.7a) et les membres de D.4b restent hors de ce commit, afin que chaque étape reste une étape.

## Implementation Decisions

Pas d'humain dans la boucle. Le skill `to-prd` demande de faire valider par l'utilisateur les modules, les tests et le découpage. Ces choix sont tranchés ici comme le ferait un agent `guideline` (hypothèses explicites, simplicité, changement chirurgical, critères vérifiables), et marqués **(G)**. Les rejets sont notés à chaque décision.

### Les types (G), dans `pico8_game_data.h`

Forme retenue (écrite à la main dans le `.h`, comme à D.3) :

```c
typedef struct { int count; const int *items; } pico8_int_seq_t;          /* items[0..count-1] ; items NULL si count 0 */
typedef struct { int count; const char *const *items; } pico8_str_seq_t;
typedef struct { int has_qe; pico8_fixed_t qe; pico8_fixed_t mu; } pico8_kw_t; /* qe n'a de sens que sous has_qe */
enum { PICO8_JO_DZ_COUNT = 4 };

/* 1 si v égale pico8_fixed_from_int d'un élément : `gu` du cart sur des entiers. */
int pico8_int_seq_has_fixed(const pico8_int_seq_t *seq, pico8_fixed_t v);
/* L'entrée de df dont px vaut `px`, NULL sinon : `df[px]` après la passe px/s/r/d. */
const pico8_df_entry_t *pico8_df_lookup(const char *px);
```

- `pico8_int_seq_t` et `pico8_str_seq_t` portent leur compte : un membre doit savoir combien d'entrées il a, ce que les tableaux nus de D.3 laissaient à une constante de module. L'indexation 1-based reste `pico8_int_seq_at(seq->items, seq->count, i, &out)` (D.3) : pas d'aide de plus.
- `pico8_kw_t` : `qe` est toujours un fixe en jeu (H4) ; `mu` est toujours présent (défaut `{mu=0}`, `lz` le pose). Pas de `has_mu`.
- Le module reste **pur** : aucune de ces déclarations ne demande `pico8_value.h`. `pico8_int_seq_has_fixed` prend un fixe ; l'appelant qui tient un `pico8_value_t` teste `kind == PICO8_VALUE_FIXED` d'abord (un non-fixe n'est dans aucune séquence d'entiers, comme `pico8_value_equal` le disait pour `gu`).
- Rejeté : **un `pico8_value_t qe`** dans `pico8_kw_t`, pour garder la chaîne `"x"` des tests. Seuls deux TU écrivaient une chaîne ; aucun écrivain du jeu (H4).
- Rejeté : **des tableaux inline dans la struct** (`int ka[4]; int ka_count;`). Les défauts sont constants et partagés par classe ; un pointeur garde « le membre est le défaut de telle classe » vérifiable par égalité de pointeurs, ce que les TU figent.

### Forme par membre (G)

| Membre | Forme C du membre | Défaut (généré, `pico8_game_data.c`) | Écritures après D.4a |
|---|---|---|---|
| `jo.nu` et le `nu` des filles | `const pico8_str_seq_t *nu` | `pico8_jo_nu_tags`, `pico8_qh_nu_tags`, `pico8_nl_nu_tags`, `pico8_os_nu_tags`, `pico8_hj_nu_tags`, `pico8_pm_nu_tags`, `pico8_ga_nu_tags` (compte 0), `pico8_nr_nu_tags`, `pico8_mx_nu_tags` (`fr` garde celui de `nr`) | `*_struct_init` |
| `jo.ka` | `const pico8_int_seq_t *ka` | `pico8_jo_ka_seq` | init |
| `jo.dz` | `const pico8_jb_row_t *dz` (`PICO8_JO_DZ_COUNT` rangées) | `pico8_jo_dz_rows[PICO8_JO_DZ_COUNT]` | init |
| `jo.cq` | `const pico8_int_seq_t *cq` | `pico8_jo_cq_seq` | init |
| `jo.kw` | `pico8_kw_t *kw`, plus un membre de stockage `pico8_kw_t kw_own` | `pico8_jo_kw_start` (`const`, `{0, 0, 0}`) recopié dans le statique **non `const`** `g_jo_kw_shared` de `planet.c` par chaque `pico8_jo_init()` ; les planètes nues pointent `&g_jo_kw_shared` | `lz` : `kw_own = {has_o, o, 0}`, `kw = &kw_own` ; `bb`, `mx:kf` : à travers le pointeur ; écrivain par nom (TU) |
| `jo.em` | `const pico8_jb_row_t *em` | `pico8_jo_em_row` (faux, 3, 0, -1) | init |
| `jo.nk` | `const pico8_df_entry_t *nk` | aucun (NULL) | `jo:hu` (`pico8_df_lookup(hp.go)`), `jo:hh`, écrivain par nom (TU) |
| `qh.px` | `const pico8_jb_row_t *px` | `pico8_qh_px_row`, `pico8_nl_px_row`, `pico8_os_px_row`, `pico8_hj_px_row`, `pico8_pm_px_row`, `pico8_mx_px_row` | init |
| `qh.ba` | `const pico8_int_seq_t *ba` | `pico8_qh_ba_seq`, `pico8_nl_ba_seq` | init |
| `hj.k_` | `const pico8_int_seq_t *k_` | `pico8_hj_k__seq`, `pico8_pm_k__seq`, `pico8_mx_k__seq` | init |
| `ga.k_` | `const pico8_int_seq_t *k_` | `pico8_ga_k__seq` ; clé de `pm:pq` : `pico8_pm_ga_k__seq` | init, clé de construction `k_` |
| `nr.hz` | `const pico8_int_seq_t *hz` | `pico8_nr_hz_seq`, `pico8_fr_hz_seq` | init |
| `mx.ht` | `const pico8_int_seq_t *ht` | `pico8_mx_ht_seq` ; après acceptation, `g_mx_ht_spent` (`static const`, compte 0) de `planet_family_trade.c` | `mx:kf`, écrivain par nom (TU) |

- **`nu` (question de la ligne)** : une séquence ordonnée de chaînes, par classe. Le masque de bits est la phase 2 ; D.4a ne change ni l'ordre d'enregistrement dans `cg` ni la liste des étiquettes.
- **`kw` : pointeur + stockage `kw_own`** (G). L'aliasing du cart est gardé : une planète nue partage `g_jo_kw_shared` avec toutes les autres, `bb` et `mx:kf` y écrivent comme ils écrivaient dans la table de la spécification ; `lz` donne à la planète le sien. `kw_own` n'est pas un champ par nom (ni `field`, ni `set_field` ne le nomment).
  - Rejeté : **`malloc` d'un `pico8_kw_t` par `lz`** (lecture littérale de `{}`). Aucun détenteur de l'ancien `kw` ne survit à `lz` depuis que `jo:gh` pousse une vue (plus bas) ; le stockage dans la struct évite une allocation de données de plus.
  - Rejeté : **`kw` inline sans pointeur**. Le partage des planètes nues, figé par `test_pico8_jo:491-504`, serait perdu.
- **`em`, `dz`, `px`** : des rangées de mise en page, exactement la forme `pico8_jb_row_t` de D.2 (chaîne, nombre, faux, `c`).
- **Noms** : aucun ne correspond à un motif figé (`_default`, `_defaults`, `_spec`, `_field`, `_take_`, `pico8_ob`). Suffixe de forme (`_tags`, `_seq`, `_row`, `_rows`, `_start`) plutôt que le nom nu, qui heurterait les accesseurs existants (`pico8_jo_ka()`, `pico8_jo_hf()`).
- **Accesseurs** : `pico8_jo_ka()`, `pico8_jo_cq()` rendent `const pico8_int_seq_t *`, `pico8_jo_dz()` et `pico8_jo_em()` `const pico8_jb_row_t *`, **jamais NULL** (données statiques, présentes avant `pico8_jo_init()` : H8). Les gardes `!= NULL` de leurs lecteurs tombent. Nouvel accesseur `pico8_kw_t *pico8_jo_shared_kw(void)` : le `kw` des planètes nues, pour les TU (contrat : « remis à `{mu=0}` par `pico8_jo_init()` »).

### Le générateur (G) : jetable, en C, sur l'arbre de départ

- **Programme C de l'implémenteur, dans son scratchpad, jamais versionné**, compilé contre `$(DATA_ENGINE_SRCS)` de `e24a5e5`, **avant** toute modification : `pico8_pi_init()`, puis `pico8_ob(s, NULL)` sur les dix chaînes de spécification (`planet.c` : `jo` ; `planet_family.c` : `qh`, `nl`, `os`, `hj`, `pm`, `ga` ; `planet_family_trade.c` : `nr`, `fr`, `mx`) et sur la chaîne de clés de `pm:pq` (`planet_family.c:753-757`, sans ses `defaults`), **copiées octet pour octet**. Il émet, pour chaque clé table de l'inventaire, l'objet `static const` de la section précédente.
- **Il échoue fort** si une hypothèse de forme tombe (H2) : positionnelles non contiguës depuis 1 ; nommée présente dans une table d'entiers ; non-entier ; `nu` avec un non-chaîne ; rangée de `px`/`dz`/`em` qui n'est pas « positionnelles (chaîne, entier ou faux), puis `c` seul » ; `kw` autre que `{mu=0}`.
- Sortie collée dans `pico8_game_data.c`, mise en forme à la main (≤ 120 colonnes), sans valeur éditée. Les `?` sont échappés (`\?`), comme à D.3 (aucun n'est attendu).
- Le message de commit décrit le générateur en ≤ 15 lignes.
- Rejeté : **écrire ces trente petites tables à la main**. La puce dit « générés une fois », et le test ponctuel juge de toute façon.

### Coexistence : ce que rendent `field`/`set_field` (G)

Règle : **la lecture par nom d'un membre converti rend `pico8_value_ref((void *)membre)`** — un REF non tagué vers la donnée typée, ou 0 (absent) si le membre est NULL. La présence ne change donc pour aucun nom, seul le genre du REF change (table ob → donnée typée). Les lecteurs par nom de `src/` restent des lecteurs par nom (comptés par `par-nom`) et castent vers le type du membre :

| Lecteur par nom (`src/`) | Après D.4a |
|---|---|
| `economy_score.c:190` (`kw`) | lecture par nom puis `const pico8_kw_t *` ; `qe` comparé au fixe de `r.w` si `has_qe` (TROU-3 : la comparaison par contenu n'a plus d'objet sur deux fixes, le commentaire nu reste sur la ligne) |
| `economy_formulas.c:109` (`bx_visit`, `kw`) | `field_ref_table` n'a plus que cet appelant : il devient une lecture de `const pico8_kw_t *` ; `qe` absent → retour ; la clé `set_by_value` est `pico8_value_fixed(kw->qe)` |
| `economy_tiers.c:231` (`bb`, `kw`) | écrit `((pico8_kw_t *)ref)->mu = base + jh` |
| `planet_trade.c:196` (`jo:cn`, `ba`) | `const pico8_int_seq_t *` ; `accepted = t_qe` fixe `&&` `pico8_int_seq_has_fixed(ba, t_qe.as.fixed)` |
| `research_display.c:325` (`bj`, `nk`) | `const pico8_df_entry_t *` ; `px_str = nk->px` |
| `planet_display.c:153` (`jo:gh`, `px`) | `const pico8_jb_row_t *` ; l'icône remise à `pz` est `pico8_value_ref(pico8_jb_row_view(px))` (vue, plus bas) |
| `entity.c:215`, `:245` (`lt`, `he`, `nu`) | **double forme** : tag `PICO8_TYPE_TAG_OB_TABLE` → branche d'aujourd'hui (`mh`, `g_`, `jc`, `mz`, jusqu'à D.4b) ; sinon `const pico8_str_seq_t *`, chaque chaîne enregistrée/désenregistrée dans le même ordre |
| `particle.c:125` (`f_`, `k_`) | **double forme** : même règle (`g_`, `fv` jusqu'à D.4b ; `hj`, `pm`, `mx`, `ga` typés) ; la couleur de l'index `idx` est lue par `pico8_int_seq_at` dans la branche typée |

- **Écrivains par nom** (`pico8_jo_set_field` `kw`/`nk`, `pico8_mx_set_field` `ht`, atteints par les TU seulement) : l'ensemble des noms ne change pas (17 pour `jo`, lecture de 1b.23a2). Chaque branche accepte **nil ou un REF non tagué** (limite `ref:autre` de 1b.23b : un REF non tagué n'est pas plus identifié que pour un vecteur) ; un REF tagué (table ob, liste, struct) ou un autre genre avorte, message au format actuel (« a write neither nil nor … does not fit the member's type -- stop, do not widen the member »). `jo_take_table` est remplacée par `jo_take_kw` et `jo_take_nk` ; `family_trade_take_table` par une aide de séquence du même préfixe.
- **Clé de construction `k_` de `ga`** : même règle que l'écrivain (nil ou REF non tagué → `const pico8_int_seq_t *`), par une aide de séquence qui remplace `family_take_table`. **`pm:pq`** retire `k_=o(1,2,8,14,14,14,14,8,8,2,2,1,1),` de sa chaîne `ob` et pose la clé par `pico8_ob_table_set_name(overrides, "k_", pico8_value_ref((void *)&pico8_pm_ga_k__seq))` après le parse (H10 : l'ordre des clés ne change aucun membre). C'est la seule clé de construction touchée ; les autres restent à D.6a.
- **Lecteurs typés directs** (pas par nom) : `jo:hb`, `jo:gh`, `jo:ib` lisent `self->kw` (`qe` → `pico8_value_fixed(qe)` si `has_qe`, sinon nil, là où un `pico8_value_t` est attendu) ; `jo:gh` teste `makes` par `pico8_int_seq_has_fixed(pico8_jo_cq(), …)` ; `jo:kj` (`planet_display.c:490`) lit `ka[mi]` par `pico8_int_seq_at` ; `planet_display.c:477` lit `self->nk->s` ; `nr:hb` lit `hz[#oe+1]` par `pico8_int_seq_at` ; `ga:kj` lit `pico8_ga_k__seq` (TROU-21, la classe, jamais l'instance) ; `mx:kf` teste `qe ∈ ht` par `pico8_int_seq_has_fixed`, écrit `kw->qe`/`has_qe` puis `ht = &g_mx_ht_spent`.
- Les aides `positional_values`/`pico8_jo_positional_values` de `planet_display.c`, `planet_trade.c` et `planet_family_trade.c` qui n'ont plus d'appelant sont supprimées (`-Werror`).
- Rejeté : **`field` rend 0 pour les membres sans lecteur par nom dans `src/`** (`ka`, `dz`, `cq`, `em`, `hz`, `ht`). Les TU plient la présence de ces noms (`has(e, "ka")`) ; la faire tomber changerait des constantes pour rien, et l'ensemble des champs par nom est l'objet de D.9.
- Rejeté : **une vue ob persistante par classe pour `nu` et `k_`**, rendue par `field`. Il faudrait associer chaque donnée typée à sa vue (neuf classes), avec une durée de vie à gérer ; la double forme de `lt`/`he`/`f_` est deux branches que D.4b retire. Précédent : les lecteurs à double forme de 1a.16.
- Rejeté : **convertir `lt`/`he`/`f_` à la seule forme typée**. `mh.nu`, `g_.nu`/`k_`, `jc.nu`, `mz.nu` sont D.4b.

### Vues ob pour les consommateurs de mise en page (G)

- `data_init.c` publie sa vue de rangée de D.2/D.3 sous le nom **`pico8_jb_row_view`** (déclarée dans `data_init.h`, même corps que `data_init_row_view`) : table ob neuve, positionnelles puis `c`, faux rendu `pico8_value_bool(0)` ; l'appelant possède la table de tête, jamais les rangées imbriquées.
- **`jo:gh`** : l'icône `px` et `dz[mi]` (`1 ≤ mi ≤ PICO8_JO_DZ_COUNT`) partent vers `pz` comme `pico8_jb_row_view(...)`, construite à l'appel ; l'item `kw` de la rangée « exports:/makes: » est une table ob neuve construite par une aide locale de `planet_display.c` : `qe` (si `has_qe`) puis `mu`, dans l'ordre où `lz` les posait. `lq:mn` y lit le même `qe` qu'avant.
- **`jo.ex`** (`research_display.c:250`) : l'item `em` est `pico8_jb_row_view(pico8_jo_em())`, construit à l'appel.
- Ces tables ne sont jamais libérées (TROU-7, comme les rangées que `jo:gh` et `jo.ex` construisent déjà à chaque appel) ; `sanitize-net` tourne avec `detect_leaks=0`. Ponts fermés par D.6b (clé `lq` de `js`, `layout`) et D.7a (`planet_display`, `research_display`).
- Rejeté : **des vues persistantes construites par `pico8_jo_init()`**. `px` est lu par nom et typé : il faudrait retrouver la vue de chaque rangée ; `jo:gh` et `jo.ex` allouent déjà leurs rangées à chaque appel.
- Rejeté : **pousser le `kw` typé tel quel dans la rangée**. `lq:mn` le lirait comme une table ob, sans tag : exactement le cast aveugle que le §6 garde.

### Fermeture du pont `pico8_df()` (G)

- `pico8_df_lookup(px)` (écrite à la main à la fin de `pico8_game_data.c`) : recherche linéaire dans `pico8_df_table`, NULL si absent ou `px` NULL.
- `jo:hu` : `self->nk = pico8_df_lookup(hp.go)` ; `jo:hh` : `use_ll = pico8_df_lookup("ll") != NULL` (l'enregistrement `ll` est toujours vrai), sinon `pico8_gn_next(g_jo_oc)` rend un REF vers une entrée typée ; `ep = nk ? nk->d : NULL` ; l'argument de `ks` est `nk->me` s'il existe, sinon `"known"`.
- `bw` (`galaxy.c`) construit `hr` avec `pico8_value_ref((void *)&pico8_df_table[i])` pour `i` croissant ; `pico8_bw_positional_values` disparaît.
- `gn` (`gen.c`) lit le poids comme `((const pico8_df_entry_t *)e.as.ref)->r` ; `gen.c` inclut `pico8_game_data.h` au lieu de `pico8_ob.h` ; le contrat de `gen.h` dit « les entrées de `hr` sont des REF vers des `pico8_df_entry_t` ». Le pool de tickets, la fenêtre anti-répétition et `pico8_gn_next` ne changent pas (H7).
- `data_init.c` : `data_init_df_view`, `g_df`, son `free` et `pico8_df()` disparaissent ; `data_init.h` perd `pico8_df()` et gagne `pico8_jb_row_view`.
- Rejeté : **garder la vue pour `galaxy` jusqu'à D.7a** (lecture « s'il reste » de la note de D.3). Trois lignes de `gen.c` suffisent, et la vue n'aurait plus d'autre client.

### `Makefile`, docs

- **Le `Makefile` ne change pas** : aucun fichier nouveau ; `game_data.c`, `data_init.c`, `gen.c` sont déjà dans `DATA_ENGINE_SRCS`, `ENGINE_SRCS` et les listes de `main_raylib`. `DATA_RESIDUE_GUARD` reste vide.
- `docs/glossaire-noms.md` : lignes `nu`, `ka`, `dz`, `cq`, `kw`, `em`, `nk`, `px` (arbre `jo`), `ba`, `k_`, `hz`, `ht` → noms C d'après D.4a ; `df` ne nomme plus la vue ; lignes nouvelles `pico8_int_seq_t`, `pico8_str_seq_t`, `pico8_kw_t`, `pico8_jb_row_view`, `pico8_df_lookup`.
- `docs/plan-3.md` : note « Découvert à D.4a » en fin de ligne D.4a (texte dans Further Notes).
- **Commentaires** : les contrats de `planet.h`, `planet_family.h`, `planet_family_trade.h` (« hold the very tables of X's spec, borrowed ») sont réécrits pour dire ce que pointe chaque membre ; `research_display.h` (« Reads `jo.em` through pico8_jo_em(), so `jo`'s own init must have run ») perd l'exigence d'init ; `entity.c` et `particle.c` gagnent un commentaire de contrat d'une ligne sur la double forme (quatre sortes de `CLAUDE.md`). Aucun renvoi à D.4a ni au plan dans `src/` ou `tests/`.

### Ce qui ne change pas

- Aucun pixel, aucune instanciation, aucune condition de jeu.
- Aucun `*.sha256`, aucun fichier de `tests/e2e/`, aucun fichier d'événements.
- Les spécifications (`g_X_spec`, leur parse, `pico8_X_default`, les défauts non table `kq`, `gq`, fixes, chaînes) : D.5c. `pico8_X_default("nu")` rend toujours la table de la spécification ; plus aucun membre ne la pointe.
- Les autres clés de construction (`overrides` de `pico8_*_new`) : D.6a ; `pz`, `js`, `layout` : D.6b ; les tables ad hoc de `planet_display`, `research_display`, `planet_trade` : D.7a.
- `pico8_ob.c/.h`, `pico8_map_tables.*`, `pico8_layout.c`, `pico8_ui_button.c`, `pico8_msgbox.c`, `pico8_probe.c`, `pico8_help.c`, `pico8_slipway.c`, `pico8_wormhole.c`.
- `layout_nil_item_draws_nothing`.
- `tools/data_residue.sh`, ses motifs, `DATA_RESIDUE_GUARD`, `tools/class_coverage_*`, `KNOWN[]`.
- **TROU** : aucun écart port/cart n'est touché ni ajouté ; TROU-3 (commentaires nus sur les lignes de `qe`), TROU-7, TROU-21 préservés.

### Empreinte remesurée

- **`src/` : 22 chemins modifiés**, aucun nouveau : `planet.c/.h`, `planet_family.c/.h`, `planet_family_trade.c/.h`, `planet_display.c`, `planet_trade.c`, `research_display.c/.h`, `economy_score.c`, `economy_formulas.c`, `economy_tiers.c`, `entity.c`, `particle.c`, `game_data.c/.h`, `data_init.c/.h`, `galaxy.c`, `gen.c/.h`.
- **`tests/` : 16 chemins** : `test_pico8_jo`, `qh`, `nl`, `os`, `hj`, `pm`, `ga`, `nr`, `fr`, `mx`, `test_pico8_ui`, `test_pico8_hp`, `test_pico8_jo_spec_defaults`, `test_pico8_map_tables`, `test_pico8_planet_gh_row_lq`, `test_pico8_ga_export_panel_by`.
- Plus `docs/glossaire-noms.md` et `docs/plan-3.md`, soit **40 chemins** (38 de code).
- Écart avec la ligne du plan (34 : 22, 12) : `src/` égal en nombre (`entity.c`, `particle.c`, `gen.c/.h`, `galaxy.c` entrent par la double forme et la fermeture de `df` ; d'autres fichiers comptés par le majorant, noms courts partagés, ne changent pas) ; `tests/` +4 (`test_pico8_ui`, `test_pico8_hp`, `test_pico8_jo_spec_defaults`, `test_pico8_map_tables`, qui lisent les membres ou `pico8_df()`).

### Effet sur `make data-residue`

Attendu après D.4a (H17) :

| Catégorie | Avant | Après | Pourquoi |
|---|---|---|---|
| `stash` | 28/4 | **28/4** (exact) | — |
| `ob` | 2208 hors test (2266) | **< 2208 hors test, attendu ≤ 2136** ; le test ponctuel varie | −≈ 110 à 130 lignes de `src/` (membres et statiques `pico8_ob_table_t *`, trois aides de défaut, `jo_take_table`, accesseurs, `lz`, lectures de `df`, vue `df` de `data_init.c`, `bw`, `gn`, lectures `get_name`/`get_index` des membres), −≈ 30 à 50 dans les TU ; +≈ 10 (aide `kw` de `planet_display.c`, ligne `k_` de `pm:pq`, types des vues) |
| `overrides` | 449 | **450** (exact) | la ligne `pico8_ob_table_set_name(overrides, "k_", …)` de `pm:pq` ; D.6a en hérite (son « = 315 » devient 316) |
| `spec` | 939 | **< 939**, attendu ≈ 860 à 880 | −58 lignes `pico8_X_default("…")`/`X_default_table(` dans `src/`, −3 définitions, − les `class_ref(pico8_X_default, …)` des TU réécrits |
| `par-nom` | 1081 | **1081**, écart ≤ 2 expliqué | remplacements un pour un (`jo_take_table` → `jo_take_kw`/`jo_take_nk`, aides de séquence) ; une lecture par nom de `kw` peut s'ajouter dans `economy_score.c` si `field_ref_table` y garde ses appelants `po` |
| `type-tag` | 445 | rapporté au commit | −6 assertions `PICO8_TYPE_TAG_OB_TABLE` des aides de défaut et d'écriture, + les aides non taguées et les deux doubles formes |
| `ref-audit` | 70 | **70** (exact) | — |
| `dispatch` | 197 | **197** (exact) | aucune ligne de dispatch touchée |
| `valeur` | 6028/133 | rapporté au commit | — |
| Fichiers | 142 | ≤ 142 | aucun fichier n'entre |

- **Le majorant `ob` ≤ 2136 est attendu mais pas garanti (G)** : les vues de mise en page et l'aide `kw` en ajoutent. L'implémenteur écrit le compte exact N dans le commit et dans la note ; les majorants `ob` suivants se décalent de N − 2033. Au-dessus de 2136, c'est expliqué au commit, pas un arrêt ; au-dessus de 2207, c'est un échec.
- Aucune catégorie ne tombe à 0 : **`DATA_RESIDUE_GUARD` reste vide.** Aucun motif ne change.

### Découpage (G) : un seul commit

- 38 chemins de code, 40 avec les docs : au seuil de ~40, pas au-delà. **Pas de section « Implementation Chunks »**, que `prd-autopilot` traiterait comme autant de commits.
- La découpe naturelle — (1) `jo.nk` et la fermeture de `pico8_df()` ; (2) les douze autres membres — a été évaluée et écartée : les deux moitiés touchent `planet.c/.h` (`field`/`set_field`, `struct_init`), `test_pico8_ui` (`JO_MEMBERS_HASH`, `JO_WRITTEN_HASH` plient `kw` et `nk` ensemble), `test_pico8_jo` et `test_pico8_map_tables`, et `sanitize-net`/`class-coverage-net` passeraient deux fois pour une conversion.
- **Repli pour le pilote** : si l'empreinte mesurée dépasse 42 chemins de code, ou si le budget saute, `plan-pilot` recoupe selon (1)/(2) ci-dessus, (1) d'abord ; chaque moitié garde les cibles finales de ce PRD.

### Hypothèses (rien n'a été lancé)

- **H1** : `e24a5e5` est vert : `make test`, `make e2e` (26 conformes), `make sanitize-net` (26 propres), `make class-coverage-net` identique, `make data-guard` code 0, `make data-residue` égal à l'inventaire. Si c'est démenti au départ, l'implémenteur s'arrête et note : D.4a ne répare rien.
- **H2** : ce qu'`ob()` produit a la forme de l'inventaire (entiers seulement dans `ka`, `cq`, `ba`, `k_`, `hz`, `ht` ; chaînes seulement dans `nu` ; `px`/`dz` = une chaîne puis `c` ; `em` = faux, 3, 0, -1 ; `kw` = `{mu=0}`). Le générateur le vérifie. Démentie : le type suit ce qu'`ob()` produit, le test ponctuel juge, l'écart est noté ; arrêt seulement si un membre ne tombe sous aucune forme.
- **H3** : aucun lecteur par nom de `src/` autre que ceux de l'inventaire n'atteint ces noms ; les aides à nom non littéral (`field_fixed(inst, name)` et semblables de `economy_score.c`, `particle.c`, `probe.c`, `score_panel.c`, `entity_geom.c`) ne reçoivent que des noms scalaires. Démentie : ce lecteur est converti de même, noté au commit.
- **H4** : en jeu, `kw.qe` est toujours un fixe : `lz` pose `hp.o` (fixe, sous `has_o`) ; `mx:kf` n'écrit `qe` que si `qe ∈ ht`, ce qui implique un fixe. Seuls deux TU (§6) écrivaient la chaîne `"x"`.
- **H5** : `jo:ib` passe `kw.qe` à la clé `qe` de `trade` comme `pico8_value_fixed(qe)` ou nil, exactement ce que la table donnait.
- **H6** : un REF vers `pico8_str_seq_t`/`pico8_int_seq_t` lit `PICO8_TYPE_TAG_UNKNOWN` (premier mot = compte ≤ 13) : la double forme de `lt`/`he`/`f_` ne peut pas les confondre avec une table ob.
- **H7** : `gn` reçoit les mêmes entrées, dans le même ordre, avec les mêmes `r` (bits identiques à ceux de la vue) : même pool de tickets (66), mêmes tirages des aléas de `jo:hh`, e2e conformes.
- **H8** : aucun TU ni e2e ne dépend de `pico8_jo_dz()`/`cq()`/`ka()`/`em()` NULL avant `pico8_jo_init()` ou après `pico8_jo_shutdown()`.
- **H9** : les vues construites à l'appel ne changent aucun pixel : même contenu, même ordre ; `js:pq` recopie `lq` (`pico8_mw`/`pico8_lq_table_view`) sans en garder l'identité ; aucun code ne compare l'identité de l'item `kw` hors `gh_row_lq`.
- **H10** : déplacer la clé `k_` en fin de la table de `pm:pq` ne change aucun membre du `ga` : la boucle de clés de `ga` affecte chaque membre indépendamment.
- **H11** : les TU réécrits gardent leurs constantes de hachage : les identités « membre = défaut de telle classe » deviennent des égalités de pointeurs vers les données générées (mêmes vérités), les présences ne changent pas, `kw` plie les mêmes `qe`/`mu`, `nk` le même index dans `df`. Seule exception prévue : `test_pico8_hp`, dont `fold_planet` plie le tag de type du REF `kw` ; un autre pli de tag trouvé à l'implémentation suit la même exception.
- **H12** : `make sanitize-net` rend 26 propres : aucun `free` d'une donnée statique (les `*_shutdown` ne libéraient que les spécifications, jamais les tables des membres), aucune lecture hors bornes (aides bornées par `count`).
- **H13** : `make class-coverage-net` est identique : aucune instanciation n'est ajoutée ni retirée.
- **H14** : `-Werror` reste muet : les aides devenues sans appelant sont supprimées ; les casts `(void *)` sur des données `const` sont ceux de la règle de D.3 (`pico8_value_ref((void *)pico8_jo_em())`).
- **H15** : les durées sont courtes : `make test` en quelques secondes plus une recompilation large (`game_data.h`, `planet.h`), `make e2e` ~1 s, `sanitize-net` et `class-coverage-net` < 2 min chacun, compilation comprise.
- **H16** : les chaînes des `nu` générés ne sont comparées que par contenu (`cg` en garde une copie possédée) : leur adresse ne compte pas.
- **H17** : `make data-residue` rend les comptes du tableau ; `ob` exact dépend de l'écriture des vues.
- **H18** : `test_pico8_jo_spec_defaults` garde `STRUCT_HASH` si `same_value` compare un REF typé à la table de spécification par la relation « le membre est la donnée générée depuis la spécification qui a répondu » (voir Testing Decisions) ; `READY_HASH`, `SPEC_HASH`, `SHARE_HASH` ne lisent que les spécifications, inchangées.
- **H19** : `test_pico8_hp:521` rend le même index avec `nk - pico8_df_table` qu'avec `entry_of(pico8_df(), nk)` (position de l'enregistrement dans les positionnelles de la vue).
- **H20** : `make raylib` (non exigé, jamais lancé) lierait : aucun fichier nouveau.

## Testing Decisions

Un bon test juge ce que le code rend (valeurs, ordres, pixels, pointeurs), pas la présence d'une ligne. D.4a change la représentation de treize membres : **la preuve est que les données générées égalent ce qu'`ob()` produisait, que chaque TU qui figeait ces membres fige les mêmes vérités sur la nouvelle forme, et que tout ce qui mesure le comportement rend la même chose.**

### Le test ponctuel `tests/test_pico8_map_tables.c`, étendu (G)

- **Il construit son état, il ne rejoue rien.** Ses 2 lignes `stash` ne changent pas ; il ne nomme ni `spec` (`g_X_spec`, `pico8_X_default`), ni `overrides`, ni `type-tag`.
- **Chaînes de référence** : les dix spécifications et la chaîne de clés de `pm:pq`, copiées octet pour octet de `e24a5e5`, parsées après `pico8_pi_init()` (elles contiennent `b(...)` et `v(...)`).
- **Pour chaque défaut généré** : la clé de même nom de la table parsée existe ; séquence d'entiers → mêmes positionnelles, même compte, chaque valeur `pico8_fixed_from_int(items[i])`, aucune nommée ; liste d'étiquettes → mêmes chaînes (`strcmp`) dans l'ordre ; rangée → `pico8_jb_row_view(&row)` égale à la table parsée par l'aide d'égalité profonde existante ; `pico8_jo_kw_start` → `{mu=0}` seul.
- **`df`** : la comparaison de la vue (`test_df_view_matches_the_parsed_table_after_its_pass`) devient une comparaison de `pico8_df_table` à `df_reference()` : pour chaque enregistrement, positionnelles 1..3 (4 si `d`), nommées `px`, `s`, `r`, `d`, `me` ; et `pico8_df_lookup(px)` rend l'entrée de même index pour chacun des 11 `px`, NULL pour `"zz"` et pour NULL.
- **Aides** : `pico8_int_seq_has_fixed` rend 1 pour chaque élément, 0 pour un fixe absent, pour un fixe non entier de même partie entière, et pour une séquence vide.
- **`gn` sur `df` typé** (`gen.c` est dans `DATA_ENGINE_SRCS`) : `pico8_gn` sur la liste que `bw` construit (les onze entrées de `pico8_df_table`, dans l'ordre), `og` = 2, rend un pool de **66** tickets, chaque entrée présente `floor(2·r)` fois (0 pour `ll`) ; aucun appel du tirage n'est nécessaire.
- Rejeté : **comparer aux membres d'une instance construite.** Modules hors du lien du test ; l'identité membre/défaut est l'affaire des TU de classe.

### TU touchés : lire la struct, mêmes constantes (G)

- **Règle** : chaque lecture par nom d'un membre converti (`get_ref(e, "nu")`, `class_ref(pico8_X_default, "nu")`, `pico8_ob_table_get_name(get_ref(e, "kw"), …)`, `size_of(e, "k_")`, `pico8_df()`) devient une lecture du membre typé ou de la donnée générée ; les identités « membre = défaut de la spécification de X » deviennent « membre = donnée générée de X » (pointeurs) ; `kw` se lit sur `pico8_kw_t` (`has_qe`, `qe`, `mu`), le défaut partagé par `pico8_jo_shared_kw()`. **Aucune constante `*_HASH` ne change** (H11), **sauf** là où l'ancien pli portait le tag de type du REF d'un membre converti (`fold_value` → `pico8_type_tag_of`) : repéré à l'écriture, `test_pico8_hp` (`fold_planet` plie `kw` par `fold_field`, `:512`). Là, le pli du tag est remplacé par le contenu typé du membre (`has_qe`, `qe`, `mu`) et la constante est reprise, le commit nommant chaque constante reprise et sa cause. Toute autre constante qui bouge est un arrêt : l'implémenteur cherche la vérité perdue.
- **Écrivain par nom** (`test_pico8_ui` `test_jo_written_by_name`, `test_jo_writer_stops_off_row`, `test_pico8_ga:535`, `test_pico8_mx:674`, `test_pico8_ui:3477`) : les écritures de `kw`/`nk` passent un REF vers un `pico8_kw_t` / une entrée de `df` du test ; la liste hors rangée gagne la table ob (qui avorte désormais) ; `JO_WRITTEN_HASH` inchangé.
- **`test_pico8_jo`** : `kw` partagé muté (`:491-504`) lu sur `pico8_jo_shared_kw()` ; **assertion ajoutée** : après une écriture de `mu` dans le `kw` partagé, un nouvel appel de `pico8_jo_init()` le remet à 0 (support de M1). Accesseurs (`:585`) comparés aux données générées.
- **`test_pico8_ga`** : la clé `k_` de `ga` (`:813-821`) passe un REF vers une séquence du test ; `k_` de `roads[1]` (né de `pm`) est `&pico8_pm_ga_k__seq`, compte 13.
- **`test_pico8_jo_spec_defaults`** : `same_value(spec, struct)` gagne une règle pour les treize noms convertis : quand la spécification rend une table ob et la struct un REF non tagué, vrai si et seulement si ce REF est la donnée générée depuis la spécification du module qui a répondu (et, pour `kw`, `pico8_jo_shared_kw()`) ; le contenu est en plus **asserté** égal (pas plié). `STRUCT_HASH` inchangé (H18) ; son commentaire « à ne pas reprendre » reste vrai.
- **`test_pico8_hp:521`** : l'index de `nk` devient `nk - pico8_df_table` (H19).
- **`test_pico8_ui`** : `jo_identity`, `jo_accessor_is`, les assertions de tag (`PICO8_TYPE_TAG_OB_TABLE` → `UNKNOWN` pour les membres convertis) et la ligne `:1661` (`pico8_df_lookup`) suivent la règle ; `JO_MEMBERS_HASH` inchangé.

### Les deux tests de régression du §6 (G)

| Test | Changement | Bug d'origine | Preuve rouge (sur le code final) |
|---|---|---|---|
| `gh_row_lq` (l. 115-121, 142-143) | `kw` = `pico8_kw_t` du test `{has_qe=1, qe=pico8_fixed_from_int(99), mu=pico8_fixed_from_int(1)}`, posé par `planet->kw = &kw` ; le commentaire dit « 99 is deliberately not one of `jo.cq`'s resources » ; l'item 3 est un REF tagué `PICO8_TYPE_TAG_OB_TABLE` dont `qe` vaut 99 et `mu` 1 (au lieu de « le même pointeur que `kw` ») ; les autres assertions (4 items, « exports: », « 0/1 », pas de 5e, `fs` à 4) inchangées | 8361468 : `js:pq` passait la liste `lq` d'une ligne à `lq()` par un cast aveugle en `ob_table` | Remplacer localement le cas `PICO8_TYPE_TAG_LIST` de `pico8_js_lq_arg_table` par `return (const pico8_ob_table_t *)v.as.ref;`. Attendu : rapport ASan heap-buffer-overflow, ou à défaut les asserts de champ. Restaurer, vert. |
| `ga_export_panel_by` (l. 113-118) | `kw` = `pico8_kw_t` `{1, 99, 2}`, `planet->jo.kw = &kw`, même commentaire ; « 0/2 » puis « 2/2 » inchangés | 9183fee : le défaut `by=o()` de `ga` restait une `ob_table` lue comme liste | Retirer localement le bloc de remplacement de `by` dans `pico8_planet_family_init` (« `by=o()` parses to an ob_table… »). Attendu : abort dans `PICO8_REF_AS_LIST_CONST` pendant `jo:gh`, ou à l'assert de tag du test. Noter lequel. Restaurer, vert. |

Le fixe 99 n'est ni dans `cq` (12, 36, 24, 25) ni le `w` de `r` : la branche « exports: » est prise comme avec `"x"`. Chaque preuve est notée au commit (recette, sortie, restauration).

### Pas de nouvel e2e (G)

Les lignes nouvelles sont atteintes par les 26 e2e (démarrage, galaxie, panneaux) et par les TU ; un e2e n'est admis que s'il ajoute de la couverture de lignes en moins de 180 frames. Aucun n'est ajouté.

### Preuve des tests (§7, gcov + mutations)

- **gcov**, dans une copie jetable du scratchpad : `test_pico8_map_tables`, `test_pico8_jo`, `test_pico8_ga` et `test_pico8_mx` compilés en `--coverage`. Chaque ligne ajoutée ou modifiée de `game_data.c`, `planet.c`, `planet_family*.c`, `gen.c`, `galaxy.c` et des branches typées de `entity.c`/`particle.c` doit être exécutée par l'un d'eux ; les lignes des lecteurs de `planet_display.c`, `research_display.c`, `planet_trade.c` et de l'économie sont justifiées au commit par le TU qui les atteint ou par le filet e2e.
- **Six mutations**, chacune doit rougir un test :
  - **M1** : `pico8_jo_init()` ne remet pas `g_jo_kw_shared` à `pico8_jo_kw_start` → `test_pico8_jo` (assertion ajoutée) rouge ;
  - **M2** : `pico8_int_seq_has_fixed` compare l'entier brut au fixe (sans `pico8_fixed_from_int`) → `test_pico8_map_tables` rouge ;
  - **M3** : la branche typée de `lt` saute la première étiquette → `test_pico8_jo` (étiquettes `jo`/`np`/`prober` dans `cg`) rouge ;
  - **M4** : `gn` lit `s` au lieu de `r` comme poids → `test_pico8_map_tables` (pool de 66) rouge ;
  - **M5** : `pm:pq` pose `&pico8_pm_k__seq` au lieu de `&pico8_pm_ga_k__seq` → `test_pico8_ga` rouge ;
  - **M6** : l'aide `kw` de `planet_display.c` omet `qe` → `gh_row_lq` rouge.

  Chaque mutation est annulée. Une mutation qui ne rougit pas est un trou, justifié au commit ou couvert par une assertion ajoutée. La branche typée de `f_` (`particle.c`) n'a pas de mutation imposée : sa couverture est gcov (`test_pico8_ga`, éclat) ; si aucun TU n'en plie la couleur, c'est dit au commit.

### Vérification, budget et tableau « Runs »

**Budget : environ 12 minutes de temps total de tests.**

| Cible | Durée attendue |
|---|---|
| générateur (scratchpad, arbre de départ) | < 1 min, compilation comprise |
| `make test` | quelques secondes, plus une recompilation large |
| `make e2e` | ~1 s |
| `make sanitize-net` | < 2 min (H15) |
| `make class-coverage-net` | < 2 min (H15) |
| gcov + 6 mutations + 2 preuves rouges (scratchpad) | < 5 min |
| `make data-guard`, `make data-residue` | < 1 s |

Ordre conseillé :

1. `make data-residue` sur l'arbre de départ (H1) ;
2. le générateur sur l'arbre de départ, **avant toute modification** ;
3. types, données, aides et test ponctuel, puis `make test` ;
4. `jo.nk` et la fermeture de `pico8_df()` (`planet.c`, `gen`, `galaxy`, `data_init`, lecteurs de `nk`), puis `make test` ;
5. les douze autres membres, lecteurs, écrivains, doubles formes, vues, `pm:pq`, puis `make test` ;
6. les TU et les deux tests du §6, puis `make test` ;
7. `make e2e`, `make sanitize-net`, `make class-coverage-net` une fois à la fin ;
8. preuves rouges du §6, gcov, mutations ;
9. gardes, `grep`, glossaire, note du plan.

Au-delà du budget, l'implémenteur s'arrête et note ce qui manque, plutôt que de relancer. **Jamais l'appli raylib.** `make raylib` n'est pas exigé.

**L'implémenteur tient un tableau « Runs »** dans son journal de progression, une ligne par lancement :

| # | Commande exacte | État de l'arbre (HEAD + « données » / « nk+df » / « membres » / « TU »…) | Code de sortie | Ligne clé | Durée |
|---|---|---|---|---|---|

Exemples de ligne clé : `résidu D : 9685 ligne(s), 142 fichier(s)`, puis le pied final ; `test_pico8_map_tables: all tests passed` ; `26 conforme(s)` ; `sanitize: 26 propre(s)`. Le vérificateur réutilise ce tableau et ne relance que ce qui suit le dernier changement de l'arbre.

**Cibles finales**, toutes vertes sur l'arbre final :

1. `timeout 5m make test` ;
2. `timeout 5m make e2e` : **26 « conforme »** ;
3. `timeout 10m make sanitize-net` : `sanitize: 26 propre(s)` ;
4. `timeout 10m make class-coverage-net` : identique à `tools/class_coverage_net_expected.txt` ;
5. `make data-guard` : code 0 ;
6. `make data-residue` : `stash` 28/4, `overrides` 450, `ref-audit` 70, `dispatch` 197, `spec` < 939, `par-nom` 1081 (écart ≤ 2 expliqué), `ob` hors test ponctuel < 2208 (attendu ≤ 2136) ; écart expliqué.

### Contrôles du vérificateur

1. **Périmètre du diff** : `git show --stat` liste les 40 chemins de « Empreinte remesurée », et rien d'autre. Aucun `*.sha256`, rien sous `tests/e2e/`, aucun fichier d'événements, aucun changement du `Makefile`. `layout_nil_item_draws_nothing` intact.
2. **Membres** : les treize membres ont la forme du tableau « Forme par membre » ; plus aucun `pico8_ob_table_t *` parmi eux ; `kw_own` n'est lu ni écrit par nom.
3. **Données** : les défauts sont dans `pico8_game_data.c`, `static const` ou `const` exportés, un objet par table et par classe ; `pico8_game_data.c/.h` n'incluent toujours ni `pico8_ob.h`, ni `pico8_value.h` ; `g_jo_kw_shared` est le seul défaut non `const` et `pico8_jo_init()` le remet à `pico8_jo_kw_start`.
4. **Coexistence** : `field` rend un REF non tagué pour chaque membre converti ; les huit lecteurs par nom de `src/` de l'inventaire lisent la forme typée ; `lt`/`he`/`f_` ont exactement deux branches (tag table ob / sinon) ; les écrivains par nom `kw`/`nk`/`ht` et la clé `k_` de `ga` acceptent nil ou un REF non tagué, et avortent sur une table ob.
5. **`df`** : `pico8_df()`, `data_init_df_view` et `pico8_bw_positional_values` ont disparu ; `gen.c` n'inclut plus `pico8_ob.h` ; `bw` passe les onze entrées dans l'ordre de `pico8_df_table`.
6. **Vues** : `pico8_jb_row_view` est le seul constructeur de vue de rangée ; `jo:gh` et `jo.ex` construisent à l'appel les vues de `px`, `dz[mi]`, `em` et l'item `kw` ; aucune vue n'est libérée par erreur (`free` d'une donnée statique).
7. **`pm:pq`** : seule la clé `k_` a changé de forme ; `ga:kj` lit toujours la palette de la classe (TROU-21).
8. **TU** : aucune constante `*_HASH` n'a changé hors l'exception nommée (`test_pico8_hp`, pli du tag de `kw`) ; les deux tests du §6 construisent le `kw` typé et leurs preuves rouges figurent au commit ; `test_pico8_map_tables` compare les onze chaînes de départ (octet pour octet contre `git show e24a5e5:src/…`) et garde 2 lignes `stash`.
9. **Commentaires** : ceux qui sont ajoutés ou reformulés relèvent des quatre sortes de `CLAUDE.md` (contrats des membres, des aides, de la double forme, propriété des vues) ; aucun récit de chantier.
10. **Docs** : glossaire à jour ; note « Découvert à D.4a » en fin de ligne D.4a, rien d'autre dans le plan.

### Critères d'échec (traduits en `FAILED:` par le vérificateur de `prd-autopilot`)

**Départ et filet (§7)**

- `FAILED:` si l'arbre de départ n'est pas `e24a5e5` (ou un commit de docs au-dessus) propre, ou si `make data-residue` n'y rend pas l'inventaire.
- `FAILED:` si le filet (`make test`, `make e2e`, `make sanitize-net`, `make class-coverage-net`) reste rouge après **3 tentatives**.
- `FAILED:` si un `*.sha256` est modifié ou supprimé, si un fichier de `tests/e2e/` est touché, si un e2e est ajouté ou retiré, ou si un TU rejoue un fichier d'événements.
- `FAILED:` si `make e2e` ne rend pas **26 « conforme »**, si `make sanitize-net` ne rend pas 26 propres, ou si `make class-coverage-net` diffère de sa référence.
- `FAILED:` si `layout_nil_item_draws_nothing` est modifié, ou si `gh_row_lq`/`ga_export_panel_by` changent autre chose que la construction de `kw`, son commentaire et l'assertion d'identité de l'item 3.
- `FAILED:` si une preuve rouge du §6 manque au commit ou ne rougit pas.

**Périmètre**

- `FAILED:` si un des treize membres reste un `pico8_ob_table_t *`, ou si une ligne du `grep` de la « Sortie de l'étape » subsiste.
- `FAILED:` si une valeur générée a été écrite ou corrigée à la main, si un générateur est versionné, ou si `pico8_game_data.c/.h` incluent `pico8_ob.h` ou `pico8_value.h`.
- `FAILED:` si le `kw` des planètes nues n'est plus partagé, ou n'est pas remis à `{mu=0}` par `pico8_jo_init()`.
- `FAILED:` si le diff convertit une spécification (`g_X_spec`, un défaut non table), une clé de construction autre que `k_` de `ga`, `pz`, `js`, `layout`, ou un membre de D.4b (`mh.nu`, `g_.nu`/`k_`, `jc.nu`, `mz.nu`…).
- `FAILED:` si `ga:kj` lit la palette de l'instance (TROU-21), si un TROU est ajouté ou modifié, ou si un écart port/cart est corrigé.
- `FAILED:` si le test ponctuel nomme un motif de `spec`, d'`overrides` ou de `type-tag`, ou porte plus de 2 lignes `stash`.

**Gardes et comptes**

- `FAILED:` si `make data-guard` n'est pas vert, ou si `DATA_RESIDUE_GUARD`, `tools/data_residue.sh` ou un motif change.
- `FAILED:` si `make data-residue` ne rend pas `stash` 28/4, `overrides` 450, `ref-audit` 70, `dispatch` 197 ; si `spec` n'est pas < 939 ; si `par-nom` s'écarte de 1081 de plus de 2 ou sans explication ; si `ob` hors test ponctuel n'est pas < 2208.
- `FAILED:` si `make constants-check`, `float-grep` ou `float-guard` rougit, ou si `tools/float_guard.txt` gagne une ligne.

**Preuve, commit et note**

- `FAILED:` si une constante `*_HASH` d'un TU change hors l'exception « pli du tag de type d'un membre converti », ou sans la justification exigée plus haut.
- `FAILED:` si le rapport gcov ou l'une des mutations M1 à M6 manque, ou si une mutation ne rougit pas sans justification au commit.
- `FAILED:` si le travail n'est pas en un seul commit vert (sauf repli du pilote en deux commits, chacun vert).
- `FAILED:` si le budget est dépassé sans note de ce qui n'a pas été lancé, s'il manque le tableau « Runs », ou si le message de commit ne porte pas le pied de `make data-residue` avant et après et la description du générateur.
- `FAILED:` s'il manque la note « Découvert à D.4a », ou si elle omet un pont et son étape, ou le compte `ob` réel.
- `FAILED:` si un commentaire ajouté ou reformulé sort des quatre sortes de `CLAUDE.md`.

## Out of Scope

- **Les membres table de D.4b** : `bg.pl`, `hy.po`/`fu`, `fv.rb`, `mh.nu`, `g_.nu`/`k_`, `hp.mr`/`gp`/`fk`/`lq`, `pj.gs`/`i_`, `qb.pc`, `jc.nu`/`k_`/`hg`, `ui.ic`, `js._cr_t`, `mz.nu`/`kk` — et avec eux le retrait des branches ob de `lt`/`he`/`f_`.
- **Les spécifications de classe** de l'arbre `jo` et leurs défauts non table (`kq`, `gq`, fixes, chaînes, `by`) : D.5c.
- **Les clés de construction** (`overrides`) autres que `k_` de `ga` : D.6a.
- **Les consommateurs ob de mise en page** : `pz`/`js`/`layout` (D.6b), les tables ad hoc de `planet_display`, `research_display`, `planet_trade`, `economy*` (D.7a), et avec eux les vues construites à l'appel.
- **Le masque de bits de `nu`** : phase 2.
- **Libérer les vues et les rangées** construites par `jo:gh`/`jo.ex` : TROU-7.
- `make raylib`, l'appli raylib, l'audio, le rendu, `make e2e-gap` et `tools/line_coverage_reference.txt` (hors suite, décalés par tout diff de `src/`).

## Further Notes

- **Décisions de l'agent à la place de l'utilisateur** (consigne `to-prd` : vérifier modules, tests et découpage avec l'utilisateur), tranchées en `guideline` et marquées (G) plus haut :
  - **forme par membre** : `nu` → `const pico8_str_seq_t *` (étiquettes ordonnées, pas de masque) ; `ka`, `cq`, `ba`, `k_`, `hz`, `ht` → `const pico8_int_seq_t *` ; `dz`, `em`, `px` → `const pico8_jb_row_t *` ; `kw` → `pico8_kw_t *` + stockage `kw_own`, défaut partagé `g_jo_kw_shared` non `const` remis par `pico8_jo_init()` ; `nk` → `const pico8_df_entry_t *`.
  - **défauts** : générés une fois dans `pico8_game_data.c`, un objet par table et par classe ; les trois aides `*_default_table` et leurs 29 paires de lectures disparaissent.
  - **coexistence** : `field` rend un REF non tagué vers la donnée typée ; lecteurs par nom de `src/` convertis ; `lt`/`he`/`f_` à double forme jusqu'à D.4b ; écrivains par nom inchangés en noms, genre « nil ou REF non tagué ».
  - **ponts** : `pico8_df()` fermé (y compris `bw`/`gn`) ; vues ob à l'appel par `pico8_jb_row_view` pour `px`, `dz[mi]`, `em`, et une aide locale pour l'item `kw`, jusqu'à D.6b/D.7a.
  - **tests** : test ponctuel étendu (défauts, `df`, aides, pool de `gn`) ; TU réécrits à constantes inchangées (sauf le pli du tag de `kw` dans `test_pico8_hp`) ; deux tests du §6 avec preuve rouge ; pas d'e2e ; gcov et six mutations ; `class-coverage-net` et `sanitize-net` exigés.
  - **découpage** : un seul commit (40 chemins), repli en deux commits laissé au pilote.
- **Note du plan (G)**, à ajouter en fin de ligne D.4a (le §7 ne garde que ce qui change une étape suivante) :

  > Découvert à D.4a : formes `pico8_int_seq_t`, `pico8_str_seq_t` (`nu` : étiquettes ordonnées, le masque reste la phase 2), `pico8_jb_row_t` (`dz`, `em`, `px`), `pico8_kw_t` (`kw` pointe `g_jo_kw_shared`, non `const`, remis par `pico8_jo_init()`, ou `kw_own` après `lz`), `const pico8_df_entry_t *` (`nk`) ; défauts générés dans `pico8_game_data.c`, un objet par table et par classe (`pico8_X_nu_tags`, `_seq`, `_row`, `_rows`) ; `pico8_int_seq_has_fixed`, `pico8_df_lookup`. La lecture par nom d'un membre converti rend un REF non tagué vers sa donnée ; `lt`/`he` (`nu`) et `f_` (`k_`) lisent à double forme (table ob ou donnée typée) : D.4b retire la branche ob avec `mh.nu`, `g_.nu`/`k_`, `jc.nu`, `mz.nu`. Pont `pico8_df()` fermé (`bw` et `gn` lisent `pico8_df_table`). Ponts nouveaux : `pico8_jb_row_view` (publique, `data_init.c`), appelée à chaque appel par `jo:gh` (`px`, `dz[mi]`) et `jo.ex` (`em`), et l'item `kw` de `jo:gh` (D.6b pour `js`/`layout`, D.7a pour `planet_display`/`research_display`). `pm:pq` pose la clé `k_` de `ga` en REF typé : `overrides` = 450, et le « = 315 » de D.6a devient 316. `ob` après D.4a : N (au lieu de ≤ 2136) ; les majorants `ob` suivants se décalent de N − 2033.

  N est le compte réel, hors test ponctuel. Les noms sont ceux de l'implémentation.
- **Contrôle d'étape** : un sous-agent frais relit le diff contre ce PRD avant le commit (hors budget de tests).
- **Le message de commit** porte : le périmètre (40 chemins) ; la forme par membre et ses rejets ; la règle de coexistence ; le générateur en ≤ 15 lignes ; le pied de `make data-residue` avant et après, et le compte `ob` hors test ; les deux preuves rouges du §6 ; le rapport gcov et les mutations ; le tableau « Runs » résumé ; les réponses à H1 à H20.
- **Surprises pour le pilote** :
  - La ligne du plan ne voit pas que **trois lecteurs par nom sont partagés hors du groupe** : `lt`/`he` lisent `nu` et `f_` lit `k_` pour toutes les entités, dont `mh`, `g_`, `fv`, `jc`, `mz` de D.4b. Typer le groupe impose une lecture à double forme jusqu'à D.4b.
  - **`kw` est un item de mise en page** : `jo:gh` le pousse tel quel dans une rangée `lq`, que `layout` lit comme table ob. D'où l'aide de vue `kw`, et l'assertion d'identité de l'item 3 de `gh_row_lq` qui devient une assertion de contenu.
  - **`qh.px` et `jo.dz` sont des clés `lq` de `js`** (icône du panneau), et `pico8_js_lq_arg_table` avorte sur un REF non tagué : vues à l'appel jusqu'à D.6b.
  - **`ga.k_` est aussi une clé de construction** (la palette d'éclat de `pm:pq`) : typer le membre oblige à toucher cette seule clé de `pm:pq` (+1 `overrides`).
  - **Fermer `pico8_df()` touche `gen.c`** : `gn` lisait le poids `r` par nom ob ; `gn` n'a que `bw` pour client, trois lignes suffisent. La note de D.3 laissait `galaxy` à D.7a ; ce n'est plus nécessaire.
  - Les deux tests du §6 construisaient `kw` avec la chaîne `"x"` : `kw.qe` devient un fixe (H4), le fixe 99 garde l'intention (« pas une ressource de `cq` »).
  - `test_pico8_jo_spec_defaults` fige `STRUCT_HASH` par identité de REF « à ne pas reprendre » : la relation d'identité est gardée en la reportant sur les données générées, le contenu étant asserté à côté.
  - L'empreinte (40 chemins) est au seuil de ~40 ; le repli en deux commits est décrit plutôt que des « Implementation Chunks ».
