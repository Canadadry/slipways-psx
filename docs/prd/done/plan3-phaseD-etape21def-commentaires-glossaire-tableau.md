---
title: "Plan 3, étapes D.21d, D.21e, D.21f : deux commentaires faux réécrits et la ligne vide de `ui:bh` retirée, six entrées du glossaire qui citent un symbole supprimé marquées, le tableau du §2 barré — trois commits, aucune ligne exécutable touchée"
description: "La revue de sortie de D (B1, B2, B4) a relevé des textes qui renvoient à un modèle de valeurs ou à des symboles qui n'existent plus. D.21d, e et f les corrigent, chacune dans son commit, sans changer un octet compilé hors du `__LINE__` déclaré de deux `assert`."
status: needs-triage
---

## Problem Statement

**Écrit sur `f5a8c48`** (branche `claude/plan-pilot-3-7zdft0`), en lecture seule : seuls `grep`, `sed`, `awk` et `git` ont tourné. Rien n'a été compilé ni lancé.

Vu du mainteneur du port : la phase D a supprimé la couche objet (`pico8_value`, `pico8_ob`, `pico8_stash`, `pico8_list`, `pico8_type_tag.h`, `pico8_ref_audit.h`), mais trois sortes de textes la décrivent encore comme vivante. Un lecteur qui les suit cherche une analyse de spec, une REF ou une fonction qui n'existe plus.

Lignes des étapes, recopiées de `docs/plan-3.md` à `f5a8c48` (l. 302-304) :

> - D.21d — deux commentaires faux (≈ 15 min, PRD commun avec e et f) : `src/pico8_cart.h:7-13` (analyse de spec disparue depuis D.5) et `src/pico8_score_panel_display.c:291-293` (`ed` n'est plus une REF) réécrits ou supprimés selon les quatre destinations de `CLAUDE.md`. Preuves : sortie du préprocesseur identique ; `gcov` et mutations sans objet.
> - D.21e — glossaire (≈ 20 min, même PRD) : les entrées qui citent `pico8_ft_index`/`pico8_ft_name`, `PICO8_GT`, la « vue ob `pico8_dr` », `pico8_ho_apply` et `unstash` comme symbole C sont marquées supprimées avec l'étape, sur le modèle de `fi`. Compte : 5 entrées ; chaque symbole à 0 par `grep -rnw` dans `src/` et `tests/`, vérifié avant de marquer (un symbole encore présent : l'entrée reste, déclaré au commit).
> - D.21f — tableau du §2 (≈ 15 min, même PRD) : `pico8_value` (D.19c), `pico8_ob` (D.8), `pico8_type_tag.h` (D.20b2) / `pico8_ref_audit.h` (D.13) et `pico8_list` (D.12d) barrés avec leur étape, comme `pico8_class` ; ligne `pico8_entity` sans `mv` (D.10a) ni `ci`. Compte : chaque fichier barré absent (`ls`), chaque symbole retiré de la ligne à 0 dans `src/`.

Règle commune de D.21 (l. 297) : aucune empreinte ne bouge (aucune ligne `*_HASH` ni `FROZEN_*` dans le diff) ; `make test` vert, `make e2e` **26 conformes** ; le mot « REF » ailleurs dans `src/` et `tests/` est pour la phase 3.

**Ajout au périmètre de D.21d** (décidé par le pilote) : `src/pico8_ui.c:359` est une ligne vide que D.21a a laissée à la place du marqueur `/* TROU-6 */`, pour ne pas décaler le `__LINE__` des deux `assert` qui suivent (note « Découvert à D.21a », l. 298). Elle ne porte rien ; elle part. Le décalage `__LINE__` 361/366 → 360/365 est **accepté et déclaré**, comme à D.19c1 (« `__LINE__` des `assert`, identiques avec une ligne vide à la place de l'include »).

### D.21d — ce que disent les commentaires, ce que fait le code

**`src/pico8_cart.h:7-17`** (contrat de `pico8_cart_load`) :

```c
/* Parses every module's spec and builds every data table the cart's
 * top-level statements build, which real Lua runs once before `_init()`.
 * Reads the cart memory through pico8_dispatch_mem(): the caller must have
 * set a `mem` with pico8_load_map_json() already run against it, otherwise
 * the cart data tables are simply not built and every spec still is. A
 * second call frees and reparses every spec, so it dangles every default
 * REF a struct built before it still holds.
 *
 * The asserts in its body cover the ordering constraints that were found;
 * an adjacency they do not cover is not thereby free. Re-derive before
 * inserting an `_init()`. */
```

Le code (`src/pico8_cart.c:48-92`) :

- aucune spec n'est analysée : chaque `pico8_X_init()` appelle d'abord son `pico8_X_shutdown()`, puis lève un drapeau `ready` ; les défauts sont `static const` depuis D.5 ;
- la mémoire cart **n'est pas lue** : `mem = pico8_dispatch_mem(); if (mem != NULL) pico8_data_init();` — `pico8_data_init` (`data_init.c:6-12`) construit `gb` depuis la table générée `pico8_fd_table` (`research.c:298`) et remet `nb.e_` ; `mem` n'est qu'une garde. `pico8_load_map_json` n'est donc pas un prérequis de ce corps ;
- une seconde passe libère ce que la première a construit : `pico8_research_shutdown` → `pico8_research_free_gb_instances` (`research.c:267-272`), `pico8_jo_shutdown` libère `g_jo_oc`/`g_jo_ir` (`planet.c:213-222`), `pico8_probe_shutdown` libère `g_fm` (`probe.c:528-531`). Un pointeur pris entre-temps vers l'un d'eux pend. La phrase « dangles every default REF » est fausse dans son objet (plus de REF par défaut), pas dans son principe.
- le second paragraphe (les `assert` d'ordre, `cart.c:52`, `:77`, `:80`, `:85`) est **juste** : il reste mot pour mot.

Le cart (`pico8/lua.lua`) : les instructions de premier niveau (p. ex. l. 187 `ci=ob(…)`, l. 1389 `foreach(df,ho(df,ob(…)))`) s'exécutent une fois au chargement, avant `_init()`. Rien de plus à citer.

**Texte proposé** (contrat C : ce que construit la fonction, sous quelle condition, ce qui pend ; mise en forme libre, sens imposé) :

```c
/* Runs every module's init, then builds the tables the cart's top-level
 * statements build (real Lua runs them once before `_init()`), but only
 * when pico8_dispatch_mem() is set: with a NULL mem they stay unbuilt. A
 * second call frees what the first built before building it again, so a
 * pointer into it taken in between dangles.
 *
 * The asserts in its body cover the ordering constraints that were found;
 * an adjacency they do not cover is not thereby free. Re-derive before
 * inserting an `_init()`. */
```

L'implémenteur vérifie avant d'écrire, par `grep -n 'mem' src/pico8_data_init.c src/pico8_research.c src/pico8_tech_gate.c`, que `pico8_data_init` ne lit toujours pas la mémoire ; s'il la lit, la clause « with pico8_load_map_json() already run against it » est gardée et c'est déclaré.

**`src/pico8_score_panel_display.c:291-293`** :

```c
        /* `self.ed`'s class-level default is the number 1, never a REF, and a
         * non-REF `ed` is never `kc`: the block has to fire on the first
         * eligible tick. Comparing only a REF-valued `ed` silently disables it. */
        if (self != NULL) {
            ed_equals_kc = self->ed != NULL && pico8_kc() == self->ed;
        }
```

- `ed` est un `pico8_mh_t *` (`score_panel.h:24`), NULL au départ (`score_panel.c:297`), et `score_panel.h:17-18` dit déjà « `ed` NULL reads as `pico8_pj_start.ed`, never nil » ; `pico8_pj_start.ed = 0x10000` (`game_data.c:482`), le nombre 1 ;
- `pico8_kc()` rend la boîte ouverte **ou NULL** (`msgbox.h:40-43`) ;
- le cart, l. 846 `ed=1` et l. 882 `if self.c_ and kc~=self.ed then` : `nil ~= 1` est vrai, le bloc tire au premier tick.

Le piège est réel et se réintroduit en un geste : sans `self->ed != NULL`, un `ed` NULL et un `kc` NULL sont égaux, le bloc ne tire jamais. Le commentaire reste (sorte « piège », ≤ 40 mots), débarrassé de « REF ».

**Texte proposé** (39 mots) :

```c
        /* A NULL `ed` is the class default, the number 1, which is never
         * `kc`, not even a nil one: without the `ed != NULL` test, NULL == NULL
         * would keep the block from firing on the first eligible tick. */
```

**`src/pico8_ui.c:359`** : la ligne vide entre `kn.held != NULL) {` (l. 358) et `if (l) {` (l. 360) est supprimée. Rien d'autre dans le fichier. Les deux `assert` de `pico8_ui_bh` passent de 361/366 à **360/365** ; ce sont les deux seuls `assert` de `ui.c` après la ligne 359 (`awk 'NR>359 && /assert\(/'`), et `ui.c` n'emploie pas `__LINE__`/`__FILE__` lui-même. Aucun test ni outil de la suite ne lit ces numéros : `tools/line_coverage_reference.txt` en porte (`:5515` `ui.c:359`…), mais il est décalé depuis #32, non recalculé, et `make e2e-gap` est hors suite (note D.13) — il n'est pas touché.

### D.21e — les symboles supprimés du glossaire

Vérifié à `f5a8c48`, `grep -rnw <symbole> src tests | wc -l` :

| Symbole | `src/` | `tests/` | Parti à | Commit |
|---|---|---|---|---|
| `pico8_ft_index` | 0 | 0 | D.7a | `e252fde` |
| `pico8_ft_name` | 0 | 0 | D.7a | `e252fde` |
| `PICO8_GT` | 0 | 0 | D.8 (avec `pico8_stash.c`) | `ad85525` |
| `pico8_dr` (la vue ob, `pico8_dr(void)` de `data_init.h`) | 0 | 0 | D.6b | `ca29aaa` |
| `pico8_ho_apply` | 0 | 0 | D.8 (dans `collect.c`) | `ad85525` |
| `pico8_unstash` | 0 | 0 | D.8 | `ad85525` |
| `unstash` (mot nu) | 0 | **1** | — | — |

Le seul `unstash` de `tests/` est `tests/test_data_residue.c:63`, une chaîne coupée (`"pico8_" "unstash(x);\n"`) : la fixture qui prouve que la garde `stash` mord. **Gardée, nommée** ; elle ne rend pas le symbole vivant.

Lignes du glossaire, vérifiées à `f5a8c48` : `:40`, `:54`, `:138`, `:292`, `:656` sont celles de la revue. **Une sixième** : `:293` (`pico8_ho_gp_has` « lu par `pico8_ho_apply` et par `hp` ») cite le même symbole supprimé ; son seul lecteur est `hp_ho` (`research.c:106`, appel `:118`). Le compte passe donc de 5 à **6 lignes** (5 entrées de la revue + la ligne voisine de `ho`), déclaré au commit.

Ce que chaque symbole est devenu : `ft` — `ui:gw` ajoute directement aux couches de `ui.ic` (`pico8_ptrs_t`, `ui.c:106-109`) ; `gt` — plus rien, les tables décodées sont générées ; `dr` — `pico8_dr_table` lu directement par `eu()` (`cart.c:190`) ; `ho` — `hp_ho`, `static` dans `pico8_research.c` ; `unstash` — plus de fonction C qui rende une chaîne décodée.

**Autres symboles de la couche supprimée** — `grep -n 'pico8_value\|pico8_ob\b\|pico8_ob_\|stash\|ref_audit\|ref-audit\|type_tag\|pico8_list\|pico8_class\|instance_get_field\|value_ref' docs/glossaire-noms.md` ne rend que : `:55` (`unstash(pd)` : nom de fonction **du cart**, Symbole C déjà « — (tables décodées générées : `pico8_map_tables.c`) », juste), `:54` (idem, le mot `unstash` dans le sens de `gt`), `:262` (`fi`, déjà marqué : le modèle), `:656` (traité ici). `pico8_mv` (`:38`) est déjà marqué « supprimé (D.10a) ». **Aucun autre symbole C supprimé n'est cité.** Hors périmètre, relevé pour mémoire : `:275` (`ns`, « remplacée par la remontée de chaîne à chaque lecture de champ ») et `:644` (`ko`, `iy`) décrivent peut-être un mécanisme de la couche objet ; ce ne sont pas des symboles C, D.21e n'y touche pas (question ouverte 2).

### D.21f — le tableau du §2

À `f5a8c48`, `docs/plan-3.md:21-29` :

```
| `pico8_value` | valeur taguée (fixe, chaîne, booléen, REF, fonction) |
| `pico8_ob` | table ordonnée clé/valeur + parseur de spec `ob()`/`fi()` |
| ~~`pico8_class`~~ | classes à chaîne `parent`, … ; supprimé à 1a.42e, … |
| `pico8_entity` | `hv`/`cv`/`cg`, `lt`/`he`, `dc`/`mv`, `ci`, `mo` |
| `pico8_type_tag.h`, `pico8_ref_audit.h` | désambiguïsation des REF |
| `pico8_list` | tableau Lua |
```

`ls src/pico8_value.* src/pico8_ob.* src/pico8_type_tag.h src/pico8_ref_audit.h src/pico8_list.*` : **tous absents** ; `src/pico8_entity.c/.h` présents. Commits de suppression : `011c168` (D.19c2), `ad85525` (D.8), `9d45f74` (D.20b2), `ce41e38` (D.13), `d346ffc` (D.12d).

`mv` : `pico8_mv` = 0 dans `src/` (supprimé à D.10a, `d05e35e`) ; le mot nu `mv` ne reste que dans un commentaire qui nomme la fonction **du cart** (`ui_button.h:119`). Retiré de la ligne.

`ci` : **écart à la ligne du plan**. La revue dit que `ci` « n'existe plus » ; c'est inexact. `ci` est une table de données du cart (`pico8/lua.lua:187`, `ci=ob([[0.5,1,…]])`), portée par `pico8_ci_table` (`game_data.h:449`, `game_data.c:19`) et lue par `dc` (`entity.c:281`). Elle n'est pas un mécanisme de la couche : c'est une donnée que `pico8_entity` lit, comme il lit bien d'autres tables générées. La retirer de la ligne est juste ; le compte « à 0 dans `src/` » ne l'est pas pour elle. Compte remplacé : `grep -rn 'pico8_ci_table' src` = **3** lignes (déclaration, définition, lecture par `dc`), aucune dans `pico8_entity.h`.

## Solution

Du point de vue du mainteneur : les deux contrats relus disent ce que fait le code ; le glossaire ne donne plus pour vivant un symbole disparu ; le tableau du §2 montre d'un coup d'œil que la couche est partie, sauf `pico8_entity`. Trois commits, dans cet ordre.

### Commit 1 — `refactor(D.21d): …` (ou `docs(D.21d)`)

1. `src/pico8_cart.h:7-13` : le premier paragraphe remplacé par le texte proposé ci-dessus ; le second (l. 15-17) inchangé.
2. `src/pico8_score_panel_display.c:291-293` : remplacé par le texte proposé ci-dessus. La ligne de code `:295` inchangée.
3. `src/pico8_ui.c:359` : ligne vide supprimée.

### Commit 2 — `docs(D.21e): …`

`docs/glossaire-noms.md`, six lignes, sur deux modèles déjà présents dans le fichier : `fi` (`:262`, « — (supprimé à D.8 avec `pico8_ob.c`) ») quand rien ne remplace le symbole, `mv` (`:38`, « … ; `pico8_mv` supprimé (D.10a) ») quand un autre symbole le remplace.

| Ligne | Colonne « Symbole C » (ou texte) après |
|---|---|
| `:40` `ft` | `— (pico8_ft_index, pico8_ft_name supprimés à D.7a)` ; sens : garder la citation du cart, retirer « La clé polymorphe … (`ui:gw`) » (elle décrit les deux points d'entrée disparus) |
| `:54` `gt` | `— (PICO8_GT supprimé à D.8 avec pico8_stash.c)` ; sens inchangé |
| `:138` `dr` | `pico8_dr_table` ; `vue ob pico8_dr supprimée (D.6b)` ; sens inchangé |
| `:292` `ho` | `hp_ho` (`static`, `pico8_research.c`) ; `pico8_ho_apply supprimé (D.8)` ; sens inchangé |
| `:293` | « … ; lu par `hp_ho` » au lieu de « lu par `pico8_ho_apply` et par `hp` » |
| `:656` | « `tostr`, `sub` et `ne` rendent une chaîne … » (`unstash` retiré de la liste : plus de fonction C) |

(Symboles entre backticks dans le fichier ; mise en forme libre, sens imposé.)

### Commit 3 — `docs(D.21f): …`

`docs/plan-3.md`, tableau du §2, sur le modèle de `pico8_class` :

```
| ~~`pico8_value`~~ | valeur taguée (fixe, chaîne, booléen, REF, fonction) ; supprimé à D.19c |
| ~~`pico8_ob`~~ | table ordonnée clé/valeur + parseur de spec `ob()`/`fi()` ; supprimé à D.8 |
| ~~`pico8_class`~~ | (inchangé) |
| `pico8_entity` | `hv`/`cv`/`cg`, `lt`/`he`, `dc`, `mo` |
| ~~`pico8_type_tag.h`~~, ~~`pico8_ref_audit.h`~~ | désambiguïsation des REF ; supprimés à D.20b2 et D.13 |
| ~~`pico8_list`~~ | tableau Lua ; supprimé à D.12d |
```

Rien d'autre dans `plan-3.md` : la note « Découvert à D.21d/e/f » est le travail du pilote.

## User Stories

1. En tant que lecteur de `pico8_cart.h`, je veux que le contrat de `pico8_cart_load` dise ce que la fonction construit et à quelle condition, afin de ne pas chercher une analyse de spec qui n'existe plus.
2. En tant qu'auteur d'un test, je veux savoir qu'un `mem` NULL laisse `gb` et `nb.e_` non construits, afin de poser ce que mon test exige et pas plus.
3. En tant que mainteneur, je veux savoir qu'un second `pico8_cart_load` libère ce que le premier a construit, afin de ne pas garder un pointeur vers `gb` d'une partie à l'autre.
4. En tant que mainteneur de `score_panel_display.c`, je veux que le piège `ed != NULL` soit décrit dans le modèle actuel (pointeur, NULL = défaut 1), afin de ne pas le « simplifier » en `kc == ed`.
5. En tant que relecteur, je veux que le texte du piège cite le cart (`kc~=self.ed`, `ed=1`) au PRD, pas au code, afin que le commentaire reste court.
6. En tant que mainteneur, je veux que la ligne vide orpheline de `ui:bh` disparaisse, afin qu'un vide ne passe pas pour une trace de quelque chose.
7. En tant que relecteur, je veux la preuve que le seul effet de ce retrait est `__LINE__` de deux `assert`, afin d'accepter un préprocesseur non identique octet pour octet.
8. En tant que lecteur du glossaire, je veux que chaque symbole C cité existe, ou soit marqué supprimé avec son étape, afin de ne pas `grep` pour rien.
9. En tant que lecteur du glossaire, je veux, quand un symbole a un successeur (`hp_ho`, `pico8_dr_table`), qu'il soit nommé, afin de retrouver le code.
10. En tant que lecteur du glossaire, je veux que les noms **du cart** (`unstash`, `gt`, `ft`, `ho`) restent en colonne 1, afin de garder la traçabilité vers `pico8/lua.lua`.
11. En tant que lecteur du plan, je veux voir d'un coup d'œil, au §2, quels modules de la couche sont partis et à quelle étape, afin de mesurer ce qui reste.
12. En tant que lecteur du plan, je veux que la ligne `pico8_entity` ne nomme que ce que le module porte encore, afin de ne pas y chercher `mv`.
13. En tant que pilote, je veux un commit par étape, afin que la note « Découvert à » de chacune pointe un hash.
14. En tant que pilote, je veux que chaque grep de Sortie « vide » nomme ce qu'il exclut, afin qu'un vide soit vérifiable par un tiers.
15. En tant que pilote, je veux que l'écart sur `ci` (donnée vivante, pas symbole supprimé) soit écrit au PRD, afin que le compte de D.21f ne soit pas déclaré faux au commit.
16. En tant que relecteur, je veux qu'aucune empreinte, aucun test ni aucun e2e ne bouge, afin de n'avoir rien à re-baseliner.

## Implementation Decisions

### Modules

- Commit 1 : `src/pico8_cart.h`, `src/pico8_score_panel_display.c`, `src/pico8_ui.c`. Rien d'autre.
- Commit 2 : `docs/glossaire-noms.md`. Rien d'autre.
- Commit 3 : `docs/plan-3.md`, tableau du §2 seulement (l. 21-29 à `f5a8c48`). Rien d'autre.

Pas de `tests/`, `Makefile`, `tools/` (dont `line_coverage_reference.txt`), `docs/trous-de-portage.md`, `*.sha256`, `tests/e2e/`.

### Aucune ligne exécutable touchée

Commit 1 : chaque ligne `-`/`+` du diff de `src/` est une ligne de commentaire, sauf la ligne vide `ui.c:359` retirée (une ligne `-` vide). Les `assert`, leurs messages et `ed_equals_kc = …` restent mot pour mot.

### Commentaires (règle de `CLAUDE.md`)

- `cart.h` : contrat C (ce que construit la fonction, condition sur `mem`, ce qui pend après un second appel). Le second paragraphe, déjà un piège d'ordre, reste.
- `score_panel_display.c` : piège qu'on réintroduirait, ≤ 40 mots.
- Aucun commentaire ne cite D.5, D.21, la revue, le plan, « REF », « spec » ou l'historique : tout ça va au message de commit.
- Le mot « REF » ailleurs (`research.h:13`, `planet_trade.c:240`, `research_icon.c:22`, `probe.h:38`, `:137`, et les 9 lignes de `tests/`) n'est pas touché (phase 3).

### Ce qui ne change pas

Toute constante `*_HASH`, `FROZEN_*` ; tout test ; tout e2e ; les numéros des TROU ; les autres lignes du glossaire et du plan.

## Testing Decisions

Rien d'exécutable ne change : `gcov` et mutations sans objet, comme le disent les lignes du plan. La preuve est que rien de compilé ne bouge, hors l'écart `__LINE__` déclaré.

### D.21d — préprocesseur identique, à l'écart `__LINE__` près

Au parent (`f5a8c48` ou le HEAD de départ) puis à l'arrivée du commit 1, dans le scratchpad, pour **chacun des 154** `.c` de `src/` et `tests/` (les deux en-têtes touchés sont inclus par plusieurs unités ; `raylib.h` vide fourni aux TU raylib, comme à D.21a) :

```sh
for f in src/*.c tests/*.c; do
  cc -std=c99 -Isrc -I"$STUB" -E -P "$f" | grep -v '^[[:space:]]*$' > "$OUT/$(echo "$f" | tr / _).i"
done
```

Attendus :

1. `diff -rq départ arrivée` → **un seul fichier** : `src_pico8_ui.c.i`.
2. `diff départ/src_pico8_ui.c.i arrivée/src_pico8_ui.c.i` → exactement **2 lignes changées**, et chacune ne diffère que par le nombre de ligne passé à `__assert_fail` : `361` → `360`, `366` → `365`.
3. Normalisation : dans la copie de départ de `src_pico8_ui.c.i`, remplacer ces deux nombres (`sed` ciblé sur les deux lignes `__assert_fail` qui citent `ui.lheld:` et `ui.rheld:`) → `diff` avec l'arrivée **vide**.

**Contre-épreuves** (chacune dans une copie jetable, rien de committé) :

- C1, l'écart n'est que `__LINE__` : les mêmes deux passes avec `-DNDEBUG` → `diff -rq` **vide** sur les 154 fichiers (les `assert` disparaissent, le décalage avec eux).
- C2, le comparateur mord : dans une copie de l'arbre d'arrivée, changer un caractère d'un message d'`assert` de `ui.c` (p. ex. `"ui.rheld:"` → `"ui.rheld;"`) → l'étape 3 (normalisée) rend **non vide**. Sans cela, un diff vide ne prouverait rien.
- C3, `NDEBUG` n'est pas posé par la suite : `grep -n NDEBUG Makefile` → vide (les `assert` sont compilés, donc le décalage est réel et il faut le déclarer, pas l'ignorer).

### Diff limité aux commentaires

```sh
git diff -U0 HEAD^ HEAD -- src | grep -E '^[-+][^-+]' | grep -vE '^[-+][[:space:]]*(/\*|\*)'
```

→ **vide** pour le commit 1 (la ligne vide retirée est une ligne `-` nue, que le `[^-+]` n'attrape pas : `git diff -U0 HEAD^ HEAD -- src/pico8_ui.c` la montre seule, `@@ -359 +358,0 @@`).

Commits 2 et 3 : `git show --stat HEAD` → le seul fichier nommé.

### Filet (après chacun des trois commits)

- `make test` vert ;
- `make e2e` : **26 conformes** ;
- `git diff <départ> HEAD -- tests Makefile tools '*.sha256'` → vide ;
- `git diff <départ> HEAD | grep -E '^[-+].*(_HASH|FROZEN_)'` → vide.

### Greps de Sortie

Chaque « vide » exclut nommément ce qu'on garde.

**D.21d**

1. `grep -n 'spec\|reparse\|REF' src/pico8_cart.h` → **vide**.
2. `grep -n 'REF' src/pico8_score_panel_display.c` → **vide**.
3. `grep -rnw 'REF' src | wc -l` : 9 → **5** ; les 5 restants, nommés : `research.h:13`, `planet_trade.c:240`, `research_icon.c:22`, `probe.h:38`, `probe.h:137` (phase 3). `grep -rnw REF tests | wc -l` : 9 → **9**.
4. `awk 'NR>=356 && NR<=366' src/pico8_ui.c | grep -c '^[[:space:]]*$'` → **1** (la ligne vide qui sépare les déclarations de `if (pico8_ui_jz…`, l. 356, gardée) ; `grep -n 'assert(ui_is_held_class\|assert(kn.held->cls' src/pico8_ui.c` → `:360`, `:365`.
5. `grep -n 'TROU-' src/pico8_ui.c` → `:213`, `:471` (ex-`:472`, TROU-7).
6. `grep -c 'self->ed != NULL && pico8_kc() == self->ed' src/pico8_score_panel_display.c` → **1**.

**D.21e**

7. `grep -rnw 'pico8_ft_index\|pico8_ft_name\|PICO8_GT\|pico8_ho_apply\|pico8_unstash' src tests` → **vide** ; `grep -rn 'pico8_dr(' src tests` → **vide** ; `grep -rnw unstash src tests` → **une** ligne, `tests/test_data_residue.c:63` (fixture de la garde `stash`, gardée).
8. `grep -n 'pico8_ft_index\|pico8_ft_name\|PICO8_GT\|pico8_ho_apply\|vue ob' docs/glossaire-noms.md | grep -v 'supprimé'` → **vide**. Gardées par nom : les lignes `:40`, `:54`, `:138`, `:292`, qui citent le symbole **avec** « supprimé ».
9. `grep -n 'unstash' docs/glossaire-noms.md` → exactement `:54` (sens de `gt`) et `:55` (`unstash(pd)`, nom de fonction du cart) ; `:656` n'y est plus.
10. `grep -c 'supprimé' docs/glossaire-noms.md` : **2 → 6** (`:38` `mv` et `:262` `fi` au départ ; gagnent `:40`, `:54`, `:138`, `:292` — « supprimés »/« supprimée » comptent ; `:293` et `:656` n'en gagnent pas). Un autre nombre est expliqué ligne à ligne au message.
11. `grep -n 'pico8_ho_gp_has' docs/glossaire-noms.md` → `:293` seule, qui dit « lu par `hp_ho` ».

**D.21f**

12. `ls src/pico8_value.c src/pico8_value.h src/pico8_ob.c src/pico8_ob.h src/pico8_type_tag.h src/pico8_ref_audit.h src/pico8_list.c src/pico8_list.h 2>&1 | grep -vc 'No such file'` → **0** ; `ls src/pico8_entity.c src/pico8_entity.h` → deux fichiers.
13. `sed -n '21,29p' docs/plan-3.md | grep -c '~~'` : **1 → 5** lignes barrées (`pico8_value`, `pico8_ob`, `pico8_class`, `pico8_type_tag.h`/`pico8_ref_audit.h`, `pico8_list`) ; `sed -n '21,29p' docs/plan-3.md | grep -o '~~[^~]*~~' | wc -l` : **1 → 6** noms barrés.
14. `sed -n '21,29p' docs/plan-3.md | grep 'pico8_entity'` → `| \`pico8_entity\` | \`hv\`/\`cv\`/\`cg\`, \`lt\`/\`he\`, \`dc\`, \`mo\` |` ; `grep -rnw 'pico8_mv' src` → **vide** ; `grep -rn 'pico8_ci_table' src` → **3** (`game_data.h:449`, `game_data.c:19`, `entity.c:281` : la donnée du cart lue par `dc`, gardée — voir l'écart plus haut).
15. `git diff HEAD^ HEAD -- docs/plan-3.md | grep -c '^[-+]|'` → 10 (5 lignes `-`, 5 lignes `+`), aucune autre ligne.

### Budget et tableau « Runs »

**≈ 50 min** pour les trois commits, preuves comprises (d 15, e 20, f 15) ; plafond 65 min.

| Cible | Durée attendue |
|---|---|
| `make test` | de l'ordre de la minute |
| `make e2e` | ~1 s |
| préprocesseur, départ + arrivée + `-DNDEBUG` + C2 | ~2 min |
| greps | secondes |

L'implémenteur tient un tableau « Runs » (une ligne par lancement : #, commande exacte, état de l'arbre, code de sortie, ligne clé, durée) et le met au message de chaque commit, pour les lancements qui le concernent.

### Critères d'échec (`FAILED:` pour le vérificateur de `prd-autopilot`)

- `FAILED:` si l'arbre de départ n'est pas `f5a8c48` ou un descendant propre qui ne touche aucun des cinq fichiers de « Modules ».
- `FAILED:` si les trois étapes ne sont pas trois commits distincts, dans l'ordre d, e, f, ou si un commit touche un fichier hors de sa ligne de « Modules ».
- `FAILED:` si une ligne non commentaire de `src/` change (hors la ligne vide `ui.c:359`), ou si le préprocesseur diffère ailleurs que dans les deux `__LINE__` déclarés, ou si C1 ou C2 ne rend pas ce qui est écrit.
- `FAILED:` si `make test` n'est pas vert ou si `make e2e` ne rend pas 26 conformes après un commit (3 tentatives au plus).
- `FAILED:` si une constante `*_HASH` ou `FROZEN_*` change, si un `*.sha256` ou un fichier de `tests/e2e/` est touché.
- `FAILED:` si un commentaire écrit sort des quatre sortes de `CLAUDE.md`, dépasse 40 mots pour le piège, ou cite D.5, D.21, la revue, le plan, « REF » ou « spec ».
- `FAILED:` si un symbole marqué supprimé au glossaire a une occurrence `-w` dans `src/` ou `tests/` hors la fixture nommée ; si un nom **du cart** est retiré de la colonne 1.
- `FAILED:` si une grep de Sortie ne rend pas ce qui y est écrit, ou si une exclusion y est ajoutée sans être rapportée.
- `FAILED:` s'il manque au message : le rapport du préprocesseur (fichiers comparés, les deux lignes, C1, C2), les comptes de Sortie avant/après, l'écart `ci`, la sixième ligne du glossaire, le tableau « Runs ».

## Out of Scope

- Le mot « REF » restant dans `src/` (5 lignes après D.21d) et `tests/` (9) : phase 3.
- `:275` (`ns`) et `:644` (`ko`, `iy`) du glossaire, s'ils décrivent la couche disparue : pas des symboles C (question ouverte 2).
- Recalculer `tools/line_coverage_reference.txt` (décalé depuis #32).
- Les autres lignes de D.21 (a, b1, b2, c) et les points B3, B5-B10 de la revue.
- Toute ligne exécutable ; `make raylib` exécuté ; l'appli raylib ; l'audio ; la PSX.

## Further Notes

**Questions ouvertes (pour l'humain, non bloquantes).**

1. **`ci` dans le tableau.** Le plan demande de retirer `ci` de la ligne `pico8_entity` parce qu'il « n'existe plus ». En fait, `ci` existe toujours : c'est une petite table de données du jeu (la taille de dessin de chaque couche), et le module d'entités la lit. Ce PRD la retire quand même de la ligne, parce que ce n'est pas un morceau de la couche objet mais une donnée du jeu. Est-ce bien ce que tu voulais, ou préfères-tu la garder dans la ligne ?
2. **Deux entrées du glossaire à relire plus tard.** Dans la section sur les classes, `ns` est décrit comme « remplacé par la remontée de chaîne à chaque lecture de champ », et `ko`/`iy` parlent de « remontée de chaîne ». Ce mécanisme appartenait peut-être à la couche supprimée. Ce ne sont pas des symboles C, donc hors de D.21e. Faut-il une petite passe à part pour les relire ?
3. **Le second appel de `pico8_cart_load`.** Le nouveau commentaire dit qu'un second appel libère ce que le premier a construit (les projets de recherche, entre autres), donc qu'un pointeur gardé vers eux devient invalide. C'est vrai du code, mais est-ce qu'un second appel arrive vraiment dans le jeu (nouvelle partie) ou seulement dans les tests ? Si c'est seulement dans les tests, faut-il le dire ?

**Ordre des commits.** d touche du code (preuve lourde), e et f ne touchent que des docs : d d'abord, pour que ses preuves portent sur un parent sans autre changement.

## Annexe — reproduire (lecture seule)

```sh
sed -n 7,17p src/pico8_cart.h; sed -n 48,92p src/pico8_cart.c
sed -n 6,16p src/pico8_data_init.c; sed -n 267,272p src/pico8_research.c
sed -n 287,296p src/pico8_score_panel_display.c; sed -n 17,24p src/pico8_score_panel.h
sed -n 846p pico8/lua.lua; sed -n 882p pico8/lua.lua
sed -n 356,367p src/pico8_ui.c; awk 'NR>359 && /assert\(/' src/pico8_ui.c
for n in 40 54 138 262 292 293 656; do sed -n "${n}p" docs/glossaire-noms.md; done
for s in pico8_ft_index pico8_ft_name PICO8_GT pico8_ho_apply pico8_unstash unstash; do echo "$s $(grep -rnw "$s" src tests | wc -l)"; done
git log --oneline -S'pico8_ft_index' -- src | head -1   # e252fde, D.7a
git log --oneline -S'pico8_dr(' -- src | head -1        # ca29aaa, D.6b
sed -n 21,29p docs/plan-3.md
grep -rn 'pico8_ci_table' src; sed -n 187p pico8/lua.lua
```
