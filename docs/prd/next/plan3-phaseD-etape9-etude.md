# Plan 3, étape D.9 — étude sans conversion : `par-nom`, `dispatch`, `pico8_list_t`

Réponse à l'arrêt « champ sans règle » constaté à l'écriture de la liste de D (`docs/plan-3.md`, paragraphe qui précède D.9). Aucun code n'est changé, et `docs/plan-3.md` non plus : ce fichier propose la section à y ajouter. L'humain tranche les décisions de la dernière partie avant la première étape qui les applique.

**Mesuré sur `2dec71f`** (D.6c gradée, PRD de D.7a en triage), dans une copie `git archive 2dec71f` du scratchpad. Seuls `sh tools/data_residue.sh` (avec `--detail`) et des `grep` ont tourné. Rien n'a été compilé ni lancé. Le classement des sites vient de la lecture des sources et, pour les clés dynamiques, de `pico8/lua.lua`.

Départ (`make data-residue`, lignes/fichiers) : `valeur` 5422/133, `ob` 1277/102, `stash` 28/4, `type-tag` 403/69, `ref-audit` 68/17, **`par-nom` 1038/81** (src 462/43, tests 576/38), **`dispatch` 199/60** (src 76/29, tests 123/31), `overrides` 30/8, `spec` 0 ; résidu D 7385 lignes / 140 fichiers. `pico8_list_t` n'a pas de catégorie. Avec le motif proposé plus bas (Q10), il compterait **987/101** (src 574/63, tests 413/38).

Unités. Une **ligne** est une ligne comptée par `data_residue.sh`. Un **site** est un appel qui nomme un champ ou une méthode. Les aides à nom en paramètre (`field_fixed(inst, name)` et semblables) ne sont comptées par `par-nom` qu'à leur définition. Leurs **82 lignes d'appel**, toutes à nom littéral, sont recensées à part, parce qu'elles sont aussi des lectures par nom.

---

## 1. `par-nom`

### 1.1 Inventaire, `src/` (462 lignes)

**Infrastructure : 298 lignes.** Elle meurt quand plus aucun consommateur par nom ne l'atteint :

| Nature | Lignes | Objets |
|---|---|---|
| lecteurs `field` des descripteurs | 65 | 40 fonctions `pico8_X_field` (dont 3 remplaçants : `mo`, `je`, `fm`) |
| écrivains `set_field` | 74 | 18 fonctions `pico8_X_set_field` |
| aides `*_take_*` (définitions) | 57 | 55 aides `static` |
| lecteurs de sorte (`kind_field`, D.5c) | 19 | 10 fonctions (`jo`, `qh`, `nl`, `os`, `hj`, `pm`, `ga`, `nr`, `fr`, `mx`) |
| emplacements `field`/`set_field` des 40 descripteurs et des 11 sortes | 69 | `pico8_struct_class_t`, `pico8_entity_kind_t` |
| en-têtes (typedefs `pico8_struct_field_fn`/`_set_field_fn`, prototypes, commentaires) | 14 | `pico8_struct.h`, `pico8_entity.h`, `pico8_ke.h`… |

**Sites : 164 lignes comptées et 82 lignes d'appel d'aides**, classées par forme de remplacement :

| Classe | Forme retenue | Lignes comptées | Appels d'aides | Fichiers |
|---|---|---|---|---|
| **A** — receveur de type connu : `self` typé, membre pointeur typé (`self->lq`, `self->qu`, `dk`), ou élément d'une famille fixée par le cart (`cg.jo`, `jo.by`/`cc` → `trade`, `trade.rl` → `jc`), `me`/`t` de `ke` sur `hv` | accès au membre typé (`pt->mi`, `pt->has_mi`), avec un cast vérifié de l'en-tête vers la famille | 41 | 36 | 18 |
| **B** — membre d'une fille lu sur la famille `jo` (`qh`, `nl`, `hp`, `px`, `s`, `ba`, `qw`) | test d'appartenance puis accès typé : `pico8_qh_of(jo)` rend NULL hors de la lignée (Q5) | 7 | 1 | 5 |
| **C** — champ transversal lu sur une entité quelconque (`hv`, `cv`, calques de `dc`, `ui.ic`, `ui.o_`/`gj`, `jc.et`, `jo.fo`) : `p_`, `fa`, `ra`, `nu`, `ma`, `kq`, `dw`, `m_`, `ql`, `fg`, présence de `gv` et des cinq noms `bm` | accesseur typé par champ dans la table de la classe, NULL = absent (Q6) | 16 | 6 | 6 |
| **D** — singleton à remplaçant : `fm` (remplaçant `{jk,nz}` ou `hy`), `je` (remplaçant ou `nf`), `mo` (remplaçant ou `qb`) | accesseur de singleton typé (`pico8_fm_hy()`, `pico8_je_nf()`, `pico8_mo_qb()`, NULL sur le remplaçant) ; le remplaçant de `fm` reste une struct typée pour `jk`/`nz` | 8 | 21 | 11 |
| **E** — nom variable | `pico8_mv(e, method_name)` : disparaît avec le point 2 ; `pico8_ui_je_button_table(g_je, bt)`, `bt` ∈ {`l`,`r`} : deux accès (`nf.l`, `nf.r`), typés par D.7a | 2 | — | 2 |
| **F** — vue lue par `pz` : `p` peut être la vue `ha_pz`, `fv`, `jo`, `pj`… ; lit `ep`, `eo`, `hi`, `ff`, `ca`, `gq`, `ra` | struct locale de cadre (`{ep, eo, hi, ff, ca, gq, ra}`), remplie par chacun des cinq appelants depuis ses membres typés | 5 | 2 | 1 |
| **G** — `*_take_*` hors descripteur, qui déballe un argument ou un retour de méthode (`rp`, `me` de `ks`, `kn`, `mt`, résultat de `my`, `g_ie_gy`) | disparaît avec le point 2 : l'argument arrive typé | 9 | — | 6 |
| **H** — définitions des aides par nom (`field_truthy`, `field_fixed`, `pico8_probe_field_*`, `fm_field_*`, `pico8_ui_field_*`…, 27 aides dans 15 fichiers) | disparaissent avec leurs appelants (A, B, C, D, M) | 39 | — | 15 |
| **K** — lecture des sortes : `pico8_ie_ke_view` lit `rr`, `s`, `me`, `od` par `kind->field` | enregistrement typé par sorte (`{rr, has_s, s, me, has_od, od}`), résolu le long de la chaîne à la génération, pointé par `pico8_entity_kind_t` | 4 | — | 1 |
| **M** — groupe partagé entre branches : émetteur de particules (`ei`, `ez`, `f_`, `g__mm` : `pn`, `t`, `du`, `py`, `ki`, `pk`, `qy`, `fa`, `ra`, `qo`, `qg`, `ro`, `q_`, `k_`, `fc`, sur `g_`, `fv`, `hj`, `pm`, `mx`, `ga`) et sonde (`bv`, `dl`, `d_` : `ra`, `p`, `et`, `fo`, `me`, `od`, `dq`, `fp`, `ik`, `p_`, `dw`, `qs`, sur `g_`, `fv`, les huit classes de l'arbre `qh`, `mz` pour `dl`) | **aucune forme de D** : c'est la règle 2 de la phase 2 (composant). Voir Q7 | 16 | 16 | 2 |
| **X** — pont `js`/`lq` (`pico8_js_new_table`, `pico8_lq_new_table`) | **meurt avec D.7a** (plus les 12 définitions `button_take_*`/`lq_take_*` de l'infrastructure) | 17 | — | 2 |
| **Total** | | **164** | **82** | |

Genres lus : fixes (`mi`, `jh`, `hs`, `t`, `fa`, `ma`, `dw`, `en`, `trade`, `od`, `s`, `qw`…), booléens (`p_`, `m_`, `fx`), `hf` (booléen ou fixe, TROU-3), chaînes (`me`, `px`, `hl`, `id`), REF (listes `jy`/`cc`/`by`/`lp`/`rl`/`oe`/`qy`/`invented`, `kw`, `hn`, `hg`, `nk`, `po`, vecteurs `ra`/`p`, entités `qu`/`re`/`et`/`fo`/`o_`). Écritures : `kr`, `mi`, `cc`, `by`, `lq`, `ma`, `v`, `m_`, `fo`, `qy`, `ra`, `p`, `et`, `p_`.

Détail des sites (classe, fichier, fonction, `R:` lu / `W:` écrit) : annexe A.

### 1.2 Inventaire, `tests/` (576 lignes, 38 fichiers)

`pico8_entity_get_field` : 248 lignes. `pico8_entity_set_field` : 200. Appels directs des descripteurs `cls->field`/`->set_field` : 145. Lectures de sorte : 3. Les noms les plus lus sont `fo` 31, `m_` 26, `p_` 21, `p` 17, `ra` 15, `et` 14. Deux formes :

- **T1** — mise en état et lecture d'état d'un TU : accès aux membres typés (`qh->jo.mi = …; qh->jo.has_mi = 1;`). Le TU connaît son type.
- **T2** — tests des descripteurs eux-mêmes (lecture par nom, refus d'un genre hors rangée, `assert` « champ sans règle » en mort) : leur objet disparaît avec les descripteurs. Ils sont supprimés comme tests de la couche (§6, premier point ; Q11).
- §6 : `test_pico8_planet_gh_row_lq` (3 lignes) et `test_pico8_ga_export_panel_by` (3 lignes) sont réécrits pour lire les membres, avec leur preuve rouge.

### 1.3 Ce qui tombe déjà avec D.7a–D.8 (à ne pas compter deux fois)

- D.7a : classe X (17 lignes) plus 12 définitions (`button_take_fa/vec/my/function/fixed/string/nb/jo/hp`, `lq_take_fixed/bg/vec`), soit **≈ 29 lignes**. H-D6c-5 en estimait 24 ; D.7a donne le compte réel. La lecture par nom de `je.l`/`je.r` reste (le PRD de D.7a le dit).
- D.8 : `tests/test_pico8_map_tables.c`, 1 ligne.
- D.7b n'enlève aucun site par nom : sa ligne ne porte que `ob`.
- **Départ attendu de la suite : `par-nom` ≈ 1008** (src ≈ 433, dont 286 d'infrastructure et 147 de sites ; tests 575).

---

## 2. `dispatch`

### 2.1 Inventaire

- **Table de méthodes** : `pico8_value_function_t methods[23]` dans `pico8_struct_class_t`, 196 entrées dans les 40 descripteurs. Elles pointent **81 adaptateurs** à signature `(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1)`. Ceux-ci sont comptés par `valeur`, pas par `dispatch` ; la plupart enveloppent déjà une fonction typée `pico8_X_Y`.
- **`pico8_mv`** : 26 appels dans `src/`, plus sa définition et sa déclaration.
- **Méthodes comme valeurs** : `pico8_entity_get_field` rend une méthode comme un champ (`entity.c:176-177`). `lt`/`he` inscrivent dans `cv` sur cette présence. `mv` appelle ce qu'il lit.
- **Tests d'appartenance par pointeur de méthode** : 7 prédicats `*_is_*_class` comparent `methods[HQ]` à `pico8_bv_dispatch` ou testent `methods[LY]`.
- **Membres fonction** (rappels) : `js.my`/`hb`, `fq.my`, `mh.lh`, `hp.lq` (membre donnée, ou méthode quand il est NULL), les `my`/`hb` des rangées de `menu`, `jo:fe`, `de.my`, `eu` de `help`, `init` de `score_panel`.

Les 76 lignes de `src/` se répartissent ainsi :

| Groupe | Lignes |
|---|---|
| appels `pico8_mv`, définition, déclaration, commentaire (`economy_score.h`) | 29 |
| méthode rendue comme valeur par un lecteur (`entity.c:177`, `msgbox.c:134`, `research.c:178`, `ui_button.c:89,159,668`) | 6 |
| rappels (membres, clés, rangées, `eu`/`init`, leurs en-têtes) | 33 |
| déclarations du type (`pico8_value.h` 5, `pico8_value.c` 1, `pico8_struct.h` 1, `pico8_dispatch_context.h` 1) | 8 |

Tests : 123 lignes (98 `pico8_mv`, 25 `pico8_value_function`), 31 fichiers.

### 2.2 Méthodes : implémentations, appelants, signature typée

| Méthode | Impl. distinctes | Appelant(s) | Arguments observés | Retour observé | Forme |
|---|---|---|---|---|---|
| `kj` | 12 | `dc("kj")` | `ra` (vecteur ou nil) | ignoré | virtuelle `void (*kj)(void *, const pico8_vec_t *ra)` |
| `ev` | 5 | `dc("ev")` | `ra` | ignoré | virtuelle, même forme |
| `hd` | 4, plus `hd=1` sur `bg` et `fq` (valeur) | `ui:gw` | `qa`, `je.ra` | vérité | virtuelle `int (*hd)(…)` ; `hd=1` devient une fonction constante vraie (seule la vérité est lue, et la présence inscrit dans `cv.hd` comme aujourd'hui) |
| `hk` | 2 | `hy:hk` (`cv.hk`) | `yr` fixe | ignoré | virtuelle |
| `hb` | 4, plus le rappel `js.hb` | `hy:lj` (`cv.hb`) | aucun | ignoré | virtuelle ; `js.hb` est un rappel de même signature |
| `ldown`/`rdown` | 6 / 1 | `ui:jz(down)` | `qa`, `je.ra` | trois états : faux (on continue), vrai non-REF (on s'arrête sans rien tenir), REF (on tient l'entité) | virtuelle ; le retour devient un petit struct `{stop, held}` |
| `gv` | 2 | `ui:jz("gv")` | l'entité tenue | idem | idem |
| `hq` | 4 (dont `bv`, partagé) | `ui:bh` | `qa`, `je.ra` | ignoré | virtuelle |
| `ey` | 3 (dont `d_`) | `ui:bh` | un argument | ignoré | virtuelle |
| `kf` | 2 | `jc:ey` | le slipway | présence seulement | virtuelle ; NULL = absent |
| `cn` | 2 | `jo:ib`, `nr:cn` | `&proposition` | ignoré | virtuelle |
| `ol` | 2 (`lq`, `jb`) | `jo:kj` sur `self->lq` | décalage, `a` | ignoré | virtuelle |
| `mm` | 2 (`g__mm`, `ga_mm`) | `ez` | aucun | la particule | virtuelle (classe M pour le receveur) |
| `eb`, `ib`, `gg`, `md`, `lq` (`hp`), `lh` (`mh`) | 1 | 5 / 1 / 1 / 1 / 1 / 1 sites | — | — | **appel direct** de la fonction typée : plus de table (`gg` : seule la vérité est lue à ce site ; `pico8_jo_gg` garde son retour, TROU-3) |
| états `ph`, `ly`, `ll`, `kz`, `pr` | 5 / 2 / 1 / 1 / 1 | `dg` : `qc[qc.me](qc, qc.t)` | `t` fixe | ignoré | table d'états typée `void (*)(void *, pico8_fixed_t t)` indexée par état (Q3) |

### 2.3 Clés dynamiques du point 2 : toutes à ensemble fini

- **`dg`** : le nom est `me`. Ses valeurs possibles sont `ph`, `kz`, `unknown`, `known`, `ly`, `ll`, `pr`. On les trouve dans `ke:ks`, `jo:ldown` (`nk.me or "known"`), `d_` (`dq or "ph"`) et les défauts générés. Sept valeurs, dont cinq ont une méthode ; le §5 les énumère.
- **`dc(nt)`** : `nt` ∈ {`kj`, `ev`} (`cart.c:247-248`). `lt`/`he` parcourent les cinq noms `bm` (`kj`, `ev`, `hd`, `hk`, `hb`). Ce sont cinq emplacements de la table typée ; la présence se lit comme « emplacement non NULL ».
- **`ui:jz(rm)`** : `rm` ∈ {`ldown`, `rdown`, `gv`}. Trois appels, ou un paramètre qui choisit l'emplacement.
- Aucune clé dynamique à ensemble non fini n'a été trouvée.

### 2.4 Ce qui tombe déjà avec D.7a–D.8

D.7a remplace les rangées ob de `menu`, `jo:fe` et `pj` par des clés typées. Les lignes `pico8_ob_table_set_name(row, "my", pico8_value_function(…))` (≤ 13 : `menu.c` 9, `planet_display.c:399`, `score_panel_display.c:98,321,337`) deviennent des champs de clé. Son PRD les dit « compté pareil » : le compte réel est à lire dans sa note. D.8 : rien.

---

## 3. `pico8_list_t`

### 3.1 Inventaire (motif proposé en Q10 : 574 lignes / 63 fichiers dans `src/`, 413 / 38 dans `tests/`)

| Classe | Listes | Éléments | Forme |
|---|---|---|---|
| **L1** — registres d'entités | `hv`, `cv[5]`, `cg[*]`, calques de `dc` (15), `ui.ic` (16), `ui.le` (partagée, puis tas), `g_ga_by_shared` | REF vers un en-tête d'entité | liste de pointeurs (Q9) ; `del` par identité, tolérance de `all()` (TROU-5) inchangées |
| **L2** — listes membres des entités | `jo.oe` (`jc`), `jo.cc`/`by` (`trade`), `trade.lp` (`jo`)/`rl` (`jc`), `mz.ox`, proposition `ib`, `qy` (particules de `g_`/`hj`/`pm`), `jc.ts` (`pico8_ts_t`, D.7a), `hy.invented` (chaînes `px`), `jo.jy`/`hp.i` (`pico8_jy_t`, D.7a), `gen.oc` (entrées de `df`) | REF homogène, ou chaîne pour `invented` | liste de pointeurs, élément typé à la lecture (le cast `.as.ref` d'aujourd'hui devient un cast de `void *`) ; tableau fixe quand la borne est connue (`gen.oc`) |
| **L3** — listes hétérogènes | `lq.items`, `mh.items`, `jb.ls` ; `lq.fs` (largeurs) | `lq.items` : chaîne, fixe, nil/faux, tuple REF (après D.7a une `pico8_jb_row_t`) ; une fonction ou un autre REF vrai compte comme le sprite 0 (`lq:mn`) | **union étiquetée locale** à `layout` : {rien, fixe, chaîne, rangée, autre-vrai} (Q10) ; `lq.fs` : tableau typé de fixes |
| **L4** — temporaires et aides génériques | `jp` (filtre), `nv` (map), `gu`/`gu_content`, `la_append` (`collect.c`) ; une quarantaine de listes locales (`planet_display` `bs`/`ji`/`jx`/`prefix`…, `menu`, `msgbox:pz`, `trade:kg`, `research`, `research_display`, `score_panel*`, `tech_gate`, `slipway`, `probe`, `galaxy`) | REF, chaînes | aides typées sur la liste de pointeurs (prédicat `int (*)(void *, void *ctx)`) ; locales bornées en tableaux fixes quand la borne est dans le cart |

`PICO8_REF_AS_LIST*` (`ref-audit`) et `PICO8_TYPE_TAG_LIST` (`type-tag`) tombent avec L1-L3. `test_pico8_list` n'est pas un test qui perd son objet : les sémantiques `del`/`all` de PICO-8 restent. Il est réécrit sur le nouveau conteneur (§6, premier point : il ne perd pas son objet).

### 3.2 Ce qui tombe déjà avec D.7a–D.8

- Les aides `*_positional_values` (`help`, `layout`, `menu`, `research_display`, `score_panel`, `score_panel_display`, `tech_gate`) et `ix_items` de `cart:eu` lisent des tables ob : 46 lignes du motif, en comptant `pico8_fi`, `ft_index`/`ft_name`.
- Le reste de `pico8_ob.c/.h` (D.8), et 5 lignes du test ponctuel (D.8).
- **Au total ≈ 55 lignes.** D.7a ne change que le pointé de `lq.items`, `mh.items`, `jy`, `hp.i` et `ts`, et D.7b celui de `qy`. Ces listes restent L1-L3.

---

## 4. Ce qui n'entre dans aucune forme

1. **Classe M, 32 lignes** (16 comptées et 16 appels d'aides, `particle.c`, `probe.c`) : l'émetteur de particules et la sonde sont du code partagé par des branches distinctes (`g_`/`fv` et l'arbre `qh`, plus `mz` pour `dl`), qui lit par nom des champs que chaque branche porte à sa place. C'est la règle 2 de la phase 2. Le §5 cite d'ailleurs ce groupe `py`/`fc`/`k_`/`qo`/`ro`/`qg`/`q_`/`du` comme « exemple connu ». Aucune forme de D ne le couvre sans anticiper cette règle, ou sans ajouter environ 25 accesseurs jetables (Q7).
2. **Les valeurs à genres mêlés, hors des trois points** : `hf` et `gg` (booléen ou fixe, TROU-3), `qh_qh`, `et` (vraie cible, struct sans champ, ou `false`), les retours de `ldown`/`gv`, `js.my` (nil, `false` ou fonction), `js.h_` (TROU-23), `pj_ed`, `trade_qe`, l'`id` de `jc_hg`, `gen.nm[3]`, `pico8_jo_hf`… On compte **62 prototypes publics à `pico8_value_t`** ; environ 20 sont des méthodes que le point 2 type.
   - Une union étiquetée locale par cas est la forme probable, mais aucune puce ne la fixe.
   - `valeur`, `type-tag` et `ref-audit` ne tombent donc pas avec D.10-D.12. C'est un quatrième point, et le prochain arrêt probable (Q12).
3. **Aucune clé dynamique à ensemble non fini.** Toutes les clés à nom variable ont un ensemble fini et s'énumèrent :
   - `me` : 7 valeurs ;
   - `nt` : 2 valeurs ; les noms `bm` : 5 ;
   - `rm` : 3 valeurs ;
   - `bt` : 2 valeurs ;
   - les 27 aides à nom en paramètre n'ont que des appelants à littéral (82 lignes).

---

## 5. Section proposée pour `docs/plan-3.md` (à insérer après D.9)

> **Liste de D, suite (proposée par D.9, à écrire après sa décision).** Trois puces ajoutées à la phase D :
>
> - **Lecture par nom** : un site qui nomme un champ devient un accès au membre typé. Le receveur est l'une de quatre choses :
>   - son type, déjà connu ;
>   - la famille que le cart fixe, par un cast vérifié ;
>   - la fille, par un test d'appartenance (lien parent du descripteur, Q5) ;
>   - l'accesseur typé du singleton (`fm`, `je`, `mo`).
>
>   Un champ transversal lu sur une entité quelconque passe par un accesseur typé de la table de classe (Q6). Une vue construite pour un lecteur par nom devient une struct locale (`pz`) ou un enregistrement typé (sortes).
>
>   Les descripteurs perdent `field`/`set_field`, et `pico8_entity_get_field`/`set_field`, les aides `*_take_*` et les `kind_field` sont supprimés. Un TU lit et écrit les membres. Un test du descripteur est un test de la couche (§6).
> - **Dispatch** : `methods[]` devient une table de pointeurs typés, un membre nommé par méthode, avec sa propre signature (Q2).
>   - Les méthodes d'une seule implémentation sont appelées directement.
>   - Les états lancés par `dg` passent par une table d'états typée indexée par l'état (Q3).
>   - `pico8_mv`, les adaptateurs à `pico8_value_t` et `pico8_value_function_t` sont supprimés.
>   - Un membre fonction prend la signature de son unique appelant.
>   - `hd=1` devient une fonction constante vraie.
> - **Listes** : une liste d'éléments `pico8_value_t` devient une liste de pointeurs (registres, listes membres ; mêmes `add`/`del`/`all`/`foreach`) ou un tableau fixe typé quand sa borne est dans le cart (Q9). Une liste hétérogène (`lq.items`, `mh.items`, `jb.ls`) prend une union étiquetée locale à son module (Q10).
>
> **Ordre** : `dispatch`, puis `par-nom`, puis les listes. `mv` passe par la lecture par nom, et les sites G et E ne disparaissent qu'avec le dispatch typé. Les composants (classe M) se placent selon Q7. `valeur`/`type-tag`/`ref-audit` : étude D.9b (Q12).

### Étapes proposées, avec leurs comptes

Départ supposé : après D.8, `par-nom` ≈ 1008 et `dispatch` ≤ 199. Chaque PRD remesure. « ≈ » veut dire estimé par le classement de D.9, « ≤ » un majorant, « = » exact.

| Étape | Contenu | Empreinte estimée | Compte après |
|---|---|---|---|
| **D.10a** | table de méthodes typée (23 emplacements nommés et la table d'états), 81 adaptateurs retirés, 26 appels `pico8_mv` remplacés (appel direct : `eb`, `ib`, `gg`, `md`, `lq`, `lh` ; virtuel pour le reste), `dg`/`dc`/`jz`/`lt`/`he` sur les emplacements, prédicats `*_is_*_class` sur les emplacements, `pico8_entity_get_field` ne rend plus de méthode, `hd=1` en fonction constante, sites G (9) et E `mv` (1) | ≈ 40 fichiers de `src/` (29 `dispatch`, plus les fichiers d'adaptateurs seuls) : **découper** a1 (méthodes de `dc`, `lt`, `he`, `jz`, `dg`) / a2 (le reste) | `dispatch` src ≤ 46 (76 − 30) ; `par-nom` ≈ 998 ; `valeur` en forte baisse, rapportée |
| **D.10b** | rappels typés (`js.my`/`hb`, `fq.my`, `mh.lh`, `hp.lq`, rappels de `menu`/`jo:fe`/`de`, `eu`, `init`) ; `PICO8_VALUE_FUNCTION` et `pico8_value_function_t` retirés de `pico8_value.h` | ≈ 15 fichiers | `dispatch` src = 0 |
| **D.10c** | TU du dispatch : 98 `pico8_mv` deviennent des appels typés, 25 rappels | 31 fichiers | `dispatch` = 0, dans `DATA_RESIDUE_GUARD` |
| **D.11a** | `par-nom` A et D (41 + 8 lignes, 57 appels d'aides), E `je.l`/`je.r`, aides H mortes (39) | ≈ 22 fichiers | `par-nom` ≈ 909 |
| **D.11b** | B (test d'appartenance, 7 + 1), C (accesseurs transversaux, 16 + 6), F (cadre de `pz`, 5 + 2), K (sortes, 4) | ≈ 14 fichiers | `par-nom` ≈ 877 |
| **D.11c** | M selon Q7 (composants émetteur et sonde) | `particle`, `probe`, plus les 13 fichiers de classes qui les portent | `par-nom` ≈ 861 |
| **D.11d** | TU de `par-nom` : T1 en accès aux membres, T2 supprimés (Q11), deux §6 réécrits avec leur preuve rouge | 38 fichiers : **découper** en deux (≈ 19 chacun) | `par-nom` ≈ 286 |
| **D.11e** | suppression de l'infrastructure : 40 lecteurs, 18 écrivains, 10 `kind_field` et les emplacements des 11 sortes, 55 aides `*_take_*` (43 après D.7a), `pico8_entity_get_field`/`set_field`, typedefs `pico8_struct_field_fn`/`_set_field_fn` ; les 11 structs-vues deviennent typées ou disparaissent : remplaçants `mo`/`je`/`fm`, `jc_too_far`, `jc_mouse_point`, `bv_et`, `nr_ldown_empty`, `pj_cr`, `mz_jj`, `ha_pz`, `ie_ke_view` | ≈ 35 fichiers de `src/` | `par-nom` = 0, dans `DATA_RESIDUE_GUARD` |
| **D.12a** | catégorie `liste` ajoutée (Q10) ; conteneur de pointeurs (Q9) ; registres L1 et leurs parcours | `entity.*`, `bootstrap`, `ui.*`, `economy*.c`, `cart`, `galaxy`, `bg`, `hud` (≈ 15) | `liste` ≤ 987 − 55 (D.7a-D.8) − L1, rapporté |
| **D.12b** | listes membres L2 et aides L4 de `collect` | ≈ 25 fichiers | rapporté |
| **D.12c** | listes hétérogènes L3 (union locale) et locales d'affichage L4 | ≈ 15 fichiers | `liste` src = 0 |
| **D.12d** | TU (413 lignes, 38 fichiers ; `test_pico8_list` réécrit) : **découper** en deux ; `PICO8_REF_AS_LIST*`, `PICO8_TYPE_TAG_LIST` retirés | 38 | `liste` = 0, dans `DATA_RESIDUE_GUARD` |

Pour chaque étape :

- `class-coverage-net` passe si l'étape touche un constructeur : D.10a, D.11c, D.11e.
- `sanitize-net` passe à chaque étape qui change la durée de vie ou le propriétaire d'une donnée : D.11c, D.12a-c.
- §6 : les deux tests restants de la liste sont réécrits dans D.11d, et `layout_nil_item_draws_nothing` dans D.12c, qui lui fait passer son item par l'union ; chacun avec sa preuve rouge.
- Aucun `*.sha256` ni e2e ne change.

---

## 6. Décisions humaines

Chaque question est à trancher par oui/non ou A/B. La recommandation et sa raison suivent.

**Q1. Ordre `dispatch` → `par-nom` → listes, placé en D (avant la Sortie) et non en phase 2 ?**
Oui. `mv` lit par nom : typer le dispatch en premier enlève les sites G et E et le rendu des méthodes comme champs. Et la Sortie de D exige de supprimer `pico8_value.c`, donc aucun des trois points ne peut attendre la phase 2 sans réécrire la Sortie.

**Q2. Table de méthodes : (A) struct de pointeurs typés, un membre nommé par méthode ; (B) garder `methods[]` indexé par `pico8_method_t` et caster à l'appel ?**
A. Le compilateur vérifie chaque signature, et les surcharges restent des remplacements d'entrée. B appelle par un pointeur de fonction casté vers un autre type, ce qui est un comportement indéfini, et rien n'y est vérifié.

**Q3. États lancés par `dg` : (A) table d'états typée indexée par un enum, `me` reste une chaîne et le nom est converti par `strcmp` comme `pico8_method_of` le fait aujourd'hui ; (B) `me` devient un enum ?**
A. `me` est comparé comme chaîne dans une quinzaine de sites et porté par les données générées. B change le type d'un champ lu partout, ce qui déborde du point 2.

**Q4. `gfx`/`mem`/`input` : garder `pico8_dispatch_context` (oui) ou les passer en arguments des méthodes typées (non) ?**
Oui, le garder. Il n'appartient à aucune catégorie, et le passer en argument élargirait D.10a sans rien retirer au résidu. Son commentaire est réécrit.

**Q5. Membres d'une fille lus sur la famille (B : `qh`, `nl`, `hp`, `px`, `s`, `ba`, `qw`) : (A) test d'appartenance par un lien parent ajouté à `pico8_struct_class_t` (`pico8_qh_of(jo)`) ; (B) remonter ces membres dans `jo` ?**
A. La mise en page ne change pas. Les prédicats `*_is_*_class` montrent que ce test existe déjà sous une autre forme, et la phase 2 reclasse les membres de toute façon. B anticipe la règle 1 de la phase 2 pour 7 membres, sans son test ponctuel.

**Q6. Champs transversaux (C : `p_`, `fa`, `ra`, `nu`, `ma`, `kq`, `dw`, `m_`, `ql`, `fg`, présence de `gv`) : (A) un accesseur typé par champ dans la table de classe, NULL = absent ; (B) monter ces champs dans `ke` ?**
A. Le noyau de la phase 2 ne nomme que la table de méthodes, `me`, `t` et `nu`. Monter `p_`/`fa`/`ra` dans `ke` changerait la mise en page de 40 structs pour 22 lignes.

**Q7. Classe M (émetteur de particules, sonde ; 32 lignes) : (A) anticiper en D la règle 2 de la phase 2 pour ces deux groupes seulement (composant porté par pointeur) ; (B) environ 25 accesseurs typés, jetés en phase 2 ; (C) exclure ces sites du compte de D (`par-nom` ≠ 0 à la Sortie, reporté en phase 2) ?**
A. Le plan cite déjà ce groupe comme l'exemple connu de composant, et la règle 2 est écrite, pas inventée. B produit du code jetable. C casse « sortie vide = sortie de D ». Risque à noter dans le PRD : la règle 2 s'applique sans le test ponctuel d'archétype, qui ne porte que sur les défauts.

**Q8. Singletons `fm`/`je`/`mo` : des accesseurs typés qui rendent NULL sur le remplaçant, le remplaçant de `fm` restant une struct typée pour `jk`/`nz` ?**
Oui. `pico8_fm_hy()` existe déjà sur ce modèle. Les lectures par défaut (vecteur nul, absent) restent celles d'aujourd'hui.

**Q9. Conteneur des listes : (A) un seul conteneur de pointeurs (`void *`, mêmes `add`/`del`/`all`/`foreach`, `del` par identité), plus des tableaux fixes typés là où la borne est dans le cart ; (B) un type de liste par type d'élément (macro) ; (C) garder `pico8_list_t` de `pico8_value_t` ?**
A. La tolérance de `all()` à la suppression (TROU-5) et la sémantique de `del` restent écrites une seule fois, et le cast `void *` au site remplace le cast `.as.ref` d'aujourd'hui. B multiplie ces sémantiques. C laisse survivre `pico8_value.c`, donc la Sortie de D devient impossible.

**Q10. Ajouter une catégorie `liste` au compteur (motif `\bpico8_list_[a-z_]+\b|\bPICO8_REF_AS_LIST[A-Z_]*|\bPICO8_TYPE_TAG_LIST\b|pico8_list\.h`, 987/101 à `2dec71f`), dans le premier commit de D.12a, et une union étiquetée locale à `layout` pour `lq.items`/`mh.items`/`jb.ls` ?**
Oui pour les deux. Sans catégorie, les étapes D.12 n'ont pas de « Compte », contrairement à la règle de la liste de D. L'union locale ne porte que les cinq cas que `lq:mn` distingue. Le nouveau conteneur doit prendre un nom hors du motif (règle commune : un nom nouveau ne correspond à aucun motif de la catégorie qu'il remplace).

**Q11. Tests des descripteurs (T2 : environ 145 lignes `cls->field`/`set_field` et les tests de mort des `assert` d'écrivain) : les supprimer avec les descripteurs, comme tests de la couche (§6, premier point) ?**
Oui. Leur objet, la lecture et l'écriture par nom, disparaît. Les valeurs des membres restent couvertes par les TU en accès typé (T1) et par les e2e.

**Q12. Ouvrir une étude D.9b pour les valeurs à genres mêlés (§4, point 2 ; 62 prototypes publics) avant les étapes `valeur`/`type-tag`/`ref-audit` ?**
Oui. Les trois points de D.9 ne suffisent pas à éteindre `valeur`. Sans règle écrite pour `hf`/`gg` (TROU-3), `et`, `qh_qh`, `js.my`/`h_` et les retours de `ldown`, la première étape `valeur` rencontrera l'arrêt « champ sans règle ». D.9b suivrait le même modèle.

---

## Annexe A — sites `par-nom` de `src/` hors infrastructure, par classe

Format : classe, fichier (`src/pico8_`), fonction, `R:` lu / `W:` écrit (`(var)` : nom passé en paramètre, ou déballage d'une valeur). Les appels d'aides sont inclus.

```
A  bootstrap.c        pico8_dg                  R:me R:t R:t
A  economy.c          pico8_hy_lj               W:kr
A  economy_formulas.c bx_visit                  R:jy (+field_kw)
A  economy_formulas.c ck_visit                  R:jy
A  economy_formulas.c pico8_dj                  R:p,p
A  economy_score.c    pico8_da                  R:mi
A  economy_score.c    pico8_db                  R:by R:hf R:hs R:kw R:mi R:mi R:trade R:trade
A  economy_score.c    sum_by_mi_visit           R:mi
A  economy_tiers.c    bb_visit                  R:hn R:hn R:jh R:jy R:kw R:mi W:mi
A  economy_tiers.c    bd_reset_visit            R:cc W:cc
A  economy_tiers.c    bd_route_visit            R:cc R:hg R:id R:lp R:rl W:by
A  economy_tiers.c    pico8_dh                  R:by R:cc R:hf R:jy R:me R:qe
A  entity_geom.c      pico8_ij                  R:p
A  entity_geom.c      pico8_ma                  R:t
A  galaxy.c           pico8_bw                  R:en
A  menu.c             pico8_cr_row_my_dispatch  R:_cr_t
A  menu.c             pico8_ix2_hb_dispatch     W:lq
A  planet_display.c   pico8_jo_fe_my_dispatch   R:_jo_fe_b R:_jo_fe_od R:_jo_fe_pt
A  planet_display.c   pico8_jo_gv               R:qu
A  planet_family_trade.c pico8_mx_kf            R:jy
A  planet_family_trade.c pico8_nr_gv            R:qu
A  planet_trade.c     pico8_jo_kf               R:fx
A  probe.c            pico8_bv_jc_predicate     R:p R:p R:qu R:re
A  research_display.c pico8_bj_predicate        R:nk
A  score_panel_display.c pico8_pj_ph_map        R:px
A  slipway.c          pico8_jc_ce               R:oe
A  slipway.c          pico8_jc_ce_jc_predicate  R:mt, R:qu R:re R:p R:p (wrappers)
A  slipway.c          pico8_jc_hq               R:mt R:mt R:o_
A  slipway_display.c  pico8_jc_ch               R:id R:ra R:ra
A  wormhole.c         pico8_mz_pq_ox_predicate  R:fx
A  wormhole.c         pico8_mz_pr               R:me R:p
A  wormhole_display.c pico8_mzd_ra_of_ij        R:ra
B  economy_score.c    pico8_da                  R:qh
B  economy_tiers.c    pico8_dh                  R:nl
B  planet.c           pico8_jo_pq               R:hp
B  planet_display.c   pico8_jo_gh               R:px R:qh
B  planet_display.c   pico8_jo_kj               R:qw R:s
B  planet_trade.c     pico8_jo_cn               R:ba
C  bootstrap.c        pico8_dg                  R:p_
C  entity.c           pico8_dc                  R:fa R:ra
C  entity.c           pico8_lt / pico8_he       R:(noms bm) R:nu
C  entity_geom.c      pico8_gd / gk / ma / na   R:ra R:fa R:kq R:ma W:ma
C  planet_display.c   pico8_jo_kj               R:dw (sur jo.fo)
C  slipway.c          pico8_jc_hq               R:gv W:fo (sur jc.et)
C  ui.c               pico8_ui_ev / pico8_ui_gw R:ql R:fa R:fg W:m_ W:m_
D  bg.c               pico8_bg_hq               W:v (mo)
D  economy_score.c    pico8_ky / pico8_bf       R:happiness R:invented R:po R:po (fm)
D  entity_geom.c      pico8_gd                  R:ra (mo)
D  menu.c             pico8_cr                  R:ew R:invented (fm)
D  msgbox.c           pico8_iv_pq               R:ra R:ra (je, mo)
D  planet_family.c    pico8_ga_pq               W:ma (mo)
D  probe.c            pico8_dl                  R:ra (je)
D  score_panel.c      pico8_pj_hb               R:ge R:happiness R:jk R:lo R:nz R:po R:yr (fm)
D  score_panel_display.c pico8_pj_ph / _ph_map  R:hl R:hl (fm)
D  tech_gate.c        pico8_cb_predicate / pico8_gi_cost / pico8_oj  R:ew R:jk R:fu R:invented (fm)
D  ui.c               pico8_ui_bh / ev / gw / lj R:ra x5 (je, mo)
E  entity.c           pico8_mv                  R:(var)
E  ui.c               pico8_ui_je_button_table  R:(var) (bt ∈ {l,r})
F  msgbox.c           pico8_pz / pico8_pz_step  R:eo R:ep R:ff R:hi R:ra R:ca R:gq
G  bg.c               pico8_bg_hq / _rdown      (rp)
G  ke.c               pico8_ke_ks               (me)
G  research_icon.c    pico8_ie_hb               (g_ie_gy)
G  slipway.c          pico8_jc_hq               (mp, mt)
G  ui.c               pico8_ui_bh               (kn) x2
G  ui_button.c        pico8_fq_ldown            (résultat de my)
H  27 aides dans economy_formulas, economy_score, economy_tiers, entity_geom, msgbox, particle, planet_display, probe, score_panel, score_panel_display, slipway, slipway_display, ui, wormhole, wormhole_display
K  research_icon.c    pico8_ie_ke_view          rr s me od (kind->field)
M  particle.c         pico8_ei / pico8_ez / pico8_f_  R:fa R:fc R:q_ R:qg R:qo R:qy R:ra R:ro R:k_ W:qy
M  probe.c            pico8_bv / pico8_d_ / pico8_dl / pico8_g__mm  R:dw R:dq R:et R:fp R:ik R:od R:me R:qs R:du R:ki R:pk R:pn R:py R:t W:et W:fo W:p W:ra W:p_
X  layout.c           pico8_lq_new_table        (4, D.7a)
X  ui_button.c        pico8_js_new_table        (13, D.7a)
```

## Annexe B — reproduire

```sh
git archive 2dec71f | tar -x -C "$SCRATCH/snap" && cd "$SCRATCH/snap"
sh tools/data_residue.sh; sh tools/data_residue.sh --detail | grep -E '^(par-nom|dispatch)'
grep -ohE '\[PICO8_METHOD_[A-Z]+\] *= *[a-z0-9_]+' src/*.c | sort -u          # 196 entrées, implémentations par méthode
grep -nE '^(static )?pico8_value_t [a-z0-9_]+\(void \*self, int argc, pico8_value_t arg0, pico8_value_t arg1\)' src/*.c   # 81 adaptateurs
grep -cE '\bpico8_list_[a-z_]+\b|\bPICO8_REF_AS_LIST[A-Z_]*|\bPICO8_TYPE_TAG_LIST\b|pico8_list\.h' src/*.c src/*.h tests/*.c   # 987/101
```
