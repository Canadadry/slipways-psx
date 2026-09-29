---
title: "Plan 3, étape D.11a : les lectures et écritures par nom sur un receveur de type connu (classe A) deviennent des accès aux membres typés, derrière un cast vérifié vers la famille que le cart fixe ; les singletons `fm`/`je`/`mo` (classe D) passent par `pico8_fm_hy()`, `pico8_je_nf()`, `pico8_mo_qb()` (NULL sur le remplaçant, Q8) ; `je.l`/`je.r` (E) par `pico8_nf_l()`/`pico8_nf_r()` ; les sites G `slipway.c` (`mt`) et `research_icon.c` (`ie.gy`) ; les aides H devenues sans appelant supprimées ; deux chunks a1/a2"
description: "Après D.10b, `par-nom` compte encore les lectures par nom que le code du jeu fait sur un receveur dont le type est connu (bouton, planète, route, voie, économie, caméra, lecteur de souris) : D.11a les remplace par l'accès au membre typé, sans changer un pixel, et supprime les aides par nom qui n'ont plus d'appelant ; `par-nom` `src/` passe d'environ 418 à environ 342."
status: needs-triage
---

## Problem Statement

**Écrit pendant que D.10b est implémentée** (`plan-pilot`, étape 2b). D.10b n'est pas gradée. Tout ce qui suit a été lu et mesuré sur l'arbre **commis** au commit du PRD de D.10b, `ba4768d` (`543f484`, D.10a gradée, plus le commit qui met le PRD de D.10b en file), extrait par `git archive origin/claude/plan-pilot-3-7zdft0` dans le scratchpad : `grep`, `sh tools/data_residue.sh` (avec `--detail`) et la recette d'occurrences de D.8 seulement. **Rien n'a été compilé ni lancé.** Chaque résultat de D.10b dont ce PRD dépend est une hypothèse `H-D10b-n` (plus bas), et la section « Sections à revérifier » dit quoi relire quand D.10b sera gradée. Les numéros de ligne sont ceux de `ba4768d` ; D.10b touche `menu.c`, `planet_display.c`, `msgbox.c`, `score_panel.c`, `score_panel_display.c` (et d'autres fichiers hors de l'empreinte de D.11a) : leurs numéros bougeront (⟲).

**D.10c** : le PRD de D.10b conclut que D.10c devient vide (ses lignes `pico8_mv` sont parties à D.10a, ses lignes `pico8_value_function` à D.10a et D.10b, sans pont). Si la note « Découvert à D.10b » le confirme (H-D10b-1), **D.11a suit directement D.10b** : le pilote constate D.10c sans commit de code, puis D.11a est l'étape suivante.

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Phase D — Données ») :

> - D.11a — `par-nom`, receveurs de type connu et singletons : classe A (41 lignes, 36 appels d'aides ; cast vérifié vers la famille que le cart fixe), classe D (8, 21 ; `pico8_fm_hy()`, `pico8_je_nf()`, `pico8_mo_qb()`, NULL sur le remplaçant, Q8), E `je.l`/`je.r` (`pico8_ui_je_button_table`), et les aides H (39) devenues sans appelant. Empreinte ≈ 22 (22, 0). Compte : `par-nom` ≈ 910. §6 : aucun. `class-coverage-net` : seulement si le diff touche un constructeur. `sanitize-net` : non.

Lignes voisines, recopiées (D.10a, gradée, avec sa note « Découvert à D.10a » ; D.10b et D.10c, non gradées ; D.11b à D.11e, qui reçoivent ce que D.11a ne prend pas) :

> - D.10a — `dispatch`, table de méthodes : `methods[]` → struct de pointeurs typés, 23 emplacements nommés (Q2), et table d'états typée de `dg` (`ph`, `ly`, `ll`, `kz`, `pr` ; Q3) ; les 82 adaptateurs `(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1)` retirés ; les 26 appels `pico8_mv` de `src/` remplacés (appel direct de la fonction typée pour `eb`, `ib`, `gg`, `md`, `lq`, `lh` ; virtuel pour le reste) ; `dg`, `dc`, `ui:jz`, `lt`, `he` et les prédicats `*_is_*_class` sur les emplacements ; `pico8_entity_get_field` ne rend plus de méthode ; `hd=1` en fonction constante vraie ; `pico8_dispatch_context` gardé (Q4) ; sites `par-nom` G (9) et E de `mv` (1). Deux chunks : a1 (méthodes lancées par `dc`, `lt`, `he`, `ui:jz`, `dg`), a2 (le reste). Empreinte 37 (37, 0) aujourd'hui. Compte : `dispatch` `src/` ≤ 35 (67 moins les 29 lignes `pico8_mv`, `entity.c:177`, `methods[]` de `pico8_struct.h` et le commentaire de `pico8_dispatch_context.h`), ≤ 37 si `pico8_mv` reste en pont pour les TU ; `par-nom` ≈ 999 ; `valeur` rapporté. §6 : aucun. `class-coverage-net` : oui. `sanitize-net` : non. Découvert à D.10a : deux chunks (a1 : 73 chemins ; a2 : 62). `pico8_struct_class_t` porte 14 membres typés (`kj`, `ev`, `hd`, `hk`, `hb`, `ldown`, `rdown`, `gv`, `hq`, `ey`, `kf`, `cn`, `ol`, `mm`), la table `states[]` (`pico8_state_t`, `pico8_state_of`) et le crochet `has_hb` (présence de `hb` par instance, pour `js`) ; `ldown`/`rdown`/`gv` rendent `pico8_down_t {stop, held}` ; `eb`, `ib`, `gg`, `md` sont appelés directement (`pico8_jo_is_family`) ; `hd=1` est `pico8_hd_true` ; `pico8_ui_jz` prend un sélecteur. Pas de pont : les TU ont suivi (les lignes `pico8_mv` et les lignes `pico8_value_function` de méthode, `fns[]` et aides `method()` compris) ; D.10c ne garde que les rappels des TU (9 lignes : `fq` 3, `js` 4, `mh` 1, `pj` 1, plus ce que D.10b y laisse) et perd ses lignes `pico8_mv`. D.10b hérite des 19 lignes d'adaptateurs de rappel (dont `pico8_hp_lq_dispatch`, `pico8_jo_fe_my_dispatch`) et des membres `my`/`hb`/`lh`/`lq`, appelés directement depuis D.10a ; le crochet `has_hb` survit au typage de `js.hb` ; le commentaire de `pico8_value_function_t` (`pico8_value.h`, « appelé par dc()/mv() ») est à réécrire avec lui. Sites G laissés : `ui_button.c:599` (D.10b), `slipway.c:261` et `research_icon.c:182` (D.11a), `ke.c:43` (D.11c) ; pris à D.11b (classe C) : la présence des noms `bm` dans `lt`/`he` et de `gv` dans `jc:hq`. `lt`/`he` assertent eux-mêmes le tag de struct (l'assert passait par `pico8_entity_get_field`). TROU-11 sans site. `dispatch` : `src/` 35 lignes / 37 occurrences / 15 fichiers, `tests/` 9 ; `par-nom` 986 (`src/` 419 au lieu de 420 : le commentaire de la déclaration de `pico8_mv` nommait aussi `pico8_entity_get_field`) ; `valeur` 4272 / 4639, `type-tag` 347 / 551, `ref-audit` 55 / 56 ; résidu D 5137 lignes dans 126 fichiers. Balayage ASan+UBSan des TU propre, `class-coverage-net` identique aux deux commits.
> - D.10b — `dispatch`, rappels : `js.my`/`hb`, `fq.my`, `mh.lh`, `hp.lq`, rappels des rangées de `menu` (`g_ix_my`, `g_ix_hb`, `ha`), `jo:fe`, `de.my` (`pico8_de_my`), `eu` de `help`, `init` de `score_panel` → pointeurs de fonction à la signature de leur unique appelant ; leurs lecteurs (`msgbox.c`, `research.c`, `ui_button.c`) ne les rendent plus comme valeur ; `PICO8_VALUE_FUNCTION`, `pico8_value_function()` et `pico8_value_function_t` retirés de `pico8_value.h/.c`. Empreinte 18 (18, 0) aujourd'hui. Compte : `dispatch` `src/` = 0, pont pour les TU excepté. §6 : aucun. `class-coverage-net` : seulement si le diff touche un constructeur. `sanitize-net` : non.
> - D.10c — `dispatch`, TU : 98 lignes `pico8_mv` → appels typés, 26 lignes `pico8_value_function` → rappels typés ; le pont éventuel de D.10a/D.10b supprimé. Empreinte 31 (0, 31). Compte : `dispatch` = 0, ajouté à `DATA_RESIDUE_GUARD`. §6 : aucun. `class-coverage-net`, `sanitize-net` : non, aucun fichier de `src/` ne change hors pont.
> - D.11b — `par-nom`, lectures sans receveur typé : B (7 + 1 ; test d'appartenance par le lien parent, `pico8_qh_of(jo)`, Q5), C (16 + 6 ; accesseur typé par champ dans la table de classe, NULL = absent, Q6), F (5 + 2 ; struct locale de cadre `{ep, eo, hi, ff, ca, gq, ra}` remplie par les cinq appelants de `pz`), K (4 ; enregistrement typé par sorte `{rr, has_s, s, me, has_od, od}`, résolu le long de la chaîne, pointé par `pico8_entity_kind_t`). Empreinte ≈ 14 (14, 0). Compte : `par-nom` ≈ 878. §6 : aucun. `class-coverage-net` : seulement si le diff touche un constructeur. `sanitize-net` : non.
> - D.11c — `par-nom`, classe M (Q7) : émetteur de particules (`ei`, `ez`, `f_`, `g__mm`) et sonde (`bv`, `dl`, `d_`) → composants portés par pointeur, règle 2 de la phase 2 appliquée à ces deux groupes seulement. Empreinte ≈ 15 (15, 0) : `particle`, `probe` et les 13 fichiers des classes qui les portent. Compte : `par-nom` ≈ 862. §6 : aucun. `class-coverage-net` : oui. `sanitize-net` : oui.
> - D.11d — `par-nom`, TU : T1 (mise en état et lecture d'état) en accès aux membres typés ; T2 (tests des descripteurs, tests de mort des `assert` d'écrivain) supprimés comme tests de la couche (Q11, §6). Deux chunks d'environ 19 fichiers. Empreinte 38 (0, 38). Compte : `par-nom` ≈ 286 (l'infrastructure de `src/`). §6 : `test_pico8_planet_gh_row_lq` et `test_pico8_ga_export_panel_by` (3 lignes chacun) lisent les membres, preuve rouge. `class-coverage-net`, `sanitize-net` : non, aucun fichier de `src/` ne change.
> - D.11e — `par-nom`, suppression de l'infrastructure : lecteurs `field` et écrivains `set_field` des descripteurs et leurs emplacements dans `pico8_struct_class_t`, les 10 `kind_field` et les emplacements des 11 sortes, les aides `*_take_*` restantes, `pico8_entity_get_field`/`set_field`, les typedefs `pico8_struct_field_fn`/`_set_field_fn` ; les structs-vues deviennent typées ou disparaissent (remplaçants de `mo`/`je`/`fm`, `jc_too_far`, `jc_mouse_point`, `bv_et`, `nr_ldown_empty`, `pj_cr`, `mz_jj`, `ha_pz`, `ie_ke_view`). Empreinte ≈ 35 (35, 0). Compte : `par-nom` = 0, ajouté à `DATA_RESIDUE_GUARD`. §6 : aucun. `class-coverage-net` : oui. `sanitize-net` : non.

Décisions de D.9, recopiées :

> **Décisions de D.9 (humain, 2026-09-29).** D.10 à D.12 sont ajoutées telles que l'étude les propose (`docs/prd/done/plan3-phaseD-etape9-etude.md`, §5), avec ses recommandations pour Q1 à Q11 : (Q1) ordre `dispatch` → `par-nom` → listes, tout en D, avant la Sortie ; (Q2) table de méthodes en struct de pointeurs typés, un membre nommé par méthode avec sa propre signature ; (Q3) états lancés par `dg` : table d'états typée indexée par un enum, `me` reste une chaîne, convertie par `strcmp` ; (Q4) `pico8_dispatch_context` gardé, son commentaire réécrit ; (Q5) membres d'une fille lus sur la famille : test d'appartenance par un lien parent ajouté à `pico8_struct_class_t` (`pico8_qh_of(jo)`, NULL hors de la lignée) ; (Q6) champs transversaux : un accesseur typé par champ dans la table de classe, NULL = absent ; (Q7) classe M (émetteur de particules, sonde) : la règle 2 de la phase 2 (composant porté par pointeur) appliquée dès D à ces deux groupes seulement, sans le test ponctuel d'archétype, risque écrit dans le PRD ; (Q8) singletons `fm`/`je`/`mo` : accesseurs typés, NULL sur le remplaçant, celui de `fm` restant une struct typée pour `jk`/`nz` ; (Q9) un seul conteneur de pointeurs (`void *`, mêmes `add`/`del`/`all`/`foreach`, `del` par identité, TROU-5 inchangé), plus des tableaux fixes typés là où la borne est dans le cart ; (Q10) nouvelle catégorie `liste` au compteur, union étiquetée locale à `layout` pour `lq.items`/`mh.items`/`jb.ls` ; (Q11) tests des descripteurs (T2) supprimés avec eux, comme tests de la couche (§6). **Q12 reste ouverte** : l'étude D.9b des valeurs à genres mêlés (`hf`/`gg` TROU-3, `et`, `qh_qh`, `js.my`/`h_`, retours de `ldown`/`gv` ; 62 prototypes publics à `pico8_value_t`), avant toute étape `valeur`/`type-tag`/`ref-audit`, n'est pas tranchée ; `plan-pilot` la pose à l'humain à la fin de D.12.

Comptes au départ de D.10, avec la règle du pont des TU, recopiés :

> Comptes au départ de D.10 (HEAD 5a13008, `make -s data-residue` et `--detail` ; l'étude mesurait `2dec71f`, avant D.7a–D.8) : `valeur` 4961/126, `type-tag` 352/65, `ref-audit` 56/15, `par-nom` 1009/81 (`src/` 433/43, `tests/` 576/38), `dispatch` 191/61 (`src/` 67/30, `tests/` 124/31), résidu D 5868 lignes / 128 fichiers ; `liste`, pas encore au compteur, 927/99 avec le motif de Q10 (`src/` 496/61, `tests/` 431/38). Les classes A–M, T1/T2 et L1–L4 sont celles de l'étude (annexe A) ; ses « ≈ » viennent de son classement, chaque PRD remesure. Un TU qui nomme un symbole retiré par une étape de `src/` la suit dans le même commit, ou le symbole reste comme pont compté, fermé par l'étape TU de son groupe (D.10c, D.11d, D.12d) ; le PRD tranche. `sanitize-net` : aux seules étapes qui changent une durée de vie ou un propriétaire (§7).

En-tête de la liste de D, règles communes et ordre, recopiés :

> Liste de D, écrite après D.1 (HEAD 3816427, agent `guideline`), à partir de `make data-residue` et `sh tools/data_residue.sh --detail`. Comptes à l'écriture (lignes/fichiers) : `valeur` 5989/132, `ob` 2245/128, `stash` 56/14, `type-tag` 445/71, `ref-audit` 70/17, `par-nom` 1081/80, `dispatch` 197/59, `overrides` 449/62, `spec` 939/68, **9678 lignes distinctes dans 143 fichiers** ; ceux de la note de D.0 sont d'avant D.1. Chaque ligne nomme la catégorie qu'elle pousse et son « Compte » après elle : « = » attendu exact, « ≤ » majorant par grep (lignes de la catégorie qui nomment les sites des groupes déjà passés, cumulées dans l'ordre de la liste). Le test ponctuel (deuxième puce de D) est hors compte jusqu'à D.8, où il est supprimé ; il ne nomme aucun motif de `spec` ni `overrides`. Une catégorie tombée à 0 entre dans `DATA_RESIDUE_GUARD` dans le même commit.
>
> Empreinte (mesurée à l'écriture, chaque PRD la remesure) : fichiers de `src/*.[ch]` et `tests/*.[ch]` qui portent une ligne de la catégorie du groupe, ou qui nomment ses tables ou membres (majorant : noms courts partagés, `nu`, `k_`, `lq`). Règles communes : un nom nouveau ne correspond à aucun motif figé de la catégorie qu'il remplace (pas de `g_X_spec`/`g_X_defaults`/`pico8_X_default`, et le paramètre `overrides` typé est renommé) ; un lecteur par nom qui survit reste compté par `par-nom` ; un défaut muté (§5, 1a : `pj.ou`, `pj.ea`, `pj.cr`, `ga.by`, `nf.l`, `nf.r`, `jo.kw`) devient un tableau fixe statique non `const`. Un TU construit son état et ne rejoue jamais de fichier d'événements ; un e2e nouveau ajoute de la couverture de lignes en moins de 180 frames ; aucun `*.sha256` ni fichier de `tests/e2e/` ne change. `make sanitize-net` : à chaque étape qui touche `src/`, puisque toutes font passer des tables ob à des données statiques ou locales (§7, « En D et en 2 »).
>
> Ordre : `stash`, puis les tables constantes nommées par la première puce ; les membres table (`pico8_ob_table_t *`, R1 `ref:table`) avant les spécifications, dont les défauts table en dépendent ; `spec`, `ke` d'abord et l'arbre `jo` en dernier ; `overrides` ; les tables ad hoc restantes, puis les TU ; la suppression de `pico8_ob.c`/`pico8_stash.c`. `par-nom`, `dispatch`, `pico8_list_t`, puis `valeur`/`type-tag`/`ref-audit` viendraient après : arrêt, voir la fin de la liste.

Puces de la phase D, recopiées (la quatrième est la sortie écrite de la phase) :

> - Les tables constantes parsées au démarrage (`g_es`, `g_ct`, `g_b_`, `g_co`, `g_fb`, `g_ek`, `ci`, menus `g_ix`/`g_ha`, aide `g_bo`, score `g_ef`/`g_bz`, `g_df`) et celles décodées par `unstash` (`g_hc`, `g_fd`, `g_dr`, `g_nb`, `g_r`, `g_iz`) deviennent des tableaux/structs `static const`, **générés une fois** à partir de ce que `ob()`/`unstash` produisent aujourd'hui, puis versionnés.
> - Test ponctuel, supprimé dans le commit qui supprime `pico8_ob.c`/`pico8_stash.c` : chaque table générée est égale, entrée par entrée, à ce que `ob()`/`unstash` produit. Le filet ne voit pas les entrées jamais affichées (aide, score, menus).
> - Les tables ad hoc construites en cours de partie (~68 `ob_table_new()`) deviennent des structs locales ou des tableaux fixes.
> - **Sortie** : `pico8_ob.c`, `pico8_value.c`, `pico8_type_tag.h`, `pico8_ref_audit.h`, `pico8_stash.c` supprimés ; plus aucun `malloc` pour des données. Les entités gardent le leur : c'est Plan 2 §8.

§5, recopié hors de ce qui précède. Les notes « Découvert à D.0 » à « Découvert à D.8 » sont en fin des lignes D.0 à D.8 ci-dessous (D.1 et D.9 n’n ont pas ; celle de D.10a est dans sa ligne, recopiée plus haut ; D.10b n’n a pas encore). Les listes d’\étapes livrées de 1a (l. 93-149 du plan) et de 1b (l. 169-224) ne sont pas recopiées : historique livré, aucune de leurs lignes ne lie D.11a ; leurs règles de phase le sont.

> ## 5. Phases
>
> ### Phase 1a — Entités en structs, champs `pico8_value_t`
>
> Transformation mécanique. Chaque classe devient un `struct` dont le premier membre est la struct de sa classe parente ; chaque champ de sa chaîne (défauts et champs écrits sur instance) devient un membre de type `pico8_value_t`, **nom minifié conservé**. Nil reste `pico8_value_ref(NULL)`, comme aujourd'hui.
>
> - **Méthodes** : une table `static const` de pointeurs de fonction par classe ; une surcharge remplace l'entrée héritée. Les méthodes appelées par nom de chaîne (`mv(held,"hq")`, `jz(rm,…)`, `dc(nt)`) passent par un enum de méthode qui indexe cette table.
> - **Clés construites à l'exécution** : `self[bt.."held"]` (`pico8_ui.c:328`) → deux champs `lheld`/`rheld` ; `ldown`/`rdown` côté méthodes. Tout autre site de clé dynamique trouvé est énuméré de même, ou déclenche un arrêt (§7) si l'ensemble des clés n'est pas fini.
> - **Défauts mutés sur la classe** : `ie.gy`, `ie.id` (`pico8_research_icon.c:158,182`, `pico8_cart.c:158`) deviennent un état statique partagé de la classe, jamais un champ d'instance. Même traitement pour tout autre défaut réaffecté après l'initialisation **ou dont la table pointée est modifiée en place** (connus : `pj.ou`, `pj.ea`, `pj.cr`, `ga.by`, `nf.l`, `nf.r`) ; la liste des étapes les nomme.
> - **Instances sans classe** : converties comme des classes à part entière, sans parent.
> - **Coexistence** : une instance convertie porte un nouveau tag (`PICO8_TYPE_TAG_STRUCT`) et un pointeur vers sa table de méthodes, dans le même premier emplacement qu'aujourd'hui. `hv`/`cv`/`cg`, `dc`, `mv` traitent les deux formes pendant la migration ; l'ordre d'enregistrement est préservé.
> - **Champ d'un écrivain générique** : un champ écrit par un site qui reçoit un élément de `cg.<tag>` ou `hv` sans filtre de classe devient membre de l'ancêtre le plus haut qui porte ce tag (`jo` pour `cg.jo`) ; seul un champ écrit par la méthode propre d'une fille va dans la struct de la fille. En 1a, un membre nil de trop est sans effet (absent à la lecture) ; la phase 2 reclasse.
> - **Ordre** : une classe par commit, la parente avant ses filles (`ke` → `jo` → `qh` → …). Les racines sans parent dans n'importe quel ordre.
> - **Premier commit** : une vraie racine sans parent ni fille (`jb`), qui prouve la coexistence. **Deuxième** : `ke` **dans le même commit** que sa fille directe sans descendance de plus petite emprise (mesurée par le PRD de l'étape), pour que la struct de `ke` soit construite et vue par le filet dès son introduction. Ensuite les autres filles directes sans descendance de `ke`, par emprise croissante ; l'arbre `jo` et `g_`→`fv` après elles ; `ui` et `pj` (clés dynamiques, défauts mutés) en fin de liste.
> - **Héritage entre formes et classes-valeurs** : une classe convertie garde sa `pico8_class_t` enregistrée (registres `fz` et nommé compris) jusqu'à la dernière étape de 1a, qui les supprime toutes avec `pico8_class.c`. Dans le commit de conversion, sa `pico8_class_t` reçoit un pointeur vers le constructeur de struct, et `pico8_class_instantiate` délègue à ce constructeur quand il est posé : `gx[]`, `ie.ke`, `lookup_named` et `kv[hp.cw]` construisent ainsi la forme struct sans être touchés avant la conversion de leur propre classe. Les descendants non convertis trouvent leurs champs hérités en remontant `parent`, qui reste valide.
> - **Méthodes d'une parente convertie** : tant qu'une descendante n'est pas convertie, chaque fonction de la parente qu'une instance descendante peut atteindre (dispatcher hérité dans `defaults`, ou appel direct `jo.kj(self,…)`, `jo.kf(self,…)`) reste à double forme : lecture/écriture par `pico8_entity_get_field`/`set_field`, ou aiguillage sur le tag comme `pico8_ke_ks`. Elle ne passe à la forme struct seule qu'au commit de sa dernière descendante (pour `jo` : 1a.26). La caractérisation de 1a.17 lit au moins une instance de chaque fille non convertie construite par le filet (`qh`, et `pm`/`ga` via `kv[hp.cw]`) : champ hérité de `jo` et résultat d'une méthode de `jo` appelée sur elle.
> - **Accès directs aux défauts** : tout site hors `pico8_class.c` qui lit ou écrit `<classe>->defaults` (grep `->defaults`, ~100 sites dans 22 fichiers) migre dans le commit de cette classe : la valeur passe dans l'état statique de la classe (défaut muté) ou dans la table de méthodes (installation de méthode), et la `pico8_class_t` retenue n'en garde, pour les descendants non convertis, que la **même REF** que l'état de la struct : une table de défaut d'une classe qui a des descendants non convertis est prise dans `cls->defaults` à l'init, jamais re-parsée depuis la spec. Le PRD de `jo` tranche `kw` (écrit en place par `mx:kf`, `planet_family_trade.c:280`, si l'instance n'a pas couru `lz`) : défaut modifié en place → état statique partagé, pointé aussi par `defaults`.
> - **Sortie** : plus aucune `pico8_class_t`/`pico8_instance_t` à l'exécution ; `pico8_class.c` supprimé ; `make class-coverage-net` toujours sans zéro — vérifié par `grep -rn 'pico8_class_t\|pico8_instance_t\|->defaults\|lookup_named' src/` vide.
>
> ### Phase 1b — Types C réels
>
> Struct par struct, chaque membre `pico8_value_t` prend son type C (`pico8_fixed_t`, `pico8_vec_t`, pointeur de struct typé, `const char *`, booléen…).
>
> - **Nil** : un indicateur de présence **uniquement** là où le code observe nil (test de vérité, `mv` qui rend 0 sur champ absent, affectation `= nil`). Rappel : en Lua `0` est vrai, donc un fixe à 0 n'est pas nil. Pas de valeur sentinelle.
> - Les REF deviennent des pointeurs typés : les sites de `pico8_ref_audit.h` disparaissent au fil de l'eau.
> - **Recensement (1b.0)** : outil seul, aucune struct typée, sur le modèle de l'étape 0.2 et du build `-DPICO8_CLASS_COVERAGE`. Un build à drapeau recense, par (struct, membre), l'ensemble des genres de `pico8_value_t` écrits et si nil est observé par un lecteur (test de vérité, `mv` qui rend 0, `= nil`). Crochets : init des structs, écrivains `set_field`, lecteurs `*_truthy` et par nom, les ~24 sites à table d'offsets `*(pico8_value_t *)((char *)self + offset)`. Passé sur les rejeux de `FRAMES_EVENTS` et `make test`. Sortie : une table versionnée, et la durée du rejeu ASan+UBSan (§7). Un membre sans genre observé est signalé : son PRD ajoute une caractérisation dirigée avant de le typer.
> - **Coexistence** (fixée à l'écriture de la liste, agent `guideline`) : le `field`/`set_field` du descripteur est le seul point de conversion ; les sites par nom (`pico8_entity_get_field`/`set_field`, `*_truthy`, `src/` et `tests/`) ne changent pas.
>   - Lecture : le membre typé est rendu emballé, du genre qu'il aurait eu en 1a, un pointeur (REF, chaîne, fonction) repassé tel quel ; absent exactement là où 1a rendait 0 — pointeur NULL, ou indicateur de présence faux.
>   - Écriture : l'écrivain déballe ; nil éteint la présence ; un genre hors de la ligne de recensement du membre est un `assert` (arrêt « champ sans règle », §7).
>   - Tables d'offsets (`members[]`, `defaults[]`, `pico8_X_member`) : elles ne listent que des `pico8_value_t`. Le commit qui type un membre l'en retire partout, tables des filles comprises (`offsetof(fille, parent.m)`), et l'initialise par affectation typée ; un défaut REF garde la même REF.
>   - Présence : un membre non pointeur lisible par nom (`l`) qui peut valoir nil (nil observé, ou tenu) garde un indicateur, sauf preuve du PRD qu'aucune lecture par nom ne le voit nil.
>   - Garantit : présent/absent, valeur et identité de REF lus par nom identiques à 1a ; aucune écriture `*(pico8_value_t *)` sur un membre typé. Vérifié à chaque étape par `grep -n 'offsetof(pico8_T_t, \([a-z0-9_]*\.\)*m)' src/` vide (m typé, T la struct et ses filles) et la mutation (2) du §7.
>   - Un site par nom remplacé par un accès direct est noté dans « Découvert à 1b.N » ; les autres survivent à 1b.
> - **Pilote (1b.1)** : la plus petite struct dont la ligne de recensement mêle au moins un fixe simple, un membre à nil observé et un membre REF. Son « Découvert à 1b.1 » fixe le patron (disposition du membre typé, indicateur de présence, écriture à l'init, emballage du lecteur) ; les PRD suivants le citent, et s'en écarter est une « lecture nouvelle d'une règle » (§7).
> - **Liste** : chaque ligne porte son empreinte mesurée (fichiers de `src/` et `tests/` qui lisent ou écrivent ses membres) ; une ligne de plus de ~40 fichiers est découpée dans la liste elle-même, par groupe de membres (1b.Na, 1b.Nb…).
> - **Sortie** : aucune struct d'entité ne contient de `pico8_value_t` — constaté par une commande nommée : un script qui parcourt les typedefs de struct d'entité (premier membre `pico8_struct_header_t` ou struct d'entité enchâssée) et liste tout membre `pico8_value_t` ; sortie vide. Le même script sert de compteur de progrès à chaque étape.
>
> **Sortie de la phase 1b, constatée à 1b.37** : `make value-members` vide (`membres pico8_value_t : 0`) ; `grep -rn offsetof src/` vide ; `make class-coverage-net` identique à `tools/class_coverage_net_expected.txt` ; `make sanitize-net` : 15 propres ; `make frames-check` : 15 conformes. Point d'entrée de la phase D : toute entité est une struct à membres C typés ; `pico8_value_t` ne vit plus que dans les tables ob, les arguments et retours de dispatch, et les lecteurs/écrivains par nom des descripteurs. Restent sans objet : le recensement (`pico8_member_census`, ses crochets, `tools/member_census.*`, `make member-census`), supprimé par D.1 ; `make value-members` reste la garde de cette sortie, dans `make test` à partir de D.0.
>
> ### Phase D — Données
>
> - **D.0 — mesure seule, sans conversion, sur le modèle de 1b.0** (ajouté à la revue de sortie de 1b) : un compteur nommé (`make data-residue`, `tools/data_residue.sh`) compte, par catégorie, dans `src/` et `tests/`, les usages de `pico8_value_t`/`pico8_value_*()`, `pico8_ob_table_t`/`pico8_ob*()`, `pico8_list_t` si le §6 le retire, `pico8_type_tag_of` et `pico8_ref_audit.h` — dont les lecteurs/écrivains par nom (`pico8_entity_get_field`/`set_field`, `*_truthy`, `field`/`set_field` des descripteurs et leurs aides `*_take_*`), le dispatch à arguments `pico8_value_t`, les `overrides` des constructeurs, les spécifications de classe lues par `pico8_ob()` et leurs `pico8_X_default`. Sa sortie vide **est** la sortie de la phase D, et chaque ligne de la liste porte son « Compte : N ». D.0 met aussi `make value-members` et `grep -rn offsetof src/` dans `make test`, en mode vérification (code non nul si non vide), à côté de `float-guard` ; une catégorie du compteur tombée à 0 y entre de même. La liste ordonnée de D est écrite après D.0, à partir de sa sortie : une catégorie par groupe nommé, découpée au-delà de ~40 fichiers ; chaque test de régression du §6 qui lit par nom est rattaché au groupe qui force sa réécriture, avec sa preuve rouge. Une catégorie qu'aucune puce de D ne couvre est l'arrêt « champ sans règle » du §7. Découvert à D.0 : `make data-residue` (`tools/data_residue.sh`, modes `--detail` pour la liste de D et `--check`), neuf catégories figées, lignes qui se chevauchent, commentaires compris — `valeur`, `ob` (avec `pico8_fi`), `stash`, `type-tag` (tout `pico8_type_tag.h`, dont le `tag` de `pico8_struct_header_t`), `ref-audit`, `par-nom` (`*field_truthy` seulement), `dispatch` (`pico8_value_function(_t)`, `pico8_mv` ; pas `methods[]`), `overrides`, `spec` (hors six `_default` à argument omis). Comptes initiaux après la fusion de master (lignes/fichiers) : 6001/133, 2280/128, 56/14, 450/72, 70/17, 1070/81, 180/57, 421/57, 942/68, **9711 lignes distinctes dans 145 fichiers** (100 de `src/`, 45 de `tests/`). Sortie vide = la seule ligne `résidu D : 0 ligne(s), 0 fichier(s)`. `pico8_list_t` sans catégorie (§6 ne le retire pas), mais ses éléments `pico8_value_t` comptent dans `valeur` : la liste de D doit en changer la représentation. « Plus aucun `malloc` pour des données » lu comme les allocations de `pico8_ob.c`/`pico8_stash.c`/`pico8_value.c`, éteintes avec leurs catégories ; une lecture plus large ajoute une catégorie dans son propre commit. Garde : `make data-guard` dans `make test` (`entity_value_members.sh --check`, `grep -rn offsetof src/`, `DATA_RESIDUE_GUARD`, vide à D.0) ; une étape qui éteint une catégorie l'ajoute à `DATA_RESIDUE_GUARD`. Motifs figés : une étape ne change un motif qu'avec la preuve que les lignes retirées ne touchent plus aucune catégorie transversale.
> - **D.1 — suppression du recensement**, premier groupe après D.0 : `pico8_member_census.c/.h`, les crochets `SEE`/`READ`/`WRITE`/`CHILD_EXIT`/`STOP` de `src/` et `tests/`, `tools/member_census.*`, la cible `member-census` et ses entrées de listes de sources, le mode `--c-table` de `tools/entity_value_members.sh`. Les messages d'`assert` qui renvoient à « census row » / « plan-3 section 5 » sont réécrits pour nommer le contrat violé. Aucun `.sha256` ne change ; `make class-coverage-net` identique.
>
> - D.2 — `stash` : tables décodées par `unstash` — `g_hc`, `g_fd`, `g_dr`, `g_nb` (`data_init.c`, avec la passe `nb_mr` et son routage `gp={"nk"}`), `g_r`, `g_iz` (`pico8_stash.c`, lus par `pico8_r()`/`pico8_iz()`) → `static const` générés (première puce) ; le test ponctuel naît ici : chaque table générée est égale, entrée par entrée, à `pico8_ob(pico8_unstash(mem, pd))`, la mémoire étant chargée par `pico8_load_map_json` (pas de rejeu). `pico8_stash.c/.h` et `test_pico8_stash.c` restent jusqu'à D.8. Empreinte 22 (15, 7). Compte : `stash` = 26 (`pico8_stash.c` 7, `pico8_stash.h` 7, `test_pico8_stash.c` 12) ; `ob` ≤ 2195. §6 : aucun. `class-coverage-net` : seulement si le diff touche un constructeur. Découvert à D.2 : `pico8_map_tables.c/.h` (données typées générées une fois, sans dépendance ob ; `pico8_r_lookup`, `pico8_iz_lookup`, `pico8_hc_lookup`, `pico8_jb_row_t`). Trois ponts, des vues ob construites par `data_init.c` depuis les tables typées : `pico8_fd()` (fermé par D.6c, entrée de `hp`), `pico8_dr()` (D.6b, entrée de `jb`), `pico8_nb()` (D.4b `js._cr_t`, puis D.7a `menu`/`tech_gate`). `pico8_hp_po` rend `const pico8_hc_entry_t *`. `pico8_data_init(mem)` ne lit plus `mem` (retrait à D.7a). Test ponctuel : `tests/test_pico8_map_tables.c`, 2 lignes `stash` jusqu'à D.8. `ob` après D.2 : 2227 (au lieu de ≤ 2195) ; les majorants `ob` suivants se décalent de 32.
> - D.3 — `ob`, tables constantes parsées au démarrage (liste de la première puce) : `g_es`, `g_ct`, `g_b_`, `g_co`, `g_fb` (`game_data.c`), `g_ek`, `g_df` avec sa passe `df_mr` (`data_init.c`), `g_ci` (`entity.c`), `g_ix`/`g_ha` (`menu.c`), `g_bo` (`help.c`), `g_ef`/`g_bz` (`score_panel.c`) → `static const` générés, test ponctuel étendu. Empreinte 21 (17, 4). Compte : `ob` ≤ 2105. §6 : aucun. `class-coverage-net` : seulement si le diff touche un constructeur. Découvert à D.3 : `pico8_game_data.c/.h` devient le module pur des tables constantes (générées une fois, sans dépendance ob ni `init` ; `pico8_int_seq_at`, `pico8_fixed_seq_at` ; un non-entier est l'hexadécimal signé des bits qu'`ob()` rendait, masque `0xffff.fffe` compris, jamais `PICO8_FIXED_LIT`) ; `pico8_jb_item_t` gagne `is_false`. Huit tables lues typées (`es`, `ct`, `b_`, `co`, `fb`, `ek`, `ci`, `ef`). Cinq ponts, des vues ob construites par `data_init.c` (publiques `pico8_ix_view`, `pico8_ha_view`, `pico8_bo_view`, `pico8_bz_view`, appelées par les `*_init`) : `pico8_df()` (fermé par D.4a, `jo.nk` ; `galaxy` à D.7a s'il reste), `pico8_ix()` et `pico8_ha()` (D.6b, clés de `js` et appelants de `menu.c`), `g_bo` (D.6b, `pz` → `js`), `pico8_bz()` (D.6b, entrée de `mh`, puis D.7a `score_panel_display`). `ob` après D.3 : 2208 (au lieu de ≤ 2137) ; les majorants `ob` suivants se décalent de 103 (2208 − 2105).
> - D.4a — `ob`, membres table de l'arbre `jo` : `jo.nu`/`ka`/`dz`/`cq`/`kw`/`em`/`nk`, `qh.px`/`ba`, `hj.k_`, `ga.k_`, `nr.hz`, `mx.ht` → struct ou tableau fixe par membre (troisième puce), forme tranchée par le PRD (le masque de bits de `nu` est la phase 2) ; leurs défauts table (`jo_default_table`, `family_default_table`, `family_trade_default_table`) sont générés `static const` dans la même étape (première puce). Empreinte 34 (22, 12). Compte : `ob` ≤ 2033. §6 : `gh_row_lq` (sa table `kw`, l. 102-120) et `ga_export_panel_by` (sa table `kw`, l. 98-117) construisent le nouveau `kw`, preuve rouge dans ce commit. `class-coverage-net` : oui (init des constructeurs). Découvert à D.4a : formes `pico8_int_seq_t`, `pico8_str_seq_t` (`nu` : étiquettes ordonnées, le masque reste la phase 2), `pico8_jb_row_t` (`dz`, `em`, `px`), `pico8_kw_t` (`kw` pointe `g_jo_kw_shared`, non `const`, remis par `pico8_jo_init()`, ou `kw_own` après `lz`), `const pico8_df_entry_t *` (`nk`) ; défauts générés dans `pico8_game_data.c`, un objet par table et par classe (`pico8_X_nu_tags`, `_seq`, `_row`, `_rows`, `pico8_jo_kw_start`) ; `pico8_int_seq_has_fixed`, `pico8_df_lookup`. La lecture par nom d'un membre converti rend un REF non tagué vers sa donnée ; `lt`/`he` (`nu`) et `f_` (`k_`, lu par `ei`) lisent à double forme (table ob ou donnée typée) : D.4b retire la branche ob avec `mh.nu`, `g_.nu`/`k_`, `jc.nu`, `mz.nu`. Un écrivain par nom d'un membre converti accepte nil ou un REF non tagué : un vecteur n'y est plus refusé (limite `ref:autre` de 1b.23b), les TU hors rangée le remplacent par une table ob. Pont `pico8_df()` fermé (`bw` et `gn` lisent `pico8_df_table`). Ponts nouveaux : `pico8_jb_row_view` (publique, `data_init.c`), appelée à chaque appel par `jo:gh` (`px`, `dz[mi]`) et `jo.ex` (`em`), et l'item `kw` de `jo:gh` (aide locale `jo_kw_view`) (D.6b pour `js`/`layout`, D.7a pour `planet_display`/`research_display`). `pm:pq` pose la clé `k_` de `ga` en REF typé : `overrides` = 450, et le « = 315 » de D.6a devient 316. `ob` après D.4a : 2037 (au lieu de ≤ 2033) ; les majorants `ob` suivants se décalent de 4 (2037 − 2033).
> - D.4b — `ob`, autres membres table : `bg.pl` (et `g_bg_pl`), `hy.po`/`fu`, `fv.rb`, `mh.nu`, `g_.nu`/`k_`, `hp.mr`/`gp`/`fk`/`lq`, `pj.gs`/`i_`, `qb.pc` (et `g_qb_pc`), `jc.nu`/`k_`/`hg`, `ui.ic`, `js._cr_t`, `mz.nu`/`kk`, même règle. Empreinte 39 (27, 12). Compte : `ob` ≤ 1982. §6 : aucun. `class-coverage-net` : oui. Découvert à D.4b : trois chunks (≈ 70 chemins ; le majorant de la ligne ne voyait pas les signatures forcées : `ho_apply`, `hy:kt`/`gi`, la rangée étendue). Formes : `pico8_vec_seq_t` (`qb.pc`) ; `pico8_jb_item_t` gagne `is_fixed`/`fixed`, `pico8_jb_row_t` gagne `fn` (`fv.rb`, `pj.gs`/`i_`, `hp.fk`, `hp.lq` tenue dans `hp` : `lq_own`/`lq_inner`) ; `hp.mr`/`gp` en `pico8_str_seq_t`, `ho_apply` les prend typés ; `pico8_qe_map_t` (`hy.po`/`fu`, stockages `po_own`/`fu_own` dans `hy`, `cm`/`bx`/`ck` remplissent une table fournie ; une clé `qe` chaîne est absente) ; `pico8_jc_hg_t` (`jc.hg`, 90 `id` au plus, `pico8_jc_hg_add`/`_has`) ; `pico8_ui_ic_t` (`ui.ic`, 16 couches vidées à chaque `gw`) ; `js._cr_t` pointe une entrée de `pico8_nb_table` (`pico8_nb_lookup`), `hy:kt` la prend typée par `pico8_gi_nb`, qui partage son calcul avec `pico8_gi`. `lt`, `he`, `ei` n'ont plus qu'une forme. Ponts restants : `pico8_nb()` et `pico8_gi(ob)` (D.7a, `menu`/`tech_gate`) ; vues à l'appel par `pico8_jb_row_view` de `hp.fk`, `hp.lq` (D.7a `research_display`/`planet_display`, D.6b `js`), `fv.rb`, `pj.i_` (D.7a `pi`), `pj.gs[idx]` (D.7a `score_panel`) ; `pico8_ft_index`/`ft_name` sans appelant (D.7a `collect`). `ob` après D.4b : 1882 (au lieu de ≤ 1986) ; les majorants `ob` suivants se décalent de −100 (1882 − 1982).
> - D.5a — `spec`, `ke` et les classes d'interface : `g_X_spec`, `pico8_X_default`, `pico8_X_defaults_reset` et aides `X_default_<genre>` de `ke`, `bg`, `ui`, `js`, `fq`, `jb`, `lq`, `qb`, `nf`, `mh`, `iv`, `mz`, `jc`, avec les appels `pico8_ke_default` des autres fichiers → défauts `static const` générés depuis `pico8_ob(spec)`, test ponctuel étendu. Lecture de la première puce : une spécification de classe est une table constante parsée au démarrage. `test_pico8_spec_defaults.c` et `test_pico8_jo_spec_defaults.c` lisent les défauts statiques, même intention. Empreinte 30 (21, 9). Compte : `spec` ≤ 779. §6 : aucun. `class-coverage-net` : oui. Découvert à D.5a : deux chunks (interfaces, puis `ke` ; ≈ 40 chemins, `pico8_game_data.*`, `pico8_bootstrap.c` et les TU qui plient l'identité de `me` hors du majorant). Défauts générés `pico8_X_start` (type `pico8_X_start_t`, `pico8_game_data.h`), un par classe à clés (`ke`, `bg`, `ui`, `js`, `fq`, `qb`, `nf`, `mh`, `iv`, `mz`, `jc`), clés propres de la spécification, lus par les constructeurs ; les tables gardent l'objet de D.4a/D.4b ; `mz.jj` s'initialise de `pico8_mz_jj_qs` ; `lq`/`jb` sans objet ; `ui.le` : liste statique de `ui.c` (`pico8_ui_le_shared`) ; `qb.v`/`p`/`ra`, `iv.v`, `ui.hw` pointent la donnée générée (membres `const`) ; `nf.ra` pointe le statique non `const` `g_nf_ra`, affecté depuis `pico8_nf_start.ra` à `pico8_nf_init` (repli : `test_pico8_entity_kind` et `test_pico8_ie` écrivent `je.ra` en place) ; `nf.l`/`nf.r` restent `g_nf_l`/`g_nf_r` (D.7b). Le cycle `_init`/`_shutdown`/`_ready` des treize modules reste, sur un drapeau. `pico8_ke_start_field` (compté par `par-nom`) remplace `pico8_ke_default` au bout de `pico8_pj_default`, `pico8_g__default`, `pico8_ie_default` (retirés à D.5b) et `pico8_jo_default` (D.5c, avec le cas `M_KE` de `test_pico8_jo_spec_defaults`) : D.5c le supprime. `pico8_rk_default` (variante de dessin, comptée par `spec`) est à D.5b. `spec` après D.5a : 634 (au lieu de ≤ 779) ; les majorants `spec` suivants se décalent de −145 (634 − 779). `par-nom` : 1090.
> - D.5b — `spec`, `g_`/`fv` (aide), `hp`, `ie` (et `ie_chain`), `pj`/`rk`, `hy`, `trade`, même règle. Empreinte 23 (16, 7). Compte : `spec` ≤ 547. §6 : aucun. `class-coverage-net` : oui. Découvert à D.5b : deux chunks (exclusion de `rk`, puis les sept classes ; 29 chemins, `pico8_game_data.*` et trois TU hors du majorant). `pico8_rk_default` (variante de dessin à argument omis) entre dans `SPEC_EXCLUSION`, preuve : ses quatre lignes ne portent aucune autre catégorie. Défauts générés `pico8_X_start` de `g_`, `fv`, `hp`, `ie`, `pj`, lus par les constructeurs ; `hy`, `trade` sans objet. `ie` : un seul défaut pour la chaîne et les objets prêtés, `kq`/`ra`/`eh` pointent la donnée générée, plus libérés à l'arrêt. `fv.hi`/`gq`/`kq`, `g_.ra`, `ie.kq`/`ra`/`eh`, `pj.cr.hi`/`gq` : membres `const` vers la donnée générée, aucun repli. `pj.cr` : `g_pj_cr` reste (son `ep` est muté), valeurs de `pico8_pj_cr_start`. `pj.ou`/`pj.ea` : rangées générées `pico8_pj_ou_row`/`pico8_pj_ea_row`, d'où `pico8_score_panel_init` construit une fois par init les tables d'état `g_pj_ou`/`g_pj_ea` (`pico8_jb_row_view`), jamais libérées comme celles de la spécification ; leurs lecteurs (`hb`, `hk`, `kj`, `pico8_mh_new`) restent des tables ob (D.6b, D.7a). `test_pico8_spec_defaults` n'a plus de registre ni de constante. `pico8_ke_start_field` n'a plus que `pico8_jo_default` et le cas `M_KE`. `spec` après D.5b : 430 (au lieu de ≤ 402 après le décalage de D.5a : 28 lignes des sites de D.5b, retirées par D.4a/D.4b, étaient comptées à la fois dans le majorant 547 et dans ce décalage ; majorant corrigé 430) ; le « = 0 » de D.5c est inchangé. `par-nom` : 1085.
> - D.5c — `spec`, arbre `jo` : `jo`, `qh`, `nl`, `os`, `hj`, `pm`, `ga`, `nr`, `fr`, `mx`, aides `family_default_*` et `family_trade_default_*`, même règle ; le `field` des sortes (`pico8_entity_kind_t`, `g_qh_kind` … `g_mx_kind`) lit les défauts statiques sous un nom hors motif, compté par `par-nom`. Empreinte 19 (6, 13). Compte : `spec` = 0, ajouté à `DATA_RESIDUE_GUARD`. §6 : aucun. `class-coverage-net` : oui. Découvert à D.5c : deux chunks (données, puis conversion et garde ; 27 chemins, `pico8_game_data.*`, `ke.c/.h`, le test ponctuel et le `Makefile` hors du majorant ; le lien du test ponctuel prend `pico8_planet_family_trade.c`, pour interroger `nr`/`fr`/`mx`). Défauts générés `pico8_X_start` des dix classes de l'arbre, clés propres, lus par les constructeurs, l'héritage par le `struct_init` de la parente ; chaînes dans des tableaux distincts, un par jeton (des littéraux se fusionneraient, TROU-3). `jo.kq`/`gq`, `pm.ng` : membres `const` vers la donnée générée, aucun repli. Le `field` des sortes est `pico8_X_kind_field` : `rr`, `s`, `me`, `od` seulement, lus dans `pico8_X_start`, puis la parente jusqu'à `pico8_jo_kind_field` (seul déclaré, `planet.h`) ; 0 si le module n'est pas prêt ; compté par `par-nom` (1106 après D.5c), à typer par D.9. `pico8_ke_start_field` supprimé. `_ready` sur `g_jo_up`, `g_family_up`, `g_family_trade_up`. `ga.by` : `g_ga_by_shared`, `pico8_list_t` statique non `const` vidée à chaque init (précédent `ui.le`), contenu à typer par D.7a. `spec` après D.5c : 0, dans `DATA_RESIDUE_GUARD`. `ob` : 1721 hors test ponctuel.
> - D.6a — `overrides`, constructeurs de l'arbre `jo` (`pico8_jo_new` … `pico8_mx_new`), `new_entity` de `pico8_entity_kind_t` (`pico8_struct.h`) et ses appelants (`galaxy.c`, `jo_kv`…) : la table de clés de construction devient une struct locale (troisième puce), paramètre renommé. Empreinte 11 (8, 3). Compte : `overrides` = 315. §6 : `gh_row_lq`, commentaire de la l. 2 seulement ; aucune lecture ne change, pas de preuve rouge. `class-coverage-net` : oui. Découvert à D.6a : un commit (30 chemins : `research_icon.c` pour `ie:ldown`, `pico8_game_data.*` et quatorze TU qui construisent dans une variable `b`, hors du majorant). Clés typées : `pico8_jo_keys_t` (`id`, `p`, `ra`, NULL = absent, `pico8_struct.h`) pour neuf classes et `new_entity` ; `pico8_ga_keys_t` (`jo` et `pq`, NULL = défauts de `ga` ; `k_` NULL sous `pq` = nil) ; la chaîne que `pm:pq` parsait devient `pico8_pm_ga_pq_keys` (`pico8_ga_pq_keys_t`, générée) ; la sorte `ga` passe par une adaptatrice (`pico8_ga_kind_new`, `pq` NULL). `pico8_X_new(NULL)` inchangé ; les classes « seulement `id` » (`qh`, `nl`, `os`, `hj`, `fr`, `mx`) l'assertent sur `p`/`ra`. Tests de mort d'une clé positionnelle, sans membre ou hors rangée à la construction retirés (le type les interdit). `pico8_ob.h` quitte `planet.h`, `planet_family.h`, `planet_family_trade.h` ; `pico8_struct.h` le garde (`slipway.h`, `wormhole.h` en dépendent par lui). Le commentaire l. 2 de `gh_row_lq` parle des clés de `js` : il passe à D.6b (le changer ici serait un renommage à type constant). `overrides` après D.6a : 329 lignes / 348 occurrences / 55 fichiers (au lieu de 316 : le « = 315 » de D.1 était le total des fichiers de l'empreinte, dont 30 lignes de D.6b/D.6c — `iv`/`jc` de `planet.c` et `planet_family_trade.c`, `jc` de `test_pico8_mx`, `gh_row_lq`, `ie` de `test_pico8_entity_kind` — et sans 16 lignes de D.6a — `ie:ldown`, `nr` de `test_pico8_mz`/`test_pico8_bg` ; la ligne de `k_` ajoutée par D.4a est dans le périmètre) ; le compte de D.6b est à reprendre : sur les 329 lignes, 232 sont à D.6b (lignes classées par le constructeur appelé ou déclaré, commentaires de `lq`/`jb`/`gh_row_lq` compris, `iv` d'`economy.c` et `jc` des TU `hp`/`hy`/`trade`/`ui`/`mx`/`mz` compris) et 97 à D.6c : « `overrides` = 97 » après D.6b au lieu de 117 ; le « = 0 » de D.6c est inchangé. `par-nom` : 1075.
> - D.6b — `overrides`, mise en page et entités d'écran : `lq`/`jb` (`layout`), `js`/`fq` (`ui_button`), `mh`/`iv` (`msgbox`), `ui`, `qb` (`scroll`), `nf` (`mouse_reader`), `bg`, `mz` (`wormhole`), `jc` (`slipway`), appelants de `menu.c`, même règle. Empreinte 30 (19, 11). Compte : `overrides` = 117. §6 : `layout_nil_item_draws_nothing` (l. 106-112) passe son item par la struct de clés de `pico8_lq_new` ; l'item reste un `pico8_value_t`, élément de `lq.items` ; même intention (nil et fixe 0), preuve rouge. `class-coverage-net` : oui. Découvert à D.6b : trois chunks (classes sans clé ; `iv`/`jc`/`fq` ; `lq`/`jb`/`js`/`mh` avec les ponts, `dr` et le §6 — 58 chemins : appelants à NULL, `economy.c`, `score_panel_display`, `research_display`, `data_init` et les TU qui construisent dans `b` hors du majorant). `bg`, `nf`, `qb`, `ui`, `mz` sans paramètre ; clés typées `pico8_iv_keys_t`, `pico8_jc_keys_t`, `pico8_fq_keys_t`, `pico8_lq_keys_t` (items `pico8_value_t`), `pico8_js_keys_t` (`lq`/`h_`/`jb` restent des sources de rangée `pico8_value_t`, présence TROU-23), `pico8_mh_keys_t` ; `jb` prend `const pico8_jb_row_t *`. `js:pq` copie une rangée par `pico8_lq_keys_of` et une liste par ses éléments (plus de vue ni de conversion) ; `jb:pq` n'emballe plus ses lignes chaîne ; `fq(de)` lit `pico8_de_my()`. Pont `pico8_dr()` fermé (`pico8_jb_new(&pico8_dr_table[i])`). Ponts nouveaux, paramètre `overrides` gardé (type inchangé, comptés, 21 lignes) : `pico8_lq_new_table`, `pico8_jb_new_table` (`layout.c`), `pico8_js_new_table` (`ui_button.c`), `pico8_mh_new_table` (`msgbox.c`) — fermés quand les rangées de `pz` et les sources `lq`/`h_`/`jb`/`mh` sont typées (`planet_display`, `menu`, `score_panel_display`, `research_display` : D.7a ; `help`, `msgbox` : D.7b ; le « 0 dans `layout` » de D.7a exige les deux premiers). Les TU d'un pont gardent leur table : `test_pico8_mh` (clé `lh` nil, clés refusées : 7 lignes `overrides`, table de même type et même nom) et `test_pico8_lq` (deux commentaires qui nomment la table passée au pont) tombent avec lui. Reportés, avec eux : `pico8_ix()`/`pico8_ha()`, `g_bo`, `pico8_bz()` (vues par init ; `ix` porte un item imbriqué que seule une vue sait donner à `lq:mn`), les vues à l'appel de `hp.lq` et de `kw`. `pico8_lq_table_view` sans appelant de `src/` ; une table qui porte `fs` ou `w` est refusée par le pont `lq` (aucun producteur n'en pose). `overrides` après D.6b : 127 lignes / 138 occurrences / 27 fichiers (97 lignes de D.6c + 21 des ponts + 9 des TU des ponts ; le « = 117 » de D.1 était un total de fichiers, qui comptait dans ceux de D.6c 23 lignes de D.6b — `iv` de `economy.c`, `jc` de `test_pico8_hp`/`_hy`/`_trade`) ; le « = 0 » de D.6c devient « = 30 » (les ponts et leurs TU), et `overrides` entre dans `DATA_RESIDUE_GUARD` à l'étape qui ferme le dernier pont. D.7a hérite d'un chantier plus lourd : fermer les ponts `lq`/`jb` demande de typer les producteurs de rangées de `planet_display`, `menu`, `score_panel_display`, `research_display` et les rangées `bs` de `pz` (fichier de D.7b) ; son PRD tranche l'ordre ou la fusion avec D.7b. `par-nom` : 1062.
> - D.6c — `overrides`, reste : `g_` (`probe`), `fv` (`help`), `ie` (`research_icon`, `cart.c`), `hp` (`research`), `hy` (`economy`), `trade` (et `planet_trade.c`), `pj` (`score_panel`), même règle. Empreinte 21 (16, 5). Compte : `overrides` = 0, ajouté à `DATA_RESIDUE_GUARD`. §6 : aucun. `class-coverage-net` : oui. Découvert à D.6c : deux chunks (`hy`/`fv`/`pj`/`g_`/`ie`/`trade`, puis `hp` et le pont `fd` — 37 chemins : appelants à NULL, TU qui construisent dans `b`/`o`, `data_init`, `collect` et le test ponctuel hors du majorant). `hy`, `fv`, `pj` sans paramètre ; clés typées `pico8_g__keys_t`, `pico8_ie_keys_t`, `pico8_trade_keys_t` (triple `qe` du membre ; TROU-24 au site de `trade:kg`), `pico8_hp_keys_t` (`ri` : enregistrement `pico8_fd_entry_t`, `rr`) ; `hp` applique `ho` aux champs typés (`pico8_ho_gp_has`, extraite de `collect.c`). Pont `pico8_fd()` fermé (`pico8_research_build_gb(void)` parcourt `pico8_fd_table`) ; `pico8_ho_apply` sans appelant de `src/` (D.7a `collect`, ou D.8 avec le test ponctuel). `overrides` après D.6c : 30 lignes / 34 occurrences / 8 fichiers (les 21 lignes des quatre ponts de D.6b et les 9 de leurs TU, le « = 30 » de la note de D.6b ; le « = 0 » de D.1 comptait sans eux) ; `overrides` n'entre pas dans `DATA_RESIDUE_GUARD` : il y entre à l'étape qui ferme le dernier pont — `js`, dont les rangées `bs` de `pz` sont à D.7b ; D.7a ferme `lq`, `jb`, `mh` et les autres sources de `js` si son PRD ne réordonne pas. La table `{qe=r.e}` de `jo:cn` (`planet_trade.c`, `jy`) reste une `pico8_ob_table_new` : ce n'est pas une clé de construction (D.7a). `par-nom` : 1038.
> - D.7a — `ob`, tables ad hoc de l'affichage, de l'économie, de la recherche et des données (troisième puce) : tout `ob` restant de `planet_display`, `research_display`, `score_panel_display` (`g_de`, `spec1`/`spec2`), `slipway_display`, `wormhole_display`, `layout`, `menu`, `collect`, `economy*`, `research` (`g_gb`, `pico8_fi`), `planet_trade` (enregistrements de `jo:ib`), `tech_gate`, `score_panel`, `pi`, `gen`, `galaxy`, `cart`, `data_init`, `game_data`, `mouse` → structs locales ou tableaux fixes, les TU touchés suivant dans le même commit. Empreinte 39 (39, 0) aujourd'hui. Compte : `ob` ≤ 1057, et 0 dans ces fichiers. §6 : `ga_export_panel_by` (`route_a`/`route_b`, l. 102-103 et 147-150, enregistrements de `jo:ib` poussés dans `ga.by`), preuve rouge. `class-coverage-net` : seulement si le diff touche un constructeur. Découvert à D.7a : six chunks (familles isolées ; `gb` ; enregistrements `{qe,gc}` ; articles de `bl` et panneau de score ; pipeline d'affichage et chaînon `pz` ; `nb` — 74 chemins : suiveurs forcés, TU qui lisent `gb` ou les ponts, test ponctuel hors du majorant), **commis dans l'ordre 1, 2, 4, 3, 5, 6** : les enregistrements `{qe,gc}` de `jo.jy` (chunk 3) et les sous-rangées de `bz` (chunk 4) sont des tuples de `lq:mn`, que `layout.c` lisait comme tables ob ; typer l'une ou l'autre famille avant ce lecteur demandait une conversion typée → ob ou un `lq:mn` à double lecture, tous deux interdits. Le chunk 4 a donc pris tous les tuples de `lq:mn` (et les articles de `bl`, qui partageaient la même vue de `data_init`), le 3 a suivi, le 5 est resté le chaînon `pz` sans les tuples ; décision du pilote. **Ordre D.7a/D.7b : D.7a a pris à D.7b le chaînon `pz`** (`pico8_pz`, `pico8_mh_new_table`, `g_bo` et `fv.rb` de `help.c`, `pico8_js_new_table` et les branches `OB_TABLE` de `js:pq`), **`g_nf_l`/`g_nf_r`** (avec `bu`) et les lignes qui lisaient ses types convertis dans `planet.c`, `planet_family_trade.c`, `ui.c` ; D.7b ne les a plus. Formes : rangées, tuples de `lq:mn` et articles de `bl` en `pico8_jb_row_t` (sorte REF opaque pour `ld` ; `{qe=x}` gardé comme `has_qe`/`qe` de la rangée et lu `{x,5}`) ; `pz(p, bs, n)` sur `pico8_js_keys_t` ; sources `lq`/`h_`/`jb` typées (`lq_row`/`h_row` : `pico8_lq_keys_t`, `pico8_lq_keys_of_row` ; `jb_row`) ; `{qe,gc}` en `pico8_jy_t` ; `gb` tableau fixe (`pico8_gb_at`, `pico8_gb_find`) ; `nb.e_` (`pico8_nb_e_`, remis par `pico8_data_init`), `pj.ou`, `pj.ea`, `bz` (`pico8_pj_bz`), `nf.l`/`nf.r` statiques non `const` ; `pico8_de()` supprimé (`pico8_de_my`). Vues fermées : `ix`, `ha`, `bo`, `bz`, `nb`, `pico8_jb_row_view`, `jo_kw_view`, `hp:jb`, `pico8_lq_table_view` ; `pico8_data_init(void)`. `overrides` = 0, dans `DATA_RESIDUE_GUARD` depuis le chunk 5. `ob` dans les fichiers de la ligne : 13 lignes (enregistrements de constructeurs de `ob()` `'v'`/`'s'` dans `pi.c` et leur commentaire de `pi.h`, `'b'` et `pico8_ho_apply` dans `collect.c`/`.h`), servant le test ponctuel, supprimées par D.8 ; le « = 187 » de D.7c devient 187 + 13. `ob` après D.7a : 552 lignes / 613 occurrences, dont 199 dans le test ponctuel (hors test ponctuel : 353) ; majorant de D.7b : ≤ 309 hors test ponctuel (353 moins les 44 lignes `ob` de ses fichiers de `src/`). Les greps de sortie `pico8_fi\(` et `pico8_ob_table_t \*g_` ne rendent plus que `pico8_ob.c/.h` et `pico8_stash.c`, laissés à D.8. Fuite par frame (LeakSanitizer, `message_box_hold`/`game_end_final_score`) : 142406 / 110943 → 19213 / 59310 octets (3480 / 2423 → 1099 / 1481 allocations). `par-nom` : 1009. Correctifs aux messages de commit de D.7a (comptes remesurés sur chaque arbre) : (1) classification des lignes retirées, (a) construction supprimée, (b) signature, contrat ou variable dont le type passe d'une table ob à un type C, (c) même type, autre nom : 5e58f7c, `ob` 340 lignes = (a) 317 + (b) 23 (les contrats de `pico8_pz` dans `msgbox.h` et de `pico8_hp_jb` dans `research_display.c/.h` ; les variables retypées `b` de `pz_step`, `row` de `jo:fe`, `l` de `jo.ex`, `row` de `cr_step` ; dans les TU, `build`, `fold_built_from`, `positionals`, `call_row_my` et leurs 12 variables `t`/`spec`/`lq_t`/`h_t`/`b`/`jb`), `overrides` 18 = (a) 18 (les trois ponts et leurs deux commentaires de TU) ; (c) = 1, hors de toute catégorie comptée : `pico8_bz()` → `pico8_pj_bz()`, renommage à type égal fait après que fa997e7 avait déjà typé son retour, parce que la grep de sortie `pico8_(ix|ha|nb|bz|de)\(\)` contredisait la l. 326 du PRD (qui fait rendre à `pico8_bz` les rangées typées) ; aucun compte n'en bouge. 1a1687a, `ob` 58 lignes = (a) 54 + (b) 4 (`t` de `cr_step` et du prédicat de `cb`, `tech` de `test_pico8_data_init`, `record` du test ponctuel), `overrides` 0, aucune (c). (2) lignes qui gagnent une catégorie : 5e58f7c, `valeur` `help.c` +1, `layout.h` +1, `menu.h` +1, `ui_button.h` +1 ; `type-tag` `test_pico8_js` +1 ; `dispatch` `menu.h` +1, `test_pico8_fq` +1 ; 1a1687a, `ob` +7 et `valeur` +6 dans le test ponctuel (lecture de la référence décodée) ; fa997e7 omettait `valeur` `help.c` +1 et `layout.c` +2 ; c5bee4b : `valeur` `research.h` +1, `test_pico8_hp` +2, `test_pico8_lq` +4, `test_pico8_trade` +2. (3) fa997e7 a aussi ajouté une lecture ob → typée temporaire dans `jo:gh` (`jo_gh_record_tuple`, `planet_display.c` l. 85 : `qe` lu dans l'enregistrement ob pour poser le tuple typé), retirée par c5bee4b : l'ordre 4 avant 3 demandait lui aussi une conversion, le temps d'un commit. (4) c5bee4b : `sizeof(pico8_jy_t)` vaut 48, pas 40 (40 est `pico8_jb_row_t`). (5) La hausse de fuite de c5bee4b à 5e58f7c (+32 o / +4 allocations sur `message_box_hold`, +8 o / +1 sur `game_end_final_score`) vient d'allocations faites une fois par `pico8_fv_pq` à la construction de l'écran d'aide (le `ra` partagé et les `qf` de ses boutons, joignables par `g_bo` avant), pas d'une fuite par frame.
> - D.7b — `ob`, reste de `src/` hors `pico8_ob.*` et `pico8_stash.*` : entités et interface (`planet*`, `probe`, `particle`, `slipway`, `wormhole`, `msgbox`, `mouse_reader` avec `g_nf_l`/`g_nf_r`, `ui*`, `help`, `scroll`, `trade`, `bg`, `ke`, `entity`, `bootstrap`, `hud`, `research_icon`) et en-têtes (`pico8_struct.h`, `pico8_type_tag.h`, `pico8_ref_audit.h`, `pico8_value.h`, `pico8_list.h`), même règle. Empreinte 42 (42, 0) aujourd'hui. Compte : `ob` ≤ 827, et 0 dans `src/` hors ces deux modules. §6 : aucun. `class-coverage-net` : seulement si le diff touche un constructeur. Découvert à D.7b : trois chunks (`nf.l` dans `test_pico8_spec_defaults`, qu'une table ob écrivait hors du `pico8_nf_button_t` de D.7a ; particules de `qy` ; inclusions et `assert` d'init — 26 chemins). Le TU corrompu : `test_second_jl_keeps_specs` posait une clé de table ob dans `g_nf_l` (16 octets) et débordait sur `g_nf_r` depuis `e252fde` ; il passait en relisant sa propre corruption. Réparé (marque dans `hm`), preuve rouge sous ASan. Un balayage des 81 TU sous `-fsanitize=address,undefined` (copie jetable, `Makefile` inchangé) n'a rien trouvé d'autre ; les TU ne sont compilés sous ASan dans aucune cible. Formes : `pico8_qy_t` (`p`, `v`, `has_a`/`a`, `l` par valeur ; une allocation par `mm`, mise à jour en place par `ei`, plus d'allocation par frame) ; `pico8_help_init` lit `pico8_pi_dispatch_lookup` ; `slipway.h`, `ui.h`, `wormhole.h` incluent `pico8_list.h`. `pico8_ob.h` n'est plus inclus dans `src/` que par `pico8_ob.c`, `pico8_collect.h`, `pico8_stash.h`. R = 22 : les 13 de D.7a et 9 lignes de commentaire qui décrivent la couche encore vivante (`pico8_type_tag.h` 2, `pico8_ref_audit.h` 4, `pico8_value.h` 1, `pico8_list.h` 1, `pico8_hud.c` 1), que D.8 retire ou réécrit avec `pico8_ob.h` et `PICO8_TYPE_TAG_OB_TABLE` ; le « 187 + 13 » de D.7c devient 187 + 22. `ob` après D.7b : 491 lignes / 550 occurrences (hors test ponctuel : 292 ; `src/` : 147 ; TU hors test ponctuel, `test_pico8_ob` et `test_pico8_stash` : 83 lignes dans 28 fichiers, le travail de D.7c). Fuite par frame (LeakSanitizer, `message_box_hold`/`game_end_final_score`) : 19213 / 59310 → 19213 / 58910 octets (1099 / 1481 → 1099 / 1431 allocations) ; `message_box_hold` n'exécute ni `ei` ni un `mm` (son `qy` reste vide) ; `tech_geoharvesting_burst` : 240568 → 47328 octets (23752 → 1682 allocations).
> - D.7c — `ob`, TU restants : fixtures construites en `pico8_ob_table_*` → structs, état construit directement. Empreinte 41 (0, 41) aujourd'hui. Compte : `ob` = 187 (`pico8_ob.c` 82, `pico8_ob.h` 32, `test_pico8_ob.c` 55, `pico8_stash.c` 8, `pico8_stash.h` 3, `test_pico8_stash.c` 7). §6 : aucun. `sanitize-net` et `class-coverage-net` : non, aucun fichier de `src/` ne change. Découvert à D.7c : trois commits (inclusions seules ; valeurs hors rangée, tests fermés et `by` ; plis de hachage — 29 chemins, 28 TU et ce plan, aucun de `src/`). Les cas hors rangée qui offraient une table ob à un membre typé sont retirés quand un frère garde la clause, remplacés par une liste sinon (`ma`/`m_` de `test_jo_writers_stop_off_row`) ; les tests fermés passent une liste locale ; `test_pico8_ga` poussait des tables ob dans `by`, qui tient des routes depuis D.7a : il pousse des routes. Aucune constante `*_HASH` ne change. Arbitrage du pilote : les trois octets de sorte de clé (`fold_names` de `test_pico8_hp`, `fold_qe_map` de `test_pico8_hy`, `fold_hg` de `test_pico8_jc`) ne deviennent pas des littéraux `0u`/`1u` (un identifiant compté remplacé par sa valeur est un alias) : ces trois lignes et les trois inclusions de `pico8_ob.h` qu'elles demandent restent, et D.8 les hérite avec la l. 2 de `test_pico8_planet_gh_row_lq` (R_t : l'énoncé du bug d'un test du §6 nomme `pico8_ob_table_t`), d'où « §6 : `gh_row_lq`, commentaire de la l. 2 seulement, pas de preuve rouge » à D.8. Les octets de `hp` et `hy` sont atteints (les retirer rougit `DEFAULTS_HASH` et `CONSTRUCTION_HASH`) : D.8 doit plier la même valeur sans la couche, arbitrage à reprendre, jamais une constante mise à jour ; celui de `jc` ne l'est pas (aucune constante ne plie un `hg` non vide : sauter toute la boucle de `fold_hg` laisse chaque constante verte). `ob` après D.7c : 415 lignes / 470 occurrences dans 20 fichiers (hors test ponctuel : 216 = 187 + 22 + 1 + 6, les 6 étant ces trois plis et leurs trois inclusions) ; `tests/` ne nomme plus `PICO8_TYPE_TAG_OB_TABLE` ; `type-tag` 373 → 362 lignes (592 → 572 occurrences). Balayage ASan+UBSan jetable des 28 TU touchés : propres.
> - D.8 — suppression : `pico8_ob.c/.h` (avec `pico8_fi`), `pico8_stash.c/.h`, `test_pico8_ob.c` et `test_pico8_stash.c` (§6, tests de la couche), le test ponctuel, `PICO8_TYPE_TAG_OB_TABLE`, leurs entrées du `Makefile`. Compte : `ob` = 0, `stash` = 0, ajoutés à `DATA_RESIDUE_GUARD`. §6 : aucun. `class-coverage-net` : non. Découvert à D.8 : deux commits (octet de `jc`, non atteint : `test_pico8_jc.c` seul, chaque constante inchangée, un octet remis reste vert ; suppression — 26 chemins, dont `tools/float_guard.txt`, que `make float-guard` réécrit dans `make test` : `garde fichiers` 60 → 58, les deux fichiers du coeur supprimés, précédent D.2 ; 27 en tout). `test_pico8_map_tables.c` perd la comparaison à la référence (`decode`, `parse`, les `*_SRC`, les comparateurs, `pico8_load_map_json`, `pico8_pi_init` et `pico8_data_init` de `main`) et garde les tests des modules typés qui ne la lisent pas : six tels quels, et les bornes de `pico8_int_seq_at`/`pico8_fixed_seq_at` et les recherches `r`/`iz`/`hc`/`df` reformulées sur la table typée, sans assertion nouvelle. Les octets de sorte de clé de `hp` et `hy` sont pliés par une constante locale nommée (`VIEW_INDEX_KEY`, existante ; `TABLE_INDEX_KEY`, nouvelle), octet gelé d'une énumération supprimée, (c) déclarées ; aucune constante `*_HASH` ne change. `PICO8_TYPE_TAG_OB_TABLE`, `PICO8_REF_AUDIT_EXPECT_OB_TABLE` et le compteur `ob_table` de l'audit supprimés (`tools/ref_audit_report.sh` inchangé, son mot `ob_table` est mort) ; `pico8_ho_apply`, les constructeurs `'b'`/`'v'`/`'s'` et leur enregistrement supprimés ; TROU-4 et TROU-9 sans site. `ob` = 0, `stash` = 0, dans `DATA_RESIDUE_GUARD` ; `type-tag` 352 lignes / 560 occurrences (au lieu de ≈ 354 : les renvois de `pico8_list.h` et de `pico8_ref_audit.h` à l'explication du pas de lecture de `pico8_type_tag.h`, retirée avec la table ob, partent aussi), `ref-audit` 56 / 57, `valeur` 4961 / 5742 ; `par-nom` 1009, `dispatch` 191 inchangés ; résidu D 5868 lignes dans 128 fichiers. `sanitize-net` propre, `class-coverage-net` identique. **Sortie de la phase non atteinte** : `pico8_value.c`, `pico8_type_tag.h`, `pico8_ref_audit.h` restent, avec `valeur`, `type-tag`, `ref-audit`, `par-nom`, `dispatch` ; la suite attend les décisions humaines de D.9.
>
> **Arrêt « champ sans règle » (§7), constaté à l'écriture de la liste : elle s'arrête à D.8.** Aucune puce de D ne dit ce que deviennent `par-nom` (1081/80) et `dispatch` (197/59). La première et la troisième puce ne couvrent que des tables : `ob` et `stash`, plus, par lecture, `spec` et `overrides`. La **Sortie** nomme des fichiers à supprimer, pas la forme de leurs usagers, et D.0 ne fait que compter. `valeur` (5989/132), `type-tag` (445/71) et `ref-audit` (70/17) ne peuvent tomber qu'avec eux. Les décisions de D.9 (ci-dessous) lèvent cet arrêt pour `par-nom`, `dispatch` et `pico8_list_t` (D.10 à D.12) ; la Sortie de D exige encore le travail sur `valeur`, `type-tag` et `ref-audit`, suspendu à Q12.
> 1. `par-nom` : `pico8_entity_get_field`/`set_field` (132 lignes de `src/`), `*field_truthy(`, les `field`/`set_field` des descripteurs (`pico8_struct_field_fn`, `pico8_struct_set_field_fn`), le `field` des sortes et 191 lignes d'aides `*_take_*`. La puce **Coexistence** de 1b les fait survivre à 1b, et aucune puce ne dit en quoi ils se changent. Les aides à nom non littéral (`field_fixed(inst, name)` et semblables, dans `economy_score.c`, `particle.c`, `probe.c`, `score_panel.c` et `entity_geom.c`) relèvent en plus de la clé dynamique du §5.
> 2. `dispatch` : `pico8_value_function_t methods[PICO8_METHOD_COUNT]` (`pico8_struct.h`), `pico8_mv()`, et les membres fonction (`js.my`/`hb`, `mh.lh`…). Typer les arguments demande une signature par méthode, qu'un tableau indexé par `pico8_method_t` ne tient pas. Or la note de D.0 dit que la table survit à D.
> 3. `pico8_list_t` : la note de D.0 exige d'en changer la représentation sans en fixer la forme. `lq.items` tient des chaînes, des fonctions, nil et le fixe 0 (`layout_nil_item_draws_nothing`), et aucune puce ne type ces éléments hétérogènes.
>
> - D.9 — **étude, sans conversion** (demandée par l'humain à l'arrêt) : pour chacun des trois points ci-dessus, recenser les sites par grep (fichier, fonction, noms lus ou écrits, genres des valeurs), les classer par forme de remplacement possible — accès au membre typé (`p->hp`) à la place de la lecture par nom ; fonction C par méthode, à arguments typés, à la place de `pico8_mv` et de la table `methods[]` ; tableau typé, union étiquetée locale ou struct à la place d'une `pico8_list_t` d'éléments `pico8_value_t` —, et pour chaque classe de sites : la forme retenue, le nombre de sites, ce qui n'entre dans aucune forme (clé dynamique du §5 incluse). Sortie : une section de ce plan qui propose les puces à ajouter à D (ou à reporter en phase 2), avec leur liste d'étapes et leurs comptes ; l'humain tranche avant la première étape qui les applique. D.2 à D.8 ne dépendent pas de cette étude.
>
> - D.12a — `liste`, registres : catégorie `liste` ajoutée à `tools/data_residue.sh` dans un premier commit seul, motif `\bpico8_list_[a-z_]+\b|\bPICO8_REF_AS_LIST[A-Z_]*|\bPICO8_TYPE_TAG_LIST\b|pico8_list\.h` (Q10 ; la règle de D.0 fige les motifs existants, mais une catégorie nouvelle s'ajoute dans son propre commit avec son motif énoncé) ; conteneur de pointeurs (Q9) sous un nom hors de ce motif ; registres L1 (`hv`, `cv[5]`, `cg[*]`, calques de `dc`, `ui.ic`, `ui.le`, `g_ga_by_shared`) et leurs parcours. Empreinte ≈ 15 (15, 0) : `entity.*`, `bootstrap`, `ui.*`, `economy*.c`, `cart`, `galaxy`, `bg`, `hud`. Compte : `liste` ≤ 927 moins L1, rapporté. §6 : aucun. `class-coverage-net` : seulement si le diff touche un constructeur. `sanitize-net` : oui.
> - D.12b — `liste`, listes membres L2 (`jo.oe`, `jo.cc`/`by`, `trade.lp`/`rl`, `mz.ox`, proposition `ib`, `qy`, `jc.ts`, `hy.invented`, `jo.jy`/`hp.i` ; `gen.oc` en tableau fixe) et aides L4 de `collect` (`jp`, `nv`, `gu`/`gu_content`, `la_append` ; prédicat `int (*)(void *, void *ctx)`). Empreinte ≈ 25 (25, 0). Compte : `liste` rapporté. §6 : aucun. `class-coverage-net` : seulement si le diff touche un constructeur. `sanitize-net` : oui.
> - D.12c — `liste`, listes hétérogènes L3 et locales L4 : `lq.items`, `mh.items`, `jb.ls` → union étiquetée locale à `layout` {rien, fixe, chaîne, rangée, autre-vrai} (Q10), `lq.fs` en tableau typé de fixes ; listes locales d'affichage (`planet_display`, `menu`, `msgbox:pz`, `trade:kg`, `research*`, `score_panel*`, `tech_gate`, `slipway`, `probe`, `galaxy`) en tableaux fixes là où la borne est dans le cart. Empreinte ≈ 16 (15, 1). Compte : `liste` = 0 dans `src/` hors `pico8_list.c/.h`, `pico8_ref_audit.h` et `pico8_type_tag.h` (49 lignes aujourd'hui, à D.12d). §6 : `layout_nil_item_draws_nothing` passe son item par l'union, même intention (nil et fixe 0), preuve rouge. `class-coverage-net` : seulement si le diff touche un constructeur. `sanitize-net` : oui.
> - D.12d — `liste`, TU (431 lignes, 38 fichiers ; `test_pico8_list` réécrit sur le nouveau conteneur, il ne perd pas son objet, §6), en deux chunks ; `pico8_list.c/.h` supprimés, `PICO8_REF_AS_LIST*` et `PICO8_TYPE_TAG_LIST` retirés. Empreinte 42 (4, 38). Compte : `liste` = 0, ajouté à `DATA_RESIDUE_GUARD`. §6 : aucun. `class-coverage-net`, `sanitize-net` : non.
>
> ### Phase 2 — Composition
>
> La hiérarchie par enchâssement de 1a devient :
>
> - un **noyau** `entity` commun (ce que `ke` apporte : table de méthodes, `me`, `t`, tags `nu` en masque de bits) ;
> - des **archétypes** `static const` : les défauts de chaque classe, résolus une fois le long de sa chaîne (`NL_DEF = { .hp=6, .s=68, … }`) ;
> - des **composants** optionnels portés par pointeur.
>
> Règles, appliquées sans supervision :
>
> 1. **Archétype ou état** : un champ va dans l'archétype si aucun code ne l'écrit sur une instance (prouvé par grep) ; sinon dans l'état mutable de la struct. L'état partagé de classe (`ie.gy`, `ie.id`) reste statique.
> 2. **Composant** : un groupe de champs devient un composant s'il apparaît dans **deux branches distinctes** de l'arbre (exemple connu : `py`/`fc`/`k_`/`qo`/`ro`/`qg`/`q_`/`du` dans `hj`, `pm`, `mx` et `ga`, alors que `ga` descend de `jo`). Sinon il reste dans la struct du type.
> 3. Test ponctuel, supprimé en fin de phase : pour chaque ancienne classe, chaque champ résolu par sa chaîne est égal à la valeur de l'archétype correspondant (piège connu : le `nu=o()` vide de `ga` masque celui de `jo`).
>
> - **Sortie** : aucune struct d'entité n'enchâsse plus la struct de sa parente (tout type = noyau + archétype + composants) ; chaque ancienne classe a son archétype `static const` ; le test ponctuel est passé pour chacune avant d'être supprimé.
>
> ### Phase 3 — Renommage
>
> Un champ est renommé **seulement** si `docs/glossaire-noms.md` en donne le sens. Sinon, il garde son nom minifié. Aucun sens n'est inventé. Un commit par groupe de champs renommés ; le glossaire est mis à jour en conséquence (l'entrée pointe du nom minifié vers le nouveau nom).

§6, recopié :

> - Les tests **de la couche** (`test_pico8_entity_smoke`, `test_pico8_ob`, `test_pico8_value`, et `test_pico8_list`/`test_pico8_stash` s'ils perdent leur objet) sont supprimés dans le commit qui supprime le code testé.
> - Les tests **de régression de bugs** (17 fichiers : `nil_row`, `gh_row_lq`, `ga_export_panel_by`, `game_end`, …) ne sont **jamais supprimés** par ce plan. Depuis la fusion de master (e2e rapide, #32), seuls `gh_row_lq`, `ga_export_panel_by` et `layout_nil_item_draws_nothing` restent ; les autres ont été remplacés par les e2e de `tests/e2e/`, qui en portent la garde. Chacun est réécrit pour lire la struct, dans le commit qui convertit la classe qu'il touche, et doit toujours échouer si le bug d'origine est réintroduit. Vérifié dans ce commit : bug remis localement, test rouge, correctif restauré.

§7, recopié :

> `plan-pilot`, phase après phase, un commit par classe (1a, 1b) ou par groupe (D, 2, 3).
>
> - **Une étape `plan-pilot`** = une classe en 1a/1b, ou une préparation sans conversion nommée dans la liste (1a.16b), un groupe nommé en D, 2 et 3. La liste ordonnée des étapes d'une phase (classes et instances sans classe en 1a/1b, dans l'ordre du §5 ; groupes en D, 2 et 3) est écrite dans ce plan à l'entrée de la phase, avant son premier commit — pour 1b, après 1b.0 et avant 1b.1, à partir de la table de 1b.0 ; elle fait foi sur le §2 quand ils divergent, et chaque PRD d'étape reprend sa ligne telle quelle.
> - Chaque commit de 1a passe `make class-coverage-net` (critère du PRD, hors `make test`), avec, pour chaque classe, un compte d'instanciations **égal** à celui de la porte 0.4 (valeurs versionnées à côté des exemptions à l'étape 1a.0). En 1b, seulement les étapes dont le diff touche un constructeur, un appel `pico8_class_coverage_record` ou `KNOWN[]` ; toujours à la sortie de phase (note de sortie : une passe identique à `tools/class_coverage_net_expected.txt`).
> - **ASan+UBSan, en 1b** : chaque étape ajoute un build ASan+UBSan du binaire de rejeu, passé sur `FRAMES_EVENTS` ; critère : sortie propre, sans comparaison d'empreinte. Si la durée mesurée à 1b.0 est trop longue, seulement aux étapes qui touchent une table de membres ou une boucle d'init, et à la sortie de phase.
> - **Filets depuis la fusion de master (e2e rapide)** : `FRAMES_EVENTS` n'existe plus ; `make e2e` rejoue les 26 e2e de `tests/e2e/` depuis l'écran titre, `make sanitize-net` et `make class-coverage-net` rejouent ce même jeu, `tools/class_coverage_net_expected.txt` y a été reposé à la fusion ; aucun TU ne rejoue de fichier d'événements : un TU construit son état directement, un rejeu est un e2e de `tests/e2e/` (moins de 180 frames, et il doit ajouter de la couverture de lignes). Les mentions de `FRAMES_EVENTS`, `frames-check` et `float-audit` dans les étapes livrées sont historiques.
> - **En D et en 2** (ajouté à la revue de sortie de 1b) : `make class-coverage-net` aux étapes dont le diff touche un constructeur, un appel `pico8_class_coverage_record` ou `KNOWN[]`, et à la sortie de phase ; `make sanitize-net` aux étapes qui changent la durée de vie ou le propriétaire d'une donnée (table ob → locale ou statique, `malloc`/`free` retiré), et à la sortie de phase.
> - **Preuve des tests, à partir de 1a.23** : `gcov` (test de l'étape compilé en `--coverage` dans une copie jetable du scratchpad) montre que chaque ligne ajoutée ou modifiée par la conversion est exécutée par la caractérisation ; s'y ajoutent **2 à 3 mutations** ciblées sur ce que `gcov` ne prouve pas (valeur d'un défaut propre, entrée du descripteur remplacée, ordre du constructeur). Le vérificateur relit le diff et le rapport `gcov`, sans banc de mutations propre. Une ligne non exécutée est soit couverte par un test ajouté, soit justifiée dans le message de commit. En 1b, les mutations comprennent au moins : (1) indicateur de présence forcé vrai sur un membre à nil observé ; (2) lecture emballée d'un fixe typé qui rend « présent 0 » là où il était absent ; (3) quand ça s'applique, boucle d'init qui saute un membre typé. Chacune rougit un test, sinon le trou est justifié dans le message de commit.
> - **Note « Découvert à 1a.N », à partir de 1a.27** : la ligne du plan ne garde que ce qui change une étape suivante — entrée d'enum ajoutée, fonction laissée à double forme **avec l'étape qui la fermera**, lecture nouvelle d'une règle du §5 ; le reste (sites, comptes, mesures) va dans le message de commit.
> - **Nommage des PRD** : `plan3-phase<X>-etape<M>-*.md` (X ∈ `0`, `1a`, `1b`, `D`, `2`, `3`). « La plus haute » s'entend dans l'ordre du plan — `0` < `1a` < `1b` < `D` < `2` < `3`, puis `M` numérique, puis la lettre (`16` < `16b`, `42a` < `42b`) —, jamais dans l'ordre texte des noms. Seuls ces fichiers de `docs/prd/done/` comptent pour trouver l'étape suivante — les `phaseD-*`/`phaseE-*` de Plan 2 n'en sont pas. Les étapes 0.1 et 0.2 sont livrées, 0.3 est humaine : l'autopilote démarre à la porte 0.4, puis au premier commit de 1a.
>
> L'autopilote **s'arrête et demande à l'humain** quand :
>
> - le filet reste rouge après **3 tentatives** de correction sur le même commit ;
> - un champ ne tombe sous aucune règle (§5 : clé dynamique à ensemble non fini ; en phase 2, champ ni archétype ni état, ou groupe au classement ambigu) ;
> - un test de régression de bug devrait être modifié **dans son intention** (pas seulement dans sa façon de lire l'état) pour passer ;
> - la porte de l'étape 0 échoue ;
> - le diff modifie ou supprime un `.sha256` du filet, ou retire un e2e de `tests/e2e/` — `make test` resterait vert.
>
> Un site ou une instance sans classe découvert en cours d'étape et **couvert par une règle existante du §5** n'est pas un arrêt : l'autopilote l'ajoute à la liste des étapes dans le commit qui le traite, avec une ligne « découvert à 1a.M » dans le message de commit. Seul un site qu'aucune règle ne couvre déclenche l'arrêt.
>
> Un écart port/cart découvert en cours d'étape n'est pas un arrêt : il est **préservé à l'identique**, décrit par une nouvelle entrée `TROU-N` de `docs/trous-de-portage.md` (source, atteinte par le filet) et référencé par `/* TROU-N */` au site, dans le commit de l'étape. Le corriger est interdit dans ce plan ; si le préserver exige un champ qu'aucune règle du §5 ne couvre, c'est l'arrêt « champ sans règle ».
>
> Chacune de ces conditions est écrite dans le PRD de l'étape concernée comme critère que le vérificateur de `prd-autopilot` traduit en `FAILED:` — c'est le seul arrêt que `plan-pilot` honore.

### Hypothèses sur l'issue de D.10b (`H-D10b-n`)

Chacune est ce que ce PRD suppose de l'état laissé par D.10b. Aucune n'est confirmée : D.10b est en cours d'implémentation, et ce PRD n'a lu que `ba4768d`.

- **H-D10b-1** : D.10b est gradée en deux commits (b1 rappels typés, b2 suppression du genre fonction), au-dessus de `ba4768d`, avec la note « Découvert à D.10b » de son gabarit ; elle y écrit que **D.10c n'a plus d'objet**. Le pilote graduera D.10c sans commit de code : D.11a suit alors directement D.10b.
- **H-D10b-2** : `dispatch` = **0** (`src/` et `tests/`), dans `DATA_RESIDUE_GUARD` (`spec,overrides,stash,ob,dispatch`).
- **H-D10b-3** : `pico8_value_function_t`, `pico8_value_function()` et `PICO8_VALUE_FUNCTION` n'existent plus. Les rappels ont le type de leur appelant : `js.my`/`js.hb` sont `void (*)(pico8_js_t *self)`, `fq.my` est `int (*)(pico8_fq_t *)` et `mh.lh` est `int (*)(pico8_mh_t *)`. `pico8_cr_row_my_dispatch`, `pico8_ix2_hb_dispatch` (`menu.c`) et `pico8_jo_fe_my_dispatch` (`planet_display.c`) gardent leur nom. Ils reçoivent un `pico8_js_t *self`, et leurs **lectures par nom sont intactes** : `_cr_t`, l'écriture de `lq`, `_jo_fe_pt`/`_jo_fe_b`/`_jo_fe_od`. Le PRD de D.10b les laisse à D.11a, et un `FAILED:` de D.10b l'exige.
- **H-D10b-4** : les lecteurs par nom ne répondent plus pour `my`/`hb` (`js`), `my` (`fq`), `lh` (`mh`), ni pour `lq` NULL (`hp`), et `pico8_js_my()` n'existe plus. Aucun site de D.11a ne lit ces noms.
- **H-D10b-5** : le site G `ui_button.c:603` (retour de `fq.my`) est retiré. `par-nom` `src/` vaut **418** (419 − 1). Le détail par fichier de `src/` est celui de `ba4768d`, sauf `ui_button.c` (16 → 15).
- **H-D10b-6** : `par-nom` `tests/` vaut 568 moins les lectures par nom de rappels que D.10b remplace (rapporté par D.10b). Le total `par-nom` de départ de D.11a est donc 418 + ce nombre.
- **H-D10b-7** : D.10b ne change ni `pico8_jo_is_family()` (`planet.c`), ni `jc_is_endpoint_class()` (`slipway.c`), ni `pico8_fm()`/`pico8_fm_hy()` (`probe.c`), ni `pico8_dk()`, ni `pico8_je()`/`pico8_mo()`, ni les structs `pico8_js_t`, `pico8_jo_t`, `pico8_jc_t`, `pico8_hy_t`, `pico8_qb_t`, `pico8_nf_t` hors des membres de rappel.
- **H-D10b-8** : D.10b ne touche aucune des lignes de TU que D.11a touche : `pico8_lt(&g_giver.header)` et `giver_t` de `test_pico8_ui`, lignes `pico8_ie_eh()`/`pico8_ie_set_gy()`/`hb_from_gy` de `test_pico8_entity_kind`, `test_pico8_fm`. Hors numéros de ligne : `test_pico8_entity_kind` est dans le chunk b2 de D.10b, pour ses bras `PICO8_VALUE_FUNCTION`.
- **H-D10b-9** : `valeur`, `type-tag`, `ref-audit` sont rapportés par D.10b. D.11a les remesure au départ et ne dépend d'aucune valeur exacte.
- **H-D10b-10** : aucun constructeur n'a changé de texte à D.10b, et `tools/class_coverage_net_expected.txt` est inchangé.

### Sections à revérifier quand D.10b sera gradée

Comparer à la note finale de D.10b et au HEAD gradé, puis corriger ce PRD avant de le passer en `docs/prd/triage/` :

1. ⟲ **État de départ** : comptes, occurrences, `par-nom` `src/` 418, `tests/`, résidu D, `DATA_RESIDUE_GUARD`. Corriger aussi le critère `FAILED:` de départ (HEAD attendu, comptes attendus).
2. ⟲ **Inventaire** : les numéros de ligne de `menu.c`, `planet_display.c`, `msgbox.c`, `score_panel.c`, `score_panel_display.c` (touchés par D.10b), et la forme des trois rappels de la classe A (`self` typé, H-D10b-3).
3. ⟲ **Comptes attendus par fichier** (table « Effet sur `make data-residue` ») et **classification attendue**.
4. ⟲ **Empreinte et découpage** : les listes de chemins de a1/a2 si D.10b touche un fichier de plus dans l'empreinte.
5. **« D.11a suit D.10b »** : ne tient que si la note de D.10b déclare D.10c vide (H-D10b-1). Sinon D.10c passe d'abord, et ce PRD attend.
6. **Règle Q8 de `pico8_fm_hy()`** : relire le contrat de `pico8_fm_hy()` à HEAD (H-D10b-7).
7. La note « Découvert à D.11a » (gabarit en « Further Notes »), qui cite les comptes de départ.

### Lecture de ces textes pour D.11a

- **Périmètre** : la ligne nomme quatre familles de sites, et ce PRD les prend telles quelles :
  - la classe A (« receveurs de type connu ») ;
  - la classe D (singletons `fm`/`je`/`mo`) ;
  - E `je.l`/`je.r` ;
  - les aides H « devenues sans appelant ».
  Il y ajoute les deux sites G que la note de D.10a lui a laissés : `slipway.c:261` et `research_icon.c:182` de la note, **l. 248 et l. 180 à `ba4768d`**. Les classes B, C, F, K, M, T1/T2 et l'infrastructure (lecteurs `field`, écrivains `set_field`, `kind_field`, aides `*_take_*` des écrivains) sont à D.11b-D.11e.
- **« cast vérifié vers la famille que le cart fixe »** : un accès au membre typé sur un en-tête `pico8_struct_header_t *` exige une preuve. La preuve est un test exécuté juste avant, dans la même fonction : `assert(pico8_jo_is_family(e) && "…")`, `e->cls == pico8_jc_struct_class()`, ou `e->cls == pico8_trade_struct_class()`. À défaut, c'est le type du receveur lui-même (`self` déjà typé, membre typé `pico8_js_t *`, `pico8_ui_t *`). Un cast sans l'une de ces preuves est un échec. Précédents : `call_gg_truthy` (`economy_score.c:78`), `bd_route_visit` (`economy_tiers.c:98`), `jo:kf` (`planet_trade.c:69`), `mx:kf` (`planet_family_trade.c:331`), `cheat.c:28`.
- **« `pico8_fm_hy()`, `pico8_je_nf()`, `pico8_mo_qb()`, NULL sur le remplaçant, Q8 »** : décision humaine. À `ba4768d`, `pico8_fm_hy()` rend NULL si l'emplacement est vide et **asserte** sur le remplaçant (`probe.c:745-753`). Or D.9 écrivait « `pico8_fm_hy()` existe déjà sur ce modèle ». Q8 lie : ce PRD aligne son contrat (voir « Singletons (G) »).
- **« le remplaçant de `fm` restant une struct typée pour `jk`/`nz` »** : `pico8_fm_placeholder_t` (`probe.h`) existe déjà. Seul un accès typé manque, pour les trois lectures de `jk`/`nz` qui peuvent le rencontrer.
- **« aides H devenues sans appelant »** : une aide par nom meurt quand son dernier appelant est converti **dans cette étape**. Une aide qui garde un appelant B, C, F, K ou M reste (lignes « H restantes » de l'inventaire).
- **« Empreinte ≈ 22 (22, 0) »** : remesurée à **33 fichiers de `src/`** et 3 TU (voir « Empreinte »). Trois raisons :
  - les accesseurs nouveaux vivent dans les en-têtes des modules qui possèdent chaque singleton ;
  - le cast vérifié de `ke` passe par `pico8_lt` (`entity.c/.h`, `ke.c`) ;
  - le site G de `ie.gy` retype un état de module public (`research_icon.c/.h`).
- **« Compte : `par-nom` ≈ 910 »** : l'estimation de ce PRD donne `src/` ≈ **342** (418 − 76), `tests/` inchangé à ± quelques lignes, soit un total ≈ 900 selon H-D10b-6. Le compte est rapporté, écart expliqué.
- **« §6 : aucun »** : `test_pico8_planet_gh_row_lq`, `test_pico8_ga_export_panel_by` et `test_pico8_layout_nil_item_draws_nothing` lisent par nom depuis un TU (D.11d). Aucun ne lit un site de `src/` que D.11a change de forme : les lecteurs `field` restent jusqu'à D.11e. Aucun n'est touché.
- **« `class-coverage-net` : seulement si le diff touche un constructeur. `sanitize-net` : non »** : aucun constructeur ne change de texte. Mais `pico8_lt`, la porte d'inscription de tous les constructeurs, change de type de paramètre (a2) : `class-coverage-net` est lancé à a2 (G, par prudence). Aucune durée de vie ni aucun propriétaire ne change, et les vecteurs de `ie.gy`, de `mo.v` et des `p` de souris restent fuités comme aujourd'hui (TROU-7) : pas de `sanitize-net`. Le balayage ASan+UBSan des TU est fait à la fin (consigne du pilote).
- **Leçons des notes à ne pas refaire** :
  - classification (a)/(b)/(c) absente ou fausse ;
  - (c) non déclarée ;
  - lignes qui gagnent une catégorie omises (correctifs de D.7a) ;
  - conversion typée → valeur ajoutée le temps d'un commit (D.7a, point 3) ;
  - constante de hachage « mise à jour » ;
  - octet gelé remplacé par un littéral nu (D.7c/D.8) ;
  - TU corrompu qui passait en relisant sa propre corruption (D.7b : balayage ASan des TU) ;
  - site « typé » en déplaçant son déballage (D.10b, user story 13).
- **Consignes de l'utilisateur, liantes** :
  - un TU ne rejoue jamais un `events.json`, et rien de lent ;
  - aucun `*.sha256` ni fichier de `tests/e2e/` touché ;
  - interdits : jonction de lignes, alias, registre, renommage pour échapper aux greps, cast non vérifié, cale valeur ↔ typé permanente ;
  - mutations restaurées par `cp` ;
  - binaires lancés exactement comme le `Makefile` (`ASAN_OPTIONS=detect_leaks=0` pour un TU lié sous ASan) ;
  - balayage ASan+UBSan des TU ;
  - découpage en chunks d'au plus ~40 chemins ;
  - jamais l'appli raylib.

### État de départ ⟲

**Mesuré sur `ba4768d`** (arbre archivé, D.10b non implémentée dans cet arbre). `sh tools/data_residue.sh`, `--detail` et la recette d'occurrences de D.8, sans build.

| Catégorie | Lignes | Fichiers | Occurrences | `src/` (lignes / occ.) | `tests/` (lignes) |
|---|---|---|---|---|---|
| `valeur` | 4276 | 125 | 4643 | 1512 | 2764 |
| `type-tag` | 347 | 64 | 551 | 113 | 234 |
| `ref-audit` | 55 | 15 | 56 | 55 | 0 |
| `par-nom` | 987 | 81 | 1103 | 419 / 480 | 568 |
| `dispatch` | 44 | 19 | 46 | 35 | 9 |
| **résidu D** | **5141** | **126** | — | 1898 | 3243 |

Attendu au départ de D.11a, selon H-D10b-2 à H-D10b-6 :
- `dispatch` 0 ;
- `par-nom` `src/` 418, dont `ui_button.c` 15 ;
- `par-nom` `tests/` 568 − k ;
- `valeur` en baisse d'environ 90 lignes, rapportée par D.10b.

L'implémenteur remesure tout sur le HEAD gradé.

`par-nom` `src/` par fichier de l'empreinte à `ba4768d` (lignes / occurrences) :
- `bg.c` 8/10, `bootstrap.c` 4/4, `economy.c` 3/4 ;
- `economy_formulas.c` 6/6, `economy_score.c` 14/14, `economy_tiers.c` 19/19 ;
- `entity.c` 12/13, `entity.h` 4/5, `entity_geom.c` 5/5, `galaxy.c` 1/1, `ke.c` 3/3 ;
- `menu.c` 6/7, `mouse_reader.c` 3/4, `msgbox.c` 12/14 ;
- `planet_display.c` 11/11, `planet_family.c` 59/71, `planet_family_trade.c` 33/40, `planet_trade.c` 2/2 ;
- `probe.c` 42/47, `probe.h` 1/1, `research_display.c` 1/1, `research_icon.c` 19/22, `research_icon.h` 1/1 ;
- `score_panel.c` 8/10, `score_panel_display.c` 2/2 ;
- `slipway.c` 21/26, `slipway_display.c` 1/1, `tech_gate.c` 4/4 ;
- `ui.c` 11/13, `ui.h` 1/1, `wormhole.c` 17/20, `wormhole_display.c` 1/1.

### Inventaire (mesuré sur `ba4768d`) ⟲

Recette : `sh tools/data_residue.sh --detail`, puis `grep -nE "$(pattern par-nom)" src/*.[ch]`. Pour les appels d'aides (lignes non comptées, nom littéral), `grep` de chaque aide à nom en paramètre. Chaque site a été lu. Le receveur est donné par son type statique au site et par la famille que le cart fixe (d'où il vient). Notations :
- **R** : lu ; **W** : écrit ;
- **c** : ligne comptée par `par-nom` ; **h** : ligne d'appel d'une aide (non comptée).

**Classe A — receveur de type connu (a2).** 40 lignes comptées et 38 appels d'aides sur 33 lignes, dans 18 fichiers.

| Fichier:ligne | Fonction | Membre | Receveur (type statique → famille fixée) | Remplacement | c/h |
|---|---|---|---|---|---|
| `bootstrap.c:80` | `pico8_dg` | R `me` | `qc` en-tête ∈ `hv` → `ke` | `ke->me` (NULL = absent) sur `pico8_ke_t *ke`, `hv` n'admettant que des `ke` (voir « `ke` sur `hv` ») | c |
| `bootstrap.c:87`, `:98` | `pico8_dg` | R `t` (×2) | idem | `ke->t` (toujours présent pour un `ke`) | c, c |
| `economy.c:304` | `pico8_hy_lj` | W `kr` | `p` ∈ `cg("jo")` → `jo` | `assert(pico8_jo_is_family(p))`, `jo->kr = kr; jo->has_kr = 1` | c |
| `economy_formulas.c:113`, `:116` | `bx_visit` | R `jy` (vérité), `kw` | `pt` ∈ `cg("jo")` → `jo` | `jo->jy != NULL` (vérité d'une liste présente), `jo->kw` | c, h |
| `economy_formulas.c:133` | `ck_visit` | R `jy` | `l` ∈ `cg("qh")` → `jo` | `jo->jy` | h |
| `economy_formulas.c:163` | `pico8_dj` | R `p` ×2 | `pb`, `to` = `jc.qu`, `jc.re` → extrémité de voie (famille `jo`, ou le point de souris `{ra,p}` de `jc:hq`) | `pico8_jc_end_p(to)`, `pico8_jc_end_p(pb)` (voir « Extrémités de voie ») | h (2 appels) |
| `economy_score.c:90` | `pico8_da` | R `mi` | `pt` → `jo` (`call_gg_truthy` l'asserte déjà) | `jo->has_mi ? jo->mi` | c |
| `economy_score.c:118`, `:122`, `:145`, `:173` | `pico8_db` | R `by`, `mi`, `hs`, `kw` | `pt` → `jo` | `jo->by`, `jo->mi`/`has_mi`, `jo->hs`, `jo->kw` | h, c, h, c |
| `economy_score.c:143`, `:159` | `pico8_db` | R `hf` (vérité), `mi` | `oz` = `trade:to()` → `jo` | vérité de `pico8_jo_hf(jo)` (TROU-3 inchangé), `jo->mi`/`has_mi` | c, c |
| `economy_score.c:152`, `:183` | `pico8_db` | R `trade` | `dk` : `const pico8_js_t *` (`pico8_dk()`) | `dk->has_trade ? dk->trade` | c, c |
| `economy_score.c:209` | `sum_by_mi_visit` | R `mi` | `pt` ∈ `cg("jo")` (`call_gg_truthy` l'asserte) | `jo->mi`/`has_mi` | c |
| `economy_tiers.c:79`, `:86` | `bd_reset_visit` | R/W `cc` | `pt` ∈ `cg("jo")` → `jo` | `jo->cc` (libéré, puis `jo->cc = cc`) | c, c |
| `economy_tiers.c:100` | `bd_route_visit` | W `by` | `pt` (asserté l. 98) | `jo->by = by` | c |
| `economy_tiers.c:118`, `:119` | `bd_route_visit` | R `rl`, `lp` | `route` ∈ `by` → `trade` | `assert(route->cls == pico8_trade_struct_class())`, `trade->rl`, `trade->lp` | h, h |
| `economy_tiers.c:142` | `bd_route_visit` | R `id` | `planet` ∈ `trade.lp` → `jo` | `jo->id` (NULL = absent : `continue`), passé à `pico8_jc_hg_add` (paramètre `pico8_value_t` inchangé) | c |
| `economy_tiers.c:147` | `bd_route_visit` | R `hg` | `w` ∈ `trade.rl` → `jc` | `assert(w->cls == pico8_jc_struct_class())`, `jc->hg` | c |
| `economy_tiers.c:159` | `bd_route_visit` | R `cc` | `dest` = `trade:to()` → `jo` | `jo->cc` | c |
| `economy_tiers.c:183`, `:190`, `:193`, `:200`, `:206`, `:220`, `:223` | `bb_visit` | R `mi`, `hn`, `jy`, W `mi`, R `hn`, `jh`, `kw` | `pt` ∈ `cg("jo")` → `jo` | `jo->mi`/`has_mi`, `jo->hn`, `jo->jy`, `jo->mi = jw; jo->has_mi = 1`, `jo->jh`/`has_jh`, `jo->kw` | c, h, c, c, h, c, c |
| `economy_tiers.c:248`, `:253`, `:254`, `:311`, `:325` | `pico8_dh` | R `me`, `jy`, `cc`, `hf` (vérité), `by` | `pt` → `jo` | `jo->ke.me`, `jo->jy`, `jo->cc`, vérité de `pico8_jo_hf(jo)`, `jo->by` | c, h, h, c, h |
| `economy_tiers.c:278` | `pico8_dh` | R `qe` | `im` ∈ `jo.cc` → `trade` | `assert` de classe `trade`, `trade->has_qe` et `pico8_trade_qe(trade)` (TROU-3 inchangé) | c |
| `entity_geom.c:74` | `pico8_ma` | R `t` | `e` : `jo:kj` (`jo`) ou `qb:ph` (`qb`) → `ke` | `assert(pico8_jo_is_family(e) \|\| e->cls == pico8_qb_struct_class())`, `((pico8_ke_t *)e)->t` | h |
| `galaxy.c:99` | `pico8_bw` | R `en` | `dk` : `const pico8_js_t *` | `dk->has_en ? dk->en` | c |
| `menu.c:112` | `pico8_cr_row_my_dispatch` | R `_cr_t` | `self` : `pico8_js_t *` (H-D10b-3) | `self->_cr_t` | c |
| `menu.c:347` | `pico8_ix2_hb_dispatch` | W `lq` | `self` : `pico8_js_t *` | `self->lq = built` (`pico8_lq_t *` rendu par `pico8_lq_new`) | c |
| `planet_display.c:307` | `pico8_jo_gv` | R `qu` | `s` tenu : `jc`, `bg`, `mz`, planète (tests `test_pico8_bg`/`_mz`) → seul `jc` porte `qu` | `s->cls == pico8_jc_struct_class() && ((const pico8_jc_t *)s)->qu == self` | c |
| `planet_display.c:335`, `:339`, `:342` | `pico8_jo_fe_my_dispatch` | R `_jo_fe_pt`, `_jo_fe_b`, `_jo_fe_od` | `self` : `pico8_js_t *` | `self->_jo_fe_pt`, `self->_jo_fe_b`, `self->has__jo_fe_od ? self->_jo_fe_od` | c, c, c |
| `planet_family_trade.c:38` | `pico8_nr_gv` | R `qu` | `s` tenu (comme `jo:gv`) | test de classe `jc`, `->qu == self` | c |
| `planet_family_trade.c:342` | `pico8_mx_kf` | R `jy` | `cp` = `jc:eb(self)` → `jo` | `assert(pico8_jo_is_family(cp))`, `jo->jy` | c |
| `planet_trade.c:74` | `pico8_jo_kf` | R `fx` | `other` = `jc:eb(self)` → `jo` | `jo->has_fx && jo->fx` | c |
| `probe.c:177`, `:178`, `:179`, `:180` | `pico8_bv_jc_predicate` | R `qu`, `re`, `p` ×2 | `sw` ∈ `cg("jc")` → `jc` ; `qu`/`re` → extrémités | `assert` de classe `jc`, `jc->qu`, `jc->re`, `pico8_jc_end_p(qu)`, `pico8_jc_end_p(re)` | h ×4 |
| `research_display.c:327` | `pico8_bj_predicate` | R `nk` | `ctx->jo_instance` (`pico8_bj(self)` de `jo:ex`) → `jo` | `assert(pico8_jo_is_family(…))`, `jo->nk` | c |
| `score_panel_display.c:230` | `pico8_pj_ph_map` | R `px` | `dk` : `const pico8_js_t *` | `dk->px` (NULL = absent) | c |
| `slipway.c:227` | `pico8_jc_hq` | R `o_` | `ok` : `pico8_ui_t *` (`pico8_ok()`) | `ok != NULL ? ok->o_ : NULL` | h |
| `slipway.c:320` | `pico8_jc_ce_np_predicate` | R `p` | `n` ∈ `cg("np")` → `jo` | `pico8_jc_end_p(n)` (branche `jo`) | h |
| `slipway.c:340`, `:345`, `:346`, `:347` | `pico8_jc_ce_jc_predicate` | R `mt`, `qu`, `re`, `p` ×2 | `sw` ∈ `cg("jc")` → `jc` | `assert` de classe `jc`, `jc->has_mt`/`mt` comparés à `ctx` (typé), `jc->qu`, `jc->re`, `pico8_jc_end_p` | h ×5 |
| `slipway.c:365`, `:366`, `:377` | `pico8_jc_ce` | R `p` ×2, `oe` | `self->qu`, `self->re` → extrémités ; `qu` → `jo` (asserté à la construction par `jc_is_endpoint_class`) | `pico8_jc_end_p(qu/re)`, `assert(pico8_jo_is_family(qu))`, `jo->oe` | h ×3 |
| `slipway_display.c:48`, `:51`, `:52` | `pico8_jc_ch` | R `id`, `ra` ×2 | `f`, `t` = `qu`/`re` du `jc` dessiné → extrémités | `id` : `assert(pico8_jo_is_family(t))`, `jo->id` passé à `pico8_jc_hg_has` (paramètre `pico8_value_t` inchangé, voir « Consommateurs à valeur ») ; `pico8_jc_end_ra(f)`, `pico8_jc_end_ra(t)` | h ×3 |
| `slipway_display.c:88`, `:90` | `pico8_jc_lg` | R `ra` ×2 | `qu`, `re` → extrémités | `pico8_jc_end_ra(qu)`, `pico8_jc_end_ra(re)` | h ×2 |
| `wormhole.c:213`, `:215` | `pico8_mz_pr` | R `me`, `p` | `pt` ∈ `cg("jo")` → `jo` | `assert(pico8_jo_is_family(pt))`, `jo->ke.me`, `jo->p` | c, h |

**Classe G laissée à D.11a (a2).** 2 sites et 2 définitions d'aides `*_take_*` devenues sans appelant.

| Fichier:ligne | Fonction | Déballage | Remplacement |
|---|---|---|---|
| `slipway.c:246-248` (note de D.10a : l. 261) | `pico8_jc_hq` | `mt` lu par nom sur `qu`, puis sur `re`, déballé par `jc_take_fixed(mt_val, &self->mt, &self->has_mt)` | seul `fr` porte `mt` (`pico8_fr_field`, `planet_family_trade.c:273`) ; `fr` n'a pas de fille : test de classe exact `e->cls == pico8_fr_struct_class()`, puis `((const pico8_fr_t *)e)->mt`. `qu` d'abord, `re` si `qu` n'en a pas (le `or` du cart : un `mt` présent est un fixe, toujours vrai). `jc_take_fixed` (déf. l. 161) sans appelant : supprimée |
| `research_icon.c:180` (note : l. 182) | `pico8_ie_hb` | `ie.gy`, état de module `pico8_value_t g_ie_gy`, déballé par `ie_take_vec(g_ie_gy, &self->ra)` | `g_ie_gy` devient `const pico8_vec_t *` (NULL = nil). `pico8_ie_set_gy(const pico8_vec_t *)` ; `pico8_ie_eh()` rend `const pico8_vec_t *` (NULL hors init) ; `self->ra = g_ie_gy`. Le lecteur `"gy"` de `pico8_ie_field` emballe le pointeur (couche de D.11e). `ie_take_vec` (déf. l. 138) sans appelant : supprimée. `cart.c:141` (`pico8_ie_set_gy(pico8_ie_eh())`) compile tel quel |

**Classe D — singletons (a1).** 8 lignes comptées et 22 appels d'aides sur 21 lignes, dans 11 fichiers.

| Fichier:ligne | Fonction | Singleton, membre | Remplacement |
|---|---|---|---|
| `bg.c:272` | `pico8_bg_hq` | `mo` W `v` | `mo != NULL` : `qb = pico8_mo_qb(); assert(qb != NULL && "…")` (le remplaçant n'a pas d'écrivain : l'arrêt d'aujourd'hui est gardé), `qb->v = v_new` — c |
| `planet_family.c:750` | `pico8_ga_pq` | `mo` W `ma` | idem, `qb->ma = 3` — c |
| `menu.c:206`, `:209` | `pico8_cr` | `fm` R `ew`, `invented` | `hy = pico8_fm_hy()` : `hy ? hy->ew : 0`, `hy && hy->invented ? count : 0` — c, c |
| `tech_gate.c:35` | `pico8_oj` | `fm` R `invented` | `hy && hy->invented` — c |
| `tech_gate.c:57` | `pico8_gi_cost` | `fm` R `fu` | `hy && hy->fu` — c |
| `tech_gate.c:95` | `pico8_cb_predicate` | `fm` R `jk` | `hy ? hy->jk` ; sinon `ph = pico8_fm_placeholder()`, `ph && ph->has_jk ? ph->jk` ; sinon 0 — c |
| `tech_gate.c:102` | `pico8_cb_predicate` | `fm` R `ew` | `hy ? hy->ew : 0` — c |
| `economy_score.c:230`, `:231`, `:233` | `pico8_ky` | `fm` R `happiness`, `invented`, `po` | `hy && hy->has_happiness`, `hy->invented`, `hy->po` — h ×3 |
| `economy_score.c:272` | `pico8_bf` | `fm` R `po` | `hy ? hy->po : NULL` — h |
| `score_panel.c:226`-`232` | `pico8_pj_hb` | `fm` R `ge`, `nz`, `jk` (valeur), `happiness`, `lo`, `yr` (valeur), `po` | `hy` pour tous. `jk`/`nz` du remplaçant par `pico8_fm_placeholder()` (présence `has_jk`/`has_nz`). `fm_jk`/`fm_yr` restent des `pico8_value_t` (destination : l'item de rangée `ou[2]`/`ou[6]`, inchangée), construits du membre : fixe présent, sinon nil — h ×7 |
| `score_panel_display.c:221`, `:274` | `pico8_pj_ph_map`, `pico8_pj_ph` | `fm` R `hl` | `hy && hy->hl ? chaîne : "current score"` (même repli) — h ×2 |
| `entity_geom.c:49` | `pico8_gd` | `mo` R `ra` | `qb = pico8_mo_qb()`, `qb && qb->ra ? *qb->ra : zéro` — h |
| `msgbox.c:210`, `:211` | `pico8_iv_pq` | `je` R `ra`, `mo` R `ra` | `nf = pico8_je_nf()`, `nf && nf->ra ? *nf->ra : zéro` ; `qb` idem — h, h |
| `probe.c:285` | `pico8_dl` | `je` R `ra` | idem — h |
| `ui.c:260` | `pico8_ui_lj` | `je` R `ra`, `mo` R `ra` | idem — h (2 appels) |
| `ui.c:301` | `pico8_ui_ev` | `je` R `ra` | idem — h |
| `ui.c:326`, `:384` | `pico8_ui_gw`, `pico8_ui_bh` | `je` R `ra` (brut : pointeur ou NULL) | `nf ? nf->ra : NULL` (le `je_ra_val` intermédiaire part) — h, h |

**Classe E (a1).** 1 ligne comptée.

| Fichier:ligne | Fonction | Remplacement |
|---|---|---|
| `ui.c:81` | `pico8_ui_je_button_table(bt)`, `bt` ∈ {`l`, `r`} (assert gardé) | `pico8_je_nf() == NULL` → NULL (le remplaçant n'a pas de table de bouton) ; sinon `pico8_nf_l()` ou `pico8_nf_r()`, nouveaux accesseurs de `mouse_reader.c`, NULL hors de `pico8_nf_init`/`_shutdown` (même règle que le lecteur `"l"`/`"r"` de `pico8_nf_field`) |

**Aides H qui meurent à D.11a** (définitions, 20 lignes comptées). Le chunk est celui où part le dernier appelant.

| Fichier:ligne | Aide | Derniers appelants | Chunk |
|---|---|---|---|
| `economy_formulas.c:20`/`22`, `:30`, `:38`, `:46` | `field_truthy`, `field_vec`, `field_kw`, `field_list` | A seulement | a2 |
| `economy_score.c:27` | `field_fixed` | A `:145`, D `:230` | a2 |
| `economy_score.c:43` | `field_qe_map` | D `:233`, `:272` | a1 |
| `economy_score.c:51` | `field_list` | A `:118`, D `:231` | a2 |
| `economy_tiers.c:41`, `:49` | `field_list`, `field_ref_instance` | A seulement | a2 |
| `probe.c:146` | `pico8_probe_field_vec` | A `:179-180`, D `:285` | a2 |
| `probe.c:154` | `pico8_probe_field_instance` | A `:177-178` | a2 |
| `score_panel.c:37`, `:45`, `:53` | `fm_field_fixed`, `fm_field_value`, `fm_field_qe_map` | D | a1 |
| `score_panel_display.c:33` | `fm_field_value` | D | a1 |
| `slipway.c:133` | `pico8_jc_entity_value` (et ses enveloppes non comptées `pico8_jc_entity_instance`, `pico8_jc_entity_vec`, `pico8_jc_value_vec`) | A, G | a2 |
| `slipway_display.c:16` | `pico8_jc_display_entity_value` (et `_entity_vec`, `_value_vec`) | A | a2 |
| `ui.c:57` | `pico8_ui_field_vec` | D | a1 |
| `wormhole.c:64` | `pico8_mz_field_vec` | A `:215` | a2 |

**Aides H qui restent** (un appelant B, C, F ou M) :
- `economy_score.c:33`/`35` `field_truthy` (B `qh`, `:83`) ;
- `economy_tiers.c:31`/`33` `field_truthy` (B `nl`, `:293`) ;
- `entity_geom.c:18`, `:26` (C : `fa`, `ra`, `ma`, et `p` de `ij`, voir plus bas) ;
- `msgbox.c:49` (F : `gq`, `ra` de `pz`) ;
- `planet_display.c:38`/`40`, `:48` (B : `qh`, `qw`) ;
- `probe.c:123`, `:129`/`131` (M) ;
- `ui.c:65` `pico8_ui_field_raw` (C : `ql`, `fg`) ;
- `wormhole_display.c:21` (C, plus bas).

**Commentaires de contrat devenus faux** (a1 ; lignes comptées qui partent ou sont réécrites) :
- `entity.h:62` (« Either is read and written through pico8_entity_get_field()/pico8_entity_set_field() », sur `mo` ; 1 ligne, 2 occurrences) ;
- `probe.h:108` (« read it through pico8_entity_get_field », sur `fm`) ;
- `ui.h:77` (« Read it only through `pico8_entity_get_field()` », sur `je`).

Chacun est ramené au contrat qui reste : l'accesseur typé, NULL sur le remplaçant. `entity.h:49` (commentaire de `pico8_bm_has`) et `research_icon.h:19` (classe K) ne changent pas.

**Reclassés hors de D.11a — receveur hétérogène (C, D.11b), découvert ici.** Trois sites que l'annexe A de D.9 classait A ont un receveur qui couvre deux branches de l'arbre. `mz.ox` est `jp(hv, fx)` : `pico8_g__field` répond `fx` (le `fx=1` de `g_`, `lua.lua:710`, hérité par `fv`). Il tient donc des planètes **et** des sondes et aides. Ces trois sites restent par nom, avec leurs aides : ce sont des champs transversaux (Q6, D.11b).
- `wormhole.c:78` (`pico8_mz_pq_ox_predicate`, R `fx` sur tout `hv`) ;
- `entity_geom.c:99` (`pico8_ij`, R `p`, appelé sur `cg("jo")`, `cg("np")` **et** `mz.ox`) ;
- `wormhole_display.c:41` (`pico8_mzd_ra_of_ij`, R `ra` sur `mz.ij` ∈ `mz.ox`).

Précédent : le §7 dit qu'un site couvert par une règle existante n'est pas un arrêt.

**Autres sites par nom des fichiers de l'empreinte, laissés** (classe, étape) :
- `bootstrap.c:94` `p_` (C) ;
- `economy_score.c:83` `qh`, `economy_tiers.c:293` `nl`, `planet_display.c:162` `px`, `:222` `qh`, `:458` `qw`, `:466` `s`, `planet_trade.c:167` `ba` (B) ;
- `planet_display.c:496` `dw` sur `jo.fo`, `slipway.c:254` W `fo` sur `jc.et`, `ui.c:295`/`336`/`349`/`353`/`357` (C) ;
- `msgbox.c:377`-`461` (F) ;
- `probe.c:211`-`419` (M) ;
- `entity.c:236`/`265`/`299`/`328` (C) ;
- `ke.c:43` (G, D.11c) ;
- les lecteurs, écrivains et `*_take_*` des descripteurs (D.11e).

**TU** : D.11a ne change la forme d'aucune lecture par nom de TU (les lecteurs `field` restent). Trois TU nomment un symbole dont le type change, et suivent dans le même commit (règle du pont, tranchée : aucun pont) :
- `test_pico8_ui` (a2) : `pico8_lt(&g_giver.header)` ×2, l. 930 et 999 ; le `giver_t` en en-tête seul (l. 803-805, 851), inscrit dans `hv` ;
- `test_pico8_entity_kind` (a2) : `pico8_ie_eh().as.ref` (l. 553), `pico8_ie_set_gy(pico8_value_ref(NULL))` (l. 595), et `hb_from_gy` (l. 707-711) avec ses quatre cas hors rangée (l. 751-754), devenus inexprimables ;
- `test_pico8_fm` (a1) : une assertion ajoutée, qui couvre `pico8_fm_hy()` NULL sur le remplaçant (règle 8 des tests).

## Solution

**Deux commits (chunks a1, a2), chacun vert seul**, sans pont :

1. **a1 — singletons (D, E).**
   - Accesseurs typés : `pico8_fm_hy()` (contrat Q8 : NULL sur l'emplacement vide **et** sur le remplaçant), `pico8_fm_placeholder()` (le remplaçant typé, pour `jk`/`nz`), `pico8_je_nf()`, `pico8_mo_qb()`, `pico8_nf_l()`/`pico8_nf_r()`.
   - Les 8 lignes comptées et les 22 appels d'aides de la classe D, et la ligne E, lisent et écrivent les membres par ces accesseurs.
   - Meurent les aides sans appelant : `fm_field_*` de `score_panel.c`/`score_panel_display.c`, `field_qe_map` d'`economy_score.c`, `pico8_ui_field_vec`.
   - Les trois commentaires de contrat qui disaient « lu par `pico8_entity_get_field` » (`entity.h`, `probe.h`, `ui.h`) sont réécrits.
   - Une assertion est ajoutée à `test_pico8_fm`.
2. **a2 — receveurs de type connu (A) et sites G.**
   - Les 40 lignes comptées et 38 appels d'aides de la classe A deviennent des accès aux membres typés, derrière un cast vérifié.
   - Extrémités de voie : `pico8_jc_end_p()` et `pico8_jc_end_ra()` (`slipway.c/.h`), qui couvrent la famille `jo` et le point de souris de `jc:hq`.
   - `ke` sur `hv` : `pico8_lt` prend `pico8_ke_t *`.
   - `jc.mt` est lu sur `fr` par un test de classe exact (`jc_take_fixed` part).
   - `ie.gy` devient `const pico8_vec_t *` (`ie_take_vec` part).
   - Meurent les aides restantes sans appelant.
   - Suivent `test_pico8_ui` (`giver_t` porte un `pico8_ke_t`) et `test_pico8_entity_kind` (`gy` typé, 4 cas `hb_from_gy` inexprimables retirés).
   - Enfin : note « Découvert à D.11a », `class-coverage-net`, balayage ASan+UBSan des TU.

**Sortie de l'étape**, constatée par commande sur le dernier commit :

- `make test` vert (`make data-guard` inchangé : `résidu gardé (spec,overrides,stash,ob,dispatch) : 0 ligne(s)`) ;
- `make e2e` : **26 conformes** ;
- balayage ASan+UBSan des TU propre ;
- `make class-coverage-net` identique à `tools/class_coverage_net_expected.txt` ;
- `make data-residue` : `par-nom` `src/` = **départ − 76** (≈ 342 si le départ est 418, H-D10b-5), `tests/` rapporté, le détail par fichier égal à la table « Effet sur `make data-residue` » ; `valeur`, `type-tag`, `ref-audit` rapportés ;
- aucun `*.sha256`, aucun fichier de `tests/e2e/` ni de `tools/` touché, le `Makefile` inchangé, aucune constante `*_HASH`/`identity[]` changée ;
- ces commandes rendent exactement ce qui est écrit (`src/` seulement) :

  ```sh
  # 1. aides par nom mortes : vide
  grep -nE '\b(field_kw|field_vec|field_qe_map|field_ref_instance|fm_field_[a-z_]+|pico8_probe_field_(vec|instance)|pico8_ui_field_vec|pico8_mz_field_vec|pico8_jc_entity_(value|instance|vec)|pico8_jc_value_vec|pico8_jc_display_(entity_value|entity_vec|value_vec)|jc_take_fixed|ie_take_vec)\b' src/*.[ch]
  grep -nE '\bfield_(list|fixed)\(' src/pico8_economy_formulas.c src/pico8_economy_score.c src/pico8_economy_tiers.c
  # 2. plus aucune lecture par nom des noms des classes A, D et E hors des descripteurs :
  #    chaque ligne rendue est un `strcmp(name, "…")` d'un lecteur ou écrivain `field`/`set_field`
  grep -nE '"(_cr_t|_jo_fe_pt|_jo_fe_b|_jo_fe_od|en|trade|kr|hg|lp|rl|oe|jh|hn|hs|happiness|invented|ew|fu|ge|lo|yr|hl|jk|nz|mt|o_|qu|re|l|r)"' src/*.c | grep -v 'strcmp(name, "'
  #    attendu : les seules lignes restantes sont des chaînes d'affichage ou de données sans rapport
  #    (listées au message, une par une), aucune lecture par nom
  # 3. singletons : plus aucun appel de lecteur par nom sur pico8_fm()/pico8_je()/pico8_mo()/g_je
  grep -nE 'pico8_entity_(get|set)_field\((pico8_fm\(\)|pico8_je\(\)|pico8_mo\(\)|g_je|fm|mo)\b' src/*.c
  grep -nE '_(field_[a-z]+|field)\((pico8_fm\(\)|pico8_je\(\)|pico8_mo\(\)|g_je|fm)\b' src/*.c
  # 4. porte de hv typée
  grep -n 'pico8_lt(' src/*.[ch] tests/*.c
  # 5. comptes
  make -s data-residue; sh tools/data_residue.sh --detail | grep -E '^par-nom'
  ```

## User Stories

1. En tant que mainteneur du port, je veux que chaque lecture d'un champ sur un receveur dont le type est connu soit un accès au membre typé, afin que le compilateur vérifie le nom, le type et la présence que lit le code.
2. En tant que mainteneur du port, je veux que chaque cast d'un en-tête vers une struct soit précédé, dans la même fonction, d'un test de famille ou de classe (`pico8_jo_is_family`, classe exacte `jc`/`trade`/`fr`/`qb`), afin qu'un receveur inattendu arrête le jeu au lieu de lire la mauvaise mise en page.
3. En tant que mainteneur du port, je veux que `self` des rappels de bouton (`_cr_t`, `lq`, `_jo_fe_*`) soit lu comme le `pico8_js_t` qu'il est depuis D.10b, afin que ces trois sites n'aient plus de lecteur par nom.
4. En tant que mainteneur du port, je veux que `fm`, `je` et `mo` se lisent par `pico8_fm_hy()`, `pico8_je_nf()`, `pico8_mo_qb()`, NULL sur le remplaçant, afin que la décision Q8 soit appliquée et que la valeur par défaut d'un remplaçant soit celle d'aujourd'hui (vecteur nul, absent, 0).
5. En tant que mainteneur du port, je veux que `jk`/`nz` du `fm` provisoire se lisent sur `pico8_fm_placeholder_t`, afin que le code de triche tapé sur l'écran titre reste visible là où il l'est aujourd'hui.
6. En tant que mainteneur du port, je veux qu'une écriture sur le `mo` provisoire arrête encore le jeu, afin que la garde d'aujourd'hui (le remplaçant n'a pas d'écrivain) survive au typage.
7. En tant que mainteneur du port, je veux que `je[bt]` passe par `pico8_nf_l()`/`pico8_nf_r()`, afin que l'état des boutons de souris soit lu typé, NULL hors init comme aujourd'hui.
8. En tant que mainteneur du port, je veux que `p` et `ra` d'une extrémité de voie se lisent par un accesseur qui connaît les deux formes que le cart y met (planète, point de souris), afin que les quatre modules qui les lisent (`economy_formulas`, `probe`, `slipway`, `slipway_display`) n'aient qu'une règle.
9. En tant que mainteneur du port, je veux que `jc.mt` soit lu sur la seule classe qui le porte (`fr`), afin que le déballage `jc_take_fixed` (site G) disparaisse.
10. En tant que mainteneur du port, je veux que `ie.gy` soit un pointeur de vecteur typé, afin que le déballage `ie_take_vec` (site G) disparaisse et que `pico8_ie_eh()`/`pico8_ie_set_gy()` quittent la liste des prototypes à `pico8_value_t` (Q12).
11. En tant que mainteneur du port, je veux que `hv` n'admette que des `pico8_ke_t` à sa porte (`pico8_lt`), afin que lire `me` et `t` sur une entité de `hv` soit un cast vérifié par le compilateur.
12. En tant que mainteneur du port, je veux que chaque aide par nom sans appelant soit supprimée dans le chunk où part son dernier appelant, afin qu'aucune aide morte ne reste pour D.11e.
13. En tant que mainteneur du port, je veux que les aides qui gardent un appelant B, C, F ou M restent intactes, afin que D.11b-D.11c trouvent leurs sites tels que l'étude les décrit.
14. En tant que gardien du compteur, je veux `par-nom` `src/` égal au départ moins 76, fichier par fichier comme la table de ce PRD, afin que chaque baisse soit un site retiré.
15. En tant que gardien du compteur, je veux, au message de chaque commit, pour chaque catégorie et chaque fichier : les comptes avant/après en lignes et en occurrences ; la classification (a)/(b)/(c) des lignes `par-nom` retirées ; les lignes qui gagnent une catégorie (« aucune » compris) ; la justification de chaque Δlignes > Δoccurrences. Afin qu'aucune baisse ne soit un tassement.
16. En tant que gardien du compteur, je veux qu'aucune ligne ne soit jointe, et qu'aucun alias, registre, renommage pour les greps, cast non vérifié ni cale valeur ↔ typé ne soit ajouté, afin que la baisse veuille dire « retiré ».
17. En tant que gardien des TU, je veux que les trois TU qui nomment un symbole retypé suivent dans le même commit, même intention, afin qu'aucun pont ne survive.
18. En tant que gardien des TU, je veux que chaque cas de test rendu inexprimable soit retiré et listé au message avec sa raison, afin qu'aucune couverture ne disparaisse sans trace.
19. En tant que gardien des constantes, je veux qu'aucune constante `*_HASH`/`identity[]` ne change, afin que l'identité de l'état reste prouvée.
20. En tant que gardien des filets, je veux `make test` et `make e2e` verts à chaque commit, `class-coverage-net` identique à a2 et le balayage ASan+UBSan des TU propre, afin de prouver qu'aucun comportement n'a changé.
21. En tant que `plan-pilot`, je veux que les sites de l'annexe A de D.9 dont le receveur couvre deux branches (`mz.ox`) soient nommés dans la note et laissés à D.11b, afin que D.11b ne les croie pas faits.
22. En tant que `plan-pilot`, je veux une note « Découvert à D.11a » qui dise les accesseurs nouveaux, la porte typée de `hv`, les reclassements et les aides qui restent, afin que D.11b-D.11e partent d'un état exact.
23. En tant que `plan-pilot`, je veux les inconnues d'exécution écrites en hypothèses H1…Hn, et celles sur D.10b en `H-D10b-n`, afin que le vérificateur et le pilote sachent quoi confirmer.

## Implementation Decisions

Pas d'humain dans la boucle. Le skill `to-prd` demande de faire valider par l'utilisateur les modules, les tests et le découpage. Ces choix sont tranchés ici comme le ferait un agent `guideline` (hypothèses explicites, simplicité, changement chirurgical, critères vérifiables) et marqués **(G)**. Les rejets sont notés à chaque décision. Q1-Q11 sont des décisions humaines (plan, « Décisions de D.9 ») : ce PRD les applique ; Q12 reste ouverte et n'est pas tranchée ici.

### Modules

- **`probe`** (`fm`) : `pico8_fm_hy()` prend le contrat Q8, et `pico8_fm_placeholder()` est ajouté.
- **`ui`** (`je`) : `pico8_je_nf()`.
- **`entity`** (`mo`, porte de `hv`) : `pico8_mo_qb()`, et `pico8_lt(pico8_ke_t *)`.
- **`mouse_reader`** : `pico8_nf_l()`, `pico8_nf_r()`.
- **`slipway`** : `pico8_jc_end_p()`, `pico8_jc_end_ra()`.
- **`research_icon`** : `ie.gy` typé.
- Modules consommateurs, sites seulement : `bootstrap`, `economy`, `economy_formulas`, `economy_score`, `economy_tiers`, `entity_geom`, `galaxy`, `menu`, `msgbox`, `planet_display`, `planet_family`, `planet_family_trade`, `planet_trade`, `research_display`, `score_panel`, `score_panel_display`, `slipway_display`, `tech_gate`, `wormhole`, `bg`, `ke`.
- **Aucun module nouveau.** Chaque accesseur vit dans le module qui possède la donnée. Tous les TU qui lient un de ces modules lient `$(ENGINE_SRCS)`, qui les contient tous (H2) : le `Makefile` ne change pas.

### Singletons (G, a1)

Forme retenue (fragment de décision, pas un code à recopier) :

```c
/* pico8_probe.h */
/* The `hy` of `fm`: NULL on the empty slot and on the provisional. */
pico8_hy_t *pico8_fm_hy(void);
/* The provisional `fm` (jk/nz), NULL once `fm` is a `hy` or the slot is empty. */
const pico8_fm_placeholder_t *pico8_fm_placeholder(void);

/* pico8_ui.h (inclut pico8_mouse_reader.h) */
pico8_nf_t *pico8_je_nf(void);      /* NULL: no je, or the field-less placeholder */
/* pico8_entity.h (inclut pico8_scroll.h) */
pico8_qb_t *pico8_mo_qb(void);      /* NULL: no mo, or the placeholder */
/* pico8_mouse_reader.h */
const pico8_nf_button_t *pico8_nf_l(void);   /* NULL outside pico8_nf_init()..shutdown() */
const pico8_nf_button_t *pico8_nf_r(void);
```

- **Chaque accesseur asserte** que l'occupant est soit le remplaçant, soit la classe attendue (`hy`, `nf`, `qb`). Le cast qu'il rend est donc vérifié. Le test de classe se fait dans le module qui possède la donnée, comme `cheat.c` le fait déjà pour `fm`.
- **`pico8_fm_hy()` et Q8.** Q8 est une décision humaine, et la ligne nomme `pico8_fm_hy()` : son assert sur le remplaçant part, et il rend NULL.
  - Conséquence : ses dix appelants de méthode (`cart.c:200-201`, `cheat.c:36`, `menu.c:113`, `:298`, `planet.c:788`, `planet_display.c:347`, `probe.c:343`, `:372`, `slipway.c:301`) reçoivent NULL là où ils auraient arrêté le jeu sur le remplaçant. Les méthodes de `hy` prennent déjà NULL pour « pas de `fm` » (`assert_hy` admet NULL, `economy.c:129-133`). `cart.c:200` et `cheat.c:28-35` gardent leurs propres tests.
  - Aucun TU ni e2e n'atteint cet assert : aucun test de mort ne le vise (grep `pico8_fm_hy` dans `tests/` : `test_pico8_fm:132,134` sur l'emplacement vide, `test_pico8_fr:340` sur un `hy`).
  - La garde perdue est déclarée au message et dans la note, comme décision Q8.
  - Rejetés :
    - (i) garder l'assert et ajouter un second accesseur pour les lectures (Q8 nomme `pico8_fm_hy()` pour les lectures, et deux accesseurs au même occupant se distingueraient par un détail de contrat) ;
    - (ii) ajouter l'assert aux dix appelants de méthode (dix lignes hors périmètre).
- **Valeurs par défaut** : un remplaçant lit ce qu'il lit aujourd'hui (vecteur nul, absent, 0, `"current score"`). Les lectures faites sur un `je`/`mo` NULL ou sur un remplaçant rendent la même chose qu'avant.
- **Écritures sur `mo`** (`bg:hq` `v`, `ga:pq` `ma`) : `mo == NULL` saute l'écriture, comme aujourd'hui. Sur le remplaçant, l'écriture par nom arrêtait le jeu (le remplaçant n'a pas de `set_field`, `entity.c:215`). Elle arrête encore : `assert(qb != NULL && "mo: the placeholder has no writer; a write before jl() stops the run")`. C'est un piège qu'on réintroduirait, donc un assert porteur de message (`CLAUDE.md`).
- **`jk`/`nz`** : lus sur `hy` s'il y en a un, sinon sur `pico8_fm_placeholder()` avec `has_jk`/`has_nz` ; présence et valeur sont celles d'aujourd'hui. `cheat.c` n'est pas touché (il écrit, il ne lit pas par nom).
- **`je[bt]`** : `bt` reste une chaîne ∈ {`l`, `r`}, assertée, et le choix se fait par `strcmp`, comme aujourd'hui. Ce n'est pas un registre : deux accesseurs nommés, un test.
- **Rejetés** : lire les singletons par `pico8_fm()`/`pico8_je()`/`pico8_mo()` suivi d'un cast au site (onze sites répéteraient le test de classe) ; monter `nf.l`/`nf.r` dans `pico8_nf_t` (D.7a les a faits statiques non `const` ; changer la mise en page déborde).

### Extrémités de voie (G, a2)

- `jc.qu` est une planète. Le constructeur l'asserte déjà : `jc_is_endpoint_class`, `slipway.c:542,625`.
- `jc.re` est une planète ou le point de souris `{ra,p}` que `jc:hq` accroche au-dessus du vide (`pico8_jc_mouse_point_t`, vue statique de `slipway.c`). `re` peut aussi être NULL avant le premier `hq`.
- `cg("jc")` inscrit le `jc` dès sa construction : `bv` et `jc:ce` voient donc aussi le `jc` en cours de tracé.
- Forme retenue : `pico8_vec_t pico8_jc_end_p(const pico8_struct_header_t *end)` et `pico8_jc_end_ra(...)`, dans `slipway.c/.h` :
  - NULL → vecteur nul (le repli d'aujourd'hui) ;
  - `pico8_jo_is_family(end)` → `*jo->p` (NULL → nul) ;
  - `end->cls == &g_jc_mouse_point_struct_class` → `*point->p` ;
  - sinon `assert`.
- Appelants : `pico8_dj` (`economy_formulas.c`), `pico8_bv_jc_predicate` (`probe.c`), `jc:ce` et ses deux prédicats (`slipway.c`), `jc:ch`/`jc:lg` (`slipway_display.c`).
- Rejetés :
  - changer la signature de `pico8_dj` pour des vecteurs (portage fonction par fonction : `dj(pb,to)` reste) ;
  - un cast vers `jo` seul (faux pour le point de souris, qui est une forme légitime du cart) ;
  - un accesseur par champ dans la table de classe (c'est la forme C de Q6, pour un champ lu sur n'importe quelle entité ; ici l'ensemble est fermé et fixé par le cart).

### `ke` sur `hv` (G, a2)

- `dg` lit `me` et `t` sur chaque entité de `hv`. Toutes sont des `pico8_ke_t` hors TU : `pico8_lt` n'a qu'un appelant dans `src/`, `pico8_ke_struct_pq(pico8_ke_t *)`, et `ke.h` l'écrit. Mais aucun test d'exécution ne peut vérifier qu'un en-tête est un `ke`. Or `test_pico8_ui` inscrit dans `hv` un `giver_t` fait d'un en-tête seul (l. 803-805, 930, 999) : lu comme `pico8_ke_t`, il serait lu hors de sa taille.
- Retenu : **`pico8_lt(pico8_ke_t *e)`**. La porte de `hv` n'admet que des `ke`, par le compilateur, et le cast du retour de `hv` en est l'inverse. `pico8_ke_struct_pq` passe `self` sans cast. `test_pico8_ui` donne à `giver_t` un `pico8_ke_t` en premier membre, initialisé statiquement à `me` NULL et `t` 0 (même intention : un donneur inscrit dans `hv`). `lt_a_table` (test de mort, `void *`) compile tel quel et garde son objet.
- `pico8_he` et le type des éléments de `hv` ne changent pas (D.12a).
- Rejetés :
  - lire `me`/`t` par un cast non vérifié (interdit) ;
  - un marqueur « est un `ke` » dans `pico8_struct_class_t` (membre nouveau pour une seule lecture ; le lien parent de Q5 est à D.11b) ;
  - laisser `me`/`t` de `dg` par nom (ce sont les sites A que l'étude nomme).
- Les autres lectures de `me`/`t` de D.11a portent sur des receveurs de famille connue : `dh` et `mz:pr` → `jo`, `ma` → `jo` ou `qb`. Elles passent par le cast de cette famille, pas par `hv`.

### `jc.mt` (G, a2)

`mt` n'est répondu que par `pico8_fr_field` (défaut propre de `fr`, `planet_family_trade.h:29-37`) et par le `jc` lui-même. `fr` n'a pas de fille, donc le test `e->cls == pico8_fr_struct_class()` est exact : ce n'est pas le test d'appartenance à une lignée de Q5 (D.11b).
- `jc:hq` : `qu` d'abord ; `re` seulement si `qu` n'a pas de `mt` (un `mt` présent est un fixe, vrai en Lua).
- `jc_ce_jc_predicate` compare `(has_mt, mt)` du `jc` voisin à ceux de `self`, que le contexte porte désormais typés. `pico8_value_equal` disparaît de ce site : deux absents sont égaux, deux fixes le sont par valeur, un présent et un absent ne le sont pas.

### `ie.gy` (G, a2)

- `static const pico8_vec_t *g_ie_gy` (NULL = nil). `ie:hb` fait `self->ra = g_ie_gy`, puis rebinde `g_ie_gy` sur un vecteur neuf. Même fuite qu'aujourd'hui (TROU-7).
- `pico8_ie_eh()` rend `&pico8_ie_start.eh` ou NULL hors init ; `pico8_ie_set_gy(const pico8_vec_t *)`.
- Le lecteur `"gy"` de `pico8_ie_field` rend absent sur NULL, sinon le REF : c'est la couche que D.11e supprime. Ses lignes gagnent `valeur`, déclaré.
- L'assert de `ie_take_vec` (« a gy neither nil nor a vector ref ») devient le type.

### Consommateurs à valeur (G)

Trois fonctions qui reçoivent ou rendent un `pico8_value_t` sont dans le périmètre de Q12 et **ne sont pas retypées** : `pico8_jc_hg_add`/`pico8_jc_hg_has` (`id`), `pico8_trade_qe`, `pico8_jo_hf` (TROU-3).
- Le site A lit le membre typé et passe à ces fonctions la valeur que le lecteur par nom leur passait, par le constructeur existant (`pico8_value_string(jo->id)`), à l'unique appel.
- Ce n'est pas une cale : aucune fonction de conversion n'est ajoutée, et la valeur n'existe que pour une API à valeur qui préexiste. Chaque ligne qui gagne `valeur` pour cela est listée au message.
- Rejeté : retyper `jc.hg` (`test_pico8_jc` l. 871-899 teste des `id` non chaîne : ce serait une étape `valeur`).
- Même règle pour `ou[2]`/`ou[6]` de `pj:hb` : les items de rangée sont des `pico8_value_t`, destination inchangée.

### Anti-tassement (liant)

Le compteur compte des **lignes**. Une baisse obtenue sans retirer la lecture par nom est un échec.

- **Interdit** :
  - joindre sur une ligne ce qui était sur plusieurs ;
  - renommer un identifiant, un paramètre ou un commentaire compté dont la donnée existe toujours ;
  - macro, `typedef`, constante, `enum`, pointeur ou fonction enveloppe qui renomme un identifiant vivant, ou un alias ;
  - un registre nouveau (tableau, `switch` ou boucle qui passe d'un nom à un membre) ;
  - un cast d'en-tête sans test de famille ou de classe dans la même fonction ;
  - une cale valeur ↔ typé : fonction de conversion, ou lecture par nom déplacée dans une aide non comptée ;
  - déplacer du code compté hors de `src`/`tests` ou hors `.c`/`.h` ;
  - supprimer un commentaire qui ne porte pas sur une construction retirée ou un contrat devenu faux ;
  - tout changement d'une constante de hachage.
- **Classification exigée** ⟲, au message de chaque commit, pour chaque ligne `par-nom` retirée. Recette : `git diff -U0` par fichier, lignes `-` qui portent le motif, en regard des lignes `+`.
  - **(a1) site** : lecture ou écriture par nom remplacée par l'accès au membre typé ;
  - **(a2) construction supprimée** : définition d'aide sans appelant, aide `*_take_*` sans appelant, commentaire d'un contrat devenu faux ;
  - **(b)** : signature, déclaration ou variable dont le type change, attendu 0 pour `par-nom` ;
  - **(c)** : même donnée, autre nom, **attendu 0**.
- **Attendu, par chunk (`src/`, lignes)** :
  - a1, 18 lignes :
    - (a1) : `bg.c:272`, `planet_family.c:750`, `menu.c:206`, `:209`, `tech_gate.c:35`, `:57`, `:95`, `:102`, `ui.c:81` ;
    - (a2) : `economy_score.c:43`, `score_panel.c:37`, `:45`, `:53`, `score_panel_display.c:33`, `ui.c:57`, `entity.h:62`, `probe.h:108`, `ui.h:77`.
  - a2, 58 lignes :
    - (a1), 42 lignes :
      - `bootstrap.c:80`, `:87`, `:98`, `economy.c:304`, `economy_formulas.c:113` ;
      - `economy_score.c:90`, `:122`, `:143`, `:152`, `:159`, `:173`, `:183`, `:209` ;
      - `economy_tiers.c:79`, `:86`, `:100`, `:142`, `:147`, `:159`, `:183`, `:193`, `:200`, `:220`, `:223`, `:248`, `:278`, `:311` ;
      - `galaxy.c:99`, `menu.c:112`, `:347`, `planet_display.c:307`, `:335`, `:339`, `:342` ;
      - `planet_family_trade.c:38`, `:342`, `planet_trade.c:74`, `research_display.c:327`, `score_panel_display.c:230`, `wormhole.c:213` ;
      - `slipway.c:248`, `research_icon.c:180` (les deux sites G) ;
    - (a2), 16 lignes :
      - `economy_formulas.c:20`, `:22`, `:30`, `:38`, `:46`, `economy_score.c:27`, `:51`, `economy_tiers.c:41`, `:49` ;
      - `probe.c:146`, `:154`, `slipway.c:133`, `:161`, `slipway_display.c:16`, `research_icon.c:138`, `wormhole.c:64`.
  - `tests/` : lignes `par-nom` retirées, rapportées et classées de même. Attendu 0 ; au plus les lignes des cas `hb_from_gy` retirés, s'il y en a de comptées.
- **Mesure exigée**, avant et après chaque commit, par catégorie : les **lignes** (`make data-residue`, `--detail` par fichier) **et les occurrences** (recette figée de D.8, `$S` : dossier du scratchpad) :

  ```sh
  sed -n '/^pattern() {/,/^}/p' tools/data_residue.sh > "$S/pat.sh"; . "$S/pat.sh"
  F=$(find src tests \( -name '*.c' -o -name '*.h' \) -type f | sort)
  for c in valeur type-tag ref-audit par-nom dispatch; do
    printf '%s\t%s\n' $c "$(cat $F | grep -oE "$(pattern $c)" | wc -l)"; done
  ```

  Mesuré sur `ba4768d` : `valeur` 4643, `type-tag` 551, `ref-audit` 56, `par-nom` 1103 (`src/` 480), `dispatch` 46. Le détail par fichier (lignes et occurrences) de chaque fichier du diff est donné avant et après.
- **Contrôle Δlignes/Δoccurrences** : pour chaque (catégorie, fichier) du diff, donner Δlignes et Δoccurrences. Si Δlignes > Δoccurrences en valeur absolue d'une baisse, le message justifie le couple. Attendu : `par-nom` `entity.h` −1 ligne / −2 occurrences (a1 : la ligne 62 porte deux noms, Δocc > Δlignes) ; ailleurs Δlignes = Δocc.
- **Lignes qui gagnent une catégorie**, listées par fichier et par catégorie, « aucune » compris. Attendues :
  - `valeur` : `research_icon.c` (lecteur `"gy"`, qui emballe désormais le pointeur) ;
  - `economy_tiers.c` et `slipway_display.c` (`pico8_value_string(jo->id)` vers `pico8_jc_hg_add`/`_has`) ;
  - `score_panel.c` (`fm_jk`/`fm_yr` construits du membre) ;
  - `tech_gate.c`, `economy_score.c` si une ligne reconstruit une valeur pour une API à valeur ;
  - chacune déclarée ; aucune autre catégorie.

### Empreinte remesurée et découpage (G) ⟲

| Chunk | `src/` | `tests/` | autres | `par-nom` retirées |
|---|---|---|---|---|
| a1 | 17 : `probe.c/.h`, `ui.c/.h`, `mouse_reader.c/.h`, `entity.c/.h`, `bg.c`, `planet_family.c`, `menu.c`, `tech_gate.c`, `economy_score.c`, `score_panel.c`, `score_panel_display.c`, `entity_geom.c`, `msgbox.c` (+ `scroll.h` si H1 tombe) | 1 : `test_pico8_fm` | — | 18 |
| a2 | 23 : `bootstrap.c`, `economy.c`, `economy_formulas.c`, `economy_score.c`, `economy_tiers.c`, `entity.c/.h`, `ke.c`, `entity_geom.c`, `galaxy.c`, `menu.c`, `planet_display.c`, `planet_family_trade.c`, `planet_trade.c`, `probe.c`, `research_display.c`, `research_icon.c/.h`, `score_panel_display.c`, `slipway.c/.h`, `slipway_display.c`, `wormhole.c` | 2 : `test_pico8_ui`, `test_pico8_entity_kind` | `docs/plan-3.md` (note) | 58 |

- a1 : 18 chemins, a2 : 26 : sous les ~40 de la règle. En tout, 33 fichiers distincts de `src/` et 3 TU.
- **Pourquoi deux chunks** : a1 introduit les accesseurs de singletons et change un contrat public (`pico8_fm_hy`). a2 est la masse des sites de receveur connu, plus deux retypages de porte (`pico8_lt`, `ie.gy`). Chaque chunk a ses mutations et sa preuve. Un seul commit (≈ 45 chemins) dépasserait la règle.
- **Pourquoi a1 avant a2** : `economy_score.c` perd `field_qe_map` en a1, puis `field_fixed`/`field_list` en a2, quand leurs derniers appelants A partent. Dans l'autre ordre, a2 devrait laisser ces aides vivantes pour les seuls sites D, pour rien.
- Un fichier de `src/` peut être touché par les deux chunks (`economy_score.c`, `menu.c`, `probe.c`, `entity.c/.h`, `entity_geom.c`, `score_panel_display.c`), mais un TU par un seul.
- Rejetés :
  - un chunk par classe de site A (dix commits qui touchent les mêmes fichiers) ;
  - les G seuls (deux sites : le retypage de `ie.gy` et le test `fr` n'ont de sens qu'avec leurs voisins A, `pico8_jc_hq` et `pico8_ie_hb`).

### Effet sur `make data-residue` ⟲

| Catégorie | Départ (`ba4768d`, puis −1 par D.10b) | Après a1 | Après a2 | Pourquoi |
|---|---|---|---|---|
| `par-nom` `src/` | 418 (H-D10b-5) | **400** | **342** | exact : 18 puis 58 lignes, listées plus haut |
| `par-nom` `tests/` | 568 − k (H-D10b-6) | = | = (± cas `hb_from_gy` comptés) | aucune lecture par nom de TU ne change de forme |
| `valeur` | rapporté par D.10b | baisse, rapportée | baisse, rapportée | aides à `pico8_value_t v` supprimées, `pico8_value_equal` de `ce_jc`, `ie.gy`, `pico8_ie_eh`/`_set_gy` ; hausses déclarées plus haut |
| `type-tag` | 347 / 551 (sauf D.10b) | = ou baisse, rapportée | baisse, rapportée | assert de `ie_take_vec` (`PICO8_TYPE_TAG_UNKNOWN`, `pico8_type_tag_of`) |
| `ref-audit` | 55 / 56 | baisse, rapportée | baisse, rapportée | `PICO8_REF_AS_LIST_CONST` des aides `field_list` supprimées (`economy_formulas.c` 2, `economy_score.c`, `economy_tiers.c`), `PICO8_REF_AS_LIST` des lectures de `cc`/`jy` devenues typées |
| `dispatch` | 0 (H-D10b-2) | 0 | 0 | — |

`par-nom` par fichier de `src/` après a2 (départ `ba4768d` → arrivée) :
- `bg.c` 8→7, `bootstrap.c` 4→1, `economy.c` 3→2, `economy_formulas.c` 6→0 ;
- `economy_score.c` 14→3, `economy_tiers.c` 19→3, `entity.h` 4→3, `galaxy.c` 1→0 ;
- `menu.c` 6→2, `planet_display.c` 11→7, `planet_family.c` 59→58, `planet_family_trade.c` 33→31 ;
- `planet_trade.c` 2→1, `probe.c` 42→40, `probe.h` 1→0, `research_display.c` 1→0, `research_icon.c` 19→17 ;
- `score_panel.c` 8→5, `score_panel_display.c` 2→0, `slipway.c` 21→18, `slipway_display.c` 1→0 ;
- `tech_gate.c` 4→0, `ui.c` 11→9, `ui.h` 1→0, `wormhole.c` 17→15.

Tous les autres sont inchangés (`entity.c` 12, `entity_geom.c` 5, `msgbox.c` 12, `ke.c` 3, `mouse_reader.c` 3, `wormhole_display.c` 1…). « exact » est une égalité, sous réserve des numéros de ligne ⟲. « rapporté » est mesuré au commit, écart expliqué.

### Documentation (G)

- `docs/plan-3.md` (a2) : note « Découvert à D.11a » en fin de ligne D.11a (texte en « Further Notes »), rien d'autre.
- `docs/glossaire-noms.md` : inchangé ; aucun nom minifié ne change de sens. Les accesseurs nouveaux portent des noms composés de noms déjà glosés (`fm`, `hy`, `je`, `nf`, `mo`, `qb`, `l`, `r`).
- `docs/trous-de-portage.md` : inchangé. TROU-3 (`hf`, `qe`) et TROU-7 restent à leurs sites. Aucun écart port/cart n'est créé : les lectures typées rendent la même présence et la même valeur que les lectures par nom. La garde de `pico8_fm_hy()` sur le remplaçant n'est pas un écart observable : aucun rejeu ne l'atteint, et le cart y lèverait une erreur, pas un autre rendu. Décision Q8, déclarée.
- Les PRD livrés, `docs/plan.md` et `docs/plan-2.md` sont historiques et ne sont pas touchés.

### Ce qui ne change pas

- `tools/`, le `Makefile`, `tests/e2e/`, tout `*.sha256`, tout fichier d'événements ; aucun e2e ajouté.
- Les lecteurs `field`, les écrivains `set_field`, les aides `*_take_*` des écrivains, les `kind_field` et `pico8_entity_get_field`/`set_field` eux-mêmes (D.11e).
- Les aides H qui gardent un appelant B/C/F/M, et les sites B, C, F, K, M, dont les trois reclassés.
- Les signatures de `pico8_dj`, `pico8_bj`, `pico8_db`, `pico8_da`, `pico8_dh`, `pico8_ma`, `pico8_ij` ; `pico8_fm()`, `pico8_je()`, `pico8_mo()` et leurs setters ; `cheat.c` ; `pico8_he`.
- L'ordre des appels et des tirages aléatoires (`jc:ch` tire avant le test `hg`), les fuites (TROU-7), les arrêts d'aujourd'hui (écriture sur le `mo` provisoire, `je[bt]` hors `l`/`r`, clés de construction de `jc`).

### Hypothèses d'exécution (rien n'a été compilé ni lancé)

- **H1** : `pico8_ui.h` peut inclure `pico8_mouse_reader.h`, et `pico8_entity.h` peut inclure `pico8_scroll.h`, sans cycle. Constaté par grep à `ba4768d` : `scroll.h` et `mouse_reader.h` n'incluent ni `entity.h` ni `ui.h`. Sinon, la déclaration va dans l'en-tête du type (`scroll.h`, `mouse_reader.h`) et la définition reste dans le module du singleton ; déclaré.
- **H2** : tout TU qui lie `ui.c`, `entity.c`, `probe.c`, `slipway.c` ou `mouse_reader.c` lie `$(ENGINE_SRCS)` (les TU d'entités, dont `test_pico8_ui` l. 519 du `Makefile`) : aucun symbole nouveau ne manque à l'édition de liens. `DATA_ENGINE_SRCS` (`test_pico8_data_init`, `test_pico8_map_tables`) ne contient aucun des fichiers touchés, ou les contient avec leurs dépendances ; sinon, arrêt et PRD à reprendre, **le `Makefile` ne change pas**.
- **H3** : aucun TU n'appelle `pico8_dg` pendant que `g_giver` (`test_pico8_ui`) est dans `hv` avec un état que la lecture typée changerait. `me` NULL et `t` 0 rendent « pas d'état » et un `t` incrémenté que rien ne lit ; les constantes de `test_pico8_ui` sont inchangées.
- **H4** : `mz.ox` contient des `g_`/`fv` (`fx` fixe 1) dans au moins un e2e, ce qui confirme le reclassement. Sinon, le reclassement tient quand même : le receveur **peut** l'être.
- **H5** : `cg("qh")` ne contient que des entités de la famille `jo` (étiquettes `nu` de `qh` seulement) ; `cg("np")` et `cg("jo")` aussi (`jo`, `qh`, `nl`, `os`, `hj`, `pm`, `nr`, `mx`).
- **H6** : `pico8_jo_is_family` reconnaît `ga` (`LL` = `pico8_jo_ll_dispatch`, `planet_family.c:854`). Aucun `ga` n'est une extrémité de voie.
- **H7** : le `s` tenu que lisent `jo:gv`/`nr:gv` est un `jc`, un `bg`, un `mz` ou une planète (`test_pico8_bg:458,466`, `test_pico8_mz:688,695`). Le test de classe `jc` rend « `qu` absent » pour tous les autres, comme le lecteur par nom.
- **H8** : `pico8_nf_l()`/`pico8_nf_r()` rendent NULL hors de `pico8_nf_init`…`pico8_nf_shutdown`, comme la branche `"l"`/`"r"` de `pico8_nf_field` (`g_nf_up`).
- **H9** : l'`id` d'une planète est toujours présent quand `bd_route_visit` la lit. Sinon `continue`, comme aujourd'hui (présent ↔ `jo->id != NULL`, `jo_field_string`).
- **H10** : aucun comportement du binaire de rejeu ne change : 26 e2e conformes, `class-coverage-net` identique.
- **H11** : aucune constante `*_HASH`/`identity[]` ne change. Les cas `hb_from_gy` retirés s'exécutaient dans un fils (`must_abort_value`) et n'étaient pliés dans aucune constante.

## Implementation Chunks

Deux commits, dans cet ordre. Chacun est vert seul (`make test`, `make e2e`, `make data-guard`), avec ses comptes, sa classification, ses lignes gagnées, son `gcov` et ses mutations :

1. **a1 — singletons (D, E).**
   - Accesseurs : `pico8_fm_hy()` (Q8), `pico8_fm_placeholder()`, `pico8_je_nf()`, `pico8_mo_qb()`, `pico8_nf_l()`/`pico8_nf_r()`.
   - Sites D et E.
   - Aides mortes : `fm_field_*` ×4, `field_qe_map`, `pico8_ui_field_vec`.
   - Trois commentaires de contrat.
   - Assertion de `test_pico8_fm`.
   - 18 chemins, `par-nom` `src/` −18.
2. **a2 — receveurs connus (A) et sites G.**
   - `pico8_lt(pico8_ke_t *)` et `test_pico8_ui` (`giver_t`).
   - `pico8_jc_end_p()`/`pico8_jc_end_ra()`.
   - Sites A.
   - `jc.mt` sur `fr` (`jc_take_fixed` part).
   - `ie.gy` typé et `test_pico8_entity_kind` (`ie_take_vec` part, quatre cas `hb_from_gy` retirés).
   - Aides mortes.
   - Note « Découvert à D.11a ».
   - `class-coverage-net` ; balayage ASan+UBSan des TU.
   - 26 chemins, `par-nom` `src/` −58.

Ordre conseillé dans chaque commit :
1. mesure de départ (lignes et occurrences par fichier) ;
2. en-têtes, puis `.c` de `src/`, puis TU ;
3. `make test`, `make e2e` ;
4. `gcov` et mutations (copie jetable, restauration par `cp`) ;
5. mesure d'arrivée, classification, lignes gagnées, greps de la « Sortie » ;
6. docs, message.

## Testing Decisions

Un bon test juge ce que le code rend ou fait (valeurs, présences, identités, registres, effets d'un appel), pas la présence d'une ligne. D.11a change la **forme** des lectures, pas ce qu'elles lisent. La preuve :
- chaque TU garde son intention et sa constante ;
- les 26 e2e restent conformes ;
- chaque ligne nouvelle de `src/` est exécutée par un TU (`gcov`) et mord sous mutation.

### TU touchés (G)

1. **`test_pico8_fm` (a1)** : une assertion ajoutée. `pico8_fm_hy() == NULL` et `pico8_fm_placeholder() != NULL` quand `fm` est le provisoire ; `pico8_fm_placeholder() == NULL` quand `fm` est un `hy`. Elle couvre les branches nouvelles (règle 8).
2. **`test_pico8_ui` (a2)** : `giver_t` prend un `pico8_ke_t` en premier membre, initialisé statiquement (`me` NULL, `t` 0). Les deux `pico8_lt(&g_giver.header)` deviennent `pico8_lt(&g_giver.ke)`. Même intention, aucune assertion changée, aucune constante changée (H3).
3. **`test_pico8_entity_kind` (a2)** :
   - `pico8_ie_eh().as.ref` → `pico8_ie_eh()` (l. 553) ;
   - `pico8_ie_set_gy(pico8_value_ref(NULL))` → `pico8_ie_set_gy(NULL)` (l. 595) ;
   - `pico8_ie_set_gy(pico8_ie_eh())` (l. 625) inchangé ;
   - `hb_from_gy` (l. 707-711) et ses quatre cas (l. 751-754 : fixe 1, fixe 0, `false`, REF vers un `ie`) sont **retirés, listés au message**. Raison : `ie.gy` est un `const pico8_vec_t *`, le type interdit ces valeurs, et l'assert de `ie_take_vec` qu'ils prouvaient n'existe plus.
   - Les lectures `ie_ref(…, "gy")` restent (lecteur par nom, D.11d) et rendent le même REF.
4. **Pas de registre de TU**, pas de pont, aucun autre TU touché. Un TU qui casse sans nommer un symbole retypé est un **bug de D.11a**, pas un TU à adapter : il part à `help-me-debug`.
5. **Constantes** : aucune `*_HASH` ni `identity[]` ne change. Une constante qui rougit est un échec, jamais une mise à jour.
6. **§6** : aucun des trois tests n'est touché.
7. **Test ajouté** : aucun fichier nouveau. Une assertion nouvelle n'est admise que pour couvrir une ligne nouvelle de `src/` que `gcov` montre non exécutée, dans un TU existant de la classe (`test_pico8_je`, `test_pico8_mo`, `test_pico8_nf`, `test_pico8_jc`, `test_pico8_fm`…), listée au message avec la ligne qu'elle couvre. Candidates :
   - la branche remplaçant de `pico8_je_nf()`/`pico8_mo_qb()` ;
   - l'assert de l'écriture sur le `mo` provisoire (test de mort dans un fils, sur le modèle `must_abort`) ;
   - la branche point de souris de `pico8_jc_end_p`/`_ra`.
8. **Prior art** :
   - casts vérifiés : `call_gg_truthy`, `bd_route_visit`, `jo:kf`, `mx:kf`, `cheat.c` ;
   - accesseur de singleton : `pico8_fm_hy()` ;
   - tests de mort dans un fils : `must_abort`, `must_abort_value` ;
   - règles de TU : D.10a/D.10b.

### Preuve des tests (§7 : `gcov` et mutations)

`gcov` compile les TU des classes touchées en `--coverage`, dans une copie jetable du scratchpad. Il doit montrer exécutées :
- a1 : chaque accesseur (branches NULL, remplaçant, classe attendue), chaque site D et E ;
- a2 : chaque site A converti, `pico8_jc_end_p`/`_ra` (branches `jo`, point de souris, NULL), la lecture de `mt` sur `fr` et hors `fr`, `ie:hb` avec `gy` NULL et non NULL, `pico8_lt`.

Une ligne non exécutée est couverte par une assertion (règle 7) ou justifiée au message.

Mutations **une à la fois, dans l'arbre de travail**. Le fichier est sauvegardé par `cp` avant et restauré par `cp` après (jamais `git checkout`/`stash`). Chaque TU est lancé exactement comme le `Makefile` le lance ; un TU lié sous ASan l'est avec `ASAN_OPTIONS=detect_leaks=0`. Chaque mutation doit rougir au moins un TU, nommé au message :

- a1 :
  - **M1** `pico8_fm_hy()` rend le remplaçant casté au lieu de NULL → `test_pico8_fm` (assertion de la règle 1) ;
  - **M2** `pico8_je_nf()` rend toujours NULL → `test_pico8_ui` (le survol et les clics lisent `je.ra`) ou `test_pico8_iv` (`iv:pq`) ;
  - **M3** `pico8_mo_qb()` rend toujours NULL → `test_pico8_bg` (`bg:hq` n'écrit plus `mo.v`, ou s'arrête) ou `test_pico8_ga` (`mo.ma`) ;
  - **M4** `pico8_nf_l()` et `pico8_nf_r()` échangés → `test_pico8_ui` (boutons gauche et droit) ;
  - **M5** `jk` du provisoire ignoré (lu 0) dans `cb` ou `pj:hb` → un TU qui tape le code avant `eu()` et lit l'effet (`test_pico8_fm`, `test_pico8_pj`) ; sinon trou justifié ou assertion ajoutée.
- a2 :
  - **M6** `dg` n'incrémente plus `ke->t` → `test_pico8_ui`/`test_pico8_fq`/… (séquences) ;
  - **M7** `pico8_dh` tient `me` pour toujours `"ly"` → `test_pico8_fr`/`_hj`/`_nl`/`_os`/`_pm`/`_mx` (`pico8_dh(e)` plié) ;
  - **M8** `pico8_jc_end_p` ignore le point de souris (rend nul) → `test_pico8_jc` (coût `od` de `jc:hq`, `pico8_dj`) ;
  - **M9** `jc:hq` ne lit `mt` que sur `qu` → `test_pico8_fr` ou `test_pico8_jc` (voie vers une porte d'infraspace) ;
  - **M10** `jo:gv` omet le test de classe `jc` et compare `qu` à NULL → `test_pico8_bg`/`test_pico8_mz` (`pico8_jo_gv(ly, bg)`) ;
  - **M11** `ie:hb` ne pose plus `self->ra` → `test_pico8_entity_kind` ;
  - **M12** `bb_visit` écrit `mi` sans `has_mi` → `test_pico8_hy`/`test_pico8_trade` ;
  - **M13** `bd_route_visit` saute `pico8_jc_hg_add` → `test_pico8_jc` (`hg`) ;
  - **M14** le lecteur `_cr_t` de `cr` lit NULL → `test_pico8_ui`/`test_pico8_js` (rangées de `cr`).
- Une mutation qui ne rougit rien est un trou : couvert par une assertion (règle 7) ou justifié au message.

### Filets

- `make test`, `make e2e` (26 conformes) à chaque commit. Aucun e2e ajouté : les 26 passent par `dg`, `bb`/`bd`/`dh`, `jc:hq`/`ce`, `bv`, `ui:lj`/`gw`/`bh` et `iv:pq`, et un e2e nouveau n'ajouterait de couverture qu'à des lignes que les TU couvrent déjà.
- `make class-coverage-net` à a2 (porte `pico8_lt` retypée) : identique à `tools/class_coverage_net_expected.txt`.
- `make sanitize-net` : non (aucune durée de vie ni aucun propriétaire ne change).
- **Balayage ASan+UBSan des TU** à la fin de a2 : tous les TU compilés sous `-fsanitize=address,undefined` dans une copie jetable (`Makefile` inchangé), lancés avec `ASAN_OPTIONS=detect_leaks=0`, comme à D.7b, D.10a et D.10b. Sortie propre. Ce balayage attrape un cast vers `pico8_ke_t` d'un en-tête plus court (H3).

### Vérification, budget et tableau « Runs »

**Budget : environ 100 minutes pour l'étape** (a1 ≈ 40, a2 ≈ 60), mesures et filets compris. La relecture par un sous-agent frais est hors budget.

| Cible | Durée attendue |
|---|---|
| `make test` | de l'ordre de la minute (recompilation : `pico8_entity.h`, `pico8_probe.h`, `pico8_ui.h`, `pico8_slipway.h` changent) |
| `make e2e` | ~1 s |
| `make class-coverage-net` (a2) | quelques secondes |
| `make data-residue`, recette, classification, greps | < 1 min |
| `gcov` des TU touchés | quelques minutes par chunk |
| mutations (5 + 9, chacune un `make` ciblé du TU) | < 15 min |
| balayage ASan+UBSan des TU | quelques minutes, une fois |

Au-delà du budget, l'implémenteur s'arrête et note ce qui n'a pas été lancé. **Jamais l'appli raylib** ; `make raylib` n'est pas exigé. Aucun TU ne rejoue de fichier d'événements.

**L'implémenteur tient un tableau « Runs »** dans son journal, une ligne par lancement : `#`, commande exacte, état de l'arbre (HEAD + chunk + étape), code de sortie, ligne clé, durée. Le vérificateur le réutilise et ne relance que ce qui suit le dernier changement de l'arbre.

### Contrôles du vérificateur

1. **Périmètre** : `git show --stat` de chaque commit ne liste que les chemins de la table « Empreinte » (a1 : `src/` et `test_pico8_fm` ; a2 : `src/`, `test_pico8_ui`, `test_pico8_entity_kind`, `docs/plan-3.md`). Ni `tools/`, ni `Makefile`, ni `tests/e2e/`, ni `*.sha256`.
2. **Greps** de la « Sortie » : exactement ce qui y est écrit, après a2. Après a1 : les greps 1 (partie a1) et 3 sont vides.
3. **Casts** : chaque cast d'un `pico8_struct_header_t *` (ou `void *`) vers une struct, ajouté ou gardé dans une fonction touchée, est précédé dans la même fonction d'un test de famille ou de classe, ou vient d'un accesseur qui le fait. Relu dans le diff, cast par cast.
4. **Sites** : chaque site A, D, E et G de l'inventaire est converti ; aucun site B, C, F, K, M n'est touché, en particulier `wormhole.c:78`, `entity_geom.c:99` et `wormhole_display.c:41` (reclassés).
5. **Constantes** : aucune `*_HASH`/`identity[]` changée.
6. **Anti-tassement** :
   - comptes avant/après en lignes ET occurrences, par catégorie et par fichier ;
   - classification présente et juste, (b) = (c) = 0 pour `par-nom` ;
   - lignes gagnées listées ;
   - aucune cale valeur ↔ typé, aucun registre, aucun alias.
7. **Tests retirés** listés avec leur raison. Mutations, `gcov`, balayage ASan+UBSan et `class-coverage-net` rapportés.
8. **Commentaires** : quatre sortes de `CLAUDE.md`. Les trois contrats réécrits disent l'accesseur typé. Aucun renvoi à D.11a ni au plan dans `src/` ou `tests/`.
9. **Docs** : note « Découvert à D.11a » en fin de ligne D.11a ; rien d'autre dans le plan.

### Critères d'échec (traduits en `FAILED:` par le vérificateur de `prd-autopilot`)

**Départ et filet (§7)**

- `FAILED:` si l'arbre de départ n'est pas le HEAD où D.10b (et D.10c, si elle a une note) est gradée, propre. Échoue aussi si `make data-residue` n'y rend pas `dispatch` 0 et `par-nom` `src/` 418 sans explication (H-D10b-2, H-D10b-5).
- `FAILED:` si le filet (`make test`, `make e2e`) reste rouge après **3 tentatives** sur un même commit.
- `FAILED:` si un `*.sha256` est modifié ou supprimé, si un fichier de `tests/e2e/` est touché, si un e2e est ajouté ou retiré, ou si un TU rejoue un fichier d'événements.
- `FAILED:` si un test de régression du §6 change, ou si un test devrait changer d'intention pour passer.
- `FAILED:` si `make class-coverage-net` diffère de l'attendu versionné à a2, ou si le balayage ASan+UBSan des TU n'est pas propre.
- `FAILED:` si un champ ou un site ne tombe sous aucune règle (arrêt « champ sans règle » du §7), ou si un écart port/cart découvert n'est pas préservé et décrit par une entrée `TROU-N`.

**Périmètre et types**

- `FAILED:` si une grep de la « Sortie » ne rend pas ce qui y est écrit.
- `FAILED:` si un cast d'en-tête vers une struct n'a pas son test de famille ou de classe dans la même fonction, ou si un accesseur de singleton rend un occupant qu'il n'a pas testé.
- `FAILED:` si une lecture par valeur par défaut change : vecteur nul, absent ou 0 sur un remplaçant, un NULL ou une classe qui n'a pas le champ.
- `FAILED:` si une écriture sur le `mo` provisoire cesse d'arrêter le jeu.
- `FAILED:` si un site B, C, F, K ou M est converti (c'est D.11b-D.11c), dont les trois reclassés, ou si une aide qui garde un appelant B/C/F/M est supprimée.
- `FAILED:` si un fichier de `tools/` ou le `Makefile` change.

**Gardes, comptes et anti-tassement**

- `FAILED:` si `make data-guard` n'est pas vert.
- `FAILED:` si `par-nom` `src/` ne vaut pas départ − 18 après a1 et départ − 76 après a2, ou si le détail par fichier diffère de la table sans écart expliqué.
- `FAILED:` **si un compte baisse par tassement** : lignes jointes, construction déplacée, renommage d'un identifiant vivant, commentaire compté réécrit hors de son sujet, alias, registre nouveau, cale valeur ↔ typé, lecture par nom déplacée dans une aide non comptée, code déplacé hors du compteur, ou une (c).
- `FAILED:` si le **message d'un commit** omet l'un de ces éléments :
  - par catégorie et par fichier, les comptes avant/après **en lignes et en occurrences** ;
  - la classification (a1)/(a2)/(b)/(c) des lignes `par-nom` retirées ;
  - la liste par fichier des lignes qui gagnent une catégorie (« aucune » compris) ;
  - la justification de chaque couple Δlignes > Δoccurrences.
  Un commit de rattrapage ne compense pas un message incomplet.
- `FAILED:` si `make constants-check`, `float-grep` ou `float-guard` rougit.

**Preuve, commit et note**

- `FAILED:` si une constante `*_HASH` ou `identity[]` change.
- `FAILED:` si un cas de test retiré n'est pas listé au commit avec la raison qui le rend inexprimable.
- `FAILED:` si une mutation manque, ne se restaure pas par `cp`, ou ne rougit rien sans justification.
- `FAILED:` si le budget est dépassé sans note, ou s'il manque le tableau « Runs ».
- `FAILED:` s'il manque la note « Découvert à D.11a », ou si elle omet l'un de ces éléments : les comptes réels, les accesseurs, la porte typée de `hv`, le contrat Q8 de `pico8_fm_hy()`, les trois reclassés et les aides qui restent.
- `FAILED:` si un commentaire ajouté ou reformulé sort des quatre sortes de `CLAUDE.md`.

## Out of Scope

- **Classes B, C, F, K** (D.11b), dont les trois sites reclassés de `mz.ox`. **Classe M** (D.11c), dont `probe.c:280` (`me` de `dl`) et le G `ke.c:43`. **TU** (D.11d : T1/T2, les deux tests §6). **Infrastructure** (D.11e : lecteurs `field`, écrivains `set_field`, `*_take_*` des écrivains, `kind_field`, `pico8_entity_get_field`/`set_field`, structs-vues).
- Retyper une API à valeur (`pico8_jc_hg_add`/`_has`, `pico8_trade_qe`, `pico8_jo_hf`, `pico8_jc_ce`, items de rangée) : Q12, ouverte.
- Les listes (`hv`, `cg`, `jo.cc`/`by`/`jy`, `trade.lp`/`rl`, `mz.ox`) : D.12. D.11a lit leurs éléments comme aujourd'hui (`.as.ref`), avec le cast vérifié.
- `valeur`, `type-tag`, `ref-audit` au-delà de ce que D.11a retire en passant.
- `tools/`, le `Makefile`, `tests/e2e/`, la phase 2, `make raylib`, l'appli raylib, l'audio, le rendu.

## Further Notes

- **Décisions de l'agent à la place de l'utilisateur** (consigne `to-prd` : vérifier modules, tests et découpage avec l'utilisateur), tranchées en `guideline` et marquées (G) :
  - **cast vérifié** : test de famille (`pico8_jo_is_family`) ou de classe exacte (`jc`, `trade`, `fr`, `qb`) dans la même fonction, ou type statique du receveur. Rejeté : le cast non vérifié.
  - **`ke` sur `hv`** : `pico8_lt(pico8_ke_t *)`, `giver_t` de `test_pico8_ui` porte un `ke`. Rejetés : un marqueur de classe, le cast non vérifié.
  - **singletons** : un accesseur par singleton dans le module qui le possède, qui asserte l'occupant. `pico8_fm_hy()` prend le contrat Q8 (garde du remplaçant perdue, déclarée). `pico8_fm_placeholder()` pour `jk`/`nz`. `pico8_nf_l()`/`pico8_nf_r()` pour `je[bt]`. Écriture sur le `mo` provisoire toujours arrêtée.
  - **extrémités de voie** : `pico8_jc_end_p`/`_ra` (planète ou point de souris). Rejetés : signature de `dj` changée, cast vers `jo` seul, accesseur de classe (Q6).
  - **G** : `jc.mt` lu sur `fr` par classe exacte ; `ie.gy` typé.
  - **consommateurs à valeur** : non retypés (Q12), valeur reconstruite à l'unique appel, déclarée.
  - **reclassement** : `wormhole.c:78`, `entity_geom.c:99`, `wormhole_display.c:41` → C, D.11b.
  - **découpage** : a1 (singletons, 18 chemins), a2 (receveurs connus et G, 26) ; un TU touché une fois.
  - **filets** : `class-coverage-net` à a2, pas de `sanitize-net`, balayage ASan+UBSan des TU à la fin.
- **D.10c.** Si la note de D.10b déclare D.10c vide (H-D10b-1), l'étape suivante de `plan-pilot` après D.10b est D.10c, constatée sans commit de code, puis D.11a. Ce PRD ne change pas le plan.
- **Note du plan (G)**, à ajouter en fin de ligne D.11a (le §7 ne garde que ce qui change une étape suivante) :

  > Découvert à D.11a : deux chunks (a1 : N1 chemins ; a2 : N2). Singletons (Q8) : `pico8_fm_hy()` rend NULL sur le `fm` provisoire (son assert part, les méthodes de `hy` prennent NULL), `pico8_fm_placeholder()` le rend typé pour `jk`/`nz`, `pico8_je_nf()`, `pico8_mo_qb()` (une écriture sur le `mo` provisoire arrête toujours), `pico8_nf_l()`/`pico8_nf_r()` pour `je[bt]`. Receveurs connus : cast vérifié par `pico8_jo_is_family` ou une classe exacte (`jc`, `trade`, `fr`, `qb`) ; `pico8_lt` prend `pico8_ke_t *` (porte de `hv`, `me`/`t` de `dg` lus typés ; `giver_t` de `test_pico8_ui` porte un `ke`) ; extrémités de voie par `pico8_jc_end_p`/`pico8_jc_end_ra` (planète ou point de souris). G : `jc.mt` lu sur `fr` (`jc_take_fixed` supprimée), `ie.gy` en `const pico8_vec_t *` (`pico8_ie_eh`/`pico8_ie_set_gy` typés, `ie_take_vec` supprimée, quatre cas `hb_from_gy` retirés). **Reclassés en C (D.11b)** : `mz_pq_ox_predicate` (`fx` sur `hv`), `pico8_ij` (`p`) et `pico8_mzd_ra_of_ij` (`ra`), parce que `mz.ox` tient des planètes et des `g_`/`fv` (`fx=1`). Aides H restantes (appelant B/C/F/M) : `field_truthy` d'`economy_score`/`economy_tiers`/`planet_display`, `field_fixed` de `planet_display`, `pico8_entity_geom_field_*`, `pico8_msgbox_field_vec`, `pico8_probe_field_fixed`/`_truthy`, `pico8_ui_field_raw`, `pico8_mzd_field_vec`. Q12 : `pico8_ie_eh`/`pico8_ie_set_gy` quittent la liste ; `pico8_jc_hg_add`/`_has`, `pico8_trade_qe`, `pico8_jo_hf` reçoivent une valeur reconstruite du membre à leur appel. `par-nom` P (`src/` S) ; `valeur` V ; résidu D R.

  N1, N2, P, S, V, R sont les valeurs réelles.
- **Le message de chaque commit** porte :
  - le périmètre du chunk ;
  - la classification (a1)/(a2)/(b)/(c) des lignes `par-nom` retirées ;
  - les comptes avant et après **en lignes et en occurrences**, par catégorie et par fichier, avec les couples Δlignes > Δoccurrences justifiés ;
  - les lignes qui gagnent une catégorie, par fichier (« aucune » compris) ;
  - les cas de test retirés et leur raison, les assertions ajoutées et la ligne qu'elles couvrent ;
  - les constantes (inchangées), les mutations, `gcov`, les filets ;
  - le tableau « Runs » résumé ;
  - les réponses aux hypothèses H et `H-D10b` qui le concernent ;
  - la garde perdue de `pico8_fm_hy()` (a1).
- **Contrôle d'étape** : un sous-agent frais relit le diff de chaque chunk contre ce PRD avant son commit (hors budget) : casts, anti-tassement, classification, sites laissés, constantes, cas retirés.
- **Surprises pour le pilote** :
  - **`pico8_fm_hy()` asserte sur le remplaçant**, alors que D.9 le disait « déjà sur ce modèle » (NULL). Q8 impose NULL ; la garde des appels de méthode sur le `fm` provisoire disparaît, déclarée.
  - **Trois sites A de l'annexe de D.9 ont un receveur hétérogène** : `mz.ox` = `jp(hv, fx)`, et `g_` porte `fx=1`. Ils sont reclassés en C (D.11b).
  - **Aucun test d'exécution ne prouve qu'un en-tête est un `ke`**, et `test_pico8_ui` inscrit dans `hv` un en-tête seul : `pico8_lt` est retypé.
  - **`jc.re` peut être le point de souris** (vue de `slipway.c`) et `cg("jc")` inscrit le `jc` en cours de tracé : les lectures de `p`/`ra` d'extrémité ont besoin d'un accesseur à deux formes.
  - **`mt` n'existe que sur `fr`** : le site G de `slipway.c` se ferme par un test de classe exact, sans le lien parent de Q5.
  - **`ie.gy` est un état de module public à valeur** : fermer le site G retype `pico8_ie_eh`/`pico8_ie_set_gy` et rend inexprimables quatre cas de `test_pico8_entity_kind`.
  - **Empreinte 33 + 3** (la ligne dit ≈ 22) ; `par-nom` `src/` ≈ 342 ; total ≈ 900 (la ligne dit ≈ 910).
  - **Les singletons `je`/`mo` n'ont ni `jk` ni table de bouton sur le remplaçant**, mais le `fm` provisoire porte `jk`/`nz` : trois lectures de `jk`/`nz` doivent le consulter, sans quoi le code de triche tapé sur l'écran titre cesserait d'être lu.
