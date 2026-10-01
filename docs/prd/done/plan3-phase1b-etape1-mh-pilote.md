---
title: "Plan 3, étape 1b.1 : `mh` (boîte de message), pilote de la phase 1b — ses 5 membres `pico8_value_t` prennent leur type C, et le patron de 1b est fixé"
description: "`mh` est la plus petite struct d'entité dont la ligne de recensement mêle un fixe simple (`fa`, `rh`), un membre à nil observé (`lh`, `p_`) et un REF (`nu`). La typer fixe, pour les 36 étapes suivantes, la disposition d'un membre typé, l'indicateur de présence, l'écriture à l'init, l'emballage du lecteur et le déballage de l'écrivain, sans qu'un seul site par nom ne change ni qu'un pixel du filet ne bouge."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Étapes de la phase 1b ») :

> - 1b.1 — `mh` (`msgbox.h:22`), **pilote** : 5 membres, à égalité avec `bg` et `qb`, la plus petite qui mêle fixes simples (`fa`, `rh`, jamais nil), nil observé par preuve directe (`lh`, `p_` : `lu`, la lecture par nom a rendu 0 — celui de `bg.m_` n'est que `stat?`, partagé par 6 typedefs) et REF (`nu`) ; `ha_pz`, `trade`, `pj_cr` (3-4 membres) n'ont pas de fixe simple. `items` (`pico8_list_t`) hors compte. Empreinte 19 (src 8, tests 11). Compte : 265.

Puces du §5 (« Phase 1b ») qui cadrent l'étape :

> - **Nil** : un indicateur de présence **uniquement** là où le code observe nil (test de vérité, `mv` qui rend 0 sur champ absent, affectation `= nil`). Rappel : en Lua `0` est vrai, donc un fixe à 0 n'est pas nil. Pas de valeur sentinelle.
> - Les REF deviennent des pointeurs typés : les sites de `pico8_ref_audit.h` disparaissent au fil de l'eau.
> - **Coexistence** (fixée à l'écriture de la liste, agent `guideline`) : le `field`/`set_field` du descripteur est le seul point de conversion ; les sites par nom (`pico8_entity_get_field`/`set_field`, `*_truthy`, `src/` et `tests/`) ne changent pas.
>   - Lecture : le membre typé est rendu emballé, du genre qu'il aurait eu en 1a, un pointeur (REF, chaîne, fonction) repassé tel quel ; absent exactement là où 1a rendait 0 — pointeur NULL, ou indicateur de présence faux.
>   - Écriture : l'écrivain déballe ; nil éteint la présence ; un genre hors de la ligne de recensement du membre est un `assert` (arrêt « champ sans règle », §7).
>   - Tables d'offsets (`members[]`, `defaults[]`, `pico8_X_member`) : elles ne listent que des `pico8_value_t`. Le commit qui type un membre l'en retire partout, tables des filles comprises (`offsetof(fille, parent.m)`), et l'initialise par affectation typée ; un défaut REF garde la même REF.
>   - Présence : un membre non pointeur lisible par nom (`l`) qui peut valoir nil (nil observé, ou tenu) garde un indicateur, sauf preuve du PRD qu'aucune lecture par nom ne le voit nil.
>   - Garantit : présent/absent, valeur et identité de REF lus par nom identiques à 1a ; aucune écriture `*(pico8_value_t *)` sur un membre typé. Vérifié à chaque étape par `grep -n 'offsetof(pico8_T_t, \([a-z0-9_]*\.\)*m)' src/` vide (m typé, T la struct et ses filles) et la mutation (2) du §7.
>   - Un site par nom remplacé par un accès direct est noté dans « Découvert à 1b.N » ; les autres survivent à 1b.
> - **Pilote (1b.1)** : la plus petite struct dont la ligne de recensement mêle au moins un fixe simple, un membre à nil observé et un membre REF. Son « Découvert à 1b.1 » fixe le patron (disposition du membre typé, indicateur de présence, écriture à l'init, emballage du lecteur) ; les PRD suivants le citent, et s'en écarter est une « lecture nouvelle d'une règle » (§7).
> - **Sortie** : aucune struct d'entité ne contient de `pico8_value_t` — constaté par une commande nommée […]. Le même script sert de compteur de progrès à chaque étape.

Et le §7 :

> - Chaque commit de 1a passe `make class-coverage-net` […]. En 1b, seulement les étapes dont le diff touche un constructeur, un appel `pico8_class_coverage_record` ou `KNOWN[]` ; toujours à la sortie de phase.
> - **ASan+UBSan, en 1b** : chaque étape ajoute un build ASan+UBSan du binaire de rejeu, passé sur `FRAMES_EVENTS` ; critère : sortie propre, sans comparaison d'empreinte.
> - **Preuve des tests** […] En 1b, les mutations comprennent au moins : (1) indicateur de présence forcé vrai sur un membre à nil observé ; (2) lecture emballée d'un fixe typé qui rend « présent 0 » là où il était absent ; (3) quand ça s'applique, boucle d'init qui saute un membre typé. Chacune rougit un test, sinon le trou est justifié dans le message de commit.

Note « Découvert à 1b.0 », extrait utile : script de sortie `make value-members` (`tools/entity_value_members.sh`), compte initial **270** ; table `tools/member_census.txt`, « une entrée de la liste et pas un oracle », réécrite par `make member-census` (hors suite) ; les sites à table d'offsets ne sont pas crochetés, le recensement observe en quatre points (`PICO8_MEMBER_CENSUS_SEE` en fin de constructeur, lecture et écriture par nom dans `entity.c`, `WALK` sur `hv` en fin de frame) ; `make sanitize-net` 15 propres, 221 s machine libre (445 s sous la charge de `make e2e`), donc **chaque étape 1b lance `make sanitize-net`**.

### État de départ

HEAD c296833 (plan 3 : règle de coexistence et liste 1b.1 à 1b.37 écrites). 1b.0 livrée à d64baa4. Arbre propre.

**Rien n'a été compilé ni lancé pour ce PRD.** Numéros de ligne et comptes : `grep`/`sed` sur c296833. Tout fait d'exécution est une hypothèse (H1 à H12).

### Ligne de recensement de `mh` (`tools/member_census.txt:54-58`)

| membre | accès | genres | nil | preuves-nil | par-nom |
|---|---|---|---|---|---|
| `nu` | offset | `ref:table` | jamais | - | l |
| `fa` | offset | `fixe` | jamais | - | l |
| `rh` | offset | `fixe` | jamais | - | l |
| `lh` | offset | `fonction` | observé | lu | l |
| `p_` | offset | `bool` | observé | lu | l |

Aucun membre n'est écrit par nom (colonne `par-nom` sans `e`) : le descripteur de `mh` n'a pas d'écrivain (`set_field` NULL, `msgbox.c:148`).

### Ce que fait le cart (`pico8/lua.lua:580-595, 855-899`)

```lua
mh=ke:jr([[
nu=o("mh"),
fa=14,
rh=5,
]])function mh:pq()
if (kc)kc.p_=true
kc=self
end
function mh:ev()local t=self.t/self.rh-10
if t>=0 and mv(self,"lh")then
self.t,t=self.rh*10,0
end
local nc=-t^3/9
bl(self,nc)camera()if t>10.5 then
self.p_,kc=true
end
end
```

- `mh(iw)` fait de la table de construction l'instance : `pj:hk` passe `pj.ea` (positionnels `[1]`, `[2]`, `[3]` facultatif, et `lh` posé une fois pour toutes dès la première année à message), `pj:ph` passe une table neuve avec `rh=3` et `lh`.
- `p_` ne reçoit que `true` (deux sites) ; `dg` retire de `hv` une entité dont `p_` est vrai. Sa valeur est donc nil ou `true`, jamais `false`.
- `lh` est une fonction ou nil ; `mv(self,"lh")` rend nil quand elle manque.
- `nu`, `fa` : défauts de classe jamais réécrits ; `rh` : défaut 5, ou 3 par la table de `pj:ph`.

### Le code aujourd'hui

- `src/pico8_msgbox.h:22-29` : `typedef struct { pico8_ke_t ke; pico8_value_t nu, fa, rh, lh, p_; pico8_list_t items; } pico8_mh_t;` (un membre par ligne).
- `src/pico8_msgbox.c` :
  - `pico8_mh_pq` (`:56-67`) : `previous->p_ = pico8_value_bool(1)` ;
  - `pico8_mh_ev` (`:82-119`) : `rh = pico8_mh_value_fixed(self->rh)` (`:92`), `mv(self,"lh")` **par nom** (`:97`), `self->p_ = pico8_value_bool(1)` (`:115`) ;
  - lecteur `pico8_mh_field` (`:121-143`) : chaîne de `strcmp` sur les 5 noms vers un `const pico8_value_t *member`, absent si `REF NULL`, sinon repli sur `pico8_ke_field`. **Aucune table d'offsets** : `grep -rn 'offsetof(pico8_mh_t' src tests` et `grep -n 'offset\|(pico8_value_t \*)' src/pico8_msgbox.c` sont vides ;
  - descripteur `g_mh_struct_class` (`:145-149`) : `EV` seul, lecteur, **écrivain NULL** ;
  - constructeur `pico8_mh_struct_new` (`:151-184`) : 5 affectations `pico8_value_t` (`nu` = `g_mh_nu`, la table partagée parsée par `pico8_mh_init`), `items`, boucle sur les clés de construction (positionnel → `items`, `rh` et `lh` → membre tel quel, toute autre → assert « champ sans règle »), couverture, `mh:pq`, `ke:pq` (`lt`, qui lit `nu` et les noms de `bm` par nom), `PICO8_MEMBER_CENSUS_SEE(self, pico8_mh_t)`.
- Constructions : `pico8_mh_new` depuis `score_panel_display.c:100` (`pj:hk`, `pj.ea`) et `:317` (`pj:ph`, `rh`=3 fixe, `lh` fonction) ; `tests/test_pico8_mh.c:350` (`lh` fonction) ; `tests/test_pico8_spec_defaults.c:182` (`NULL`). Aucune autre clé nommée n'arrive (`score_panel_display.c:94,313-314`).
- Lectures par nom sur un `mh` : `nu` par `lt`/`he` (`entity.c`), `fa` par `dc` (tri par couche de `cv.ev`), `lh` par `mv` (`msgbox.c:97`), `p_` par `dg` (`bootstrap.c:91`), et, dans les tests, `fa`/`rh`/`nu`/`p_`/`lh` (`test_pico8_mh.c:106,237,240,311-321,360`) et les noms de `test_pico8_spec_defaults.c` (`STRUCT_HASH`).
- Aucun accès direct à un membre de `mh` hors de `msgbox.c` : `score_panel_display.c` ne fait que construire (`self->ed` est un membre de `pj`, 1b.16). Aucune fille de `mh`.

### Empreinte remesurée

La ligne dit 19 fichiers (src 8, tests 11), majorant dû aux noms courts partagés (`fa`, `p_`, `nu`). Fichiers **modifiés** par l'étape : `src/pico8_msgbox.h`, `src/pico8_msgbox.c`, `tests/test_pico8_mh.c`, `docs/plan-3.md`. Les 17 autres ne lisent `mh` que par nom et gardent un diff vide.

## Solution

1. **Caractérisation d'abord, figée sur l'ancien code**, dans `test_pico8_mh` : les 5 membres lus **par nom seulement** (`pico8_entity_get_field`, `pico8_mv`), présents/absents, genre, valeur, identité de REF et de pointeur de fonction, pour une boîte sans clé, une boîte à clés `rh`/`lh`, et la boîte précédente marquée `p_` par `mh:pq`. Elle compile et passe à l'identique avant et après le typage.
2. **Typage des 5 membres** dans `pico8_mh_t` : `nu` → `pico8_ob_table_t *`, `fa`/`rh` → `pico8_fixed_t`, `lh` → `pico8_value_function_t`, `p_` → `int` avec l'indicateur `has_p_`. Constructeur, `mh:pq`, `mh:ev` et lecteur réécrits selon le patron ci-dessous ; les sites par nom ne changent pas.
3. **Le patron de 1b**, règles R1 à R9 ci-dessous, appliqué ici et résumé dans la note « Découvert à 1b.1 » que les 36 PRD suivants citent.

Effet mesurable : `make value-members` passe de 270 à **265**, aucune ligne `pico8_mh_t`. Rien d'autre ne bouge : 15 « conforme », `class-coverage-net` identique, `sanitize-net` propre.

## User Stories

1. En tant que lecteur de `msgbox.c`, je veux lire `self->rh` comme un `pico8_fixed_t`, afin de ne plus passer par `pico8_mh_value_fixed` pour un membre qui n'a jamais tenu autre chose qu'un fixe.
2. En tant que lecteur de `msgbox.c`, je veux que `lh` soit un pointeur de fonction, NULL quand la boîte n'en a pas, afin que « absent » se lise sur le type.
3. En tant que lecteur de `msgbox.c`, je veux que `nu` soit un `pico8_ob_table_t *`, afin que la table de tags partagée ait un type.
4. En tant que lecteur de `msgbox.c`, je veux que `p_` porte à côté de lui un indicateur `has_p_`, afin que « nil » et « vrai » restent distincts sans valeur sentinelle.
5. En tant que `dg`, je veux que `p_` lu par nom soit absent tant que la boîte n'est pas fermée, puis `BOOL` vrai, afin de retirer la boîte de `hv` à la même frame qu'avant.
6. En tant que `mh:ev`, je veux que `mv(self,"lh")` rende 0 sur une boîte sans `lh` et appelle la même fonction sur une boîte qui en a une, afin que le compte à rebours de l'année soit inchangé.
7. En tant que `lt`/`he`, je veux lire `nu` par nom comme la même REF qu'avant, afin que la boîte entre et sorte de `cg("mh")` comme avant.
8. En tant que `dc("ev")`, je veux lire `fa` par nom comme le fixe 14, afin que la boîte reste dans la même couche.
9. En tant que `pj:ph`, je veux que la clé `rh`=3 de ma table de construction arrive dans le membre typé, afin que la boîte des scores garde son rythme.
10. En tant que `pj:hk`, je veux que la clé `lh` de `pj.ea` arrive dans le membre typé avec la même identité de fonction, afin que « last year! » compte ses 90 appels.
11. En tant que test par nom (`test_pico8_mh`, `test_pico8_spec_defaults`), je veux lire les mêmes présences, genres et valeurs qu'avant, afin que mes empreintes figées ne bougent pas.
12. En tant que gardien de l'invariant §3, je veux `make test` vert, 15 « conforme » et aucun `*.sha256` touché.
13. En tant que gardien de la sécurité mémoire, je veux `make sanitize-net` propre sur les 15 rejeux, afin qu'un membre non initialisé ou mal dimensionné soit vu.
14. En tant que gardien de la couverture, je veux `make class-coverage-net` identique à sa référence, le constructeur de `mh` étant touché.
15. En tant que `plan-pilot`, je veux `make value-members` à 265 et aucune ligne `pico8_mh_t`, afin de mesurer le progrès de la phase.
16. En tant qu'auteur d'un PRD 1b suivant, je veux une règle écrite pour chaque question du patron (type C par genre, indicateur, init, lecteur, écrivain, tables d'offsets, crochets du recensement, compte), afin de la citer au lieu de la redécider.
17. En tant qu'auteur d'un PRD 1b suivant dont la struct a un écrivain, je veux la règle de déballage fixée ici même si `mh` n'a pas d'écrivain, et le point où le pilote l'exerce (clés de construction), afin que la première struct à écrivain ne soit pas une « lecture nouvelle ».
18. En tant qu'auteur d'un PRD 1b suivant, je veux savoir que les crochets du recensement restent tels quels sur une struct typée, et pourquoi, afin de ne pas les réécrire.
19. En tant que vérificateur, je veux une caractérisation écrite **avant** le typage et passée sur l'ancien code, qui ne lit l'état que par nom, afin qu'elle juge le typage sans avoir été ajustée à lui.
20. En tant que vérificateur, je veux les trois mutations du §7 déclinées sur `mh`, chacune rouge sur un test nommé, afin de savoir que la caractérisation mord.
21. En tant que vérificateur, je veux le rapport `gcov` des lignes modifiées de `msgbox.c`, afin de voir qu'aucune ligne typée n'échappe aux tests.
22. En tant que vérificateur, je veux qu'un genre de clé de construction hors de la ligne de recensement avorte, afin que l'arrêt « champ sans règle » soit un fait et non une intention.
23. En tant qu'autopilote, je veux chaque condition d'arrêt du §7 écrite comme critère `FAILED`.
24. En tant que lecteur du plan, je veux la note « Découvert à 1b.1 » sur la ligne de l'étape, qui fixe le patron en quelques lignes.
25. En tant qu'implémenteur, je veux un budget de tests de ~20 min et un tableau « Runs », afin de ne relancer que ce qui suit le dernier changement.

## Implementation Decisions

Pas d'humain dans la boucle. Le skill `to-prd` demande de faire valider les modules et les tests par l'utilisateur ; ces choix sont tranchés ici comme le ferait un agent `guideline` (hypothèses explicites, simplicité, changement chirurgical, critères vérifiables), et marqués (G), avec les options rejetées.

### Le patron de 1b (R1 à R9)

Chaque règle est appliquée à `mh` dans cette étape, sauf R5 et R7 dont la forme générale est fixée ici et l'exercice sur `mh` précisé.

- **R1, type C par genre de la ligne de recensement.** Le membre garde son nom minifié et sa place dans le typedef.

  | genre | type C |
  |---|---|
  | `fixe` | `pico8_fixed_t` |
  | `bool` | `int` (0 ou 1) |
  | `chaine` | `const char *` |
  | `fonction` | `pico8_value_function_t` |
  | `ref:table` | `pico8_ob_table_t *` |
  | `ref:liste` | `pico8_list_t *` |
  | `ref:pico8_X_t` | `pico8_X_t *` |
  | `ref:autre`, genres mêlés, sans genre | tranché par le PRD de l'étape, sur sa ligne |

  - (G) `int` pour `bool` : c'est la représentation de `as.boolean`, de `pico8_value_bool(int)` et de tous les prédicats du port. Rejeté : `_Bool`/`<stdbool.h>`, absent de `src/`, et `unsigned char`, économie sans objet avant Plan 2 §8 et la phase 2.
  - (G) Pointeurs non `const` : le lecteur rend `pico8_value_ref(void *)` ; un `const` imposerait un cast qui retire le qualificatif à chaque emballage. Rejeté pour `nu` pour cette raison, alors même que rien n'écrit par `mh->nu`.
- **R2, présence.** Un indicateur `int has_<m>` n'existe que pour un membre **non pointeur**, lisible par nom (`l`), dont la ligne dit nil `observé` ou `tenu` (§5, puce Présence), sauf preuve du PRD qu'aucune lecture par nom ne le voit nil.
  - **Où** : déclaré immédiatement après `<m>`, dans le même typedef que `<m>`. L'indicateur d'un membre de parente vit donc dans la parente, et une fille ne le touche pas.
  - **Nom** : `has_` suivi du nom minifié exact (`has_p_`, `has_m_`). Aucun membre d'entité ne commence par `has_` (grep sur `src/*.h`, vide).
  - **Sens** : `has_<m>` = 0 veut dire nil ; `<m>` vaut alors 0 et n'est jamais lu. Aucune valeur de `<m>` ne signifie nil (pas de sentinelle).
  - Un **pointeur** n'a pas d'indicateur : NULL est nil (§5 : « pointeur NULL »). Un membre `jamais` nil n'en a pas.
  - (G) Rejeté : un masque de bits par struct (`uint32_t present`). Plus compact, mais il demande une énumération de bits par typedef, ne se lit pas dans un débogueur, et se partage mal entre une parente et ses filles enchâssées. Rejeté : un `int p_` à trois états (−1 nil). C'est une sentinelle.
- **R3, écriture à l'init.** Chaque membre typé est posé par **affectation typée**, à la place de son affectation `pico8_value_t`, avant toute clé de construction et avant tout `pq`. L'indicateur est posé explicitement (0 pour un membre nil à l'init), et `<m>` avec lui (0), pour qu'aucun octet de la struct `malloc`ée ne reste indéterminé. Un défaut REF garde **la même REF** (`nu` = `g_mh_nu`, pas une nouvelle table).
- **R4, lecteur du descripteur (emballage).** Une branche par membre typé dans le `field` du descripteur, avant le repli sur la table d'offsets restante ou sur le lecteur de la parente :
  - pointeur NULL, ou `has_<m>` = 0 : `return 0`, `*out_value` intact (contrat de `pico8_struct_field_fn`) ;
  - sinon `*out_value = pico8_value_<genre>(<m>)` du genre de la ligne (1a) : `pico8_value_fixed`, `pico8_value_bool`, `pico8_value_string`, `pico8_value_function`, `pico8_value_ref` ; un pointeur est repassé tel quel, jamais copié ; `return 1`.
  - Un membre non pointeur sans indicateur est toujours présent.
- **R5, écrivain du descripteur et clés de construction (déballage).** Même règle aux deux endroits, puisque les deux reçoivent un `pico8_value_t` à ranger dans un membre typé :
  - nil (`REF NULL`) : pointeur mis à NULL, ou `has_<m>` = 0 ; sur un membre sans indicateur, `assert` « champ sans règle » (§7) ;
  - genre de la ligne : `<m> = value.as.<champ>`, `has_<m>` = 1 s'il existe ; pour un `ref:pico8_X_t`, `assert` sur le tag et le descripteur de la cible ; `return 1` pour un écrivain ;
  - tout autre genre : `assert` dont le message nomme le membre, le genre refusé, la ligne de recensement et l'arrêt « champ sans règle » du §7.
  - **Exercice sur `mh`** : `mh` n'a pas d'écrivain (ligne sans `e`, `set_field` NULL). R5 s'applique à ses deux clés de construction `rh` et `lh`. Aucun écrivain n'est ajouté à `mh` (G : pas de code sans appelant). La première étape dont la struct a un écrivain (`bg` à 1b.15 pour `m_` ; `bv_et` à 1b.22 pour `fo`) applique R5 telle quelle.
- **R6, écriture directe dans les méthodes.** Un site qui écrivait `self-><m> = pico8_value_<genre>(x)` écrit `self-><m> = x` ; s'il existe un indicateur, les deux affectations sont côte à côte (`self->p_ = 1; self->has_p_ = 1;`). Une lecture directe qui testait `kind`/`as.ref == NULL` lit le membre typé ; le compilateur désigne tous ces sites.
- **R7, tables d'offsets.** Le commit qui type `<m>` retire son entrée de toutes les tables (`members[]`, `defaults[]`, `keys[]`, `pico8_X_member`), celles des filles comprises (`offsetof(pico8_F_t, parent.<m>)`), et remplace la recopie de défaut par l'affectation typée de R3. **Preuve** : pour T la struct et chacune de ses filles, `grep -n 'offsetof(pico8_T_t, \([a-z0-9_]*\.\)*\(m1\|m2\|…\))' src/` vide, plus `grep -rn 'offsetof(pico8_T_t' src/` relu à la main si le typedef garde des membres `pico8_value_t`.
  - **Exercice sur `mh`** : le lecteur est une chaîne de `strcmp`, sans table ; `mh` n'a pas de fille. `grep -n 'offsetof(pico8_mh_t, \([a-z0-9_]*\.\)*\(nu\|fa\|rh\|lh\|p_\))' src/` est vide **avant** comme après : la preuve est vacante ici, et la mutation (2) porte seule la garantie de lecture. `grep -n '(pico8_value_t \*)\|(const pico8_value_t \*)' src/pico8_msgbox.c` reste vide.
- **R8, crochets du recensement.** Inchangés. `PICO8_MEMBER_CENSUS_SEE(self, pico8_mh_t)` reste en fin de constructeur avec le même typedef. Le recensement ne connaît que les membres listés par `tools/entity_value_members.sh --c-table`, qui ne liste que des `pico8_value_t` : un membre typé en sort de lui-même, O1/O4 (instantanés) cessent de le lire, O2/O3 (par nom) l'ignorent. Un typedef sans membre `pico8_value_t` est déjà géré (`pico8_os_t`, placeholders `mo`/`je`). Aucun crochet n'est ajouté pour un membre typé : son type est la garantie. `tools/member_census.txt` garde les lignes typées jusqu'à la prochaine régénération (1b.0 : « pas un oracle ») ; aucune ligne n'y est éditée à la main.
- **R9, compteur.** `make value-members` baisse exactement du nombre de membres typés. `int has_<m>` et `pico8_list_t` ne comptent pas (le parseur ne compte que `pico8_value_t`). Ici : 270 → **265**, et `make value-members | grep -c ' pico8_mh_t '` = 0.
- **Coexistence, rappel appliqué** : les sites par nom ne changent pas. `mv(self,"lh")` de `mh:ev` reste par nom (G : changement chirurgical ; un appel direct `self->lh(...)` serait une réécriture de site, à noter dans « Découvert », sans gain ici).

### Les 5 membres de `mh`, justifiés par leur ligne et le code

| membre | ligne | type C | indicateur | init | lecteur | écriture |
|---|---|---|---|---|---|---|
| `nu` | `ref:table`, jamais nil | `pico8_ob_table_t *` | non (pointeur) | `= g_mh_nu` (même REF) | NULL → 0 ; sinon `pico8_value_ref(mh->nu)` | aucune après l'init |
| `fa` | `fixe`, jamais nil | `pico8_fixed_t` | non (jamais nil) | `= pico8_fixed_from_int(14)` | toujours `pico8_value_fixed(mh->fa)` | aucune après l'init |
| `rh` | `fixe`, jamais nil | `pico8_fixed_t` | non (jamais nil) | `= pico8_fixed_from_int(5)` | toujours `pico8_value_fixed(mh->rh)` | clé `rh` : R5 (FIXED déballé ; nil ou autre genre → assert) |
| `lh` | `fonction`, nil observé (`lu`) | `pico8_value_function_t` | non (pointeur) | `= NULL` | NULL → 0 ; sinon `pico8_value_function(mh->lh)` | clé `lh` : R5 (FUNCTION déballé ; nil → NULL ; autre → assert) |
| `p_` | `bool`, nil observé (`lu`) | `int` | **`has_p_`** | `p_ = 0`, `has_p_ = 0` | `has_p_` = 0 → 0 ; sinon `pico8_value_bool(mh->p_)` | `mh:pq` et `mh:ev` : `p_ = 1; has_p_ = 1;` (R6) |

- `nu` : un pointeur, NULL seulement avant `pico8_mh_init` (`g_mh_nu` NULL) ; absent alors comme en 1a (`REF NULL`).
- `fa` : lu par nom par `dc` (couche) et les tests ; jamais écrit après l'init, donc toujours présent.
- `rh` : lu directement par `mh:ev` (`self->rh` à la place de `pico8_mh_value_fixed(self->rh)`), par nom par les tests. Ses seules clés de construction sont des fixes (`score_panel_display.c:313`) : un nil sur `rh` n'a pas de règle (membre sans indicateur, la ligne dit `jamais`), d'où l'assert.
- `lh` : lu **seulement** par nom (`mv`). Pointeur : NULL = nil, sans indicateur (R2). Une clé `lh` nil (`REF NULL`) donne NULL, comme 1a où le membre valait alors `REF NULL`.
- `p_` : valeurs nil et `true` seulement (cart, et les deux sites de `msgbox.c`) ; lu par nom par `dg` et vu nil (`test_pico8_mh.c:237`). Membre non pointeur, lisible par nom, nil observé : indicateur obligatoire (R2). (G) Rejeté : ne garder que l'indicateur en supposant la valeur `true` ; c'est `p_` réduit à sa présence, une sentinelle déguisée, et un patron qui ne vaudrait pas pour `jo.p_` ou `g_.p_` (écrits par nom).
- `items` (`pico8_list_t`) : hors compte et hors de l'étape.
- `pico8_mh_value_fixed` reste (il sert `self->ke.t`, membre de `ke`, 1b.14b).

### Contrats (commentaires, CLAUDE.md)

- `msgbox.h` : sur le typedef, une ligne de contrat pour `has_p_` (« 0 : `p_` est nil ; `p_` n'est alors pas lu ») et `lh` NULL = absent. Le commentaire existant sur `nu` (table partagée) reste. Rien d'autre : le patron va dans la note du plan et le message de commit.
- `msgbox.c` : les messages d'`assert` de R5 portent le piège (genre hors ligne → arrêt §7). Aucun commentaire de récit.

### Ce qui ne change pas

- Aucun site par nom de `src/` ni de `tests/` ; aucun test existant hors de `test_pico8_mh.c`, où seuls des tests sont **ajoutés**.
- Le descripteur (`EV` seul, lecteur, écrivain NULL), l'ordre du constructeur (clés, couverture, `mh:pq`, `ke:pq`, `SEE`), `pico8_mh_new`/`_default`/`_ready`/`_init`/`_shutdown`, `kc`.
- `KNOWN[]`, `tools/class_coverage_*`, `*.sha256`, `FRAMES_EVENTS`/`RECORDINGS`, `tools/member_census.txt`, les crochets du recensement, l'enum de méthodes.
- `pj` (dont `ed`, REF vers un `mh`) : 1b.16.

### Hypothèses à vérifier à l'exécution (rien n'a été lancé pour ce PRD)

- **H1** : aucune clé de construction `rh`/`lh` d'un autre genre que FIXED/FUNCTION n'arrive, ni nil (grep : `score_panel_display.c:94,313-314`, `test_pico8_mh.c:349`, `spec_defaults` sans clé). Les asserts de R5 ne se déclenchent sur aucun rejeu ni TU.
- **H2** : `test_pico8_mh` avec la caractérisation dure moins de 30 s (il reste dans `UNIT_TESTS`).
- **H3** : la caractérisation passe sur l'ancien code (c296833) sans aucune modification des valeurs après le typage.
- **H4** : `STRUCT_HASH` et `SPEC_HASH` de `test_pico8_spec_defaults` ne bougent pas (présence, genre et égalité de valeur par nom identiques).
- **H5** : aucun comportement ne dépend de la taille ou de la disposition de `pico8_mh_t` (la struct passe de 5×`pico8_value_t` à des scalaires et pointeurs) : 15 « conforme ».
- **H6** : `-std=c99 -Wall -Wextra -Werror` accepte `pico8_value_ref(mh->nu)` depuis un `const pico8_mh_t *` (le pointeur est `pico8_ob_table_t * const`, converti en `void *` sans perte de qualificatif sur la cible).
- **H7** : l'arbre `-DPICO8_MEMBER_CENSUS` compile toujours (table générée sans ligne `mh`, `SEE` sur un typedef sans membre comme `os`). Non exigé par l'étape (crochets inchangés) ; vérifiable en passe réduite si le budget le permet.
- **H8** : `make sanitize-net` : 15 propres, ~4 min seul.
- **H9** : `make class-coverage-net` identique à `tools/class_coverage_net_expected.txt` (ligne `mh` comprise).
- **H10** : la preuve `lu` de `lh` vient d'une boîte de `pj:hk` bâtie avant toute année à message (`pj.ea` sans `lh`) ; la caractérisation la reproduit directement par `pico8_mh_new(NULL)`.
- **H11** : `gcov` : toutes les lignes modifiées de `msgbox.c` sont exécutées par `test_pico8_mh`, sauf les branches d'assert (prouvées dans un fils) et la branche `nu == NULL` du lecteur (atteinte seulement avant `pico8_mh_init`), justifiée dans le commit.
- **H12** : `pico8_jl()` ne rappelle pas `pico8_mh_init` (seul `cart.c:69` le fait) : la table `g_mh_nu` est la même pour toutes les boîtes d'un processus de test.

## Implementation Chunks

Emprise : 3 fichiers de code et de test, plus le plan. Un seul commit (§7 : un commit par struct en 1b). Ordre de travail, chacun vérifiable seul :

### A. Caractérisation sur l'ancien code

Dans `tests/test_pico8_mh.c`, une fonction `test_mh_members_by_name` appelée après les tests existants (voir Testing Decisions). Lancée **sur c296833 non modifié** : verte, valeurs figées, ligne au tableau « Runs ».

### B. Typage

`msgbox.h` (typedef, contrats), `msgbox.c` (constructeur, `mh:pq`, `mh:ev`, lecteur) selon R1 à R6. `make test` vert, `make value-members` à 265.

### C. Forme

Dans `test_pico8_mh.c`, `test_mh_rejects_off_row_keys` : dans un fils (`must_abort`, modèle `test_pico8_fv`), `pico8_mh_new` avec `rh` chaîne, puis `rh` nil, puis `lh` fixe, avorte. N'existe qu'après B (l'ancien code accepte tout genre).

### D. Preuves et cibles

`gcov` et les trois mutations dans une copie jetable du scratchpad ; cibles finales ; note « Découvert à 1b.1 » dans `docs/plan-3.md`.

## Testing Decisions

- **Un bon test juge ce qu'on lit par nom** : présence, genre, valeur, identité de REF et de fonction rendus par `pico8_entity_get_field`/`pico8_mv`, appartenance à `hv`/`cv`/`cg`, pixels. Jamais `sizeof`, l'ordre des membres ni un accès `->` à un membre de `mh` : la caractérisation doit compiler sur les deux formes.
- **Caractérisation dirigée, figée sur l'ancien code** (`test_mh_members_by_name`, `test_pico8_mh`, pas de nouveau TU) :
  1. `pico8_jl()` ; `a = pico8_mh_new(NULL)` :
     - `fa` présent, `FIXED` 14 ; `rh` présent, `FIXED` 5 ;
     - `nu` présent, `REF`, table d'un seul tag `"mh"`, **même pointeur** que le `nu` d'une seconde boîte ;
     - `lh` absent et `p_` absent, `*out_value` intact (sentinelle `pico8_value_fixed(0x5A5A)` posée avant, relue après, comme `test_pico8_spec_defaults`) ;
     - `pico8_mv(a, "lh", …)` rend 0 ;
     - `a` dans `hv`, `cv.ev`, `cg("mh")`, et `pico8_kc() == a`.
  2. Clés `{rh = FIXED 3, lh = FUNCTION counting_lh}` : `b = pico8_mh_new(overrides)` :
     - `rh` présent, `FIXED` 3 ; `lh` présent, `FUNCTION`, **pointeur identique** à `counting_lh` ; `pico8_mv(b, "lh", …)` rend 1 et incrémente le compteur une fois ;
     - `p_` de `b` absent ; `p_` de `a` présent, `BOOL`, `as.boolean` = 1 (marqué par `mh:pq` de `b`) ;
     - un tag `pico8_type_tag_of(b)` = `STRUCT`.
  3. Empreinte : les 5 noms de `a` et `b` pliés (présence, genre, valeur ou identité) dans une empreinte figée `MEMBERS_BY_NAME_HASH`, relevée sur l'ancien code.

  Les tests existants de `test_pico8_mh` (séquences et écrans figés quand `mh` était une instance de classe) restent à diff vide et couvrent `rh`=3 de `pj:ph`, `lh` de `pj:hk`, `p_` vu nil puis vrai, `fa` par `dc`.
- **Forme** (`test_mh_rejects_off_row_keys`, après B) : les trois clés hors ligne avortent dans un fils.
- **Test de régression (§6)** : aucun ne lit un membre de `mh` directement ; tous gardent un diff vide.
- **Preuve des tests (§7)** :
  - **`gcov`** : dans une copie jetable du scratchpad, `test_pico8_mh` compilé en `--coverage -O0` avec les objets du moteur, lancé, `gcov` sur `pico8_msgbox.c`. Piège connu de 1a.23-1a.28 : dans la copie seulement, `-include` d'un en-tête qui fait précéder les `_exit` des fils de `__gcov_dump()`. Rapport dans le message de commit : chaque ligne exécutable ajoutée ou modifiée et son compte ; lignes à 0 justifiées (branches d'assert vues par `SIGABRT` dans un fils ; `nu == NULL`, H11).
  - **Mutations** (dans la copie, restaurées, `cmp` contre l'arbre), chacune rouge sur un test nommé, notée dans le commit :
    1. **(§7-1) indicateur forcé vrai** : `has_p_ = 1` à l'init. Attendu rouge : étape 1 de `test_mh_members_by_name` (`p_` absent), `test_two_years_in_a_row` (`:237`), et les empreintes de séquence (`fold_box` plie `p_`).
    2. **(§7-2) « présent 0 » là où c'était absent** : sur `mh`, aucun fixe n'est jamais absent (`fa`, `rh` : `jamais`), la forme littérale n'a pas de cible. Appliquée sous sa forme la plus proche, sur le pointeur à nil observé : la branche NULL de `lh` rend `pico8_value_fixed(0)` et 1. Attendu rouge : étape 1 (`lh` absent, `mv` rend 0), et les séquences de `test_pico8_mh` (en Lua `0` est vrai : le compte à rebours se réarme, la boîte ne se ferme plus, `assert(run->ticks < 5000)`). Le trou de la forme littérale est noté dans le commit.
    3. **(§7-3) boucle d'init qui saute un membre typé** : `mh` n'a pas de boucle de mise à nil ni de recopie de défauts ; sa seule boucle d'init est celle des clés de construction. Mutation : elle saute la clé `rh`. Attendu rouge : étape 2 (`rh` = 3) et `test_scores_box_follows_c_` (`SCORES_TICKS`).
- **Prior art** : `test_pico8_spec_defaults` (lecture par nom, sentinelle d'`out_value` intacte) ; `test_pico8_fv` (`must_abort`, fils par scénario, piège `gcov`/`_exit`) ; les caractérisations de 1a.26-1a.28, figées avant la conversion.

### Vérification, budget et tableau « Runs »

**Budget : environ 20 minutes de temps total de tests.** Ordres de grandeur (1a.39-1b.0) :

| Cible | Durée |
|---|---|
| `make test` | 20 s à 1 min |
| `make e2e` | 4 à 6 min |
| `make class-coverage-net` | 3 à 4 min |
| `make sanitize-net` | ~4 min seul, ~7,5 min sous la charge de `make e2e` |
| `make value-members` | quelques secondes |
| `test_pico8_mh` seul | à mesurer (H2) |

Répartition indicative : `test_pico8_mh` seul sur l'ancien code (A) ; `make test` après B et en fin ; `gcov` et 3 mutations sur `test_pico8_mh` seul ; `make e2e` et `make class-coverage-net` dans la même invocation (arbres distincts) ; `make sanitize-net` **ensuite, seul** (sous charge, sa durée double). Au-delà du budget, l'implémenteur s'arrête et note ce qui manque plutôt que de relancer.

**L'implémenteur tient un tableau « Runs »** dans son journal, une ligne par lancement :

| # | Commande exacte | État de l'arbre (HEAD + « chunk X appliqué » / « mutation N, copie jetable ») | Code de sortie | Ligne clé | Durée |
|---|---|---|---|---|---|

Exemples de ligne clé : `test_pico8_mh: all tests passed`, `membres pico8_value_t : 265`, `frames-check : 15 conformes`, `sanitize: 15 propre(s)`, `couverture: OK -- … (15 rejeu(x))`, l'assert qui rougit une mutation. Le vérificateur réutilise ce tableau et ne relance que ce qui suit le dernier changement de l'arbre.

**Cibles finales**, vertes sur l'arbre final :

1. `timeout 10m make test` ;
2. `timeout 15m make e2e` : `frames-check` rend **15 « conforme »**, `UNIT_TESTS_SLOW` passent ;
3. `timeout 15m make sanitize-net` : `sanitize: 15 propre(s)` ;
4. `make value-members` : `membres pico8_value_t : 265`, aucune ligne `pico8_mh_t` ;
5. `timeout 15m make class-coverage-net` : **s'applique**. Le diff touche un constructeur (`pico8_mh_struct_new`, affectations typées et déballage des clés), même si l'appel `pico8_class_coverage_record` et `KNOWN[]` ne bougent pas (§7). Sortie identique à `tools/class_coverage_net_expected.txt` ;
6. `make member-census` : **ne s'applique pas**. Aucun crochet du recensement ne change (R8). Facultatif en fin de budget, en passe réduite (`CENSUS_EVENTS=<un rejeu> CENSUS_UNIT= CENSUS_TABLE=tmp/census/m.txt`), pour H7 ; la table versionnée n'est pas réécrite.

Jamais l'appli raylib.

### Critères d'échec (le vérificateur de `prd-autopilot` les traduit en `FAILED:`)

Conditions d'arrêt du §7 :

- `FAILED:` si le filet (`make test` ou `make e2e`) reste rouge après **3 tentatives** de correction sur ce commit.
- `FAILED:` si le diff modifie ou supprime un `*.sha256` du filet, ou retire une entrée de `FRAMES_EVENTS` ou de `RECORDINGS`.
- `FAILED:` si un champ ne tombe sous aucune règle du §5 et reçoit pourtant un traitement improvisé : un assert de R5 se déclenche sur un rejeu ou un TU hors `must_abort` ; un membre ou un indicateur autre que ceux du tableau est ajouté ; une clé de construction autre que positionnelle, `rh`, `lh` est acceptée ; un écrivain est ajouté au descripteur de `mh`.
- `FAILED:` si un test de régression de bug (§6) doit changer, ou si une valeur figée existante (`test_pico8_mh` : 4 séquences ; `test_pico8_spec_defaults` : `SPEC_HASH`, `STRUCT_HASH` ; tout autre test) est modifiée.
- `FAILED:` si `make class-coverage-net` ne rend pas 0 ou n'est pas identique à `tools/class_coverage_net_expected.txt`, ou si ce fichier, `tools/class_coverage_exempt.txt` ou `KNOWN[]` change.

Critères propres à l'étape :

- `FAILED:` si `make sanitize-net` n'est pas `15 propre(s)`, ou n'a pas été lancé sur l'arbre final.
- `FAILED:` si `make value-members` ne rend pas 265, ou liste encore un membre de `pico8_mh_t`, ou si un autre typedef perd ou gagne une ligne.
- `FAILED:` si un membre typé n'a pas le type du tableau, si `p_` n'a pas son indicateur `has_p_` déclaré juste après lui, si un autre membre de `mh` reçoit un indicateur, ou si une valeur de `p_` (ou de tout membre) sert de sentinelle de nil.
- `FAILED:` si le lecteur ne rend pas, pour chaque nom, la même présence, le même genre, la même valeur et la même identité de REF ou de fonction qu'en 1a, ou écrit `*out_value` sur un membre absent.
- `FAILED:` si `nu` n'est plus la REF `g_mh_nu`, ou si une table est parsée ou copiée pour lui.
- `FAILED:` si un site par nom de `src/` ou de `tests/` change (`mv(self,"lh")` compris), ou si un site par nom est remplacé par un accès direct sans être noté dans « Découvert à 1b.1 ».
- `FAILED:` si un crochet du recensement change, si `tools/member_census.txt` est édité à la main, ou si `tools/entity_value_members.sh` change.
- `FAILED:` si `grep -n 'offsetof(pico8_mh_t, \([a-z0-9_]*\.\)*\(nu\|fa\|rh\|lh\|p_\))' src/` ou `grep -n '(pico8_value_t \*)\|(const pico8_value_t \*)' src/pico8_msgbox.c` n'est pas vide.
- `FAILED:` si une autre struct que `mh` change de forme (`iv`, `pj`, `ke` et `ke.t` compris), ou si `pico8_mh_value_fixed` est retiré alors que `ke.t` le lit.
- `FAILED:` si la caractérisation n'a pas été lancée verte sur l'ancien code **avant** le typage (ligne du tableau « Runs » sur c296833), si elle lit un membre de `mh` autrement que par nom, ou si une de ses valeurs figées change après le typage.
- `FAILED:` si le message de commit ne contient pas le rapport `gcov` (lignes ajoutées ou modifiées de `msgbox.c` et leur compte, lignes à 0 justifiées), ou si les trois mutations ne sont pas notées avec le test qui les attrape, le trou de la forme littérale de (2) justifié.
- `FAILED:` si `test_pico8_mh` dure plus de 30 s et n'est pas dans `UNIT_TESTS_SLOW`.
- `FAILED:` si un écart port/cart découvert est corrigé au lieu d'être préservé et décrit par un `TROU-N`.
- `FAILED:` si la ligne 1b.1 de `docs/plan-3.md` ne porte pas la note « Découvert à 1b.1 » qui fixe le patron (voir Further Notes), ou si la note contredit R1 à R9 sans le dire.
- `FAILED:` s'il manque le tableau « Runs ».

## Out of Scope

- `items` (`pico8_list_t`) de `mh`, et `lq.items` : hors compte (1b.0).
- `ke.me`, `ke.t` (1b.14a, 1b.14b) ; `pico8_mh_value_fixed` reste pour `ke.t`.
- `pj.ed`, REF vers un `mh` (1b.16) ; `iv`, dans le même fichier (1b.21).
- Ajouter un écrivain à `mh`, ou réécrire `mv(self,"lh")` en appel direct.
- Régénérer `tools/member_census.txt`, ou changer l'outil de recensement.
- Réduire la taille des indicateurs ou les grouper en masque (Plan 2 §8, phase 2).
- Libérer les boîtes (TROU-7).
- Tout changement de rendu ou de comportement ; l'audio ; lancer l'appli raylib.

## Further Notes

- **Décisions de l'agent à la place de l'utilisateur** (consigne `to-prd` : vérifier modules et tests avec l'utilisateur), tranchées en `guideline` :
  - modules : `msgbox.h/.c` seuls ; aucun module nouveau, aucune aide générique d'emballage (cinq branches ne la justifient pas ; si elle devient utile, une étape suivante la crée et le note) ;
  - tests : `test_pico8_mh` étendu (caractérisation avant, forme après), pas de nouveau TU ;
  - pas de découpage : 4 fichiers modifiés.
- **Note « Découvert à 1b.1 » à écrire sur la ligne 1b.1 de `docs/plan-3.md`** (substance ; l'implémenteur l'ajuste aux faits mesurés et la garde courte, §7) : patron de 1b —
  - type C par genre (`fixe` → `pico8_fixed_t`, `bool` → `int`, `chaine` → `const char *`, `fonction` → `pico8_value_function_t`, `ref:table`/`ref:liste`/`ref:pico8_X_t` → pointeur typé non `const`), nom et place gardés ;
  - indicateur `int has_<m>` juste après `<m>`, dans le typedef qui déclare `<m>`, seulement pour un non-pointeur lu par nom à nil observé ou tenu ; NULL pour un pointeur ; 0 = nil, pas de sentinelle ;
  - init par affectation typée, indicateur et valeur posés, défaut REF gardé ;
  - lecteur : branche par membre typé, absent (NULL, `has_` 0) → 0 sans écrire, sinon emballé du genre de 1a, pointeur tel quel ;
  - écrivain **et** clés de construction : nil → NULL / `has_` 0 (assert sans indicateur), genre de la ligne → déballé, autre → assert « champ sans règle » ; exercé sur `mh` par ses clés `rh`/`lh`, `mh` n'ayant pas d'écrivain ;
  - tables d'offsets : entrée retirée partout, filles comprises, grep `offsetof` vide (vacant sur `mh`, sans table) ;
  - crochets du recensement inchangés, un membre typé sort de la table générée de lui-même ;
  - `make value-members` baisse du nombre de membres typés (270 → 265) ;
  - sites par nom inchangés, `mv(self,"lh")` compris.
- **Pour les étapes suivantes** : R5 sera exercée par un écrivain pour la première fois à 1b.15 (`bg.m_`) ou 1b.22 (`bv_et.fo`), selon l'ordre ; R7 pour la première fois à 1b.2 (`jb.ls`, si sa struct a une table) ou à la première struct à `members[]`. Ce ne sont pas des lectures nouvelles : les règles sont écrites ici.
- **Contrôle d'étape** : un sous-agent frais relit le diff contre ce PRD avant le commit (hors budget de tests).
- Le message de commit porte : le périmètre et les fichiers touchés ; les décisions (G) et leurs rejets ; R1 à R9 en résumé ; le tableau « Runs » résumé ; le compte `value-members` avant/après (270 → 265) ; le rapport `gcov` ; les trois mutations et le test qui attrape chacune ; les réponses à H1 à H12 ; la note « Découvert à 1b.1 ».
- Mesures : grep sur `src/`, `tests/`, `tools/`, `Makefile` à c296833 ; lecture de `pico8/lua.lua:5,156,173,580-595,855-899`, `src/pico8_msgbox.c/.h`, `src/pico8_entity.c` (`get_field`, `set_field`, `lt`, `he`, `mv`), `src/pico8_bootstrap.c:85-98`, `src/pico8_score_panel_display.c:64-104,290-322`, `src/pico8_struct.h`, `src/pico8_ke.h`, `src/pico8_value.h`, `src/pico8_member_census.h`, `tools/entity_value_members.sh`, `tools/member_census.sh` (P4), `tests/test_pico8_mh.c`, `tests/test_pico8_spec_defaults.c`, du PRD de 1b.0 et de celui de 1a.28.
