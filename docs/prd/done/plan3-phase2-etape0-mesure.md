---
title: "Plan 3, étape 2.0 : compteur du résidu de composition (`make composition-residue`, `tools/composition_residue.sh`), douze catégories nommées sur `src/` et `tests/`, relevé « écrit sur une instance » champ par champ pour la règle 1, mesure de départ — mesure seule, aucune garde, aucune ligne du jeu changée"
description: "La phase 2 doit remplacer l'enchâssement des structs parentes par un noyau, des archétypes `static const` et des composants. Aucune commande ne dit aujourd'hui ce qui reste à faire, et le bloc « Reste de D » renvoie neuf points à des catégories de 2.0 qui n'existent pas encore. 2.0 livre le compteur, ses tests et la mesure de départ ; la liste de la phase 2 s'écrit ensuite depuis cette mesure."
status: needs-triage
---

## Problem Statement

**Écrit sur `eb18618`** (branche `claude/plan-pilot-3-7zdft0`), en lecture seule. Seuls `make test`, `sh tools/data_residue.sh`, `grep`, `awk` et `git` ont tourné. Les comptes viennent d'un prototype jetable du scratchpad, qui n'est pas versionné. L'implémenteur les remesure avec le vrai script ; un écart s'explique dans le message de commit.

Vu du mainteneur du port : la phase D est close (Sortie atteinte à D.20b2, nettoyage D.21a-f livré). Le prochain pas du plan est **2.0**. La phase 2 (« Composition ») a une Sortie écrite en prose, mais rien ne la compte. Le bloc « Reste de D et report vers la phase 2 » rattache chacun de ses points à une catégorie de 2.0 (`nu`, `aiguillage-nom`, `un-ou-faux`, `void-ptr`, `ptrs-entité`, `noyau-inclut`, `atteinte-à-prouver`, `inclusion-mutuelle`, « lien `parent()` », « accesseurs Q6 », « défauts de classe »). Aucune de ces catégories n'existe encore. Sans compteur, on ne peut ni écrire la liste ordonnée de la phase 2, ni constater sa sortie. Et un point du Reste qu'aucune catégorie ne compte sortirait de la phase sans bruit (revue de sortie de D, M3).

### Texte du plan, recopié (`docs/plan-3.md`, § « Phase 2 — Composition »)

> Étape **2.0 — mesure seule** (sur le modèle de D.0 et 1b.0), avant toute liste de la phase : un compteur à catégories nommées — struct parente enchâssée en premier membre ; lien `parent()` et aides `pico8_X_of` ; accesseurs Q6 ; défauts de classe pas encore `static const` ; pour la règle 1, la preuve grep « écrit sur une instance ou non », champ par champ. Catégories ajoutées à la revue de sortie de D (chacune avec son motif énoncé et sa liste de sites, comme `data-residue --detail`) : `nu` (tags `nu` pas encore en masque de bits) ; `aiguillage-nom` (`hp_pose` : nom → membre à la construction) ; `un-ou-faux` (`pk`/`pk_is_bool`, `fp_is_bool`) ; `void-ptr` (prototypes publics qui prennent ou rendent une entité par `void *`, hors `*_dispatch`, hors accesseurs Q6 déjà comptés, hors `void *ctx` des parcours ; avec, en détail, les 11 sites R2 de D.20b et leur plus proche `void *` en amont) ; `ptrs-entité` (listes membres d'entités sur `pico8_ptrs_t`, ajout non typé) ; `noyau-inclut` (includes de données de jeu dans l'en-tête du noyau, aujourd'hui `struct.h` → `game_data.h`) ; `atteinte-à-prouver` (questions d'atteinte ouvertes, une ligne par question dans une liste versionnée, aujourd'hui `pj:hb` avec D.18c M6/M7 ; une ligne sort par une entrée TROU et son test, ou par `tools/line_coverage_unreachable.txt`) ; `inclusion-mutuelle` (paires de modules qui s'incluent, **mesurée seulement** : ni garde ni Sortie). La sortie vide du compteur, hors `inclusion-mutuelle`, **est** la Sortie de la phase ; chaque catégorie vidée entre dans une garde de `make test`, comme `DATA_RESIDUE_GUARD`. La liste ordonnée de la phase 2 est écrite depuis cette mesure ; les deux groupes de Q7 y sont nommés comme devant passer le test ponctuel de la règle 3.

La Sortie de la phase, pour la frontière :

> - **Sortie** : aucune struct d'entité n'enchâsse plus la struct de sa parente (tout type = noyau + archétype + composants) ; chaque ancienne classe a son archétype `static const` ; le test ponctuel est passé pour chacune avant d'être supprimé.

Règles du §7 qui touchent 2.0 : « En D et en 2 », `make class-coverage-net` aux étapes dont le diff touche un constructeur, `pico8_class_coverage_record` ou `KNOWN[]` ; `make sanitize-net` aux étapes qui changent la durée de vie d'une donnée. **2.0 ne touche ni l'un ni l'autre** : aucune des deux cibles n'est exigée. §3 : `make test` vert, `make e2e` **26 conformes**, aucun `*.sha256` modifié.

### Ce que 2.0 livre, et ce qu'elle ne livre pas

- **Livre** : le compteur (script et cible `make composition-residue`, hors suite), ses douze catégories avec leur motif, leurs exclusions nommées et leurs listes de sites, le relevé champ par champ de la règle 1 (`--champs`), deux listes versionnées (questions d'atteinte, sites R2), son TU dans `UNIT_TESTS`, la mesure de départ dans une note « Découvert à 2.0 ».
- **Ne livre pas** : aucune garde (aucune catégorie n'est à 0 sur `eb18618`, donc rien à garder) ; aucune ligne de `src/` ; aucune liste ordonnée de la phase 2 (voir « Après 2.0 »).

### État de départ (`eb18618`)

| Commande | Résultat |
|---|---|
| `make test` | exit 0 en 5,6 s ; **70** binaires « all tests passed » ; `offsetof dans src/ : 0` ; `résidu gardé (spec,overrides,stash,ob,dispatch,par-nom,liste,ref-audit,valeur,type-tag) : 0 ligne(s)` ; `float-grep coeur 116 exclus 33 lignes 0` ; `garde fichiers 57 en-echec 0` |
| `sh tools/data_residue.sh` | `résidu D : 0 ligne(s), 0 fichier(s)`, code 0 |
| `make e2e` | non lancé pour ce PRD ; 26 conformes à D.21f (note du plan) |

`src/` compte 149 fichiers, sans sous-dossier. `tests/` n'a aucun `.h`. `src/pico8_game_data.h` fait **574 lignes**, `src/pico8_game_data.c` 725. `tools/line_coverage_unreachable.txt` est vide.

## Solution

Trois livrables, sans aucune ligne de `src/` changée :

1. **Le compteur** `tools/composition_residue.sh` et la cible `make composition-residue` (hors suite). Il reprend la forme de `tools/data_residue.sh` : mêmes modes (`--detail`, `--check`), même unité (la ligne de `.c`/`.h`, commentaires compris), même code 2 sur une mesure sur rien. Il ajoute un mode `--champs`, le relevé de la règle 1.
2. **Deux listes versionnées**, lues par le compteur :
   - `tools/atteinte_a_prouver.txt` : une ligne par question d'atteinte ouverte ;
   - `tools/composition_r2_sites.txt` : les 11 sites R2 de D.20b avec leur plus proche `void *` en amont.
3. **Le TU** `tests/test_composition_residue.c`, dans `UNIT_TESTS`, sur un mini-arbre fabriqué, comme `tests/test_data_residue.c`. Chaque motif y mord sur une ligne témoin et ignore ses faux positifs.

La note « Découvert à 2.0 » donne, dans le plan, la commande, les catégories et leurs comptes de départ.

## User Stories

1. En tant qu'auteur de la liste de la phase 2, je veux le nombre de lignes et de fichiers de chaque catégorie, afin de nommer un groupe par catégorie et de le découper au-delà de ~40 fichiers.
2. En tant qu'auteur de la liste, je veux la liste des fichiers de chaque catégorie avec leur compte (`--detail`), afin de mesurer l'empreinte de chaque groupe.
3. En tant qu'auteur de chaque ligne de la liste, je veux un « Compte : N » calculable par une commande, afin que chaque étape déclare le compte attendu après elle.
4. En tant que `plan-pilot`, je veux que la sortie vide du compteur soit la Sortie de la phase 2, `inclusion-mutuelle` exclue, afin de constater la fin de phase par une commande.
5. En tant que `plan-pilot`, je veux que l'ensemble des catégories qui comptent pour la Sortie tienne dans une seule variable du script, afin que la décision humaine (catégories nouvelles en condition de sortie ou non) change une ligne et pas le compteur.
6. En tant qu'auteur d'une étape qui vide une catégorie, je veux un mode `--check <catégories>` qui sort 1 sur une occurrence, afin de la garder dans `make test` le jour où elle tombe à 0, sans réinventer la garde.
7. En tant qu'auteur d'une étape « archétype », je veux pour chaque (type, membre) le nombre d'écritures hors copie de défaut, et combien sont hors constructeur, afin d'appliquer la règle 1 avec une preuve par grep.
8. En tant que lecteur du Reste de D, je veux que chacun de ses points soit compté par une catégorie nommée, afin qu'aucun ne sorte de la phase sans bruit.
9. En tant que gardien de la perte acceptée à D.20b, je veux voir les 11 sites R2 avec leur plus proche `void *` en amont, afin de savoir lesquels ne sont protégés que par l'absence d'un appelant non typé.
10. En tant que gardien de la même perte, je veux compter les listes membres d'entités sur `pico8_ptrs_t`, afin que leur ajout non typé ait un compte qui baisse.
11. En tant que responsable des questions d'atteinte, je veux une liste versionnée, une ligne par question, afin qu'une question ne sorte que par une entrée TROU et son test, ou par une ligne de `tools/line_coverage_unreachable.txt`.
12. En tant que lecteur de la revue de mi-D, je veux le nombre de paires de modules qui s'incluent mutuellement, mesuré à 2.0 et à la sortie de la phase, afin de voir si la phase 2 en ajoute.
13. En tant que mainteneur, je veux que chaque motif et chaque exclusion soient écrits dans ce PRD et dans le script, afin que n'importe qui recompte à la main.
14. En tant que mainteneur, je veux une sortie stable (ordre fixe des catégories, fichiers triés en `LC_ALL=C`), afin que deux passes sur le même arbre donnent le même texte.
15. En tant que vérificateur, je veux un code 2 sur une mesure sur rien (dossier absent, aucun `.c`/`.h`, catégorie inconnue, liste versionnée absente), afin qu'un compteur cassé ne passe pas pour « vide ».
16. En tant que vérificateur, je veux un TU où chaque motif mord sur un témoin et ignore chacun de ses faux positifs nommés, afin que les « Compte : N » de la liste soient fiables.
17. En tant que vérificateur, je veux que le TU du compteur ne compte dans aucune catégorie, ni de ce compteur, ni de `data_residue.sh`, afin qu'il ne rende aucune des deux sorties inatteignable.
18. En tant que gardien de l'invariant §3, je veux qu'aucun fichier de `src/`, aucun `*.sha256` et aucun e2e ne change, afin que 2.0 reste une mesure pure.
19. En tant que lecteur du `Makefile`, je veux `composition-residue` dans la légende de tête, marquée « hors suite », et la ligne `test:` inchangée.
20. En tant que `make test`, je veux que le TU du compteur coûte moins de quelques secondes, afin que la boucle rapide reste rapide.

## Implementation Decisions

Pas d'humain dans la boucle : les choix que la skill `to-prd` ferait valider sont tranchés ici comme le ferait un agent `guideline`, et marqués **(G)**.

### Unité de compte (G)

- **Une ligne de fichier `.c`/`.h`** qui porte le motif de la catégorie, commentaires et chaînes compris, comme `data_residue.sh`. Un commentaire qui décrit la chaîne d'héritage est du résidu : il deviendra faux.
- **Les catégories se chevauchent.** Par exemple, `pico8_jo_get_nu` compte dans `accesseurs-q6` et dans `nu`, et `pico8_jo_set_m_` dans `accesseurs-q6` et dans `void-ptr`. Chaque ligne de catégorie compte ses lignes ; le pied compte les lignes distinctes (couples `fichier:ligne`).
- **Deux catégories ne comptent pas des lignes de `src/`/`tests/`** :
  - `atteinte-à-prouver` compte les lignes utiles de sa liste versionnée (`tools/atteinte_a_prouver.txt:<n>`) ;
  - `inclusion-mutuelle` compte des paires de modules, et reste **hors du pied**.
- **Ensemble de sortie** (G) : une variable du script, `SORTIE`, liste les catégories comptées dans le pied. Par défaut, ce sont les onze catégories hors `inclusion-mutuelle`, comme le dit le plan. Si l'humain décide que les catégories nouvelles sont seulement mesurées, ou que `inclusion-mutuelle` devient un objectif, l'étape qui écrit la liste change cette ligne. Le compteur, ses modes et son TU ne changent pas.
- **Dossiers lus** : `src tests` par défaut, chemins imprimés tels que reçus, comme `data_residue.sh`.

### Ensemble E des types d'entité (partagé par `parente`, `ptrs-entité` et `--champs`)

Un lecteur `awk` parcourt les corps de struct de premier niveau (ceux qui s'ouvrent par une ligne `struct … {` hors de tout autre corps). Pour chacun, il relève :

- le premier membre déclaré (lignes vides et commentaires sautés) et son type ;
- le nom du type : l'étiquette `struct X` et le nom du `typedef` de la ligne `} nom;` ; les deux entrent dans E.

E est la fermeture : un type est dans E si son premier membre est `pico8_struct_header_t` ou un type de E. Sur `eb18618`, E compte **45 types**, dont `pico8_ke_t`, l'arbre `jo`, les classes d'interface et quatre faux de TU (`fake_toast_t`, `giver_t`, `ra_only_t`, …). `pico8_struct_header_t` y figure comme germe.

### Les douze catégories

Ordre de sortie fixe, celui du tableau. Les motifs sont des `grep -E` (GNU, `\b` accepté, comme `data_residue.sh`). Une exclusion nommée est un `sed` appliqué avant le `grep`, ou une règle de lecture, jamais un filtre sur la ligne entière. Même raison que `SPEC_EXCLUSION` : un filtre sur la ligne entière cacherait une vraie occurrence voisine. Les comptes sont `lignes / fichiers` sur `eb18618`.

| # | Catégorie | Motif | Exclusions nommées (faux positifs connus) | Compte |
|---|---|---|---|---|
| 1 | `parente` | lecteur E : premier membre d'un corps de struct, de type ∈ E et ≠ `pico8_struct_header_t`. Recompte à la main, exact sur `eb18618` : `^[[:space:]]+pico8_(ke\|jo\|qh\|hj\|nr\|g_)_t[[:space:]]+[a-z0-9_]+;` | (a) `pico8_struct_header_t header;` en premier membre : c'est le noyau, pas une parente. (b) une variable locale d'entité par valeur dans un corps de fonction : ce n'est pas un corps de struct. (c) un membre pointeur `pico8_jo_t *jo;`. (d) un membre de type E qui n'est pas le premier. | **26 / 17** (24 `src/`, 2 faux de TU) |
| 2 | `lien-parent` | `\bpico8_[a-z0-9_]+_of\b\|\b[a-z0-9_]*(emitter\|probe)_of\b\|(->\|\.\|\(\*)parent\b` | `LIEN_EXCLUSION='s/pico8_state_of//g'` : nom d'état → enum, pas une lignée. Le mot `parent` en prose (« the parent struct ») ne compte pas, faute de `->`, `.` ou `(*` devant. | **111 / 27** |
| 3 | `accesseurs-q6` | `(\b\|_)(get_(nu\|fa\|ra\|p\|kq\|ql\|fg\|mt\|fx\|p_)\|set_m_)\b` : les onze membres Q6 de `pico8_struct_class_t`, leurs implantations `pico8_{jo,g_}_get_*`, `X_get_*` et leurs appels | Un mot qui contient `get_…` sans frontière (`budget_p`) ; un suffixe plus long (`get_pa`, `get_p_x`). | **262 / 34** |
| 4 | `défauts` | `\bpico8_[a-z0-9_]+_start(_t)?\b` : les enregistrements de défauts propres (`pico8_X_start`, `pico8_X_start_t`, `pico8_jo_kw_start`, `pico8_pj_cr_start`), que les `pico8_X_struct_init` recopient le long de la chaîne | `_start` sans préfixe `pico8_` : `run_start`, `line_start`, `obj_start`, `test_start`. | **701 / 52** |
| 5 | `nu` | `\bnu\b\|get_nu\b\|_nu_(tags\|items)\b` | `NU_EXCLUSION='s/\\"nu\\"//g'` : `"nu"` échappé dans un littéral de fixture (`tests/test_data_residue.c:108`, seule occurrence). Mots qui commencent par `nu` (`number`, `numeric`, `pico8_pi_number`). | **208 / 37** |
| 6 | `aiguillage-nom` | `\bhp_(pose\|ho)\b\|\bpico8_ho_gp_has\b\|\bpico8_hp_(mr\|gp)_names\b\|(->\|\.)(mr\|gp)\b` | `mr`/`gp` en mot nu (paramètre, variable). Aucun autre membre `mr`/`gp` n'existe sur `eb18618` ; un nouveau serait compté à tort, et le recompte de chaque étape le verrait. | **30 / 8** |
| 7 | `un-ou-faux` | `\b(pk\|fp)_is_bool\b` | Hors motif, par nom, et tranchés en D : `qe_is_bool` (`trade`, valeur à trois genres portée par `pico8_qe_t`, D.17), `bg_is_bool` (`lq`, D.16b), `h_is_false` (clés de `js`). | **27 / 14** |
| 8 | `void-ptr` | lecteur de prototypes : dans les `.h` lus, une déclaration de fonction de premier niveau, **jointe jusqu'à son `;`** (un prototype sur plusieurs lignes compte à sa première ligne), qui porte `\bpico8_[a-z0-9_]+\(` et `void \*` | (a) `VOID_EXCLUSION='s/void \*+(r_)?ctx\b//g'` : les `void *ctx` des parcours. (b) nom en `_dispatch(`. (c) nom en `_get_…(` : compté par `accesseurs-q6`. (d) `typedef` de pointeur de fonction. (e) `pico8_ptrs.h` : le conteneur lui-même (Q9). (f) membres de `pico8_struct_class_t` et `new_entity` de `pico8_entity_kind_t` : ce sont des membres, pas des prototypes. | **11 / 7** |
| 9 | `ptrs-entité` | lecteur E : dans un corps de struct de type ∈ E, un membre `^[[:space:]]+(const[[:space:]]+)?pico8_ptrs_t[[:space:]]*\*?[[:space:]]*[a-z0-9_]+;` | `PTRS_NON_ENTITE`, liste nommée du script (`type.membre`, chacun avec l'écrivain qui le prouve) : `pico8_hy_t.invented` (chaînes, `economy.c:284`), `pico8_jo_t.jy` (entrées, `economy_tiers.c:167`), `pico8_hp_t.i` (enregistrements `jy`, `research.c:233`), `pico8_jc_t.ts` (particules, `slipway_display.c:43`). Hors motif par la règle : les registres `hv`/`cv`/`cg` (statiques d'`entity.c`, écrivain unique garanti par l'édition de liens, revue de sortie de D) ; les locales ; les membres de structs hors E (`emitter.qy`, clés de `trade`, `proposal.ib`). | **7 / 4** |
| 10 | `noyau-inclut` | dans les fichiers nommés `pico8_struct.h`, `pico8_ke.h` ou `pico8_entity.h` (liste `NOYAU`) : `^#include "pico8_(game_data\|map_tables\|data_init)\.h"` | Un include qui n'est pas de données (`pico8_coverage.h`, `pico8_vec.h`, `pico8_ptrs.h`, `pico8_scroll.h`) ; un include de données hors du noyau (26 fichiers de `src/` incluent `pico8_game_data.h`). | **1 / 1** |
| 11 | `atteinte-à-prouver` | lignes de `tools/atteinte_a_prouver.txt` qui portent `^[^#[:space:]]` | Lignes vides, lignes `#`. | **1 / 1** (le fichier naît à 2.0) |
| 12 | `inclusion-mutuelle` | paires `{a, b}` de modules de `src/` (module = nom de fichier sans extension) telles que `a.c` ou `a.h` porte `^#include "b\.h"` et `b.c` ou `b.h` porte `^#include "a\.h"` | Includes système `<…>` ; un module qui inclut son propre en-tête ; `tests/`. | **21 paires / 21 modules** ; hors pied |

**Exemples** (une ou deux lignes par catégorie, `eb18618`) :

- `parente` : `src/pico8_planet_family.h:51:    pico8_qh_t qh;` (`nl` enchâsse `qh`) ; `src/pico8_help.h:21:    pico8_g__t g_;`.
- `lien-parent` : `src/pico8_planet_family.c:50:    .parent = pico8_jo_struct_class,` ; `src/pico8_economy_tiers.c:264:    if (pico8_nl_of(pt) != NULL) {`.
- `accesseurs-q6` : `src/pico8_struct.h:99:    const pico8_vec_t *(*get_ra)(const void *self);` ; `src/pico8_planet.h:118:const pico8_vec_t *pico8_jo_get_ra(const void *self);`.
- `défauts` : `src/pico8_planet_family.c:158:    self->rr = pico8_nl_start.rr;` ; `src/pico8_game_data.h:536:extern const pico8_nl_start_t pico8_nl_start;`.
- `nu` : `src/pico8_planet.c:80:    return ((const pico8_jo_t *)self)->nu;` ; `src/pico8_game_data.h:461:extern const pico8_str_seq_t pico8_nl_nu_tags;`.
- `aiguillage-nom` : `src/pico8_research.c:129:        hp_pose(self, p, s, is_fixed, fixed, i_src);` ; `src/pico8_research.c:118:        if (s != NULL && pico8_ho_gp_has(self->gp, name)) {`.
- `un-ou-faux` : `src/pico8_probe.c:372:        pk = emitter->pk_is_bool ? emitter->pk != 0 : 1;` ; `src/pico8_slipway.c:415:    self->probe->fp_is_bool = 1;`.
- `void-ptr` : `src/pico8_probe.h:180:pico8_qy_t *pico8_g__mm(void *self);` ; `src/pico8_trade.h:63:int pico8_trade_to(const pico8_trade_t *self, void **out);`.
- `ptrs-entité` : `src/pico8_planet.h:43:    pico8_ptrs_t *oe;` ; `src/pico8_wormhole.h:36:    pico8_ptrs_t *ox;`.
- `noyau-inclut` : `src/pico8_struct.h:7:#include "pico8_game_data.h"`.
- `atteinte-à-prouver` : la ligne `pj-hb-nil` de la liste (plus bas).
- `inclusion-mutuelle` : `pico8_planet <-> pico8_planet_display` ; `pico8_entity <-> pico8_ke`.

**Listes de sites connues à l'écriture** (l'implémenteur les vérifie par `--detail`) :

- `void-ptr` (11) : `pico8_gu_ptrs` (`collect.h:19`), `pico8_hd_true` (`entity.h:47`), `pico8_ha_pz` (`menu.h:25`, rend l'entité), `pico8_jo_set_m_` (`planet.h:123`), `pico8_qh_probe_of` (`planet_family.h:43`), `pico8_hj_emitter_of` (`:79`), `pico8_ga_mm` (`:212`), `pico8_g__emitter_of`, `pico8_g__probe_of`, `pico8_g__mm` (`probe.h:62`, `:63`, `:180`), `pico8_trade_to` (`trade.h:63`, rend l'entité par `void **`). Avec les 13 `pico8_{jo,g_}_get_*` comptés par `accesseurs-q6` (7 `jo`, 6 `g_`), cela fait **24** prototypes, et non les 25 de la revue de sortie. Le plan dit que « 2.0 en fixe la liste exacte par grep » : l'écart se déclare au commit, avec son explication probable (le `void *r_ctx` de `pico8_gn`, exclu comme `ctx`, ou le membre `new_entity`).
- `ptrs-entité` (7) : `jo.oe`, `jo.cc`, `jo.by` (`planet.h:43`, `:58`, `:59`), `trade.lp`, `trade.rl` (`trade.h:12-13`), `ui.le` (`ui.h:35`), `mz.ox` (`wormhole.h:36`). `jc.qu`, cité par le Reste, est un `pico8_struct_header_t *`, déjà typé : ce n'est pas une liste.
- `parente` (26) : `ke` enchâssé par `bg`, `nf`, `mh`, `iv`, `jo`, `g_`, `ie`, `pj`, `qb`, `jc`, `ui`, `js`, `fq`, `mz` et deux faux de TU (`test_pico8_iv.c:172`, `test_pico8_ui.c:718`) ; `jo` par `qh` et `ga` ; `qh` par `nl`, `os`, `hj`, `nr` ; `hj` par `pm`, `mx` ; `nr` par `fr` ; `g_` par `fv`.
- `inclusion-mutuelle` (21) : `data_init`↔`research`, `economy`↔`planet`, `economy_formulas`↔`slipway`, `entity`↔`ke`, `entity_geom`↔`planet`, `particle`↔`probe`, `planet`↔{`planet_display`, `planet_family`, `planet_trade`, `probe`, `slipway`}, `planet_display`↔`planet_family`, `probe`↔{`slipway`, `ui`, `wormhole`}, `research`↔`research_display`, `score_panel`↔`score_panel_display`, `slipway`↔{`slipway_display`, `ui`, `wormhole`}, `wormhole`↔`wormhole_display`. Parmi elles, **5** paires logique ↔ `*_display`, comme l'a relevé la revue de mi-D.

**Pied sur `eb18618`** (prototype) : **≈ 1 318 lignes distinctes dans ≈ 84 fichiers**, soit 1 317 dans 83 fichiers de `src/`/`tests/`, plus la ligne de la liste d'atteinte.

### Lectures du plan, tranchées ici (G)

- **« Défauts de classe pas encore `static const` »** : depuis D.5, les défauts propres de chaque classe sont déjà `static const` (`pico8_X_start`). Ce qui manque, c'est l'**archétype** : un défaut résolu une fois le long de la chaîne (`NL_DEF = { .hp=6, .s=68, … }`). Aujourd'hui, chaque `pico8_X_struct_init` le recompose à l'exécution depuis les `pico8_X_start` de sa lignée. La catégorie compte donc les `pico8_X_start`. Un archétype devra porter un nom hors motif (par exemple `pico8_X_arch`), comme la règle de D l'imposait aux noms nouveaux. Le nombre de lignes de `src/pico8_game_data.h` (574) est rapporté par `--detail`, sur une ligne `info`.
- **`nu`** : le motif suit le nom du champ. Un masque de bits qui garderait le nom `nu` ne ferait pas tomber le compte. Le membre converti devra donc changer de nom. Le glossaire donne le sens de `nu` (« étiquettes d'une classe ») ; c'est une question pour l'humain (Q4).
- **`un-ou-faux`** : il ne compte que l'indicateur (`pk_is_bool`, `fp_is_bool`), pas `pk` ni `fp`. Quand l'indicateur part, la valeur reste, sous sa forme B1 (entier vrai/faux).
- **`void-ptr`** : « prototypes publics » = déclarations de fonctions de premier niveau dans les `.h` lus. `pico8_jo_set_m_` est compté par `accesseurs-q6` (membre Q6) **et** par `void-ptr` (le Reste de D le range là) : le chevauchement est voulu. Les membres de la table de méthodes (`void *self`) ne sont pas des prototypes. Ils apparaissent comme « `void *` en amont » dans la liste R2.
- **Liste R2** : `tools/composition_r2_sites.txt`, versionnée, **11 lignes** `fonction<TAB>fichier<TAB>plus proche void * en amont<TAB>preuve`. L'implémenteur l'établit en lisant les appelants. `--detail` l'imprime sous `void-ptr` (lignes `void-ptr-r2`) ; elle n'entre pas dans le compte. Les onze sites (fonctions à `ca68736`, table de D.20b) :
  - `entity.c` : `pico8_bm_has`, `pico8_dc` (boucle sur `cv`) ;
  - `ke.c` : `pico8_ke_ks` ;
  - `particle.c` : `pico8_f_`, `pico8_ez`, `pico8_ei` ;
  - `probe.c` : `pico8_bv`, `pico8_dl`, `pico8_d_` ;
  - `score_panel.c` : `pico8_eg_gg_pred` ;
  - `slipway.c` : `pico8_jc_hq` (`self->probe->et`).

  Candidats à vérifier, qui sont des hypothèses : les `*_dispatch(void *self)` (`pico8_ei_dispatch`, `pico8_bv_dispatch`, `pico8_dl_dispatch`, `pico8_d__dispatch`) ; le prédicat `pico8_jp_ptrs_predicate_fn(void *e, …)` pour `eg_gg_pred` ; « aucun » quand toute la chaîne est typée (`bm_has`, `dc`).
- **Liste d'atteinte** : `tools/atteinte_a_prouver.txt`, une ligne par question, `id<TAB>question<TAB>sites<TAB>sortie attendue`. Contenu initial, **une ligne** :
  - id `pj-hb-nil` ;
  - question : `pj:hb` écrit nil dans `pj.ou` là où le `pairs` du cart saute la valeur (peut-être aussi via `ef[fm.lo]` avec un `lo` fractionnaire, contre le `flr` du port) ; avec elle, le `false` ou le nil de `pj.ou` lu par `pico8_pi_is_number` (D.18c M6/M7), jamais atteint ;
  - sites : `score_panel.c` `pico8_pj_hb`, `pi.c` `pico8_pi_is_number` ;
  - sortie attendue : atteinte prouvée → entrée TROU et test ; inaccessible prouvée → ligne de `tools/line_coverage_unreachable.txt`.

  Le compteur ne juge pas le contenu : il compte les lignes.

### Le relevé `--champs` (règle 1)

Pour chaque membre des types de E déclarés dans les `.h` de `src/`, faux de TU exclus, le relevé saute :

- le membre `header` ;
- le premier membre de type E (la parente enchâssée) : ce n'est pas un champ ;
- les membres de pointeurs de fonction.

Il imprime ensuite `<type>\t<membre>\t<écritures>\t<hors constructeur>`, trié, puis le pied `champs : N ; 0 écriture : K ; 0 hors constructeur : M`. Les colonnes se lisent ainsi :

- **Écriture** : une ligne de `src/*.c` qui porte `(->|\.)<m>[[:space:]]*(=[^=]|\+=|-=|\*=|/=|\+\+|--)` ou `(\+\+|--)[A-Za-z0-9_>.()-]*(->|\.)<m>\b`. La copie d'un défaut de même nom (`=[[:space:]]*pico8_[a-z0-9_]+_start\.<m>\b`) est retirée par `sed` avant le compte.
- **Hors constructeur** : l'écriture est dans une fonction dont le nom ne finit ni par `_struct_init`, ni par `_new`, ni par `_init`. Le nom est celui de la dernière définition de premier niveau (`^[a-z].*\b<nom>\(…\)[[:space:]]*\{$`), jusqu'à la `}` de colonne 0.

Le relevé ne sort jamais en erreur sur un compte ; il n'a pas de `--check`. Sur `eb18618` (prototype) : **286 champs**, **15** à 0 écriture, **27** à 0 hors constructeur. La recherche par nom est un **majorant** : un membre court (`s`, `p`, `ra`, `fa`) est compté pour tous les types qui le portent (`qh.s` : 119). Qu'elle tranche si peu de champs est un constat pour la liste, et la question Q3 pour l'humain.

### Sortie du compteur, alignée sur `data_residue.sh`

- **Sans option** : `<cat>\t<lignes>\t<fichiers>` pour chaque catégorie non nulle, dans l'ordre du tableau. Pour `inclusion-mutuelle`, c'est `<paires>\t<modules>`. Puis le pied `résidu 2 : N ligne(s), M fichier(s)`, calculé sur les catégories de `SORTIE`. Code 0 quel que soit le compte.
- **Vide** (Sortie de la phase, avec `SORTIE` par défaut) : aucune ligne de catégorie de `SORTIE`, et le pied `résidu 2 : 0 ligne(s), 0 fichier(s)`. Une ligne `inclusion-mutuelle` peut rester au-dessus du pied : elle n'est pas du résidu.
- **`--detail`** :
  - `<cat>\t<fichier>\t<lignes>` par couple non nul, trié par catégorie puis par fichier ;
  - pour `inclusion-mutuelle`, `inclusion-mutuelle\t<a>\t<b>`, une ligne par paire, `a < b` ;
  - sous `void-ptr`, les 11 lignes `void-ptr-r2\t<fonction>\t<amont>` ;
  - la ligne `info\tsrc/pico8_game_data.h\t<n> lignes` ;
  - puis le même pied. Code 0.
- **`--check <cat>[,<cat>…]`** : `<cat>: <fichier>:<ligne>:<texte>` par occurrence (pour `inclusion-mutuelle` : `inclusion-mutuelle: <a> <-> <b>`), puis `résidu gardé (<cats>) : N ligne(s)`. Code 1 si N > 0. Toutes les catégories sont acceptées, pour que la décision humaine sur `inclusion-mutuelle` n'exige pas de changer le script.
- **`--champs`** : le relevé ci-dessus. Code 0.
- **Code 2** dans chacun de ces cas, pour qu'une mesure sur rien ne passe pas pour vide :
  - option ou catégorie inconnue, catégorie vide dans la liste de `--check` ;
  - dossier absent, aucun `.c`/`.h` ;
  - liste d'atteinte ou liste R2 absente ;
  - échec de `grep`, `sed` ou `awk`.
- **Chemins des listes** : `tools/atteinte_a_prouver.txt` et `tools/composition_r2_sites.txt` par défaut, remplaçables par les variables `COMPOSITION_ATTEINTE` et `COMPOSITION_R2` (le TU s'en sert).

### Ce qui ne change pas

- Aucun fichier de `src/`, aucun test existant, aucun `*.sha256`, aucun fichier de `tests/e2e/`.
- La ligne `test:` du `Makefile`, `data-guard`, `DATA_RESIDUE_GUARD`, `tools/data_residue.sh`.
- **Aucune garde** : pas de variable `COMPOSITION_RESIDUE_GUARD`, pas de cible `composition-guard`. La première catégorie qui tombe à 0 crée la garde, dans son commit, comme D l'a fait.
- Fichiers ajoutés ou modifiés : `tools/composition_residue.sh`, `tools/atteinte_a_prouver.txt`, `tools/composition_r2_sites.txt`, `tests/test_composition_residue.c`, `Makefile` (légende, cible `composition-residue` et son `.PHONY`, règle du TU, une entrée de `UNIT_TESTS`), `docs/plan-3.md` (note « Découvert à 2.0 »).

### Commentaires

Le script ne garde que ce que `CLAUDE.md` admet : l'usage en tête (contrat des modes et des codes, comme `data_residue.sh`) et, pour chaque exclusion, le piège qu'elle évite (moins de 40 mots). Le TU dit, comme `test_data_residue.c`, pourquoi ses jetons sont coupés.

## Implementation Chunks

Trois commits verts, chacun d'environ 50 minutes. `prd-autopilot` fait un commit par chunk.

1. **2.0a — squelette et catégories grep** (≈ 50 min).
   - Livre : le script avec ses modes sans option, `--detail` et `--check`, les codes 2 et `SORTIE` ; les catégories `lien-parent`, `accesseurs-q6`, `défauts`, `nu`, `aiguillage-nom`, `un-ou-faux` et `noyau-inclut`, avec leurs exclusions ; la cible `composition-residue` et sa légende ; le TU (cas 1 à 4, 8 et 9 ci-dessous, restreints à ces catégories).
   - Compte attendu : les sept lignes du tableau.
2. **2.0b — catégories structurelles et listes** (≈ 50 min).
   - Livre : le lecteur E, puis `parente`, `ptrs-entité` et `PTRS_NON_ENTITE` ; le lecteur de prototypes et `void-ptr` ; les deux listes versionnées (R2 établie en lisant les appelants) ; `atteinte-à-prouver` ; `inclusion-mutuelle` hors pied ; les lignes `info` et `void-ptr-r2` de `--detail` ; le TU (cas 5 à 7, et les témoins des catégories nouvelles).
   - Compte attendu : le pied complet.
3. **2.0c — relevé `--champs` et note** (≈ 40 min).
   - Livre : `--champs` et son cas de TU ; la note « Découvert à 2.0 » dans le plan ; les greps de Sortie ; `make test` ; `make e2e`.

Repli (écrit, comme à D.20b) : une sous-étape arrivée au plafond sans commit ne commet rien. `plan-pilot` la relance coupée en deux, sans pont. 2.0b se coupe en {E, `parente`, `ptrs-entité`} puis {`void-ptr`, les listes, `inclusion-mutuelle`}.

## Testing Decisions

Un bon test regarde le comportement externe du compteur : texte de sortie et code de retour sur un arbre donné. Il ignore comment le script découpe son travail. Modèle : `tests/test_data_residue.c`.

- mini-arbre sous `tmp/test_composition_residue_work/{src,tests}` ;
- le script lancé par `system()`, le code relu depuis un fichier (`echo $? > code`, hors `<sys/wait.h>`) ;
- `assert` sur le texte exact.

### Le TU `test_composition_residue`

**Auto-exclusion.** Le script lit `tests/`. Chaque jeton de fixture est donc coupé en deux littéraux (`"pico8_" "qh_t qh;"`, `"get" "_ra"`, `"n" "u"`), faute de quoi ce fichier entrerait dans le résidu qu'il mesure. Il ne doit porter non plus aucun jeton des catégories de `data_residue.sh` (`pico8_value_`, `overrides`, `field_truthy`, `pico8_list_`…), puisque `make test` les garde dans `tests/`.

Cas (chacun : témoin qui mord, puis faux positifs nommés qui ne mordent pas) :

1. **Arbre propre** : la seule ligne `résidu 2 : 0 ligne(s), 0 fichier(s)` (listes d'atteinte et R2 fournies, vides), code 0.
2. **Un témoin par catégorie, dans l'ordre du tableau**, dont une paire `a.h`↔`b.h` pour `inclusion-mutuelle`. Sortie attendue au texte près ; le pied compte 11 lignes, sans la paire.
3. **Faux positifs grep**, chacun seul dans un fichier, pied vide attendu :
   - `lien-parent` : `pico8_state_of(me)` ; `/* the parent struct */` ;
   - `accesseurs-q6` : `int budget_p;` ; `int get_pa;` ;
   - `défauts` : `int run_start;` ;
   - `nu` : `int number;`, `pico8_pi_number(e, 1);`, `"jo_default(\"nu\", &k);"` (la fixture de `test_data_residue.c`, reproduite) ;
   - `aiguillage-nom` : `int mr = 0;` ;
   - `un-ou-faux` : `self->qe_is_bool = 0;`, `self->bg_is_bool = 0;`, `k.h_is_false = 1;` ;
   - `noyau-inclut` : `#include "pico8_vec.h"` dans `pico8_struct.h`, et `#include "pico8_game_data.h"` dans `pico8_planet.h`.
4. **Exclusion par `sed`, pas par ligne** : une ligne qui porte `pico8_state_of` **et** `pico8_qh_of` compte 1 dans `lien-parent` ; une ligne qui porte `\"nu\"` **et** `->nu` compte 1 dans `nu`.
5. **Lecteur E et `parente`** :
   - mordent : un `pico8_ke_t` à premier membre `header` ; un `pico8_jo_t` à premier membre `pico8_ke_t ke;` ; un petit-enfant `pico8_qh_t` à premier membre `pico8_jo_t jo;` (fermeture) ; un type à étiquette seule (`struct pico8_mh { pico8_ke_t ke; … };`) ;
   - ne mordent pas : un `pico8_jo_t *jo;` membre ; un `pico8_jo_t jo;` second membre ; `pico8_jo_t jo;` comme variable locale d'une fonction ; `pico8_struct_header_t header;` en premier membre ; une struct hors E dont le premier membre est `pico8_fixed_t`.
6. **`ptrs-entité`** : `pico8_ptrs_t *oe;` dans `pico8_jo_t` mord. Ne mordent pas : `pico8_ptrs_t *jy;` dans `pico8_jo_t` (liste `PTRS_NON_ENTITE`) ; `pico8_ptrs_t *qy;` dans une struct hors E ; `pico8_ptrs_t list;` locale.
7. **`void-ptr`** :
   - mordent : `pico8_qy_t *pico8_g__mm(void *self);` ; `const void *pico8_ha_pz(void);` ; un prototype sur deux lignes dont le `void *` est sur la seconde (compté à la première) ;
   - ne mordent pas : `void pico8_ei_dispatch(void *self, …);` ; `int pico8_jo_get_fx(const void *self);` (compté par `accesseurs-q6` seulement) ; `pico8_ptrs_t pico8_jp_ptrs(…, void *ctx);` ; `typedef int (*f)(void *e, void *ctx);` ; tout prototype de `pico8_ptrs.h` ; le membre `void (*kj)(void *self, …);` d'une struct ; un prototype dans un `.c`.
   - `--detail` imprime les lignes `void-ptr-r2` de la liste R2 fournie.
8. **Chevauchement** : `const pico8_str_seq_t *pico8_jo_get_nu(const void *self);` compte 1 dans `accesseurs-q6` et 1 dans `nu`, et 1 au pied.
9. **Modes et codes** :
   - `--detail` trié par catégorie puis par fichier, identique sur deux passes (`cmp`) ;
   - `--check` : code 1 sur une catégorie présente, code 0 et `résidu gardé (…) : 0 ligne(s)` sinon ; `--check inclusion-mutuelle` rend 1 sur la paire témoin ;
   - code 2 pour : `--check nope`, `--check ''`, `--check a,,b`, `--check` sans argument, `--bogus`, dossier absent, dossier sans `.c`/`.h`, `COMPOSITION_ATTEINTE` vers un fichier absent, `COMPOSITION_R2` vers un fichier absent ;
   - `atteinte-à-prouver` : une ligne `#` et une ligne vide ne comptent pas, une ligne utile compte, affichée `<chemin de la liste>:<n>`.
10. **`--champs`** : sur un `pico8_jo_t` à trois membres,
    - `a` n'est écrit que par `self->a = pico8_jo_start.a;` dans `pico8_jo_struct_init` : `0\t0` ;
    - `b` est écrit par `self->b = 1;` dans `pico8_jo_struct_init` : `1\t0` ;
    - `c` est écrit par `self->c += 2;` dans `pico8_jo_hk` : `1\t1`.

    Le pied est `champs : 3 ; 0 écriture : 1 ; 0 hors constructeur : 2`.

### Greps de Sortie (sur l'arbre final)

Chaque « vide » exclut nommément ce qu'on garde :

1. `git diff --stat eb18618..HEAD -- src/ tests/e2e/` : **vide**. Aucune ligne du jeu, aucun e2e.
2. `git diff --name-only eb18618..HEAD | grep -vE '^(tools/composition_residue\.sh|tools/atteinte_a_prouver\.txt|tools/composition_r2_sites\.txt|tests/test_composition_residue\.c|Makefile|docs/plan-3\.md|docs/prd/)'` : **vide**. On garde les six livrables et le déplacement du PRD.
3. `git diff eb18618..HEAD -- Makefile | grep -E '^[-+]test:|^[-+].*(GUARD|data-guard)'` : **vide**. On garde les lignes ajoutées pour la légende, la cible `composition-residue`, son `.PHONY`, la règle du TU et l'entrée de `UNIT_TESTS`.
4. `grep -nE 'composition' Makefile | grep -vE 'composition-residue|test_composition_residue|composition_residue\.sh'` : **vide**. Aucune autre mention, en particulier aucune garde.
5. `sh tools/composition_residue.sh --detail | grep -c test_composition_residue` : **0**. Le TU ne se compte pas lui-même.
6. `sh tools/data_residue.sh --check spec,overrides,stash,ob,dispatch,par-nom,liste,ref-audit,valeur,type-tag src tests` : `résidu gardé (…) : 0 ligne(s)`. Le TU n'entre pas dans le résidu D.
7. `sh tools/composition_residue.sh --detail | grep -E '^nu\b' | grep -c test_data_residue` : **0**. L'exclusion `NU_EXCLUSION` tient.
8. `sh tools/composition_residue.sh --check inclusion-mutuelle; echo $?` : **1**. 21 paires attendues : la mesure existe, et elle n'est dans aucune garde.
9. `grep -c . tools/composition_r2_sites.txt` : **11**. `grep -cE '^[^#[:space:]]' tools/atteinte_a_prouver.txt` : **1**.

### Filet, budget et tableau « Runs »

- `timeout 10m make test` : vert, **71** binaires « all tests passed » (70 + `test_composition_residue`), le reste de la sortie identique au départ.
- `timeout 5m make e2e` : **26 conformes**. Il est lancé même sans ligne de `src/` changée (invariant §3).
- `make composition-residue`, `--detail` et `--champs` sur l'arbre final : leur sortie va au message de commit. Un écart aux comptes du tableau est expliqué ligne à ligne.
- Pas de `class-coverage-net` ni de `sanitize-net` : la règle « En D et en 2 » n'est pas déclenchée.
- Jamais l'appli raylib ; `make raylib` n'est pas concerné.
- **Budget** : environ 2 h 20 pour les trois sous-étapes. Temps de tests : `make test` ≈ 6 s, `make e2e` ≈ 1 s, le TU seul < 5 s.

L'implémenteur tient un tableau « Runs » (`# | commande exacte | état de l'arbre | code | ligne clé | durée`). Le vérificateur le réutilise et ne relance que ce qui suit le dernier changement de l'arbre.

### Contrôles du vérificateur

1. Motifs, exclusions (`LIEN_EXCLUSION`, `NU_EXCLUSION`, `VOID_EXCLUSION`, `PTRS_NON_ENTITE`, liste `NOYAU`) et ordre : ceux du tableau, au caractère près.
2. Recompte indépendant : pour `lien-parent`, `accesseurs-q6`, `nu` et `parente`, la commande à la main du tableau rend les mêmes lignes que `--detail`.
3. « Vide » : le cas 1 du TU asserte la ligne de pied seule, au texte près.
4. `SORTIE` : une seule variable. La retirer d'une catégorie (copie jetable) change le pied et rien d'autre.
5. Plan : la note « Découvert à 2.0 » existe sur la ligne 2.0. Le diff du plan n'ajoute aucune liste d'étapes 2.1+.

### Critères d'échec (`FAILED:` pour le vérificateur de `prd-autopilot`)

- `FAILED:` si un fichier de `src/`, un `*.sha256`, un e2e ou un test existant change.
- `FAILED:` si `make test` ou `make e2e` (26 conformes) reste rouge après **3 tentatives** sur le même commit.
- `FAILED:` si une garde est ajoutée à `make test` (autre que le TU dans `UNIT_TESTS`), ou si `composition-residue` entre dans `test` ou `e2e`.
- `FAILED:` si un motif, une exclusion, l'ordre ou le format diffère du PRD sans justification dans le commit ; si une catégorie est ajoutée ou retirée.
- `FAILED:` si le compteur rend 0 sur une mesure sur rien, si `--check` rend 0 sur une occurrence présente, ou si un mode rend autre chose que 0 sur une entrée valide.
- `FAILED:` si `test_composition_residue.c` apparaît dans `--detail`, ou s'il entre dans le résidu D.
- `FAILED:` si le diff écrit la liste ordonnée de la phase 2 ou une ligne 2.1+.
- `FAILED:` s'il manque le tableau « Runs », la sortie initiale du compteur dans un message de commit ou la note « Découvert à 2.0 » ; ou si un commentaire ajouté sort des quatre sortes de `CLAUDE.md`.

## Out of Scope

- Toute conversion : noyau, archétype, composant, masque de bits, typage d'une signature ou d'une liste.
- Toute garde de `make test` sur une catégorie de ce compteur.
- L'écriture de la liste ordonnée de la phase 2, et le test ponctuel de la règle 3.
- Répondre à la question d'atteinte `pj-hb-nil` : 2.0 la liste, elle ne la tranche pas.
- Les points que le Reste de D laisse hors plan : rangement de fichiers, globaux de `dispatch_context.h`/`pi.h`, noms de fichiers, commentaires en deux langues, en-tête commun aux tests, constantes d'empreinte gelées, copies de `fnv`/`rank_in`, `make clean`, `ttf_to_pixelfont.py`, noms `test_*_closed`, le mot « REF ».
- `offsetof` dans `tests/` (question ouverte de D.21c).
- `class-coverage-net`, `sanitize-net`, l'appli raylib, l'audio.

## Further Notes

### Après 2.0 : la liste de la phase 2 (ce que le plan prévoit, hors de cette étape)

Le plan ne prévoit pas d'étude à 2.0. Il prévoit que **la liste ordonnée de la phase 2 est écrite depuis cette mesure**, après 2.0 et avant 2.1, comme celle de D après D.0. Cette écriture n'appartient pas à ce PRD. Ses contraintes, déjà dans le plan, sont rappelées ici pour que la mesure les serve :

- une catégorie par groupe nommé, découpée au-delà de ~40 fichiers, avec son « Compte : N » ;
- les deux groupes de Q7 (émetteur de particules et sonde, faits en D sans le test ponctuel d'archétype) sont nommés comme devant passer le test ponctuel de la règle 3 ;
- une catégorie qu'aucune règle de la phase 2 ne couvre est l'arrêt « champ sans règle » du §7. Sur `eb18618`, les puces de la phase couvrent `parente`, `lien-parent`, `accesseurs-q6`, `défauts` et `nu` (noyau). Aucune puce ne dit ce que deviennent `aiguillage-nom`, `un-ou-faux`, `void-ptr`, `ptrs-entité`, `noyau-inclut` ni `atteinte-à-prouver`, sauf le remède « types d'entité aux signatures, ajout typé » que le Reste donne pour les deux du milieu. C'est l'objet de Q1.

### Questions pour l'humain

2.0 mesure tout, quelle que soit la réponse. Les réponses ne changent que la variable `SORTIE` et l'écriture de la liste.

1. **Les nouvelles mesures doivent-elles être à zéro pour que la phase 2 soit finie ?** La revue de fin de la phase précédente a ajouté sept mesures au plan : les étiquettes de classe pas encore rangées en bits ; le choix d'un champ par son nom à la construction d'un projet de recherche ; deux valeurs qui valent « un » ou « faux » avec un drapeau à côté ; les fonctions publiques qui passent un objet du jeu sans type ; les listes d'objets du jeu sans type ; l'en-tête commun qui inclut les tables de données ; les questions ouvertes sur du code peut-être jamais atteint. Le texte du plan les met toutes dans la condition de fin. Faut-il les garder ainsi, ou seulement les compter et les rapporter ?
2. **Les 21 paires de fichiers qui s'incluent l'un l'autre : objectif ou simple mesure ?** Le plan dit « mesurée seulement » : comptée au début et à la fin de la phase, sans condition. Veut-on plutôt un objectif, par exemple « pas plus qu'au départ », ou zéro ?
3. **Quelle preuve pour « ce champ ne change jamais » ?** Pour savoir si un champ est une valeur fixe de sa classe ou un état propre à chaque objet, le plan demande une recherche de texte. Beaucoup de champs portent le même nom court dans plusieurs types, si bien que cette recherche ne tranche que 15 champs sur 286. Accepte-t-on aussi une preuve par le compilateur : dans une copie jetable, on rend le champ non modifiable et on regarde si le code compile encore ?
4. **Les étiquettes de classe en bits changent-elles de nom ?** Le compteur reconnaît ces étiquettes par le nom de leur champ. Pour que le compte tombe à zéro, le champ converti doit prendre un autre nom, qui dirait « étiquettes ». Le glossaire en donne le sens, mais le plan réserve d'habitude les renommages à la dernière phase. D'accord pour ce renommage dans la phase 2 ?

### Décisions de l'agent à la place de l'utilisateur (G)

- Modules : un script neuf à quatre modes, deux listes versionnées, une cible hors suite, un TU. Aucune garde.
- Tests : un TU sur mini-arbre, avec témoin et faux positifs par catégorie ; pas de `gcov`, puisque rien de `src/` ne change.
- Découpage : trois commits d'environ 50 minutes (Implementation Chunks), parce que les lecteurs structurels (E, prototypes, graphe d'includes) doublent la taille de D.0.

### Annexe — reproduire (lecture seule, `eb18618`)

```sh
F=$(find src tests -name '*.[ch]' | sort)
grep -HnE '^[[:space:]]+pico8_(ke|jo|qh|hj|nr|g_)_t[[:space:]]+[a-z0-9_]+;' $F | wc -l          # parente 26
grep -HnE '\bpico8_[a-z0-9_]+_of\b|\b[a-z0-9_]*(emitter|probe)_of\b|(->|\.|\(\*)parent\b' $F | grep -v pico8_state_of | wc -l   # 111
grep -HnE '(\b|_)(get_(nu|fa|ra|p|kq|ql|fg|mt|fx|p_)|set_m_)\b' $F | wc -l                      # 262
grep -HnE '\bpico8_[a-z0-9_]+_start(_t)?\b' $F | wc -l                                            # 701
grep -HnE '\bnu\b|get_nu\b|_nu_(tags|items)\b' $F | grep -v test_data_residue.c | wc -l           # 208
grep -HnE '\bhp_(pose|ho)\b|\bpico8_ho_gp_has\b|\bpico8_hp_(mr|gp)_names\b|(->|\.)(mr|gp)\b' $F | wc -l   # 30
grep -HnE '\b(pk|fp)_is_bool\b' $F | wc -l                                                         # 27
grep -nE '^[a-z].*\bpico8_[a-z0-9_]+\(' src/*.h | grep -v '^src/pico8_ptrs.h' \
  | sed -E 's/void \*(r_)?ctx\b//g' | grep -E 'void \*' | grep -vE '_dispatch\(|_get_[a-z0-9_]+\(|:typedef '   # void-ptr 11
grep -n '#include "pico8_game_data.h"' src/pico8_struct.h src/pico8_ke.h src/pico8_entity.h      # noyau-inclut 1
wc -l src/pico8_game_data.h                                                                         # 574
```

Dans la ligne `lien-parent`, le script retire `pico8_state_of` par `sed` ; à la main, `grep -v pico8_state_of` rend le même compte sur `eb18618`, car aucune des deux lignes qui le portent (`struct.h:21`, `bootstrap.c:64`) ne porte aussi le motif.
