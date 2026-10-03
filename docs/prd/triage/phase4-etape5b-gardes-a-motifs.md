---
title: "Plan 5, sous-étape 5b — gardes à motifs : motifs du cart qui apparient 0 retirés avec leurs fixtures (alerte 14 de D-68, D-5 à D-11), D-12, D-57, D-63 ; test de vivacité en noms anglais pour chaque catégorie gardée"
description: "composition-guard et data-guard sont verts dans make test parce que rien ne peut plus apparier leurs motifs écrits en noms du cart ; appliquer le verdict commun de l'alerte 14 (un motif du cart qui apparie 0 part avec ses fixtures) aux deux scripts de garde, puis donner à chaque catégorie restée gardée une violation plantée en noms anglais qui fait échouer --check, avant les renommages de 5c."
status: needs-triage
---

## Texte de l'étape (copié de `docs/plan-5.md`, §6, à `39bf814`)

> 5. **Clôture.** Condition d'entrée : chaque entrée de `docs/decisions-plan-5.md` porte un verdict humain, pris dans l'ensemble fermé du registre (« Conventions » : `proposition`, `variante nommée`, `garder tel quel`, `reporter hors plan 5`), l'alerte 14 de D-68 comprise. Une PRD par sous-étape, dans l'ordre 5a–5f ; chacune applique les entrées de sa famille (colonne « Application » de l'index du registre) et finit par son contrôle de sortie, avec l'invariant du §5. Une entrée `garder tel quel` ou `reporter hors plan 5` n'édite rien, sauf la ligne d'exception d'un reste compté (5d).
>
> 5b. **Gardes à motifs** (famille G : D-5 à D-12, D-57, D-63, et l'alerte 14 de D-68, tranchée en premier), par la procédure G, avant tout renommage : chaque motif gardé reçoit son test de vivacité, une violation plantée écrite avec les noms anglais, pour qu'un ancien nom réintroduit en 5c soit vu. Une exemption qu'un renommage de 5c touche (D-44) suit ce renommage dans son commit, procédure G comprise.
>
>    **Sortie** : `--detail` de `tools/composition_residue.sh` et de `tools/data_residue.sh` relevé avant et après, égal à celui d'avant moins ce que les verdicts retirent ; chaque motif gardé a sa violation plantée qui fait échouer `--check` ; `make test` vert.

Phase 4 du pilotage (tronçon 4 : étape 5). Étape 5b = la deuxième sous-étape de l'entrée 5, après 5a (famille O) et avant 5c (famille N).

Entrées lues, toutes au commit `39bf814` (`git show HEAD:<chemin>`, rien de non commité) : `docs/plan-5.md` (§1, §2, §4, §5, la méthode des sous-étapes de l'étape 3 et sa puce *Gardes à motifs*, l'inventaire des gardes à motifs de la note de 3a, les notes de 3b à 3l sur les gardes, les notes de 4l2 et de 4l4, « Fin de l'étape 4 » et ses trois constats, l'entrée 5 et 5a–5f) ; `docs/decisions-plan-5.md` (« Conventions », procédure G, index, D-5 à D-12, D-44, D-57, D-63, D-66, D-67, D-68 et son alerte 14, « Clôture (4l4) ») ; `tools/composition_residue.sh`, `tools/composition_residue_sites.txt`, `tools/data_residue.sh`, `tests/test_composition_residue.c`, `tests/test_data_residue.c`, les lignes `data-guard`, `composition-guard`, `DATA_RESIDUE_GUARD`, `COMPOSITION_RESIDUE_GUARD` et la légende du `Makefile` ; `tools/rename_count.py` (ce que voit la garde anti-cart) ; la PRD de 4l4 (`git show 939ed36:docs/prd/triage/phase3-etape4l4-scripts-de-garde-commentaires.md`) pour la forme.

Aucun humain n'était disponible pour les points « vérifier avec l'utilisateur » de `to-prd` (modules, tests, découpage) : l'agent rédacteur a chargé lui-même le skill `guideline` et l'a appliqué (changements chirurgicaux, orphelins de ses propres retraits retirés et rien d'autre, critères vérifiables, hypothèses dites, aucun verdict préjugé au-delà de sa lettre). Sa règle 5 sur les commentaires cède devant la règle du `CLAUDE.md` (trois destinations), comme aux étapes 3 et 4.

**5a n'est pas faite au moment de la rédaction.** Tout ce que cette PRD suppose de son issue est écrit en hypothèse `H-5a-k` (§ « Hypothèses sur 5a »), avec la liste des sections à relire quand la note « Découvert » de 5a existera.

**Relu contre la note « Découvert » de 5a (`040eb67`)** : H-5a-1 à H-5a-8 tiennent. Sortie de 5a égale au tableau « Restes comptés » ; aucun des fichiers, variables ni cibles de H-5a-2 touché (seuls les numéros de ligne du `Makefile` ont bougé : les relire au `HEAD`) ; `old-names-guard`, `ast-guard` et `french-guard` existent, hors `test` et `test-all` ; la partie texte ne cherche que les lignes `renommé` du TSV ; lecture des verdicts consignée (« ok » = `proposition`, alerte 14 relevant de 5b) ; aucune convention de marquage du registre ; extraction par `make rename-extract`, même format ; `french_exceptions.txt` inchangé.

## Problem Statement

Le §5 du plan veut des gardes qui voient le code qu'elles visent. Deux gardes de `make test`, `composition-guard` et `data-guard`, ne sont vertes que parce que plus rien n'apparie une partie de leurs motifs, écrits en noms du cart (constat de « Fin de l'étape 4 » : `tools/composition_residue.sh:66-69`, `tools/data_residue.sh:20`). Ces motifs passent à vide sur l'arbre ; leurs fixtures, coupées en deux littéraux, les exercent encore et gardent dans les tests 76 lignes au préfixe `pico8_`/`PICO8_` dans `tests/test_composition_residue.c` et 23 dans `tests/test_data_residue.c`. Une catégorie gardée qui ne peut plus rien voir donne une fausse assurance ; une catégorie restée utile, sans violation plantée écrite avec les noms d'aujourd'hui, peut devenir aveugle sans que personne ne le voie quand 5c renommera les noms qu'elle cite.

L'humain a tranché (commit `39bf814`, « ok » sur chaque ligne `Verdict`, lu `proposition`) : l'alerte 14 de D-68 fixe une règle commune, « un motif écrit en nom du cart qui apparie 0 est retiré avec ses fixtures (D-11 (a), étendu à la branche de D-5, D-7, D-8 et D-9), la garde anti-cart bloquante (5e) interdisant déjà ces noms ». D-6, D-10, D-12, D-57 et D-63 en découlent. Il reste à l'appliquer sans toucher au reste, et à rendre chaque catégorie gardée vérifiablement vivante.

## Solution

1. Avant tout, une mesure de départ sur l'arbre de fin de 5a : `--detail` des deux scripts, `--fields`, `--check` de chaque liste de garde, vivacité des onze exemptions sur le vrai arbre, compteurs sans `SUBSTEP`.
2. Un commit `test:` d'abord : un test de vivacité par catégorie qui restera gardée, violation plantée écrite en noms anglais, qui fait échouer `--check <catégorie>` ; un contrôle que la liste de ces catégories est exactement celle du `Makefile`. Chaque test est prouvé mordant par une mutation jouée sur une copie.
3. Un commit `refactor:` par script (forme de la proposition de D-11) : catégories, branches et exclusions retirées selon les verdicts, leurs fixtures avec elles, l'entrée de la liste de garde du `Makefile`, et ce que ces retraits rendent orphelin dans le script. Rouge joué : le script seul fait rougir chaque fixture retirée, puis les fixtures partent.
4. La note « Découvert » sous l'entrée 5b du plan.

À la sortie : `--detail` des deux scripts identique à l'octet à celui d'avant (les catégories retirées apparient 0 ligne sur l'arbre ; les lignes `void-ptr-r2` et `info` restent), `--fields` identique, `make test` vert, chaque catégorie gardée prouvée vivante en noms anglais, et l'exemption de D-44 laissée à 5c avec sa preuve de vivacité sur le vrai arbre déjà relevée.

## User Stories

1. En tant que mainteneur, je veux qu'une catégorie de `composition-guard` ou de `data-guard` ne reste dans sa liste de garde que si elle peut encore voir quelque chose, pour qu'un vert veuille dire « vérifié » et non « aveugle ».
2. En tant que mainteneur, je veux que chaque catégorie gardée ait une violation plantée écrite avec les noms C d'aujourd'hui, qui fait échouer `--check`, pour savoir qu'elle mord encore.
3. En tant qu'exécutant de 5c, je veux que les tests de vivacité existent avant mes renommages, pour qu'un nom que je renomme et qu'un motif cite rougisse un test au lieu de rendre la garde muette.
4. En tant qu'exécutant de 5c, je veux que les fixtures de vivacité n'utilisent aucun ancien nom d'une entrée N, pour ne pas hériter de sites supplémentaires à renommer.
5. En tant qu'exécutant de 5c appliquant D-44, je veux savoir que l'exemption `planet_inspection_panel_ctx_t.unmet_needs` est vivante sur le vrai arbre (retirée seule, elle fait sortir `planet_display.c:88`), pour la suivre dans mon commit par la procédure G.
6. En tant que relecteur, je veux que le verdict commun de l'alerte 14 soit appliqué en premier et que chacune des entrées D-5 à D-12 en découle visiblement, pour relire une décision et non dix.
7. En tant que relecteur, je veux un commit par script, pour relire chaque garde séparément.
8. En tant que relecteur, je veux que `--detail` des deux scripts soit égal à l'octet avant et après, pour voir que rien de vivant n'a été retiré.
9. En tant que relecteur, je veux que `--fields` de `composition_residue.sh` soit égal à l'octet avant et après le retrait de la branche `pico8_X_start` de D-8.
10. En tant que mainteneur, je veux que `--check <catégorie retirée>` soit refusé (code 2, catégorie inconnue), pour qu'une liste de garde périmée casse au lieu de passer.
11. En tant que mainteneur, je veux que l'outillage d'exclusion devenu sans emploi (exclusions par `sed` de `composition_residue.sh`, branche `spec` de `collect` dans `data_residue.sh`) parte avec la dernière exclusion, pour ne pas laisser de code mort créé par ces retraits.
12. En tant que mainteneur, je veux que les fixtures au préfixe du cart des catégories restées (`parent`, `entity-ptrs`, `registries`, `void-self`…) restent telles quelles, comme D-57 et D-63 (a) le disent, pour ne pas élargir 5b au-delà du verdict.
13. En tant que mainteneur, je veux que les témoins des sept noms `_default` exemptés par `SPEC_EXCLUSION` restent comme faux positifs nommés, comme D-9 le dit, même une fois l'exclusion retirée.
14. En tant que mainteneur, je veux que la liste `tools/composition_residue_sites.txt` reste imprimée par `--detail`, à la même place, puisque aucun verdict ne la retire.
15. En tant que mainteneur, je veux que les fixtures neuves soient coupées en deux littéraux comme les anciennes, pour que les deux gardes, qui lisent `tests/`, restent vertes sur le vrai arbre.
16. En tant que pilote de 5c, je veux que 5b n'élève aucun compteur sans `SUBSTEP` au-dessus du tableau « Restes comptés », puisque ce tableau est le cliquet de 5c.
17. En tant qu'humain, je veux lire dans la note « Découvert » les motifs écrits en noms du cart qu'aucune entrée G ne couvre et qui restent gardés, pour trancher plus tard ; aucun n'est supprimé sans verdict.
18. En tant qu'humain, je veux lire dans la note que la garde anti-cart ne voit que les anciens noms des lignes `renommé` du TSV, et donc pas les noms historiques dont l'alerte 14 retire les motifs, pour savoir ce que le retrait laisse sans garde.
19. En tant qu'exécutant, je veux que toute mesure et toute mutation se fasse dans `tmp/` ou dans une copie jetable, pour ne jamais laisser l'arbre de travail dans un état intermédiaire.
20. En tant qu'agent, je veux que l'appli raylib ne soit jamais lancée, `make raylib` et `make screencast-viewer` seulement compilés.

## Implementation Decisions

### Condition d'entrée

- Commits de 5a poussés sur la branche du chantier ; note « Découvert » de 5a présente sous l'entrée 5a du plan ; sortie de 5a tenue (H-5a-1).
- Verdicts : à `39bf814`, les 66 lignes `- **Verdict** :` du registre portent « ok », lu `proposition` (les quatre valeurs du registre sont fermées ; « ok » sur la proposition prête est la lecture retenue par la session, et la seule qui ait un sens pour chaque entrée). Les deux lignes `Verdict (un par alerte)` de D-28 et de D-68 sont **restées vides** dans ce commit : la lecture retenue par la session est `proposition` pour chaque alerte, alerte 14 comprise. 5b ne rouvre pas cette lecture ; elle vérifie seulement que 5a l'a consignée ou qu'un humain a rempli ces lignes (H-5a-5). Si 5a l'a lue autrement pour l'alerte 14, 5b n'ouvre pas (« 5b n'ouvre pas sans lui », D-68).
- D-66 = option (c), selon la session (H-5a-3).

### Mesure de départ (sur l'arbre de fin de 5a, dans `tmp/5b_baseline/` seulement)

Relevée avant tout commit ; les valeurs de `39bf814` sont données pour comparaison et ne remplacent pas la mesure.

- `sh tools/composition_residue.sh --detail` : 11 lignes `void-ptr-r2` (`is_message_name` … `slipway_drag`), `info	src/game/game_data.h	438 lines`, `residue 2: 0 line(s), 0 file(s)`.
- `sh tools/data_residue.sh --detail` : le seul pied `residue D: 0 line(s), 0 file(s)`.
- `sh tools/composition_residue.sh --fields` : 260 lignes et le pied `fields: 260; no write: 15; no write outside a constructor: 27` ; la sortie entière est gardée pour la comparaison à l'octet.
- `make composition-guard`, `make data-guard` : sorties gardées.
- Vivacité des exemptions sur le vrai arbre (une copie du script, une exemption retirée à la fois) : chacune des onze fait sortir exactement une ligne. `REGISTRIES_EXEMPT` : `particle.h:50`, `planet_display.c:88` (`unmet_needs`, D-44), `collect.h:24`, `collect.h:30`, `research_display.h:26`, `collect.h:11`, `collect.h:17` ; `ENTITY_PTRS_EXEMPT` : `main_economy.h:30`, `planet.h:41`, `research.h:42`, `slipway.h:59`.
- Compteurs sans `SUBSTEP` après extraction fraîche (mécanisme de 5a, H-5a-7) : égaux au tableau « Restes comptés » de la « Clôture (4l4) » du registre (H-5a-1).
- Croisement des noms : chaque nom cité par un motif, une exemption, `REG_TYPES`, `CORE_HEADERS` ou la première colonne de `tools/composition_residue_sites.txt`, contre les anciens noms des entrées N du registre. Attendu : seul `unmet_needs` (D-44). Un autre résultat va dans la note et ajoute une ligne au § « D-44 et les autres exemptions ».

### L'alerte 14 de D-68, tranchée en premier

Règle appliquée, selon le verdict : un motif écrit en nom du cart qui apparie 0 ligne sur l'arbre est retiré avec ses fixtures, pour les six entrées que l'alerte nomme (D-5, D-7, D-8, D-9, D-11) et ce qui en découle (D-6, D-10, D-12, D-57, D-63). La règle ne s'étend pas d'elle-même aux motifs qu'aucune entrée G ne couvre (§ « Ce qui reste écrit en noms du cart ») : « aucune entrée n'est appliquée avant son verdict », et la méthode du plan veut qu'une catégorie intraduisible aille dans « Découvert », pas à la suppression.

Ce que la règle donne, entrée par entrée (lignes à `39bf814`) :

| Entrée | Script | Retiré | Reste |
|---|---|---|---|
| D-11 | `composition_residue.sh` | catégories `nu`, `name-dispatch`, `one-or-false` (lignes 67–69) et `NU_EXCLUSION` (80) | — |
| D-11 | `data_residue.sh` | catégories `value` (29), `ob` (30), `stash` (31), `type-tag` (32), `ref-audit` (33), `list` (40–41) | — |
| D-5 | `composition_residue.sh` | catégorie `void-ptr` : le lecteur de prototypes et `VOID_EXCLUSION`, qu'elle seule emploie | lignes `void-ptr-r2` de `--detail`, à la même place (après `header-accessors`, avant `void-self`) ; `tools/composition_residue_sites.txt`, `COMPOSITION_R2` et le refus d'une liste absente |
| D-7 | `composition_residue.sh` | branche `\bpico8_[a-z0-9_]+_of\b` de `parent-link` | branches `(emitter\|probe)_of` et `(->\|.\|(*)parent` |
| D-6 | `composition_residue.sh` | `PARENT_LINK_EXCLUSION` (proposition : suivre D-7, qui retire sa branche) | le faux positif nommé `entity_state_of` de `tests/test_composition_residue.c` |
| D-8 | `composition_residue.sh` | catégorie `defaults` ; le `gsub` `pico8_X_start` de `--fields` (ligne 697) | `--fields` et son suffixe de constructeur |
| D-9 | `data_residue.sh` | branche `\bpico8_[a-z0-9_]+_(default\|defaults_reset)\b` de `spec` ; `SPEC_EXCLUSION` (49) et son emploi (94) | branches `g_X_(spec\|defaults)` et `X_default_(fixed\|…)` ; les témoins des sept noms `_default`, comme faux positifs nommés |
| D-10 | `composition_residue.sh` | `x == "PICO8" \|\| x == "PICO8_" \|\|` de `mutual-usage` (542) | le reste du filtre |
| D-12 | — | rien (proposition (a) : le suffixe reste) | `--fields` identique |
| D-57 | `tests/test_composition_residue.c` | les fixtures des catégories et branches retirées ((c)) | les noms arbitraires des fixtures des catégories restées ((a)) |
| D-63 | `tests/test_data_residue.c` | les fixtures des catégories et branches retirées ((c)) | les noms arbitraires des fixtures restées ((a)), dont `pico8_jo_truthy` (faux positif de `by-name`) |

Alerte 15 (`SPEC_EXCLUSION` en dur) : abandonnée par son verdict, sans objet une fois l'exclusion retirée. Alerte 16 (767 lignes) : reportée hors plan 5 ; 5b ne découpe rien.

### Orphelins des retraits

Retirés parce que 5b les rend sans emploi, et rien d'autre (guideline, règle 3) :
- `composition_residue.sh` : `PARENT_LINK_EXCLUSION` et `NU_EXCLUSION` partis, la fonction `exclusion` et la branche « exclusion par `sed` » de `collect` n'ont plus d'appelant, avec le commentaire qui énonce leur règle ; `read_prototypes`, `VOID_EXCLUSION` et leurs commentaires ; la mention de `pico8_X_start` dans l'en-tête de `--fields` et dans le commentaire de la fonction `fields` ; la phrase « under void-ptr » de l'en-tête devient la place réelle des lignes `void-ptr-r2`.
- `data_residue.sh` : la branche `spec` de `collect` (le `sed` devient sans objet : `grep` direct, comme D-9 le dit) et le commentaire de `SPEC_EXCLUSION`.
- `CATEGORIES` et `FOOTER_CATEGORIES` perdent les catégories retirées, dans le même ordre pour celles qui restent.
- `Makefile` : `COMPOSITION_RESIDUE_GUARD` et `DATA_RESIDUE_GUARD` perdent les catégories retirées, dans le commit de leur script. Après 5b : `core-includes,reachability-to-prove,entity-ptrs,registries,header-accessors,parent-link,parent,mutual-inclusion,mutual-usage,void-self` et `spec,overrides,dispatch,by-name`. La légende ne cite aucune catégorie : elle ne change pas.
- Les commentaires touchés suivent la règle du `CLAUDE.md` (trois destinations) ; le récit (« retiré par l'alerte 14 ») va au message de commit.

### Tests de vivacité (commit `test:`, avant les retraits)

- « Catégorie gardée » = chaque catégorie des listes de garde du `Makefile` après 5b (14 : 10 + 4). Chacune reçoit une violation plantée, écrite avec des noms C du code d'aujourd'hui (`planet_t`, `pointer_list_t`, `emitter_of`, `g_planet_spec`, `field_truthy`…), qui fait sortir `--check <catégorie>` en code 1 sur la ligne attendue, et un arbre propre qui le laisse à 0. Les catégories tenues par un lecteur plutôt que par un motif `grep` (`parent`, `entity-ptrs`, `registries`, `reachability-to-prove`, `mutual-inclusion`, `mutual-usage`) en reçoivent une aussi : elles sont gardées et lisent des noms (`entity_t`, `pointer_list_t`, la liste d'atteinte).
- Un test par script, piloté par table (catégorie → fichier planté → ligne attendue), plus un contrôle que l'ensemble des catégories de la table est exactement la valeur de la variable de garde lue dans le `Makefile` : une catégorie ajoutée à la garde sans vivacité, ou retirée de la garde avec sa vivacité restée, rougit le test.
- Les noms plantés évitent tout ancien nom d'une entrée N (croisement de la mesure de départ) et tout nom d'exemption que 5c renomme : 5c n'hérite d'aucun site neuf. Ils sont coupés en deux littéraux, comme les fixtures existantes : les deux gardes lisent `tests/`.
- Exception nommée : `dispatch` ne reconnaît que `pico8_value_function` et `pico8_mv(` ; aucune violation en noms anglais ne peut le faire mordre. Sa violation plantée est le jeton du cart dans un fichier et un contexte en noms anglais, et il est consigné « vert par construction » dans la note (§ suivant). Même traitement pour les deux branches `pico8_` de `by-name`, dont la vivacité en noms anglais passe par ses autres branches.
- Preuve que chaque test mord : sur une copie du script dans `tmp/5b_mutation/`, le motif ou le lecteur de la catégorie neutralisé seul fait rougir sa ligne de table et elle seule ; le relevé (catégorie, ligne rouge) va au message de commit.
- Les fixtures existantes des catégories restées ne sont pas réécrites (D-57 (a), D-63 (a)) ; les tests génériques qui s'appuient sur une catégorie retirée (ordre de table, chevauchement, tri de `--detail`, `--check` qui échoue seulement sur la catégorie nommée, mesure sur rien) sont réancrés sur des catégories restées, en noms anglais, dans le commit `refactor:` de leur script.

### Commits `refactor:` (un par script)

- Ordre : `composition_residue.sh`, puis `data_residue.sh`.
- Rouge joué dans chacun : le script édité seul, `make test` rougit aux fixtures des catégories retirées (`--check <retirée>` rend 2, les attentes de comptage changent) et nulle part ailleurs ; puis fixtures retirées, tests génériques réancrés, liste de garde du `Makefile` réduite ; vert. Le relevé du rouge va au message.
- Un test d'absence par script : `--check <catégorie retirée>` rend 2 (« unknown category ») pour chacune.
- Procédure G par commit : `--detail` des deux scripts avant et après, égal à l'octet ; `--fields` égal à l'octet (commit `composition_residue.sh`) ; vivacité du commit `test:` verte ; `make test` vert.
- Messages en français, terminés par les lignes d'attribution de la session.

### Ce qui reste écrit en noms du cart après 5b (pour la note « Découvert »)

Aucune entrée G ne couvre ces motifs ; 5b les garde, avec leur fixture comme violation plantée, et les consigne « vert par construction » pour décision humaine :
- `dispatch` de `data_residue.sh` (`pico8_value_function`, `pico8_mv(`), classée « historique » par l'inventaire de 3a avec `valeur`, absente de la liste de D-11 ;
- les branches `pico8_entity_(get|set)_field` et `pico8_X_(set_)field(_fn)` de `by-name` (« historique ; générique » à 3a) ;
- l'alternative `get_nu` de `header-accessors`, nom de champ du cart absent de l'arbre et du TSV.

Constat à consigner : la garde anti-cart ne cherche que les anciens noms des lignes `renommé` de `docs/renommage.tsv` (`tools/rename_count.py`, classe `Names`). Mesuré à `39bf814` : aucune ligne du TSV pour `pico8_value_t`, `pico8_value`, `pico8_value_function`, `pico8_ob`, `pico8_unstash`, `pico8_mv`, `pico8_list_t`, `pico8_entity_get_field`, `pico8_jo_start`, `pico8_nl_start`, `hp_pose`, `pico8_ho_gp_has`, `pk_is_bool`, `fp_is_bool`, `nu`. La justification de l'alerte 14 et de D-11 (« la garde anti-cart bloquante interdisant déjà ces noms ») ne vaut donc que pour les noms du TSV ; un nom historique ou un `pico8_X` générique réintroduit après 5b n'est vu par aucune garde. Le verdict est appliqué tel quel ; la note le dit, avec une mesure de contrôle : sur une copie, `pico8_` `value_t v;` planté dans `src/`, `cart-names-count` (ou la partie texte de la garde anti-cart sous D-66 (c)) rend 0 (H-5a-3).

### D-44 et les autres exemptions

- 5b n'édite aucune exemption. `planet_inspection_panel_ctx_t.unmet_needs` de `REGISTRIES_EXEMPT`, son commentaire (`planet_inspection_panel_filter_step`) et les deux lignes `chaîne` de `docs/renommage.tsv` qui la citent suivent le renommage de D-44 dans le commit de 5c, procédure G comprise : `--detail` avant et après, vivacité par la ligne `planet_display.c:88` qui sort quand l'exemption est retirée seule.
- Le filet de 5c est déjà là : renommer le champ sans traduire l'exemption fait sortir `composition-guard` sur le vrai arbre dans `make test`. 5b le consigne dans la note, avec les onze relevés de vivacité de la mesure de départ.
- Les tests de vivacité de 5b n'emploient ni `unmet_needs` ni aucun autre ancien nom d'une entrée N.

### Note « Découvert » (sous l'entrée 5b de `docs/plan-5.md`)

Commit `docs:` final. Contenu : la lecture des verdicts (« ok » → `proposition`, lignes de D-28 et D-68) telle que 5a l'a consignée ; ce que chaque entrée a retiré, en lignes et en fixtures ; nombre de lignes `pico8_`/`PICO8_` de chaque test avant et après (76 et 23 à `39bf814`) ; `--detail` et `--fields` égaux ; la table de vivacité et sa preuve par mutation ; le § « Ce qui reste écrit en noms du cart » et le constat sur la garde anti-cart ; les onze relevés de vivacité des exemptions et le renvoi à D-44 pour 5c ; tout écart à une hypothèse H-5a-k. Si un constat contredit le §5 ou l'entrée 5b, le plan est corrigé dans le même commit.

### Fin de PRD

Ordre : `make test-all` ; `--detail`, `--fields`, `make composition-guard`, `make data-guard` comparés à `tmp/5b_baseline/` ; extraction fraîche par le mécanisme de 5a, sans ligne `warning:` ; compteurs sans `SUBSTEP`.

**Critère de fin** :
- `--detail` de `composition_residue.sh` et de `data_residue.sh` égaux à l'octet à la mesure de départ ; `--fields` égal à l'octet ;
- chaque catégorie de `COMPOSITION_RESIDUE_GUARD` et de `DATA_RESIDUE_GUARD` a sa ligne de table de vivacité, rouge par mutation, et la table égale la variable ; chaque catégorie retirée rend 2 sous `--check` ;
- `grep -c 'pico8_\|PICO8_'` des deux tests : seulement les fixtures des catégories restées et les jetons de `dispatch` et `by-name` du § « Ce qui reste », listés dans la note ;
- compteurs sans `SUBSTEP` (`french-count`, `cart-names-count`, `cart-citations`, `cryptic-locals`, `scope-count`) : pas au-dessus du tableau « Restes comptés » (cliquet de 5c) ;
- `make test` vert.

**Invariant (§5)** : `make test-all` vert à chaque commit, `tests/e2e/*.sha256` inchangés ; `make raylib` et `make screencast-viewer` compilent et linkent, jamais lancés ; `class-coverage-net` et `e2e-gap` avec des sorties égales à celles de la fin de 5a ; `rename-tool-test` et les `rename-*-test` verts ; les cibles que 5a a ajoutées à l'invariant (H-5a-3).

## Implementation Chunks

Un seul morceau ; quatre commits, `make test-all` vert à chacun :
1. Mesure de départ dans `tmp/5b_baseline/` (aucun commit) : sorties, vivacité des exemptions, croisement des noms, compteurs.
2. `test:` — tables de vivacité des deux scripts et contrôle contre le `Makefile`, preuve par mutation dans `tmp/5b_mutation/`.
3. `refactor:` — `composition_residue.sh` : D-11 (trois catégories, `NU_EXCLUSION`), D-5, D-7 et D-6, D-8, D-10, orphelins, `COMPOSITION_RESIDUE_GUARD`, fixtures (D-57 (c)), tests génériques réancrés, absence des catégories retirées ; rouge joué, procédure G.
4. `refactor:` — `data_residue.sh` : D-11 (six catégories), D-9 (branche et `SPEC_EXCLUSION`), orphelins, `DATA_RESIDUE_GUARD`, fixtures (D-63 (c)), tests génériques réancrés, absence ; rouge joué, procédure G.
5. `docs:` — note « Découvert » de 5b.

## Testing Decisions

- Un bon test juge une sortie observable : sortie, erreur et code du script sur un petit arbre construit, jamais le texte du script. Le vert d'une garde ne prouve rien seul : c'est ce qui a motivé l'alerte 14 ; d'où la mutation qui rougit chaque ligne de vivacité.
- Tests modifiés : `tests/test_composition_residue.c`, `tests/test_data_residue.c` (C99, `system()` sur le script, sortie relue par fichier). Aucun test neuf de fichier ; aucune nouvelle cible.
- Les attentes des tests génériques changent avec les catégories qu'ils lisent, dans le commit qui retire ces catégories, après le rouge joué par le script seul.
- Prior art : la vivacité des exemptions traduites à 3c, 3e, 3f, 3g (exemption retirée seule sur le vrai arbre, fixture rouge d'abord) ; 4l2 et 4l4 (catégories comme contrat de ligne de commande, lecteurs dans le `Makefile` et les tests, rouge joué par l'écrivain seul) ; `test_check_fails_only_on_a_named_category_present` des deux tests (forme d'un `--check`).

## Out of Scope

- Toute entrée hors famille G, dont D-44 (5c), D-62 (fixtures de trois autres tests de garde, 5c), les lignes d'exception (5d), le blocage des gardes du §5 (5e).
- Retirer ou traduire `dispatch`, les branches `pico8_` de `by-name`, `get_nu` de `header-accessors` : sans verdict, ils vont à la note.
- Toute exemption (`ENTITY_PTRS_EXEMPT`, `REGISTRIES_EXEMPT`, `REG_TYPES`, `CORE_HEADERS`) ; `tools/composition_residue_sites.txt` et `tools/reachability_questions.txt` ; le nom `void-ptr-r2`.
- Renommer les fixtures arbitraires au préfixe du cart des catégories restées (D-57 (a), D-63 (a)) ; la macro `ATT` (D-58, 5c).
- Découper `composition_residue.sh` (alerte 16, reportée) ; les outils Python de 5a.
- Lancer l'appli raylib (`CLAUDE.md`) ; régénérer une référence.

## Hypothèses sur 5a

Écrites à `39bf814`, 5a non faite. Chacune se vérifie sur la note « Découvert » de 5a et sur l'arbre de sa fin ; une hypothèse fausse fait relire les sections nommées avant d'ouvrir 5b.

- **H-5a-1** — 5a est poussée, sa note « Découvert » existe, et sa sortie tient : sur une extraction fraîche sans `warning:`, les compteurs sans `SUBSTEP` rendent exactement le tableau « Restes comptés » de la « Clôture (4l4) ». *À relire* : Condition d'entrée ; Mesure de départ ; Fin de PRD (cliquet).
- **H-5a-2** — 5a ne touche ni `tools/composition_residue.sh`, ni `tools/data_residue.sh`, ni `tools/composition_residue_sites.txt`, ni `tools/reachability_questions.txt`, ni leurs deux tests, ni `COMPOSITION_RESIDUE_GUARD`, `DATA_RESIDUE_GUARD`, les cibles `composition-guard`, `data-guard`, `composition-residue`, `data-residue` et leur légende (alertes 15 et 16 sans édition ; D-66 (c) ne porte que sur les deux gardes du §5). Les numéros de ligne de cette PRD sont ceux de `39bf814`. *À relire* : Mesure de départ ; L'alerte 14 (table) ; Orphelins des retraits ; Tests de vivacité ; Commits `refactor:`.
- **H-5a-3** — D-66 (c), construit par 5a sans bloquer : la partie texte de la garde anti-cart (noms distinctifs du TSV, mot entier) est une cible sans libclang, branchée dans `make test` au plus en compteur non bloquant ; la partie AST (noms courts, portée) est une cible séparée, non bloquante avant 5e. La partie texte ne cherche que les anciens noms des lignes `renommé` du TSV, sans détection générique du préfixe `pico8_`/`PICO8_`. *À relire* : Ce qui reste écrit en noms du cart (constat et sa mesure) ; Fin de PRD (invariant : cibles ajoutées, libclang requis ou non par `make test-all`) ; Further Notes.
- **H-5a-4** — Les refontes d'outillage de 5a (module de lecture commun pour `read_table` et `read_declarations`, alertes 1, 4, 6, 10 ; `module_of` unique, alerte 5 ; lecteur de triplets d'exceptions, alerte 9 ; variable `RENAME_TABLE := docs/renommage.tsv` du `Makefile`, alerte 12) gardent le chemin `docs/renommage.tsv`, ses colonnes (`ancien`, `nouveau`, `statut`…) et les noms et options des cibles de compteurs (`french-count`, `cart-names-count`, `cart-citations`, `cryptic-locals`, `scope-count`, `MODULES=`). 5b ne dépend d'aucun module Python. *À relire* : Mesure de départ (croisement des noms) ; Ce qui reste écrit en noms du cart (renvoi à `Names`) ; Fin de PRD.
- **H-5a-5** — 5a, première à appliquer des alertes de D-68 et à vérifier la condition d'entrée de l'étape 5, consigne la lecture des verdicts : « ok » vaut `proposition`, et les lignes vides `Verdict (un par alerte)` de D-28 et D-68 valent `proposition` pour chaque alerte, l'alerte 14 comprise (ou un humain les a remplies). *À relire* : Condition d'entrée ; L'alerte 14 ; Note « Découvert ».
- **H-5a-6** — 5a ne fixe aucune convention neuve pour marquer une entrée appliquée dans le registre (ligne « Appliqué », renvoi de commit) ; si elle en fixe une, 5b la suit pour D-5 à D-12, D-57, D-63 et l'alerte 14. *À relire* : Note « Découvert » ; Implementation Chunks (commit `docs:`).
- **H-5a-7** — L'extraction fraîche se produit par `make rename-extract` (environ 98 s) ou par le cache que D-66 aurait introduit, sans changer le format de `tmp/ast_rename/` que lisent les compteurs. *À relire* : Mesure de départ ; Fin de PRD (ordre).
- **H-5a-8** — Sous D-67 (a), 5a construit la troisième garde sans la brancher en bloquant ; `tools/french_exceptions.txt` n'a pas encore les 17 lignes de 5d. Les fixtures neuves de 5b, en anglais, ne doivent rien y ajouter. *À relire* : Tests de vivacité (choix des noms) ; Fin de PRD (cliquet `french-count`).

## Further Notes

- La sortie de l'entrée 5b dit « égal à celui d'avant moins ce que les verdicts retirent » : mesuré à `39bf814`, toutes les catégories retirées apparient 0 ligne, donc `--detail` doit rester égal à l'octet. Une différence n'est pas un retrait légitime : elle veut dire qu'un retrait a touché un motif vivant, ou que les lignes `void-ptr-r2` ont bougé.
- Garder les lignes `void-ptr-r2` à la place qu'avait `void-ptr` est un choix de cette PRD : aucun verdict ne retire la liste R2, sa première colonne est en noms anglais depuis 3j, et la déplacer changerait `--detail`. Le test `test_detail_prints_r2_sites_under_void_ptr_and_the_info_line` garde ce qu'il prouve ; son nom ne dit plus vrai (`void-ptr` n'existe plus) : il est renommé dans le commit de `composition_residue.sh` (nom de test, aucune ligne au TSV).
- Les catégories retirées laissent des mentions dans les notes « Découvert » déjà écrites et dans le registre : c'est l'historique, on n'y touche pas.
- Après 5b, plus aucune catégorie de la liste de garde ne peut être verte par aveuglement sans que sa ligne de vivacité ne le montre, sauf `dispatch` et les deux branches `pico8_` de `by-name`, nommées dans la note. C'est l'état que 5e rend bloquant.
