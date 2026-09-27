---
title: "Plan 3, étape 1b.4 : `ha_pz` (enveloppe de `pz` sur `g_ha`) — `hi`/`gq` deviennent des `pico8_vec_t *`, `ca` un `pico8_fixed_t` avec `has_ca`"
description: "`ha_pz` a trois membres `pico8_value_t` lus par nom (`hi`, `gq` : `ref:autre` ; `ca` : `fixe`), tous nil après `pico8_menu_shutdown` ; les typer selon le patron R1–R9 de 1b.1 fait passer `make value-members` de 263 à 260 sans qu'un pixel du filet ne bouge. Pas d'écrivain par nom et pas de table d'offsets : R5 et R7 restent vacantes."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Étapes de la phase 1b ») :

> - 1b.4 — `ha_pz` (`menu.c:34`), accès par nom seul (typedef d'un `.c`, pas de table d'offsets). Empreinte 4 (2, 2). Compte : 260.

Cadre : les puces **Nil**, **REF**, **Coexistence** et **Sortie** du §5 « Phase 1b », le §3 (`make test` vert, aucun `*.sha256` modifié) et le §7 (arrêts, ASan+UBSan, `gcov`, mutations 1b). Le **patron R1–R9** est celui de la note « Découvert à 1b.1 » de `docs/plan-3.md`. Il est cité ici et n'est pas redécidé. La note « Découvert à 1b.3 » s'applique aussi : sur une instance statique sans constructeur, R3 se lit sur l'initialiseur agrégé.

### État de départ

HEAD 7090dde, avec 1b.3 (`mz_jj`) dans l'arbre de travail : elle est en cours d'implémentation. **Précondition** : 1b.3 est commitée avant que 1b.4 commence. `make value-members` rend alors 263.

**Rien n'a été compilé ni lancé pour ce PRD.** Les faits viennent de `grep` et de la lecture du code. Tout fait d'exécution est une hypothèse (H1 à H8).

### Lignes de recensement (`tools/member_census.txt:50-52`)

| membre | accès | genres | nil | preuves-nil | par-nom |
|---|---|---|---|---|---|
| `hi` | nom | `ref:autre` | observé | stat? | l |
| `gq` | nom | `ref:autre` | observé | stat? | l |
| `ca` | nom | `fixe` | observé | stat? | l |

Les preuves `stat?` sont partagées avec d'autres typedefs (`fv`, `jo`, `pj_cr`). La preuve directe de nil est un test : `test_ha_pz_is_a_struct` lit `!has(o, "hi"/"gq"/"ca")` après `pico8_menu_shutdown()`, sur un pointeur gardé (`tests/test_pico8_js.c:1139-1141`).

### Ce que fait le cart (`pico8/lua.lua:986-997`)

`ha=ob([[… hi=v(0,107),gq=v(0,-11), ca=14,]])`, puis `ix[4].my` fait `pz(ha,ha)`. `pz` lit `hi` (sinon `ra`), `gq` et `ca` sur `p`, et ne les écrit jamais.

### Le code aujourd'hui (`src/pico8_menu.c`)

- Typedef local au `.c` (`:32-37`) : `{ header; pico8_value_t hi, gq, ca; }`. Aucun `offsetof`, pas de fille.
- **Instance statique unique** `g_ha_pz` (`:67-72`), pas de constructeur. Initialiseur : trois `REF NULL`.
- **Lecteur** `pico8_ha_pz_field` (`:39-57`) : trois `strcmp`, puis un test commun « `REF NULL` → `return 0` ».
- **Écrivain NULL** (`:59-63`). Toute écriture par nom avorte, ce qui est figé par `must_abort(write_hi/write_ca)` dans `test_ha_pz_is_a_struct`. **Il n'y a pas d'écrivain par nom : R5 n'est pas exercée ici.**
- **Init** (`pico8_menu_init`, `:490-499`) : `pico8_ob_table_get_name(g_ha, "hi"/"gq"/"ca", &g_ha_pz.<m>)`, puis un `assert` qui exige `hi` et `gq` REF non NULL et `ca` fixe. `PICO8_MEMBER_CENSUS_SEE(&g_ha_pz, pico8_ha_pz_t)` (`:518`).
- **Shutdown** (`:526-528`) : remet les trois membres à `REF NULL`, puis libère `g_ha`.
- La cible de `hi`/`gq` est un `pico8_vec_t` que le constructeur `v` de l'analyseur alloue sans tag (`pico8_pi.c:121-130`). D'où le genre `ref:autre`. Le pointeur appartient à `g_ha`, et `g_ha_pz` l'emprunte.
- Lecteurs par nom : `pico8_pz`/`pico8_pz_step` (`msgbox.c:454,458,511`) et `test_pico8_js.c` (`same_as_ha`, `has`). Ils ne changent pas.

### Empreinte remesurée

4, comme la ligne :
- src : `pico8_menu.c` et `pico8_msgbox.c` ;
- tests : `test_pico8_js.c` et `test_pico8_pause_menu_music.c`. Ce dernier est un majorant : il lit la table `g_ha`, pas la struct.

Fichiers modifiés : `src/pico8_menu.c`, `tests/test_pico8_js.c` (caractérisation seule) et `docs/plan-3.md`. `pico8_menu.h` ne change pas.

## Solution

1. **Caractérisation d'abord, figée sur l'ancien code** (voir Testing Decisions).
2. **Typage** :
   - `hi` et `gq` deviennent `pico8_vec_t *`. Ce sont des pointeurs : pas d'indicateur, NULL = nil.
   - `ca` devient `pico8_fixed_t`, avec `int has_ca` juste après.
   - Le lecteur rend `pico8_value_ref(hi/gq)` (le même pointeur) ou `pico8_value_fixed(ca)`, et `return 0` sur NULL ou `has_ca == 0`.
3. Effet mesurable : `make value-members` passe de 263 à **260** et ne liste plus aucune ligne `pico8_ha_pz_t`. Rien d'autre ne bouge.

## User Stories

1. En tant que lecteur de `menu.c`, je veux `hi`/`gq` typés `pico8_vec_t *` et `ca` typé `pico8_fixed_t`, afin que l'enveloppe se lise sur ses types.
2. En tant que `pz`, je veux lire par nom `hi`, `gq` et `ca` avec les mêmes genres, les mêmes valeurs et **les mêmes pointeurs** que la table `g_ha`, afin que le menu pause se pose au même endroit.
3. En tant que `test_ha_pz_is_a_struct`, je veux que les trois membres soient absents après `shutdown` et de nouveau égaux à ceux de `g_ha` après une nouvelle `init`.
4. En tant que `test_ha_pz_is_a_struct`, je veux qu'une écriture par nom avorte toujours, afin que l'écrivain NULL soit gardé.
5. En tant que gardien du §3, je veux `make test` vert, 15 « conforme » et aucun `*.sha256` touché.
6. En tant que gardien de la mémoire, je veux `make sanitize-net` propre sur les 15 rejeux.
7. En tant que `plan-pilot`, je veux `make value-members` à 260.
8. En tant qu'auteur de l'étape suivante, je veux savoir comment `ref:autre` vers un vecteur a été tranché, et que R5 et R7 n'ont toujours pas de forme concrète.
9. En tant que vérificateur, je veux une caractérisation par nom, lancée sur l'ancien code, et les mutations du §7 déclinées sur `ha_pz`.

## Implementation Decisions

Pas d'humain dans la boucle. Le skill `to-prd` demande de faire valider modules et tests par l'utilisateur. Ces choix sont tranchés ici comme le ferait un agent `guideline`, et marqués (G).

### R1–R9 appliquées à `ha_pz` (patron de 1b.1, cité)

| règle | sur `ha_pz` |
|---|---|
| R1 | `hi`, `gq` → `pico8_vec_t *`. (G) `ref:autre` est tranché par la cible : un `pico8_vec_t` alloué par `v(…)`. `pico8_vec_t` est le type que le §5 nomme pour les vecteurs. Le pointeur n'est pas `const` (R1). `ca` → `pico8_fixed_t`. Noms et ordre gardés. |
| R2 | `hi`, `gq` : pas d'indicateur (pointeurs). `ca` : `int has_ca` juste après `ca`. C'est un non-pointeur lu par nom, à nil observé, et un lecteur le voit nil (`test_ha_pz_is_a_struct` après `shutdown`). Aucune preuve contraire n'est possible. |
| R3 | (1b.3) L'initialiseur statique devient `{ {STRUCT, &g_ha_pz_struct_class}, NULL, NULL, 0, 0 }`. Dans `pico8_menu_init`, la lecture se fait dans des locales `pico8_value_t`, **avant** les `ix[i].my`, et l'`assert` existant est gardé tel quel (même message). Ensuite : `hi = (pico8_vec_t *)hi_v.as.ref`, `gq = (pico8_vec_t *)gq_v.as.ref`, `ca = ca_v.as.fixed`, `has_ca = 1`. Même REF qu'avant, pas de copie. |
| R4 | Une branche par membre. `hi`/`gq` NULL → `return 0`. `has_ca == 0` → `return 0`. Dans ces cas, `*out_value` reste intact. Sinon, `pico8_value_ref(o->hi/gq)` ou `pico8_value_fixed(o->ca)`, puis `return 1`. Tout autre nom rend 0. Le test commun `REF NULL` disparaît. |
| R5 | **Vacante.** Il n'y a pas d'écrivain par nom (`set_field` NULL) ni de clé de construction. La copie de l'init depuis `g_ha` n'est pas une boucle de clés : c'est trois lectures nommées fixes, gardées par l'`assert` de 1a.41, et elle relève de R3. (G) La forme concrète de R5 n'est pas écrite ici, faute de site pour l'exercer. |
| R6 | `pico8_menu_shutdown` écrit nil : `hi = NULL`, `gq = NULL`, `ca = 0`, `has_ca = 0`. Aucune méthode. |
| R7 | **Vacante.** Le lecteur fait des `strcmp` sans `offsetof`. La preuve `grep -n 'offsetof(pico8_ha_pz_t' src/` est vide avant comme après. |
| R8 | `PICO8_MEMBER_CENSUS_SEE(&g_ha_pz, pico8_ha_pz_t)` est gardé à sa place. `tools/member_census.txt` n'est pas édité. |
| R9 | 263 → 260, aucune ligne `pico8_ha_pz_t`. |

- Aucun site par nom ne change : `pz`, `pz_step` et les lectures de `test_pico8_js` restent tels quels. Le cast `(const pico8_vec_t *)hi_val.as.ref` de `pz` reste valide.
- (G) La cible `pico8_vec_t` n'a pas de tag : on ne peut pas y mettre d'`assert` de tag. L'`assert` « REF non NULL » de l'init en tient lieu.
- Le commentaire de `g_ha_pz` (« never allocated nor freed… resets them to nil ») reste exact, et il est gardé. On n'ajoute rien, sauf le contrat de `has_ca` au format de `mh.h` (« 0: `ca` is nil, and `ca` is then never read »), qui est un contrat C.

### Hypothèses (rien n'a été lancé)

- **H1** : `make value-members` rend 263 sur l'arbre de départ, puis 260.
- **H2** : `test_pico8_js` (dans `UNIT_TESTS`, hors `UNIT_TESTS_SLOW`) reste sous 30 s avec la caractérisation.
- **H3** : tant que `pico8_ha_pz()` est non NULL, `hi`/`gq` sont non NULL et `has_ca == 1` (`assert` de l'init). Nil n'est lisible qu'à travers un pointeur gardé au-delà de `shutdown`.
- **H4** : `pico8_type_tag_of` rend `UNKNOWN` sur la cible de `v(…)`. Cela explique `ref:autre`, et aucun autre genre n'atteint `hi`/`gq`.
- **H5** : les empreintes de `pause_menu_music` et du rejeu `pause_menu_music_events` ne bougent pas : `pz` lit les mêmes valeurs.
- **H6** : `STRUCT_HASH`/`SPEC_HASH` de `test_pico8_spec_defaults` ne couvrent pas `ha_pz` (aucune mention dans le test).
- **H7** : le build `-DPICO8_MEMBER_CENSUS` compile encore, et la table générée perd les 3 lignes. Ce n'est pas lancé.
- **H8** : `test_pico8_js` ne fait pas partie des tests de régression de bug du §6. Il n'y reçoit qu'un ajout.

## Testing Decisions

- **Un bon test juge ce qu'on lit par nom ou à l'écran.** La caractérisation ne fait aucun accès `->hi/gq/ca` et compile sur les deux formes.
- **Caractérisation, figée sur l'ancien code** : `test_ha_pz_members_by_name`, dans `test_pico8_js`, lancée par `in_fresh_child` comme `test_ha_pz_is_a_struct`. Pas de nouveau TU.
  1. `start_game(PAUSE)`, puis `o = pico8_ha_pz()`. `out` est posé à un témoin, un fixe `0x1234`.
  2. `pico8_entity_get_field(o, "hi")` rend 1, de genre `REF`, avec le **même pointeur** que `g_ha.hi`, et un vecteur pointé de `(0,107)`. Pour `gq` : même pointeur que `g_ha.gq`, vecteur `(0,-11)`. Pour `ca` : `FIXED` 14.
  3. `cls->field(o, n, &out)` rend 0 pour `n` ∈ {`"ra"`, `"ep"`, `""`}, et `out` reste égal au témoin.
  4. Après `pico8_menu_shutdown()`, `cls->field(o, m, &out)` rend 0 pour `hi`, `gq` et `ca`, avec `out` intact.
  5. Après `pico8_menu_init()`, l'étape 2 est rejouée contre la nouvelle table `g_ha`.

  Elle est lancée verte sur l'arbre de départ **avant** le typage. Ses valeurs ne changent pas ensuite. L'écran est déjà figé par les rejeux du menu pause. On n'ajoute pas d'empreinte.
- **Mutations (§7)**, dans une copie jetable du scratchpad. Chacune est notée dans le commit avec le test qui la rougit.
  1. (§7-1) *indicateur forcé vrai* : le lecteur ignore `has_ca`. Attendu rouge : caractérisation 4, `test_ha_pz_is_a_struct` (`!has(o,"ca")`).
  2. (§7-2) *« présent 0 » là où c'était absent* : `shutdown` pose `ca = 0` sans `has_ca = 0`. Attendu rouge : les mêmes tests qu'en 1.
  3. (§7-3) *init qui saute un membre typé* : l'init ne pose pas `gq`, qui reste NULL. Attendu rouge : caractérisation 2, `same_as_ha(o,"gq")`, les empreintes du menu pause.
  4. Propre à l'étape, *identité de REF* : `hi` pointe sur une copie `malloc` du vecteur au lieu de celui de `g_ha`. Les pixels restent identiques. Attendu rouge : caractérisation 2, `same_as_ha(o,"hi")`.
- **`gcov`** (§7, `test_pico8_js` en `--coverage -O0`, avec le piège `__gcov_dump` des fils de 1a.23) : chaque ligne modifiée du lecteur, de l'init et du shutdown est exécutée, y compris les branches nil, par la caractérisation 4. **Aucun trou n'est attendu.** L'initialiseur statique n'est pas exécutable : il est couvert par la caractérisation 4, qui lit l'état nil après `shutdown`. Le rapport va dans le commit.
- **Prior art** : `test_ha_pz_is_a_struct` et `same_as_ha` (`test_pico8_js`), `test_mh_members_by_name` (1b.1), `test_jj_qs_by_name` (1b.3).

### Vérification, budget et tableau « Runs »

**Budget strict : environ 20 minutes de temps de tests au total.** Au-delà, l'implémenteur s'arrête et note ce qui manque au lieu de relancer. Ordres de grandeur :
- `make test` : 20 s à 1 min ;
- `make e2e` : 4 à 6 min ;
- `make sanitize-net` : ~4 min seul, le double sous charge (on le lance seul) ;
- `make value-members` : quelques secondes.

Le journal tient un tableau « Runs », une ligne par lancement. Le vérificateur ne relance que ce qui suit le dernier changement de l'arbre.

| # | Commande exacte | État de l'arbre | Code de sortie | Ligne clé | Durée |
|---|---|---|---|---|---|

**Cibles finales**, vertes sur l'arbre final :

1. `timeout 10m make test` ;
2. `timeout 15m make e2e` : **15 « conforme »**, et les `UNIT_TESTS_SLOW` passent ;
3. `timeout 15m make sanitize-net` : **`sanitize: 15 propre(s)`** ;
4. `make value-members` : **`membres pico8_value_t : 260`**, sans aucune ligne `pico8_ha_pz_t`.

`make class-coverage-net` **ne s'applique pas** (§7). `ha_pz` n'a pas de constructeur. `pico8_menu_init` n'appelle pas `pico8_class_coverage_record`, et le diff ne touche ni `KNOWN[]` ni un constructeur. S'il en touche un, la cible redevient obligatoire, avec une sortie identique à `tools/class_coverage_net_expected.txt`.

`make member-census` ne s'applique pas (R8). Jamais l'appli raylib.

### Critères d'échec (le vérificateur de `prd-autopilot` les traduit en `FAILED:`)

- `FAILED:` si le filet (`make test` ou `make e2e`) reste rouge après **3 tentatives** sur ce commit.
- `FAILED:` si un `*.sha256` est modifié ou supprimé.
- `FAILED:` si le diff ou le commit touche `FRAMES_EVENTS`, `RECORDINGS` ou un fichier d'événements (`tests/fixtures/*events*.json`). L'utilisateur y travaille : ses changements non commités sur ces fichiers ne sont ni modifiés, ni indexés, ni commités.
- `FAILED:` (« champ sans règle ») si un membre, un indicateur autre que `has_ca`, un écrivain ou une méthode est ajouté à `ha_pz`, ou si le lecteur accepte un autre nom.
- `FAILED:` si `hi`/`gq` ne sont pas `pico8_vec_t *` empruntés à `g_ha` (même pointeur, pas de copie), ou si `ca`/`has_ca` ne sont pas `pico8_fixed_t`/`int`.
- `FAILED:` si le lecteur ne rend pas 0 avec `*out_value` intact sur NULL ou `has_ca == 0`, ou si `shutdown` ne remet pas les trois à nil.
- `FAILED:` si l'`assert` de l'init ou son message change.
- `FAILED:` si un test de régression de bug (§6) change, si une valeur figée existante change, ou si le diff de `test_pico8_js.c` n'est pas un pur ajout.
- `FAILED:` si `make sanitize-net` n'est pas `15 propre(s)` sur l'arbre final.
- `FAILED:` si `make value-members` ne rend pas 260, liste encore `pico8_ha_pz_t`, ou si un autre typedef gagne ou perd une ligne.
- `FAILED:` si `g_ha` (la table), `pz`, `pz_step` ou un site par nom change.
- `FAILED:` si le diff touche un constructeur, `pico8_class_coverage_record` ou `KNOWN[]` sans une ligne « Runs » de `make class-coverage-net` identique à la référence.
- `FAILED:` si un crochet du recensement change, si `tools/member_census.txt` est édité à la main, ou si `tools/entity_value_members.sh` change.
- `FAILED:` si la caractérisation n'a pas de ligne « Runs » verte sur l'arbre de départ avant le typage, si elle lit un membre directement, ou si ses valeurs changent après le typage.
- `FAILED:` si le commit ne contient pas le rapport `gcov` et les 4 mutations avec le test qui rougit chacune.
- `FAILED:` si un écart port/cart est corrigé au lieu d'être préservé par un `TROU-N`.
- `FAILED:` s'il manque la note « Découvert à 1b.4 », ou si elle écrit une forme concrète pour R5 ou R7.
- `FAILED:` s'il manque le tableau « Runs ».

## Out of Scope

- La table `g_ha` et `g_ix` (phase D).
- Les membres homonymes `hi`/`gq`/`ca` de `fv`, `jo` et `pj_cr` (leurs étapes respectives).
- Donner une forme concrète à R5 ou à R7.
- Les fichiers d'événements et `FRAMES_EVENTS`.
- Tout changement de rendu ; l'audio ; lancer l'appli raylib.

## Further Notes

- **Décisions à la place de l'utilisateur** (G) :
  - modules : `menu.c` seul ;
  - tests : `test_pico8_js` étendu, sans nouveau TU ;
  - un commit, sans découpage ;
  - `ref:autre` tranché en `pico8_vec_t *` ;
  - `has_ca` gardé, parce qu'un lecteur voit nil ;
  - R5 vacante, parce qu'il n'y a pas d'écrivain par nom ;
  - `class-coverage-net` non lancé, parce qu'il n'y a pas de constructeur.
- **Note « Découvert à 1b.4 »**, à écrire sur la ligne 1b.4 de `docs/plan-3.md`. Elle ne garde que ce qui change une étape suivante : « patron 1b.1 appliqué sans écart ; `ref:autre` vers un vecteur de `v(…)` → `pico8_vec_t *` emprunté, même pointeur ; ni écrivain ni table d'offsets, R5 et R7 restent à fixer par la première étape qui en a ». Le reste va dans le commit.
- Le message de commit porte :
  - le périmètre ;
  - le compte 263 → 260 ;
  - le résumé du tableau « Runs » ;
  - le rapport `gcov` ;
  - les mutations ;
  - les réponses à H1 à H8.
- Un sous-agent frais relit le diff contre ce PRD avant le commit.
