---
title: "Plan 3, étape 2.6 : la question `pj-hb-nil` tranchée — inaccessible ; un article faux ou nil de `pj.ou` n'atteint jamais `pico8_pi_is_number` en jeu, la ligne sort de `tools/atteinte_a_prouver.txt` et entre, justifiée, dans `tools/line_coverage_unreachable.txt` (`atteinte-à-prouver` 1 → 0, gardé ; pied 1291/89 → 1290/88) — un commit, jeu identique"
description: "Le cart saute les nil de `pj:hb` là où le port les écrit, et `pico8_pi_is_number` lit un faux ou un nil de `pj.ou` sans qu'aucun test ne le voie. Lecture du cart et `gcov` des 26 e2e : aucun faux ni nil n'y arrive en jeu, parce que `pj:hb` réécrit les sept articles avant tout dessin. Le bras faux/nil de `pico8_pi_is_number` passe sur sa propre ligne, qui est à `#####`, et cette ligne est justifiée dans `tools/line_coverage_unreachable.txt`."
status: needs-triage
---

## Problem Statement

**Écrit sur `d1e5a30`** (branche `claude/plan-pilot-3-7zdft0`, 2.5 notée). Aucune ligne du dépôt n'a changé pour écrire ce PRD. Les comptes et les preuves viennent d'une copie jetable de `d1e5a30` (`p`, le parent, puis `a`, 2.6 appliquée), supprimée depuis. Un premier passage sur `756b3b9` (avant 2.5) donnait les mêmes verdicts ; 2.5 ne touche aucun fichier de 2.6 (`git diff --name-only 756b3b9 d1e5a30` ne nomme ni `pi.c`, ni `score_panel.c`, ni `economy.c`, ni `tools/*.txt`). Seuls changent la garde du `Makefile` (2.5 y a ajouté `aiguillage-nom`), le pied et le total de `make line-coverage`.

Vu du mainteneur du port : `tools/atteinte_a_prouver.txt:6` pose une question ouverte depuis D.18c.

> `pj:hb` écrit nil dans `pj.ou` là où le `pairs` du cart saute la valeur (peut-être aussi via `ef[fm.lo]` avec un `lo` fractionnaire, contre le `flr` du port) ; avec elle, le false ou le nil de `pj.ou` lu par `pico8_pi_is_number` (D.18c M6/M7), jamais atteint.

D.18c avait constaté, par une sonde, que M6 (le `false` initial d'`ou` lu comme un nombre) et M7 (le nil d'`ou` lu comme un nombre) ne rougissent ni TU ni e2e. Il n'avait pas prouvé si c'était un trou de test ou un chemin mort. 2.6 tranche : **le chemin est mort en jeu.**

### Texte du plan, recopié (`docs/plan-3.md` à `d1e5a30`, ligne 2.6)

> - 2.6 — `atteinte-à-prouver` : la question `pj-hb-nil` (`tools/atteinte_a_prouver.txt:6`) tranchée par lecture du cart (`pj:hb`, `pairs` qui saute nil, `ef[fm.lo]` avec `lo` fractionnaire contre le `flr` du port) et `gcov` des e2e. Atteinte prouvée : entrée `TROU-N` et test qui rougit sous M6/M7 de D.18c ; inaccessibilité prouvée : ligne dans `tools/line_coverage_unreachable.txt`, justifiée. La ligne sort de la liste ; le fichier reste, en-tête seul (le compteur sort en code 2 sans lui, `composition_residue.sh:133`). Modules : `score_panel`, `pi` (lecture), `docs/trous-de-portage.md` ou `tools/`. Compte : `atteinte-à-prouver` 1 → **0**, gardé ; pied → **1290 / 88**. Preuves : atteinte → le test rouge au parent sous M6 puis M7 ; inaccessibilité → `gcov` des 26 e2e à `#####` sur `pico8_pi_is_number` (bras faux/nil) et argument du cart au commit. `class-coverage-net` : non. `sanitize-net` : non. Budget ≈ 50 min.

Règles communes de la phase : `make test` vert ; `make e2e` **26 conformes** ; aucun `*.sha256` ni `tests/e2e/` modifié ; aucune constante d'empreinte de TU (`*_HASH`, `FROZEN_*`) changée. Une catégorie tombée à 0 entre dans `COMPOSITION_RESIDUE_GUARD`, avec une preuve de garde mordante.

### Le verdict : inaccessible. La preuve par le cart (`pico8/lua.lua`)

**Qui lit `pj.ou`.** Un seul lecteur : `pj:kj` (`:853`, `bl(la(pj.i_,pj.ou))`), un dessin. Il n'est appelé que par `dc("kj")` dans `_draw` (`:1188`). Le cart n'a ni `flip()`, ni `yield`, ni `cocreate` (grep vide) : tout ce que fait un `_update60` (`:1187`) se termine avant le dessin suivant. **Une valeur de `pj.ou` qui est réécrite dans la même mise à jour n'est donc jamais dessinée.**

**Qui écrit `pj.ou`.** Les articles naissent `false` (`:838-844`, `o(f,…,fn="qr")`). Seul `pj:hb` les réécrit (`:847-851`), et `pj:hb` n'est appelé que par `hy:lj` (`:1236-1237`, `for e in all(cv.hb) do e:hb() end`). Tous les appels de `lj` passent par `fm`, qui est un `hy` : `fm:lj()` en fin d'`eu` (`:1186`), `self:lj()` dans `hy:nx` (`:1206`), `hy:hk` (`:1214`) et `hy:kt` (`:1453`), qui sont eux-mêmes appelés par `fm:nx` (`:460`, `:571`, `:640`), `fm:hk()` (`:980`) et `fm:kt(t)` (`:957`).

**1. Le `false` initial (M6) n'est jamais dessiné.** Le panneau `pj` naît dans `eu` (`:1182`, `fm,ju=hy(),pj()`), et `eu` finit par `fm:lj()` (`:1186`), qui appelle `pj:hb`. Les sept `false` sont donc remplacés avant le premier dessin qui suit `eu`. Aucun autre appel ne remet un `false`.

**2. Les nil de `fm.jk` et `fm.yr` n'existent pas en jeu.** `pj:hb` ne tourne que sous `hy:lj`, avec `fm` qui vaut le `hy` créé par `eu`. `hy:pq` range `jk=0` et `yr=3401` (`:1200-1202`), et seuls des `+=`/`-=` numériques les touchent ensuite (`:1212-1213`, `:1447`). Dans le port, les nil de `fm_jk` (`score_panel.c:147`) et de `fm_yr` (`:151`) ne sont pris que quand `fm` n'est pas un `hy` (`pico8_fm_hy()` NULL) : cela n'arrive que dans le TU `test_pico8_fm` (`:102`, `:106`), qui appelle `pico8_pj_hb` directement. En jeu, `pico8_hy_lj` n'est appelé qu'avec `pico8_fm_hy()` (`economy.c:101`, `:140`, `:298` ; `cart.c:194-195`, qui asserte qu'il n'est pas NULL).

**3. `lo` n'est jamais fractionnaire.** `lo` naît à 1 (`:1202`), revient à 1 par le menu (`:979`), n'avance que de `hx` (`:1205`) et ne recule que de `kx` (`:1207`). Les `hx` possibles sont tous entiers : `fp=2` ou `ov("autoassemblers",3,2)` (`:451`, `:459-460`), `self.fp or 0` d'une sonde (`:571`), qui vaut 0 (faux), 1 ou 2 (2.4), et `1` (`:640`). `kx` vaut `ov("time compression",12,15)` (`:1204`, `ov` en `:1467-1470`), donc 12 ou 15. Le `flr` du port (`score_panel.c:202`) rend exactement l'index du cart. **L'hypothèse du `lo` fractionnaire tombe.**

**4. Le nil de `ef[fm.lo]` existe, mais il est écrasé dans la même mise à jour.** `ef` a 15 noms (`:797`, `JAN` … `DEC`, `UND`, `DUO`, `TER` ; `PICO8_EF_COUNT = 15`, `game_data.h:17`). Hors d'un `hy:nx`, `lo` reste dans `1..kx`, donc dans `1..15`. Dans `hy:nx` (`:1204-1209`), `lj` est appelé **après** `lo+=hx` et **avant** le retour en arrière. `lo` y vaut au plus `kx+3`. Avec `kx=12`, c'est au plus 15, et `ef[lo]` existe. Avec `kx=15` (« time compression » inventée), `lo` peut valoir 16, 17 ou 18. `ef[lo]` est alors nil, le `pairs` du cart saute la case, et le port y écrit nil. Mais `lo>kx` est alors vrai : `lo-=kx` le ramène dans `1..3` et `self:hk()` rappelle `self:lj()` (`:1214`), donc `pj:hb`, qui réécrit les sept cases avec des valeurs non nil. Entre les deux `pj:hb` ne tournent que les autres écouteurs de `cv.hb` et le début de `hy:hk`, et aucun ne lit `pj.ou`. Le port suit le même ordre (`economy.c:87-106`). **L'écart « `pairs` saute nil » n'est donc jamais visible**, ni dans le cart ni dans le port.

**Conclusion.** En jeu, un article de `pj.ou` qui arrive à `pico8_pi_is_number` est une chaîne ou un nombre, jamais `false` ni nil. Les autres articles que `bl` passe à `pico8_pi_is_number` (`fv_rb`, `pj_gs`, `pj_ea`, `rk`, `eq`, `spr`, le troisième argument de `ld` ; table B10 de D.18c) ne sont jamais faux, et les pointeurs de `ld` passent par `pico8_pi_ref`, pas par `pico8_pi_is_number`. **Le bras faux/nil de `pico8_pi_is_number` est inaccessible en jeu.** Il n'y a pas d'entrée TROU à écrire.

### La preuve par `gcov` (26 e2e, arbre `p` = `d1e5a30`)

Les objets instrumentés de `make line-coverage` ont été rejoués sur les 26 e2e, depuis l'écran titre, avec des `.gcda` cumulés. Puis `gcov -b -c` sur `src/pico8_pi.c` :

```
105979:   72:    return item != NULL && item->s == NULL && !item->is_false && !item->is_ref;
branch  0 taken 93086 (fallthrough)
branch  1 taken 12893        <- item NULL (au-delà de la fin)
branch  2 taken 83422 (fallthrough)
branch  3 taken 9664         <- chaîne
branch  4 taken 83422 (fallthrough)
branch  5 taken 0            <- is_false : jamais
branch  6 taken 83422 (fallthrough)
branch  7 taken 0            <- is_ref : jamais
```

Même mesure sur `src/pico8_score_panel.c` (`pico8_pj_hb`, 258 appels) : les trois bras nil (`:147`/`:164`, `:151`, `:204`) ne sont **jamais pris** par les e2e (`fm != NULL` 258 fois sur 258, `lo` dans `1..15` 258 fois sur 258). Aucun e2e n'invente « time compression » puis ne passe le mois 15. Cela ne change pas le verdict, puisque le cas 4 est écrasé avant tout dessin.

Le plan demande `#####` **sur une ligne**, mais le bras faux/nil partage aujourd'hui la ligne `:72` avec le cas courant, qui est exécuté 105 979 fois. `gcov` par ligne ne peut donc pas le montrer, et `tools/line_coverage_unreachable.txt` se lit par ligne (`fichier:ligne raison`, `line_coverage.sh` `gap`). 2.6 met donc le bras faux/nil sur sa propre ligne (décision G1). Mesuré à l'enfant : `pi.c:76` à `#####`.

## Solution

Un seul commit.

- `pico8_pi_is_number` s'écrit en deux tests. Le second, « faux ou référence → 0 », a son `return 0;` sur une ligne à lui (`pi.c:76`). Le comportement ne change pas : c'est la même expression booléenne, dépliée.
- `tools/line_coverage_unreachable.txt` reçoit une ligne : `src/pico8_pi.c:76` et sa raison.
- La question `pj-hb-nil` sort de `tools/atteinte_a_prouver.txt`. Le fichier garde son en-tête de quatre lignes.
- `atteinte-à-prouver` entre dans la garde.
- `docs/trous-de-portage.md` ne change pas : aucun écart visible n'est prouvé.

## User Stories

1. En tant que mainteneur du port, je veux savoir si le `pairs` du cart qui saute les nil de `pj:hb` produit un écart visible, afin de ne pas laisser un trou non documenté.
2. En tant que lecteur de `pico8_pi_is_number`, je veux savoir que son bras faux/nil ne tourne jamais en jeu, afin de ne pas chercher un test qui ne peut pas exister.
3. En tant que vérificateur, je veux ce bras sur une ligne à `#####` dans le `gcov` des 26 e2e, afin que l'inaccessibilité se relise avec `make line-coverage`.
4. En tant que lecteur de `tools/line_coverage_unreachable.txt`, je veux la raison écrite à côté de la ligne, afin de ne pas refaire la lecture du cart.
5. En tant que gardien du compteur, je veux `atteinte-à-prouver` dans `COMPOSITION_RESIDUE_GUARD`, et la preuve qu'une question rajoutée rougit `make test`, afin qu'aucune question d'atteinte ne reste ouverte sans bruit.
6. En tant que gardien du jeu, je veux `make e2e` à 26 conformes et aucun `*.sha256` touché, afin que le jeu reste le même image par image.
7. En tant que vérificateur, je veux M6 et M7 de D.18c rejouées au parent et à l'enfant, afin de voir qu'elles sont vertes aux deux (chemin jamais atteint) et qu'aucune mutation n'est verte à l'enfant et rouge au parent.
8. En tant que lecteur du plan, je veux une note « Découvert à 2.6 » courte, afin que 2.7 parte des vrais comptes.

## Implementation Decisions

### Modules

`pi` (une fonction réécrite, sans changer son comportement), `tools/atteinte_a_prouver.txt`, `tools/line_coverage_unreachable.txt`, `Makefile`, `docs/plan-3.md`. `score_panel` est lu, pas touché. `docs/trous-de-portage.md` n'est pas touché (branche « inaccessible »).

### Ce qui change, fichier par fichier (numéros de `d1e5a30`)

**`src/pico8_pi.c`**, `:70-73` :

```c
static int pico8_pi_is_number(const pico8_jb_row_t *e, int i) {
    const pico8_jb_item_t *item = pico8_pi_arg(e, i);
    if (item == NULL || item->s != NULL) {
        return 0;
    }
    if (item->is_false || item->is_ref) {
        return 0;
    }
    return 1;
}
```

Les lignes `:70-79` de l'enfant sont exactement celles-ci. Le second `return 0;` est la ligne **`:76`**. Il n'y a pas de commentaire : la raison vit dans `tools/line_coverage_unreachable.txt`, et aucune des quatre règles de `CLAUDE.md` n'en demande un ici. Il n'y a pas non plus d'`assert` : le plan demande une ligne justifiée, pas un arrêt (Out of Scope). Les `assert` de `pi.c` sont tous au-dessus (`:35`, `:36`, `:66`), donc leur `__LINE__` ne bouge pas.

**`tools/atteinte_a_prouver.txt`** : les lignes `:5` (vide) et `:6` (`pj-hb-nil…`) partent. Il reste les lignes `:1-4`, les `#` d'en-tête, sans ligne vide finale. Le fichier reste, parce que `composition_residue.sh:133` sort en code 2 s'il est absent.

**`tools/line_coverage_unreachable.txt`** (vide aujourd'hui) : une ligne, exactement :

```
src/pico8_pi.c:76 aucun article faux ni référence n'arrive ici en jeu : seul pj.ou en porte (false initial, nil de pj:hb), et pj:hb le réécrit avant tout dessin (eu() finit par fm:lj() ; le nil d'ef[fm.lo], lo de 16 à 18, est écrasé par le second fm:lj() de hy:nx dans la même frame)
```

Seul le premier champ compte pour `line_coverage.sh gap`. La raison est le résumé de la preuve ; le détail va dans le message de commit.

**`Makefile`**, `:864` : `COMPOSITION_RESIDUE_GUARD := noyau-inclut,un-ou-faux,aiguillage-nom` → `COMPOSITION_RESIDUE_GUARD := noyau-inclut,un-ou-faux,aiguillage-nom,atteinte-à-prouver`. Rien d'autre : `--check atteinte-à-prouver` fonctionne déjà (mesuré, code 0 à l'enfant), et le TU du compteur (`test_composition_residue.c:19`, `:276-279`) travaille sur son propre fichier d'atteinte, pas sur celui de `tools/`.

**`docs/plan-3.md`** : une note « Découvert à 2.6 » au bout de la ligne 2.6. Elle donne le verdict (inaccessible), les comptes, la valeur de la garde, la ligne `pi.c:76` et, pour les étapes suivantes, que `tools/line_coverage_unreachable.txt` a sa première ligne. Le reste va au message de commit.

Rien d'autre. Pas de `glossaire-noms.md` (aucun nom ne change de sens), ni `trous-de-portage.md`, ni `score_panel.c`, ni `tools/composition_residue.sh`, ni aucun TU.

### Comptes, mesurés sur copie de `d1e5a30`

| Catégorie | parent `d1e5a30` → 2.6 |
|---|---|
| `atteinte-à-prouver` (lignes / fichiers) | 1 / 1 → **0**, disparue de la sortie, gardée |
| les onze autres | inchangées (`parente 10 3`, `ptrs-entité 7 4`, `inclusion-mutuelle 17 16`, …) |
| pied | **1291 / 89 → 1290 / 88** (−1 ligne, −1 fichier : `tools/atteinte_a_prouver.txt`) |
| `make line-coverage`, 26 e2e | 6305 → **6308** (+3 : `pi.c:73`, `:75`, `:78` ; `:76` à `#####`) |
| `make e2e-gap` (hors suite, déjà rouge) | écart 2836 → 2837 (voir « Surprises ») |

### Anti-tassement

- **Interdit** :
  - retirer la question sans la ligne justifiée dans `tools/line_coverage_unreachable.txt`, ou écrire cette ligne sur `pi.c:72` d'avant (une ligne exécutée 105 979 fois n'est pas une ligne inaccessible) ;
  - supprimer `tools/atteinte_a_prouver.txt`, ou garder la question en commentaire `#` dans un autre fichier compté ;
  - changer ce que rend `pico8_pi_is_number` pour un article, ou toucher `pj:hb` (le nil écrit par le port reste tel quel : il est invisible) ;
  - remplacer le second `return 0;` par un `assert` ou par `abort` ;
  - toucher `tools/composition_residue.sh`, `tests/test_composition_residue.c`, `tests/e2e/`, un `*.sha256` ou une constante d'empreinte ;
  - retirer `atteinte-à-prouver` de la garde.
- **Piège à ne pas réintroduire** : refondre les deux tests en une ligne décale la ligne justifiée, et `pi.c:76` désignerait alors autre chose. Toute réécriture future de `pico8_pi_is_number` met à jour la ligne de `tools/line_coverage_unreachable.txt` dans le même commit.

## Implementation Chunks

Un commit, ≈ 30 min preuves comprises, sous le budget de 50 min. Il n'y a pas de découpage.

1. Réécrire `pico8_pi_is_number`, vider la question, écrire la ligne justifiée, élargir la garde.
2. Lancer les preuves (ci-dessous), écrire la note du plan, commiter, pousser.

**Repli** (si le budget déborde avant le commit) : aucun. Les quatre fichiers ne se coupent pas utilement : la garde exige la question vidée, et la question ne sort qu'avec sa ligne justifiée.

## Testing Decisions

- **Aucun TU nouveau, aucun e2e nouveau.** C'est la branche « inaccessible » du plan : un test qui dessinerait un `ou` faux ou nil fabriquerait un état que le jeu ne produit pas. Les TU ne rejouent jamais un `events.json`. Les e2e partent de l'écran titre (`make e2e`), comme toujours.
- **Prior art** : ensemble `L` de 2.4 ; mutations avec la règle du parent de D.21 et 2.4 ; garde mordante de 2.2 et 2.4 ; sonde de D.18c (M6/M7 verts partout).

### Preuves

1. `make test` vert ; `make e2e` **26 conformes** ; `git diff --stat <parent> -- tests/e2e '*.sha256'` vide ; `git diff <parent> | grep -E '^[-+].*(_HASH|FROZEN_)'` vide.
2. **`gcov` des e2e** (`make line-coverage EVENTS="$(ls tests/e2e/*.json | tr '\n' ' ')"`, dans une copie) au parent puis à l'enfant. Les lignes sont appariées avec un décalage de `+6` pour `src/pico8_pi.c` au-delà de `:72`. Mesuré : 6305 → **6308**. **`L` (lignes couvertes au parent, perdues à l'enfant) est vide.** Les lignes gagnées sont `pi.c:73`, `:75`, `:78`. `grep -x 'src/pico8_pi.c:76' tmp/line_coverage.txt` est **vide** à l'enfant. Toute autre différence : arrêt, rapporté.
3. **`gcov -b` au parent** (`.gcda` cumulés des 26 e2e sur les objets de `tmp/obj-gcov`, `gcov -b -c -o tmp/obj-gcov/src src/pico8_pi.c` depuis la racine) : `:72` branches 5 et 7 **`taken 0`** (tableau ci-dessus). À l'enfant, `gcov` montre `:73` 22 557, `:75` 83 422, **`:76` `#####`**, `:78` 83 422. Les deux sorties vont au message de commit.
4. **Mutations**, une à la fois, à l'enfant puis la même au parent (règle du parent : **ÉCHEC** si une mutation est verte à l'enfant et rouge au parent ; verte aux deux = « chemin jamais atteint »). Restaurer par `cp` puis `touch` (piège de 2.3 : `mv` rend un `mtime` ancien). Mesuré sur `d1e5a30` :

| # | Mutation à l'enfant ; au parent | TU rouges (`make -k unit`) | e2e conformes |
|---|---|---|---|
| M6 | (D.18c) `if (item->is_false \|\| item->is_ref)` → `if (item->is_ref)` ; au parent, `!item->is_false && ` retiré de `:72` | aucun, aux deux | 26 / 26, aux deux |
| M7 | (D.18c) `… \|\| item->is_ref)` → `if (item->is_false)` ; au parent, ` && !item->is_ref` retiré | aucun, aux deux | 26 / 26, aux deux |
| M8 | `:76` `return 0;` → `return 1;` (enfant seul, même sens que M6+M7) | aucun | 26 / 26 |
| M10 (témoin) | `:78` `return 1;` → `return 0;` ; au parent, ` && 0` ajouté en fin de `:72` | `fr`, `fv`, `g_`, `ga`, `hj`, `hp`, `lq`, `mh`, `mx`, `nr`, `pj`, `pm`, `replay_from_title`, aux deux | 0 / 26, aux deux |

   Lecture : M6, M7 et M8 sont vertes aux deux arbres. C'est la preuve attendue (chemin jamais atteint, par les TU comme par les e2e), pas un ÉCHEC. M10 montre que le filet voit bien la réponse de `pico8_pi_is_number` sur son cas courant. La même mutation écrite au parent sans `&& 0`, par `return 0;`, ne compile pas (`item` inutilisé, `-Werror`).
5. **Garde mordante** (dans une copie) : `printf 'q-temoin\tquestion\tsite\tsortie\n' >> tools/atteinte_a_prouver.txt` → `make test` code 2, avec `atteinte-à-prouver: tools/atteinte_a_prouver.txt:5:q-temoin	question	site	sortie`, `résidu gardé (noyau-inclut,un-ou-faux,aiguillage-nom,atteinte-à-prouver) : 1 ligne(s)`, `make: *** [Makefile:867: composition-guard] Error 1`. Mesuré.
6. `class-coverage-net` : non exigé (aucune init, aucune table). `sanitize-net` : non exigé. `make raylib` : non exigé.

### Greps de Sortie (sur l'arbre du commit)

1. `grep -cvE '^(#|[[:space:]]|$)' tools/atteinte_a_prouver.txt` → **0** ; `wc -l < tools/atteinte_a_prouver.txt` → **4**.
2. `cat tools/line_coverage_unreachable.txt` → la ligne exacte ci-dessus, et elle seule.
3. `sed -n 76p src/pico8_pi.c` → `        return 0;` ; `sed -n 75p src/pico8_pi.c` → `    if (item->is_false || item->is_ref) {`.
4. `sh tools/composition_residue.sh --check atteinte-à-prouver src tests` → `résidu gardé (atteinte-à-prouver) : 0 ligne(s)`, code 0.
5. `grep -n '^COMPOSITION_RESIDUE_GUARD' Makefile` → `864:COMPOSITION_RESIDUE_GUARD := noyau-inclut,un-ou-faux,aiguillage-nom,atteinte-à-prouver`.
6. `make composition-residue` → pas de ligne `atteinte-à-prouver`, **`résidu 2 : 1290 ligne(s), 88 fichier(s)`**.
7. `git diff --stat <parent>` : `Makefile`, `src/pico8_pi.c`, `tools/atteinte_a_prouver.txt`, `tools/line_coverage_unreachable.txt`, `docs/plan-3.md`, et rien d'autre.

### Budget et tableau « Runs »

Budget : **≈ 50 min**, preuves comprises. Jamais l'appli raylib.

L'implémenteur tient un tableau « Runs », une ligne par lancement. Voici celui des essais de ce PRD. `p` est la copie de `d1e5a30`, `a` la même avec 2.6 appliquée. Les lignes 1 à 3 ont d'abord été faites sur `756b3b9`, avec les mêmes verdicts (pied 1320/91 → 1319/90, `line-coverage` 6334 → 6337).

| # | Commande | Arbre | Code | Ligne clé | Durée |
|---|---|---|---|---|---|
| 1 | `make composition-residue` | `p` | 0 | `atteinte-à-prouver 1 1`, pied 1291 / 89 | < 1 s |
| 2 | 26 e2e sur `pico8_replay_gcov`, `gcov -b` de `pi.c` et `score_panel.c` | `p` | 0 | `:72` branches 5 et 7 `taken 0` ; bras nil de `pj_hb` jamais pris | ≈ 10 s |
| 3 | `make line-coverage` sur les 26 e2e | `p` / `a` | 0 | 6305 → 6308, `L` vide, `pi.c:76` absent | ≈ 5 s × 2 |
| 4 | `make composition-residue` ; `--check atteinte-à-prouver` | `a` | 0 ; 0 | pied 1290 / 88 ; 0 ligne | < 1 s |
| 5 | `make test` ; `make e2e` | `a` | 0 ; 0 | vert ; 26 conformes | ≈ 10 s ; ≈ 1 s |
| 6 | `make test` avec une question rajoutée | `a` | 2 | `composition-guard` Error 1, 1 ligne | ≈ 10 s |
| 7 | M6, M7, M10 au parent ; M6, M7, M8, M10 à l'enfant (`make -k unit` + `make -k e2e`) | `p` / `a` | 0 ou 2 | tableau des mutations | ≈ 30 s × 7 |
| 8 | `make e2e-gap` (hors suite) | `p` / `a` | 2 / 2 | écart 2836 → 2837 | ≈ 7 s × 2 |

### Critères d'échec (`FAILED:` pour le vérificateur)

- `FAILED:` si le départ n'est pas `d1e5a30` (ou un descendant qui ne touche que `docs/`), ou si les comptes de départ diffèrent de 1291 / 89 et `atteinte-à-prouver 1 1` sans explication.
- `FAILED:` si `make test` ou `make e2e` (26 conformes) reste rouge, si un `*.sha256` ou `tests/e2e/` change, ou si une constante `*_HASH`/`FROZEN_*` change.
- `FAILED:` si `L` n'est pas vide, ou si `src/pico8_pi.c:76` est couverte par les 26 e2e.
- `FAILED:` si une mutation est verte à l'enfant et rouge au parent, ou si M10 n'est pas rouge aux deux.
- `FAILED:` si la garde ne vaut pas `noyau-inclut,un-ou-faux,aiguillage-nom,atteinte-à-prouver`, ou si la question rajoutée ne rougit pas `make test`.
- `FAILED:` si une grep de Sortie ne rend pas ce qui est écrit, ou si le pied d'arrivée n'est pas 1290 / 88 sans explication.
- `FAILED:` en cas de tassement (voir « Anti-tassement »).
- `FAILED:` si le message du commit omet : l'argument du cart cité par ligne (les quatre points ci-dessus), les deux sorties `gcov` (branches au parent, `#####` à l'enfant), `L`, les mutations avec leur résultat au parent, la garde mordante, le tableau « Runs ».
- `FAILED:` s'il manque la note « Découvert à 2.6 », ou si le plan change ailleurs.
- `FAILED:` si l'appli raylib est lancée.

## Out of Scope

- **Modéliser le `pairs` du cart** dans `pj:hb` (garder l'ancienne valeur au lieu d'écrire nil) : c'est invisible, donc aucun gain, et cela ajouterait du code à `score_panel.c`.
- **Un `assert` sur le bras faux/nil** : ce serait plus fort qu'une ligne justifiée, mais cela changerait le comportement (arrêt au lieu de `""`) sur un chemin que le TU `test_pico8_fm` frôle déjà (il appelle `pico8_pj_hb` sans `hy`). Le plan ne le demande pas.
- **Rafraîchir `tools/line_coverage_reference.txt`** : elle est figée par choix (`Makefile:775-776`) et ses numéros sont ceux d'un ancien arbre. `make e2e-gap` est rouge avant 2.6 (2836) et le reste après.
- `pico8_rk_y1` et le repli 0 de `pico8_pi_number`, eux aussi non atteints par les données (D.18c) : le plan ne les met pas dans `atteinte-à-prouver`.
- `docs/trous-de-portage.md`, `tools/composition_residue.sh`, son TU ; `class-coverage-net`, `sanitize-net` ; l'appli raylib, l'audio, PSX.

## Further Notes

### Décisions de l'agent à la place de l'utilisateur (G)

- **G1. Le bras faux/nil passe sur sa propre ligne de `pi.c`**, alors que le plan écrit « `pi` (lecture) ». Sans cela, aucune ligne n'est à `#####` : `:72` est exécutée 105 979 fois, et `tools/line_coverage_unreachable.txt` se lit par ligne. Écrire `pi.c:72` dans ce fichier contredirait son contrat (`Makefile:778-779`, « par ligne de la référence qu'aucun rejeu court n'atteint »). Le dépliage garde exactement la même vérité et ne déplace aucun `assert`. Alternative rejetée : garder `:72` et mettre les branches `gcov -b` dans la raison, ce qui n'est pas vérifiable par `make line-coverage`.
- **G2. Pas de `TROU`**, puisque le verdict est « inaccessible » ; l'écart de `pairs` est invisible (point 4).
- **G3. La raison tient sur une ligne** dans `tools/line_coverage_unreachable.txt` (format `fichier:ligne raison`). La preuve longue va au commit.
- **G4. M8 et M10 s'ajoutent à M6/M7** : M8 vise la ligne neuve, et M10 est un témoin qui montre que le filet voit `pico8_pi_is_number`.

### Surprises de mesure, pour `plan-pilot`

1. **`lo` fractionnaire : impossible.** Tous les pas de `hy:nx` sont entiers (0, 1, 2, 3).
2. **Le nil d'`ef[fm.lo]` est atteignable** (« time compression » inventée, `lo` de 16 à 18), **mais seulement entre deux `pj:hb` d'un même `hy:nx`**. Aucun e2e ne l'atteint (bras `:204` jamais pris) ; il serait de toute façon invisible.
3. **`make e2e-gap` est rouge avant 2.6** (2836 lignes sur `d1e5a30`, 2823 sur `756b3b9`), parce que la référence est figée sur un ancien arbre. Le décalage de `pi.c` le fait passer à 2837 (+1 : `tools/line_coverage_reference.txt` contient une `pi.c:76` d'un ancien arbre, désormais ignorée, et deux autres lignes de référence de `pi.c` changent de sort). C'est hors suite, sans rapport avec 2.6.
4. **Première ligne de `tools/line_coverage_unreachable.txt`.** Le fichier était vide depuis sa création.

### Questions pour l'humain

Aucune.
