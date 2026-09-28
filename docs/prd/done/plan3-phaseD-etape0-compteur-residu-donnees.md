---
title: "Plan 3, étape D.0 : compteur du résidu de données (`make data-residue`, `tools/data_residue.sh`) par catégorie sur `src/` et `tests/`, et garde `make data-guard` dans `make test` (`make value-members` et `grep -rn offsetof src/` en mode vérification, catégories tombées à 0) — mesure seule, aucune conversion"
description: "La phase D doit supprimer `pico8_ob.c`, `pico8_value.c`, `pico8_type_tag.h`, `pico8_ref_audit.h` et `pico8_stash.c`. Mais aucune commande ne dit aujourd'hui ce qui reste à convertir : les lecteurs et écrivains par nom, le dispatch à arguments `pico8_value_t`, les `overrides` des constructeurs et les spécifications de classe n'ont pas de compteur. Sans ce compteur, on ne peut ni écrire la liste ordonnée de D, ni constater sa sortie. De plus, les deux gardes de la sortie de 1b (`make value-members` vide, aucun `offsetof` dans `src/`) sont hors de `make test` : une régression passerait sans bruit. D.0 livre un compteur nommé et une garde, sans rien convertir."
status: needs-triage
---

## Problem Statement

Puce de l'étape, recopiée de `docs/plan-3.md` (§5, « Phase D — Données », ajoutée à HEAD ecac570 par la revue de sortie de 1b) :

> - **D.0 — mesure seule, sans conversion, sur le modèle de 1b.0** (ajouté à la revue de sortie de 1b) : un compteur nommé (`make data-residue`, `tools/data_residue.sh`) compte, par catégorie, dans `src/` et `tests/`, les usages de `pico8_value_t`/`pico8_value_*()`, `pico8_ob_table_t`/`pico8_ob*()`, `pico8_list_t` si le §6 le retire, `pico8_type_tag_of` et `pico8_ref_audit.h` — dont les lecteurs/écrivains par nom (`pico8_entity_get_field`/`set_field`, `*_truthy`, `field`/`set_field` des descripteurs et leurs aides `*_take_*`), le dispatch à arguments `pico8_value_t`, les `overrides` des constructeurs, les spécifications de classe lues par `pico8_ob()` et leurs `pico8_X_default`. Sa sortie vide **est** la sortie de la phase D, et chaque ligne de la liste porte son « Compte : N ». D.0 met aussi `make value-members` et `grep -rn offsetof src/` dans `make test`, en mode vérification (code non nul si non vide), à côté de `float-guard` ; une catégorie du compteur tombée à 0 y entre de même. La liste ordonnée de D est écrite après D.0, à partir de sa sortie : une catégorie par groupe nommé, découpée au-delà de ~40 fichiers ; chaque test de régression du §6 qui lit par nom est rattaché au groupe qui force sa réécriture, avec sa preuve rouge. Une catégorie qu'aucune puce de D ne couvre est l'arrêt « champ sans règle » du §7.

Puces de la phase D que D.0 sert, et sa **Sortie** :

> - Les tables constantes parsées au démarrage (`g_es`, `g_ct`, `g_b_`, `g_co`, `g_fb`, `g_ek`, `ci`, menus `g_ix`/`g_ha`, aide `g_bo`, score `g_ef`/`g_bz`, `g_df`) et celles décodées par `unstash` (`g_hc`, `g_fd`, `g_dr`, `g_nb`, `g_r`, `g_iz`) deviennent des tableaux/structs `static const`, **générés une fois** à partir de ce que `ob()`/`unstash` produisent aujourd'hui, puis versionnés.
> - Test ponctuel, supprimé dans le commit qui supprime `pico8_ob.c`/`pico8_stash.c` : chaque table générée est égale, entrée par entrée, à ce que `ob()`/`unstash` produit. Le filet ne voit pas les entrées jamais affichées (aide, score, menus).
> - Les tables ad hoc construites en cours de partie (~68 `ob_table_new()`) deviennent des structs locales ou des tableaux fixes.
> - **Sortie** : `pico8_ob.c`, `pico8_value.c`, `pico8_type_tag.h`, `pico8_ref_audit.h`, `pico8_stash.c` supprimés ; plus aucun `malloc` pour des données. Les entités gardent le leur : c'est Plan 2 §8.

L'étape suivante, pour la frontière :

> - **D.1 — suppression du recensement**, premier groupe après D.0 : `pico8_member_census.c/.h`, les crochets `SEE`/`READ`/`WRITE`/`CHILD_EXIT`/`STOP` de `src/` et `tests/`, `tools/member_census.*`, la cible `member-census` et ses entrées de listes de sources, le mode `--c-table` de `tools/entity_value_members.sh`. […]

Fin de la note de sortie de 1b :

> […] `make value-members` reste la garde de cette sortie, dans `make test` à partir de D.0.

Règle du §7, ajoutée par la même revue :

> - **En D et en 2** (ajouté à la revue de sortie de 1b) : `make class-coverage-net` aux étapes dont le diff touche un constructeur, un appel `pico8_class_coverage_record` ou `KNOWN[]`, et à la sortie de phase ; `make sanitize-net` aux étapes qui changent la durée de vie ou le propriétaire d'une donnée (table ob → locale ou statique, `malloc`/`free` retiré), et à la sortie de phase.

D.0 ne touche ni constructeur, ni `pico8_class_coverage_record`, ni `KNOWN[]`, et ne change la durée de vie d'aucune donnée : **ni `make class-coverage-net` ni `make sanitize-net`** ne sont exigés par la règle. Voir aussi §3 (`make test` vert, aucun `*.sha256` modifié) et les arrêts du §7.

### État de départ

HEAD ecac570, arbre propre. La phase 1b est sortie à 1b.37 (831b2cd). La note de sortie constate :

- `make value-members` rend la seule ligne `membres pico8_value_t : 0` ;
- `grep -rn offsetof src/` est vide (revérifié par grep pour ce PRD) ;
- `make class-coverage-net` est identique à `tools/class_coverage_net_expected.txt` ;
- `make sanitize-net` donne 15 propres, `make frames-check` 15 conformes.

**Rien n'a été compilé ni lancé pour ce PRD.** Les comptes viennent de `grep -rHnE --include='*.[ch]'` sur ecac570. Tout fait d'exécution est une hypothèse (H1 à H14).

### Inventaire

- **`make test`** (`Makefile:695`) : `test: unit constants-check float-grep float-guard`. La légende de tête (`Makefile:3-4`) dit « unit + constants-check + float-grep + float-guard ».
- **`make value-members`** (`Makefile:857-860`) lance `tools/entity_value_members.sh`. Le script :
  - a deux modes, liste (défaut) et `--c-table` ; ce dernier est supprimé par D.1 ;
  - fait `cd` à la racine et lit `src/*.c src/*.h` ;
  - **sort toujours 0** : c'est un compteur, pas une garde.
- **Prior art des outils testés** : `tests/test_float_grep.c`, `test_float_guard.c`, `test_class_coverage.c`, `test_constants_check.c`. Ce sont des TU C, dans `UNIT_TESTS`. Ils fabriquent un mini-arbre sous `tmp/…_work` et lancent le script par `system()`. Le code de sortie est relu depuis un fichier (`echo $? > code`), parce que le statut de `system()` n'est pas décodable en C99. Règle de construction : `$(TMP_DIR)/test_float_grep: $(call objs,tests/test_float_grep.c) tools/float_grep.sh` (`Makefile:357`).
- **`tools/float_grep.sh`** : précédent de la forme du script. `grep -E '\b…\b'`, `LC_ALL=C sort`, code 2 sur une mesure « sur rien », une ligne de pied comptée.
- **Arbres lus** : `src/` n'a pas de sous-dossier. `tests/` n'a que `tests/fixtures/`, sans `.c` ni `.h`. En tout, 240 fichiers `.c`/`.h`.
- **Ce que les cinq fichiers de la Sortie contiennent chacun** : au moins une ligne d'une catégorie ci-dessous.
  - `pico8_value.c:1` inclut `pico8_value.h` ;
  - `pico8_ob.c:1` inclut `pico8_ob.h` ;
  - `pico8_stash.c:1` inclut `pico8_stash.h` ;
  - `pico8_type_tag.h:2` définit `PICO8_TYPE_TAG_H` ;
  - `pico8_ref_audit.h` définit `PICO8_REF_AUDIT_H`.

  Un compteur à zéro implique donc qu'ils sont supprimés ou vidés.
- **`pico8_list_t`** (`pico8_list.h`) est un tableau de `pico8_value_t` (`pico8_value_t *items`). Ses signatures prennent et rendent des `pico8_value_t`. La catégorie `valeur` compte donc 13 lignes dans `pico8_list.c/.h` et 27 dans `tests/test_pico8_list.c` (voir la décision sur `pico8_list_t`).
- **`_default` hors spécification** : six fonctions portent le suffixe `_default` sans être un défaut de spécification de classe. Ce sont des variantes à argument omis :
  - `pico8_spr_default` (`gfx.h:71`) ;
  - `pico8_fixed_rnd_default` (`fixed.h:94`) ;
  - `pico8_dv_default` et `pico8_io_default` (`hud.h:26,45`) ;
  - `pico8_lq_ol_default` (`layout.h:79`) ;
  - `pico8_bl_default` (`pi.h:35`).
- **L'en-tête d'entité** `pico8_struct_header_t` (`pico8_struct.h`) porte un membre `pico8_type_tag_t tag`. Les constructeurs posent `PICO8_TYPE_TAG_STRUCT`. `pico8_type_tag_of` n'est donc qu'une partie de ce que la suppression de `pico8_type_tag.h` exige.

## Solution

Trois livrables, sans aucune ligne de `src/` changée :

1. **Le compteur** `tools/data_residue.sh`, et la cible `make data-residue` (hors suite).
   - Il compte, dans `src/` et `tests/`, les lignes de chacune des neuf catégories figées plus bas. Pour chacune, il donne le nombre de lignes et le nombre de fichiers.
   - Il n'imprime une ligne que pour les catégories non nulles, puis un pied qui compte les lignes distinctes. **La sortie vide est la sortie de la phase D** : aucune ligne de catégorie, et le pied `résidu D : 0 ligne(s), 0 fichier(s)`.
   - Un mode `--detail` donne, par catégorie, les fichiers et leur compte. C'est l'entrée de la liste de D (empreinte, découpage au-delà de ~40 fichiers, tests du §6 rattachés).
   - Un mode `--check <catégories>` imprime les lignes des catégories nommées et sort 1 s'il en trouve.
2. **La garde** `make data-guard`, ajoutée aux prérequis de `make test` à côté de `float-guard`. Elle échoue si l'un de ces trois contrôles n'est pas vide :
   - `tools/entity_value_members.sh --check`, un nouveau mode qui sort 1 si la liste n'est pas vide ;
   - `grep -rn offsetof src/` ;
   - `tools/data_residue.sh --check $(DATA_RESIDUE_GUARD)` : les catégories tombées à 0. La liste est vide à D.0 (aucune catégorie n'est à 0) ; chaque étape de D y ajoute la sienne quand elle l'éteint.
3. **Le TU** `test_data_residue`, dans `UNIT_TESTS`, qui teste le compteur sur un mini-arbre fabriqué. S'y ajoutent trois occurrences plantées (P1 à P3), qui prouvent que la garde mord.

La note « Découvert à D.0 » donne les comptes initiaux dans le plan. **La liste ordonnée de D n'est pas écrite par cette étape** (voir Further Notes).

## User Stories

1. En tant qu'auteur de la liste de D, je veux, par catégorie, le nombre de lignes et de fichiers de `src/` et `tests/` qui restent, afin de nommer un groupe par catégorie et de le découper au-delà de ~40 fichiers.
2. En tant qu'auteur de la liste de D, je veux la liste des fichiers de chaque catégorie avec leur compte, afin de mesurer l'empreinte de chaque groupe et d'y rattacher les tests de régression du §6 qui lisent par nom.
3. En tant qu'auteur de chaque ligne de la liste, je veux un « Compte : N » calculable par une commande, afin que chaque étape déclare le compte attendu après elle.
4. En tant que `plan-pilot`, je veux que la sortie vide du compteur soit la sortie de la phase D, afin de constater la fin de phase par une commande et non par une relecture.
5. En tant que gardien de la sortie de 1b, je veux que `make test` échoue si un membre `pico8_value_t` réapparaît dans une struct d'entité, afin qu'une régression de 1b ne passe pas en silence pendant D.
6. En tant que gardien de la sortie de 1b, je veux que `make test` échoue si un `offsetof` réapparaît dans `src/`, afin qu'aucune table d'offsets par nom ne revienne.
7. En tant qu'auteur d'une étape de D qui éteint une catégorie, je veux ajouter un mot à une variable du `Makefile` pour que `make test` garde cette catégorie à 0, afin de ne pas réinventer une garde à chaque groupe.
8. En tant que mainteneur, je veux que chaque catégorie soit définie par une expression `grep -E` écrite dans le PRD et dans le script, afin que n'importe qui la recompte à la main.
9. En tant que mainteneur, je veux une sortie stable (ordre fixe des catégories, fichiers triés en `LC_ALL=C`), afin que deux passes sur le même arbre donnent le même texte et que son diff se lise.
10. En tant que vérificateur, je veux que le compteur sorte en erreur (code 2) sur une mesure sur rien (dossier absent, aucun `.c`/`.h`, catégorie inconnue), afin qu'un compteur cassé ne passe pas pour « sortie vide ».
11. En tant que vérificateur, je veux un TU qui prouve que chaque catégorie compte ce qu'elle doit, et seulement cela (exclusions, frontières de mot, chevauchements), afin que les « Compte : N » de la liste soient fiables.
12. En tant que vérificateur, je veux que le TU du compteur ne contribue lui-même à aucune catégorie, afin qu'il ne rende pas la sortie de D inatteignable.
13. En tant que vérificateur, je veux voir `make data-guard` rougir sur une occurrence plantée de chacun de ses trois contrôles, afin de savoir que la garde mord.
14. En tant que gardien de l'invariant §3, je veux qu'aucun fichier de `src/`, aucun `*.sha256` et aucune entrée de `FRAMES_EVENTS` ne change, afin que D.0 soit une mesure pure.
15. En tant que lecteur du `Makefile`, je veux `data-residue` (hors suite) et `data-guard` (dans `make test`) dans la légende de tête, et la ligne `make test` mise à jour.
16. En tant que lecteur du plan, je veux une note « Découvert à D.0 » qui donne la commande, les catégories et leurs comptes initiaux, et les lectures qui engagent la liste.
17. En tant que `plan-pilot`, je veux que la liste de D, la suppression du recensement (D.1) et toute conversion restent hors de ce commit, afin que chaque étape reste une étape.
18. En tant que `make test`, je veux que la garde coûte moins d'une seconde, afin que la boucle rapide reste rapide.

## Implementation Decisions

Pas d'humain dans la boucle. Le skill `to-prd` demande de faire valider par l'utilisateur les modules, les tests et le découpage. Ces choix sont tranchés ici comme le ferait un agent `guideline` (hypothèses explicites, simplicité, changement chirurgical, critères vérifiables), et marqués **(G)**. Les rejets sont notés à chaque décision.

### Unité de compte (G)

- **Une ligne de fichier `.c`/`.h`** qui correspond à l'expression de la catégorie, **commentaires et chaînes compris**. C'est la forme de `float-grep` (« l'expression telle quelle, commentaires compris »). Un commentaire qui nomme un type supprimé est périmé, donc il est du résidu.
  - Rejeté : retirer les commentaires. Le compteur y gagnerait un parseur, et on perdrait l'égalité avec un `grep` à la main.
- **Les catégories se chevauchent** : une ligne `pico8_entity_get_field(e, "x", &v)` compte dans `par-nom`, et sa déclaration `pico8_value_t v;` dans `valeur`. Chaque ligne de catégorie compte ses lignes. Le pied compte les **lignes distinctes** (couples fichier:ligne) et les fichiers distincts.
  - Rejeté : des catégories disjointes (« première qui correspond »). La garde `--check` d'une catégorie éteinte ne verrait plus une ligne rattachée à une catégorie antérieure.
- **Les catégories transversales garantissent la sortie ; les sous-catégories servent au progrès et au groupement.**
  - Les transversales sont `valeur`, `ob`, `stash`, `type-tag` et `ref-audit`. Quand elles sont à 0, plus aucune ligne ne nomme les types et fonctions des cinq modules. Aucun site ne peut donc encore passer une `pico8_value_t`, même sans la nommer (un appel `pico8_jo_truthy(v)`).
  - Les sous-catégories sont celles du « dont » de la puce.

### Les neuf catégories, figées (G)

Chaque catégorie est **une** expression `grep -E` (GNU et BSD acceptent `\b`, précédent `float_grep.sh`), appliquée ligne à ligne aux `*.c`/`*.h` des dossiers reçus. Recompte à la main :

```
grep -rHnE --include='*.[ch]' '<motif>' src tests
```

L'ordre ci-dessous est celui de la sortie.

| Catégorie | Ce qu'elle compte | Motif `grep -E` |
|---|---|---|
| `valeur` | `pico8_value_t`, `pico8_value_*()`, `pico8_value_function_t`, les genres `PICO8_VALUE_*`, l'en-tête | `\bpico8_value_[A-Za-z0-9_]+\|\bPICO8_VALUE_[A-Z0-9_]+\|pico8_value\.h` |
| `ob` | `pico8_ob_table_t`, `pico8_ob*()`, les constantes `PICO8_OB_*`, `pico8_fi`/`pico8_fi_fn` (parseur de `pico8_ob.c`), l'en-tête | `\bpico8_ob(_[A-Za-z0-9_]+)?\b\|\bPICO8_OB_[A-Z0-9_]+\|\bpico8_fi(_fn)?\b\|pico8_ob\.h` |
| `stash` | `unstash` et le module `pico8_stash` : `pico8_unstash`, `pico8_stash_*`, les accesseurs `pico8_r()`/`pico8_iz()`, l'en-tête | `\bpico8_unstash\b\|\bpico8_stash_[A-Za-z0-9_]+\|pico8_stash\.h\|\bpico8_(r\|iz)\(\)` |
| `type-tag` | `pico8_type_tag_of`, le type `pico8_type_tag_t` (dont le `tag` de `pico8_struct_header_t`), les constantes, l'en-tête | `\bpico8_type_tag(_of\|_t)?\b\|\bPICO8_TYPE_TAG_[A-Z0-9_]+\|pico8_type_tag\.h` |
| `ref-audit` | `pico8_ref_audit.h` et ses macros `PICO8_REF_AS_LIST(_CONST)`, `PICO8_REF_AUDIT*` | `pico8_ref_audit\.h\|\bPICO8_REF_[A-Z0-9_]+` |
| `par-nom` | lecteurs/écrivains par nom : `pico8_entity_get_field`/`set_field` ; `*field_truthy()` ; `field`/`set_field` des descripteurs (`pico8_X_field`, `pico8_X_set_field`, `pico8_struct_(set_)field_fn`, `->field(`, `.set_field`, `kind->field`) ; aides `*_take_*()` | `\bpico8_entity_(get\|set)_field\b\|\b[a-z0-9_]*field_truthy\(\|\bpico8_[a-z0-9_]+_(set_)?field(_fn)?\b\|(->\|\.)(set_)?field\b\|\b[a-z0-9_]+_take_[a-z0-9_]+\(` |
| `dispatch` | dispatch à arguments `pico8_value_t` : le type de méthode `pico8_value_function_t`, `pico8_value_function()`, `pico8_mv()` | `\bpico8_value_function(_t)?\b\|\bpico8_mv\(` |
| `overrides` | les clés de construction des constructeurs | `\boverrides\b` |
| `spec` | spécifications de classe lues par `pico8_ob()` (`g_X_spec`, `g_X_defaults`) ; leurs `pico8_X_default` et `pico8_X_defaults_reset` ; les aides de déballage `X_default_<genre>` | `\bg_[a-z0-9_]+_(spec\|defaults)\b\|\bpico8_[a-z0-9_]+_(default\|defaults_reset)\b\|\b[a-z0-9_]+_default_(fixed\|table\|vec\|string\|box\|list\|bool\|hf)\b`, **après** retrait des six `_default` hors spécification (ci-dessous) |

(Dans le tableau, `\|` note l'alternative `|` de l'expression.)

Décisions et lectures par catégorie :

- **`spec`, exclusion des six `_default` hors spécification (G).** Avant le test du motif, la ligne est passée dans `sed -E 's/pico8_(spr|fixed_rnd|dv|io|lq_ol|bl)_default//g'`. Ce `sed` n'utilise pas `\b`, que le `sed` BSD ne connaît pas. Ce sont des variantes à argument omis du cart, pas des défauts de classe : les compter rendrait `spec` impossible à éteindre.
  - Rejeté : une liste positive des classes (`pico8_(ke|jo|qh|…)_default`). Elle est plus longue, et elle manquerait un défaut ajouté.
  - C'est un piège qu'on réintroduirait : un commentaire d'au plus 40 mots dans le script, au-dessus de l'exclusion (règle CLAUDE.md).
- **`*_truthy` (G)** : la puce range `*_truthy` dans les lecteurs par nom. Ce sont les `field_truthy(inst, name)` de 1b.0 (`economy_formulas.c`, `economy_score.c`, `economy_tiers.c`, `planet_display.c`) et `pico8_probe_field_truthy`. Les autres `*_truthy(pico8_value_t v)` (`pico8_jo_truthy`, `economy_truthy`…) sont des fonctions **de valeur** : leur définition est comptée par `valeur`, et elles tombent avec elle. `par-nom` ne compte donc que `*field_truthy(`.
- **`dispatch` (G)** : seulement la forme à `pico8_value_t`. `methods[`/`PICO8_METHOD_*` ne sont **pas** comptés : la table de méthodes survit à D (la phase 2 la met dans le noyau `entity`). Un `methods[…](self, argc, a0, a1)` direct est garanti par `valeur`, puisque le type `pico8_value_function_t` ne peut plus exister à 0.
- **`type-tag` (G)** : plus large que « `pico8_type_tag_of` » de la puce. La Sortie supprime `pico8_type_tag.h`, qui définit aussi `pico8_type_tag_t` (le `tag` de chaque `pico8_struct_header_t`) et `PICO8_TYPE_TAG_STRUCT`, posé par chaque constructeur. Ne compter que `_of` laisserait à 0 une catégorie dont le fichier existe encore. Le sous-compte `pico8_type_tag_of` seul (319 lignes, H3) est donné dans le message de commit, pour information.
- **`stash` (G)** : la puce ne le nomme pas, mais la Sortie supprime `pico8_stash.c`. Or `pico8_unstash` rend un `char *` et `pico8_stash_init` prend un `pico8_mem_t *`, lignes qu'aucune autre catégorie ne voit. Sans `stash`, le compteur pourrait tomber à 0 avec `pico8_stash.c` présent.
- **`ob` inclut `pico8_fi`** : même raison, le parseur `fi()` vit dans `pico8_ob.c`.
- **`pico8_list_t` : pas de catégorie propre (G).** La puce le compte « si le §6 le retire ». Le §6 ne le retire pas : il supprime `test_pico8_list` « s'ils perdent leur objet », sans rien décider, et la Sortie ne supprime pas `pico8_list.c`. En revanche, ses éléments sont des `pico8_value_t`. Ces lignes (13 dans `pico8_list.c/.h`, 27 dans `test_pico8_list.c`) et chaque `pico8_list_add(…, pico8_value_…)` sont comptés par `valeur`. **Conséquence pour la liste de D** : la représentation de `pico8_list_t` doit changer en D (éléments typés ou module retiré), sinon `valeur` ne tombe jamais à 0. C'est à trancher à l'écriture de la liste, pas ici.
  - Rejeté : exclure `pico8_list.*` de `valeur`. La sortie vide ne vaudrait plus « `pico8_value_t` a disparu ».
- **`overrides` (G)** : le mot seul. Aujourd'hui, les 421 lignes sont toutes le paramètre `const pico8_ob_table_t *overrides` ou ses usages (vérifié par grep). Si une étape de D garde ce nom sur un paramètre typé, elle le renomme : c'est un nom du portage, pas un nom minifié du cart, et le §5 « Phase 3 » ne le protège pas.
- **Motifs figés** : une étape de D ne change un motif que dans son commit, avec, dans le message, la preuve (grep) que les lignes retirées du compte ne touchent plus aucune catégorie transversale. Sans cette preuve, c'est l'arrêt « champ sans règle » du §7.

### Sortie du compteur

- **Mode compteur** (défaut) : `sh tools/data_residue.sh [dossier…]`, dossiers par défaut `src tests`, chemins imprimés tels que reçus. Le script ne fait pas de `cd` : la cible `make` tourne à la racine, et le TU passe ses propres dossiers.
  - Une ligne par catégorie **non nulle**, dans l'ordre du tableau : `<catégorie>\t<lignes>\t<fichiers>` (tabulations).
  - Puis le pied, toujours : `résidu D : <N> ligne(s), <M> fichier(s)`, où N compte les couples fichier:ligne distincts toutes catégories confondues, et M les fichiers distincts.
  - Code 0, même si le compte n'est pas nul : c'est un compteur, comme `value-members`.
- **Mode `--detail`** : une ligne `<catégorie>\t<fichier>\t<lignes>` par couple non nul, triée dans l'ordre des catégories puis en `LC_ALL=C` sur le fichier. Puis le même pied. Code 0.
- **Mode `--check <cat>[,<cat>…]`** : une ligne `<catégorie>: <fichier>:<ligne>:<texte>` par occurrence des catégories nommées, puis `résidu gardé (<cats>) : <N> ligne(s)`. Code 1 si N > 0, sinon 0.
- **Code 2** dans tous les modes, message sur la sortie d'erreur, quand :
  - une option est inconnue ;
  - une catégorie de `--check` est inconnue, ou la liste est vide ;
  - un dossier reçu est absent ;
  - aucun `.c`/`.h` n'est trouvé dans les dossiers ;
  - `grep` rend plus de 1.

  Une mesure sur rien passerait pour « sortie vide » (précédent `float_grep.sh`).
- **« Sortie vide = sortie de D »**, défini une fois pour toutes : `make data-residue` n'imprime **que** la ligne `résidu D : 0 ligne(s), 0 fichier(s)`. C'est la même convention que `make value-members` à la sortie de 1b (`membres pico8_value_t : 0`). Les cinq fichiers de la Sortie contiennent chacun une ligne comptée (inventaire) : zéro implique leur suppression.
  - Rejeté : une catégorie « fichiers » qui teste leur présence, redondante (H8).
  - Pour la moitié « plus aucun `malloc` pour des données », voir la lecture plus bas.

Sortie attendue sur ecac570 (H3) :

```
valeur	6129	149
ob	2331	143
stash	56	14
type-tag	454	74
ref-audit	70	17
par-nom	1104	92
dispatch	180	57
overrides	421	57
spec	942	68
résidu D : 9896 ligne(s), 161 fichier(s)
```

Répartition `src` / `tests` (lignes/fichiers), donnée dans la note du plan :

| Catégorie | `src/` | `tests/` |
|---|---|---|
| `valeur` | 2392 / 90 | 3737 / 59 |
| `ob` | 1383 / 87 | 948 / 56 |
| `stash` | 44 / 13 | 12 / 1 |
| `type-tag` | 180 / 35 | 274 / 39 |
| `ref-audit` | 70 / 17 | 0 / 0 |
| `par-nom` | 504 / 43 | 600 / 49 |
| `dispatch` | 70 / 28 | 110 / 29 |
| `overrides` | 311 / 43 | 110 / 14 |
| `spec` | 722 / 43 | 220 / 25 |
| lignes distinctes | 102 fichiers | 59 fichiers |

### Lecture de « plus aucun `malloc` pour des données » (G)

- Sous cette lecture, les allocations « de données » de la phase D sont celles des structures de la couche :
  - les tables ob (`pico8_ob_table_new`, le parseur `ob()`, ~68 `ob_table_new()` ad hoc) ;
  - les tables décodées par `unstash` ;
  - les chaînes de `pico8_value_tostr`.

  Elles vivent dans `pico8_ob.c`, `pico8_stash.c` et `pico8_value.c`, ou passent par leurs fonctions : `ob`, `stash` et `valeur` à 0 les éteignent.
- Les 122 autres sites `malloc`/`calloc`/`realloc` de `src/` sont :
  - des entités (Plan 2 §8, exclus par la puce) ;
  - des chaînes (`pico8_str.c`, concaténations) ;
  - des `pico8_vec_t`, des `pico8_list_t` et des boîtes allouées pour être rangés dans une REF.

  Les classer demande un jugement site par site, pas un grep : ce n'est pas l'objet d'un compteur figé.
- Si l'écriture de la liste lit « données » plus largement (les cellules allouées pour une REF), elle ajoute une catégorie au compteur dans son propre commit, en éditant la puce D.0 du plan. D.0 ne le fait pas.

### Garde `make data-guard` (G)

- **Une cible, trois contrôles**, dans cet ordre. Chacun imprime ce qu'il trouve, et la cible sort non nulle au premier contrôle non vide :
  1. `sh tools/entity_value_members.sh --check`. C'est un nouveau mode, qui imprime exactement la sortie du mode liste et sort 1 si le compte n'est pas 0. Le mode liste par défaut et `--c-table` ne changent pas (D.1 retire ce dernier).
  2. `grep -rn offsetof src/`, la commande littérale de la puce. Sur une correspondance, la cible imprime les lignes et `data-guard : offsetof dans src/`, puis sort 1. Sinon elle imprime `offsetof dans src/ : 0`. Le `grep` rend 1 quand rien ne correspond, et ce n'est pas une erreur ici ; un code supérieur à 1 fait échouer la cible.
  3. `sh tools/data_residue.sh --check $(DATA_RESIDUE_GUARD) src tests`, seulement si `DATA_RESIDUE_GUARD` n'est pas vide. Sinon la cible imprime `catégories gardées : aucune`.
- **`DATA_RESIDUE_GUARD :=`** (vide) est une variable versionnée du `Makefile`, liste de catégories séparées par des virgules. Aucune catégorie n'est à 0 sur ecac570 (H3). Une étape de D qui éteint une catégorie l'y ajoute dans son commit : c'est « une catégorie du compteur tombée à 0 y entre de même ».
  - Rejeté : un fichier de liste sous `tools/`. Un mot dans une variable suffit, sans lecteur de fichier à tester.
  - Rejeté : reporter le mode `--check` à la première étape qui éteint une catégorie. La puce fixe la règle à D.0, le mode tient en quelques lignes, et le TU le prouve dès maintenant.
- **`test: unit constants-check float-grep float-guard data-guard`**. La cible n'écrit rien sur disque et ne compile rien : pas de course sous `-j` (H6).
- **`make data-residue`** : `@sh tools/data_residue.sh`, hors suite, ni dans `test` ni dans `e2e`.
- **Légende de tête du `Makefile`** :
  - ligne `make test` : « unit + constants-check + float-grep + float-guard + data-guard » ;
  - `make data-residue -> compte le résidu de données par catégorie (tools/data_residue.sh) ; la phase D sort sur une sortie vide, hors suite` ;
  - `make data-guard -> make value-members vide, aucun offsetof dans src/, catégories de DATA_RESIDUE_GUARD à 0, dans make test`.

  La ligne `make value-members` reste « hors suite » (le compteur) ; elle précise que `data-guard` en fait la garde.

### Ce qui ne change pas

- **Aucun fichier de `src/`.** Aucun test existant, aucun `*.sha256`, `FRAMES_EVENTS`, `RECORDINGS`, `KNOWN[]`, `tools/class_coverage_*`, `tools/member_census.*`, `CENSUS_UNIT_TESTS`, `UNIT_TESTS_SLOW`.
- Fichiers touchés, et seulement eux :
  - `tools/data_residue.sh` (nouveau) ;
  - `tools/entity_value_members.sh` (mode `--check`) ;
  - `tests/test_data_residue.c` (nouveau) ;
  - `Makefile` : légende, `UNIT_TESTS`, règle du TU, `DATA_RESIDUE_GUARD`, `data-residue`, `data-guard`, `test:`, `.PHONY` ;
  - `docs/plan-3.md` : note « Découvert à D.0 ».
- **Commentaires** (CLAUDE.md) :
  - en tête du script, le contrat : usage, modes, codes de sortie, définition de « vide » ;
  - le piège de l'exclusion `spec` ;
  - dans le `Makefile`, l'aide à la lecture des deux recettes ;
  - le reste va dans le message de commit.
- **TROU** : aucun écart port/cart n'est touché.

### Hypothèses (rien n'a été lancé)

- **H1** : sur ecac570, `sh tools/entity_value_members.sh` imprime la seule ligne `membres pico8_value_t : 0` (note de sortie de 1b). Le mode `--check` sort donc 0.
- **H2** : `grep -rn offsetof src/` est vide à l'implémentation, comme sur ecac570 (grep de ce PRD). La sortie de 1b le constate.
- **H3** : `make data-residue` imprime exactement la sortie attendue ci-dessus. Les motifs sont figés et `grep` est déterministe ; un écart vient d'un arbre changé ou d'un `grep` différent. L'implémenteur l'explique dans le commit. Sous-compte informatif `pico8_type_tag_of` : 319 lignes (105 dans `src/`, 214 dans `tests/`).
- **H4** : le TU `test_data_residue` ne contribue à aucune catégorie. `sh tools/data_residue.sh --detail | grep -c test_data_residue` vaut 0. Les comptes de `tests/` sont donc inchangés par le commit.
- **H5** : `grep -E` accepte `\b` et `--include` sur la machine (précédent `float_grep.sh`, qui tourne aussi sous Apple clang). Le `sed -E` de l'exclusion `spec` n'utilise pas `\b`.
- **H6** : `make data-guard` dure moins d'une seconde (awk sur `src/`, un `grep`) et n'écrit rien. `make test` ne s'allonge pas de façon mesurable.
- **H7** : le TU dure moins de 5 s (une dizaine de `system()` sur un mini-arbre). Il va dans `UNIT_TESTS`, pas dans `UNIT_TESTS_SLOW`.
- **H8** : chacun des cinq fichiers de la Sortie contient au moins une ligne comptée (inventaire). Aucune catégorie « fichiers » n'est nécessaire.
- **H9** : aucun appelant ne dépend du code de sortie de `entity_value_members.sh` sans option : il reste 0. `member-census` n'appelle que `--c-table`, qui ne change pas.
- **H10** : les 421 lignes `overrides` sont toutes des clés de construction (paramètre `pico8_ob_table_t *overrides` ou ses usages). Un grep `\boverrides\b | grep -v pico8_ob` montre des usages comme `pico8_ob_table_set_name(overrides, …)` et `overrides->count`. L'implémenteur en relit un échantillon et note tout faux positif.
- **H11** : les motifs `par-nom` n'attrapent pas de faux positif hors couche : 71 aides `*_take_*` distinctes, toutes R5 ; 61 noms `pico8_*_field` distincts ; aucun `.field` de prose. L'implémenteur relit les identifiants distincts (`grep -ohE … | sort -u`) et note un faux positif s'il y en a.
- **H12** : `make test` reste vert : seules des cibles et un TU sont ajoutés.
- **H13** : aucun fichier de `src/` ne change, donc `make frames-check` rend 15 conformes sans nouvelle référence.
- **H14** : l'arbre de départ est ecac570 (ou un commit de docs au-dessus), sans changement non commité. La ligne « Runs » n° 1 le constate (`git status --short` vide).

## Testing Decisions

Un bon test juge le compteur par ce qu'il imprime et par son code de sortie, sur un arbre dont on connaît la réponse. Il ne juge pas la disposition interne du script. La garde se juge par ce qui la fait rougir.

### Le TU `test_data_residue` (G)

- **Forme** : C99, sur le modèle exact de `tests/test_float_grep.c` :
  - dossier de travail `tmp/test_data_residue_work`, recréé par `setup()` ;
  - fichiers écrits par `write_file` ;
  - script lancé par `system()`, code relu depuis un fichier ;
  - sortie lue dans un tampon statique ;
  - assertions par `strstr` ou par égalité de texte.

  Il est dans `UNIT_TESTS`, avec la règle `$(TMP_DIR)/test_data_residue: $(call objs,tests/test_data_residue.c) tools/data_residue.sh`.
- **Le TU ne doit rien compter de lui-même** (H4). Le script lit `tests/`, donc toute ligne du TU qui correspond à un motif entre dans le résidu et rendrait la sortie de D inatteignable.
  - Les jetons des fixtures sont écrits **coupés** par concaténation de littéraux C : `"pico8_" "value_t"`, `"over" "rides"`, `"field_" "truthy("`, `"g_jo_" "spec"`, `"PICO8_" "REF_AS_LIST"`. Le compilateur recolle ce que `grep` ne voit pas.
  - Aucun nom de fonction du TU ne correspond à un motif (par exemple, pas de `…_default_fixed`).
  - Rejeté : des fixtures `.c` sous `tests/fixtures/`, que le compteur lirait. Des fixtures `.txt` renommées à la copie ajoutent un fichier par cas pour un gain de lisibilité mince.
- **Cas** (chacun sur un mini-arbre `src/` + `tests/` sous le dossier de travail) :
  1. **Arbre propre** : un `.c` sans aucun jeton. Code 0, sortie = la seule ligne `résidu D : 0 ligne(s), 0 fichier(s)`. C'est la définition de « vide ».
  2. **Une occurrence par catégorie** : un fichier par catégorie, avec une ligne qui ne correspond qu'à elle. Chaque catégorie imprime `1\t1`, dans l'ordre du tableau.
  3. **Chevauchement** : une ligne `pico8_entity_get_field(e, "x", &v)` suivie de `pico8_value_t v;`. `par-nom` compte 1, `valeur` 1, et le pied compte 2 lignes, 1 fichier. Puis une seule ligne qui porte les deux jetons : les deux catégories comptent 1, le pied 1 ligne.
  4. **Exclusion `spec`** : `pico8_spr_default(gfx, mem, 1, 0, 0);` ne compte dans aucune catégorie ; `pico8_jo_default("nu", &v);` compte dans `spec` (et `valeur` si la ligne a le type). `jo_default_fixed(` compte ; `pico8_io_default(` non.
  5. **Frontières de mot** : `mypico8_value_t`, `overridesx`, `fields` ne comptent pas ; `self->field(` et `.set_field =` comptent dans `par-nom` ; `pico8_jo_truthy(v)` ne compte pas dans `par-nom` ; `field_truthy(e, "x")` compte.
  6. **Tri et ordre** : deux fichiers `b.c` et `a.c` dans une catégorie. `--detail` les rend dans l'ordre `a.c`, `b.c` ; deux passes donnent un texte identique.
  7. **`--check`** :
     - une catégorie nommée présente : code 1 et la ligne `<cat>: <fichier>:<n>:<texte>` ;
     - une catégorie absente de l'arbre : code 0 ;
     - deux catégories séparées par une virgule, dont une présente : code 1 ;
     - une catégorie inconnue : code 2.
  8. **Mesure sur rien** : dossier absent, ou dossier sans `.c`/`.h` : code 2 ; option inconnue : code 2.
- **Pas de TU pour `entity_value_members.sh --check` ni pour le `grep offsetof`.** Le premier script fait `cd` à la racine et lit `src/` en dur ; le second est une commande de la puce, recopiée. Leur preuve est P1 et P2 ci-dessous.
  - Rejeté : ajouter un paramètre de dossier à `entity_value_members.sh` pour le tester. C'est un changement de contrat pour un script que D.1 retouche, sans gain sur ce que P1 prouve.

### Occurrences plantées (preuve que la garde mord)

Faites dans l'arbre de travail, fichier sauvegardé dans le scratchpad puis restauré, `cmp` contre la sauvegarde et `git status --short` vide ensuite. `make data-guard` ne compile rien, donc aucune copie jetable du dépôt n'est nécessaire.

- **P1, `offsetof`** : une ligne `/* offsetof */` ajoutée à un `.c` de `src/`. `make data-guard` sort non nul et imprime la ligne. Puis `make test` sort non nul, ce qui prouve le branchement dans `test` ; au moins `data-guard` est en échec dans la sortie. Restauré.
- **P2, membre `pico8_value_t`** : une ligne `pico8_value_t zz;` ajoutée dans un typedef d'entité (par exemple `pico8_bg_t`, dans `pico8_bg.h`). `make data-guard` sort non nul avec `…: pico8_bg_t zz` et `membres pico8_value_t : 1`. Restauré. Le `.h` touché fait recompiler au `make test` suivant : c'est attendu.
- **P3, catégorie gardée** : `make data-guard DATA_RESIDUE_GUARD=ref-audit` sur l'arbre propre. La cible sort non nulle et imprime des lignes `ref-audit: src/pico8_ref_audit.h:…` et `résidu gardé (ref-audit) : 70 ligne(s)`. Puis `make data-guard` sans surcharge sort 0. Cela prouve la plomberie de la variable.

Pas de `gcov` ni de mutations au sens du §7 (« Preuve des tests » porte sur les lignes d'une conversion ; D.0 n'en a pas). Précédent : 1b.0.

### Vérification, budget et tableau « Runs »

**Budget : environ 20 minutes de temps total de tests** pour le passage. Ordres de grandeur (1b.36, 1b.37) :

| Cible | Durée |
|---|---|
| `make test` | de 20 s à 1 min |
| `make e2e` | 4 à 6 min |
| `make data-guard`, `make data-residue`, `make value-members` | quelques secondes |
| `test_data_residue` seul | < 5 s (H7) |

Répartition indicative :

- `make data-residue` et `make value-members` sur l'arbre de départ ;
- TU rouge puis vert, par itérations sur le seul TU (`make tmp/test_data_residue && ./tmp/test_data_residue`) ;
- `make data-guard`, puis P1 à P3 (secondes, sauf le `make test` de P1) ;
- `make test` en fin ;
- `make e2e` une fois en fin (invariant §3, même sans ligne de `src/` changée).

Pas de `make class-coverage-net` ni de `make sanitize-net` (règle « En D et en 2 », non déclenchée). Au-delà du budget, l'implémenteur s'arrête et note ce qui manque, plutôt que de relancer.

**L'implémenteur tient un tableau « Runs »** dans son journal de progression, une ligne par lancement :

| # | Commande exacte | État de l'arbre (HEAD + « script écrit » / « P1 planté »…) | Code de sortie | Ligne clé | Durée |
|---|---|---|---|---|---|

Exemples de ligne clé :

- `résidu D : 9896 ligne(s), 161 fichier(s)` ;
- `membres pico8_value_t : 0` ;
- `data-guard : offsetof dans src/` ;
- `résidu gardé (ref-audit) : 70 ligne(s)` ;
- `frames-check : 15 conformes`.

Le vérificateur réutilise ce tableau et ne relance que ce qui suit le dernier changement de l'arbre.

**Cibles finales**, toutes vertes sur l'arbre final :

1. `timeout 10m make test`, avec `data-guard` et `test_data_residue` dans sa sortie ;
2. `timeout 15m make e2e` : `frames-check` rend **15 « conforme »**, et les `UNIT_TESTS_SLOW` passent ;
3. `make data-residue` : sortie de H3, ou écart expliqué ;
4. `make data-guard` : `membres pico8_value_t : 0`, `offsetof dans src/ : 0`, `catégories gardées : aucune`, code 0.

Jamais l'appli raylib. `make raylib` n'est pas concerné.

### Contrôles du vérificateur

1. **Périmètre du diff** : `git show --stat` ne liste que les cinq fichiers de « Ce qui ne change pas ». Aucun fichier de `src/`, aucun test existant, aucun `*.sha256`, aucun fichier d'événements.
2. **Motifs** : les neuf expressions du script sont celles du tableau, au caractère près. Il en va de même de l'exclusion `spec` et de l'ordre des catégories.
3. **Recompte indépendant** : pour `valeur`, `par-nom` et `spec` (avec son `sed`), la commande de recompte à la main rend les mêmes lignes et fichiers que `sh tools/data_residue.sh --detail`.
4. **Auto-exclusion** : `sh tools/data_residue.sh --detail | grep -c test_data_residue` vaut 0 (H4).
5. **« Vide »** : le cas 1 du TU asserte la ligne de pied seule, au texte près.
6. **Garde** : `make data-guard` vert sur l'arbre final. P3 est rejoué (sans modification de fichier). P1 et P2 sont relus dans « Runs », et rejoués seulement si leur ligne manque.
7. **Branchement** : la ligne `test:` du `Makefile` contient `data-guard` ; `data-residue` n'est ni dans `test` ni dans `e2e` ; la légende est à jour.
8. **Plan** : la note « Découvert à D.0 » existe sur la puce D.0. Le diff du plan n'ajoute ni liste d'étapes de D, ni ligne D.2+.
9. **Commentaires** : ceux qui sont ajoutés relèvent des quatre sortes de `CLAUDE.md`.

### Critères d'échec (traduits en `FAILED:` par le vérificateur de `prd-autopilot`)

**Départ et filet (§7)**

- `FAILED:` si l'arbre de départ n'est pas ecac570 (ou un commit de docs au-dessus) propre, ou si `make value-members` n'y rend pas `membres pico8_value_t : 0`.
- `FAILED:` si le filet (`make test` ou `make e2e`) reste rouge après **3 tentatives**.
- `FAILED:` si un `*.sha256` est modifié ou supprimé, ou si le diff touche `FRAMES_EVENTS`, `RECORDINGS` ou un fichier d'événements.
- `FAILED:` si un test existant change. Un test de régression du §6 qui changerait d'intention est l'arrêt du §7 ; ici, toute modification d'un test existant l'est.

**Périmètre**

- `FAILED:` si un fichier de `src/` change. D.0 est « mesure seule, sans conversion ».
- `FAILED:` si le diff supprime ou modifie le recensement : `pico8_member_census.*`, ses crochets, `tools/member_census.*`, `member-census`, `--c-table`. C'est D.1.
- `FAILED:` si le diff écrit la liste ordonnée de D, une ligne D.2 ou suivante, ou rattache un test du §6 à un groupe. C'est l'écriture de la liste, après D.0.
- `FAILED:` si `data-residue` entre dans `make test` ou `make e2e`, ou si `data-guard` n'est pas un prérequis de `test`.

**Compteur**

- `FAILED:` si un motif, l'exclusion `spec`, l'ordre ou le format de sortie diffère du PRD sans justification dans le commit ; ou si une catégorie est ajoutée ou retirée. Une catégorie nouvelle est une décision de l'écriture de la liste (lecture « `malloc` »), pas de D.0.
- `FAILED:` si le compteur rend 0 sur une mesure sur rien (dossier absent, aucun `.c`/`.h`, catégorie inconnue), si `--check` rend 0 sur une occurrence présente, ou si le mode compteur rend autre chose que 0 sur une entrée valide.
- `FAILED:` si `test_data_residue.c` apparaît dans `--detail` (H4).
- `FAILED:` si la sortie de `make data-residue` diffère de H3 sans explication dans le commit.

**Garde**

- `FAILED:` si P1, P2 ou P3 ne rougit pas comme décrit, ou s'il manque sa ligne « Runs ».
- `FAILED:` si `entity_value_members.sh` sans option change de sortie ou de code, ou si `--c-table` change.
- `FAILED:` si `DATA_RESIDUE_GUARD` n'est pas vide sur l'arbre final. Aucune catégorie n'est à 0 sur ecac570 ; une catégorie à 0 à l'arbre final veut dire que l'arbre a changé (H14).

**Commit et note**

- `FAILED:` si le travail n'est pas en un seul commit vert.
- `FAILED:` si le budget est dépassé sans note de ce qui n'a pas été lancé, ou s'il manque le tableau « Runs », la sortie initiale de `make data-residue` dans le message de commit, ou la note « Découvert à D.0 ».
- `FAILED:` si un commentaire ajouté sort des quatre sortes de `CLAUDE.md`.

## Out of Scope

- **Toute conversion** : typer un argument de dispatch, remplacer une table ob, générer une table `static const`, retirer un `malloc`.
- **L'écriture de la liste ordonnée de D** : les groupes, leur ordre, leurs « Compte : N », le découpage au-delà de ~40 fichiers, le rattachement des tests du §6 avec leur preuve rouge. Il en va de même de la décision sur la représentation de `pico8_list_t` et de toute catégorie « `malloc` ».
- **D.1** : la suppression du recensement, dont `--c-table`.
- Un paramètre de dossier pour `entity_value_members.sh` ; un TU pour le `grep offsetof`.
- `make class-coverage-net`, `make sanitize-net`, `make member-census`, `make ref-audit`.
- Les `malloc` des entités (Plan 2 §8), TROU-7.
- Fichiers d'événements, `FRAMES_EVENTS`, rendu, audio, appli raylib.

## Further Notes

- **Décisions de l'agent à la place de l'utilisateur** (consigne `to-prd` : vérifier modules, tests et découpage avec l'utilisateur), tranchées en `guideline` et marquées (G) plus haut :
  - modules : un script neuf (`data_residue.sh`, trois modes), un mode `--check` ajouté à `entity_value_members.sh`, deux cibles (`data-residue`, `data-guard`) et une variable (`DATA_RESIDUE_GUARD`) ;
  - tests : un TU sur mini-arbre pour le compteur, trois occurrences plantées pour la garde, pas de `gcov` ;
  - découpage : aucun. Cinq fichiers, bien sous le seuil de ~40. Un seul commit, donc **pas de section « Implementation Chunks »**, que `prd-autopilot` traiterait comme autant de commits.

  Ordre de travail conseillé :
  1. le TU rouge ;
  2. le script ;
  3. le TU vert ;
  4. le `--check` de `entity_value_members.sh` ;
  5. les cibles et la légende ;
  6. P1 à P3 ;
  7. la note du plan.
- **Qui écrit la liste de D — lecture (G)** : **l'implémenteur de D.0 ne l'écrit pas.** La puce dit « La liste ordonnée de D est écrite après D.0, à partir de sa sortie ». Le §7 dit : « écrite dans ce plan à l'entrée de la phase, avant son premier commit — pour 1b, après 1b.0 et avant 1b.1 ». Le précédent de 1b l'a faite en commit de docs séparé du pilote : c296833, « règle de coexistence de 1b et liste des étapes 1b.1 à 1b.37 », après d64baa4 (1b.0) et sa graduation 37d73d8. D.1 est déjà nommée par le plan et reste le premier groupe. Le pilote écrit la liste à partir de `sh tools/data_residue.sh --detail`, dans un commit `docs:` entre la graduation de D.0 et le PRD de D.1 (ou D.2 si D.1 passe d'abord, puisqu'elle est déjà écrite). Rien dans le plan n'exige que la liste soit dans le commit de D.0.
- **Note « Découvert à D.0 »** (à ajouter à la fin de la puce D.0 ; le reste va dans le commit), à ajuster aux chiffres réels :

  > Découvert à D.0 : `make data-residue` (`tools/data_residue.sh`, modes `--detail` et `--check`), neuf catégories figées — `valeur`, `ob` (avec `pico8_fi`), `stash`, `type-tag` (tout `pico8_type_tag.h`, dont le `tag` de `pico8_struct_header_t`), `ref-audit`, `par-nom` (`*field_truthy` seulement), `dispatch` (`pico8_value_function(_t)`, `pico8_mv` ; pas `methods[]`), `overrides`, `spec` (hors six `_default` à argument omis) — lignes qui se chevauchent, commentaires compris ; comptes initiaux (lignes/fichiers) 6129/149, 2331/143, 56/14, 454/74, 70/17, 1104/92, 180/57, 421/57, 942/68, **9896 lignes distinctes dans 161 fichiers** (102 de `src/`, 59 de `tests/`). Sortie vide = la seule ligne `résidu D : 0 ligne(s), 0 fichier(s)`. `pico8_list_t` sans catégorie (§6 ne le retire pas), mais ses éléments `pico8_value_t` comptent dans `valeur` : la liste de D doit en changer la représentation. « Plus aucun `malloc` pour des données » lu comme les allocations de `pico8_ob.c`/`pico8_stash.c`/`pico8_value.c`, éteintes avec leurs catégories. Garde : `make data-guard` dans `make test` (`entity_value_members.sh --check`, `grep -rn offsetof src/`, `DATA_RESIDUE_GUARD`, vide à D.0) ; une étape qui éteint une catégorie l'ajoute à `DATA_RESIDUE_GUARD`. Motifs figés : une étape ne change un motif qu'avec la preuve que les lignes retirées ne touchent plus aucune catégorie transversale.
- **Pour l'écriture de la liste** (informations, pas des décisions de D.0) :
  - `valeur` (149 fichiers) et `ob` (143) sont transversales. Elles décroissent à mesure que les sous-catégories tombent, et n'ont pas vocation à former un groupe d'un seul tenant.
  - `tests/` porte 61 % des lignes de `valeur`, dont 513 dans `test_pico8_ui.c`.
  - Les tests de la couche (`test_pico8_value`, `test_pico8_ob`, `test_pico8_stash`, `test_pico8_list`) tombent avec leur module (§6).
  - `ref-audit` n'a aucune ligne dans `tests/`, et 9 dans `pico8_list.c`.
  - `spec` est concentrée dans `pico8_planet_family.c` (241 lignes) et `pico8_planet_family_trade.c` (103).
- **Le compteur sert ensuite de compteur de progrès** à chaque étape de D, comme `value-members` en 1b : son pied avant et après va dans chaque message de commit.
- **Contrôle d'étape** : un sous-agent frais relit le diff contre ce PRD avant le commit (hors budget de tests).
- Le message de commit porte :
  - le périmètre et les fichiers touchés ;
  - les décisions (G) et leurs rejets ;
  - la sortie initiale complète de `make data-residue` et le sous-compte `pico8_type_tag_of` ;
  - le tableau « Runs » résumé, dont P1 à P3 ;
  - les réponses à H1 à H14 ;
  - les faux positifs relevés (H10, H11) ;
  - la note « Découvert à D.0 ».
