---
title: "Plan 3, étape D.5c : les spécifications de l'arbre `jo` (`jo`, `qh`, `nl`, `os`, `hj`, `pm`, `ga`, `nr`, `fr`, `mx`) deviennent des défauts `static const` générés une fois (`pico8_X_start`) que les constructeurs lisent ; les dix `pico8_X_default`, les `*_defaults_reset`, les aides `jo_default_*`/`family_default_*`/`family_trade_default_*`, la centaine de statiques recopiés et `pico8_ke_start_field` disparaissent ; le `field` des neuf sortes devient un vrai lecteur par nom des défauts statiques (`pico8_X_kind_field`, compté par `par-nom`) ; `ga.by` devient une liste statique non `const` ; `spec` tombe à 0 et entre dans `DATA_RESIDUE_GUARD` — deux commits (chunks), aucun pixel changé"
description: "Après D.5b, seul l'arbre `jo` parse encore ses spécifications par `ob()` à l'init de ses trois modules, puis les relit par nom pour remplir une centaine de statiques recopiés ; ses dix lecteurs `pico8_X_default` servent aussi de `field` aux neuf sortes que `ie` interroge, et la chaîne finit sur `pico8_ke_start_field`. D.5c génère ces défauts une fois en `static const`, fait lire ces données par les constructeurs (une seule source), remplace le `field` des sortes par un lecteur par nom honnête et compté des défauts statiques, supprime spécifications, lecteurs, aides, statiques, `pico8_ke_start_field` et les registres de `test_pico8_jo_spec_defaults`, éteint `spec` et le garde, en mesurant chaque catégorie en lignes ET en occurrences, sans tassement."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Phase D — Données », liste écrite après D.1) :

> - D.5c — `spec`, arbre `jo` : `jo`, `qh`, `nl`, `os`, `hj`, `pm`, `ga`, `nr`, `fr`, `mx`, aides `family_default_*` et `family_trade_default_*`, même règle ; le `field` des sortes (`pico8_entity_kind_t`, `g_qh_kind` … `g_mx_kind`) lit les défauts statiques sous un nom hors motif, compté par `par-nom`. Empreinte 19 (6, 13). Compte : `spec` = 0, ajouté à `DATA_RESIDUE_GUARD`. §6 : aucun. `class-coverage-net` : oui.

En-tête de la liste de D, règles communes et ordre, recopiés :

> Liste de D, écrite après D.1 (HEAD 3816427, agent `guideline`), à partir de `make data-residue` et `sh tools/data_residue.sh --detail`. Comptes à l'écriture (lignes/fichiers) : `valeur` 5989/132, `ob` 2245/128, `stash` 56/14, `type-tag` 445/71, `ref-audit` 70/17, `par-nom` 1081/80, `dispatch` 197/59, `overrides` 449/62, `spec` 939/68, **9678 lignes distinctes dans 143 fichiers** ; ceux de la note de D.0 sont d'avant D.1. Chaque ligne nomme la catégorie qu'elle pousse et son « Compte » après elle : « = » attendu exact, « ≤ » majorant par grep (lignes de la catégorie qui nomment les sites des groupes déjà passés, cumulées dans l'ordre de la liste). Le test ponctuel (deuxième puce de D) est hors compte jusqu'à D.8, où il est supprimé ; il ne nomme aucun motif de `spec` ni `overrides`. Une catégorie tombée à 0 entre dans `DATA_RESIDUE_GUARD` dans le même commit.
>
> Empreinte (mesurée à l'écriture, chaque PRD la remesure) : fichiers de `src/*.[ch]` et `tests/*.[ch]` qui portent une ligne de la catégorie du groupe, ou qui nomment ses tables ou membres (majorant : noms courts partagés, `nu`, `k_`, `lq`). Règles communes : un nom nouveau ne correspond à aucun motif figé de la catégorie qu'il remplace (pas de `g_X_spec`/`g_X_defaults`/`pico8_X_default`, et le paramètre `overrides` typé est renommé) ; un lecteur par nom qui survit reste compté par `par-nom` ; un défaut muté (§5, 1a : `pj.ou`, `pj.ea`, `pj.cr`, `ga.by`, `nf.l`, `nf.r`, `jo.kw`) devient un tableau fixe statique non `const`. Un TU construit son état et ne rejoue jamais de fichier d'événements ; un e2e nouveau ajoute de la couverture de lignes en moins de 180 frames ; aucun `*.sha256` ni fichier de `tests/e2e/` ne change. `make sanitize-net` : à chaque étape qui touche `src/`, puisque toutes font passer des tables ob à des données statiques ou locales (§7, « En D et en 2 »).
>
> Ordre : `stash`, puis les tables constantes nommées par la première puce ; les membres table (`pico8_ob_table_t *`, R1 `ref:table`) avant les spécifications, dont les défauts table en dépendent ; `spec`, `ke` d'abord et l'arbre `jo` en dernier ; `overrides` ; les tables ad hoc restantes, puis les TU ; la suppression de `pico8_ob.c`/`pico8_stash.c`. `par-nom`, `dispatch`, `pico8_list_t`, puis `valeur`/`type-tag`/`ref-audit` viendraient après : arrêt, voir la fin de la liste.

Notes « Découvert à D.x » des étapes livrées, recopiées (D.0, D.2, D.3, D.4a, D.4b, D.5a, D.5b) :

> Découvert à D.0 : `make data-residue` (`tools/data_residue.sh`, modes `--detail` pour la liste de D et `--check`), neuf catégories figées, lignes qui se chevauchent, commentaires compris — `valeur`, `ob` (avec `pico8_fi`), `stash`, `type-tag` (tout `pico8_type_tag.h`, dont le `tag` de `pico8_struct_header_t`), `ref-audit`, `par-nom` (`*field_truthy` seulement), `dispatch` (`pico8_value_function(_t)`, `pico8_mv` ; pas `methods[]`), `overrides`, `spec` (hors six `_default` à argument omis). Comptes initiaux après la fusion de master (lignes/fichiers) : 6001/133, 2280/128, 56/14, 450/72, 70/17, 1070/81, 180/57, 421/57, 942/68, **9711 lignes distinctes dans 145 fichiers** (100 de `src/`, 45 de `tests/`). Sortie vide = la seule ligne `résidu D : 0 ligne(s), 0 fichier(s)`. `pico8_list_t` sans catégorie (§6 ne le retire pas), mais ses éléments `pico8_value_t` comptent dans `valeur` : la liste de D doit en changer la représentation. « Plus aucun `malloc` pour des données » lu comme les allocations de `pico8_ob.c`/`pico8_stash.c`/`pico8_value.c`, éteintes avec leurs catégories ; une lecture plus large ajoute une catégorie dans son propre commit. Garde : `make data-guard` dans `make test` (`entity_value_members.sh --check`, `grep -rn offsetof src/`, `DATA_RESIDUE_GUARD`, vide à D.0) ; une étape qui éteint une catégorie l'ajoute à `DATA_RESIDUE_GUARD`. Motifs figés : une étape ne change un motif qu'avec la preuve que les lignes retirées ne touchent plus aucune catégorie transversale.

> Découvert à D.2 : `pico8_map_tables.c/.h` (données typées générées une fois, sans dépendance ob ; `pico8_r_lookup`, `pico8_iz_lookup`, `pico8_hc_lookup`, `pico8_jb_row_t`). Trois ponts, des vues ob construites par `data_init.c` depuis les tables typées : `pico8_fd()` (fermé par D.6c, entrée de `hp`), `pico8_dr()` (D.6b, entrée de `jb`), `pico8_nb()` (D.4b `js._cr_t`, puis D.7a `menu`/`tech_gate`). `pico8_hp_po` rend `const pico8_hc_entry_t *`. `pico8_data_init(mem)` ne lit plus `mem` (retrait à D.7a). Test ponctuel : `tests/test_pico8_map_tables.c`, 2 lignes `stash` jusqu'à D.8. `ob` après D.2 : 2227 (au lieu de ≤ 2195) ; les majorants `ob` suivants se décalent de 32.

> Découvert à D.3 : `pico8_game_data.c/.h` devient le module pur des tables constantes (générées une fois, sans dépendance ob ni `init` ; `pico8_int_seq_at`, `pico8_fixed_seq_at` ; un non-entier est l'hexadécimal signé des bits qu'`ob()` rendait, masque `0xffff.fffe` compris, jamais `PICO8_FIXED_LIT`) ; `pico8_jb_item_t` gagne `is_false`. Huit tables lues typées (`es`, `ct`, `b_`, `co`, `fb`, `ek`, `ci`, `ef`). Cinq ponts, des vues ob construites par `data_init.c` (publiques `pico8_ix_view`, `pico8_ha_view`, `pico8_bo_view`, `pico8_bz_view`, appelées par les `*_init`) : `pico8_df()` (fermé par D.4a, `jo.nk` ; `galaxy` à D.7a s'il reste), `pico8_ix()` et `pico8_ha()` (D.6b, clés de `js` et appelants de `menu.c`), `g_bo` (D.6b, `pz` → `js`), `pico8_bz()` (D.6b, entrée de `mh`, puis D.7a `score_panel_display`). `ob` après D.3 : 2208 (au lieu de ≤ 2137) ; les majorants `ob` suivants se décalent de 103 (2208 − 2105).

> Découvert à D.4a : formes `pico8_int_seq_t`, `pico8_str_seq_t` (`nu` : étiquettes ordonnées, le masque reste la phase 2), `pico8_jb_row_t` (`dz`, `em`, `px`), `pico8_kw_t` (`kw` pointe `g_jo_kw_shared`, non `const`, remis par `pico8_jo_init()`, ou `kw_own` après `lz`), `const pico8_df_entry_t *` (`nk`) ; défauts générés dans `pico8_game_data.c`, un objet par table et par classe (`pico8_X_nu_tags`, `_seq`, `_row`, `_rows`, `pico8_jo_kw_start`) ; `pico8_int_seq_has_fixed`, `pico8_df_lookup`. La lecture par nom d'un membre converti rend un REF non tagué vers sa donnée ; `lt`/`he` (`nu`) et `f_` (`k_`, lu par `ei`) lisent à double forme (table ob ou donnée typée) : D.4b retire la branche ob avec `mh.nu`, `g_.nu`/`k_`, `jc.nu`, `mz.nu`. Un écrivain par nom d'un membre converti accepte nil ou un REF non tagué : un vecteur n'y est plus refusé (limite `ref:autre` de 1b.23b), les TU hors rangée le remplacent par une table ob. Pont `pico8_df()` fermé (`bw` et `gn` lisent `pico8_df_table`). Ponts nouveaux : `pico8_jb_row_view` (publique, `data_init.c`), appelée à chaque appel par `jo:gh` (`px`, `dz[mi]`) et `jo.ex` (`em`), et l'item `kw` de `jo:gh` (aide locale `jo_kw_view`) (D.6b pour `js`/`layout`, D.7a pour `planet_display`/`research_display`). `pm:pq` pose la clé `k_` de `ga` en REF typé : `overrides` = 450, et le « = 315 » de D.6a devient 316. `ob` après D.4a : 2037 (au lieu de ≤ 2033) ; les majorants `ob` suivants se décalent de 4 (2037 − 2033).

> Découvert à D.4b : trois chunks (≈ 70 chemins ; le majorant de la ligne ne voyait pas les signatures forcées : `ho_apply`, `hy:kt`/`gi`, la rangée étendue). Formes : `pico8_vec_seq_t` (`qb.pc`) ; `pico8_jb_item_t` gagne `is_fixed`/`fixed`, `pico8_jb_row_t` gagne `fn` (`fv.rb`, `pj.gs`/`i_`, `hp.fk`, `hp.lq` tenue dans `hp` : `lq_own`/`lq_inner`) ; `hp.mr`/`gp` en `pico8_str_seq_t`, `ho_apply` les prend typés ; `pico8_qe_map_t` (`hy.po`/`fu`, stockages `po_own`/`fu_own` dans `hy`, `cm`/`bx`/`ck` remplissent une table fournie ; une clé `qe` chaîne est absente) ; `pico8_jc_hg_t` (`jc.hg`, 90 `id` au plus, `pico8_jc_hg_add`/`_has`) ; `pico8_ui_ic_t` (`ui.ic`, 16 couches vidées à chaque `gw`) ; `js._cr_t` pointe une entrée de `pico8_nb_table` (`pico8_nb_lookup`), `hy:kt` la prend typée par `pico8_gi_nb`, qui partage son calcul avec `pico8_gi`. `lt`, `he`, `ei` n'ont plus qu'une forme. Ponts restants : `pico8_nb()` et `pico8_gi(ob)` (D.7a, `menu`/`tech_gate`) ; vues à l'appel par `pico8_jb_row_view` de `hp.fk`, `hp.lq` (D.7a `research_display`/`planet_display`, D.6b `js`), `fv.rb`, `pj.i_` (D.7a `pi`), `pj.gs[idx]` (D.7a `score_panel`) ; `pico8_ft_index`/`ft_name` sans appelant (D.7a `collect`). `ob` après D.4b : 1882 (au lieu de ≤ 1986) ; les majorants `ob` suivants se décalent de −100 (1882 − 1982).

> Découvert à D.5a : deux chunks (interfaces, puis `ke` ; ≈ 40 chemins, `pico8_game_data.*`, `pico8_bootstrap.c` et les TU qui plient l'identité de `me` hors du majorant). Défauts générés `pico8_X_start` (type `pico8_X_start_t`, `pico8_game_data.h`), un par classe à clés (`ke`, `bg`, `ui`, `js`, `fq`, `qb`, `nf`, `mh`, `iv`, `mz`, `jc`), clés propres de la spécification, lus par les constructeurs ; les tables gardent l'objet de D.4a/D.4b ; `mz.jj` s'initialise de `pico8_mz_jj_qs` ; `lq`/`jb` sans objet ; `ui.le` : liste statique de `ui.c` (`pico8_ui_le_shared`) ; `qb.v`/`p`/`ra`, `iv.v`, `ui.hw` pointent la donnée générée (membres `const`) ; `nf.ra` pointe le statique non `const` `g_nf_ra`, affecté depuis `pico8_nf_start.ra` à `pico8_nf_init` (repli : `test_pico8_entity_kind` et `test_pico8_ie` écrivent `je.ra` en place) ; `nf.l`/`nf.r` restent `g_nf_l`/`g_nf_r` (D.7b). Le cycle `_init`/`_shutdown`/`_ready` des treize modules reste, sur un drapeau. `pico8_ke_start_field` (compté par `par-nom`) remplace `pico8_ke_default` au bout de `pico8_pj_default`, `pico8_g__default`, `pico8_ie_default` (retirés à D.5b) et `pico8_jo_default` (D.5c, avec le cas `M_KE` de `test_pico8_jo_spec_defaults`) : D.5c le supprime. `pico8_rk_default` (variante de dessin, comptée par `spec`) est à D.5b. `spec` après D.5a : 634 (au lieu de ≤ 779) ; les majorants `spec` suivants se décalent de −145 (634 − 779). `par-nom` : 1090.

> Découvert à D.5b : deux chunks (exclusion de `rk`, puis les sept classes ; 29 chemins, `pico8_game_data.*` et trois TU hors du majorant). `pico8_rk_default` (variante de dessin à argument omis) entre dans `SPEC_EXCLUSION`, preuve : ses quatre lignes ne portent aucune autre catégorie. Défauts générés `pico8_X_start` de `g_`, `fv`, `hp`, `ie`, `pj`, lus par les constructeurs ; `hy`, `trade` sans objet. `ie` : un seul défaut pour la chaîne et les objets prêtés, `kq`/`ra`/`eh` pointent la donnée générée, plus libérés à l'arrêt. `fv.hi`/`gq`/`kq`, `g_.ra`, `ie.kq`/`ra`/`eh`, `pj.cr.hi`/`gq` : membres `const` vers la donnée générée, aucun repli. `pj.cr` : `g_pj_cr` reste (son `ep` est muté), valeurs de `pico8_pj_cr_start`. `pj.ou`/`pj.ea` : rangées générées `pico8_pj_ou_row`/`pico8_pj_ea_row`, d'où `pico8_score_panel_init` construit une fois par init les tables d'état `g_pj_ou`/`g_pj_ea` (`pico8_jb_row_view`), jamais libérées comme celles de la spécification ; leurs lecteurs (`hb`, `hk`, `kj`, `pico8_mh_new`) restent des tables ob (D.6b, D.7a). `test_pico8_spec_defaults` n'a plus de registre ni de constante. `pico8_ke_start_field` n'a plus que `pico8_jo_default` et le cas `M_KE`. `spec` après D.5b : 430 (au lieu de ≤ 402 après le décalage de D.5a : 28 lignes des sites de D.5b, retirées par D.4a/D.4b, étaient comptées à la fois dans le majorant 547 et dans ce décalage ; majorant corrigé 430) ; le « = 0 » de D.5c est inchangé. `par-nom` : 1085.

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

Lecture de ces textes pour D.5c :

- **« Même règle »** renvoie à D.5a/D.5b : une spécification de classe est une table constante parsée au démarrage (première puce) ; elle devient un défaut `static const` généré une fois depuis `pico8_ob(spec)` (`pico8_X_start`, type `pico8_X_start_t`, `pico8_game_data.h`), que le constructeur lit — une seule source — ; `g_X_spec`, `pico8_X_default`, `*_defaults_reset` et les aides `X_default_<genre>` disparaissent ; le test ponctuel est étendu dans les deux sens ; les TU comparent directement aux données générées, sans registre.
- **« Compte : `spec` = 0, ajouté à `DATA_RESIDUE_GUARD` »** : exact, et dans **le même commit** que la dernière ligne `spec` retirée. Après ce commit, `make test` (par `make data-guard`, `sh tools/data_residue.sh --check spec src tests`) échoue si une seule ligne `spec` revient. Le « = 0 » ne dépend pas du majorant corrigé de D.5b (430, constaté exact au départ, voir l'inventaire).
- **« Le `field` des sortes […] lit les défauts statiques sous un nom hors motif, compté par `par-nom` »** : c'est le seul lecteur par nom **que la ligne garde**, et le seul point où l'étape pourrait tricher. Il est lu ici au sens fort (décision plus bas) : un vrai lecteur par nom, qui lit **les données générées** `pico8_X_start` (jamais une table parsée ni une table nom/valeur), dont **le nom correspond au motif `par-nom`** (`\bpico8_[a-z0-9_]+_(set_)?field(_fn)?\b`) pour que chaque ligne qui le nomme compte, qui n'a **pas d'autre appelant** que la sorte (`->field`, lui aussi compté) et la chaîne des sortes, et qui ne répond qu'aux noms qu'un consommateur réel lui demande. Un renommage de `pico8_X_default` qui garderait la lecture de la spécification, ou un nom qui sortirait aussi du motif `par-nom` (une fonction `static` sans le préfixe `pico8_`, par exemple `qh_kind_field`, ne serait comptée nulle part), est l'esquive que les critères d'échec interdisent.
- **`pico8_ke_start_field`** : la note de D.5a et celle de D.5b le laissent à D.5c, avec le cas `M_KE` de `test_pico8_jo_spec_defaults`. Sa suppression n'est pas un déplacement : parmi les quatre noms que les sortes servent (`rr`, `s`, `me`, `od`), `ke` n'en fournit aucun dans l'arbre `jo` (`jo` surcharge `me`), donc la chaîne des sortes s'arrête à `jo` sans rien reprendre de `ke`.
- **Défauts mutés de l'arbre `jo`** (règle commune : « un défaut muté (§5, 1a : `pj.ou`, `pj.ea`, `pj.cr`, `ga.by`, `nf.l`, `nf.r`, `jo.kw`) devient un tableau fixe statique non `const` ») : `jo.kw` l'est déjà depuis D.4a (`g_jo_kw_shared`, non `const`, remis par `pico8_jo_init()`) ; `ga.by` est aujourd'hui une `pico8_list_t` du tas, substituée à la table ob parsée (`by=o()`) et jamais libérée. D.5c en fait un statique non `const` sur le précédent exact de `ui.le` (D.5a, `g_shared_le`). Son contenu (des enregistrements de `jo:ib`) reste une `pico8_list_t` d'éléments `pico8_value_t` : le typer, c'est D.7a (sa ligne rattache `ga_export_panel_by`, avec preuve rouge).
- **`make class-coverage-net`** (ligne : « oui ») et **`make sanitize-net`** (§7 « En D et en 2 ») au chunk 2 : il touche dix constructeurs et change la durée de vie de données (tables ob parsées à l'init → `static const` ; boîtes et vecteurs du tas partagés → données générées ; en-tête de liste de `ga.by` du tas → statique). Le chunk 1 n'ajoute que des données sans lecteur : ni l'un ni l'autre.
- **§6 : aucun**, et c'est exact pour les lectures ; mais le site du correctif que garde `ga_export_panel_by` (la substitution du `by=o()` par une vraie liste, `planet_family.c` l. 1444-1459) **disparaît** avec la spécification de `ga`. Le test ne change pas ; une mutation (M4) prouve qu'il rougit toujours si le `by` de `ga` cesse d'être une liste. Ce n'est pas un changement d'intention, donc pas un arrêt.

### État de départ

HEAD `e52b1c2` (branche `claude/plan-pilot-3-7zdft0`), arbre propre. D.5b est livrée (`7724ef4`, `f652c78`) et gradée (`e52b1c2`). Décisions de l'utilisateur qui engagent D.5c : **un TU ne rejoue jamais un `events.json`** (il construit son état), **rien de lent**, **aucun `*.sha256` ni fichier de `tests/e2e/` touché**, **un e2e nouveau ajoute de la couverture de lignes en moins de 180 frames**. Consigne de pilotage, liante : **une étape précédente a été notée FAILED pour avoir fait baisser un compte par tassement de lignes** ; le compteur compte des lignes, et toute mise en forme, tout renommage, tout alias, tout déplacement ou tout registre qui baisse un compte sans retirer la construction est interdit (section « Anti-tassement »).

**Rien n'a été compilé ni lancé pour ce PRD.** Seuls `make -s data-residue`, `sh tools/data_residue.sh --detail`/`--check`, la recette d'occurrences (`grep -oE` sur les motifs figés) et des `grep`/`sed -n` ont tourné. Le reste vient de la lecture des sources de `e52b1c2`. Tout fait d'exécution est une hypothèse (H1 à H21).

### Inventaire (mesuré sur `e52b1c2`)

**`make data-residue`** (lignes / fichiers) et **occurrences** des mêmes motifs (recette de la section « Anti-tassement », même exclusion `SPEC_EXCLUSION` pour `spec`) :

| Catégorie | Lignes | Fichiers | Occurrences |
|---|---|---|---|
| `valeur` | 5831 | 133 | 6741 |
| `ob` | 1922 | 123 | 2152 |
| `stash` | 28 | 4 | 28 |
| `type-tag` | 423 | 70 | 686 |
| `ref-audit` | 69 | 17 | 70 |
| `par-nom` | 1085 | 80 | 1140 |
| `dispatch` | 197 | 59 | 199 |
| `overrides` | 450 | 62 | 479 |
| `spec` | **430** | **19** | **470** |
| **résidu D** | **8817 lignes** | **141 fichiers** | — |

`ob` 1922 = **1780 hors test ponctuel** + 142 lignes de `tests/test_pico8_map_tables.c` (occurrences : 2003 hors test + 149). `spec` = 430 est exactement le compte que la note de D.5b annonçait ; **tout `spec` restant est dans l'arbre `jo`** : 19 fichiers, 6 de `src/` et 13 de `tests/`, l'empreinte de la ligne (« 19 (6, 13) »).

**`spec` par fichier** (lignes / occurrences) et ce qui est compté :

| Fichier | `spec` | Ce qui est compté |
|---|---|---|
| `src/pico8_planet.c` (`jo`) | 30/30 | `g_jo_spec`, `pico8_jo_defaults_reset`, `jo_default_box`/`_vec`/`_fixed`/`_hf`, parse, sept paires `found = pico8_jo_default(…)` / `g_jo_X = jo_default_…`, lecture de `me`, libération, `pico8_jo_ready`, `pico8_jo_default` (fin de chaîne `pico8_ke_start_field`, l. 756), `assert` d'init de `pico8_jo_struct_init` |
| `src/pico8_planet.h` | 1/1 | déclaration de `pico8_jo_default` |
| `src/pico8_planet_family.c` (`qh`, `nl`, `os`, `hj`, `pm`, `ga`) | 208/214 | six `g_X_spec`, six `pico8_X_defaults_reset`, `family_default_box`/`_vec`/`_fixed`/`_list`/`_hf`/`_string`, six parses, ≈ 80 paires `found = pico8_X_default(…)` / `g_X_Y = family_default_…`, la substitution de `ga.by` (l. 1444-1459), six libérations, six `assert` de constructeur, six `pico8_X_default` et leurs six sortes `{pico8_X_struct_new, pico8_X_default}` |
| `src/pico8_planet_family.h` | 6/6 | six déclarations `pico8_X_default` |
| `src/pico8_planet_family_trade.c` (`nr`, `fr`, `mx`) | 88/91 | trois `g_X_spec`, trois remises, `family_trade_default_box`/`_fixed`/`_string`/`_hf`, trois parses, 24 paires, libérations, trois `assert`, trois `pico8_X_default` et leurs sortes |
| `src/pico8_planet_family_trade.h` | 3/3 | trois déclarations |
| **modules** | **336/345** | |
| `tests/test_pico8_entity_kind.c` | 9/9 | le `switch` `default_of` (neuf lecteurs), comparé à `kind_of(i)->field` |
| `tests/test_pico8_jo_spec_defaults.c` | 10/10 | le registre `spec_field` (dix lecteurs + `pico8_ke_start_field`, compté `par-nom`) |
| `tests/test_pico8_ui.c` | 18/31 | `identity_of`/`spec_me` (l. 1106-1112), `jo_identity` (l. 1479), tableaux de lecteurs `own[10]` (l. 1527-1529), `qh_tree_reader` (l. 2760-2762), `hj_tree_reader` (l. 3110), lectures directes (l. 1799, 1866, 2511, 3234, 3240, 3314, 3318) |
| `test_pico8_fr` 11/13, `ga` 11/12, `hj` 7/10, `jo` 3/3, `mx` 6/8, `nl` 5/7, `nr` 6/8, `os` 2/4, `pm` 4/7, `qh` 2/3 | 57/75 | en-têtes qui nomment les lecteurs, `class_ref`/`class_value(pico8_X_default, nom)` (lecteurs locaux au test), boucles sur des noms (`jo_tables[]`, `nr_tables[]`, `inherited[]`, `own_methods[]`), absences (`!pico8_X_default("kj", &v)`), discriminants `default_of == pico8_pm_default` |
| **TU** | **94/125** | |

**Les dix spécifications** (chaînes exactes parsées aujourd'hui, identiques octet pour octet à `JO_SRC` … `MX_SRC` de `tests/test_pico8_map_tables.c`) : clés propres, chaîne de repli, sorte :

| Classe (module) | Clés propres (nombre) | Parente | Sorte |
|---|---|---|---|
| `jo` (`planet`) | `nu`, `me`="unknown", `hf`=1, `hs`=1, `fa`=6, `kq`=b(-7,-7,7,7), `ma`=0, `ka`, `dz`, `cq`, `kw`=o(mu=0), `gq`=v(0,11), `ca`=9, `em` (14) | `ke` | — |
| `qh` (`planet_family`) | `nu`, `hp`=1, `px`, `me`="kz", `dq`="ly", `hf`=f, `qh`=1, `od`=15, `fp`=2, `dw`=0.025, `mi`=1, `s`=174, `ba` (13) | `jo` | `g_qh_kind` |
| `nl` | `nu`, `rr`="trade league", `qh`=f, `nl`=1, `hp`=6, `s`=68, `px`, `od`=20, `qw`=15, `ba` (10) | `qh` | `g_nl_kind` |
| `os` | `nu`, `hp`=4, `px`, `qh`=f, `s`=100, `od`=15, `hs`=0.5 (7) | `qh` | `g_os_kind` |
| `hj` | `nu`, `rr`="ascension", `hp`=2, `qh`=f, `px`, `od`=60, `s`=166, `py`=0.12, `ki`=9, `pn`=5, `pk`=1, `fc`=0.5, `k_`, `du`=7.5, `qo`=0.98, `ro`=-0.07, `qg`=0.07, `q_`=0.98 (18) | `qh` | `g_hj_kind` |
| `pm` | `fz`="pm", `nu`, `hp`=3, `px`, `me`="ly", `s`=98, `py`=0.06, `ki`=9, `pn`=8, `pk`=f, `ng`=v(1,0), `du`=2, `fc`=1, `k_`, `qo`=1.02, `ro`=-0.0525, `qg`=0.0525, `q_`=1.02 (18) | `hj` | `g_pm_kind` |
| `ga` | `fz`="ga", `nu`=o(), `hf`=f, `by`=o(), `kq`=b(0,0,-1,-1), `fa`=3, `fc`=0, `qo`=0.9, `ro`=0, `qg`=0, `q_`=0.9, `k_`, `iu`=-0.06, `py`=6, `ms`=3 (15) | `jo` | `g_ga_kind` |
| `nr` (`planet_family_trade`) | `nu`, `od`=10, `fp`=1, `kq`=b(-4,-4,5,5), `s`=170, `hz`, `dw`=0.01, `rr`="slipgates" (8) | `qh` | `g_nr_kind` |
| `fr` | `mt`=1, `rr`="infraspace", `od`=25, `s`=78, `hz` (5) | `nr` | `g_fr_kind` |
| `mx` | `nu`, `rr`="void synthesis", `hp`=5, `s`=66, `px`, `od`=40, `ht`, `py`=0.12, `ki`=9, `pn`=5, `pk`=1, `fc`=0.3, `k_`, `du`=5, `qo`=0.98, `ro`=-0.07, `qg`=0.07, `q_`=0.98 (18) | `hj` | `g_mx_kind` |

Les tables (`nu`, `ka`, `dz`, `cq`, `kw`, `em`, `px`, `ba`, `k_`, `hz`, `ht`) ont déjà leur objet généré par D.4a (`pico8_X_nu_tags`, `_seq`, `_row`, `_rows`, `pico8_jo_kw_start`) ; la table parsée ne sert plus qu'aux lecteurs par nom et aux TU.

**Faits qui engagent la forme :**

- **Les trois modules relisent leur spécification par nom à l'init** (`pico8_X_default` → aide typée → statique `g_X_<clé>`), une centaine de statiques recopiés (valeurs résolues le long de la chaîne, `*_is_bool` compris), lus ensuite par les constructeurs : c'est la quasi-totalité des 336 lignes de module.
- **Les boîtes et vecteurs `jo.kq`, `jo.gq`, `pm.ng`** (et les `kq` de `ga` et de `nr`) sont aujourd'hui **les objets du tas de la spécification**, partagés par toutes les instances ; `grep` ne trouve **aucune écriture à travers** ces pointeurs, dans `src/` ni dans `tests/` (les seules affectations rebranchent le pointeur : `test_pico8_ga_export_panel_by.c:125`). `jo_field` les rend par `pico8_value_ref`, `pm:pq` lit `ng` par valeur.
- **Le `field` des sortes a un seul consommateur dans `src/`** : `pico8_ie_ke_view` (`research_icon.c:87-132`), qui lit `rr`, `s`, `me`, `od` de la sorte que tient `ie.ke` (`ie_ke_take_string`/`_fixed` → `kind->field`). Aucun autre site de `src/` n'appelle `->field` sur une sorte (`grep -- '->field'`) ; `jo_kv` et `pico8_cart.c` n'utilisent que `new_entity`. Côté TU, `test_pico8_entity_kind` lit exactement ces quatre noms (`test_kind_fields_and_build`, `KIND_HASH` figé) et vérifie `kind_of(i)->field == default_of(i)`.
- **Ce que les neuf sortes répondent aujourd'hui aux quatre noms** (résolu le long de la chaîne) : `qh` : `s` 174, `me` "kz", `od` 15, `rr` absent ; `nl` : `rr`, `s` 68, `od` 20 propres, `me` de `qh` ; `os` : `s` 100, `od` 15 propres, `me` de `qh`, `rr` absent ; `hj` : `rr`, `s` 166, `od` 60 propres, `me` de `qh` ; `pm` : `s` 98, `me` "ly" propres, `rr` et `od` de `hj` ; `ga` : `me` de `jo` ("unknown"), `rr`/`s`/`od` absents ; `nr` : `rr`, `s` 170, `od` 10 propres, `me` de `qh` ; `fr` : `rr`, `s` 78, `od` 25 propres, `me` de `qh` par `nr` ; `mx` : `rr`, `s` 66, `od` 40 propres, `me` de `qh` par `hj`. **`ke` ne fournit aucun des quatre** (`jo` surcharge `me`).
- **`pico8_ke_start_field`** n'a plus que deux appelants : `planet.c:756` (fin de `pico8_jo_default`) et `test_pico8_jo_spec_defaults.c:50` (`M_KE`). Ses lignes comptées par `par-nom` : `ke.c:27`, `ke.c:32`, `ke.h:25` (commentaire), `ke.h:27`, `planet.c:756`, `test_pico8_jo_spec_defaults.c:50`.
- **Chaînes et identité (TROU-3)** : `ob()` alloue une chaîne neuve par jeton, donc deux défauts égaux n'étaient jamais le même pointeur. Or `pico8_game_data.c` porte déjà deux littéraux `"kz"` (`jc.me`, l. 392 ; `mz.me`, l. 406) et un `"ly"` (`g_.dq`, l. 418), et `planet.c:950` passe un littéral `"ly"` à `ke:ks`. Des littéraux identiques d'une même unité de compilation sont fusionnés par le compilateur (et, en sections fusionnables, par l'éditeur de liens) : **un `qh.me` écrit `"kz"` en littéral deviendrait le pointeur de `jc.me`/`mz.me`**, ce qui retournerait `identity_of` de `test_ke_me_by_name` (`identity[19]`, `identity[23]` : 0 → 3) et changerait `KE_ME_HASH` ; `pm.me` "ly" rejoindrait `qh.dq`, `g_.dq` et le littéral de `ke:ks`. D'où la décision « un tableau distinct par jeton » plus bas.
- **`ga.by`** : `by=o()` est parsé en table ob, puis remplacé à l'init par une `pico8_list_t` du tas (`malloc`, jamais libérée), que `pico8_ga_struct_new` donne à chaque `ga` (`jo.by = g_ga_by`) : une liste partagée par la classe, dans laquelle `jo:ib` ajoute ses enregistrements. C'est le correctif que garde `test_pico8_ga_export_panel_by` (§6).
- **Les `_ready`** (dix) rendent `g_X_spec != NULL` ; `pico8_cart.c:80-84` et les `assert` des `init` en dépendent ; les constructeurs assertent `g_X_spec != NULL`.
- **`pico8_pi_init()`** est appelé par `pico8_jo_init` (commentaire : « the spec contains `b(…)` and `v(…)` ») et par `pico8_planet_family_init`/`_trade_init` : D.7a (précédent D.5b, H10).
- **`test_pico8_jo_spec_defaults`** est tout entier un registre des onze modules (`enum`, `PARENT[]`, `spec_field`, `is_ready`, `build`, `shutdown_of`, `NAMES[]`, `generated()`, `answering`, `same_default`…) sous quatre constantes (`READY_HASH`, `SPEC_HASH`, `SHARE_HASH`, `STRUCT_HASH`) : D.5c le vide, comme D.5b a vidé `test_pico8_spec_defaults`.
- **Compteurs de contrôle** : `pico8_jb_row_view` a 18 sites dans `src/` ; `strcmp` : 55 lignes dans `planet.c`, 63 dans `planet_family.c`, 11 dans `planet_family_trade.c` ; la ligne `ref-audit` `planet_family.c:1445` (commentaire de la substitution de `by`) est la seule ligne d'une autre catégorie transversale que la substitution retire.

## Solution

**Deux commits (chunks, section « Implementation Chunks »), sans aucun pixel changé** :

1. **Les données** : pour les dix classes, un type `pico8_X_start_t` et un objet `static const` `pico8_X_start`, générés une fois depuis `pico8_ob()` sur les chaînes de départ, dans `pico8_game_data.h`/`.c` (module pur) ; leurs chaînes dans des tableaux distincts, un par jeton ; le test ponctuel les compare aux parses, dans les deux sens. Aucun lecteur encore : `spec`, `par-nom` et `ob` hors test ne bougent pas.
2. **La conversion** :
   - **Les constructeurs lisent `pico8_X_start`** (une seule source) ; la centaine de statiques `g_X_<clé>`, les dix remises, les aides `jo_default_*`, `family_default_*`, `family_trade_default_*`, les dix parses et libérations disparaissent ; `jo.kq`, `jo.gq`, `pm.ng` deviennent des pointeurs `const` vers la donnée générée (repli de D.5a si H3 tombe).
   - **Le `field` des neuf sortes** devient `pico8_X_kind_field` : un lecteur par nom des **défauts statiques**, qui ne répond qu'aux quatre noms que `ie` lit (`rr`, `s`, `me`, `od`), depuis les champs de `pico8_X_start` de la classe qui les possède, puis sa parente, jusqu'à `pico8_jo_kind_field` ; nom au motif `par-nom`, compté ligne à ligne ; aucun autre appelant.
   - **`pico8_ke_start_field` est supprimé**, sans remplaçant.
   - **`ga.by`** devient `g_ga_by_shared`, `pico8_list_t` statique non `const`, vidée à chaque `pico8_planet_family_init` (précédent `ui.le`) ; **`jo.kw`** reste `g_jo_kw_shared` (D.4a), remis depuis `*pico8_jo_start.kw`.
   - **Les dix `pico8_X_default` disparaissent**, sans remplaçant par nom ; les `_ready` lisent un drapeau par module.
   - **Les TU** comparent directement aux données générées ; `test_pico8_jo_spec_defaults` perd son registre et ses quatre constantes, leur vérité reprise par dix fonctions à leur nom ; le test ponctuel vérifie en plus chaque sorte sur les quatre noms.
   - **`DATA_RESIDUE_GUARD := spec`** dans le même commit : `spec` = 0.

**Sortie de l'étape** (après le chunk 2), constatée par commande :

- `make test` vert ; `make e2e` : 26 conformes ; `make sanitize-net` : 26 propres ; `make class-coverage-net` : identique ; `make data-guard` : code 0, **avec `spec` gardé** ;
- `make data-residue` : **plus de ligne `spec`** ; `sh tools/data_residue.sh --check spec src tests` : `résidu gardé (spec) : 0 ligne(s)` ; `stash` 28/4, `overrides` 450, `dispatch` 197, `ref-audit` 68, `par-nom` ≈ 1108 (≤ 1118, chaque ligne ajoutée classée), `ob` hors test ponctuel < 1780 ; comptes en lignes ET en occurrences, avant et après, par catégorie, par fichier et par chunk, conformes à « Anti-tassement » ;
- ce `grep` ne rend rien :

  ```sh
  grep -rnE '\bg_(jo|qh|nl|os|hj|pm|ga|nr|fr|mx)_spec\b|\bpico8_(jo|qh|nl|os|hj|pm|ga|nr|fr|mx)_(default|defaults_reset)\b|\b(jo|family|family_trade)_default_(box|vec|fixed|hf|list|string)\b|\bpico8_ke_start_field\b|\bg_(jo|qh|nl|os|hj|pm|ga|nr|fr|mx)_(me|hf|hf_is_bool|hs|fa|kq|ma|gq|ca|mi|hp|dq|qh|qh_is_bool|od|fp|dw|s|rr|nl|qw|py|ki|pn|pk|pk_is_bool|fc|du|qo|ro|qg|q_|ng|fz|iu|ms|by|mt)\b' src tests --include='*.[ch]'
  ```
- `grep -rn 'pico8_ob(' src/pico8_planet.c src/pico8_planet_family.c src/pico8_planet_family_trade.c` ne rend que la clé de construction de `pm:pq` (`planet_family.c`, `overrides`, D.6a) ;
- `grep -rnE '\bpico8_[a-z0-9_]+_kind_field\b' src tests --include='*.[ch]'` ne rend que les dix définitions, la déclaration de `pico8_jo_kind_field`, les neuf initialiseurs de sorte, les appels de chaîne et au plus un commentaire de contrat par en-tête ; **aucune ligne de `tests/`** ;
- `grep -c strcmp` : `planet.c` 56, `planet_family.c` 76, `planet_family_trade.c` 20 (départ + exactement les 23 comparaisons de noms des lecteurs de sorte, table plus bas) ;
- `grep -rn 'pico8_jb_row_view' src | wc -l` : 18, inchangé.

## User Stories

1. En tant que mainteneur du port, je veux que les défauts des dix classes de l'arbre `jo` soient des données `static const` générées une fois, afin qu'aucune spécification de classe ne soit plus parsée au démarrage.
2. En tant que mainteneur, je veux que chaque constructeur de l'arbre lise son `pico8_X_start` au lieu d'une centaine de statiques remplis par lecture par nom à l'init, afin qu'une valeur de défaut n'existe qu'à un seul endroit.
3. En tant que mainteneur, je veux que chaque `pico8_X_start` ne porte que les clés propres de sa classe et que l'héritage passe par le `struct_init` de la parente, afin que la chaîne du cart (`mx` → `hj` → `qh` → `jo` → `ke`) reste celle du code sans être recopiée.
4. En tant que mainteneur, je veux un type de défaut par classe, aux champs typés comme la struct (`pico8_fixed_t`, `const char *`, `hf`/`qh`/`pk` avec leur `*_is_bool`, boîte et vecteur par valeur, pointeurs `const` vers les objets de D.4a), afin de lire `pico8_hj_start.ro` sans passer par un nom.
5. En tant que gardien du comportement, je veux que les non-entiers (`qh.dw`, `os.hs`, `hj.py`/`fc`/`du`/`qo`/`ro`/`qg`/`q_`, `pm.py`/`qo`/`ro`/`qg`/`q_`, `ga.qo`/`q_`/`iu`, `nr.dw`, `mx.py`/`fc`/`qo`/`ro`/`qg`/`q_`) gardent les bits exacts qu'`ob()` rendait, afin qu'aucune trajectoire, aucune particule ni aucun texte ne bouge d'un pixel.
6. En tant que gardien du comportement, je veux que `hf`, `qh` et `pk` gardent leur double forme (fixe ou `false`) exactement là où la spécification les met, afin que `jo:hb`, `qh:qh` et les particules se comportent pareil.
7. En tant que gardien du comportement, je veux que deux défauts chaînes ne soient le même pointeur que si `ob()` les rendait tels (jamais), afin que les comparaisons par identité de TROU-3 (`pico8_value_equal`, `identity_of`) rendent ce qu'elles rendaient.
8. En tant que mainteneur, je veux que `jo.kq`, `jo.gq`, `pm.ng` et les `kq` de `ga`/`nr` pointent la donnée générée, afin de supprimer les boîtes et vecteurs du tas de la spécification.
9. En tant que gardien du comportement, je veux que toutes les instances d'une classe partagent toujours la même boîte `kq` et le même vecteur `gq`/`ng`, afin que les tests d'identité et les lectures par nom rendent la même chose.
10. En tant que mainteneur, je veux que le `field` de chaque sorte lise les défauts statiques de sa classe puis de sa parente, afin que `ie:hb`/`ie:kj` lisent `rr`, `s`, `me`, `od` sans spécification parsée.
11. En tant que gardien du compteur, je veux que ce lecteur de sorte porte un nom que le motif `par-nom` compte, sur chaque ligne qui le nomme, afin que la survie d'un lecteur par nom reste visible au compteur jusqu'à D.9.
12. En tant que gardien du compteur, je veux que ce lecteur ne réponde qu'aux quatre noms qu'un consommateur réel lui demande, depuis les données générées, et n'ait aucun appelant hors des sortes, afin qu'il ne soit pas une spécification recopiée en code ni un lecteur de défaut général pour les TU.
13. En tant que gardien du comportement, je veux qu'un nom absent d'une sorte rende 0 et laisse la sortie intacte, et qu'une sorte dont le module n'est pas prêt rende 0, afin que `ie_ke_take_*` et `KIND_HASH` ne voient aucune différence.
14. En tant que mainteneur, je veux que `pico8_ke_start_field` disparaisse, puisque `ke` ne sert aucun des quatre noms dans l'arbre `jo`, afin de fermer le dernier bout de chaîne des lecteurs de spécification.
15. En tant que mainteneur, je veux que `ga.by` soit une `pico8_list_t` statique non `const` vidée à chaque init, afin que le défaut muté de la classe ne soit plus une table parsée puis substituée.
16. En tant que gardien de la régression `ga_export_panel_by`, je veux que le `by` d'un `ga` neuf soit toujours une vraie liste, partagée par la classe, dans laquelle `jo:ib` ajoute, afin que le panneau « exports: » compte toujours les routes.
17. En tant que mainteneur, je veux que `jo.kw` reste `g_jo_kw_shared` et soit remis depuis `pico8_jo_start.kw`, afin que le défaut muté de `jo` n'ait qu'une source.
18. En tant qu'appelant des dix `pico8_X_ready()`, de `pico8_jo_shared_kw()` et des `pico8_X_kind()`, je veux que leurs contrats restent vrais (« 1 de l'init à l'arrêt », « jamais NULL »), afin que `pico8_cart.c`, `jo_kv` et les `assert` d'ordre d'init ne changent pas.
19. En tant que gardien du compteur, je veux que les lecteurs par nom supprimés n'aient aucun remplaçant par nom, ni renommé hors motif, ni `strcmp` non compté, ni registre de TU, afin que la baisse de `spec` ne cache pas un lecteur.
20. En tant que gardien du compteur, je veux `spec` à 0 et `spec` dans `DATA_RESIDUE_GUARD` dans le même commit, afin que `make test` échoue si une ligne `spec` revient.
21. En tant que vérificateur, je veux un test ponctuel qui compare chaque `pico8_X_start` à `pico8_ob()` sur la chaîne exacte de départ, clé par clé et dans les deux sens, afin de prouver qu'aucune clé, aucun genre, aucun bit n'a été perdu.
22. En tant que vérificateur, je veux que le test ponctuel prouve aussi, pour chaque sorte et chacun des quatre noms, que son `field` rend ce que la chaîne des parses rend, afin que le lecteur de sorte soit jugé contre la référence et non contre lui-même.
23. En tant que gardien des TU, je veux que chaque lecture par nom d'un défaut de l'arbre devienne une égalité directe avec la donnée générée, écrite à son site, afin que le test dise ce qu'il vérifie.
24. En tant que gardien des TU, je veux qu'aucun registre ne remplace les lecteurs supprimés (ni table nom → donnée, ni fonction qui rend un défaut par nom, ni `switch` de modules nouveau), afin que les tests ne recréent pas la couche qu'on retire.
25. En tant que gardien des TU, je veux que les vérités des quatre constantes de `test_pico8_jo_spec_defaults` (défauts servis, héritage de la parente, struct neuve qui les reprend, cycle prêt/arrêt) soient reprises, module par module, par des fonctions à leur nom, prouvées à côté de l'ancien registre, afin que supprimer les constantes ne perde rien.
26. En tant que gardien des filets, je veux `make e2e` à 26 conformes, `make sanitize-net` à 26 propres et `make class-coverage-net` identique après le chunk 2, afin de prouver qu'aucun pixel, accès mémoire ni instanciation n'a changé.
27. En tant que gardien de la mémoire, je veux que D.5c n'ajoute aucune vue construite à l'appel ou à la frame ni aucune `pico8_ob_table_new`, afin que les fuites de TROU-7 ne grandissent pas.
28. En tant que gardien du compteur, je veux les comptes avant/après en lignes ET en occurrences par catégorie, par fichier et par chunk, afin qu'une baisse prouve une construction retirée.
29. En tant que lecteur de `docs/glossaire-noms.md`, je veux les lignes `pico8_X_start` étendues à l'arbre `jo`, `pico8_X_kind_field`, `g_ga_by_shared`, et les lignes `jr`, `kv`, `by`, `kw` mises à jour, afin de retrouver un défaut depuis son nom de cart.
30. En tant que `plan-pilot`, je veux une note « Découvert à D.5c » qui donne les comptes réels, la forme du lecteur de sorte, la forme de `ga.by`, la décision sur les chaînes et l'extinction de `spec`, afin que D.6a, D.7a et D.9 partent de faits.
31. En tant que `plan-pilot`, je veux que les clés de construction (`overrides`, D.6a), les ponts et vues de D.2 à D.4b, le typage de `ga.by` (D.7a) et les appels `pico8_pi_init()` (D.7a) restent hors de ces commits, afin que chaque étape reste une étape.

## Implementation Decisions

Pas d'humain dans la boucle. Le skill `to-prd` demande de faire valider par l'utilisateur les modules, les tests et le découpage. Ces choix sont tranchés ici comme le ferait un agent `guideline` (hypothèses explicites, simplicité, changement chirurgical, critères vérifiables), et marqués **(G)**. Les rejets sont notés à chaque décision.

### Forme des défauts générés (G)

- **Un type et un objet par classe**, dans `pico8_game_data.h`/`.c` (module pur : ni `pico8_ob.h`, ni `pico8_value.h`) : `pico8_jo_start_t`/`pico8_jo_start`, `pico8_qh_start_t`/`pico8_qh_start`, … `pico8_mx_start_t`/`pico8_mx_start`. Nom en `_start` (précédent D.5a/D.5b) ; aucun ne correspond à un motif figé.
- **Champs** : exactement les clés propres de la spécification, **sans celles de la parente ni de `ke`**, dans l'ordre de la chaîne, sous le type que porte le membre : `pico8_fixed_t` (non-entier : l'hexadécimal signé des bits qu'`ob()` rendait, règle de D.3, jamais `PICO8_FIXED_LIT`) ; `const char *` ; pour `hf`, `qh`, `pk`, la paire du membre (`pico8_fixed_t X` et `int X_is_bool`, `false` = `{0, 1}`) ; `pico8_vec_t`, `pico8_qz_box_t` **par valeur** (`jo.kq`/`gq`, `ga.kq`, `nr.kq`, `pm.ng`) ; pointeur `const` vers l'objet de D.4a pour chaque table (`nu`, `ka`, `dz`, `cq`, `em`, `px`, `ba`, `k_`, `hz`, `ht`), et `const pico8_kw_t *kw` → `&pico8_jo_kw_start` pour `jo`.
- **Exception, sur le précédent de `ui.le`/`nf.l`/`nf.r` (D.5a)** : `ga.by` (`by=o()`, liste d'état vide) **n'est pas un champ** de `pico8_ga_start_t` ; le test ponctuel prouve qu'il parse en table vide.
- **Chaînes : un tableau distinct par jeton** (G). Chaque défaut chaîne (`jo.me`, `qh.me`/`dq`, `nl.rr`, `hj.rr`, `pm.fz`/`me`, `ga.fz`, `nr.rr`, `fr.rr`, `mx.rr` : onze) est un `static const char <nom>[]` propre de `pico8_game_data.c`, et le champ le pointe. Deux tableaux sont deux objets : aucun pointeur commun ne naît là où `ob()` en rendait deux distincts (TROU-3, section « Faits »). Noms hors de tout motif (par exemple `pico8_qh_me_text`), déclarés un par ligne. Rejetés : **des littéraux** (fusionnés avec `jc.me`/`mz.me` "kz" et `g_.dq` "ly" de la même unité, puis par l'éditeur de liens : `identity_of` et `KE_ME_HASH` bougeraient) ; **corriger au passage l'identité de D.5a/D.5b** (hors étape, aucun effet observé ; signalé en « Surprises »).
- **Rejetés** : une instance `static const` de la struct comme défaut (archétype de la phase 2) ; un tableau de paires nom/valeur (la table ob sous un autre nom) ; garder les statiques et générer à côté (deux sources) ; mettre dans `pico8_X_start` les valeurs **résolues** le long de la chaîne, comme les statiques d'aujourd'hui (une valeur héritée recopiée dans l'enfant : deux sources, et l'héritage ne serait plus celui du cart).

### Constructeurs et membres (G)

- **`jo`** : `pico8_jo_struct_init` pose `me`, `hf`/`hf_is_bool`, `hs`, `fa`, `ma`, `ca` depuis `pico8_jo_start`, `kq` = `&pico8_jo_start.kq`, `gq` = `&pico8_jo_start.gq`, les tables depuis ses pointeurs, `kw` = `&g_jo_kw_shared` et `kw_own = *pico8_jo_start.kw` ; `pico8_jo_init` remet `g_jo_kw_shared = *pico8_jo_start.kw`.
- **`qh`, `nl`, `os`, `hj`, `pm`, `ga`, `nr`, `fr`, `mx`** : chaque `struct_init`/`struct_new` appelle celui de la parente (inchangé), puis pose **les seules clés propres** de sa classe depuis `pico8_X_start` (y compris celles qui surchargent un membre de la parente : `os.hs`, `pm.hp`/`s`/`me`, `ga.hf`/`kq`/`fa`, `nr.kq`/`fp`/`dw`, `mx.hp`/`s`/…) ; les valeurs héritées viennent du `struct_init` de la parente, plus d'un statique résolu.
- **Membres `const`** (H3) : `pico8_jo_t.kq` → `const pico8_qz_box_t *`, `pico8_jo_t.gq` → `const pico8_vec_t *`, `pico8_pm_t.ng` → `const pico8_vec_t *` ; les lectures par nom rendent `pico8_value_ref((void *)membre)` (cast de D.3) ; `jo_field_vec` et les aides internes de `planet.c` prennent `const`. **Repli** si H3 tombe (une écriture à travers, ou `const` qui sort de `planet*.c` et des TU de l'arbre) : ce défaut garde un statique non `const` sans initialiseur, affecté depuis `pico8_X_start` à l'init du module (précédent `g_nf_ra`), nommé au commit.
- **Cycle de vie** : `_init`/`_shutdown`/`_ready` restent ; le pointeur de spécification devient un drapeau `static int` par module au nom hors motif (`g_jo_up`, `g_family_up`, `g_family_trade_up` ; précédent `g_X_up`) ; les six `_ready` de `planet_family` et les trois de `planet_family_trade` rendent le drapeau de leur module ; les `assert` des constructeurs le lisent. Rejeté : supprimer les `_ready` (`pico8_cart.c` et des TU les appellent).
- **`pico8_pi_init()`** : les appels de `pico8_jo_init`, `pico8_planet_family_init`, `pico8_planet_family_trade_init` **restent** (H10 ; `pi` est à D.7a) ; le commentaire d'en-tête de `pico8_jo_init` qui les justifiait par la spécification dit seulement le contrat.
- **Commentaires** : les contrats d'en-tête qui nomment la spécification ou un lecteur supprimé (`planet.h` l. 69-84, `planet_family.h` l. 139-144, `planet_family_trade.h` l. 50-53, `ke.h` l. 24-27, les commentaires de membre « its spec's very vector ») sont réécrits pour dire ce qui reste vrai ou supprimés avec leur code. Quatre sortes de `CLAUDE.md` seulement ; aucun renvoi à D.5c ni au plan dans `src/` ou `tests/`.

### Le `field` des sortes : un vrai lecteur par nom, compté (G)

C'est le point que l'étape doit rendre inattaquable.

- **Forme** : dix fonctions `int pico8_X_kind_field(const char *name, pico8_value_t *out_value)`, X ∈ {`jo`, `qh`, `nl`, `os`, `hj`, `pm`, `ga`, `nr`, `fr`, `mx`}. Chacune : rend 0 si son module n'est pas prêt (contrat des anciens lecteurs) ; pour chacun des quatre noms **que la classe possède en propre**, compare `name` (`strcmp`) et écrit la valeur **lue dans `pico8_X_start`** (`pico8_value_string(pico8_X_start.rr)`, `pico8_value_fixed(pico8_X_start.s)`…) ; sinon rend le lecteur de la parente. `pico8_jo_kind_field` finit la chaîne : `me` seul, puis 0. Signature et `pico8_entity_kind_t` inchangés.
- **Noms répondus, et seulement eux** (23 `strcmp`, le compte que la « Sortie » vérifie) :

  | Lecteur | Noms propres | Puis |
  |---|---|---|
  | `pico8_jo_kind_field` | `me` | 0 |
  | `pico8_qh_kind_field` | `s`, `me`, `od` | `jo` |
  | `pico8_nl_kind_field` | `rr`, `s`, `od` | `qh` |
  | `pico8_os_kind_field` | `s`, `od` | `qh` |
  | `pico8_hj_kind_field` | `rr`, `s`, `od` | `qh` |
  | `pico8_pm_kind_field` | `s`, `me` | `hj` |
  | `pico8_ga_kind_field` | — | `jo` |
  | `pico8_nr_kind_field` | `rr`, `s`, `od` | `qh` |
  | `pico8_fr_kind_field` | `rr`, `s`, `od` | `nr` |
  | `pico8_mx_kind_field` | `rr`, `s`, `od` | `hj` |

- **Pourquoi c'est un vrai lecteur par nom et pas une esquive** : (1) il lit les **données générées**, pas une table parsée ni une table nom/valeur ; (2) il a un **consommateur réel** dans `src/` (`pico8_ie_ke_view`, qui demande ces quatre noms et rien d'autre) ; (3) son nom **commence par `pico8_` et finit par `_field`** : le motif `par-nom` compte sa définition, sa déclaration, chaque initialiseur `{pico8_X_struct_new, pico8_X_kind_field}` et chaque appel de chaîne, comme il compte déjà `->field` chez `ie` ; (4) il est **`static`** dans son module, sauf `pico8_jo_kind_field` (déclaré dans `planet.h`, car `qh` et `ga` le chaînent depuis `planet_family.c`) ; les chaînes d'un fichier à l'autre (`nr` → `qh`, `mx` → `hj`) passent par `pico8_qh_kind()->field` / `pico8_hj_kind()->field`, comptés ; (5) **aucun TU ne l'appelle par son nom** : `test_pico8_entity_kind` et le test ponctuel passent par `kind->field`.
- **Pourquoi quatre noms et pas toute la spécification** : un lecteur complet (≈ 130 branches sur les 126 clés et leurs tables) serait la spécification recopiée en code, sans lecteur, précisément la « table ob sous un autre nom » que D.5b a rejetée ; `test_kind_fields_and_build` (`KIND_HASH`, figé) ne lit que ces quatre noms. Une sorte à qui l'on demande un autre nom rend 0 : c'est un changement de ce qu'une sorte répondrait, **sans aucun demandeur** (H5). Rejeté aussi : **typer la sorte** (un pointeur vers `rr`/`s`/`me`/`od` dans `pico8_entity_kind_t`), qui supprimerait le lecteur — c'est `par-nom`, donc D.9, et la ligne du plan dit « compté par `par-nom` ».
- **Rejeté, critère d'échec** : garder `pico8_X_default` sous un autre nom (hors motif `spec` et lisant encore une table) ; un lecteur de sorte qui sortirait du motif `par-nom` ; un lecteur qui répondrait à un nom hors des quatre ; toute comparaison de nom de défaut ajoutée hors de ces dix corps.

### `pico8_ke_start_field` (G)

- **Supprimé** (`ke.c`, `ke.h`), sans remplaçant : sa seule fonction était de finir la chaîne des `pico8_X_default` ; la chaîne des sortes finit sur `jo`, qui surcharge `me`. `pico8_ke_field` (le `field` du descripteur de `ke`) reste. Le cas `M_KE` disparaît avec le registre de `test_pico8_jo_spec_defaults` ; ce qu'il vérifiait (`ke` sert `me` "ph" et `t` 0) est porté par l'assertion directe sur `pico8_ke_start` qui y est déjà (`test_second_jl_keeps_ke`).

### Défauts mutés : `ga.by`, `jo.kw` (G)

- **`ga.by`** → `static pico8_list_t g_ga_by_shared;` dans `planet_family.c`, `pico8_list_init(&g_ga_by_shared)` dans `pico8_planet_family_init`, `jo.by = &g_ga_by_shared` dans `pico8_ga_struct_new`. C'est le précédent exact de `ui.le` (`g_shared_le`, D.5a). Aujourd'hui chaque init allouait une liste neuve, jamais libérée : les `ga` d'une init nouvelle voient toujours une liste vide ; un `ga` qui survivrait à une ré-init verrait désormais la liste vidée au lieu de l'ancienne (H8, même sémantique que `g_jo_kw_shared` depuis D.4a). Rien n'est libéré à l'arrêt, comme aujourd'hui. Le contenu reste une `pico8_list_t` d'éléments `pico8_value_t` (D.7a). Rejetés : **un tableau fixe d'enregistrements typés** (lecture littérale de la règle commune ; c'est D.7a, avec `jo:ib`, `economy_tiers` et la preuve rouge de `ga_export_panel_by`) ; **garder `malloc` par init** (une allocation de donnée que la règle retire) ; **un accesseur public** (aucun lecteur hors de `planet_family.c` ; les TU comparent deux `ga` neufs).
- **`jo.kw`** : déjà conforme depuis D.4a (`g_jo_kw_shared`, non `const`) ; D.5c ne change que sa source (`*pico8_jo_start.kw`).

### Ce qui remplace les lecteurs par nom (G)

| Lecteur aujourd'hui | Après D.5c | Pourquoi |
|---|---|---|
| `pico8_jo_default` … `pico8_mx_default` (inits des trois modules, chaînes) | **supprimés** ; aucun remplaçant par nom | les inits n'ont plus rien à remplir, les constructeurs lisent `pico8_X_start` |
| `pico8_X_default` comme `field` des neuf sortes | `pico8_X_kind_field`, quatre noms, sur les données générées, compté `par-nom` | la ligne du plan, décision plus haut |
| `pico8_ke_start_field` | **supprimé** | fin de chaîne sans objet |
| lectures de ces défauts dans les TU | égalité directe avec `pico8_X_start.<clé>` (ou l'objet de D.4a), écrite à son site | règle des TU plus bas |

- **Rejeté, critère d'échec** : renommer un `pico8_X_default` hors motif en gardant la recherche par nom, ou le remplacer par une suite de `strcmp` non comptée, dans `src/` ou dans un TU.

### Le générateur (G) : jetable, en C, sur l'arbre de départ

- **Programme C de l'implémenteur, dans son scratchpad, jamais versionné** (précédent D.5a/D.5b), compilé contre `$(DATA_ENGINE_SRCS)` de `e52b1c2`, **avant** toute modification : `pico8_pi_init()`, puis `pico8_ob(s, NULL)` sur les dix chaînes **copiées octet pour octet** des `init`. Il émet l'initialiseur désigné de chaque `pico8_X_start` (un champ par ligne) et les onze tableaux de chaînes.
- **Il échoue fort** si une hypothèse de forme tombe (H2) : clés en nombre ou en ordre autres que ceux de l'inventaire ; genre hors {nombre, chaîne, `false`, vecteur, boîte, table déjà générée par D.4a} ; table déjà générée dont le contenu diffère de l'objet existant ; `false` ailleurs qu'à `hf`/`qh`/`pk` des classes de l'inventaire ; `by` de `ga` non vide.
- Sortie collée dans `pico8_game_data.c`, mise en forme à la main (≤ 120 colonnes, **une déclaration par ligne, un champ par ligne**, comme les objets de D.4a à D.5b), sans valeur éditée. Le message du chunk 1 décrit le générateur en ≤ 15 lignes.

### `Makefile`, docs

- **`Makefile`** (chunk 2 seulement) : `DATA_RESIDUE_GUARD := spec`. Rien d'autre (aucun fichier nouveau).
- `docs/glossaire-noms.md` (chunk 2) : ligne `pico8_X_start` étendue aux dix classes de l'arbre ; ligne nouvelle `pico8_X_kind_field` (« le `field` d'une sorte : `rr`, `s`, `me`, `od` depuis `pico8_X_start`, puis la parente ; fin `pico8_jo_kind_field` ») ; ligne `jr` réécrite (plus aucune spécification parsée ; `pico8_ke_start_field` supprimé) ; lignes `kv`, `by` (sur `ga` : `g_ga_by_shared`), `kw` mises à jour.
- `docs/plan-3.md` : note « Découvert à D.5c » en fin de ligne D.5c, **au chunk 2** (texte dans Further Notes). Rien d'autre dans le plan.

### Ce qui ne change pas

- Aucun pixel, aucune instanciation, aucune condition de jeu, aucun tirage du RNG ; aucune valeur de défaut (le test ponctuel le prouve au bit près) ; aucune identité de chaîne (tableaux distincts).
- Aucun `*.sha256`, aucun fichier de `tests/e2e/`, aucun fichier d'événements ; aucun e2e ajouté.
- Les clés de construction (`overrides`) de l'arbre, dont la parse de `pm:pq` (D.6a) ; le paramètre `overrides` n'est renommé nulle part.
- Les appels `pico8_pi_init()` (D.7a), les ponts et vues de D.2 à D.4b (`jo:gh`, `jo.ex`…), `pico8_entity_kind_t`, `pico8_ie_ke_view` et ses `ie_ke_take_*`.
- `pico8_ob.c/.h`, `pico8_stash.*`, `tools/data_residue.sh` (aucun motif, aucune catégorie, aucune exclusion), `tools/class_coverage_*`, `KNOWN[]`.
- `gh_row_lq`, `ga_export_panel_by`, `layout_nil_item_draws_nothing` : aucun changement de texte.
- **TROU** : aucun écart port/cart touché ni ajouté ; TROU-3 (identités de chaînes) et TROU-7 (instances et listes jamais libérées) préservés. L'`#include "pico8_ref_audit.h"` de `planet_family.c`, sans usage dès aujourd'hui, **reste** (sa suppression serait une baisse `ref-audit` sans rapport).

### Anti-tassement (liant)

Le compteur compte des **lignes**. Une baisse obtenue sans retirer la construction comptée est un échec, quelle que soit la catégorie.

- **Interdit** : joindre sur une ligne des déclarations, instructions, arguments, conditions, entrées d'initialiseur ou `case` qui étaient sur des lignes distinctes ; retirer des retours à la ligne dans une ligne comptée ; déplacer une construction comptée sur une ligne comptée existante ; macro, `typedef` ou fonction enveloppe dont le seul rôle est de renommer un identifiant compté ; **renommer un lecteur par nom hors motif** (dont un lecteur de sorte sans préfixe `pico8_` ou sans suffixe `_field`) ou le remplacer par une recherche `strcmp` non comptée ; **un registre dans un TU** (nouvelle table ou fonction qui associe un nom à un défaut, nouveau `switch`/tableau de modules ou de lecteurs) ; déplacer du code compté vers un fichier que le compteur ne lit pas ; supprimer un commentaire, un `include` ou une ligne qui ne porte pas sur une spécification, un lecteur, une aide, un statique ou un contrat retiré par D.5c ; **changer un motif, une catégorie ou `SPEC_EXCLUSION`** de `tools/data_residue.sh` ; **ajouter une vue construite à l'appel ou à la frame** (`pico8_jb_row_view`, `pico8_ob_table_new`).
- **Mesure exigée, avant et après chaque chunk** : par catégorie, **lignes** (`make data-residue`, et `--detail` par fichier) **ET occurrences** (recette ci-dessous), le test ponctuel donné à part pour `ob`. Recette figée pour l'implémenteur et le vérificateur :

  ```sh
  sed -n '/^pattern() {/,/^}/p' tools/data_residue.sh > "$S/pat.sh"; . "$S/pat.sh"
  X=$(sed -n "s/^SPEC_EXCLUSION='\(.*\)'\$/\1/p" tools/data_residue.sh)
  F=$(find src tests \( -name '*.c' -o -name '*.h' \) -type f | sort)
  for c in valeur ob stash type-tag ref-audit par-nom dispatch overrides; do
    printf '%s\t%s\n' $c "$(cat $F | grep -oE "$(pattern $c)" | wc -l)"; done
  printf 'spec\t%s\n' "$(for f in $F; do sed -E "$X" "$f" | grep -oE "$(pattern spec)"; done | wc -l)"
  ```

  Départ (`e52b1c2`) : `valeur` 6741, `ob` 2152 (dont 149 dans le test ponctuel), `stash` 28, `type-tag` 686, `ref-audit` 70, `par-nom` 1140, `dispatch` 199, `overrides` 479, `spec` 470.
- **Contrôle** : pour chaque catégorie et chaque fichier du diff, Δlignes = avant − après et Δoccurrences = avant − après. **Si Δlignes > Δoccurrences** dans un couple (catégorie, fichier), le vérificateur lit le diff de ce fichier : chaque ligne comptée retirée doit correspondre à une construction retirée ou convertie, pas à une construction qui réapparaît sur une autre ligne. Le message de commit liste ces couples et leur justification.
- **Les lignes qui gagnent une catégorie** sont listées au commit, par sorte. Attendu au chunk 2 : `par-nom` + (dix définitions `pico8_X_kind_field`, une déclaration dans `planet.h`, neuf initialiseurs de sorte, neuf appels de chaîne, les déclarations anticipées éventuelles d'un lecteur `static`, au plus trois lignes de commentaire de contrat, et dans `tests/` la seule ligne `->field` de l'aide locale du test ponctuel) ; `valeur` + (les lignes `pico8_value_string`/`_fixed` des lecteurs de sorte, leurs signatures) ; aucune autre catégorie ne gagne de ligne, hors justification au commit.
- **`strcmp`** : `grep -c strcmp` sur les trois modules, avant et après, écart exactement +23, chaque nouvelle comparaison dans un corps `pico8_X_kind_field` (table plus haut).

### Empreinte remesurée

| Chunk | `src/` | `tests/` | Autres | Total |
|---|---|---|---|---|
| 1 — données | 2 : `game_data.c/.h` | 1 : `test_pico8_map_tables` | — | **3** |
| 2 — conversion | 8 : `planet.c/.h`, `planet_family.c/.h`, `planet_family_trade.c/.h`, `ke.c/.h` (+ `game_data.h` si un type s'ajuste, H3) | 14 : les 13 TU de `spec` (`entity_kind`, `fr`, `ga`, `hj`, `jo`, `jo_spec_defaults`, `mx`, `nl`, `nr`, `os`, `pm`, `qh`, `ui`) et `test_pico8_map_tables` (sortes) | `Makefile`, glossaire, plan | **25** (+1) |
| **Union** | **10** (+0) | **14** | 3 | **27** |

- **Écart avec la ligne du plan (19 : 6, 13)** : les 19 fichiers comptés sont exacts ; le plan ne voyait ni `pico8_game_data.c/.h` (où naissent les données), ni `ke.c/.h` (`pico8_ke_start_field`), ni le test ponctuel, ni le `Makefile` (la garde).
- **Découpage** (G) : l'union est sous le seuil de ~40, mais les données forment un premier commit naturel : elles n'ont aucun lecteur, leur justesse se juge seule contre les parses, et le diff de la conversion en devient lisible. La conversion reste **un seul commit** : les lecteurs de l'arbre se chaînent d'un module à l'autre (`nr` → `qh` → `jo`, `mx` → `hj`), le registre de `test_pico8_jo_spec_defaults` couvre les onze modules, et `spec` = 0 avec la garde exige la dernière ligne retirée dans ce commit. Rejetés : **tout en un** (le générateur et la conversion se jugeraient ensemble) ; **un commit par module** (états intermédiaires où la chaîne mêle lecteurs de spécification et lecteurs de sorte, registre à reprendre trois fois, garde impossible avant le dernier).

### Effet sur `make data-residue`

Attendu (H15), lignes / occurrences :

| Catégorie | Départ | Après chunk 1 | Après chunk 2 | Pourquoi |
|---|---|---|---|---|
| `spec` | 430 ; 470 | **430 ; 470** (exact) | **0 ; 0** (exact), gardé | chunk 2 : −336/−345 dans les modules, −94/−125 dans les TU |
| `par-nom` | 1085 ; 1140 | **1085** (exact) | **≈ 1108**, ≤ 1118, chaque ligne ajoutée classée | − `ke.c:27`, `ke.c:32`, `ke.h:25`, `ke.h:27`, `planet.c:756`, `test_pico8_jo_spec_defaults.c:50/427/435` ; + lecteurs de sorte (≈ 30 à 34 lignes, section « Anti-tassement ») |
| `ob` | 1780 hors test (1922) ; 2003 hors test (2152) | **inchangé hors test** ; le test ponctuel grandit | **< 1780 hors test**, attendu ≈ −55, rapporté | dix pointeurs et parses de spécification, dix libérations, dix `get_name` de lecteur, substitution de `by` (7 lignes) ; registre de `test_pico8_jo_spec_defaults` ; le test ponctuel grandit (parses de référence) |
| `stash` | 28/4 ; 28 | **28/4** | **28/4** | — |
| `overrides` | 450 ; 479 | **450** | **450** (exact) | aucune ligne `overrides` touchée (la parse de `pm:pq` reste) |
| `dispatch` | 197 ; 199 | **197** | **197** (exact) | — |
| `ref-audit` | 69 ; 70 | inchangé | **68** (exact) | le commentaire de la substitution de `by` (`planet_family.c:1445`) ; l'`include` reste |
| `type-tag` | 423 ; 686 | inchangé | rapporté, attendu ≈ −12 | `assert` de `jo_default_box`/`_vec`, `family_default_box`/`_vec`/`_list`, `family_trade_default_box`, `fold_value`/`same_default` du registre |
| `valeur` | 5831 ; 6741 | inchangé | rapporté, attendu en baisse | signatures des dix lecteurs, aides `*_default_*`, `found`/`v` des inits ; + lecteurs de sorte |
| **résidu D** | 8817 | **8817** (exact) | rapporté | — |

- **`spec` = 0 est exact et gardé** : au-delà de 0, ou toute baisse obtenue par tassement : échec. Aucune autre catégorie ne tombe à 0 : `DATA_RESIDUE_GUARD` = `spec` seul.

### Hypothèses (rien n'a été lancé)

- **H1** : `e52b1c2` est vert (`make test`, `make e2e` 26 conformes, `make sanitize-net` 26 propres, `make class-coverage-net` identique, `make data-guard` 0) et `make data-residue` égale l'inventaire. Démenti au départ : arrêt, D.5c ne répare rien.
- **H2** : ce qu'`ob()` produit a la forme de l'inventaire (clés, genres, ordre ; `false` seulement à `qh.hf`, `nl`/`os`/`hj.qh`, `pm.pk`, `ga.hf` ; tables égales aux objets de D.4a ; `by` de `ga` vide ; `kq` boîtes, `gq`/`ng` vecteurs). Le générateur le vérifie. Démentie : le type suit ce qu'`ob()` produit, le test ponctuel juge ; arrêt « champ sans règle » seulement si une clé ne tombe sous aucune forme.
- **H3** : aucune écriture à travers `jo.kq`, `jo.gq`, `pm.ng` (ni à travers ce que leurs lecteurs par nom rendent : `entity_geom` lit `kq`, `msgbox`/`menu` lisent des `gq`) ; le passage à `const` ne sort pas de `planet*.c` et des TU de l'arbre. Démentie : repli « statique non `const` affecté à l'init » pour ce défaut.
- **H4** : aucun lecteur n'atteint un défaut de l'arbre avant l'init de son module ; le drapeau « prêt » reproduit le contrat du pointeur de spécification (`pico8_cart.c:80-84`).
- **H5** : le seul appelant du `field` d'une sorte dans `src/` est `pico8_ie_ke_view`, avec `rr`, `s`, `me`, `od` ; aucun e2e n'atteint une sorte par un autre nom (il n'en existe aucun chemin dans `src/`).
- **H6** : avec un tableau par jeton, aucune comparaison par pointeur (`pico8_value_equal`, `==` de TU) sur une chaîne de l'arbre ne change de résultat ; `test_ke_me_by_name` garde `identity[]` et `KE_ME_HASH`.
- **H7** : durée de vie : aujourd'hui la libération des spécifications est superficielle (boîtes, vecteurs, chaînes et liste de `by` jamais libérés) ; après D.5c, rien de tout cela n'est libéré (données statiques) ; `make sanitize-net` rend 26 propres (`detect_leaks=0`).
- **H8** : la remise de `g_ga_by_shared` à chaque `pico8_planet_family_init` ne change rien d'observable : le jeu n'appelle `pico8_cart_load()` qu'une fois ; les TU qui rechargent ne lisent pas le `by` d'un `ga` construit avant la ré-init (même sémantique que `g_jo_kw_shared` depuis D.4a et `g_shared_le` depuis D.5a, verts).
- **H9** : constantes de hachage : seules `READY_HASH`, `SPEC_HASH`, `SHARE_HASH`, `STRUCT_HASH` de `test_pico8_jo_spec_defaults` disparaissent ; aucune constante des TU `entity_kind` (`KIND_HASH`, `RAW_PRESENCE_HASH`, `BEHAVIOR_HASH`), `ui` (les 21), `qh`, `nl`, `os`, `hj`, `pm`, `ga`, `nr`, `fr`, `mx`, `jo`, `ie` ne bouge. Toute autre constante qui bouge est un arrêt.
- **H10** : les appels `pico8_pi_init()` des trois `init` servent encore (dispatch des méthodes par la table de `pi`) ; ils restent.
- **H11** : les bits émis pour les non-entiers égalent ceux des statiques d'aujourd'hui, remplis par la même parse.
- **H12** : `make class-coverage-net` est identique : aucune instanciation ajoutée ni retirée dans `src/` (les TU ne comptent pas).
- **H13** : `-Werror` reste muet : aides, statiques et variables devenus sans usage supprimés ; casts `(void *)` sur données `const` selon D.3 ; les lecteurs `static` de sorte sont tous référencés par leur sorte ou leur enfant.
- **H14** : `constants-check`, `float-grep`, `float-guard` restent verts : aucun `PICO8_FIXED_LIT` ni flottant ajouté.
- **H15** : `make data-residue` rend les comptes du tableau ; `ob` exact dépend de l'écriture.
- **H16** : durées courtes : `make test` en quelques secondes plus une recompilation large (`game_data.h`), `make e2e` ~1 s, `sanitize-net` et `class-coverage-net` < 2 min chacun, compilation comprise.
- **H17** : `make raylib` (non exigé, jamais lancé) lierait : aucun fichier nouveau.
- **H18** : `test_pico8_ga_export_panel_by` reste vert sans changement et rougit si `g_ga_by_shared` n'est pas une liste (M4).
- **H19** : un lecteur de sorte qui rend 0 laisse `*out_value` intact (l'assertion `0x5A5A` de `test_kind_fields_and_build`).
- **H20** : les chaînes `JO_SRC` … `MX_SRC` du test ponctuel sont octet pour octet celles des `init` de `e52b1c2` (vérifié par lecture pour les dix ; le chunk 1 le reprouve par `diff`).
- **H21** : aucune autre unité que `pico8_ie_ke_view` ne dépend de ce qu'une sorte rend avant l'init de son module (le contrat « 0 si non prêt » est gardé par prudence, sans demandeur connu).

## Implementation Chunks

Deux commits, dans cet ordre ; chacun est vert seul, porte ses comptes avant/après en lignes ET en occurrences, et ses preuves. Les cibles finales (« Sortie de l'étape ») ne s'appliquent qu'au dernier.

1. **D.5c-1 — données générées de l'arbre `jo`.** Générateur (scratchpad, arbre de départ) ; dix types `pico8_X_start_t`, dix objets `pico8_X_start`, onze tableaux de chaînes, dans `pico8_game_data.h/.c` ; test ponctuel étendu (dix classes, deux sens, `by` vide). Aucun lecteur, aucun autre fichier de `src/`. Cibles : `spec` 430/470, `par-nom` 1085/1140, `ob` hors test 1780/2003, toute catégorie hors `ob` du test ponctuel inchangée au chiffre près ; `make test`, `make data-guard`. Filet : `make test` seulement (ni `e2e`, ni `sanitize-net`, ni `class-coverage-net` : aucun comportement ne change, les objets n'ont pas de lecteur).
2. **D.5c-2 — conversion de l'arbre, lecteurs de sorte, extinction de `spec`.** Constructeurs sur `pico8_X_start` ; membres `const` (ou repli) ; drapeaux « prêt » ; `pico8_X_kind_field` et sortes ; `g_ga_by_shared` ; suppression des spécifications, des dix `pico8_X_default`, des remises, aides et statiques, de `pico8_ke_start_field` ; TU convertis ; `test_pico8_jo_spec_defaults` sans registre ; test ponctuel étendu aux sortes ; `DATA_RESIDUE_GUARD := spec` ; glossaire ; note « Découvert à D.5c ». Cibles : `spec` 0, `par-nom` ≈ 1108 (≤ 1118), `ref-audit` 68, `ob` hors test < 1780.

**Repli pour le pilote** : si le chunk 2 dépasse 40 chemins à l'implémentation, `plan-pilot` le recoupe en 2a (TU de l'arbre autres que `jo_spec_defaults` et `ui`, convertis vers les données générées pour leurs valeurs scalaires seulement, lecteurs encore présents) et 2b (le reste, dont la garde) ; 2a ne supprime aucun lecteur et ne change aucune constante.

## Testing Decisions

Un bon test juge ce que le code rend (valeurs, identités, pixels), pas la présence d'une ligne. D.5c change la source des défauts de dix classes et le lecteur des sortes : **la preuve est que les données générées égalent ce qu'`ob()` produisait des chaînes de départ, que chaque struct neuve reprend exactement ces données et hérite exactement de sa parente, que chaque sorte rend aux quatre noms ce que la chaîne des parses rend, que chaque TU qui lisait un défaut par nom fige la même vérité sur la donnée générée, et que tout ce qui mesure le comportement rend la même chose.**

### Le test ponctuel `tests/test_pico8_map_tables.c`, étendu (G)

- **Il construit son état, il ne rejoue rien.** Ses 2 lignes `stash` ne changent pas ; il ne nomme aucun motif de `spec`, d'`overrides` ni de `type-tag`.
- **Chaînes de référence** : `JO_SRC` … `MX_SRC` (D.4a) sont reprouvées octet pour octet contre les `init` de `e52b1c2` (H20), au chunk 1.
- **Chunk 1, pour chaque classe, dans les deux sens** : chaque clé de la table parsée a son champ dans `pico8_X_start` (ou l'objet qu'il pointe) et lui est égale — nombre au bit près, chaîne par `strcmp` et pointeur dans un tableau distinct (aucun champ chaîne de l'arbre n'égale un autre champ chaîne de `pico8_game_data.c` par pointeur), `false` par la paire `{0, 1}`, vecteur et boîte par composante, table déjà générée par identité de pointeur plus contenu (les vérifications de D.4a continuent) — ; et le nombre de clés parsées est le nombre de champs vérifiés (`ga` : plus `by`, table vide).
- **Chunk 2, les sortes** : pour chacune des neuf sortes (un bloc écrit par sorte, pas de table de sortes nouvelle) et chacun des quatre noms, `pico8_X_kind()->field(nom)` rend exactement ce que rend la recherche dans les parses de la chaîne de la classe, dans l'ordre de la chaîne (aide locale du test ponctuel sur ses propres parses, la référence ; elle n'appelle aucun lecteur de `src/`) ; la sorte est interrogée par **une seule** aide locale qui prend la sorte en argument et appelle `kind->field` sur une ligne (une ligne `par-nom`), jamais par un `->field` écrit à chaque assertion : présence, genre, bits, contenu de chaîne ; un nom absent laisse la sortie intacte ; un nom hors des quatre (`"kq"`, `"nu"`, `"t"`) rend 0.

### TU touchés : lire la donnée générée, comparer directement (G)

- **Règle** : chaque lecture par nom d'un défaut de l'arbre devient une égalité directe avec la donnée générée, écrite à son site, sans registre. `class_value`/`class_ref` (lecteurs locaux au test) disparaissent des dix TU de classe. Une boucle sur des noms qui appelait un lecteur (`jo_tables[]`, `nr_tables[]`, `pm_tables[]`, `inherited[]`, `own_methods[]`) devient une suite d'assertions sur les membres, une par ligne ; une absence de méthode dans la spécification (`!pico8_X_default("kj", &v)`) disparaît, portée par le jeu de clés exact du test ponctuel. Un discriminant `default_of == pico8_pm_default` devient le descripteur ou le rang que le test tient déjà.
- **Tableaux de lecteurs existants** (`test_pico8_ui` : `own[10]`, `qh_tree_reader`, `hj_tree_reader`) : chaque entrée devient **la donnée que le test lisait par elle pour le seul nom qu'il lit** (par exemple `own_kq[10]` à côté de `own_nu[10]` qui existe déjà, ou le rang que `qh_tree_rank`/`hj_tree_rank_of` donnent déjà), même index, même longueur ; les comparaisons de chaînes (`dq` pour l'arbre `qh`, `rr` pour l'arbre `hj`) comparent au champ généré écrit au site (`pico8_qh_start.dq` ; `pico8_hj_start.rr` ou `pico8_mx_start.rr` selon le rang) ; `jo_identity` compare `kq`/`gq` à `&pico8_jo_start.kq`/`.gq` par deux branches écrites au site. **Aucune table ni fonction nouvelle qui associe un nom à un défaut** ; les aides de D.4a nommées par membre (`jo_member_data`, `qh_tree_data`, `hj_tree_k_`) ne gagnent aucune entrée.
- **`test_pico8_ui`** : `spec_me` supprimé ; `identity_of` compare à `pico8_jo_start.me`, `pico8_qh_start.me`, `pico8_pm_start.me` ; l. 1799/1866 comparent `gq` à `&pico8_jo_start.gq` ; l. 2511 : le `by` d'un `ga` neuf **est** celui de `e[6]` (partagé par la classe) ; l. 3314/3318 : `ng` **est** `&pico8_pm_start.ng`, `fz` **est** `pico8_pm_start.fz`. Ses 21 constantes ne bougent pas (H9).
- **`test_pico8_entity_kind`** (par son aide existante `kind_field`, sans nouvelle ligne `->field`) : `default_of` et `test_kind_reader_is_the_module_default` deviennent une vérification, sorte par sorte, que le `field` de la sorte rend les champs de `pico8_X_start` de la classe qui possède chaque nom (chaîne **identique** au pointeur généré, fixes égaux), écrite à son site ; `KIND_HASH`, `RAW_PRESENCE_HASH`, `BEHAVIOR_HASH` ne bougent pas.
- **Les dix TU de classe** (`jo`, `qh`, `nl`, `os`, `hj`, `pm`, `ga`, `nr`, `fr`, `mx`) : `kq`/`gq`/`ng` d'une struct neuve **sont** les champs de `pico8_X_start` (identité, hier « le vecteur de la spécification ») ; les défauts hérités égalent les champs de la parente (`nl.dq` **est** `pico8_qh_start.dq`) ; `fr.mt` égale `pico8_fr_start.mt` ; `ga.by` d'un `ga` neuf est celui d'un autre `ga` neuf ; en-têtes de fichier réécrits sans nommer de lecteur.
- **`test_pico8_jo_spec_defaults`** : l'enum, `PARENT`, `spec_field`, `is_ready`, `build`, `shutdown_of`, `NAMES`, `fold_value`, `same_value`, `generated`, `answering`, `positional_ints_equal`, `tables_equal`, `row_equal`, `assert_same_content`, `same_default`, `fold_ready`, `test_spec_defaults`, `test_shared_with_parent`, `test_struct_defaults` et les quatre constantes disparaissent. **Une fonction à son nom par module** (`jo`, `qh`, `nl`, `os`, `hj`, `pm`, `ga`, `nr`, `fr`, `mx`), sans boucle ni table de modules, asserte : la struct neuve (`pico8_X_new(NULL)`, comme `build()` hier) pose chaque clé propre égale au champ de `pico8_X_start` (pointeurs : identité) ; chaque défaut hérité égale le champ de la parente qui le possède (l'intention de `SHARE_HASH`), `me` compris (`ga` : `pico8_jo_start.me`) ; son `_ready` vaut 1 après `pico8_cart_load()`/`pico8_jl()` et 0 après l'arrêt de son module. `main` asserte, avant `pico8_cart_load()`, que chacun des dix est non prêt (une assertion par module, écrite). `test_second_jl_keeps_ke` asserte `pico8_ke_ready()` avant et après le second `jl()` et, pour chaque module, qu'une struct neuve garde `ke.t == pico8_ke_start.t` (une assertion par module) ; l'assertion sur `pico8_ke_start` reste. En-tête réécrit.
- **Sonde de la suppression des quatre constantes** (au lieu d'une reprise) : (a) sur l'arbre du chunk 1, les dix fonctions nouvelles, en **mode valeur** (chaînes par `strcmp`, vecteurs et boîtes par composante, tables de D.4a par identité), passent **en même temps que l'ancien registre**, ses quatre constantes inchangées ; (b) après le passage des constructeurs, elles gagnent les assertions d'identité, et le registre est supprimé dans le même pas. Le rapport de `make test` sur (a) figure au commit.
- **`test_pico8_ga_export_panel_by`** : inchangé (§6).

### Preuve des tests (§7, gcov + mutations)

- **gcov** (copie jetable du scratchpad, `--coverage`) au chunk 2 : chaque ligne ajoutée ou modifiée de `game_data.c`, des constructeurs, des lecteurs de sorte et des `init`/`shutdown` de l'arbre est exécutée par un TU ; les lignes atteintes seulement par les e2e sont justifiées au commit.
- **Mutations**, chacune doit rougir un test, puis être annulée :
  - chunk 1 — **M1** : un bit de `pico8_hj_start.ro` changé → test ponctuel rouge ; **M2** : `pico8_qh_start.hf_is_bool` à 0 → test ponctuel rouge ; **M3** : `pico8_qh_start.me` pointe le tableau de `jo` → test ponctuel rouge ;
  - chunk 2 — **M4** : `pico8_list_init(&g_ga_by_shared)` retiré (étiquette nulle) → `test_pico8_ga_export_panel_by` rouge ; **M5** : `pico8_nl_kind_field` saute son `rr` → `test_pico8_entity_kind` (`KIND_HASH`) et test ponctuel rouges ; **M6** : `pico8_jo_kind_field` ne répond plus à `me` → `test_pico8_entity_kind` rouge (`me` de `ga`) ; **M7** : `pico8_ga_struct_new` pose `kq` = `&pico8_jo_start.kq` → `test_pico8_ga` ou `test_pico8_ui` rouge ; **M8** : le constructeur de `os` ne pose plus `hs` (hérite 1 de `jo` au lieu de 0.5) → `test_pico8_os` ou `test_pico8_jo_spec_defaults` rouge ; **M9** : `pico8_jo_init` ne remet plus `g_jo_kw_shared` → un TU de `jo` rouge ; **M10** : `qh.me` écrit en littéral `"kz"` au lieu de son tableau → `test_pico8_ui` (`test_ke_me_by_name`) rouge, preuve de la décision sur les chaînes.
- Une mutation qui ne rougit pas est un trou, justifié au commit ou couvert par une assertion ajoutée.

### Pas de nouvel e2e (G)

Les lignes changées sont atteintes par les 26 e2e et par les TU ; les lecteurs de sorte par `ie:hb`/`ie:kj` et `test_pico8_entity_kind`. Un e2e n'est admis que s'il ajoute de la couverture de lignes en moins de 180 frames. Aucun n'est ajouté.

### Vérification, budget et tableau « Runs »

**Budget : environ 4 minutes pour le chunk 1, 15 minutes pour le chunk 2, 20 minutes pour l'étape.**

| Cible | Durée attendue |
|---|---|
| générateur (scratchpad, arbre de départ) | < 1 min, compilation comprise |
| chunk 1 : `make test`, `make data-guard`, `make data-residue`, recette | < 2 min |
| `make test` (×3 au chunk 2 : sonde (a) ; constructeurs et lecteurs de sorte ; TU convertis et garde) | quelques secondes chacun, plus une recompilation large |
| `make e2e` | ~1 s |
| `make sanitize-net` | < 2 min (H16) |
| `make class-coverage-net` | < 2 min (H16) |
| gcov + mutations (scratchpad) | < 5 min |
| `make data-guard`, `make data-residue`, recette d'occurrences, `grep` de sortie | < 1 s |

Ordre conseillé au chunk 2 : mesure de départ (lignes, occurrences, `strcmp`, `pico8_jb_row_view`) ; fonctions nouvelles de `test_pico8_jo_spec_defaults` en mode valeur, `make test` à côté de l'ancien registre (sonde (a)) ; constructeurs, drapeaux, lecteurs de sorte, `g_ga_by_shared`, suppressions, `make test` ; TU convertis, registre retiré, sortes au test ponctuel, `DATA_RESIDUE_GUARD := spec`, `make test` ; `make e2e`, `make sanitize-net`, `make class-coverage-net` une fois ; gcov, mutations ; mesure d'arrivée, contrôle anti-tassement, `grep` de sortie, glossaire, note du plan.

Au-delà du budget, l'implémenteur s'arrête et note ce qui manque, plutôt que de relancer. **Jamais l'appli raylib.** `make raylib` n'est pas exigé. Aucun TU ne rejoue de fichier d'événements.

**L'implémenteur tient un tableau « Runs »** dans son journal de progression, une ligne par lancement :

| # | Commande exacte | État de l'arbre (HEAD + chunk + « sonde » / « constructeurs » / « TU »…) | Code de sortie | Ligne clé | Durée |
|---|---|---|---|---|---|

Le vérificateur réutilise ce tableau et ne relance que ce qui suit le dernier changement de l'arbre.

**Cibles finales de chaque chunk**, vertes sur l'arbre du chunk :

- chunk 1 : `timeout 5m make test` ; `make data-guard` : code 0 ; `make data-residue` + recette : `spec` 430/470, `par-nom` 1085/1140, `ob` hors test 1780/2003, résidu D 8817, toute autre catégorie identique au départ en lignes et en occurrences.
- chunk 2 : `timeout 5m make test` ; `timeout 5m make e2e` : **26 « conforme »** ; `timeout 10m make sanitize-net` : `sanitize: 26 propre(s)` ; `timeout 10m make class-coverage-net` : identique à `tools/class_coverage_net_expected.txt` ; `make data-guard` : code 0 avec `DATA_RESIDUE_GUARD = spec` ; `make data-residue` + recette : `spec` absent (0/0), `stash` 28/4, `overrides` 450, `dispatch` 197, `ref-audit` 68, `par-nom` ≤ 1118 (lignes ajoutées classées), `ob` hors test ponctuel < 1780 ; `grep` de la « Sortie de l'étape » conformes ; contrôle anti-tassement passé.

### Contrôles du vérificateur

1. **Périmètre** : `git show --stat` de chaque chunk liste les chemins de sa ligne « Empreinte remesurée » (écart ≤ 3 expliqué), rien d'autre. Chunk 1 : `game_data.c/.h` et le test ponctuel seulement. Aucun `*.sha256`, rien sous `tests/e2e/`, aucun fichier d'événements ; `Makefile` : la seule ligne `DATA_RESIDUE_GUARD := spec` (chunk 2) ; `tools/` inchangé ; `gh_row_lq`/`ga_export_panel_by`/`layout_nil_item_draws_nothing` intacts.
2. **Données** : un `pico8_X_start` par classe de l'arbre, dans `pico8_game_data.c`, initialiseur désigné, un champ par ligne, non-entiers en hexadécimal signé, chaînes dans onze tableaux distincts ; `pico8_game_data.*` n'inclut ni `pico8_ob.h`, ni `pico8_value.h`.
3. **Une seule source** : aucun constructeur ni `init` de l'arbre ne pose plus un statique ou un littéral qui duplique une clé de sa classe, ni une valeur héritée résolue ; `grep` de la « Sortie » vide.
4. **Lecteur de sorte** : dix `pico8_X_kind_field`, nom au motif `par-nom`, neuf `static` et `pico8_jo_kind_field` déclaré ; chacun ne lit que `pico8_X_start` de sa classe et chaîne sa parente ; exactement les noms de la table ; aucun appel par nom depuis `tests/` ; `strcmp` +23 exactement.
5. **`pico8_ke_start_field`** absent de `src/` et `tests/`.
6. **Mémoire** : aucune donnée générée libérée ; `g_ga_by_shared` statique, vidé à l'init, jamais libéré ; `pico8_jb_row_view` : 18 sites, inchangé ; aucune `pico8_ob_table_new` ajoutée.
7. **Anti-tassement** : les comptes avant/après, en lignes ET en occurrences, par catégorie (et par fichier pour `--detail`), figurent à chaque commit et sont recalculés par le vérificateur avec la recette ; chaque couple où Δlignes > Δoccurrences est justifié et relu ; chaque ligne `par-nom` ajoutée est classée ; aucune jonction de lignes, aucun alias d'identifiant compté, aucun lecteur renommé hors motif, aucun registre de TU, aucun déplacement hors du compteur, aucun commentaire ou `include` sans rapport supprimé.
8. **TU** : plus aucune lecture par nom d'un défaut de l'arbre hors `kind->field` de `test_pico8_entity_kind` et du test ponctuel ; comparaisons écrites à leur site ; les quatre constantes supprimées avec la sonde (a) au commit ; aucune autre constante ne bouge.
9. **Garde** : `DATA_RESIDUE_GUARD := spec` dans le commit où `spec` atteint 0 ; `make data-guard` l'exécute.
10. **Commentaires** : quatre sortes de `CLAUDE.md` ; aucun récit de chantier.
11. **Docs** : glossaire à jour au chunk 2 ; note « Découvert à D.5c » en fin de ligne D.5c au chunk 2, rien d'autre dans le plan.

### Critères d'échec (traduits en `FAILED:` par le vérificateur de `prd-autopilot`)

**Départ et filet (§7)**

- `FAILED:` si l'arbre de départ n'est pas `e52b1c2` (ou un commit de docs au-dessus, ou le chunk précédent) propre, ou si `make data-residue` n'y rend pas l'inventaire.
- `FAILED:` si le filet (`make test`, et au chunk 2 `make e2e`, `make sanitize-net`, `make class-coverage-net`) reste rouge après **3 tentatives**.
- `FAILED:` si un `*.sha256` est modifié ou supprimé, si un fichier de `tests/e2e/` est touché, si un e2e est ajouté ou retiré, ou si un TU rejoue un fichier d'événements.
- `FAILED:` si `make e2e` ne rend pas **26 « conforme »**, si `make sanitize-net` ne rend pas 26 propres, ou si `make class-coverage-net` diffère de sa référence.
- `FAILED:` (arrêt « champ sans règle », §7) si une clé d'une des dix spécifications ne tombe sous aucune forme de la section « Forme des défauts générés ».

**Périmètre**

- `FAILED:` si une spécification de l'arbre est encore parsée, si un `pico8_X_default`, `*_defaults_reset`, aide `*_default_<genre>`, statique recopié ou `pico8_ke_start_field` subsiste au chunk 2 (hors repli de H3 nommé au commit), ou si une ligne du `grep` de la « Sortie de l'étape » subsiste.
- `FAILED:` si une valeur générée a été écrite ou corrigée à la main, si un générateur est versionné, si `pico8_game_data.*` inclut `pico8_ob.h` ou `pico8_value.h`, ou si un défaut chaîne de l'arbre est un littéral au lieu d'un tableau distinct.
- `FAILED:` si un constructeur ou un `init` de l'arbre garde un littéral ou un statique qui duplique une clé de sa classe, ou si un `pico8_X_start` porte une clé héritée.
- `FAILED:` si le diff convertit une clé de construction (`overrides`, dont la parse de `pm:pq`), un appel `pico8_pi_init()`, un pont ou une vue de D.2 à D.4b, `pico8_entity_kind_t`, `pico8_ie_ke_view`, ou touche un TROU.
- `FAILED:` si le diff ajoute un appel à `pico8_jb_row_view` ou une `pico8_ob_table_new`, ou une table ob construite à chaque appel ou à chaque frame.
- `FAILED:` si le chunk 1 touche un fichier de `src/` autre que `pico8_game_data.c/.h`, ou un TU autre que le test ponctuel.
- `FAILED:` si le test ponctuel nomme un motif de `spec`, d'`overrides` ou de `type-tag`, ou porte plus de 2 lignes `stash`.

**Lecteur de sorte (liant)**

- `FAILED:` si le `field` d'une sorte lit autre chose que les champs de `pico8_X_start` (table parsée, table nom/valeur, statique recopié), s'il répond à un nom hors de la table de la section « Le `field` des sortes », ou si son nom ne correspond pas au motif `par-nom` (`\bpico8_[a-z0-9_]+_(set_)?field(_fn)?\b`).
- `FAILED:` si un lecteur de sorte est appelé par son nom depuis `tests/`, ou depuis `src/` ailleurs que par sa sorte et par la chaîne de ses enfants, ou si `pico8_jo_kind_field` n'est pas le seul déclaré dans un en-tête.
- `FAILED:` si `grep -c strcmp` sur `planet.c`, `planet_family.c`, `planet_family_trade.c` ne gagne pas exactement 23, ou si une comparaison de nom de défaut est ajoutée hors des dix corps `pico8_X_kind_field`.

**Gardes, comptes et anti-tassement**

- `FAILED:` si `make data-guard` n'est pas vert, si `DATA_RESIDUE_GUARD` ne vaut pas `spec` au chunk 2 ou change au chunk 1, ou si la garde n'arrive pas dans le commit où `spec` atteint 0.
- `FAILED:` si `tools/data_residue.sh` change (motif, catégorie, `SPEC_EXCLUSION`, mode).
- `FAILED:` si le chunk 1 ne rend pas exactement `spec` 430/470, `par-nom` 1085/1140, `ob` hors test 1780/2003 et résidu D 8817, toute autre catégorie inchangée.
- `FAILED:` si le chunk 2 ne rend pas `spec` 0 (lignes et occurrences), `stash` 28/4, `overrides` 450, `dispatch` 197, `ref-audit` 68 ; si `par-nom` dépasse 1118 ou si une ligne `par-nom` ajoutée n'est pas classée ; si `ob` hors test ponctuel ne baisse pas strictement.
- `FAILED:` **si un compte baisse par tassement** : lignes jointes, retours à la ligne retirés dans une ligne comptée, construction comptée déplacée sur une autre ligne comptée, alias (macro, `typedef`, enveloppe) d'un identifiant compté, code déplacé hors de `src`/`tests` ou hors `.c`/`.h`, commentaire ou `include` sans rapport supprimé.
- `FAILED:` si un lecteur par nom d'un défaut de l'arbre survit sous un nom qu'aucun motif ne compte (renommage, recherche `strcmp` non comptée), dans `src/` ou dans un TU.
- `FAILED:` si un commit ne donne pas, par catégorie, les comptes avant/après **en lignes et en occurrences**, ou si un couple (catégorie, fichier) où Δlignes > Δoccurrences n'est pas justifié.
- `FAILED:` si un TU garde une lecture par nom d'un défaut de l'arbre (hors `kind->field` de `test_pico8_entity_kind` et du test ponctuel), ou si ses comparaisons passent par un registre nouveau (table ou fonction nom → défaut, `switch` ou tableau de modules ou de lecteurs ajouté) ou par un lecteur par nom local au test (`class_value`, `class_ref`).
- `FAILED:` si `make constants-check`, `float-grep` ou `float-guard` rougit, ou si `tools/float_guard.txt` gagne une ligne.

**Preuve, commit et note**

- `FAILED:` si une constante `*_HASH` (ou `identity[]`, `want[]`) change hors des quatre de `test_pico8_jo_spec_defaults`, ou si ces quatre sont supprimées sans la sonde (a) au commit.
- `FAILED:` si le rapport gcov ou une mutation manque, ou si une mutation ne rougit pas sans justification au commit.
- `FAILED:` si un chunk n'est pas un commit vert seul, ou si les chunks ne suivent pas l'ordre 1, 2.
- `FAILED:` si le budget est dépassé sans note de ce qui n'a pas été lancé, s'il manque le tableau « Runs », ou si le message de commit ne porte pas les comptes avant et après et (chunk 1) la description du générateur.
- `FAILED:` s'il manque au chunk 2 la note « Découvert à D.5c », ou si elle omet l'extinction de `spec` et sa garde, la forme du lecteur de sorte et son compte `par-nom`, la forme de `ga.by`, la décision sur les chaînes ou la suppression de `pico8_ke_start_field`.
- `FAILED:` si un commentaire ajouté ou reformulé sort des quatre sortes de `CLAUDE.md`.

## Out of Scope

- **Les clés de construction** (`overrides`) de l'arbre `jo`, dont la parse `pico8_ob(…)` de `pm:pq` et `new_entity(overrides)` des sortes : D.6a.
- **Le typage de `ga.by`** (enregistrements de `jo:ib`) et de ses lecteurs (`economy_tiers`, `economy_score`, `planet_trade`) : D.7a, avec la preuve rouge de `ga_export_panel_by`.
- **Le typage de la sorte** (supprimer son lecteur par nom) et tout `par-nom` : D.9.
- **Les vues à l'appel** de D.4a (`jo:gh`, `jo.ex`) et les fuites de TROU-7 : D.7a.
- **Les appels `pico8_pi_init()`** : D.7a.
- **L'identité des chaînes générées par D.5a/D.5b** (`jc.me`/`mz.me` "kz", `g_.dq` "ly") : signalée, non traitée.
- Les archétypes résolus le long de la chaîne (`mx` → `hj` → `qh` → `jo` → `ke`) : phase 2.
- `make raylib`, l'appli raylib, l'audio, le rendu, `make e2e-gap` et `tools/line_coverage_reference.txt`.

## Further Notes

- **Décisions de l'agent à la place de l'utilisateur** (consigne `to-prd` : vérifier modules, tests et découpage avec l'utilisateur), tranchées en `guideline` et marquées (G) plus haut :
  - **forme** : un `pico8_X_start` `static const` par classe de l'arbre, clés propres, types de la struct, tables par l'objet de D.4a, `kw` par `pico8_jo_kw_start` ; `ga.by` hors du type (précédent `ui.le`) ; chaînes dans onze tableaux distincts (TROU-3) ; valeurs résolues le long de la chaîne rejetées.
  - **une seule source** : constructeurs sur `pico8_X_start`, héritage par le `struct_init` de la parente ; statiques, remises et aides supprimés ; `kq`/`gq`/`ng` en `const` (repli : statique non `const` affecté à l'init).
  - **lecteur de sorte** : `pico8_X_kind_field`, dix fonctions, quatre noms (`rr`, `s`, `me`, `od`), données générées, chaîne jusqu'à `jo`, nom au motif `par-nom`, neuf `static` ; lecteur complet et sorte typée rejetés.
  - **`pico8_ke_start_field`** : supprimé sans remplaçant.
  - **défauts mutés** : `ga.by` → `g_ga_by_shared` (liste statique non `const`, précédent `ui.le`), typage à D.7a ; `jo.kw` déjà conforme.
  - **tests** : test ponctuel étendu (chunk 1 : données, deux sens ; chunk 2 : sortes contre la chaîne des parses) ; TU en comparaisons directes, tableaux de lecteurs convertis en données au même index, sans registre nouveau ; registre de `test_pico8_jo_spec_defaults` et ses quatre constantes supprimés, vérité reprise par dix fonctions à leur nom, sonde (a) ; dix mutations, dont M4 pour la garde de `ga_export_panel_by` et M10 pour les chaînes ; pas d'e2e ; `class-coverage-net` et `sanitize-net` au chunk 2.
  - **découpage** : deux chunks (données, puis conversion et garde) ; repli 2a/2b.
  - **anti-tassement** : comptes en lignes ET en occurrences, par catégorie, fichier et chunk ; lignes `par-nom` ajoutées classées ; `strcmp` +23 exactement ; interdits nommés, dont le registre de TU, le lecteur renommé hors motif et la modification de l'outil.
- **Note du plan (G)**, à ajouter au chunk 2 en fin de ligne D.5c (le §7 ne garde que ce qui change une étape suivante) :

  > Découvert à D.5c : deux chunks (données, puis conversion et garde ; ≈ 27 chemins, `pico8_game_data.*`, `ke.c/.h`, le test ponctuel et le `Makefile` hors du majorant). Défauts générés `pico8_X_start` des dix classes de l'arbre, clés propres, lus par les constructeurs, l'héritage par le `struct_init` de la parente ; chaînes dans des tableaux distincts, un par jeton (des littéraux se fusionneraient, TROU-3). `jo.kq`/`gq`, `pm.ng` : membres `const` vers la donnée générée. Le `field` des sortes est `pico8_X_kind_field` : `rr`, `s`, `me`, `od` seulement, lus dans `pico8_X_start`, puis la parente jusqu'à `pico8_jo_kind_field` ; compté par `par-nom` (P après D.5c), à typer par D.9. `pico8_ke_start_field` supprimé. `ga.by` : `g_ga_by_shared`, `pico8_list_t` statique non `const` vidée à chaque init (précédent `ui.le`), contenu à typer par D.7a. `spec` après D.5c : 0, dans `DATA_RESIDUE_GUARD`. `ob` : O hors test ponctuel.

  P est le compte `par-nom` réel, O le compte `ob` hors test ponctuel réel. Les noms sont ceux de l'implémentation (et un repli de H3 y est nommé s'il a servi).
- **Contrôle d'étape** : un sous-agent frais relit le diff de chaque chunk contre ce PRD avant son commit (hors budget de tests), anti-tassement et lecteur de sorte compris.
- **Le message de commit** de chaque chunk porte : le périmètre ; (chunk 1) le générateur en ≤ 15 lignes ; (chunk 2) la forme par classe et ses rejets, la table des lecteurs de sorte, la forme de `ga.by`, la décision sur les chaînes ; les comptes avant et après **en lignes et en occurrences** par catégorie, le compte `ob` hors test, `strcmp` avant/après, les couples Δlignes > Δoccurrences justifiés ; les lignes qui gagnent une catégorie, classées ; la sonde des constantes supprimées ; le rapport gcov et les mutations ; le tableau « Runs » résumé ; les réponses aux hypothèses qui le concernent.
- **Surprises pour le pilote** :
  - **Le `field` des sortes n'a qu'un demandeur et quatre noms** (`pico8_ie_ke_view` : `rr`, `s`, `me`, `od`) : c'est ce qui permet un lecteur de sorte petit, honnête et compté, au lieu d'une spécification recopiée en code.
  - **`pico8_ke_start_field` ne sert à rien dans la chaîne des sortes** : `jo` surcharge `me`, et `ke` n'a aucun autre des quatre noms. Sa suppression ne déplace rien.
  - **Piège des chaînes** : `pico8_game_data.c` porte déjà `"kz"` deux fois (`jc.me`, `mz.me`, D.5a) et `"ly"` (`g_.dq`, D.5b). Un `qh.me` en littéral serait fusionné avec eux et retournerait `identity_of` de `test_ke_me_by_name` (et `KE_ME_HASH`). D'où les tableaux distincts, et M10 pour le prouver. Les littéraux de D.5a/D.5b sont peut-être déjà fusionnés entre eux ; aucun test n'en dépend aujourd'hui, et ce n'est pas l'étape pour y toucher.
  - **Le correctif de `ga_export_panel_by` disparaît avec la spécification de `ga`** (la substitution du `by=o()`) : la ligne dit « §6 : aucun », et c'est vrai pour le texte du test, mais sa garde doit être reprouvée (M4).
  - **Les statiques de l'arbre tenaient des valeurs résolues le long de la chaîne** (`g_hj_hp`, `g_nl_qh`…) : les remplacer par les clés propres fait passer l'héritage par les `struct_init` des parentes, qui le faisaient déjà ; le test ponctuel et les fonctions de `test_pico8_jo_spec_defaults` jugent les deux côtés.
  - **`par-nom` monte d'environ 23 lignes** : c'est le coût honnête du lecteur que la ligne garde ; D.9 le fera tomber avec le typage des sortes.
