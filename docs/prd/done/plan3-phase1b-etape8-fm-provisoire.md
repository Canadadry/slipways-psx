---
title: "Plan 3, étape 1b.8 : `fm_placeholder` (le `fm` provisoire) — `jk`/`nz` deviennent des `pico8_fixed_t` avec `has_jk`/`has_nz`"
description: "`fm_placeholder` a deux membres `pico8_value_t` (`jk`, `nz`), lus par nom et écrits seulement par la triche sur l'écran titre, hors filet. Le recensement corrigé (1b.4b) lève leur « sans genre » : `fixe`, nil observé (`lu`). Les typer selon le patron R1–R9 de 1b.1, avec un indicateur chacun, fait passer `make value-members` de 242 à 240. Pas d'écrivain par nom : la moitié écrivain de R5 reste vacante, et ce PRD dit pourquoi."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Étapes de la phase 1b ») :

> 1b.8 — `fm_placeholder` (`probe.h:86`), **`jk`, `nz` sans genre** (écrits par la triche sur l'écran titre, hors filet). Empreinte 15 (4, 11). Compte : 240.

Cadre : les puces **Nil**, **REF**, **Recensement**, **Coexistence** et **Sortie** du §5 « Phase 1b », le §3 (`make test` vert, aucun `*.sha256` modifié), le §6 et le §7 (arrêts, ASan+UBSan, `gcov`, mutations 1b). Le **patron R1–R9** est celui de la note « Découvert à 1b.1 ». Il est cité ici, pas redécidé. S'appliquent aussi :

- « Découvert à 1b.3 » : sur une instance statique sans constructeur, R3 se lit sur l'initialiseur agrégé ;
- « à 1b.4 » : patron appliqué sans écart, R5 et R7 vacantes tant qu'aucune étape n'en a ;
- « à 1b.4b » : la table corrigée (fils forkés) fait foi. Elle lève le « sans genre » de `jk`/`nz` et exige `has_jk`/`has_nz` (voir ci-dessous) ;
- « à 1b.5 » : **forme R7 d'un accesseur `pico8_X_member`**, avec sa preuve mécanique `grep -n 'pico8_value_t \*' … | grep -v 'pico8_value_t \*out_value)'` ;
- « à 1b.6 » : le lecteur qui copie la valeur n'est pas un `pico8_X_member`, et un `fixe` suit son propre nil, pas celui d'un homonyme ;
- « à 1b.7 » : **lecture de R1 pour {bool, chaine}**, et sa lecture de R2 : un nil observé seulement par `stat?` venu d'un homonyme (ici `fm_placeholder` lui-même) ne donne pas d'indicateur. Aucune des deux ne s'applique ici. `jk`/`nz` n'ont qu'un genre (`fixe`), et leur nil est prouvé **directement** sur ce typedef (`lu`), pas par un homonyme.

### État de départ

HEAD c0a32c7. **Précondition** : 1b.7 (`hy`) est commitée avant que 1b.8 commence. Elle est en cours dans l'arbre de travail : `economy.[ch]`, `test_pico8_hy.c` et `cheat.c` (branche `hy` de `apply`, déjà typée) y sont modifiés. `make value-members` rend alors 242. Les numéros de ligne cités ici sont ceux de HEAD, plus ceux de `cheat.c` tel qu'il est dans l'arbre de 1b.7. Chaque site est désigné aussi par son contenu.

**Rien n'a été compilé ni lancé pour ce PRD.** Les faits viennent de `grep` et de la lecture du code. Tout fait d'exécution est une hypothèse (H1 à H10).

### Lignes de recensement (`tools/member_census.txt:155-156`, régénéré par 1b.4b)

| membre | accès | genres | nil | preuves-nil | par-nom |
|---|---|---|---|---|---|
| `jk` | offset | `fixe` | observé | lu,stat? | l |
| `nz` | offset | `fixe` | observé | lu,stat? | l |

- **« Sans genre » levé.** La ligne 1b.8 date de la table de 1b.0, où `jk`/`nz` n'avaient aucun genre. La note « Découvert à 1b.4b » le constate : « (1) "sans genre" levé : 1b.8 `fm_placeholder.jk`, `nz` - → `fixe` ». La cause est la suivante : les deux seuls lecteurs qui voient `jk`/`nz` écrits (`test_cheat_on_title`, `test_cheat_on_provisional`) tournent dans des fils `in_fresh_child`, que 1b.0 ne vidait pas. La moitié de la ligne qui dit « sans genre » est donc périmée. Le PRD suit la table.
- **Indicateur exigé.** La même note l'écrit : « (3) R2 exige désormais `has_<m>` (non-pointeur, lu par nom, nil tenu/observé) : 1b.8 `fm_placeholder.jk`, `nz` ». La preuve `lu` est directe : `test_pico8_fm` (`test_at_rest`, `test_new_init`, `test_cycle`, `test_replacement`) et `test_cheat_on_provisional` lisent `!has(pico8_fm(), "jk"/"nz")` sur le provisoire, et `test_pico8_jc` replie `nz` absent (`fold_frame`, `0xDEAD`) tant que `fm` est le provisoire.
- **Pas d'écrivain par nom.** La colonne par-nom vaut `l`, sans `e` : aucun `pico8_entity_set_field` n'écrit le provisoire (voir R5).
- **Caractérisation dirigée gardée.** La puce **Recensement** demande une caractérisation dirigée pour un membre sans genre. Le drapeau est levé, mais (G) on en garde une : `jk`/`nz` ne sont écrits que par la triche sur l'écran titre, que le filet ne voit jamais (1a.36 : sur les 15 rejeux, le provisoire est seulement construit puis remplacé par `eu()`). Toute la garde de l'étape repose donc sur les TU.

### Ce que fait le cart (`pico8/lua.lua`)

`fm` vaut nil avant `eu()` (`:1182`, `fm,ju=hy(),pj()`). Le provisoire est une construction du port (1a.36) : il porte ce que la triche écrit sur l'écran titre (`jk=99`, `nz=200`), que le cart écrirait sur un nil. Il n'y a rien à comparer au cart membre par membre.

### Le code aujourd'hui

- `src/pico8_probe.h:81-88` : le commentaire de contrat, puis `typedef struct { header; pico8_value_t jk; pico8_value_t nz; } pico8_fm_placeholder_t;`.
- `src/pico8_probe.c` :
  - lecteur `pico8_fm_placeholder_field` (`:25-39`) : il choisit un `const pico8_value_t *member` local par `strcmp`, puis rend 0 sur un nom inconnu ou `REF NULL`, sinon **copie** la valeur. C'est un sélecteur en ligne, pas une fonction `pico8_X_member` séparée ;
  - descripteur (`:41-45`) : aucune méthode, `set_field` NULL ;
  - initialiseur agrégé `g_fm_placeholder` (`:47-48`) : `jk`, `nz` = `{PICO8_VALUE_REF, {0}}` ;
  - `pico8_probe_init` (`:579-582`) : remise à nil (`jk`, `nz` = `pico8_value_ref(NULL)`), puis rangement dans `g_fm` et `PICO8_MEMBER_CENSUS_SEE(&g_fm_placeholder, pico8_fm_placeholder_t)` ;
  - `fm_free` (`:586-593`) ne libère jamais le provisoire. Il ne change pas.
- Écriture directe, **la seule** : la branche provisoire de `apply`, dans `src/pico8_cheat.c:29-34` (`provisional->jk = pico8_value_fixed(pico8_fixed_from_int(99))`, idem `nz` = 200). Sa branche `hy` (`:35-37`) est déjà typée par 1b.7.
- Lecteurs par nom (**inchangés**) : `test_pico8_fm` (tout le fichier), `test_pico8_hy` (`test_cheat_on_provisional`), `test_pico8_jc` (`fold_frame`, `nz`). Aucune lecture dans `src/`.
- Écrivain générique : aucun. `must_abort(write_jk)` (`test_pico8_fm`, `test_fm_is_a_struct`) fige l'`assert` de `pico8_entity_set_field` (`entity.c:197`, « entity: a struct a generic writer reaches needs a field writer »).

### Empreinte remesurée

**6 (3, 3)** : `probe.h`, `probe.c`, `cheat.c` ; `test_pico8_fm`, `test_pico8_hy`, `test_pico8_jc`. L'écart avec 15 (4, 11) vient des homonymes : le grep de la liste rattache à `fm_placeholder` tous les fichiers qui lisent `jk`/`nz` d'un `hy` (`economy.c`, `score_panel.c`, `tech_gate.c`, `cheat_code`, `playability`, les tests `set_credits`…).

**Fichiers modifiés** : `src/pico8_probe.h`, `src/pico8_probe.c`, `src/pico8_cheat.c` (2 lignes deviennent 4), `tests/test_pico8_fm.c` (caractérisation seule), et `docs/plan-3.md`.

## Solution

1. **Caractérisation d'abord, figée sur l'ancien code** (voir Testing Decisions).
2. **Typage** :

   ```c
   pico8_fixed_t jk;
   int has_jk; /* 0: `jk` is nil, and is then never read */
   pico8_fixed_t nz;
   int has_nz; /* 0: `nz` is nil, and is then never read */
   ```

   Le sélecteur local `member` du lecteur disparaît.
3. Effet mesurable : `make value-members` passe de 242 à **240** et ne liste plus aucune ligne `pico8_fm_placeholder_t`. Aucun pixel ne bouge.

## User Stories

1. En tant que lecteur de `probe.h`, je veux `jk`/`nz` du provisoire typés `pico8_fixed_t`, afin que leur genre se lise sur le type.
2. En tant que `test_pico8_fm`, je veux `jk`/`nz` absents au repos, `99`/`200` en fixe après la triche, absents de nouveau après chaque `pico8_probe_init`, comme en 1a.
3. En tant que `test_cheat_on_provisional`, je veux les mêmes lectures, puis `0`/`100` du `hy` après `eu()`.
4. En tant que `test_pico8_jc`, je veux `nz` absent sur le provisoire, afin que mes empreintes figées ne bougent pas.
5. En tant que la triche (`apply`), je veux écrire `jk`/`nz` sur le provisoire par affectation typée, présence comprise, dans le même ordre.
6. En tant que `pico8_probe_init`, je veux remettre le provisoire à nil en éteignant les deux indicateurs.
7. En tant que `test_fm_is_a_struct`, je veux que l'écriture par nom sur le provisoire avorte toujours.
8. En tant que gardien du §3, je veux `make test` vert, 15 « conforme » et aucun `*.sha256` touché.
9. En tant que gardien de la mémoire, je veux `make sanitize-net` propre sur les 15 rejeux.
10. En tant que `plan-pilot`, je veux `make value-members` à 240.
11. En tant qu'auteur de 1b.13 (`jc_too_far.fo`, première struct de la liste qui a un `set_field` qui écrit un membre recensé), je veux savoir que 1b.8 **ne** fixe **pas** la moitié écrivain de R5, et ce qu'elle laisse à fixer.
12. En tant que vérificateur, je veux une caractérisation par nom lancée sur l'ancien code, les mutations du §7 déclinées sur le provisoire, et la justification de chaque trou.

## Implementation Decisions

Pas d'humain dans la boucle. Le skill `to-prd` demande de faire valider modules et tests par l'utilisateur. Ces choix sont tranchés ici comme le ferait un agent `guideline`, et marqués (G).

### R5, moitié écrivain : 1b.8 ne l'exerce pas

La consigne de rédaction supposait que `fm_placeholder` avait un écrivain par nom pour `jk`/`nz`, ce qui aurait fait de 1b.8 la première étape à exercer la moitié écrivain de R5. **Ce n'est pas le cas**, et le PRD ne l'invente pas :

- la note « Découvert à 1a.36 » dit « sans méthode **ni écrivain** ». L'écrivain dont elle parle est `apply`, une **écriture directe** (R6), pas un `set_field` (R5) ;
- `g_fm_placeholder_struct_class.set_field` vaut NULL (`probe.c:44`), et `test_fm_is_a_struct` le fige (`cls->set_field == NULL`, `must_abort(write_jk)`) ;
- le recensement dit `l`, sans `e` ;
- 1a.36 a **rejeté** un `set_field` pour la seule triche (« ouvrirait à tout écrivain générique futur une porte que le §5 réserve aux écrivains génériques mesurés »).

(G) Ajouter un `set_field` pour « exercer R5 » serait un écrivain non mesuré : c'est un critère d'échec ci-dessous. La moitié écrivain de R5 reste vacante. **Première étape de la liste qui l'exerce : 1b.13** (`jc_too_far.fo`, recensement `le`, preuve `écrit`, `pico8_jc_too_far_set_field` dans `slipway.c`), puis 1b.15 (`bg.m_`, `le`).

Les quatre points que la consigne demandait de fixer ont, sur le provisoire, une réponse **hors R5**. On l'écrit pour que 1b.13 ne la prenne pas pour une forme de l'écrivain :

| point demandé | sur `fm_placeholder` (1b.8) | ce qui reste à fixer par 1b.13 |
|---|---|---|
| déballage par genre | aucun : `apply` écrit des `pico8_fixed_t` déjà typés (R6), sans `pico8_value_t` à ouvrir | `set_field` : `FIXED` → `<m> = value.as.fixed` (R5 de 1b.1), autre genre → `assert` |
| nil éteint la présence | seul `pico8_probe_init` remet à nil, par affectation typée (R3) : `<m> = 0; has_<m> = 0;`. Aucune écriture de nil par nom | `set_field` sur `REF NULL` → NULL ou `has_<m> = 0` |
| `assert` « champ sans règle » | l'`assert` existant de `pico8_entity_set_field` sur `set_field` NULL (`entity.c:197`), déjà figé par `must_abort(write_jk)`. Aucun `assert` nouveau | l'`assert` de genre dans le `set_field`, qui nomme le membre |
| lien avec la boucle de clés de 1b.5 | aucun : pas de constructeur, donc pas de clé | même déballage, écrit une fois pour les deux, ou deux fois (à trancher) |

### R1–R9 appliquées à `fm_placeholder` (patron de 1b.1, cité)

| règle | sur `fm_placeholder` |
|---|---|
| R1 | `jk`, `nz` → `pico8_fixed_t` (`fixe`). Noms et ordre gardés, un membre par ligne. Ni la lecture {fixe, bool} de 1b.5 ni la lecture {bool, chaine} de 1b.7 ne s'appliquent : un seul genre. |
| R2 | `int has_jk` juste après `jk`, `int has_nz` juste après `nz`. Ce sont des non-pointeurs, lus par nom (`l`), à nil observé par preuve **directe** (`lu`, voir les lignes de recensement). La lecture de R2 de 1b.7 (pas d'indicateur pour un nil qui ne vient que d'un homonyme) ne s'applique pas ici : c'est ce typedef qui fournissait la preuve à `hy`. |
| R3 | **Initialiseur agrégé** (note 1b.3) : `{ header, 0, 0, 0, 0 }`, dans l'ordre `jk`, `has_jk`, `nz`, `has_nz`. **Remise à nil de `pico8_probe_init`**, à la même place : `g_fm_placeholder.jk = 0; g_fm_placeholder.has_jk = 0;`, puis idem pour `nz`. Valeur et indicateur sont posés tous deux, comme R3 le veut. Le rangement dans `g_fm` et `PICO8_MEMBER_CENSUS_SEE` restent après. |
| R4 | Une branche `strcmp` par membre, dans l'ordre du typedef : `has_<m> == 0` → `return 0`, sinon `*out_value = pico8_value_fixed(fm-><m>); return 1;`. Tout autre nom → `return 0`, sans `assert` (décision de 1a.36 gardée). Dans tous les `return 0`, `*out_value` reste intact. |
| R5 | **Vacante, les deux moitiés.** Écrivain : voir ci-dessus. Boucle de clés : pas de constructeur. |
| R6 | `cheat.c`, branche provisoire : `provisional->jk = pico8_fixed_from_int(99); provisional->has_jk = 1;`, puis `provisional->nz = pico8_fixed_from_int(200); provisional->has_nz = 1;`. `jk` reste avant `nz`. Aucune autre écriture directe : le compilateur le confirme. |
| R7 | **Forme `pico8_X_member` de 1b.5, appliquée à un sélecteur en ligne** (G). Le `const pico8_value_t *member` local du lecteur joue le rôle de l'accesseur. Ses deux branches sont retirées, et le lecteur prend les branches R4 explicites. Sans branche restante, le sélecteur est supprimé. Preuves : (1) `grep -n 'pico8_value_t \*' src/pico8_probe.c \| grep -v 'pico8_value_t \*out_value)' \| grep -v 'g_g__members\|g_g__defaults\|keys\[found\]'` est **vide** après. Avant, il rend la ligne du `member` local. Les trois lignes exclues sont les tables d'offsets de `g_` (1b.32). (2) `grep -rn 'offsetof(pico8_fm_placeholder_t' src/` est vide (déjà vide). (3) `grep -rnE '(->\|\.)(jk\|nz)\.(kind\|as)' src/ tests/` est vide. (4) `grep -rnE '(jk\|nz) = pico8_value_' src/ tests/` est vide. Après 1b.7 et 1b.8, `hy` et `fm_placeholder`, seuls typedefs à porter `jk`/`nz` (recensement), sont typés : ces deux greps n'ont plus besoin de la restriction de fichiers de 1b.7. |
| R8 | `PICO8_MEMBER_CENSUS_SEE(&g_fm_placeholder, pico8_fm_placeholder_t)` est gardé. `tools/member_census.txt` n'est pas édité. |
| R9 | 242 → 240, aucune ligne `pico8_fm_placeholder_t`. `has_jk` et `has_nz` ne comptent pas. |

- Aucun site par nom ne change, et aucun n'est remplacé par un accès direct.
- **Commentaires** (règle de `CLAUDE.md`) :
  - le contrat de `has_jk`/`has_nz` s'écrit au format de `trade.h`, comme dans la Solution ;
  - le commentaire du typedef (« one static struct, never allocated nor freed, with no method and no writer; `jk`/`nz` are written only by the cheat code, and set back to nil by every `pico8_probe_init()` ») reste exact ;
  - celui de `pico8_fm()` et celui qui précède `apply` restent exacts ;
  - aucun autre commentaire.
- `#include "pico8_value.h"` est retiré de `cheat.c` si plus rien n'y sert (après 1b.7 et 1b.8, `apply` n'emballe plus aucune valeur). `probe.h` le garde : `g_`, `bv_et` et les prototypes l'utilisent.

### Hypothèses (rien n'a été lancé)

- **H1** : `make value-members` rend 242 sur l'arbre de départ (1b.7 commitée), puis 240.
- **H2** : aucun `pico8_entity_set_field` n'atteint le provisoire, hors `must_abort(write_jk)` (recensement `l` sans `e`, mesure de 1a.36).
- **H3** : les seuls écrivains de `jk`/`nz` du provisoire sont l'initialiseur agrégé, la remise à nil de `pico8_probe_init` et la branche provisoire de `apply`. Chacun écrit **les deux** membres ensemble, si bien que `has_jk == has_nz` dans tout état atteignable. Cela justifie le trou de la mutation 5.
- **H4** : sur les 15 rejeux du filet, le provisoire n'est ni lu ni écrit (mesure de 1a.36). Les empreintes et les `*.sha256` ne bougent pas.
- **H5** : les empreintes figées de `test_pico8_jc` ne bougent pas : `nz` reste absent sur le provisoire, donc `get` rend `REF NULL` et `fold_value` replie `0xDEAD`, comme avant.
- **H6** : les frames du rejeu de `test_pico8_jc` passent par au moins une frame où `fm` est le provisoire, ce qui fait rougir ses empreintes sous la mutation 2. Si ce n'est pas le cas, la mutation 2 n'est rougie que par `test_pico8_fm`, `test_cheat_on_provisional` et la caractérisation. C'est suffisant, et la réponse est notée dans le commit.
- **H7** : `test_pico8_fm` (dans `UNIT_TESTS`, hors `UNIT_TESTS_SLOW`) reste sous 30 s avec la caractérisation (~1 s attendu, comme à 1a.36).
- **H8** : `PICO8_MEMBER_CENSUS_SEE` compile sur un typedef sans plus aucun `pico8_value_t` (comme `mh`, `ha_pz`, `pj_cr`). Le build `-DPICO8_MEMBER_CENSUS` n'est pas lancé.
- **H9** : sous ASan+UBSan, rien de nouveau (`detect_leaks=0`, provisoire statique jamais libéré).
- **H10** : `test_pico8_cheat_code` ne passe pas par la branche provisoire de `apply`, puisqu'il ne rejoue pas `--from-title` (1a.36). Son diff reste vide, et il ne sert pas de preuve pour cette branche.

## Testing Decisions

- **Un bon test juge ce qu'on lit par nom ou à l'écran.** La caractérisation ne fait aucun accès `->jk/nz/has_*`, lit par `cls->field` et `pico8_entity_get_field`, et compile sur les deux formes.
- **Caractérisation dirigée, figée sur l'ancien code** : `test_fm_members_by_name`, dans `test_pico8_fm`, lancée par `in_fresh_child` après `test_fm_is_a_struct`. Pas de nouveau TU. Elle réutilise `type_cheat` et `others`. Le témoin est un fixe `0x1234` posé dans `out` avant chaque lecture attendue absente. Elle lit par le **lecteur brut** (`pico8_fm_placeholder_struct_class()->field`), et non par le `lookup` du fichier, qui filtre `REF NULL` : c'est ce qui rend visible un « présent nil » ou un « présent 0 ».
  1. **Au repos** (`pico8__init()`) : pour `jk`, `nz`, chaque nom de `others`, `""` et `"x"`, le lecteur rend 0 et `out` reste égal au témoin. `pico8_entity_get_field` rend 0 lui aussi.
  2. **Après `type_cheat()`** : `jk` rend 1, genre `FIXED`, `pico8_fixed_from_int(99)`. `nz` rend 1, `FIXED`, `pico8_fixed_from_int(200)`. Chaque autre nom rend 0, `out` intact.
  3. **Nouvel init** : `pico8_probe_init()` donne le même pointeur `pico8_fm()`. `jk`/`nz` rendent 0, `out` intact.
  4. **Retriche après init** : `type_cheat()` rend 1. `jk`/`nz` relisent 99/200 (l'indicateur se rallume après avoir été éteint).
  5. **Shutdown puis init** : `pico8_probe_shutdown()`, `type_cheat()` (sans effet, `fm` NULL), puis `pico8_probe_init()` : `jk`/`nz` rendent 0, `out` intact.

  Elle est lancée verte sur l'arbre de départ **avant** le typage. Ses valeurs ne changent pas ensuite. Le diff de `test_pico8_fm.c` est un pur ajout.
- **Tests existants qui figent déjà beaucoup**, inchangés : les six cas de `test_pico8_fm`, `test_cheat_on_provisional` et `test_hy_is_a_struct` (`test_pico8_hy`), les empreintes de `test_pico8_jc`.
- **Tests de régression (§6)** : aucun n'est touché. Les huit de 1a.35 lisent par nom, et leur diff est vide. `cheat_code` n'atteint pas le provisoire (H10). Il n'y a donc pas de preuve rouge à refaire.
- **Mutations (§7)**, dans une copie jetable du scratchpad, chacune lancée par `make tmp/test_pico8_fm && ./tmp/test_pico8_fm`, puis `make tmp/test_pico8_hy && ./tmp/test_pico8_hy` et `make tmp/test_pico8_jc && ./tmp/test_pico8_jc` quand ils sont cités. Chacune est notée dans le commit avec le test qui la rougit, ou la justification de son trou.
  1. (§7-1) *indicateur forcé vrai* : le lecteur ignore `has_jk`. Au repos, `jk` se lit `FIXED 0` présent. Attendu rouge : caractérisation 1, 3 et 5, `test_at_rest`, `test_new_init`, `test_cycle`, `test_replacement` (après son init final), `test_cheat_on_provisional` (`!has(provisional,"jk")`).
  2. (§7-2) *« présent 0 » là où c'était absent*. Il n'y a pas d'écrivain : la branche R5 `REF NULL` n'existe pas. Sa forme sur le provisoire est l'endroit où nil est posé : la remise à nil de `pico8_probe_init` pose `has_nz = 1` (avec `nz = 0`). Attendu rouge : caractérisation 1, 3 et 5, `test_at_rest`, `test_cheat_on_provisional`, et les empreintes de `test_pico8_jc` si H6 tient.
  3. (§7-3) *init qui saute un membre typé*. Il n'y a pas de boucle d'init. Sa forme : la remise à nil de `pico8_probe_init` ne remet pas `has_jk` à 0 (elle pose seulement `jk = 0`). Après une triche puis un nouvel init, `jk` se lit `FIXED 0` présent. Attendu rouge : caractérisation 3, `test_new_init`, `test_replacement`.
  4. Propre à l'étape, *présence oubliée dans R6* : la branche provisoire de `apply` pose `nz = 200` sans `has_nz = 1`. Attendu rouge : caractérisation 2 et 4, `test_cheat_on_title`, `test_cheat_on_provisional`.
  5. Propre à l'étape, *indicateurs croisés* : la branche `nz` du lecteur teste `has_jk`. **Trou attendu, justifié par H3** : les deux indicateurs sont posés et éteints ensemble par chacun des trois écrivains. Aucun état atteignable, ni par le filet, ni par un TU, ni par le code de `src/`, ne les distingue. Un test qui les séparerait devrait écrire un membre directement, ce que la caractérisation s'interdit. Le commit note la mutation, l'absence de rouge, et cette justification. **Si elle rougit un test, H3 est fausse** : il faut chercher l'écrivain manquant avant de committer.
- **`gcov`** : `test_pico8_fm` et `test_pico8_hy` en `--coverage -O0`, avec le piège `__gcov_dump` des fils de 1a.23, puis `gcov` sur `pico8_probe.c` et `pico8_cheat.c`. Chaque ligne ajoutée ou modifiée doit être exécutée : les deux branches du lecteur, présent comme absent, le `return 0` final, la remise à nil de `pico8_probe_init` et les quatre lignes de la branche provisoire de `apply`. L'initialiseur agrégé n'est pas une ligne exécutable : les mutations 1 et 2 prouvent sa valeur au repos. **Aucun autre trou n'est attendu.** Le rapport va dans le commit.
- **Prior art** : `test_pico8_fm` (caractérisation et forme de 1a.36), `test_cheat_on_provisional` (`test_pico8_hy`), `test_trade_members_by_name` (1b.5, lecteur brut et témoin `0x1234`), `test_cr_members_by_name` (1b.6, instance statique).

### Vérification, budget et tableau « Runs »

**Budget strict : environ 20 minutes de temps de tests au total.** Au-delà, l'implémenteur s'arrête et note ce qui manque au lieu de relancer. Ordres de grandeur :

- `make test` : 20 s à 1 min ;
- `make e2e` : 4 à 6 min ;
- `make sanitize-net` : ~4 min seul, le double sous charge ; on ne le lance **jamais** en même temps que `e2e` ;
- `make value-members` : quelques secondes.

Mutations et caractérisation passent par un binaire seul (`make tmp/test_pico8_<nom>`), jamais par `make test`.

Le journal tient un tableau « Runs », une ligne par lancement. Le vérificateur ne relance que ce qui suit le dernier changement de l'arbre.

| # | Commande exacte | État de l'arbre | Code de sortie | Ligne clé | Durée |
|---|---|---|---|---|---|

**Cibles finales**, vertes sur l'arbre final :

1. `timeout 10m make test` ;
2. `timeout 15m make e2e` : **15 « conforme »**, et les `UNIT_TESTS_SLOW` passent ;
3. `timeout 15m make sanitize-net` : **`sanitize: 15 propre(s)`** ;
4. `make value-members` : **`membres pico8_value_t : 240`**, sans aucune ligne `pico8_fm_placeholder_t`.

`make class-coverage-net` **ne s'applique pas** (§7) : **aucun constructeur ne change**. `fm_placeholder` n'a pas de constructeur. Le diff touche un initialiseur statique, deux lignes de `pico8_probe_init` (init de module, sans `pico8_class_coverage_record` pour `fm`), un lecteur et une branche de `apply`. Le seul `pico8_class_coverage_record` de `probe.c` (`"g_"`, dans `pico8_g__struct_new`) et `KNOWN[]` ne bougent pas. Si le diff en touche un, la cible redevient obligatoire, avec une sortie identique à `tools/class_coverage_net_expected.txt`. `make member-census` ne s'applique pas (R8). Jamais l'appli raylib.

### Critères d'échec (le vérificateur de `prd-autopilot` les traduit en `FAILED:`)

- `FAILED:` si le filet (`make test` ou `make e2e`) reste rouge après **3 tentatives** sur ce commit.
- `FAILED:` si un `*.sha256` est modifié ou supprimé.
- `FAILED:` si le diff ou le commit touche `FRAMES_EVENTS`, `RECORDINGS` ou un fichier d'événements (`tests/fixtures/*events*.json`, `cheat_code_events.json` compris). Les changements non commités de l'utilisateur sur ces fichiers ne sont ni modifiés, ni indexés, ni commités.
- `FAILED:` (« champ sans règle ») si :
  - un membre est ajouté à `fm_placeholder`, sauf `has_jk` et `has_nz` ;
  - un `set_field`, une méthode ou un constructeur est ajouté au provisoire ;
  - l'`assert` de `pico8_entity_set_field` tire sur le provisoire dans un rejeu ou un test, hors `must_abort(write_jk)` (H2) ;
  - le lecteur accepte un autre nom que `jk`/`nz`.
- `FAILED:` si `jk`/`nz` ne sont pas `pico8_fixed_t`, ou si `has_jk`/`has_nz` ne sont pas des `int` déclarés juste après leur membre.
- `FAILED:` si le lecteur ne rend pas 0 avec `*out_value` intact sur un membre nil ou un nom inconnu, ou lit `jk`/`nz` présents au repos ou après un `pico8_probe_init`.
- `FAILED:` si `pico8_probe_init` ne pose pas valeur **et** indicateur des deux membres, ou si la branche provisoire de `apply` n'allume pas les deux indicateurs.
- `FAILED:` si une preuve de R7 n'est pas vide, ou si le sélecteur local `member` du lecteur survit.
- `FAILED:` si le message de l'`assert` de `pico8_entity_set_field` change, ou si `fm_free`, `pico8_fm_hy` ou `pico8_probe_set_fm` changent.
- `FAILED:` si le diff d'un test fait autre chose qu'ajouter la caractérisation à `test_pico8_fm.c`, si une lecture par nom change, ou si une valeur figée existante change (`test_pico8_fm`, `test_cheat_on_provisional`, empreintes de `test_pico8_jc`).
- `FAILED:` si un test de régression de bug (§6) a un diff non vide.
- `FAILED:` si un écart port/cart est corrigé (par exemple `fm` rendu NULL avant `eu()`, comme dans le cart) au lieu d'être préservé.
- `FAILED:` si `make sanitize-net` n'est pas `15 propre(s)` sur l'arbre final.
- `FAILED:` si `make value-members` ne rend pas 240, liste encore `pico8_fm_placeholder_t`, ou si un autre typedef gagne ou perd une ligne.
- `FAILED:` si le diff touche un constructeur, `pico8_class_coverage_record` ou `KNOWN[]` sans une ligne « Runs » de `make class-coverage-net` identique à la référence.
- `FAILED:` si un crochet du recensement change, si `tools/member_census.txt` est édité à la main, ou si `tools/entity_value_members.sh` change.
- `FAILED:` si la caractérisation n'a pas de ligne « Runs » verte sur l'arbre de départ avant le typage, si elle lit un membre directement, ou si ses valeurs changent après le typage.
- `FAILED:` si le commit ne contient pas le rapport `gcov` et les 5 mutations, chacune avec le test qui la rougit ou, pour la 5, la justification de son trou. Également si la mutation 5 rougit un test sans que l'écrivain manquant ait été trouvé et noté.
- `FAILED:` s'il manque la note « Découvert à 1b.8 », ou si elle omet le constat « R5 écrivain toujours vacante, première candidate 1b.13 ».
- `FAILED:` s'il manque le tableau « Runs ».

## Out of Scope

- Donner une forme à la moitié écrivain de R5 (1b.13), ou à R7 sous sa forme `offsetof` (`g_`, 1b.32).
- Le type du slot `g_fm` (`pico8_struct_header_t *`) et `pico8_fm()`.
- `hy.jk`/`hy.nz` (1b.7, déjà typés à la précondition).
- Rendre `fm` NULL avant `eu()` comme dans le cart.
- Régénérer `tools/member_census.txt` (R8).
- Les fichiers d'événements et `FRAMES_EVENTS`.
- Tout changement de rendu ; l'audio ; lancer l'appli raylib.

## Further Notes

- **Décisions à la place de l'utilisateur** (G) :
  - modules : `probe.c/.h`, plus la branche provisoire de `apply` dans `cheat.c` ;
  - tests : `test_pico8_fm` étendu d'une caractérisation dirigée au lecteur brut, sans nouveau TU, bien que le « sans genre » soit levé (écriture hors filet) ;
  - un commit, sans découpage (deux membres) ;
  - `has_jk` et `has_nz`, sur preuve `lu` directe ;
  - aucun `set_field` ajouté : R5 vacante, les deux moitiés ;
  - R7 `pico8_X_member` lue sur le sélecteur local du lecteur ;
  - `class-coverage-net` non lancé, parce qu'aucun constructeur ne change.
- **Note « Découvert à 1b.8 »**, à écrire sur la ligne 1b.8 de `docs/plan-3.md`. Elle ne garde que ce qui change une étape suivante :
  - « "sans genre" de la ligne levé par 1b.4b (`fixe`, nil observé `lu`) ; caractérisation dirigée gardée, l'écriture étant hors filet » ;
  - « R7 `pico8_X_member` s'applique aussi à un sélecteur `pico8_value_t *` local au lecteur » ;
  - « **R5 écrivain toujours vacante** : le seul écrivain du provisoire est `apply`, une écriture directe (R6), et nil n'y est posé que par la remise à zéro de l'init (R3) ; première candidate : 1b.13 (`jc_too_far.fo`), puis 1b.15 (`bg.m_`) ».

  Le reste va dans le commit.
- Le message de commit porte :
  - le périmètre ;
  - le compte 242 → 240 ;
  - le résumé du tableau « Runs » ;
  - le rapport `gcov` ;
  - les 5 mutations, dont le trou justifié de la 5 ;
  - les réponses à H1 à H10.
- Un sous-agent frais relit le diff contre ce PRD avant le commit.
