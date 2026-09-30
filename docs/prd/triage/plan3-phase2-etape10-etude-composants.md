---
title: "Plan 3, étape 2.10 : étude B en lecture seule, règle 2, accesseurs Q6, lien parent, aplatissement, et registres d'entités non comptés. Pour chaque emplacement Q6, lecture d'archétype ou composant, avec ses sites ; forme de la planète et de la sonde aplaties (décision A) ; sort de `parent()`, des `pico8_X_of`, d'`emitter_of`/`probe_of` ; typage des cibles de table en `void *` ; inventaire de `hv`, `cv`, `cg`, `ui.ic` et extension du compteur pour qu'ils y tombent ; lignes 2.14 à 2.16 réécrites dans le plan. Trois sous-étapes, aucun code changé"
description: "L'étude qui fixe, avant 2.12, comment les accesseurs Q6 et le lien parent disparaissent et quelle forme prennent les deux arbres aplatis. Elle inventorie aussi les registres qui tiennent des entités par `void *` sans qu'aucune catégorie ne les compte (note « Découvert à 2.8c »), et dit comment étendre `tools/composition_residue.sh`. Mesuré sur `9d7d7ca` : `accesseurs-q6` 237 lignes, dont 124 entrées de table, 52 implémentations toutes triviales et 15 lectures dans `src/` ; `lien-parent` 111, dont 3 seules marches de la chaîne `parent` ; 35 déclarations `pico8_ptrs_t` porteuses d'entités dans 15 fichiers de `src/` ; registres typés `pico8_ents_t` sur copie : 20 erreurs dans `src/`, 375 dans 36 TU. Produit `docs/etude-2-10-composants.md` et réécrit les lignes 2.14, 2.15 et 2.16 du plan (annotations sur 2.17, 2.18)."
status: needs-triage
---

## Problem Statement

**Écrit sur `9d7d7ca`** (branche `claude/plan-pilot-3-7zdft0`, 2.8c commise, aucune étude encore faite) ; `522824b`, arrivé pendant l'écriture, ne fait que déplacer des PRD (2.8 vers `done/`, 2.9 vers `triage/`). Aucune ligne du dépôt n'a changé pour écrire ce PRD. Les essais ont été faits sur une seule copie jetable (`git clone` local dans le scratchpad), supprimée depuis. **Les numéros de ligne sont ceux de `9d7d7ca`.** 2.9 ne change aucun code, mais l'agent retrouve chaque site par son texte si une étape s'est glissée avant.

Du point de vue du mainteneur du port : la phase 2 veut que tout type d'entité soit « noyau + archétype + composants ». Trois restes de la couche objet s'y opposent. (1) Les **accesseurs Q6** : dix membres de la table de classe (`get_fa`, `get_ra`, `get_p`, `get_kq`, `get_ql`, `get_fg`, `get_mt`, `get_fx`, `get_p_`, `set_m_`, `struct.h:142-151`) qui lisent un champ sur une entité de type inconnu. (2) Le **lien parent** : `parent()` (`struct.h:135`), les descentes `pico8_qh_of`, `pico8_nl_of`, `pico8_g__of`, et `emitter_of`/`probe_of` (`struct.h:136-137`). (3) L'**enchâssement** de la struct parente, que 2.15 aplatit. Les étapes 2.14, 2.15 et 2.16 sont **provisoires** dans le plan ; leurs sous-étapes et leurs comptes attendent cette étude, comme D.10 à D.12 attendaient D.9.

S'y ajoute un trou du compteur, relevé à 2.8c : les registres `hv`, `cv`, `cg` et `ui.ic` tiennent des entités dans des `pico8_ptrs_t` (éléments `void *`), et aucune catégorie ne les compte. `ptrs-entité` ne lit que les membres `pico8_ptrs_t` des **types d'entité** (`composition_residue.sh:218`) ; `void-ptr` ne lit que les prototypes qui écrivent `void *` en toutes lettres (`:307`). Les registres sont des statiques de module et des prototypes qui rendent `const pico8_ptrs_t *`, et `ui.ic` est un tableau dans une sous-struct (`pico8_ui_ic_t.layers[16]`, `ui.h:24`). La Sortie de la phase veut **tout à zéro** : sans extension, le compteur rendrait 0 avec eux en place.

### Texte du plan, recopié (`docs/plan-3.md` à `9d7d7ca`, ligne 2.10)

> - 2.10 — **étude B, lecture seule : règle 2, accesseurs, lien parent, aplatissement.** (a) Les dix emplacements Q6 qui restent (`get_fa`, `get_ra`, `get_p`, `get_kq`, `get_ql`, `get_fg`, `get_mt`, `get_fx`, `get_p_`, `set_m_` ; `struct.h:97-107`) : pour chacun, lecture d'archétype (champ constant selon 2.9) ou composant (groupe présent dans deux branches distinctes, règle 2 : `ra`/`p`/`kq`, `fa`, `m_`/`p_`… d'après `--champs`), et les sites de `dc` (`entity.c:264`), `entity_geom`, `ui`, `help`… qui les lisent ; liste ordonnée de 2.14. (b) `parent()` (`struct.h:90`), `emitter_of`/`probe_of` (`:91-92`), `pico8_qh_of`, `pico8_nl_of` (`planet_family.c:679-695`), `pico8_g__of` (`probe.h:58`) ; la forme de l'arbre `jo` et de `g_`/`fv` sans enchâssement (voir la question ci-dessous) ; le typage des cibles de table encore en `void *` (un renommage en `*_dispatch` pour échapper au motif, `composition_residue.sh:310`, n'est pas une solution) ; liste ordonnée de 2.15 et 2.16. Sortie : `docs/prd/done/plan3-phase2-etape10-etude-composants.md`. Compte : aucun. Budget ≈ 2 × 50 min (a, b).

Et la note de 2.8c qui l'étend (recopiée) :

> Les registres `hv`, `cv`, `cg` et `ui.ic` tiennent encore des entités par `void *` et aucune catégorie ne les compte : l'étude 2.10 les inventorie et étend le compteur pour qu'ils y tombent (sortie de phase = tout à zéro).

Numéros du plan périmés (ils datent d'avant 2.3 et 2.7) : `struct.h:97-107` → **`:142-151`** ; `:90` → **`:135`** ; `:91-92` → **`:136-137`** ; `planet_family.c:679-695` → **`:668-684`** ; `probe.h:58` → **`:56`** ; `entity.c:264` → **`:204`** (`get_fa`) et **`:230`** (`get_ra`).

Règles de la phase qui s'appliquent (recopiées) :

> 2. **Composant** : un groupe de champs devient un composant s'il apparaît dans **deux branches distinctes** de l'arbre (exemple connu : `py`/`fc`/`k_`/`qo`/`ro`/`qg`/`q_`/`du` dans `hj`, `pm`, `mx` et `ga`, alors que `ga` descend de `jo`). Sinon il reste dans la struct du type.
>
> - **Sortie** : aucune struct d'entité n'enchâsse plus la struct de sa parente (tout type = noyau + archétype + composants) ; chaque ancienne classe a son archétype `static const` ; […] `make composition-residue` rend `résidu 2 : 0 ligne(s), 0 fichier(s)`, toutes catégories à zéro, `inclusion-mutuelle` et `nu` compris (Q1, Q2, Q4).

Et les deux dépendances d'ordre qui touchent l'étude :

> - **Accesseurs avant l'aplatissement (2.14 avant 2.15).** Les tables de classe de l'arbre `jo` partagent les accesseurs de `jo` par l'enchâssement (`planet_family.c:53-54`, `:144-145`, `:204-205` : `.get_fa = pico8_jo_get_fa`, `.get_ra = pico8_jo_get_ra`) ; aplatir d'abord obligerait à les réécrire par type.
> - **Paires d'inclusion de classe après la composition (2.17 après 2.14 et 2.15).** […]

Décisions de l'humain qui cadrent l'étude : **Q1** (toutes les catégories sont des conditions de sortie, à zéro ; une catégorie neuve pour les registres l'est aussi) ; **Q3** (preuve compilateur pour la règle 1) ; **question A du 2026-09-30** : un seul type « planète » pour les dix sortes, qui porte tous leurs champs, la sorte n'étant plus que sa table de méthodes et son archétype ; de même un seul type pour la sonde et sa variante d'aide. A tranche la forme ; l'étude en tire les conséquences, elle ne la rouvre pas.

### État de départ attendu — hypothèses

L'étude part de l'arbre d'après 2.9 (étude : aucun code ne change entre `9d7d7ca` et son départ). Chaque hypothèse est revérifiée en 2.10a ; si l'une tombe, l'étude le dit et refait les mesures touchées.

- **H-1** : `make composition-residue` rend `parente` 10 / 3, `lien-parent` 111 / 27, `accesseurs-q6` 237 / 33, `défauts` 669 / 52, `void-ptr` 8 / 4, `inclusion-mutuelle` 17 / 16, les six autres à 0 et gardées ; pied **`1073 / 85`**. Garde : `noyau-inclut,un-ou-faux,aiguillage-nom,atteinte-à-prouver,nu,ptrs-entité`. `--champs` : **282 / 15 / 27**.
- **H-2.9** : `docs/etude-2-9-archetypes.md` existe, avec (i) la classe A/I/K/E/S/H de chaque couple (type, champ), en particulier `fa`, `ra`, `p`, `kq`, `ql`, `fg`, `mt`, `fx`, `p_`, `m_` et leurs `has_*` sur chaque type porteur ; (ii) le **chemin de lecture** d'un champ d'archétype retenu (struct de classe par famille, pointeur d'archétype dans l'instance, ou accesseur `static`) ; (iii) la forme des archétypes sous A (un type d'archétype par futur type aplati) ; (iv) les lignes 2.12 et 2.13 réécrites. **Si ce fichier manque au départ, l'étude s'arrête** : « lecture d'archétype » n'a pas de sens sans la classe du champ et le chemin de lecture, et l'étude ne devine pas 2.9.
- **H-3** : `docs/etude-2-11-inclusions.md` **n'existe pas encore** (2.11 passe après et lit ce livrable, voir « Contrat avec 2.11 »).

À revoir si une hypothèse tombe : « Mesures », le tableau « Runs ».

## Solution

Une étude en trois sous-étapes, un commit de documentation chacune. Elle produit :

1. **`docs/etude-2-10-composants.md`**, le livrable (voir G1 pour le chemin), qui contient :
   - §1, la **règle 2** et les **accesseurs Q6** : pour chaque emplacement, ses porteurs, la classe du champ selon 2.9, le verdict (archétype, composant, champ du type aplati), ses sites de lecture et leur réécriture ; les lectures croisées hors Q6 (test de famille suivi d'un cast) ; la liste ordonnée de 2.14 ;
   - §2, le **lien parent et l'aplatissement** : la forme de la planète et de la sonde sous A, le sort de `parent()`, des `pico8_X_of`, d'`emitter_of`/`probe_of`, le typage des cibles de table en `void *` ; les listes ordonnées de 2.15 et 2.16 ;
   - §3, les **registres** : inventaire de tout conteneur ou prototype qui tient des entités par `void *` hors catégorie, l'extension du compteur (texte, essai sur copie, garde mordante), et les sous-étapes qui les typent ;
   - §4, la **table du sort des symboles**, pour 2.11 (voir « Contrat avec 2.11 ») ;
   - §5, les questions à l'humain, s'il y en a ;
   - en annexe, les essais et le tableau « Runs ».
2. **`docs/plan-3.md`** mis à jour : lignes 2.14, 2.15, 2.16 réécrites ; ligne ou sous-étapes neuves pour l'extension du compteur et le typage des registres (placement choisi par l'étude, voir §3) ; 2.17 et 2.18 annotées là où 2.10 change un fait ; note « Découvert à 2.10 » sur la ligne 2.10 ; ligne d'estimation mise à jour.

Aucun fichier de `src/`, `tests/`, `tools/` ou du `Makefile` ne change. L'extension du compteur est **écrite et essayée sur la copie**, versée en annexe, et commise par la sous-étape du plan que l'étude nomme. `make composition-residue` rend le même pied avant et après l'étude.

## User Stories

1. En tant que pilote de 2.14, je veux, pour chaque emplacement Q6, ses porteurs, ce qui le remplace (archétype, composant, champ du type aplati) et la liste de ses sites, pour retirer l'emplacement sans relire la table.
2. En tant que pilote de 2.15, je veux la forme exacte de la planète et de la sonde aplaties (membres, ordre des familles, collisions de noms), et ce qui remplace chaque descente `pico8_X_of`, pour aplatir sans rouvrir la décision A.
3. En tant que pilote de 2.16, je veux savoir quel membre de la table change de signature et ce que chaque changement coûte, pour typer les cibles `void *` sans les renommer.
4. En tant que pilote de la sous-étape « registres », je veux le texte de la catégorie neuve du compteur, essayé et mordant, et la liste des sites à typer, pour faire tomber les registres à zéro comme les autres catégories.
5. En tant que pilote de 2.11, je veux, pour chaque symbole que mes paires emploient, la sous-étape de 2.14/2.15/2.16 qui le retire, pour dire quelle paire tombe d'elle-même.
6. En tant que vérificateur, je veux que chaque verdict de la règle 2 cite les porteurs mesurés par `--champs` et la classe de 2.9, pour qu'aucun composant ne naisse sur une intuition.
7. En tant qu'humain, je veux au plus quelques questions, en français simple, seulement là où aucune règle ne tranche.

## Implementation Decisions

### §1 — Règle 2 et accesseurs Q6 (2.10a)

**Ce que compte `accesseurs-q6` (237 lignes, mesuré)** : 124 entrées de table (`.get_X = …`), 68 définitions et prototypes des implémentations, 28 appels de lecture et 6 tests de nullité (`src/` et `tests/`), 10 membres de `struct.h`, 1 commentaire. L'étude refait ce découpage (commande en annexe) : c'est lui qui donne les comptes d'arrivée de chaque sous-étape de 2.14.

**Porteurs, implémentations et lecteurs par emplacement** (mesuré sur `9d7d7ca`) :

| Emplacement | Types porteurs (`--champs`) | Implémentations (distinctes) | Lectures dans `src/` | Lectures de TU |
|---|---|---|---|---|
| `get_fa` | `bg`, `fq`, `g_`, `ie`, `iv`, `jc`, `jo`, `js`, `mh`, `mz`, `pj`, `ui` | 12 | `entity.c:204` (`dc`), `entity_geom.c:36`, `ui.c:293` | 1 |
| `get_ra` | `g_`, `ie`, `iv`, `jc_mouse_point`, `jo`, `js`, `lq`, `mz`, `nf`, `qb` | 10 | `entity.c:230` (`dc`), `entity_geom.c:17`, `wormhole_display.c:15` | 4 |
| `get_p` | `g_`, `jc_mouse_point`, `jo`, `mz`, `qb` | 5 | `entity_geom.c:23` | 4 |
| `get_kq` | `fv`, `ie`, `jo`, `js` | 4 | `entity_geom.c:40` | 0 |
| `get_ql` | `fv`, `ui` | 2 | `ui.c:250` | 1 |
| `get_fg` | `fv`, `js` (`has_fg`) | 2 | `ui.c:321` | 0 |
| `get_mt` | `fr`, `jc` (`has_mt`) | 2 | `slipway.c:70` | 2 |
| `get_fx` | `g_` (rend 1), `jo` (`has_fx`) | 2 | `wormhole.c:45` | 0 |
| `get_p_` | `fq`, `g_`, `iv`, `jc`, `jo`, `mh`, `mz` (`has_p_`) | 7 | `bootstrap.c:70` | 0 |
| `set_m_` | `bg`, `fq`, `fv`, `ie`, `jo`, `js` (`has_m_`) | 6 (écriture) | `ui.c:308`, `:314` | 2 |

Les 52 implémentations sont **toutes** une lecture de champ (ou l'écriture `m_ = …; has_m_ = 1` pour `set_m_`), sauf `pico8_g__get_fx`, qui rend la constante 1. Aucun calcul à préserver.

**Règle 2 à la lettre.** Depuis 2.3, chaque fille de `ke` est une racine (en-tête direct) : il ne reste d'arbre que `jo` → sortes et `g_` → `fv`. Chaque groupe du tableau apparaît dans **au moins deux racines distinctes** (`fa` : douze ; `mt` : `fr`, sous `jo`, et `jc`), donc **tous** sont candidats composants par la règle. L'étude ne s'arrête pas là : le composant n'est retenu que si le champ est de l'**état** (classe I, K ou E de 2.9) sur au moins un porteur ; s'il est A sur tous ses porteurs, la lecture passe par l'**archétype** (chemin retenu par 2.9). Un groupe mêlé (A sur l'un, état sur l'autre) est tranché par écrit, avec la règle qui le range ; aucune règle ne tranche → §5.

**Options de remplacement**, à comparer pour chaque groupe (l'étude en retient une par groupe et la justifie) :
- **(R1) champ du noyau** : le membre monte dans `pico8_struct_header_t` (comme `me`/`t` à 2.3a). Coût : chaque entité le porte, porteur ou non ; la lecture « absent » (`get_X == NULL`) doit rester distinguable si un lecteur l'observe (`dc` saute une entité sans `fa`, `entity.c:204`).
- **(R2) composant par pointeur depuis le noyau** : le noyau porte `pico8_X_comp_t *x` (NULL : absent), ce que le plan décrit (« composants optionnels portés par pointeur »). L'absence est le NULL.
- **(R3) lecture d'archétype** par le chemin de 2.9, quand le champ est A partout.

Critères, mesurés sur copie pour le groupe `fa` au moins : aucune paire d'inclusion neuve (le compteur relancé), aucun `void-ptr` neuf, aucun `parente`/`lien-parent` neuf ; la sémantique « absent » conservée pour chaque lecteur qui l'observe (`dc`, `ui.c:293`, `entity_geom.c:35`) ; la preuve de 2.14 (chemin touché : `gcov` des lecteurs, mutations).

**Lectures croisées hors Q6** (la règle 2 s'y applique aussi, le compteur ne les voit pas) : un test de famille suivi d'un cast pour lire un champ de planète sur une entité de type inconnu. Mesuré : `pico8_jo_is_family(` sur 27 lignes de 14 fichiers de `src/`, dont 21 sont des `assert` ; les lectures réelles : `entity_geom.c:62` (`ra`), `:79` (`ma` de planète ou de caméra `qb`), `slipway.c:55` (`p`), `:64` (`ra`), `slipway_display.c:29` (`id`), `particle.c:25-29` (`fa`, `ra` par `pico8_g__of`), `probe.c:92`, `:113`, `:293`. L'étude dit, pour chacune, si elle bascule sur le composant retenu (et donc à quelle sous-étape de 2.14) ou reste un cast vers le type planète (décision A). `ma` (planète et `qb`, deux racines, écrit hors construction à `entity_geom.c:85`) est un cas de règle 2 à trancher.

**Liste ordonnée de 2.14** : sous-étapes à ≈ 50 min, par groupe (le plan provisoire propose `fa` ; `ra`, `p`, `kq` ; `fx`, `p_` ; `set_m_` ; `ql`, `fg`, `mt` et retrait des membres de `struct.h`), avec pour chacune modules, compte `accesseurs-q6` d'arrivée calculé sur `--check accesseurs-q6` (lignes retirées), `void-ptr` d'arrivée (`pico8_jo_set_m_`, `planet.h:122`, part avec `set_m_`), preuves, `class-coverage-net`/`sanitize-net`, budget.

### §2 — Lien parent, aplatissement, cibles de table (2.10b)

**Ce que compte `lien-parent` (111 lignes, mesuré)**, par jeton : `probe_of` 28, `emitter_of` 23, `pico8_qh_of` 21, `pico8_nl_of` 12, `pico8_qh_probe_of` 10, `.parent` 10, `pico8_g__of` 9, `->parent` 6, `pico8_hj_emitter_of` 5, `pico8_g__probe_of` 4, `pico8_g__emitter_of` 4, `mz_probe_of` 2, `jc_probe_of` 2, `ga_emitter_of` 2, `(*parent` 1. Par fichier : `planet_family.c` 21, `probe.c` 16, `test_pico8_qh.c` 8, `planet_family_trade.c` 7, `test_pico8_ui.c` 6, `planet_family.h` 6, `particle.c` 6…

Faits mesurés que l'étude reprend :
- **`parent()` n'a que trois lecteurs** : les marches de `pico8_qh_of`, `pico8_nl_of` (`planet_family.c:672`, `:681`) et `pico8_g__of` (`probe.c:537`). Ses 10 écrivains sont les tables des 9 sortes filles et de `fv` (`help.c:131`).
- **Descentes** : `pico8_qh_of` vrai pour `qh`, `nl`, `os`, `hj`, `pm`, `nr`, `fr`, `mx` (8 sortes sur 10 : pas `jo`, pas `ga`) ; `pico8_nl_of` pour `nl` seul ; `pico8_g__of` pour `g_` et `fv`. Lecteurs dans `src/` : `economy_score.c:48`, `economy_tiers.c:261`, `particle.c:19`, `planet.c:280`, `planet_display.c:154`, `:429`, `:436`, `planet_trade.c:152`, `probe.c:83`, `:283`.
- **Piège : les étiquettes ne suivent pas la chaîne.** `PICO8_TAG_QH` n'est porté que par `qh` ; `nl`, `os`, `pm`, `mx` n'ont que `jo`/`np`/`prober`, `hj` a `hj`, `nr`/`fr` ont `nr`, et `ga` a un masque **0** (tables à `planet_family.c`, `planet_family_trade.c`). Remplacer `pico8_qh_of` par un test d'étiquette changerait le résultat pour 7 sortes. L'étude nomme un autre porteur de la sorte (membre de la table de classe ou de l'archétype, par exemple un masque de familles résolu une fois) et prouve l'équivalence sur les 10 sortes.
- **Autre test de famille existant** : `pico8_jo_is_family` compare `states[PICO8_STATE_LL]` à `pico8_jo_ll_dispatch` (`planet.c:63-65`) ; il ne marche pas la chaîne, `lien-parent` ne le compte pas. L'étude dit s'il survit à 2.15 ou s'il est remplacé par le même porteur de sorte.
- **`emitter_of` / `probe_of`** : implémentations `pico8_g__emitter_of`, `pico8_hj_emitter_of`, `ga_emitter_of` (3) ; `pico8_g__probe_of`, `pico8_qh_probe_of`, `jc_probe_of`, `mz_probe_of` (4). Lectures : `particle.c:47`, `:60`, `:90`, `planet_display.c:469`, `probe.c:156`, `:230`, `:323`, `:364` ; TU `test_pico8_ui.c:855-868`. Le groupe émetteur est dans `g_` et dans `hj`/`ga` (deux racines), le groupe sonde dans `g_`, `qh`, `jc`, `mz` : règle 2 → composants. La forme naturelle est le pointeur du noyau (R2 ci-dessus), qui remplace la méthode ; l'étude la compare au statu quo et tranche. Contrainte du motif (`composition_residue.sh:53`) : tout nom `pico8_…_of` ou `…emitter_of`/`…probe_of` compte ; un remplaçant qui garderait la méthode sous un autre nom n'est pas une solution (même règle que le renommage en `_dispatch`).

**Forme sous A** (mesuré sur `--champs`) :
- **Planète** : `jo` 37 champs ; sortes : `qh` 7, `nl` 3, `os` 0, `hj` 3, `pm` 2, `ga` 5, `nr` 2, `fr` 1, `mx` 1 (24). Noms partagés entre sortes, **tous de même type C** : `rr` (`hj`, `nl`, `nr`, `const char *`), `fz` (`ga`, `pm`, `const char *`), `emitter`/`emitter_own` (`hj`, `ga`). Aucun nom de sorte ne recoupe `jo`. Type aplati : **56 champs** (37 + 19 distincts), avant composants. L'étude donne la struct complète (ordre des membres, commentaires de destination selon `CLAUDE.md`), dit lesquels partent en composant, et chiffre la disposition (taille avant/après) pour `sanitize-net`.
- **Sonde** : `g_` 10, `fv` 11, aucun nom commun → 21 champs.
- **Chemins à réécrire** : lignes à chemin enchâssé (`.jo.`, `.qh.`, `.hj.`, `.nr.`, `.g_.`) : **123** dans 3 fichiers de `src/` (`help.c`, `planet_family.c`, `planet_family_trade.c`), **324** dans 17 TU ; casts vers un type de sorte ou `fv` : 19 dans `src/`, 10 dans les TU. 2.3b/2.3c ont réécrit des volumes comparables en ≈ 40 min par passe guidée par les erreurs du compilateur.
- **Ordre** : 2.14 avant 2.15 (dépendance du plan) ; l'étude dit si l'aplatissement de la sonde (1 `parente`, 21 champs) précède celui de la planète (9 `parente`), et comment le découper en sous-étapes à ≈ 50 min (le plan provisoire dit ≈ 6).

**Cibles de table en `void *`** (`void-ptr` 8 / 4 : `pico8_hd_true`, `pico8_jo_set_m_`, `pico8_qh_probe_of`, `pico8_hj_emitter_of`, `pico8_ga_mm`, `pico8_g__emitter_of`, `pico8_g__probe_of`, `pico8_g__mm`). Une cible de table doit avoir exactement le type du membre : la typer, c'est typer le **membre**, et donc toutes ses autres cibles. Essais sur copie :

| Essai | Erreurs de compilation | `void-ptr` | Pied |
|---|---|---|---|
| membre `hd` et `pico8_hd_true` en `pico8_struct_header_t *self` | 18 (13 dans 6 fichiers de `src/`, 5 dans 2 TU) | 8 → **7** | 1072 / 84 |
| membre `mm` et ses 2 cibles | 6 (4 de `src/` dont 2 définitions, 2 de `test_pico8_g_.c`) | 8 → **6** | 1071 / 85 |
| **tous** les membres à `void *self` (27 lignes de `struct.h` et `pico8_state_fn`) | **418**, dans 44 fichiers (`planet_family.c` 127, `planet_family_trade.c` 66, `test_pico8_g_.c` 32…) | — | — |

Cibles par membre (entrées non NULL / fonctions distinctes) : `kj` 19/12, `ev` 15/5, `hd` 15/5, `hk` 11/2, `hb` 13/5, `ldown` 14/6, `gv` 10/2, `hq` 13/4, `ey` 12/3, `kf` 10/2, `cn` 10/2, `mm` 6/2, `emitter_of` 6/3, `probe_of` 12/4, `ol` 2/2, `rdown` 1/1, `has_hb` 1/1 ; `states` 25/10. Il y a 16 prototypes `*_dispatch` publics en `void *self` (5 en-têtes), 11 `*_get_*`, 123 fonctions de `.c` qui prennent `void *self`, et 145 casts `(pico8_X_t *)self`. L'étude tranche : typer **seulement** les membres dont une cible est comptée (`hd`, `mm`, et ceux qui disparaissent avec 2.14/2.15 : `set_m_`, `emitter_of`, `probe_of`), ou toute la table (≈ 418 erreurs, à découper). Le typage de toute la table n'est **pas** exigé par la Sortie (les membres ne sont pas des prototypes de premier niveau) ; s'il est retenu, l'étude le place et le chiffre.

**Listes ordonnées de 2.15 et 2.16**, sur le modèle des lignes 2.4 à 2.8 : modules, comptes d'arrivée (`parente`, `lien-parent`, `void-ptr`, et `inclusion-mutuelle` si une sous-étape retire la dernière inclusion d'une paire, mesuré sur copie seulement si l'essai est simple, sinon laissé à 2.11), preuves, `class-coverage-net` (oui dès qu'une table ou un constructeur bouge ; comptes égaux à `tools/class_coverage_net_expected.txt`), `sanitize-net` (oui si la disposition change), budget.

### §3 — Registres d'entités hors catégorie (2.10c)

**Inventaire mesuré** (toute déclaration `pico8_ptrs_t` de `src/` dont les éléments sont des entités) :

| Registre | Stockage | Accès public | Lecteurs dans `src/` | TU |
|---|---|---|---|---|
| `hv` | `static pico8_ptrs_t g_hv` (`entity.c:32`) | `const pico8_ptrs_t *pico8_hv(void)` (`entity.h:24`) | `bootstrap.c:52` (`dg`), `wormhole.c:42` (itérateur, `void *e`) | 158 lignes dans 29 TU |
| `cv` | `pico8_cv_entry_t.list` (`entity.c:18`), 5 seaux | `pico8_cv(const char *)` (`entity.h:30`) | `economy.c:142`, `:252`, `entity.c:195` (`dc`), `ui.c:286` | ≈ 80 lignes |
| `cg` | `pico8_cg_entry_t.list` (`entity.c:27`), 10 seaux | `pico8_cg(const char *)` (`entity.h:36`) | `bg.c:182`, `economy.c:214`, `economy_formulas.c:50`, `:144`, `economy_score.c:25`, `:51`, `:74`, `economy_tiers.c:23`, `galaxy.c:67`, `probe.c:165`, `:178`, `score_panel.c:125`, `slipway.c:301`, `:302`, `wormhole.c:147` | ≈ 150 lignes |
| `ui.ic` | `pico8_ui_ic_t.layers[16]` (`ui.h:24`), dans `ui.ic_own` | membre `ui.ic` | `ui.c:108`, `:277-278`, `:298`, `:411-415` | 7 lignes, 4 TU |
| couches de `dc` | local `pico8_ptrs_t layers[15]` (`entity.c:185`) | — | `entity.c:211`, `:227-228` | — |
| aides génériques | `pico8_ij` (`entity_geom.h:31`, jumelle `pico8_ij_ents` déjà là) ; `pico8_jp_ptrs`, `pico8_la_ptrs` (`collect.h:12`, `:16`), employées **aussi** sur des listes non-entité (`planet_display.c:186`, `planet_trade.c:128`) | — | listes filtrées `economy_score.c:75`, `probe.c:173`, `score_panel.c:121`, `slipway.c:289-291` | — |
| prédicats et itérateurs | `cz_predicate`, `pico8_bv_jc_predicate`, `pico8_eg_gg_pred`, `pico8_jc_ce_np_predicate`, `pico8_jc_ce_jc_predicate` (`void *e`) ; `void *e` des boucles `pico8_ptrs_iter_next` (`wormhole.c:31`, `economy.c:117`, `:194`, `:255`, `economy_formulas.c:52`, `economy_score.c:27`, `economy_tiers.c:25`, `entity_geom.c:104`) | — | — | — |

Soit **≈ 35 lignes `pico8_ptrs_t` porteuses d'entités dans 15 fichiers de `src/`**, sur 79 lignes `pico8_ptrs_t` (35 fichiers) hors `ptrs.[ch]` ; les ≈ 44 autres portent des chaînes (`hy.invented`), des enregistrements `jy` (`jo.jy`, `hp.i`, `ji`, `jx`, `is`), des particules (`emitter.qy`, `jc.ts`). L'étude refait cet inventaire en lisant chaque écrivain (comme `PTRS_NON_ENTITE`, `composition_residue.sh:81-85`) et le complète : tout `void *` qui porte une entité hors d'une catégorie existante y figure (le membre `new_entity` de `pico8_entity_kind_t`, `struct.h:170`, qui rend `void *`, en fait partie). Deux lignes de la liste R2 (`tools/composition_r2_sites.txt` : `pico8_bm_has`, `pico8_dc`) nomment un registre comme amont `void *` ; la sous-étape qui type les registres les met à jour.

**Extension du compteur.** L'étude écrit la catégorie (nom proposé : `registres`), l'essaie sur la copie et la verse en annexe. Deux définitions ont été chiffrées pour ce PRD ; l'étude en retient une, ou une autre justifiée :
- **V1 — points de déclaration** : membres `pico8_ptrs_t` (tableaux compris) de **toute** struct de `src/`, statiques de fichier, et prototypes de `.h` qui prennent ou rendent un `pico8_ptrs_t`, hors une liste versionnée de non-entités prouvées par leur écrivain (extension de `PTRS_NON_ENTITE`, qui passe de `type.membre` à `fichier:nom` pour couvrir les statiques et les prototypes). Compte de départ estimé : `entity.c:18`, `:27`, `:32`, `entity.h:24`, `:30`, `:36`, `entity_geom.h:31`, `ui.h:24`, plus `collect.h:12`, `:16` si les aides génériques restent partagées → **8 à 10 lignes, 4 à 5 fichiers**. Voit un registre neuf déclaré n'importe où.
- **V2 — toute occurrence** de `pico8_ptrs_t` dans `src/`, hors la même liste : **79 lignes / 35 fichiers** au départ, dont ≈ 44 à exempter une à une. Plus lourde à tenir, voit aussi les locales.

Critères : (1) la catégorie rend **au moins** les lignes de `hv`, `cv`, `cg`, `ui.ic` sur l'arbre de départ (contrôle 6) ; (2) elle tombe à **0** quand ces registres sont typés `pico8_ents_t`, sans renommage ni exemption de complaisance ; (3) **mordante** : un `pico8_ptrs_t` porteur d'entités réinjecté dans une copie la fait monter, et, une fois dans `COMPOSITION_RESIDUE_GUARD`, rougit `make test` ; (4) aucune autre catégorie ne change ; (5) `tests/test_composition_residue.c` reçoit sa fixture (cas positif, cas exempté, tableau `layers[16]`, statique, prototype) ; (6) elle entre dans `CATEGORIES` et `SORTIE` (Q1 : une condition de sortie de plus), et le pied de départ de la phase change : l'étude donne le nouveau pied mesuré sur copie.

**Typage des registres**, essayé sur copie : `g_hv`, les listes de `cv`/`cg` et les trois accès passés en `pico8_ents_t` (`entity.c`, `entity.h`, 22 lignes) → **20 erreurs dans 14 fichiers de `src/`** (1 à 3 par fichier) et **375 dans 36 TU** (première vague ; la suivante n'est pas mesurée). L'étude découpe ce travail en sous-étapes à ≈ 50 min (le PRD estime : extension du compteur ; `src/` et `ui.ic` ; TU, peut-être en deux) et les **place** : le typage ne dépend que du noyau (2.3) et de `pico8_ents` (2.8), pas de 2.12 à 2.15 ; il peut passer avant 2.12. Les aides génériques reçoivent des jumelles `ents` (comme `pico8_ij_ents` à 2.8c) ou deviennent typées si l'étude prouve qu'aucune liste non-entité n'y passe.

### §4 — Contrat avec 2.11 (table du sort des symboles)

Le PRD de 2.11 (`plan3-phase2-etape11-etude-inclusions.md`, H-2.10 et contrôle 4) lit ce livrable pour dire quelle paire d'inclusion tombe d'elle-même. L'étude donne donc une table **symbole → sous-étape qui le retire** (ou « reste », ou « change de module »), au moins pour : `pico8_qh_of`, `pico8_nl_of`, `pico8_g__of`, `pico8_jo_is_family`, les casts vers `pico8_jo_t` pour `p`, `ra`, `ma`, `fa`, `id`, les dix `get_*`/`set_m_`, `parent`, `emitter_of`, `probe_of`, `pico8_qh_probe_of`, `pico8_hj_emitter_of`, `pico8_g__emitter_of`, `pico8_g__probe_of`, `pico8_hd_true`, `pico8_g__mm`, `pico8_ga_mm`, `pico8_pm_kind`, `pico8_ga_kind`, et les symboles des arêtes T5 que le PRD de 2.11 nomme (`entity_geom` ↔ `planet`, `particle` ↔ `probe`, `planet` ↔ `planet_family`, `planet` ↔ `slipway`, `planet_display` ↔ `planet_family`, `entity_geom` ↔ `scroll`). L'étude 2.10 ne tranche pas les paires ; elle nomme les sous-étapes.

### Plan (2.10c)

- Lignes **2.14, 2.15, 2.16 réécrites** (sous-étapes, modules, comptes d'arrivée, preuves, `class-coverage-net`/`sanitize-net`, budgets).
- **Registres** : la ou les sous-étapes de l'extension du compteur et du typage, à la place choisie (sous-étapes de 2.16, ou ligne neuve avant 2.12), avec leurs comptes (la catégorie neuve monte d'abord, puis tombe à 0, gardée).
- **2.17** annotée si 2.10 change la technique d'une paire (par exemple une paire T4 dont le test de classe passe par le porteur de sorte de §2) ; **2.18** annotée : treize catégories (ou plus) dans `COMPOSITION_RESIDUE_GUARD`.
- Note **« Découvert à 2.10 »** sur la ligne 2.10 : verdicts par groupe, forme retenue, porteur de sorte, choix de typage de la table, catégorie neuve et nouveau pied, chemin du livrable (G1), repli s'il a servi.
- Ligne **« Estimation »** mise à jour. **Rien d'autre du plan ne change.**

## Implementation Chunks

Trois sous-étapes, un commit chacune, ≈ 50 min. Le plan donnait 2 × 50 min : **écart déclaré** (voir « Surprises »).

1. **2.10a — règle 2 et accesseurs (§1).** Revérifier H-1, H-2.9, H-3 (arrêt si 2.9 manque). Monter la copie. Découpage des 237 lignes ; tableau des porteurs croisé avec les classes de 2.9 ; verdict par groupe ; essai sur copie du remplacement retenu pour `fa` (compilation `-fsyntax-only` de tout l'arbre, compteur) ; lectures croisées hors Q6 ; liste ordonnée de 2.14. Commit : `docs(etude): 2.10a …`.
2. **2.10b — lien parent, aplatissement, table (§2).** Porteur de sorte et preuve d'équivalence sur les 10 sortes ; forme de la planète et de la sonde (structs complètes) ; `emitter_of`/`probe_of` ; essais de typage de membres (refaire `hd` et `mm`, et tout membre que l'étude retient) ; listes ordonnées de 2.15 et 2.16. Commit : `docs(etude): 2.10b …`.
3. **2.10c — registres, contrat 2.11, plan (§3, §4, §5).** Inventaire relu écrivain par écrivain ; catégorie écrite et essayée sur copie (critères 1 à 6), texte versé en annexe ; essai de typage ; table des symboles ; réécriture de 2.14 à 2.16, sous-étapes « registres », incises sur 2.17 et 2.18, note, estimation ; questions ; tableau « Runs » ; copie supprimée. Commit : `docs(plan): 2.10c …`.

**Repli** (si une sous-étape dépasse 1,5 fois son budget avant son commit) : 2.10c en deux, **2.10c1** (§3 : inventaire, catégorie essayée, annexe) et **2.10c2** (§4, plan, note). Le repli est écrit dans la note « Découvert à 2.10 ».

## Testing Decisions

Aucun code ne change, donc ni `gcov` ni mutation dans le dépôt. Les preuves portent sur la complétude des relevés et sur les essais faits sur la copie.

- **Copie** : une seule copie jetable de l'arbre de départ dans le scratchpad (`git clone` local), jamais l'arbre de travail : un implémenteur ou un vérificateur peut y travailler en parallèle. Chaque essai est remis à zéro (`git checkout -- .`) avant le suivant ; `git status --short` de la copie vide à la fin. Copie supprimée à la fin de 2.10c ; journaux dans le scratchpad.
- **Compilation d'essai** : tous les `.c` de `src/` et `tests/` en `cc -std=c99 -Wall -Wextra -Werror -Isrc -fsyntax-only`, les deux `class_coverage` sous `-DPICO8_CLASS_COVERAGE`, les trois fichiers raylib exclus (comme 2.9). Mesuré : 152 fichiers en ≈ 1 s sur 4 cœurs.

### Contrôles de complétude (écrits dans l'étude, avec leur sortie)

1. **Accesseurs** : les lignes de `--check accesseurs-q6` (237, H-1) sont toutes rangées dans le découpage (table, définition, lecture, test de nullité, membre, commentaire) ; chaque entrée de table (124) et chaque lecture (28 + 6) est rattachée à un emplacement et à la sous-étape de 2.14 qui la retire ; la somme des baisses de 2.14 = 237.
2. **Règle 2** : chaque groupe du tableau §1 cite ses porteurs (`--champs`) et la classe de 2.9 sur chaque porteur ; aucun verdict sans les deux.
3. **Lien parent** : les lignes de `--check lien-parent` (111) sont toutes rattachées à un symbole et à la sous-étape de 2.15 (ou 2.14) qui le retire ; la somme = 111.
4. **Sortes** : le porteur de sorte retenu rend, pour chacune des 10 sortes et pour `g_`/`fv`, la même réponse que `pico8_qh_of`, `pico8_nl_of`, `pico8_g__of` et `pico8_jo_is_family` (table 12 × 4 dans l'étude, lue sur les tables de classe de la copie).
5. **Forme** : la struct planète aplatie contient les 37 champs de `jo` et les 19 noms distincts des sortes (`comm -3` avec `--champs` → vide, hors ceux que l'étude envoie en composant, listés) ; même contrôle pour la sonde (21).
6. **Registres** : la catégorie essayée rend, sur la copie de départ, chaque ligne de stockage et d'accès public de `hv`, `cv`, `cg`, `ui.ic` du tableau §3 ; une ligne réinjectée la fait monter ; les registres typés sur copie la font tomber à 0 ; aucune autre catégorie ne bouge (sortie de `composition_residue.sh` comparée ligne à ligne).
7. **Contrat 2.11** : chaque symbole listé au §4 a une sous-étape, « reste » ou « change de module ».
8. **Arbre réel intact** : `git diff --stat <parent>` ne montre que `docs/etude-2-10-composants.md` et, à 2.10c, `docs/plan-3.md`. `make composition-residue` rend le même pied qu'au départ (1073 / 85). `make test` n'est pas exigé (aucun code), recommandé une fois à 2.10c.

### Budget et tableau « Runs »

Budget : **3 × ≈ 50 min**. Au-delà de 1,5 fois, repli. Jamais l'appli raylib ; `make raylib` n'est pas exigé. `timeout` sur toute commande longue (`make test`, `make e2e` sur la copie : `timeout 900`). Pour attendre un processus de fond, attendre son PID, jamais `pgrep -f`.

L'agent de l'étude tient un tableau « Runs » en annexe de l'étude. Voici celui des essais de ce PRD, sur une copie de `9d7d7ca` :

| # | Commande | Code | Ligne clé | Durée |
|---|---|---|---|---|
| 1 | `sh tools/composition_residue.sh` | 0 | `parente 10 3`, `lien-parent 111 27`, `accesseurs-q6 237 33`, `void-ptr 8 4`, `inclusion-mutuelle 17 16` ; `résidu 2 : 1073 ligne(s), 85 fichier(s)` | 0,7 s |
| 2 | `--champs` | 0 | `champs : 282 ; 0 écriture : 15 ; 0 hors constructeur : 27` | 0,3 s |
| 3 | `-fsyntax-only` de 150 `.c` + 2 sous `-DPICO8_CLASS_COVERAGE` (arbre intact) | 0 | aucune erreur | ≈ 1 s |
| 4 | `--check accesseurs-q6`, tri par sorte de ligne (annexe) | 1 | 124 table, 68 définition/prototype, 28 lecture, 6 test de nullité, 10 membre, 1 commentaire | < 1 s |
| 5 | porteurs par emplacement (`--champs`), corps des 52 implémentations | 0 | tableau §1 ; toutes triviales sauf `pico8_g__get_fx` (rend 1) | < 1 s |
| 6 | `--check lien-parent`, par jeton et par fichier ; lecteurs de `parent` | 1 | tableau §2 ; 3 marches de chaîne | < 1 s |
| 7 | masques `tags` des 15 tables d'arbre | 0 | `TAG_QH` sur `qh` seul ; `ga` à 0 | < 1 s |
| 8 | champs par sorte, noms communs, chemins enchâssés, casts | 0 | planète 37 + 19 = 56 ; sonde 21 ; 123 lignes `src/` / 324 TU ; casts 19 / 10 | < 1 s |
| 9 | membre `hd` + `pico8_hd_true` typés ; compilation ; compteur | 1 | 18 erreurs ; `void-ptr 7 3` ; pied 1072 / 84 | ≈ 2 s |
| 10 | membre `mm` + 2 cibles typés (une définition non réécrite par le `sed`, comptée) | 1 | 6 erreurs ; `void-ptr 6 4` ; pied 1071 / 85 | ≈ 2 s |
| 11 | tous les `void *self` de la table en `pico8_struct_header_t *` | 1 | 418 erreurs dans 44 fichiers | ≈ 2 s |
| 12 | inventaire `pico8_ptrs_t` de `src/` et lecteurs de `hv`/`cv`/`cg`/`ic` | 0 | 79 lignes / 35 fichiers, dont ≈ 35 / 15 porteuses d'entités | < 1 s |
| 13 | `hv`/`cv`/`cg` typés `pico8_ents_t` (`entity.c`, `entity.h`) | 1 | 20 erreurs dans 14 fichiers de `src/`, 375 dans 36 TU | ≈ 2 s |
| 14 | `git checkout -- .` ; compteur ; `git status --short` | 0 | pied 1073 / 85 ; copie propre | < 1 s |

### Critères d'échec (`FAILED:` pour le vérificateur)

- `FAILED:` si `docs/etude-2-9-archetypes.md` manque au départ et que l'étude n'est pas arrêtée ; ou si le départ n'est pas vérifié (H-1) et que l'étude ne l'explique pas.
- `FAILED:` si un fichier hors de `docs/etude-2-10-composants.md` et `docs/plan-3.md` change (le déplacement de ce PRD par le pilote excepté), ou si un essai a été fait dans l'arbre de travail plutôt que dans la copie.
- `FAILED:` si un contrôle de complétude (1 à 7) ne boucle pas.
- `FAILED:` si un groupe est rangé en archétype alors que 2.9 classe le champ en état sur un porteur, ou en composant sans deux racines porteuses, sans écrit qui le justifie.
- `FAILED:` si un remplaçant de `pico8_X_of`, `emitter_of`, `probe_of` ou d'une cible `void *` n'est qu'un renommage qui échappe au motif (même règle que `_dispatch`).
- `FAILED:` si la famille « descend de `qh` » est remplacée par un test d'étiquette sans la table d'équivalence du contrôle 4.
- `FAILED:` si la forme contredit la décision A (un type par sorte de planète, ou deux types pour la sonde et sa variante), sans question à l'humain.
- `FAILED:` si les registres `hv`, `cv`, `cg`, `ui.ic` ne tombent dans aucune catégorie essayée, si la catégorie n'est pas prouvée mordante sur copie, ou si son texte n'est pas versé en annexe.
- `FAILED:` si 2.14, 2.15 ou 2.16 n'a pas, dans le plan, ses sous-étapes, ses comptes d'arrivée, ses preuves et ses budgets ; si les registres n'y ont pas de sous-étape ; ou si le plan change ailleurs que sur 2.10 (note), 2.14 à 2.18, les sous-étapes « registres » et la ligne d'estimation.
- `FAILED:` si une question à l'humain dépasse 100 mots, emploie un nom minifié ou un numéro de TROU, ou porte sur un point qu'une règle ou un critère de ce PRD tranche.
- `FAILED:` si le tableau « Runs » manque, ou si la copie n'est pas supprimée à la fin de 2.10c.
- `FAILED:` si l'appli raylib est lancée.

## Out of Scope

- Toute modification de code, de TU, d'outil ou du `Makefile`. Ni composant, ni aplatissement, ni catégorie neuve n'est commis : ce sont 2.14 à 2.16 et les sous-étapes « registres ».
- Les décisions de 2.9 (classes des champs, forme, place et chemin de lecture des archétypes) : 2.10 les lit, ne les rouvre pas. Si un verdict de 2.10 les contredisait, l'étude le signale dans la note et pose la question.
- Les paires d'inclusion : étude C (2.11). 2.10 vérifie seulement, par le compteur relancé, qu'aucun de ses essais n'en crée.
- Le renommage des champs et des types (phase 3). L'appli raylib, l'audio, PSX.

## Further Notes

### Décisions de l'agent à la place de l'utilisateur (G)

- **G1. Le livrable est `docs/etude-2-10-composants.md`, pas `docs/prd/done/plan3-phase2-etape10-etude-composants.md`.** Même conflit et même règle que 2.9 et 2.11 (leur G1) : ce PRD prendra ce chemin dans `done/`, et `plan-pilot` le lit pour trouver l'étape suivante. La note « Découvert à 2.10 » corrige la sortie de la ligne 2.10.
- **G2. Les registres entrent dans 2.10.** La note de 2.8c le demande ; ils relèvent du même sujet que (b) (cibles et conteneurs encore en `void *`). Une étude à part aurait coûté un PRD de plus pour 50 min de travail.
- **G3. La règle 2 est lue avec la classe de 2.9.** À la lettre, tous les groupes Q6 sont composants (deux racines au moins). Un champ constant sur tous ses porteurs n'a pas d'état à porter : il se lit sur l'archétype. C'est l'ordre du plan (« lecture d'archétype ou composant »).
- **G4. Typer la table entière n'est pas exigé.** La Sortie compte les prototypes de premier niveau, pas les membres. L'étude choisit, chiffres à l'appui (18 et 6 erreurs par membre ciblé, 418 pour toute la table).
- **G5. La catégorie des registres est une condition de sortie.** Q1 range toutes les catégories ajoutées après D dans la Sortie ; la note de 2.8c dit « sortie de phase = tout à zéro ».
- **G6. Trois sous-étapes.** Les relevés sont scriptés (< 2 s chacun) ; ce qui prend du temps est la lecture de 2.9 pour chaque groupe, l'écriture des deux structs aplaties, l'écriture et l'essai de la catégorie neuve, et la réécriture de trois lignes du plan.

### Surprises de mesure, pour `plan-pilot`

1. **Budget** : le plan donne 2 × 50 min ; ce PRD en compte **3 × 50 min** (registres). La ligne « Estimation » gagne 1 sous-étape pour l'étude, plus les sous-étapes « registres » que l'étude place (≈ 3, estimé).
2. **Les étiquettes ne suivent pas la chaîne** : `TAG_QH` sur `qh` seul, `ga` sans étiquette. Un test d'étiquette ne remplace pas `pico8_qh_of`.
3. **`parent()` n'a que trois lecteurs**, tous des marches de descente : il tombe avec elles.
4. **Les 52 implémentations Q6 sont des lectures de champ** : 2.14 ne perd aucun calcul.
5. **Le typage des registres est lourd côté TU** (375 erreurs de première vague, 36 TU) et léger côté `src/` (20) ; il ne dépend pas de 2.12 à 2.15.
6. **Numéros du plan périmés** sur la ligne 2.10 (liste en tête).

### Questions pour l'humain

Aucune. Celles que l'étude trouvera sont posées par elle, dans son §5.

## Annexe — commandes de mesure (texte exact, depuis la racine de la copie)

Découpage d'`accesseurs-q6` :

```sh
sh tools/composition_residue.sh --check accesseurs-q6 > acc.txt
awk -F': ' '/^accesseurs/{t=substr($0,index($0,$2));
  if (t ~ /\.(get_[a-z_]+|set_m_) *=/) k="table";
  else if (t ~ /(->|\.)(get_[a-z_]+|set_m_)\(/) k="lecture";
  else if (t ~ /\/\*|:\s*\*/) k="commentaire";
  else if (t ~ /\(\*(get_|set_)/) k="membre";
  else if (t ~ /^[^:]*:[0-9]+:[a-z].*\(/) k="definition/proto";
  else k="autre (test de nullite)"; n[k]++} END{for(k in n) print k, n[k]}' acc.txt
```

Porteurs d'un champ, et membres de table :

```sh
sh tools/composition_residue.sh --champs > champs.tsv
awk -F'\t' -v f=fa '$2==f{print $1}' champs.tsv
for m in kj ev hd hk hb has_hb ldown rdown gv hq ey kf cn ol mm emitter_of probe_of; do
  grep -hE "^\s*\.$m *= *[a-z]" src/*.c | grep -v NULL | sed -E 's/.*= *([a-z0-9_]+).*/\1/' | sort -u | wc -l
done
```

Compilation d'essai (tout l'arbre) :

```sh
ls src/*.c tests/*.c | grep -vE 'main_raylib|main_screencast_viewer|raylib_backend|class_coverage' \
  | xargs -P4 -n8 cc -std=c99 -Wall -Wextra -Werror -Isrc -fsyntax-only 2>&1
for f in src/main_class_coverage.c tests/test_class_coverage.c; do
  cc -std=c99 -Wall -Wextra -Werror -Isrc -DPICO8_CLASS_COVERAGE -fsyntax-only "$f" 2>&1
done
```

Essai `hd` (run 9) :

```sh
sed -i 's/int (\*hd)(void \*self,/int (*hd)(pico8_struct_header_t *self,/' src/pico8_struct.h
sed -i 's/^int pico8_hd_true(void \*self,/int pico8_hd_true(pico8_struct_header_t *self,/' src/pico8_entity.h src/pico8_entity.c
```

Inventaire `pico8_ptrs_t` (run 12) :

```sh
grep -nE '\bpico8_ptrs_t\b' src/*.c src/*.h | grep -vE 'src/pico8_ptrs\.(c|h):|ents.h:6'
```
