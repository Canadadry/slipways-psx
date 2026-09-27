---
title: "Plan 3, étape 1b.4b : recensement — observations des fils forkés (correctif de 1b.0)"
description: "Le recensement de 1b.0 perd tout ce que voient les 148 fils de `in_fresh_child` des 17 TU qui en ont un : ils finissent par `_exit(0)`, qui saute l'`atexit` du vidage. On fait vider chaque fils, compteurs hérités remis à zéro au fork, écritures sentinelles des cas `*_is_a_struct` écartées, puis on régénère `tools/member_census.txt`, on revoit les choix de 1b.1 à 1b.4 et on liste les lignes 1b qui changent, sans rien changer au build normal."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, que l'orchestrateur insère dans `docs/plan-3.md` (§5, « Étapes de la phase 1b ») entre 1b.4 et 1b.5 :

> - 1b.4b — recensement : observations des fils forkés (correctif de 1b.0)

Cadre : la puce **Recensement** du §5 « Phase 1b », le §3 (`make test` vert, aucun `*.sha256` modifié) et le §7 (arrêts, mutations). Le PRD de 1b.0 (`docs/prd/done/plan3-phase1b-etape0-recensement-membres.md`) reste la référence de l'outil. Cette étape corrige une seule de ses affirmations, fausse (`:281`) : « Un processus qui finit par `abort` ou `_exit` (fils de `must_abort`) n'écrit rien, sans perte pour l'union. » C'est vrai pour `must_abort`. C'est faux pour `in_fresh_child`, où chaque fils exécute un scénario complet.

### État de départ

HEAD 25618ed : 1b.4 (`ha_pz`) est commitée et gradée. `make value-members` rend 260. Le PRD de 1b.5 (`trade`) est dans `docs/prd/triage/`, non commité : il n'est pas touché ici.

**Rien n'a été compilé ni lancé pour ce PRD.** Les faits viennent de `grep` et de la lecture du code, plus un diagnostic `help-me-debug` antérieur, qui a mesuré sans l'écart des sentinelles. Tout fait d'exécution est une hypothèse (H1 à H15).

### Le défaut

- `start()` (`src/pico8_member_census.c:213-231`) enregistre `atexit(dump)` à la première observation. `dump()` écrit `$PICO8_MEMBER_CENSUS_OUT/<pid>.txt`, en ajout. `tools/member_census.sh` fait déjà l'union de tous les `raw/unit/*.txt`.
- 17 des 47 TU de `CENSUS_UNIT_TESTS` ont chacun leur copie d'un lanceur de scénario forké : qb, mz, bg, jc, js, nl, fr, ga, g_, fv, pj, ui, trade, hp, lq, hy et fm. Chaque copie fait `fork`, lance le cas, `fflush(stdout)`, puis `_exit(0)`. `_exit` saute l'`atexit` : tout ce que voit le fils est perdu. Il y a **un site par TU**, toujours la forme `test(); fflush(stdout); _exit(0);` :
  `qb:450`, `mz:915`, `bg:673`, `jc:1085`, `js:1236`, `nl:648` (dans `spawn`, que `in_fresh_child` appelle), `fr:625`, `ga:760`, `g_:936`, `fv:690`, `pj:730`, `ui:755`, `trade:574`, `hp:677`, `lq:706`, `hy:681`, `fm:214` (chemins `tests/test_pico8_<x>.c`).
- On compte 148 fils : 147 appels `in_fresh_child(test…)`, plus `hub = spawn(test_hub)` dans `test_pico8_nl`. Le diagnostic en annonçait 144 (H1).
- **Les autres forks ne sont pas concernés** et ne changent pas :
  - les fils de mort : `must_abort` partout, les forks internes des cas `*_is_a_struct` (`qb:480`, `mz:844,855`…), `mh.c:478-491` et `mo.c:193-220`. Ils finissent par `SIGABRT`, qui ne vide rien, et c'est voulu (H14) ;
  - `probe_mm_rnd_order.c:85`, un fils à tube qui finit par `_exit(<statut de write>)`. Ses observations (une émission de particule de `g_`) sont un sous-ensemble de celles de `test_pico8_g_`/`ga` (H13).
- Les rejeux ne forkent pas et ne sont pas touchés (H6).
- Mesure du diagnostic, passe des TU seule, **sans** l'écart des sentinelles : 98 lignes sur 260 changent ; « sans genre » passe de 13 à 0 et « nil tenu seulement » de 27 à 3 ; `trade.qe` devient `bool,fixe` / `observé` (`lu`). Ces chiffres ne sont **pas** une cible (H11).

### Deux pièges du correctif simple

1. **Compteurs hérités.** Un fils hérite des compteurs du parent au moment du fork. Avec un vidage par pid, les lignes `T` (`vu N fois`) et `S` (`muets`, `differees`, `nil-par-nom`, `hors-membre`, `debordements`, `erreurs`) du parent sont recomptées une fois par fils : `hors-membre` passerait de 14203 à 175244. Or le script **échoue** sur `debordements` ≠ 0 et sur toute ligne `X` : une valeur héritée multipliée ferait un faux échec, et une valeur héritée nulle masquerait le défaut. Les genres et les drapeaux sont des unions : ils ne sont pas faussés.
2. **Écritures sentinelles.** Cinq cas `*_is_a_struct` écrivent par nom un fixe `i+1` dans **chaque** nom d'une liste `written[]`, quel que soit le genre du membre, pour prouver que l'écrivain accepte le nom : `nl:619`, `fr:599`, `ga:739`, `g_:880` et `fv:611` (le `for` sur `written`). Ces cinq cas tournent dans un fils. Une fois les fils vidés, ils ajouteraient de faux `fixe` (par exemple `fv.m_`, aujourd'hui sans genre, ou `g_.et`, `p_`, `ra`, `p`, `qy`). `test_pico8_jo.c:715` a la même boucle, mais `test_pico8_jo` n'est pas un TU du recensement.

## Solution

1. **Chaque fils de lanceur vide ce qu'il a vu depuis le fork.** Une macro de `pico8_member_census.h` remplace le `_exit(0)` des 17 sites. Sous le drapeau, elle vide puis appelle `_exit`. Sans le drapeau, elle se développe en exactement `_exit(0)`.
2. **Les compteurs hérités sont remis à zéro dans le fils**, par un gestionnaire `pthread_atfork` côté fils, enregistré par `start()`. Le vidage d'un fils porte des lignes de contrôle, et le script échoue si un fils a hérité d'un recensement démarré sans passer par la remise à zéro.
3. **Les écritures sentinelles sont écartées** : une macro d'arrêt, sans effet hors drapeau, est posée **sur la ligne même** du `for` de chacune des 5 boucles. Le reste du fils n'observe plus rien.
4. **`tools/member_census.txt` est régénéré** par `make member-census` complet (15 rejeux et 47 TU).
5. **Les choix de 1b.1 à 1b.4 sont revus** sur un recensement corrigé de l'arbre d'avant 1b.1. Un choix invalidé est un arrêt `FAILED`, rapporté et non corrigé.
6. **Les lignes 1b suivantes qui changent sont listées** pour l'orchestrateur, dans la note « Découvert à 1b.4b ». L'étape n'édite aucune autre ligne du plan.

Effet mesurable : `make value-members` reste à **260** ; le build normal est identique (preuve `gcc -E` et `cmp` d'objets) ; la table change, et son rapport gagne une ligne `fils`.

## User Stories

1. En tant qu'auteur d'un PRD de 1b, je veux que la ligne de recensement de mon membre compte ce que voient les scénarios forkés, afin de ne pas typer sur une moitié des observations.
2. En tant qu'auteur du PRD de `trade` (1b.5), je veux que `qe` montre le `bool` que `kg` pose (TROU-24), afin que la ligne dise « genres mêlés » au lieu de le deviner.
3. En tant qu'auteur d'une étape « sans genre » (1b.8, 1b.13, 1b.33), je veux savoir si un fils a déjà observé un genre, afin de ne pas écrire une caractérisation dirigée inutile.
4. En tant que mainteneur de l'outil, je veux qu'un fils ne vide que ce qu'il a vu depuis le fork, afin que les compteurs `S` et `T` ne soient pas multipliés par le nombre de fils.
5. En tant que mainteneur de l'outil, je veux que le script échoue si un fils a hérité de compteurs sans remise à zéro, afin qu'une régression du gestionnaire de fork ne passe pas en silence.
6. En tant que lecteur de la table, je veux qu'aucun genre n'y vienne d'une valeur sentinelle écrite par un test, afin que `fixe` veuille dire « le jeu tient un fixe ».
7. En tant que gardien du §3, je veux `make test` vert et aucun `*.sha256` touché.
8. En tant que gardien du build normal, je veux la preuve que les 17 tests préprocessés sans drapeau sont identiques, à l'espace près, et que leurs objets sont identiques octet pour octet.
9. En tant que gardien des tests, je veux que les tests ne changent que par un crochet d'outil, sans cas, assertion ni valeur changés.
10. En tant qu'orchestrateur, je veux la liste des lignes 1b qui changent (pilote, découpage, drapeau « sans genre », « genres mêlés », ordre de REF), afin de les réécrire moi-même.
11. En tant qu'orchestrateur, je veux savoir si un choix déjà commité de 1b.1 à 1b.4 est invalidé, sous forme d'un arrêt `FAILED`, et non d'une correction silencieuse.
12. En tant que vérificateur, je veux trois mutations qui mordent : vidage du fils retiré, remise à zéro retirée, arrêt des sentinelles retiré.
13. En tant que vérificateur, je veux un tableau « Runs » tenu dans un budget d'environ 20 minutes.
14. En tant que gardien du rendu, je veux savoir que ni `make e2e` ni `make sanitize-net` ne sont requis, et pourquoi.

## Implementation Decisions

Pas d'humain dans la boucle. Le skill `to-prd` demande de faire valider modules et tests par l'utilisateur. Ces choix sont tranchés ici comme le ferait un agent `guideline` (changements chirurgicaux, pas de spéculatif, critères vérifiables) et marqués (G).

### Modules touchés

| fichier | changement |
|---|---|
| `src/pico8_member_census.h` | deux macros, `PICO8_MEMBER_CENSUS_CHILD_EXIT(code)` et `PICO8_MEMBER_CENSUS_STOP()`, et deux déclarations sous le drapeau |
| `src/pico8_member_census.c` | vidage d'un fils, remise à zéro au fork, arrêt, lignes `S` de contrôle ; tout reste sous `#ifdef PICO8_MEMBER_CENSUS` |
| `tools/member_census.sh` | somme des lignes de contrôle, un échec d'intégrité, deux lignes de rapport |
| 17 `tests/test_pico8_<x>.c` | le `_exit(0)` du lanceur remplacé par la macro, sur la même ligne |
| 5 d'entre eux (nl, fr, ga, g_, fv) | `PICO8_MEMBER_CENSUS_STOP()` en tête de la ligne du `for` sur `written` |
| `tools/member_census.txt` | régénéré, jamais édité à la main |
| `docs/plan-3.md` | la note « Découvert à 1b.4b » sur la ligne de l'étape, et rien d'autre |

Aucun autre fichier de `src/`. Aucun `#include` ajouté aux tests : les 17 incluent `../src/pico8_struct.h`, qui inclut `pico8_member_census.h` (`pico8_struct.h:7`).

### Les deux macros (G)

La forme sans drapeau est la décision : c'est elle qui garde le build normal identique.

```c
#ifdef PICO8_MEMBER_CENSUS
#define PICO8_MEMBER_CENSUS_CHILD_EXIT(code) pico8_member_census_child_exit(code)
#define PICO8_MEMBER_CENSUS_STOP() pico8_member_census_stop();
#else
#define PICO8_MEMBER_CENSUS_CHILD_EXIT(code) _exit(code)
#define PICO8_MEMBER_CENSUS_STOP()
#endif
```

- `CHILD_EXIT` s'écrit comme l'appel remplacé : `PICO8_MEMBER_CENSUS_CHILD_EXIT(0);`. Le préprocesseur rend exactement `_exit(0);`. `unistd.h` est déjà inclus par les 17.
- `STOP` s'écrit **sans point-virgule**, **en tête de la ligne du `for`** : `PICO8_MEMBER_CENSUS_STOP() for (i = 0; …`. Sans drapeau, il ne reste rien : pas d'instruction vide, et aucun `__LINE__` ne bouge. Or les `assert` qui suivent passent leur numéro de ligne en immédiat dans `.text`, donc un `STOP` sur sa propre ligne changerait l'objet. C'est un piège qu'on réintroduirait : le commentaire de contrat de l'en-tête le dit en une phrase. (G) C'est une forme inhabituelle. Elle est préférée à `((void)0);` sur une ligne à part, qui changerait le préprocessé **et** l'objet, et à un arrêt de tout le cas, qui perdrait les observations d'avant la boucle.
- Commentaires (CLAUDE.md, contrat C) :
  - `CHILD_EXIT` : « dans un fils forké par un test : vide ce que ce processus a vu depuis le fork, puis `_exit(code)` » ;
  - `STOP` : « plus aucune observation dans ce processus ; sans point-virgule et sur la ligne de l'instruction qu'il précède, pour qu'aucun `__LINE__` ne bouge hors drapeau ».

### Dans le module, sous le drapeau (G)

- **Remise à zéro au fork** : `start()` enregistre aussi `pthread_atfork(NULL, NULL, reset_in_child)`, une fois, à côté de l'`atexit`.
  - `reset_in_child` met à 0 `g_seen[]`, `g_muted`, `g_deferred`, `g_by_name_nil`, `g_outside`, `g_overflow`, `g_error_count` et `g_unknown_count`, puis pose `g_forked = 1`.
  - Il garde les unions : `g_kinds`, `g_flags`, `g_target_cls`, `g_in_hv`. Il garde aussi la connaissance, qui n'est pas une observation : `g_cls`/`g_cls_td` (descripteur → typedef) et `g_first`/`g_count`.
  - (G) Il **garde `g_pending`**. Une lecture par nom faite avant le fork (`pico8_cart_load()`) sur un descripteur qu'on ne verra que dans un fils se résout dans ce fils. La vider perdrait une observation réelle. Conséquence acceptée : `differees` compte une résolution par processus qui la fait. C'est un compteur de rapport, sur lequel le script n'échoue pas.
  - Stockage statique seulement, rien sur le tas (règle de 1b.0). `#include <pthread.h>` sous le drapeau, avec le `_POSIX_C_SOURCE 200112L` déjà posé (H2).
- **Vidage d'un fils** : `pico8_member_census_child_exit(int code)`.
  - Si le recensement a démarré dans ce processus ou un ancêtre (`g_started`), il appelle `dump()`. Ce vidage ajoute trois lignes : `S fils 1`, `S fils-herites h` et `S fils-remis r`. `h` vaut 1 si `start()` a tourné dans un ancêtre (on compare le pid noté par `start()` à `getpid()`). `r` vaut 1 si `h` vaut 1 et que `g_forked` est posé.
  - Puis `_exit(code)`. L'`atexit` hérité ne tourne jamais : pas de double vidage.
  - `dump()` écrit toujours `<pid>.txt` en ajout. Un pid réutilisé par un autre processus plus tard empile deux vidages dans un fichier. L'union n'en souffre pas, mais le nombre de fichiers n'est pas un contrôle (H9).
- **Arrêt** : `pico8_member_census_stop()` pose `g_stopped = 1`. Les quatre crochets (`see`, `read`, `write`, `walk`) rentrent aussitôt s'il est posé. `read` rend toujours `found`, pour qu'il n'y ait aucun effet sur le test. Le vidage écrit `S arrets 1`. Ce qui a été vu avant l'arrêt reste vidé.
- Les processus non forkés (parents, rejeux) n'écrivent ni `fils*` ni `arrets` : leur format de vidage ne change pas.

### Dans le script (G)

- Après les sommes existantes, il fait trois sommes sur `raw/unit/*.txt` et `raw/replay/*.txt` : `F = Σ fils`, `H = Σ fils-herites`, `R = Σ fils-remis`. Il fait aussi `A = Σ arrets`.
- **Nouvel échec d'intégrité** : `H ≠ R` → `fail "fils : H fils ont hérité d'un recensement démarré, R seulement remis à zéro au fork"`. Il suit la règle du script : un échec sur l'intégrité de l'outil, jamais sur le contenu.
- Rapport : `fils : F (hérités H, remis R) ; arrêts : A`.
- La ligne de résumé de la table et son en-tête ne changent pas de format.

### Pourquoi un crochet de recensement dans un test est acceptable (G)

La règle dit que les tests ne changent que par la façon dont ils lisent l'état. `CHILD_EXIT` et `STOP` ne lisent ni n'écrivent l'état du jeu. Ils ne changent aucun cas, aucune assertion, aucune valeur attendue ni aucun ordre d'exécution. Hors drapeau, le préprocessé est identique à l'espace près et l'objet l'est octet pour octet : pour le compilateur du build normal, le test n'a pas changé. C'est de l'outillage, de la même nature que les `PICO8_MEMBER_CENSUS_SEE` posés dans `src/` à 1b.0. Toute autre modification d'un test est hors périmètre et `FAILED`.

### Ce que les sentinelles écartent, et ce qu'elles n'écartent pas (G)

- Seules les 5 boucles `written[]` sont arrêtées. Elles écrivent un genre choisi par le test, pas par le jeu.
- Les autres écritures par nom que les fils vont maintenant verser portent le genre que le cart écrit à ce membre. Elles restent :
  - dans les cas `*_is_a_struct` : `bg:652` (`m_` bool), `jc:967,969,1023`, `js:1069,1071` et `g_:920` ;
  - dans les scénarios : `qb:288`, `mz:556`, `jc:693,705`, `js:940,944`, `nl:452,455`, `fr:469,476` et `ga:546`.

  H8 le vérifie. Une qui apporterait un genre absent des rejeux et du cart est traitée comme une sentinelle : un `STOP` de plus, noté dans « Découvert ».
- Ce qui suit la boucle dans le même cas n'est plus observé : `bare` de `nl`, `pico8__init()` et le parcours de `hv` de `g_`. Ces constructions sont observées par d'autres cas et par les rejeux. La perte est acceptée.

### Revue des choix de 1b.1 à 1b.4

Les membres typés sont sortis de la table générée (R8) : la table régénérée ne peut pas les juger. On recense donc l'arbre d'avant 1b.1, corrigé :

- `git worktree add --detach <scratchpad>/pre1b1 3c4bc52^` (85c4534 : 1b.0 gradée, liste 1b écrite, aucun typage). Le module et le script du recensement n'ont pas changé entre cet arbre et HEAD (`git diff --stat 37d73d8 HEAD` ne les liste pas). On y applique le diff de cette étape : `git diff -- src/pico8_member_census.[ch] tools/member_census.sh tests/ | git -C … apply -3`, à la main si un morceau refuse (H10).
- On y lance `make member-census CENSUS_EVENTS= CENSUS_TABLE=tmp/census/pre1b1.txt`, en passe TU seule (H7). La moitié rejeux de ces lignes est déjà dans la table commitée : les rejeux ne changent pas.
- Pour chacune des 10 lignes, **genres = union** de la ligne commitée et de la ligne `pre1b1`, et **nil = le plus fort** (observé > tenu > jamais). On juge ensuite chaque choix commité :

| étape | membre | choix commité | invalidé si |
|---|---|---|---|
| 1b.1 | `mh.nu` | `pico8_ob_table_t *` | un genre autre que `ref:table` |
| 1b.1 | `mh.fa`, `mh.rh` | `pico8_fixed_t`, **sans** `has_` | un genre autre que `fixe`, **ou** nil `tenu`/`observé` (R2 exigerait `has_fa`/`has_rh` : lus par nom) |
| 1b.1 | `mh.lh` | `pico8_value_function_t` (NULL = nil) | un genre autre que `fonction` |
| 1b.1 | `mh.p_` | `int` + `has_p_` | un genre autre que `bool` |
| 1b.2 | `jb.ls` | `pico8_list_t *` | un genre autre que `ref:liste` |
| 1b.3 | `mz_jj.qs` | `const char *` | un genre autre que `chaine` |
| 1b.4 | `ha_pz.hi`, `ha_pz.gq` | `pico8_vec_t *` emprunté | un genre autre que `ref:autre` |
| 1b.4 | `ha_pz.ca` | `pico8_fixed_t` + `has_ca` | un genre autre que `fixe` |

- Une nouvelle cible REF vers ces structs (par exemple `pj.ed` → `ref:pico8_mh_t`) n'invalide rien : elle concerne la struct qui pointe.
- Limite : les caractérisations ajoutées par 1b.1 à 1b.4 ne sont pas dans cet arbre. Elles lisent les membres par nom, comme le faisait déjà l'ancien code, et ne peuvent apporter qu'une preuve de nil sur `p_`/`ca`/`lh`, qui ont déjà la leur.
- Une ligne invalidée est un arrêt **`FAILED`** : la ligne, le genre ou le nil en cause, la règle cassée (R1 ou R2). **Rien n'est corrigé** dans cette étape.
- Le worktree est supprimé ensuite (`git worktree remove`).

### Lignes 1b qui changent : procédure et candidats

On compare la table régénérée aux 260 lignes communes de la table commitée (la table commitée a 270 lignes : les 10 typées de 1b.1 à 1b.4 en sont sorties par R8, ce n'est pas un effet de cette étape). Chaque ligne changée est rattachée à l'étape 1b de sa struct, puis classée :

1. **« sans genre » levé** : l'étape perd sa mention en gras « sans genre » pour ce membre. Candidats : 1b.8 (`fm_placeholder.jk`/`nz`, que `test_pico8_fm` exerce dans ses fils, `test_cheat_on_title` compris), 1b.13 (`jc_too_far.qs`/`fo`, `test_too_far` dans un fils) et 1b.33 (`fv.m_`, qui ne doit **pas** sortir de la boucle sentinelle, arrêtée).
2. **Un genre devient plusieurs** : l'étape gagne « genres mêlés `<m>` ». Candidat sûr : 1b.5 (`trade.qe`, `bool` vu par `test_kg`/`test_eq`, TROU-24). Le PRD de 1b.5 en triage cite les anciennes lignes `:230-232` et « le recensement ne l'a pas vu » : il doit être relu contre la nouvelle table. C'est à l'orchestrateur d'en décider.
3. **nil `jamais` → `tenu`/`observé`** sur un non-pointeur lu par nom : l'étape note que R2 exige `has_<m>`.
4. **Nouvelle cible REF vers une struct placée plus tard dans l'ordre** : la contrainte « cible de REF avant la struct qui la pointe » peut casser. C'est signalé, sans réordonner.
5. **Pilote** : aucun changement possible, 1b.1 est faite. Seul un `FAILED` de la revue la remet en cause.
6. **Découpage** : aucun attendu. L'empreinte est un compte de fichiers par `grep`, et le recensement n'y entre pas.
7. **Comptes** (`Compte : N`) : inchangés. `make value-members` ne dépend pas de la table.

La liste (étape, membre, avant → après, classe 1 à 4) va dans la note « Découvert à 1b.4b » et dans le message de commit. L'étape **n'édite aucune autre ligne** du plan : l'orchestrateur les réécrit.

### Hypothèses (rien n'a été lancé)

- **H1** : 148 fils atteignent le vidage (`fils : 148` au rapport). Le compte vient de `grep` : 147 appels `in_fresh_child(test…)` et 1 `spawn(test_hub)`. Le diagnostic disait 144. L'écart est expliqué dans « Découvert » par le chiffre mesuré.
- **H2** : `pthread_atfork` se lie sans `-lpthread` (glibc 2.39 : dans la libc depuis 2.34) et se déclare sous `-std=c99` avec `_POSIX_C_SOURCE 200112L`. Sinon, on ajoute `-pthread` à `CENSUS_CFLAGS` et au lien de l'arbre de recensement seulement, et on le note. Le build normal ne change pas.
- **H3** : chacun des 17 parents a démarré le recensement avant son premier fork (`pico8_cart_load()` construit des structs, donc des `SEE`). Donc `fils-herites` = `fils` = 148.
- **H4** : aucun fils n'écrit par nom un nom hors membre : aucune ligne `X`.
- **H5** : aucun plafond statique n'est dépassé dans un fils : `debordements` = 0.
- **H6** : `pico8_replay_e2e` ne forke pas. Les 15 sorties de rejeu et leurs lignes ne changent pas.
- **H7** : une passe TU seule (`CENSUS_EVENTS=`) marche : 0 fichier de rejeu pour 0 rejeu, `cat` sans fichier replié sur un fichier vide (`member_census.sh`).
- **H8** : hors des 5 boucles `written[]`, les écritures par nom des fils portent un genre déjà vu dans `raw/replay/` pour ce membre, ou que le cart y écrit. Vérifié par `grep` sur l'union des rejeux, pour chacune des écritures listées plus haut.
- **H9** : un pid réutilisé empile deux vidages dans un fichier. L'union et les sommes restent justes. Le nombre de fichiers de `raw/unit/` n'est pas contrôlé, on contrôle la somme `fils`.
- **H10** : le diff de l'étape s'applique sur `3c4bc52^`. Les lignes des lanceurs et des boucles sont identiques à HEAD, à un décalage près dans `test_pico8_js`/`mz`.
- **H11** : les chiffres du diagnostic (98 lignes, sans genre 13 → 0, tenu 27 → 3) ont été mesurés sans l'arrêt des sentinelles et sur la passe des TU seule. La table régénérée tranche, pas ces chiffres. `fv.m_` peut rester sans genre.
- **H12** : `make member-census` complet reste sous 5 min avec 148 vidages de plus. Le vidage est quelques `fprintf` par processus.
- **H13** : le fils de `probe_mm_rnd_order` n'observe rien qui ne soit déjà vu par `test_pico8_g_`/`ga`.
- **H14** : les fils de mort avortent avant le crochet `WRITE`, qui suit l'appel de l'écrivain. Ils ne vident rien, et ne doivent rien vider.
- **H15** : les objets compilés sans `-g` sont identiques octet pour octet, avant et après, pour les 17 tests. Avec `-g`, seules les colonnes de la ligne du `STOP` bougent dans `.debug_line`, d'où la comparaison en `-g0`.

## Implementation Chunks

1. **Module et script** : les macros, le vidage du fils, la remise à zéro, l'arrêt, les lignes `S` et le contrôle `H = R`. Rien ne l'utilise encore. `make test` reste vert.
2. **Tests** : 17 remplacements de `_exit(0)` et 5 `STOP`, puis les preuves `gcc -E` et `cmp` d'objets sur l'arbre de base.
3. **Table** : `make member-census` complet, table régénérée, diff classé.
4. **Revue de 1b.1 à 1b.4** sur le worktree `3c4bc52^`.
5. **Mutations** en passe réduite, dans une copie jetable.
6. **Note « Découvert à 1b.4b »**, commit.

## Testing Decisions

- **Un bon test juge le produit de l'outil**, la table et le rapport, pas l'état interne du module. On n'ajoute pas de TU : c'est la règle de 1b.0 pour l'outil, qui est prouvé par ses contrôles d'intégrité et par des mutations.
- **Preuve « build normal identique »** (§3), sur l'arbre de base `git worktree add --detach <scratchpad>/base HEAD`, **avant** toute modification :
  - pour chacun des 17 tests : `gcc -std=c99 -E -P tests/test_pico8_<x>.c` lancé depuis chaque racine. Les deux sorties passées à `diff -w` doivent être **vides**. Pour les 12 sans `STOP`, `cmp` doit même être identique ;
  - pour `test_pico8_trade` (sortie seule) et les 5 à `STOP` : `gcc -std=c99 -Wall -Wextra -Werror -g0 -c tests/test_pico8_<x>.c -o …`, lancé depuis chaque racine, puis `cmp` des deux `.o`, identiques ;
  - `src/pico8_member_census.c` sans drapeau reste une unité vide : on vérifie son `gcc -E -P` sans drapeau avant et après, identique.
- **Mutations**, dans une copie jetable du scratchpad, restaurées, puis `cmp` contre l'arbre. Chacune en passe réduite TU seule (`make member-census CENSUS_EVENTS= CENSUS_UNIT=<TU> CENSUS_TABLE=tmp/census/m.txt`) :
  1. **Vidage du fils retiré** (sous le drapeau, `CHILD_EXIT` redevient `_exit(code)`), sur `test_pico8_trade` : le rapport dit `fils : 0`, et la ligne `trade.qe` perd `bool` et sa preuve `lu` (retour à `fixe` / `tenu`, H11). La même passe sans mutation donne `fils : 7`, `bool` et `lu`.
  2. **Remise à zéro retirée** (plus de `pthread_atfork`), sur `test_pico8_js`, le TU à 27 fils : la cible **échoue** par `fils : 27 fils ont hérité… 0 seulement remis`. Avec ce contrôle neutralisé dans la copie, la somme TU de `hors-membre` au rapport est multipliée, environ par le nombre de processus (1 + 27), par rapport à la passe sans mutation.
  3. **Arrêt des sentinelles retiré** (le `STOP` de `fv:611` enlevé), sur `test_pico8_fv` : la ligne `fv.m_` (et les lignes `g_` héritées `qy`/`ra`/`p`/`et`/`p_` de `fv`) gagne `fixe`, que la passe sans mutation n'a pas, et le rapport dit `arrêts : 0` au lieu de 1.
- **Prior art** : les mutations en passe réduite de 1b.0 (O2 retiré, un `SEE` retiré, P4) ; le piège `__gcov_dump()` avant les `_exit` des fils (1a.23 à 1b.1), qui est le même défaut sur un autre outil ; `class_coverage.sh` pour le contrôle d'intégrité qui échoue sur l'outil et jamais sur le contenu.

### Vérification, budget et tableau « Runs »

**Budget strict : environ 20 minutes de temps de tests au total.** Au-delà, l'implémenteur s'arrête et note ce qui manque au lieu de relancer. Ordres de grandeur :
- `make test` : 20 s à 1 min ;
- `make member-census` complet : 1,5 à 4 min (1b.0), un peu plus avec les vidages (H12) ;
- passe TU seule sur le worktree `3c4bc52^`, construction à froid comprise : 2 à 4 min ;
- chaque passe réduite de mutation : moins d'une minute ;
- `gcc -E` et `cmp` : quelques secondes ;
- `make value-members` : quelques secondes.

Le journal tient un tableau « Runs », une ligne par lancement. Le vérificateur ne relance que ce qui suit le dernier changement de l'arbre.

| # | Commande exacte | État de l'arbre (HEAD + « chunk N appliqué » / « mutation N, copie jetable » / « worktree 3c4bc52^ ») | Code de sortie | Ligne clé | Durée |
|---|---|---|---|---|---|

Exemples de ligne clé : `fils : 148 (hérités 148, remis 148) ; arrêts : 5`, `# lignes 260 ; sans genre … ; rejeux 15 ; TU 47`, `membres pico8_value_t : 260`, ou la ligne qui échoue.

**Cibles finales**, vertes sur l'arbre final :

1. `timeout 10m make test` ;
2. `timeout 10m make member-census` (complet, 15 rejeux, 47 TU) : sortie 0, `fils : 148 (hérités 148, remis 148) ; arrêts : 5` (H1), `debordements` 0, aucune ligne `X`, table réécrite ;
3. `make value-members` : **`membres pico8_value_t : 260`**, inchangé (l'étape ne type rien).

**`make e2e` et `make sanitize-net` ne s'appliquent pas.** Hors `src/pico8_member_census.[ch]`, `src/` ne change pas. Ce module est une unité vide sans le drapeau. Les tests ne changent pas pour le compilateur normal (preuve ci-dessus). Les rejeux, les `*.sha256` et le binaire `-O2` sont donc intacts. Si le diff touche un autre fichier de `src/`, c'est hors périmètre (`FAILED`, voir plus bas) : il faudrait alors, en plus, `make e2e` (15 « conforme ») et `make sanitize-net` (15 propres). `make class-coverage-net` ne s'applique pas : aucun constructeur ni `pico8_class_coverage_record` ne change. Jamais l'appli raylib.

### Critères d'échec (le vérificateur de `prd-autopilot` les traduit en `FAILED:`)

- `FAILED:` si `make test` reste rouge après **3 tentatives** sur ce commit.
- `FAILED:` si un `*.sha256` est modifié ou supprimé.
- `FAILED:` si le diff ou le commit touche `FRAMES_EVENTS`, `RECORDINGS` ou un fichier d'événements (`tests/fixtures/*events*.json`). L'utilisateur y travaille : ses changements non commités sur ces fichiers ne sont ni modifiés, ni indexés, ni commités.
- `FAILED:` si un fichier de `src/` autre que `pico8_member_census.[ch]` change, ou si un changement du module sort de `#ifdef PICO8_MEMBER_CENSUS`, à l'exception des deux `#define` sans drapeau de l'en-tête.
- `FAILED:` si le `gcc -E -P` sans drapeau de l'un des 17 tests diffère de l'arbre de base à autre chose que l'espace près, ou si un `.o` en `-g0` diffère.
- `FAILED:` si un test change autrement que par les 17 `CHILD_EXIT` et les 5 `STOP` (plus un `STOP` justifié par H8 et noté) : aucun cas, aucune assertion, aucune valeur, aucun `#include`. Un `_exit` de fils de mort (`must_abort`, `*_is_a_struct`, `mh`, `mo`, `probe_mm_rnd_order`) remplacé est aussi un échec.
- `FAILED:` si `make member-census` complet échoue sur l'intégrité : P1 à P4, `debordements` ≠ 0, ligne `X` (une écriture par nom hors membre dans un fils est un champ sans règle du §5), contrôle `fils`. Le rapport est joint, sans correction de `src/` hors module.
- `FAILED:` si la revue de 1b.1 à 1b.4 invalide un choix commité. On donne la ligne, le genre ou le nil en cause et la règle (R1/R2), et **rien n'est corrigé**.
- `FAILED:` si `tools/member_census.txt` est édité à la main, ou si `tools/entity_value_members.sh` change.
- `FAILED:` si `make value-members` ne rend pas 260.
- `FAILED:` si le commit ne contient pas les 3 mutations, avec leur résultat.
- `FAILED:` s'il manque la note « Découvert à 1b.4b », si elle ne contient pas la liste des lignes 1b qui changent (vide comprise, dite vide), ou si une autre ligne du plan est éditée.
- `FAILED:` s'il manque le tableau « Runs ».

## Out of Scope

- Typer un membre, ou éditer une ligne 1b autre que 1b.4b. L'orchestrateur applique la liste.
- Corriger un choix de 1b.1 à 1b.4 invalidé (c'est un `FAILED`).
- Réécrire le PRD de 1b.5 en triage.
- Les fils de mort, le fils à tube de `probe_mm_rnd_order`, et `test_pico8_jo` et les autres TU hors `CENSUS_UNIT_TESTS`.
- Factoriser les 17 copies du lanceur dans un en-tête commun : les tests ne changent que par le crochet.
- Changer le format de la table ou son en-tête, ou ajouter un contrôle sur le contenu.
- Les fichiers d'événements et `FRAMES_EVENTS` ; l'audio ; lancer l'appli raylib.

## Further Notes

- **Décisions à la place de l'utilisateur** (G) :
  - modules : le module de recensement, son script et le crochet dans 17 tests ;
  - tests : pas de nouveau TU, trois mutations et un contrôle d'intégrité ;
  - remise à zéro par `pthread_atfork` plutôt qu'un delta calculé au vidage : il faudrait noter l'état au fork de toute façon, et le gestionnaire est le seul point qui voit le fork sans toucher aux tests ;
  - `g_pending` gardé dans le fils ;
  - `STOP` sur la ligne du `for`, sans point-virgule ;
  - revue sur `3c4bc52^` en passe TU seule ;
  - un commit, sans découpage.
- **Note « Découvert à 1b.4b »**, à écrire sur la ligne de l'étape dans `docs/plan-3.md`. Elle ne garde que ce qui change une étape suivante :
  - lecture corrigée de la puce **Recensement** : les fils de `in_fresh_child` sont vidés, compteurs hérités remis à zéro au fork, boucles sentinelles `written[]` arrêtées ;
  - le résumé de la nouvelle table ;
  - le verdict de la revue de 1b.1 à 1b.4 (« aucun choix invalidé », ou `FAILED`) ;
  - la liste des lignes 1b qui changent, classées de 1 à 4.

  Le reste (mutations, preuves `gcc -E`, `fils : N` contre 144, diff ligne à ligne) va dans le commit.
- Le message de commit porte :
  - le périmètre ;
  - le tableau « Runs » résumé ;
  - le diff classé de la table (lignes changées par classe) ;
  - la revue de 1b.1 à 1b.4, ligne par ligne ;
  - les mutations ;
  - les réponses à H1 à H15.
- Un sous-agent frais relit le diff contre ce PRD avant le commit.
