# Étude 2.10 — règle 2, accesseurs, lien parent, aplatissement, registres

Étude B de la phase 2 (`docs/plan-3.md`, ligne 2.10), en lecture seule, faite selon le PRD `plan3-phase2-etape10-etude-composants.md`. Elle fixe comment les accesseurs Q6 et le lien parent disparaissent, quelle forme prennent la planète et la sonde aplaties, comment les cibles de table encore en `void *` sont typées, et comment les registres d'entités hors catégorie entrent dans le compteur. Elle écrit les listes ordonnées de 2.14 à 2.16 et des sous-étapes « registres ».

Départ : `d9a2b2d` (branche `claude/plan-pilot-3-7zdft0`), puis `0f6e495` (correction de l'étude 2.9, documentation seule) et `265be79` (report de cette correction dans le plan). Aucune ligne de code ne change entre ces trois commits : les numéros de ligne sont ceux de `d9a2b2d`. Les essais sont faits sur une seule copie jetable (`git clone` local dans le scratchpad), jamais dans l'arbre de travail ; chaque essai est remis à zéro (`git checkout -- .`, `git status --short` vide) avant le suivant.

## 0. État de départ

| Hypothèse | Mesure sur `d9a2b2d` | Verdict |
|---|---|---|
| H-1 | `parente` 10/3, `lien-parent` 111/27, `accesseurs-q6` 237/33, `défauts` 669/52, `void-ptr` 8/4, `inclusion-mutuelle` 17/16 ; pied `résidu 2 : 1073 ligne(s), 85 fichier(s)` ; `COMPOSITION_RESIDUE_GUARD` = `noyau-inclut,un-ou-faux,aiguillage-nom,atteinte-à-prouver,nu,ptrs-entité` (`Makefile:870`) ; `--champs` : `282 ; 15 ; 27` | vraie |
| H-2.9 | `docs/etude-2-9-archetypes.md` existe : (i) classes A/I/K/E/S/H (annexe A), (ii) chemin de lecture (ii), un `const pico8_X_def_t *def` dans l'instance, (iii) un type d'archétype par futur type aplati (`pico8_jo_def_t`, `pico8_g__def_t`), (iv) 2.12 et 2.13 réécrites | vraie. La correction `0f6e495` (vérificateur) ne reclasse aucun des champs lus par Q6 : elle touche `mz.jj` (S) et `hy.nz`/`ew`/`lo`/`yr` (I, archétype de `hy` dans `economy.h/.c`) |
| H-3 | `docs/etude-2-11-inclusions.md` n'existe pas | vraie |

Réserves du vérificateur de 2.9, reçues pendant l'étude et intégrées ici : chemin de lecture des sous-archétypes sonde et émetteur par le code générique (§2, « Sonde et émetteur ») ; champs des types partagés entre familles qui restent dans l'instance jusqu'en 2.13e2 (§2, « Ce que l'aplatissement trouve ») ; `mz.jj` statique (§1 n'en dépend pas) ; la sonde locale `pb` de `research_icon.c:180` (§2).

## 1. Règle 2 et accesseurs Q6 (2.10a)

### Découpage des 237 lignes

`composition_residue.sh --check accesseurs-q6`, trié par sorte de ligne (commande en annexe A) :

| Sorte | Lignes |
|---|---|
| entrée de table (`.get_X = …`, `.set_m_ = …`) | 124 |
| définition ou prototype d'implémentation | 68 |
| lecture (`->get_X(`, `->set_m_(`) | 28 |
| test de nullité (`->get_X != NULL`, `assert`) | 6 |
| membre de `struct.h` (`:142-151`) | 10 |
| commentaire (`struct.h:140`) | 1 |
| **total** | **237** |

Les 52 implémentations de `src/` sont toutes une lecture de champ, ou l'écriture `m_ = v != 0; has_m_ = 1` pour `set_m_`. Seule `pico8_g__get_fx` rend la constante 1 sans lire le champ. `get_fa`, `get_ql` et `get_mt` de `fr` rendent toujours 1 (présent) : sur un porteur, ces champs ne sont jamais nil. `get_mt` de `jc` et `get_fg` de `js` rendent 0 quand `has_mt`/`has_fg` est nul. `get_fx` et `get_p_` rendent `has_X && X`.

### Porteurs, classes de 2.9, verdict

Porteurs : les tables de classe qui ont l'entrée (`--champs` et tables relues, `tables.tsv` en annexe). L'arbre `jo` partage les implémentations de `jo`, la sonde `fv` celles de `g_` pour `fa`, `ra`, `p`, `fx`, `p_`. Classes : annexe A de l'étude 2.9.

| Groupe | Porteurs et classe de 2.9 | Racines | Règle 2 | Verdict |
|---|---|---|---|---|
| `fa` | A : `bg`, `fq`, `g_` (et `fv`), `ie`, `iv`, `jo` (et ses 9 sortes), `mh`, `mz`, `ui` ; **I** : `jc`, `js`, `pj` | 12 | deux racines au moins | **composant** : état (I) sur 3 porteurs |
| `ra` | I : `g_`, `ie`, `nf`, `qb` ; E : `iv`, `jo`, `js`, `mz` ; K : `jc_mouse_point`, `lq` | 10 | oui | **composant** : état partout |
| `p` | E : `g_`, `jo`, `mz` ; K : `jc_mouse_point` ; I : `qb` | 5 | oui | **composant** : état partout |
| `kq` | A : `fv`, `ie`, `jo` ; **E** : `js` | 4 | oui | **composant** : état sur `js` |
| `ql` | A : `fv`, `ui` | 2 | oui | **archétype** : A sur ses deux porteurs |
| `fg` | A : `fv` ; **K** : `js` (avec `has_fg`, K) | 2 | oui | **composant** : constante d'instance sur `js` |
| `mt` | A : `fr` ; **E** : `jc` (avec `has_mt`, E) | 2 (`fr` sous `jo`, `jc`) | oui | **composant** : état sur `jc` |
| `fx` | A : `g_` (lu comme 1) ; **E** : `jo` (avec `has_fx`, E) | 2 | oui | **composant** : état sur `jo` |
| `p_` | E : `fq`, `g_`, `iv`, `jc`, `jo`, `mh`, `mz` (avec `has_p_`, E) | 7 | oui | **composant** : état partout |
| `m_` | E : `bg`, `fq`, `fv`, `ie`, `jo`, `js` (avec `has_m_`, E) | 6 | oui | **composant** : état partout |

Règle qui range les groupes mêlés (PRD, G3) : un groupe présent dans deux racines est un composant dès qu'il porte de l'état (I, K ou E) sur un de ses porteurs, et se lit sur l'archétype seulement s'il est A partout. K compte comme état : c'est une valeur propre à l'instance. Aucun groupe n'est rangé en archétype contre 2.9 : `ql` est A sur ses deux porteurs, et c'est le seul.

### Ce qui remplace l'emplacement : un lien typé dans le noyau

Tous les lecteurs de Q6 sont **génériques** : ils tiennent un `pico8_struct_header_t *` dont ils ignorent le type (le seau `cv("kj")` de `dc`, le survolé `o_` de `ui`, l'ancrage `ij` de `mz`, une entité quelconque de `hv` pour `dg`). C'est la raison d'être de Q6. Le chemin de 2.9, un `const pico8_X_def_t *def` typé **dans l'instance**, ne leur est donc pas accessible sans un test de type suivi d'un cast. Les trois options du PRD, comparées :

| Critère | (R1) champ du noyau par valeur | (R2) lien du noyau vers le stockage | (R3) lecture d'archétype par le chemin de 2.9 |
|---|---|---|---|
| Ce que lit `dc` | `qc->fa` | `*qc->fa` | un test de classe par porteur, puis `((pico8_X_t *)qc)->def->fa` |
| Sémantique « absent » (`dc`, `ui.c:293`, `entity_geom.c:35`) | à ajouter (`has_fa` dans le noyau) | le lien NULL | le test de classe |
| Accord avec 2.9 | **non** pour un champ A : la valeur de l'archétype serait copiée dans chaque instance | oui : le lien vise `&def->X` pour un champ A, `&self->X` pour un champ d'état | oui |
| Accès des modules porteurs (`self->fa`, `self->jo.ra`…) | tous réécrits (`fa` : 130 lignes au grep, `ra` : 287) | inchangés | inchangés |
| Casts | aucun | aucun (sauf qualificatif, voir `ra`/`p`) | un cast descendant par porteur et par lecteur |
| Suit l'aplatissement de 2.15 | chaque accès réécrit deux fois (2.14 puis 2.15) | la ligne qui pose le lien est dans l'init de la famille, une fois | les casts changent de type à 2.15 |
| `void *`, paire d'inclusion, `parente` | aucun | aucun (essai `fa`) | les tests de classe nomment les tables des porteurs : paires possibles |
| Coût mémoire | la valeur dans chaque entité | un pointeur par groupe dans chaque entité | aucun |

**Retenu : (R2), pour tous les groupes.** Le noyau reçoit un membre par groupe, un pointeur typé vers le stockage que 2.9 fixe : le champ de `def` quand le champ est A sur ce type, le champ de l'instance sinon. NULL : le type ne porte pas le champ. Le lien est posé une fois par l'init, avec `def`. C'est le « composant optionnel porté par pointeur » du plan, où le composant est le stockage du champ, là où 2.9 l'a mis. (R1) contredirait 2.9 pour `fa`, `kq`, `fg`, `mt`, `fx`, et réécrirait deux fois les chemins de l'arbre `jo`. (R3) remettrait les casts que la phase retire. `ql`, rangé en archétype, prend la même forme : un lien vers `def->ql`. C'est la lecture d'archétype du code générique.

**Piège : les constructeurs ne mettent pas le noyau à zéro.** Les 29 `_new` font `malloc(sizeof *self)`. `cls` est posé par `pico8_ke_struct_init` (`ke.c:35`) et par sept sites hors `ke` (`economy.c:26`, `layout.c:305`, `:412`, `research.c:137`, `research_icon.c:55`, une vue locale, `slipway.c:131`, `trade.c:32`). Les dix instances statiques (`.cls = …` dans un initialiseur) ont leurs autres membres à zéro. Un lien jamais posé serait un pointeur indéterminé, que `dc` suivrait. L'essai `fa` est vert parce que `malloc` rend ici de la mémoire neuve, nulle. **2.14a crée donc une init du noyau unique, qui pose `cls` et met tous les liens à NULL, et que chaque constructeur appelle.** La preuve est un grep : plus aucune écriture de `cls` hors de cette init et des initialiseurs statiques. S'y ajoute un TU qui remplit de `0xa5` un bloc, y construit un non-porteur (`lq`, qui n'a aucun lien sauf `ra`) et vérifie ses liens NULL.

Forme du noyau après 2.14 (types et sens ; le texte exact est écrit par chaque sous-étape) :

```c
typedef struct {
    const pico8_struct_class_t *cls;
    const char *me;
    pico8_fixed_t t;
    /* Liens de composant : NULL, le type ne porte pas le champ. */
    const pico8_fixed_t *fa;          /* 2.14a */
    const pico8_vec_t *const *ra;     /* 2.14b */
    const pico8_vec_t *const *p;      /* 2.14b */
    const pico8_qz_box_t *kq;         /* 2.14c, la boîte elle-même */
    const pico8_fixed_t *ql;          /* 2.14c */
    const pico8_fixed_t *fg;          /* 2.14c */
    const pico8_fixed_t *mt;          /* 2.14c */
    pico8_fixed_t *ma;                /* 2.14c, écrit par pico8_ma */
    const pico8_fixed_t *fx;          /* 2.14d1 */
    int *p_;                          /* 2.14d1, écrit par la sonde */
    int *m_;                          /* 2.14d2, écrit par ui */
} pico8_struct_header_t;
```

Taille du noyau : 24 octets aujourd'hui sur x86-64, 112 après 2.14 et 128 après 2.15 (liens sonde et émetteur) ; sur la PSX (pointeurs de 4 octets), 12, puis 56, puis 64. Le compte des entités vivantes n'est pas mesuré ici (l'appli ne tourne pas). Les planètes et les boutons d'interface sont de l'ordre de la centaine : quelques kilo-octets.

### Le lien, groupe par groupe

- **`fa`** (2.14a). `&def->fa` pour les neuf classes A, `&self->fa` pour `jc`, `js`, `pj` (I). Jamais nil sur un porteur (toutes les implémentations rendent 1), donc le lien est fixe. Lecteurs : `dc` (`entity.c:204`), `pico8_gk` (`entity_geom.c:35-36`), `ui.c:293`, et les lectures croisées de `particle.c:25-29` (ci-dessous). Essai sur copie : voir plus bas.
- **`ra`, `p`** (2.14b). Le champ est déjà un pointeur (NULL : nil) que plusieurs porteurs réassignent (E). Le lien vise donc le **membre** : `&self->ra`, type `const pico8_vec_t *const *`. En C, `pico8_vec_t **` ne se convertit pas implicitement en `const pico8_vec_t *const *` (essai : `-Werror=incompatible-pointer-types`, gcc 13.3). Les membres passent donc en `const pico8_vec_t *` là où rien n'écrit à travers. Essai : `ra` de `iv`, `js`, `lq`, `mz`, `jc_mouse_point` et `p` de `g_`, `mz`, `jc_mouse_point` passés en `const` → **0 erreur** hors `nf`. `nf.ra` est écrit à travers (`ui.c:350`, `test_pico8_ui.c:443`, `test_pico8_ie.c:549` : 3 erreurs), `jo.ra` aussi (`pico8_na`, `entity_geom.c:66`, et remis mutable aux clés d'un enfant, `planet.c:398`, `:474` : 9 erreurs), `jo.p` de même (`planet.c:397` : 3 erreurs). Ces trois membres restent mutables. Leur lien est posé avec **un cast de qualificatif seul**, `(const pico8_vec_t *const *)&self->ra`, un par membre (3), avec le commentaire de contrat « lecture seule ; le module écrit par son membre ». Les I qui visent l'archétype (`g_`/`fv`, `ie`, `qb`) gardent `self->ra = &def->ra` (2.9) ; le lien vise leur membre, comme pour les autres.
- **`kq`** (2.14c). Le lien vise la boîte elle-même : `&def->kq` pour `fv`, `ie`, `jo` (A, boîte par valeur dans l'archétype). Sur `js`, `kq` est un pointeur (E) écrit à deux endroits seulement (`ui_button.c:112` NULL, `:211` `pico8_qz(…)`) et lu seulement par `js_get_kq` : le membre de `js` **monte dans le noyau**, `js` écrit `self->header.kq`. Un seul stockage, pas de copie à tenir à jour.
- **`ql`** (2.14c). `&def->ql` pour `fv` et `ui`. Lecteur `ui.c:249-250`.
- **`fg`** (2.14c). `&def->fg` pour `fv` ; pour `js` (K), `self->has_fg ? &self->fg : NULL`, posé une fois par le `_new` (`has_fg` est une clé de construction, jamais réécrite). Lecteur `ui.c:320-321`.
- **`mt`** (2.14c). `&def->mt` pour `fr` ; NULL pour les autres sortes (sur elles, `mt` est nil dans le cart ; le lien porte la présence que l'archétype planète, qui a un `mt` pour les dix sortes, ne dit pas). Pour `jc` (E, `has_mt` réécrit par `jc_hq`, `slipway.c:144`, et par le `_new`, `:419`), le lien est reposé à ces deux écritures : `self->header.mt = self->has_mt ? &self->mt : NULL`. En `src/`, `jc_mt_of` (`slipway.c:70`) ne lit `mt` que sur les bouts d'un `jc` : une planète ou le point souris (`jc_end_point`, `slipway.c:44-48`), jamais un `jc`. Le lien de `jc` n'est lu que par `test_pico8_jc.c:466`, `:469`. Il est gardé pour la fidélité au cart, où tout bout porterait son `mt`.
- **`ma`** (2.14c, hors Q6). Lecture croisée : `jo` et `qb`, deux racines, I sur les deux, écrit par `pico8_ma` (`entity_geom.c:85`). Lien mutable `&self->ma`.
- **`fx`** (2.14d1). `&def->fx` pour `g_`/`fv` (A, `0x10000`). Pour `jo` (E), `jo.fx` passe de `int` à `pico8_fixed_t` (0 ou 1.0, trois écritures : `planet.c:158`, `:407`, `planet_trade.c:64`) et le lien vise `&self->fx`. Lecture : `e->fx != NULL && *e->fx != 0`. `has_fx` n'a pas de lecteur hors `has_fx && fx` (`planet.c:99`, l'implémentation, et `planet_trade.c:63`) et `fx` vaut 0 chaque fois que `has_fx` vaut 0 (le `_new` pose les deux à 0, chaque écriture pose les deux à 1) : `has_fx` est retiré.
- **`p_`** (2.14d1). Lien **mutable** `&self->p_` pour les sept porteurs : `probe_host_raise_p_` (`probe.c:280-312`) écrit `p_` sur l'hôte d'une sonde par un aiguillage `pico8_g__of` / `pico8_jo_is_family` / `jc` / `mz`, que le lien réduit à `*self->p_ = 1`. `has_p_` n'est lu que par les sept implémentations (`has_p_ && p_`, grep : aucune autre lecture) : il est retiré, `p_` vaut 0 tant qu'il vaut 0.
- **`m_`** (2.14d2). Lien mutable `&self->m_` pour les six porteurs ; `ui` écrit `*o_->m_ = 1`. `has_m_` doit être retiré dans le même commit : ses trois lecteurs (`research_icon.c:178`, `ui_button.c:249`, `:273`) lisent `has_m_ && m_`, et une écriture par le lien qui laisserait `has_m_` à 0 les rendrait faux. Les deux `assert(… set_m_ != NULL …)` (`ui.c:306`, `:312`) deviennent `assert(… ->m_ != NULL …)` : le cart écrit `m_` sur n'importe quelle table, le port garde sa garde.

`--champs` après 2.14 : noyau +11 (les liens), `has_fx`, `has_p_` (7 types), `has_m_` (6 types) −14, `js.kq` −1 (monté dans le noyau). Les comptes absolus dépendent de 2.13 (les champs A quittent l'instance) : ils sont remesurés à chaque sous-étape.

### Essai sur copie : `fa`

Texte exact de l'essai en annexe B (`essai_fa.py`). Sur la copie, sans `def` (2.13 n'est pas faite), le lien vise `&self->fa` pour les douze porteurs, ce qui a le même type que `&def->fa`.

| Mesure | Avant | Après |
|---|---|---|
| `-fsyntax-only`, 150 + 2 fichiers | 0 erreur | 1 erreur (`test_pico8_mz.c:317`, variable `fa` devenue inutile), 0 après sa suppression |
| `accesseurs-q6` | 237 / 33 | **193 / 32** (−44, le compte calculé ci-dessous) |
| `parente`, `lien-parent`, `défauts`, `void-ptr` | 10, 111, 669, 8 | inchangés |
| `inclusion-mutuelle` | 17 / 16 | **17 / 16** : aucune paire neuve |
| pied | 1073 / 85 | 1029 / 85 |
| `make test` | — | vert (`timeout 900`, 9 s) |
| `make e2e` | — | **26 conformes** |

20 fichiers changés, 22 lignes ajoutées, 98 retirées. `git checkout -- .` ensuite, `git status --short` vide.

### Lectures croisées hors Q6

Un test de famille suivi d'un cast, pour lire ou écrire un champ sur une entité de type inconnu. `pico8_jo_is_family(` : 29 lignes dans 15 fichiers de `src/` (définition et prototype compris), dont 20 `assert` ; 25 lignes dans 21 TU.

| Site | Champ | Verdict | Sous-étape |
|---|---|---|---|
| `slipway.c:55` (`pico8_jc_end_p`) | `p` d'un bout (planète ou point souris) | lien `p` : `*end->p`, plus de test ni de `jc_end_point` | 2.14b |
| `slipway.c:64` (`pico8_jc_end_ra`) | `ra` d'un bout | lien `ra` | 2.14b |
| `particle.c:19-29` (`emitter_host_fa_ra`) | `fa`, `ra` de l'hôte d'un émetteur (`pico8_g__of`, puis `assert` et cast `jo`) | liens `fa` et `ra` ; la ligne `pico8_g__of` de `particle.c:19` sort de `lien-parent` | 2.14b |
| `entity_geom.c:79` (`pico8_ma`) | `ma` de planète ou de caméra `qb` | lien `ma` (règle 2 : deux racines, I sur les deux) | 2.14c |
| `probe.c:293` (`probe_host_raise_p_`) | `p_` de l'hôte | lien `p_` mutable ; la ligne `pico8_g__of` de `probe.c:283` sort de `lien-parent` | 2.14d1 |
| `entity_geom.c:62` (`pico8_na`) | `ra` de la planète, écrit à travers | **reste** : `na` est un état de la planète (`qh:ly`, `nl:ly`), son `self` est une planète ; cast vers le type planète (décision A) | — |
| `probe.c:92` (`probe_host_set_ra_p`) | réassigne `ra`/`p` de l'hôte (`g_`, planète, `mz`) | **reste** : le lien est une vue en lecture ; réassigner le membre d'un autre type par une vue `const` rendrait mutable ce qui vise l'archétype. Aiguillage par famille, dont la descente `pico8_g__of` (`probe.c:83`) est remplacée à 2.15 | 2.15 (descente) |
| `probe.c:113` (`pico8_probe_et_hang_fo`) | `fo` de la cible | **reste** : `fo` a quatre porteurs (`jo`, `jc`, `jc_too_far`, `bv_et`), mais pas un seul type (`jc_too_far.fo` est un `pico8_jc_t *`) ni un seul contrat (un `assert` par cible sur ce qu'elle accepte). Ce n'est pas un même groupe de champs : la règle 2 ne s'applique pas | — |
| `slipway_display.c:29` | `id` de la cible | **reste** : `id` n'a qu'un porteur (`jo`) ; cast vers le type planète | — |

`pico8_jo_is_family` lui-même survit à 2.14 (asserts, `na`, `probe.c:92`, `:113`, `slipway_display.c:29`) ; son sort est au §2.

### Liste ordonnée de 2.14

Comptes d'arrivée calculés sur le `--check` : chaque ligne est rattachée aux emplacements qu'elle nomme et sort avec le dernier (`tests/test_pico8_mz.c:446` nomme `get_ra` et `get_p`, `struct.h:140` nomme `get_fx`, `get_p_`, `set_m_`). Détail par ligne : annexe A.

| Sous-étape | Groupes | Lignes retirées | `accesseurs-q6` d'arrivée | Autres comptes |
|---|---|---|---|---|
| 2.14a | `fa`, et l'init du noyau unique | 44 | **193** | — |
| 2.14b | `ra`, `p` (membres en `const`, 3 casts de qualificatif) | 70 | **123** | `lien-parent` 111 → **110** (`particle.c:19`) |
| 2.14c | `kq`, `ql`, `fg`, `mt`, et le lien `ma` | 43 | **80** | — |
| 2.14d1 | `fx`, `p_` ; `has_fx`, `has_p_` retirés ; `jo.fx` en `pico8_fixed_t` | 48 | **32** | `lien-parent` → **109** (`probe.c:283`) |
| 2.14d2 | `m_` ; `has_m_` retiré ; commentaire `struct.h:139-141` | 32 | **0**, gardé | `void-ptr` 8 → **7** (`pico8_jo_set_m_`, `planet.h:122`) |

Somme : 44 + 70 + 43 + 48 + 32 = **237**.

Preuves de chaque sous-étape, en plus des preuves communes : chemin touché (`gcov` des lecteurs du groupe au parent puis après ; mutations : lien laissé NULL sur un porteur → TU ou e2e rouge ; lien d'un non-porteur posé → rouge où un lecteur observe l'absence, `dc` pour `fa`, `pico8_gk` pour `kq`, `jc_mt_of` pour `mt`) ; règle du parent de D.21 ; pour `fa`, `kq`, `ql`, `fg`, `mt`, `fx`, le lien d'un champ A vise `def` (grep de l'init) ; pour 2.14d1 et 2.14d2, le retrait de `has_X` est prouvé par le grep des lectures (aucune hors `has_X && X`) et par une mutation (`X` laissé à 1 au `_new`). `class-coverage-net` : **oui** à chaque sous-étape (les tables perdent des entrées ; comptes égaux à `tools/class_coverage_net_expected.txt`). `sanitize-net` : **oui** à chaque sous-étape (la disposition du noyau change). Budget ≈ 5 × 50 min.

Ordre : `fa` d'abord (le plus lu, et l'init du noyau que les autres liens réutilisent) ; `ra`/`p` avant `ma`, parce que `pico8_ma` et `pico8_gk` partagent `entity_geom.c` ; les drapeaux à la fin, parce que `m_` retire le dernier `void-ptr` du groupe et le commentaire commun.

## 2. Lien parent, aplatissement, cibles de table (2.10b)

*À écrire en 2.10b.*

## 3. Registres d'entités hors catégorie (2.10c)

*À écrire en 2.10c.*

## 4. Table du sort des symboles (contrat avec 2.11)

*À écrire en 2.10c.*

## 5. Questions à l'humain

*À écrire en 2.10c.*

## Annexe A — découpage d'`accesseurs-q6`

Commande (depuis la racine de la copie) :

```sh
sh tools/composition_residue.sh --check accesseurs-q6 > acc.txt
awk -F': ' '/^accesseurs/{t=substr($0,index($0,$2));
  if (t ~ /\.(get_[a-z_]+|set_m_) *=/) k="table";
  else if (t ~ /(->|\.)(get_[a-z_]+|set_m_)\(/) k="lecture";
  else if (t ~ /\/\*|:\s*\*/) k="commentaire";
  else if (t ~ /\(\*(get_|set_)/) k="membre";
  else if (t ~ /^[^:]*:[0-9]+:[a-z].*\(/) k="definition/proto";
  else k="autre"; print k"\t"t}' acc.txt > acc.kind
```

« autre » rend 7 lignes : les 6 tests de nullité et le commentaire `struct.h:140` (que le motif « commentaire » de l'awk du PRD manque). Le rattachement à une sous-étape prend, pour chaque ligne, les emplacements qu'elle nomme (`(\b|_)(get_(fa|ra|p|kq|ql|fg|mt|fx|p_)|set_m_)\b`) et retient la dernière sous-étape de ces emplacements.

Lignes par sous-étape (`T:` = `tests/test_pico8_`, sans `src/pico8_` ; « 2.14d » = 2.14d1 pour `fx`/`p_`, 2.14d2 pour `m_` et le commentaire) :

**2.14a** (44 lignes)

| Sorte | Emplacement | Lignes |
|---|---|---|
| test de nullité / commentaire | `fa` | 1 : entity_geom.c:35 |
| definition/proto | `fa` | 15 : bg.c:44; msgbox.c:231,88; planet.c:79; planet.h:116; probe.c:399; probe.h:62; research_icon.c:205; score_panel.c:275; slipway.c:365; ui.c:77; ui_button.c:326,61; wormhole.c:203; T:ui.c:742 |
| lecture | `fa` | 4 : entity.c:204; entity_geom.c:36; ui.c:293; T:mz.c:331 |
| membre | `fa` | 1 : struct.h:142 |
| table | `fa` | 23 : bg.c:62; help.c:135; msgbox.c:102,249; planet.c:126; planet_family.c:142,201,278,382,53,556; planet_family_trade.c:181,253,360; probe.c:433; research_icon.c:230; score_panel.c:285; slipway.c:393; ui.c:89; ui_button.c:347,99; wormhole.c:234; T:ui.c:757 |

**2.14b** (70 lignes)

| Sorte | Emplacement | Lignes |
|---|---|---|
| definition/proto | `p` | 7 : planet.c:88; planet.h:118; probe.c:408; probe.h:64; scroll.c:34; slipway.c:36; wormhole.c:212 |
| lecture | `p` | 4 : entity_geom.c:23; T:g_.c:350; T:jc.c:769; T:qb.c:368 |
| membre | `p` | 1 : struct.h:144 |
| table | `p` | 15 : help.c:137; planet.c:128; planet_family.c:144,203,280,384,558,55; planet_family_trade.c:183,255,362; probe.c:435; scroll.c:41; slipway.c:42; wormhole.c:236 |
| lecture | `p+ra` | 1 : T:mz.c:446 |
| test de nullité / commentaire | `ra` | 1 : wormhole_display.c:14 |
| definition/proto | `ra` | 13 : layout.c:291; mouse_reader.c:27; msgbox.c:236; planet.c:84; planet.h:117; probe.c:404; probe.h:63; research_icon.c:210; scroll.c:30; slipway.c:32; ui_button.c:66; wormhole.c:208; T:mo.c:83 |
| lecture | `ra` | 6 : entity.c:230; entity_geom.c:17; wormhole_display.c:15; T:lq.c:338; T:nf.c:282; T:qb.c:367 |
| membre | `ra` | 1 : struct.h:143 |
| table | `ra` | 21 : help.c:136; layout.c:297; mouse_reader.c:33; msgbox.c:250; planet.c:127; planet_family.c:143,202,279,383,54,557; planet_family_trade.c:182,254,361; probe.c:434; research_icon.c:231; scroll.c:40; slipway.c:41; ui_button.c:100; wormhole.c:235; T:mo.c:89 |

**2.14c** (43 lignes)

| Sorte | Emplacement | Lignes |
|---|---|---|
| test de nullité / commentaire | `fg` | 1 : ui.c:320 |
| definition/proto | `fg` | 2 : help.c:112; ui_button.c:74 |
| lecture | `fg` | 1 : ui.c:321 |
| membre | `fg` | 1 : struct.h:147 |
| table | `fg` | 2 : help.c:140; ui_button.c:102 |
| definition/proto | `kq` | 5 : help.c:103; planet.c:92; planet.h:119; research_icon.c:214; ui_button.c:70 |
| lecture | `kq` | 1 : entity_geom.c:40 |
| membre | `kq` | 1 : struct.h:145 |
| table | `kq` | 13 : help.c:138; planet.c:129; planet_family.c:145,204,281,385,559,56; planet_family_trade.c:184,256,363; research_icon.c:232; ui_button.c:101 |
| definition/proto | `mt` | 2 : planet_family_trade.c:228; slipway.c:370 |
| lecture | `mt` | 3 : slipway.c:70; T:jc.c:466,469 |
| membre | `mt` | 1 : struct.h:148 |
| table | `mt` | 2 : planet_family_trade.c:259; slipway.c:394 |
| test de nullité / commentaire | `ql` | 1 : ui.c:249 |
| definition/proto | `ql` | 2 : help.c:107; ui.c:82 |
| lecture | `ql` | 2 : ui.c:250; T:ui.c:322 |
| membre | `ql` | 1 : struct.h:146 |
| table | `ql` | 2 : help.c:139; ui.c:90 |

**2.14d** (80 lignes)

| Sorte | Emplacement | Lignes |
|---|---|---|
| definition/proto | `fx` | 4 : planet.c:96; planet.h:120; probe.c:413; probe.h:65 |
| lecture | `fx` | 1 : wormhole.c:45 |
| membre | `fx` | 1 : struct.h:149 |
| table | `fx` | 12 : help.c:141; planet.c:130; planet_family.c:146,205,282,386,560,57; planet_family_trade.c:185,257,364; probe.c:436 |
| test de nullité / commentaire | `fx+m_+p_` | 1 : struct.h:140 |
| test de nullité / commentaire | `m_` | 2 : ui.c:306,312 |
| definition/proto | `m_` | 8 : bg.c:49; help.c:117; planet.c:108; planet.h:122; research_icon.c:218; ui_button.c:337,84; T:ui.c:748 |
| lecture | `m_` | 4 : ui.c:308,314; T:ie.c:499,501 |
| membre | `m_` | 1 : struct.h:151 |
| table | `m_` | 16 : bg.c:63; help.c:143; planet.c:132; planet_family.c:148,207,284,388,562,59; planet_family_trade.c:187,260,366; research_icon.c:233; ui_button.c:103,349; T:ui.c:758 |
| definition/proto | `p_` | 10 : msgbox.c:240,93; planet.c:102; planet.h:121; probe.c:418; probe.h:66; slipway.c:380; ui_button.c:331; wormhole.c:216; T:iv.c:198 |
| lecture | `p_` | 1 : bootstrap.c:70 |
| membre | `p_` | 1 : struct.h:150 |
| table | `p_` | 18 : help.c:142; msgbox.c:103,251; planet.c:131; planet_family.c:147,206,283,387,561,58; planet_family_trade.c:186,258,365; probe.c:437; slipway.c:395; ui_button.c:348; wormhole.c:237; T:iv.c:206 |

## Annexe B — essais

Compilation d'essai (`syn.sh`, depuis le scratchpad, argument : la copie) :

```sh
#!/bin/sh
# compile tout l'arbre en -fsyntax-only ; rend les lignes error:
cd "$1"
ls src/*.c tests/*.c | grep -vE 'main_raylib|main_screencast_viewer|raylib_backend|class_coverage' \
  | xargs -P4 -n8 cc -std=c99 -Wall -Wextra -Werror -Isrc -fsyntax-only 2>&1
for f in src/main_class_coverage.c tests/test_class_coverage.c; do
  cc -std=c99 -Wall -Wextra -Werror -Isrc -DPICO8_CLASS_COVERAGE -fsyntax-only "$f" 2>&1
done
```

Essai `fa` (`essai_fa.py`, argument : la copie ; le lien vise `&self->fa`, `def` n'existant pas encore) :

```python
import re,sys,os
os.chdir(sys.argv[1])
def sub(path, old, new, count=1):
    s=open(path).read()
    assert s.count(old)>=1, (path, old)
    s=s.replace(old,new) if count==0 else s.replace(old,new,count)
    open(path,'w').write(s)
# 1. header member, table member gone
sub('src/pico8_struct.h','    pico8_fixed_t t;\n} pico8_struct_header_t;','    pico8_fixed_t t;\n    const pico8_fixed_t *fa;\n} pico8_struct_header_t;')
sub('src/pico8_struct.h','    int (*get_fa)(const void *self, pico8_fixed_t *out);\n','')
# 2. table entries
import glob
for f in glob.glob('src/*.c')+glob.glob('tests/*.c'):
    s=open(f).read(); n=re.sub(r'\n[ \t]*\.get_fa *= *[a-z0-9_]+,', '', s)
    if n!=s: open(f,'w').write(n)
# 3. impls (bodies end at first "\n}\n")
def drop_fn(path, sig):
    s=open(path).read(); i=s.index(sig); j=s.index('\n}\n',i)+3
    if s[j:j+1]=='\n': j+=1
    open(path,'w').write(s[:i]+s[j:])
for path,name in [('src/pico8_bg.c','static int bg_get_fa('),('src/pico8_msgbox.c','static int mh_get_fa('),('src/pico8_msgbox.c','static int iv_get_fa('),
 ('src/pico8_planet.c','int pico8_jo_get_fa('),('src/pico8_probe.c','int pico8_g__get_fa('),('src/pico8_research_icon.c','static int ie_get_fa('),
 ('src/pico8_score_panel.c','static int pj_get_fa('),('src/pico8_slipway.c','static int jc_get_fa('),('src/pico8_ui.c','static int ui_get_fa('),
 ('src/pico8_ui_button.c','static int js_get_fa('),('src/pico8_ui_button.c','static int fq_get_fa('),('src/pico8_wormhole.c','static int mz_get_fa('),
 ('tests/test_pico8_ui.c','static int giver_get_fa(')]:
    drop_fn(path,name)
sub('src/pico8_planet.h','int pico8_jo_get_fa(const void *self, pico8_fixed_t *out);\n','')
sub('src/pico8_probe.h','int pico8_g__get_fa(const void *self, pico8_fixed_t *out);\n','')
# 4. storage link at construction (trial: &self->fa ; after 2.13, &def->fa for class A)
for path,start in [('src/pico8_bg.c','pico8_bg_start.fa;'),('src/pico8_ui_button.c','pico8_fq_start.fa;'),('src/pico8_probe.c','pico8_g__start.fa;'),
 ('src/pico8_research_icon.c','pico8_ie_start.fa;'),('src/pico8_msgbox.c','pico8_iv_start.fa;'),('src/pico8_slipway.c','pico8_jc_start.fa;'),
 ('src/pico8_planet.c','pico8_jo_start.fa;'),('src/pico8_ui_button.c','pico8_js_start.fa;'),('src/pico8_msgbox.c','pico8_mh_start.fa;'),
 ('src/pico8_wormhole.c','pico8_mz_start.fa;'),('src/pico8_score_panel.c','pico8_pj_start.fa;'),('src/pico8_ui.c','pico8_ui_start.fa;')]:
    sub(path,'self->fa = '+start,'self->fa = '+start+'\n    self->header.fa = &self->fa;')
# 5. readers
sub('src/pico8_entity.c','if (qc->cls->get_fa == NULL || !qc->cls->get_fa(qc, &fa)) {','if (qc->fa == NULL) {')
sub('src/pico8_entity.c','            layer = pico8_fixed_flr_int(fa);','            fa = *qc->fa;\n            layer = pico8_fixed_flr_int(fa);')
sub('src/pico8_entity_geom.c','    if (e != NULL && e->cls->get_fa != NULL) {\n        e->cls->get_fa(e, &fa);\n','    if (e != NULL && e->fa != NULL) {\n        fa = *e->fa;\n')
sub('src/pico8_ui.c','if (e->cls->get_fa != NULL && e->cls->get_fa(e, &fa_val)) {','if (e->fa != NULL) {\n                    fa_val = *e->fa;')
sub('tests/test_pico8_mz.c','assert(mz->header.cls->get_fa(mz, &fa) && fa == mz->fa);','assert(mz->header.fa == &mz->fa);')
sub('tests/test_pico8_ui.c','static giver_t g_giver = {{.cls = &g_giver_class}};','static const pico8_fixed_t g_giver_fa = 0xf0000;\nstatic giver_t g_giver = {{.cls = &g_giver_class, .fa = &g_giver_fa}};')
```

Essais `const` de 2.14b (runs 11 et 12) : `sed -i "<ligne>s/^    pico8_vec_t \*/    const pico8_vec_t */"` sur `layout.h:67` (`lq.ra`), `mouse_reader.h:19` (`nf.ra`), `msgbox.h:87` (`iv.ra`), `ui_button.h:36` (`js.ra`), `slipway.h:68-69` (`jc_mouse_point`), `wormhole.h:37-38` (`mz`), `probe.h:47` (`g_.p`) ; puis `planet.h:41` (`jo.p`) et `planet.h:42` (`jo.ra`), un à la fois.

## Annexe C — tableau « Runs »

Sur la copie jetable, depuis le scratchpad `etude210/`.

| # | Commande | Code | Ligne clé | Durée |
|---|---|---|---|---|
| 1 | `git clone` local de `d9a2b2d` dans le scratchpad ; `sh tools/composition_residue.sh` | 0 | `parente 10 3`, `lien-parent 111 27`, `accesseurs-q6 237 33`, `défauts 669 52`, `void-ptr 8 4`, `inclusion-mutuelle 17 16` ; `résidu 2 : 1073 ligne(s), 85 fichier(s)` | 0,8 s |
| 2 | `--champs` ; `grep COMPOSITION_RESIDUE_GUARD Makefile` ; `ls docs/etude-2-11*` | 0 / 0 / 2 | `champs : 282 ; 0 écriture : 15 ; 0 hors constructeur : 27` ; garde de six catégories ; pas de 2.11 | < 1 s |
| 3 | `syn.sh` (150 `.c` + 2 sous `-DPICO8_CLASS_COVERAGE`), arbre intact | 0 | 0 `error:` | 0,5 s |
| 4 | `--check accesseurs-q6`, découpage (annexe A) | 1 | 124 table, 68 définition/prototype, 28 lecture, 6 test de nullité, 10 membre, 1 commentaire | < 1 s |
| 5 | entrées de table par classe (`tables.tsv`) ; corps des 52 implémentations | 0 | porteurs du §1 ; toutes triviales sauf `pico8_g__get_fx` | < 1 s |
| 6 | essai `fa` (`essai_fa.py`) ; `syn.sh` ; compteur | 0 | 1 erreur (`test_pico8_mz.c:317`, variable inutile) ; `accesseurs-q6 193 32`, autres inchangées, `inclusion-mutuelle 17 16` ; pied 1029 / 85 | 1 s |
| 7 | essai `fa` corrigé ; `syn.sh` ; `timeout 900 make test` | 0 | 0 erreur ; `make test` vert | 9 s |
| 8 | essai `fa` ; `timeout 900 make e2e` | 0 | `e2e: 26 e2e dans tests/e2e`, 26 `conforme` | 0,7 s |
| 9 | `git checkout -- .` ; `git status --short` | 0 | copie propre | < 1 s |
| 10 | conversion `pico8_vec_t **` → `const pico8_vec_t *const *` (fichier d'essai hors copie, gcc 13.3) | 1 | `-Werror=incompatible-pointer-types` | < 1 s |
| 11 | membres `ra` de `iv`, `js`, `lq`, `mz`, `jc_mouse_point`, `nf` et `p` de `g_`, `mz`, `jc_mouse_point` en `const` ; `syn.sh` | 1 | 3 erreurs, toutes sur `nf.ra` (`ui.c:350`, `test_pico8_ui.c:443`, `test_pico8_ie.c:549`) | 0,5 s |
| 12 | `jo.p` en `const` ; puis `jo.ra` en `const` ; `syn.sh` ; remise à zéro | 1 / 1 | 3 erreurs (`planet.c:397`…) ; 9 erreurs (`entity_geom.c`, `planet.c` ×2, 5 TU) | 1 s |
| 13 | lectures de `has_p_`, `has_m_`, `has_fx` hors `has_X && X` (grep) | 1 | aucune | < 1 s |
| 14 | `sizeof(pico8_struct_header_t)` et des noyaux à 11 et 13 liens (x86-64) | 0 | 24, 112, 128 | < 1 s |
