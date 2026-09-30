---
title: "Plan 3, étape D.21a : TROU-6 fermé — le cart vérifie déjà que ce que rend `ldown`/`rdown` est une table, le port le porte par `held != NULL` sous un `assert` de classe, et le cas « `ldown` rend un booléen » est `stop` sans `held` ; l'entrée est supprimée, le marqueur de `ui:bh` retiré, ceux de `jo:ldown` et `nr:ldown` renvoyés à TROU-7"
description: "TROU-6 déclare un écart au cart (« castée sans vérification ») qui n'existe plus, et qui n'a en fait jamais existé côté comportement : le cart filtre par `type(pw.kn)==\"table\"`. D.21a ferme l'entrée et range ses trois marqueurs, sans toucher une ligne de code."
status: needs-triage
---

## Problem Statement

**Écrit sur `f4960fc`** (branche `claude/plan-pilot-3-7zdft0`), en lecture seule : seuls `grep`, `sed` et `git` ont tourné. Rien n'a été compilé ni lancé.

Vu du mainteneur du port : `docs/trous-de-portage.md` est la liste des écarts connus entre le port et le cart. Son entrée **TROU-6** (l. 90-103 à `f4960fc`) dit que « l'entité tenue par la souris est castée sans vérification » en `pico8_instance_t*`, que « rien ne garantit que c'en soit une ». Trois marqueurs `/* TROU-6 */` y renvoient. La revue de sortie de D (point M1, « Moyenne ») l'a relevé : l'entrée est fausse. Un écart déclaré qui n'existe pas coûte deux fois — il fait chercher un bug qui n'est pas là, et il dilue la liste des vrais écarts.

Ligne de l'étape, recopiée de `docs/plan-3.md` à `f4960fc` (l. 298) :

> - D.21a — TROU-6 (≈ 30 min) : l'entrée de `docs/trous-de-portage.md` décrit un cast sans vérification qui n'existe plus (`ui.c:360-366` asserte la classe de `lheld`/`rheld`). Fermée (entrée supprimée, convention du fichier) si le seul cas restant, « `ldown` rend un booléen », est entièrement porté par `pico8_down_t.stop` ; sinon réécrite sur ce seul cas. Les trois marqueurs (`ui.c:359`, `planet.h:153`, `planet_family_trade.h:82`, les deux derniers non cités par la revue) suivent : retirés, ou renvoyés vers l'entrée qui décrit vraiment le site (la fuite de l'instance `jc` rendue par `ldown` est-elle dans TROU-7 ? si non, marqueur retiré et rien d'inventé). Compte : `grep -rn 'TROU-6' src/ tests/ docs/` = 0 si fermée, sinon les seuls sites du cas restant, nommés au commit. Preuves : sortie du préprocesseur (`gcc -E -P`) identique pour chaque fichier de `src/` touché ; `gcov` et mutations sans objet (aucune ligne exécutable ne change).

Règle commune de D.21 (l. 297) : aucune empreinte ne bouge ; `make test` vert, `make e2e` **26 conformes**.

### La preuve : l'écart n'existe plus (et le cas booléen n'en est pas un)

**Le cart** (`pico8/lua.lua:237-247`) :

```lua
function ui:bh(bt)local b,ni=je[bt],bt.."held"if b.jn then
local pw=self:jz(bt.."down",self.qa,je.ra)if pw and type(pw.kn)=="table" then
self[ni]=pw.kn
end end
```

et `ui:jz` (l. 248-253) rend `{kv=h,kn=kn}` pour la première entité, couche 15 → 0, dont la méthode rend une valeur vraie. Deux faits :

1. **Une valeur vraie arrête la recherche** (`if kn then return …`), qu'elle soit une table ou `true`.
2. **Seule une table est tenue** (`type(pw.kn)=="table"`). Un `true` arrête la recherche mais laisse `lheld`/`rheld` tels qu'ils étaient. Le cart vérifie donc déjà — par le type Lua — ce que TROU-6 dit « non vérifié ».

**Ce que rend chaque `ldown`/`rdown` du cart**, et son portage (`pico8_down_t` = `{int stop; pico8_struct_header_t *held;}`, `src/pico8_struct.h:51-54`) :

| Méthode (cart) | Rend | Port | `stop` | `held` |
|---|---|---|---|---|
| `js:ldown` (l. 277) | `true` | `pico8_js_ldown` (`ui_button.c:308`) | 1 | NULL |
| `fq:ldown` (l. 327) | `true` | `pico8_fq_ldown` (`ui_button.c:394`) | 1 | NULL |
| `jo:ldown`, `me=="known"` (l. 412) | `true` | `pico8_jo_ldown` (`planet.c:499-502`) | 1 | NULL |
| `jo:ldown`, `me=="ly"` | `jc(…)` | `planet.c:504-510` | 1 | le `jc` |
| `jo:ldown`, autre `me` | nil | `planet.c:512` | 0 | NULL |
| `nr:ldown` (l. 1112) | `self:gv({}) and jc(…)` : faux/nil ou `jc` | `planet_family_trade.c:57-75` | 0 ou 1 | NULL ou le `jc` |
| `bg:ldown` (l. 376) | un `g_` ou un `mz` | `pico8_bg_ldown` (`bg.c:174`) | 1 | l'entité |
| `bg:rdown` (l. 378) | `self` (le `bg`) | `pico8_bg_rdown` (`bg.c:195-201`) | 1 | le `bg` |
| `ie:ldown` (l. 932) | `self.ke({…})` : une instance de `qh`, `os`, `nr`, `fr`, `nl`, `mx` ou `hj` | `pico8_ie_ldown` (`research_icon.c:147-148`) | 1 si instance | l'instance |

**Le port** (`src/pico8_ui.c:354-367` et `:400-445`) : `pico8_ui_jz` s'arrête sur `kn.stop` (fait 1) ; `pico8_ui_bh` ne range que si `kn.held != NULL` (fait 2), puis asserte la classe — `ui_is_held_class` (`jc` ∪ `g_` ∪ `mz` ∪ classes à état `LY` sauf `pm`) pour `lheld`, `== pico8_bg_struct_class()` pour `rheld`. L'ensemble de l'`assert` est exactement la colonne « `held` » du tableau.

Donc :

- « castée sans vérification » : **faux** depuis 1b.37 (`ui_take_held`/`ui_take_bg`), repris tel quel à D.10a ; `pico8_instance_t` n'existe plus depuis D.8 ;
- « en Lua c'est n'importe quelle table » : vrai pour le langage, mais **aucune** méthode du cart ne rend autre chose que la liste ci-dessus, et le port refuse (`assert`) toute autre classe — c'est un contrat, pas un écart ;
- « `ldown` rend un booléen » : c'est `stop == 1, held == NULL`, qui arrête la recherche sans rien tenir, exactement comme `if kn` puis `type(kn)=="table"`. **Entièrement porté.** Aucun cas ne reste à décrire.

**Décision : TROU-6 est fermée**, entrée supprimée (convention du fichier, l. 7-8 : « Une entrée supprimée est une entrée refermée : voir git »). Les numéros des autres entrées ne bougent pas.

### Les trois marqueurs

| Marqueur | Ce qu'il marque vraiment | Sort |
|---|---|---|
| `src/pico8_ui.c:359`, dans `pico8_ui_bh`, au rangement de `kn.held` | le cast de TROU-6, qui est désormais sous `assert` | **retiré** (ligne supprimée) |
| `src/pico8_planet.h:153`, sur `pico8_jo_ldown` | le `jc` neuf que `ui:bh` tient puis lâche (`lheld = NULL`), jamais libéré | **renvoyé à TROU-7** |
| `src/pico8_planet_family_trade.h:82`, sur `pico8_nr_ldown` | idem | **renvoyé à TROU-7** |

**Pourquoi TROU-7 décrit vraiment ces deux sites.** TROU-7 (« Fuites mémoire assumées ») s'ouvre sur « Ce port n'a pas de ramasse-miettes là où Lua en a un », et sa deuxième convention est « une réaffectation de champ abandonne l'ancienne valeur » — ce que fait `ui:bh` (`self[ni]=nil`, port `self->lheld = NULL`) sur le `jc` : si `jc:ey` échoue, le `jc` est marqué `p_` et sort de `hv` par `he()`, qui ne libère rien (`entity.c:217-241`) ; le ramasse-miettes du cart le reprendrait. Le dépôt rattache déjà ce genre de fuite à TROU-7, sans ambiguïté :

- `Makefile:862` : « `detect_leaks=0` : les entités ne sont jamais libérées (TROU-7) » ;
- dans les **mêmes** en-têtes : `planet.h:24`, `planet_family_trade.h:21`, `:32`, `:43` (« Nothing frees the struct: TROU-7 ») ;
- le **jumeau exact** : `pico8_ie_ldown`, autre `ldown` qui rend une instance neuve que `ui:bh` tient, est marqué `/* TROU-7 */` (`research_icon.h:91`).

Ce renvoi n'invente donc rien. (Que l'énumération « Deux conventions » de TROU-7 ne nomme pas les entités est une question séparée, hors périmètre : voir « Further Notes ».)

## Solution

Du point de vue du mainteneur : la liste des trous ne contient plus que de vrais écarts ; chaque `/* TROU-n */` du code renvoie à une entrée qui décrit ce qu'il marque. Aucune ligne exécutable ne change : le binaire et les empreintes sont identiques.

1. `docs/trous-de-portage.md` : les lignes 90-103 (titre `## TROU-6 …` jusqu'à la ligne vide après « Fichier : `src/pico8_ui.c`. ») sont supprimées. Rien d'autre dans le fichier.
2. `src/pico8_ui.c` : la ligne 359 (`            /* TROU-6 */`) est supprimée. Rien d'autre.
3. `src/pico8_planet.h:153` : `/* TROU-6 */` → `/* TROU-7 */`. Le commentaire de contrat qui suit (l. 154-155) dit « the caller (`ui:bh`) keeps whatever comes back », ce qui est le reste de la lecture TROU-6 et est faux (le `true` n'est pas tenu) ; il est réécrit, contrat C seulement, par exemple : `/* Returns a freshly built jc instance in held, Lua true as stop alone, or nil; ui:bh holds only the instance, and nothing frees it. */` (mise en forme libre, sens imposé : trois issues, seule l'instance est tenue, rien ne la libère).
4. `src/pico8_planet_family_trade.h:82` : `/* TROU-6 */` → `/* TROU-7 */`. Le commentaire l. 83-84 est juste ; il reste.

Un seul commit, message en français, préfixe `docs(D.21a):` ou `refactor(D.21a):` au choix de l'implémenteur.

## User Stories

1. En tant que mainteneur, je veux que `docs/trous-de-portage.md` ne déclare que des écarts réels, afin de ne pas chercher un bug qui n'existe pas.
2. En tant que mainteneur, je veux que chaque `/* TROU-n */` renvoie à une entrée qui décrit ce qu'il marque, afin qu'un marqueur soit une information et non un bruit.
3. En tant que mainteneur, je veux que la fuite du `jc` rendu par `jo:ldown`/`nr:ldown` soit marquée comme les autres fuites d'entités (TROU-7), afin que la phase qui éliminera `malloc` (`docs/plan-2.md`) la retrouve par `grep`.
4. En tant que relecteur, je veux la preuve que le cas « `ldown` rend un booléen » est porté, afin d'accepter la fermeture sans relire le cart.
5. En tant que relecteur, je veux un diff qui ne touche que des commentaires et un fichier de docs, afin de ne rien avoir à tester au-delà du filet.
6. En tant que relecteur, je veux que la sortie du préprocesseur soit identique, afin d'avoir la preuve mécanique que le diff n'est que du commentaire.
7. En tant que pilote du plan, je veux que les greps de Sortie excluent nommément l'historique qui garde le mot TROU-6, afin qu'un « vide » soit vérifiable.
8. En tant que lecteur de `pico8_planet.h`, je veux que le contrat de `pico8_jo_ldown` dise que seule l'instance est tenue, afin de ne pas croire que `ui:bh` tient un booléen.
9. En tant que mainteneur, je veux que les numéros des autres TROU ne bougent pas, afin que les ≈ 150 autres marqueurs restent justes.

## Implementation Decisions

### Modules

Exactement quatre fichiers :

- `docs/trous-de-portage.md` — entrée TROU-6 supprimée ;
- `src/pico8_ui.c` — une ligne de commentaire supprimée ;
- `src/pico8_planet.h` — marqueur renvoyé, contrat de `pico8_jo_ldown` réécrit ;
- `src/pico8_planet_family_trade.h` — marqueur renvoyé.

Hors de ces fichiers : rien. Pas de `tests/`, pas de `Makefile`, pas de `tools/`, pas de `docs/plan-3.md` (la note « Découvert à D.21a » est le travail du pilote, dans son propre commit), pas de `docs/glossaire-noms.md` (il ne cite pas TROU-6).

### Aucune modification de code hors commentaires

Chaque ligne `-`/`+` du diff de `src/` est une ligne de commentaire (`/* … */` ou sa continuation ` * …`). Les deux `assert` de `pico8_ui_bh` (`ui_is_held_class`, `== pico8_bg_struct_class()`) et leurs messages restent mot pour mot. `ui_is_held_class` n'est pas touchée.

### Commentaires (règle de `CLAUDE.md`)

Un commentaire ne reste que s'il porte l'une des quatre choses : sens d'un nom minifié, écart connu au cart (renvoi `/* TROU-n */` nu), piège qu'on réintroduirait, contrat C que le nom ne dit pas. Ici :

- `/* TROU-7 */` nu (pas de texte ajouté à côté) : écart connu ;
- le contrat de `pico8_jo_ldown` : contrat C (ce que rend la fonction, qui tient quoi, qui libère) ;
- aucun commentaire ne cite D.21, le plan, la revue, TROU-6 ou l'historique : ça va au message de commit.

### Ce qui ne change pas

Les numéros TROU-1…25 hors 6 ; le texte de TROU-7 ; toute constante `*_HASH`, `FROZEN_*` ; tout test ; tout e2e.

## Testing Decisions

Un bon test vérifie un comportement externe. Il n'y a ici aucun comportement à changer : la seule preuve utile est que **rien d'exécutable n'a bougé**. `gcov` et mutations sont sans objet (aucune ligne exécutable touchée), comme le dit la ligne du plan.

### Préprocesseur identique

Dans le scratchpad, au parent puis à l'arrivée, pour **chaque** `src/*.c` et `tests/*.c` (les deux en-têtes sont inclus par plusieurs unités) :

```sh
for f in src/*.c tests/*.c; do
  cc -std=c99 -E -P "$f" | grep -v '^[[:space:]]*$' > "$OUT/$(echo "$f" | tr / _).i"
done
```

puis `diff -r départ arrivée` → **vide**. Un seul fichier différent : arrêt, rapporté. Le nombre de fichiers comparés est au message.

### Diff limité aux commentaires

`git diff -U0 HEAD^ HEAD -- src | grep -E '^[-+][^-+]' | grep -vE '^[-+][[:space:]]*(/\*|\*)'` → **vide**.

### Filet

- `make test` vert ;
- `make e2e` : **26 conformes** ;
- `git diff HEAD^ HEAD -- tests Makefile tools '*.sha256'` → vide.

### Greps de Sortie

Chaque « vide » exclut nommément ce qu'on garde.

1. `grep -rn 'TROU-6' src/ tests/ tools/ Makefile docs/ --exclude-dir=done --exclude=plan-3.md --exclude=plan3-phaseD-etape21a-trou-6.md` → **vide**. Gardés, par nom : `docs/prd/done/` (9 PRD d'étapes passées : 1a.12, 1a.13, 1a.18, 1a.24, 1a.25, 1a.27, 1b.37, D.10a, D.20b — historique), `docs/plan-3.md` (lignes de l'historique et la ligne D.21a elle-même), ce PRD.
2. `grep -c '^## TROU-' docs/trous-de-portage.md` : 25 → **24** ; `grep -n '^## TROU-6' docs/trous-de-portage.md` → vide ; `grep -n '^## TROU-5\|^## TROU-7' docs/trous-de-portage.md` → deux lignes, séparées par l'entrée TROU-5 seule.
3. `grep -n 'TROU-' src/pico8_ui.c` → exactement les deux `/* TROU-7 */` (l. 213 et 471, ex-472).
4. `grep -n 'TROU-' src/pico8_planet.h` → `:24` (TROU-7), `:67` (TROU-8), `:131` (TROU-7), `:134` (TROU-3), `:153` (TROU-7 ; le contrat réécrit est sous le marqueur, qui ne bouge donc pas).
5. `grep -n 'TROU-' src/pico8_planet_family_trade.h` → `:9` (TROU-8), `:21`, `:32`, `:43`, `:82` (TROU-7).
6. `grep -rn 'TROU-7' src/ tests/ | wc -l` : 100 → **102** ; `grep -rn 'TROU-[0-9]' src/ tests/ | wc -l` : 157 → **156**.
7. `grep -c 'ui_is_held_class(kn.held->cls)\|kn.held->cls == pico8_bg_struct_class()' src/pico8_ui.c` : 2 → **2**.
8. `git show --stat HEAD` : les quatre fichiers de « Modules », rien d'autre.

### Budget et tableau « Runs »

**≈ 30 min** jusqu'au commit, preuves comprises ; plafond 40 min.

| Cible | Durée attendue |
|---|---|
| `make test` | de l'ordre de la minute |
| `make e2e` | ~1 s |
| préprocesseur, départ + arrivée | < 1 min |
| greps | secondes |

L'implémenteur tient un tableau « Runs » (une ligne par lancement : #, commande exacte, état de l'arbre, code de sortie, ligne clé, durée) et le met au message de commit.

### Critères d'échec (`FAILED:` pour le vérificateur de `prd-autopilot`)

- `FAILED:` si l'arbre de départ n'est pas `f4960fc` ou un descendant qui ne touche pas les quatre fichiers, propre.
- `FAILED:` si un fichier hors des quatre de « Modules » change.
- `FAILED:` si une ligne non commentaire de `src/` change, ou si la sortie du préprocesseur d'un seul `.c` de `src/` ou `tests/` diffère.
- `FAILED:` si `make test` n'est pas vert ou si `make e2e` ne rend pas 26 conformes (3 tentatives au plus).
- `FAILED:` si une constante `*_HASH` ou `FROZEN_*` change, si un `*.sha256` ou un fichier de `tests/e2e/` est touché.
- `FAILED:` si une grep de Sortie ne rend pas ce qui y est écrit, ou si une exclusion y est ajoutée sans être rapportée.
- `FAILED:` si une autre entrée de `docs/trous-de-portage.md` est éditée ou renumérotée, ou si TROU-7 est réécrite.
- `FAILED:` si un commentaire ajouté ou reformulé sort des quatre sortes de `CLAUDE.md`, ou cite D.21, le plan, la revue ou TROU-6.
- `FAILED:` s'il manque au message : la preuve de fermeture (le filtre `type(pw.kn)=="table"` du cart et le tableau des `ldown`/`rdown`), la justification du renvoi à TROU-7 (`Makefile:862`, `research_icon.h:91`), le rapport du préprocesseur, les comptes de Sortie avant/après, le tableau « Runs ».

## Out of Scope

- Toute modification de code exécutable, dont `ui_is_held_class`, les `assert` de `pico8_ui_bh` et les `ldown`/`rdown`.
- Libérer le `jc` lâché par `ui:bh` (c'est TROU-7, `docs/plan-2.md`).
- Réécrire TROU-7 pour nommer les entités parmi ses conventions (voir « Further Notes »).
- Ajouter `/* TROU-7 */` à d'autres constructeurs qui disent « nothing ever frees it » sans marqueur (`planet.h:92`, `planet_family_trade.h:56/66/75`, etc.) : l'usage du dépôt est mêlé, l'harmoniser est une passe à part.
- Les autres lignes de D.21 (b1, b2, c, d/e/f), le mot « REF » restant (phase 3), la note du plan.
- `make raylib` exécuté, l'appli raylib, l'audio, la PSX.

## Further Notes

- **Question ouverte (pour l'humain, non bloquante).** La fiche des fuites mémoire assumées annonce « deux conventions » qui fuient : les constructeurs de rangées et le remplacement d'un champ. Or le dépôt y rattache aussi, partout, les objets du jeu (planètes, routes, sondes…) qu'on crée et qu'on ne libère jamais — le `Makefile` le dit en toutes lettres pour couper la détection de fuites. Faut-il ajouter une ligne à cette fiche pour dire que les objets du jeu ne sont jamais libérés, ou la phrase d'ouverture (« pas de ramasse-miettes là où Lua en a un ») suffit-elle ? D.21a ne tranche pas et ne modifie pas la fiche ; le renvoi qu'elle fait est déjà celui du reste du dépôt.
- **Pourquoi l'entrée a tenu si longtemps.** 1b.37 et D.10a l'ont gardée volontairement (« le côté Lua de l'écart demeure »), en lisant « n'importe quelle table » comme un écart. Relu contre le cart, ce n'en est pas un : le cart lui-même filtre par type, et l'ensemble des classes qui arrivent là est fini et connu. À mettre au message de commit, pas dans le code.
- Le mot « TROU-6 » restera dans l'historique (`docs/prd/done/`, `docs/plan-3.md`) : c'est voulu, la convention du fichier renvoie à git.

## Annexe — reproduire (lecture seule)

```sh
sed -n 237,253p pico8/lua.lua                     # ui:bh et ui:jz du cart
grep -n 'function [a-z_]*:[lr]down' pico8/lua.lua # les sept méthodes
sed -n 330,368p src/pico8_ui.c                    # ui_is_held_class, bh
sed -n 51,54p src/pico8_struct.h                  # pico8_down_t
grep -rn '\.ldown = \|\.rdown = ' src/*.c         # tables de méthodes
grep -rn 'TROU-6' src tests tools Makefile docs
grep -n 'TROU-7' Makefile src/pico8_research_icon.h src/pico8_planet.h src/pico8_planet_family_trade.h
```
