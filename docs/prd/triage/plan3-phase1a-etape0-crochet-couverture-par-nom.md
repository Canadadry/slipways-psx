---
title: "Plan 3, étape 1a.0 : crochet de couverture par nom, comptes de la porte 0.4 versionnés"
description: "L'outil de couverture compte les instanciations par pointeur de classe et retrouve le nom par des accesseurs : il ne pourra pas compter les structs de 1a, et rien ne fige les comptes de la porte 0.4 que chaque commit de 1a doit reproduire."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Étapes de la phase 1a ») :

> 1a.0 — crochet de couverture par nom (§4.2) ; comptes de la porte 0.4 versionnés. Tests : `test_class_coverage`.

§4.2 :

> avant le premier commit de 1a (étape 1a.0) : le crochet compte par **nom** passé au constructeur, plus par pointeur de classe ; `KNOWN[]` devient la seule liste des noms, et l'outil sort en erreur si un nom compté n'y est pas, ou si un nom des registres `fz`/nommé n'y est pas. Ce commit ne change aucun compte (sortie identique à la porte 0.4).

§7 :

> Chaque commit de 1a passe `make class-coverage-net` (critère du PRD, hors `make test`), avec, pour chaque classe, un compte d'instanciations **égal** à celui de la porte 0.4 (valeurs versionnées à côté des exemptions à l'étape 1a.0).

**État actuel (mesuré en cadrant ce PRD, sur 2d29849).**

- Le crochet (`#ifdef PICO8_CLASS_COVERAGE` dans le module de classes) enregistre le **pointeur** de classe à chaque `pico8_class_instantiate` ; la table se consulte par pointeur (`pico8_class_coverage_count(const pico8_class_t *)`).
- Le binaire de l'outil (`main_class_coverage.c`) porte `KNOWN[]` : 30 couples `{nom, accesseur}` (`pico8_jb_class`, …). Le nom n'existe que là ; il est retrouvé à partir du pointeur rendu par l'accesseur. Ce qui ne tombe sur aucun accesseur part dans une ligne `inconnue <n>`, que le script masque quand elle vaut 0.
- Seul le module de classes est compilé avec `-DPICO8_CLASS_COVERAGE` ; le binaire de l'outil lie le reste depuis l'arbre d'objets par défaut.
- En 1a, les instances converties seront construites par des constructeurs de struct, pas par `pico8_class_instantiate` (dès 1a.1 pour `jb`, dont une partie des 831 instanciations passera par un appel direct). Un crochet par pointeur de classe ne les voit pas ; c'est pourquoi §4.2 le fait passer au **nom**, que l'étape de chaque classe reprendra dans son constructeur.
- Registres : `fz` (`pm`, `ga`, plus l'emplacement `""` partagé par toutes les classes sans `fz`) ; nommé (`jo`, `g_`, `pm`, `ga`, `mz`, `jc`). Tous ces noms sont dans `KNOWN[]` aujourd'hui, sans que rien ne le vérifie.
- Rien ne fige les comptes de la porte 0.4 : ils sont dans un message de commit (450fb80) et dans `docs/prd/done/plan3-phase0-etape4-porte-couverture-filet.md`.

**Sortie de `make class-coverage-net` remesurée** (15 rejeux du filet, ~4 min, code 0), identique aux comptes du commit 450fb80 — c'est la référence de cette étape et de toute la phase 1a :

```
ke  0  0/15  exemptée (abstraite, jamais instanciée directement)
nl  1  1/15
os  1  1/15
fq  3  2/15
mx  3  1/15
pm  4  2/15
fr  5  1/15
hj  5  1/15
nr  6  2/15
ga  7  3/15
qh  8  3/15
g_  17  9/15
bg  19  15/15
fv  19  15/15
hy  19  15/15
pj  19  15/15
nf  38  15/15
qb  38  15/15
ui  38  15/15
mh  117  10/15
ie  133  15/15
mz  146  9/15
jc  152  6/15
iv  474  9/15
hp  510  15/15
jb  831  15/15
js  1286  15/15
jo  1710  15/15
lq  10718  15/15
trade  21032  6/15
couverture: OK -- chaque classe non exemptée est instanciée (15 rejeu(x))
```

## Solution

1. **Le crochet compte par nom.** Le magasin de comptes de la couche de classes est indexé par nom (comparaison de chaînes), plus par pointeur. Il a une entrée publique, « enregistrer une instanciation sous ce nom », que `pico8_class_instantiate` appelle aujourd'hui et que les constructeurs de struct de 1a appelleront ensuite.
2. **Chaque classe porte son nom de couverture**, posé par le module qui la déclare, juste après sa déclaration (30 sites), par une macro qui ne produit rien hors du build de couverture. `pico8_class_instantiate` enregistre ce nom ; une classe sans nom de couverture est enregistrée sous le nom vide, qui n'est jamais dans `KNOWN[]` — l'oubli devient une erreur de l'outil, pas un compte perdu.
3. **`KNOWN[]` n'est plus qu'une liste de noms**, les 30 noms actuels, dans le même ordre ; les accesseurs de classe disparaissent de l'outil. Après le rejeu, l'outil vérifie que chaque nom compté, chaque nom du registre `fz` (sauf l'emplacement `""`) et chaque nom du registre nommé est dans `KNOWN[]` ; sinon il écrit le nom fautif sur stderr et rend un code d'erreur (ni 0, ni 2). Sinon il imprime `<nom> <compte>` pour chaque nom de `KNOWN[]`, comme aujourd'hui. La ligne `inconnue` disparaît (elle ne peut plus valoir que 0).
4. **Les comptes de la porte 0.4 sont versionnés** dans un fichier à côté des exemptions, `tools/class_coverage_net_expected.txt`, dont le contenu est exactement la sortie ci-dessus (31 lignes). `make class-coverage-net` compare sa sortie complète à ce fichier et échoue, diff à l'appui, sur la moindre différence. Sur égalité, sa sortie est exactement celle de la porte 0.4, rien de plus.
5. **Le binaire de couverture est compilé en entier avec `-DPICO8_CLASS_COVERAGE`** (arbre d'objets de couverture pour tous les sources du moteur), puisque les sites de nommage sont maintenant dans les modules. Le binaire de rejeu et tous les autres binaires ne sont pas touchés : la macro y est vide et la struct de classe y garde sa forme.

## User Stories

1. En tant qu'autopilote de la phase 1a, je veux que `make class-coverage-net` échoue dès qu'un compte d'instanciations s'écarte de la porte 0.4, afin que le critère du §7 soit un code de sortie et non une comparaison à la main dans chacun des 42 PRD.
2. En tant qu'autopilote, je veux que la référence soit un fichier versionné à côté des exemptions, afin qu'un commit qui la modifierait se voie dans le diff.
3. En tant qu'auteur de l'étape 1a.1, je veux une entrée « enregistrer sous ce nom » indépendante de `pico8_class_t`, afin que le constructeur de struct de `jb` puisse compter ses instances sans pointeur de classe.
4. En tant qu'auteur d'une étape de 1a, je veux que le nom compté par la classe et celui compté par sa struct soient la même chaîne, afin que le compte de la classe reste égal pendant la migration.
5. En tant que mainteneur de l'outil, je veux que `KNOWN[]` soit la seule liste des noms de classes de l'outil, afin qu'il n'y ait qu'un endroit à tenir.
6. En tant que mainteneur, je veux que l'outil sorte en erreur si un nom compté n'est pas dans `KNOWN[]`, afin qu'une classe nouvelle ou renommée ne soit jamais comptée en silence dans un fourre-tout.
7. En tant que mainteneur, je veux que l'outil sorte en erreur si un nom du registre `fz` n'est pas dans `KNOWN[]`, afin que `kv[hp.cw]` ne puisse pas construire une classe que l'outil ignore.
8. En tant que mainteneur, je veux que l'outil sorte en erreur si un nom du registre nommé n'est pas dans `KNOWN[]`, afin que `lookup_named` ne puisse pas construire une classe que l'outil ignore.
9. En tant que mainteneur, je veux que l'emplacement `""` du registre `fz` ne soit pas pris pour un nom, afin que les classes sans `fz` (le cas le plus courant) ne fassent pas échouer l'outil.
10. En tant que mainteneur, je veux qu'une classe instanciée sans nom de couverture fasse échouer l'outil, afin qu'un oubli de nommage dans une étape ne passe pas pour un compte à 0.
11. En tant que lecteur d'un échec, je veux que le message nomme le nom fautif, afin de savoir quelle ligne ajouter à `KNOWN[]` ou quel module nommer.
12. En tant que lecteur de `make class-coverage-net`, je veux qu'un échec de l'outil sur un rejeu apparaisse comme « échec: <events> (code N) » avec son stderr, comme aujourd'hui pour un crash, afin de ne pas apprendre une nouvelle forme de rapport.
13. En tant que lecteur d'un écart de comptes, je veux le diff entre la sortie obtenue et la référence, afin de voir quelle classe a bougé et de combien.
14. En tant que mainteneur, je veux que `make class-coverage DIR=<dossier>` garde son comportement (pas de comparaison à une référence), afin qu'on puisse encore mesurer un dossier quelconque.
15. En tant que mainteneur du rejeu, je veux que le binaire de rejeu normal ne contienne ni le crochet ni le nom de couverture, afin que §4.2 (« le binaire de rejeu normal n'est pas touché ») reste vrai.
16. En tant que mainteneur du Makefile, je veux que le binaire de couverture ne mélange pas d'objets des deux arbres, afin qu'une struct de classe de forme différente ne soit jamais liée avec l'autre forme.
17. En tant que gardien du filet, je veux que `make frames-check` reste conforme et qu'aucun `.sha256` ne bouge, afin de respecter l'invariant du §3.
18. En tant que gardien du filet, je veux que la liste d'exemptions reste inchangée, afin que la porte ne se relâche pas sous couvert d'outillage.
19. En tant que gardien du filet, je veux que `FRAMES_EVENTS` et `RECORDINGS` gardent toutes leurs entrées, afin que la référence porte sur les mêmes 15 rejeux.
20. En tant que mainteneur des tests, je veux que `test_class_coverage` prouve la détection d'un nom compté hors liste, d'un nom de registre hors liste et d'une classe sans nom, afin que les erreurs du §4.2 soient démontrées et non supposées.
21. En tant que mainteneur des tests, je veux que `test_class_coverage` prouve qu'une référence égale laisse passer et qu'une référence différente d'un seul compte fait échouer, afin que le critère du §7 soit tenu par un test.
22. En tant que lecteur du code, je veux que les commentaires du crochet ne parlent plus de comptage par pointeur, afin qu'ils disent ce que fait le code (règle des commentaires de CLAUDE.md).

## Implementation Decisions

Faute d'agent disponible pour lancer un agent `guideline` frais, les points où `to-prd` demande de consulter l'utilisateur ont été tranchés en appliquant moi-même le skill `guideline` (simplicité, changement chirurgical, critères vérifiables). Ils sont marqués (G).

- **(G) D'où vient le nom.** Le nom est porté par la classe et posé par le module qui la déclare, pas passé à chaque appel de `pico8_class_instantiate`. Rejeté : ajouter un paramètre nom à `pico8_class_instantiate` (35 sites, dont `kv[hp.cw]`, `lookup_named` et l'enveloppe `ie.ke`, où la classe instanciée est lue dans un champ et son nom n'est pas connu du site). Rejeté aussi : une table pointeur → nom remplie par l'outil à partir de `KNOWN[]` (le nom resterait déduit du pointeur, et `KNOWN[]` garderait ses accesseurs). « Nom passé au constructeur » se lit ainsi : le constructeur (`pico8_class_instantiate` aujourd'hui, le constructeur de struct en 1a) passe un nom au crochet.
- **Interface du crochet** (dans l'en-tête de la couche de classes, tout sous `PICO8_CLASS_COVERAGE`) :
  - enregistrer une instanciation sous un nom ;
  - lire le compte d'un nom (0 s'il n'a jamais été enregistré) ;
  - « premier nom hors liste » : prend la liste des noms connus, rend le premier nom qui est compté, ou inscrit au registre `fz` (hors `""`), ou inscrit au registre nommé, sans être dans la liste ; rend NULL si tous y sont. Une fonction, un contrat, testable seule.
  - Le compte total par pointeur et la lecture par pointeur disparaissent.
- **Nommage d'une classe** : une macro de l'en-tête de classes, appelée juste après chaque déclaration (30 sites de `src/`), qui pose le nom sur la classe dans le build de couverture et ne produit rien ailleurs. Le membre qui porte ce nom n'existe dans la struct de classe que sous `PICO8_CLASS_COVERAGE`. Le nom est un littéral : la classe ne le libère pas.
- **Noms** : exactement ceux de `KNOWN[]` aujourd'hui (`bg fq fr fv g_ ga hj hp hy ie iv jb jc jo js ke lq mh mx mz nf nl nr os pj pm qb qh trade ui`), chacun posé sur la classe que son accesseur rend aujourd'hui. Aucun nom ajouté, retiré ou changé : c'est ce qui garantit des comptes identiques.
- **Classe sans nom** : enregistrée sous `""`, qui n'est jamais dans `KNOWN[]` → erreur de l'outil. Pas d'`assert` dans `pico8_class_instantiate` : les tests de la couche (`test_pico8_entity_smoke`) déclarent des classes sans nom, mais ils ne sont pas compilés en couverture ; l'erreur au niveau de l'outil suffit et reste un code de sortie.
- **Outil** : `KNOWN[]` devient un tableau de noms. L'assertion « deux accesseurs partagent une classe » devient « deux entrées de `KNOWN[]` portent le même nom ». Après le rejeu, l'outil appelle « premier nom hors liste » ; s'il rend un nom, message sur stderr qui le cite (le nom vide cité comme tel, en disant qu'une classe instanciée n'a pas de nom de couverture) et code 3. Code 2 garde son sens (« illisible »). Sinon, une ligne `<nom> <compte>` par entrée de `KNOWN[]`, dans l'ordre de la liste ; plus de ligne `inconnue`.
- **Script** : la gestion de `inconnue` (ligne masquée à 0, exclusion du verdict) est retirée, puisque plus rien ne la produit. Nouvelle option `--expect <fichier>`, acceptée parmi les options de tête comme `--from-title` : le script calcule sa sortie comme aujourd'hui (table triée, puis ligne de verdict), l'imprime, puis, si l'option est donnée, la compare octet pour octet au fichier ; sur différence, il imprime le diff et `couverture: ÉCHEC -- sortie différente de <fichier>` et rend 1, quel que soit le verdict. Sur égalité, il n'ajoute rien : la sortie reste exactement celle de la porte 0.4. Un fichier `--expect` absent est un échec, pas une comparaison ignorée.
- **Référence** : `tools/class_coverage_net_expected.txt`, les 31 lignes du Problem Statement, octet pour octet (remesurées au moment de l'implémentation par la cible actuelle, avant tout changement, et comparées à celles-ci : tout écart est un arrêt, voir critères). Pas d'en-tête ni de commentaire dans le fichier : il est comparé tel quel.
- **Makefile** :
  - `class-coverage-net` passe `--expect` avec la référence, et dépend d'elle ; `class-coverage DIR=` ne la passe pas.
  - Le binaire de couverture est lié uniquement depuis l'arbre d'objets de couverture : `main_class_coverage.c` et tous les sources du moteur compilés avec `-DPICO8_CLASS_COVERAGE`. Le commentaire au-dessus de la règle, devenu faux (« seul pico8_class.c change sous le define »), est remplacé par l'aide à la lecture de la commande : aucun objet de l'arbre de couverture n'est lié ailleurs, et aucun objet de l'arbre par défaut n'y est lié.
  - La ligne de légende de `class-coverage-net` dit qu'elle échoue si sa sortie diffère de la référence.
- **Commentaires** (CLAUDE.md) : le commentaire d'en-tête « Counted by class pointer, not by name… » est retiré (il devient faux) ; le contrat de « premier nom hors liste » (ce qu'elle rend quand tout est connu, l'exception `""` du registre `fz`) et le fait que le nom de couverture est un littéral non libéré sont les seuls commentaires ajoutés. L'en-tête de `main_class_coverage.c` est mis à jour (plus de ligne `inconnue`, code 3).
- **Aucune modification de comportement du jeu**, aucun `.sha256`, aucune entrée de `FRAMES_EVENTS`/`RECORDINGS`, aucune ligne de `tools/class_coverage_exempt.txt`. Aucune classe convertie : `jb` est 1a.1.

## Testing Decisions

- **Un bon test ici observe l'outil de l'extérieur** : codes de sortie, lignes imprimées, verdicts — jamais la forme du magasin de comptes.
- **`test_class_coverage`** (seul fichier de test de l'étape, dans `make unit`) :
  - les deux cas existants restent (rejeu court + fichier malformé → « illisible », comptes `jo`/`ui` > 0, `nl` à `0  0/1` ; exempter chaque classe à 0 → verdict OK), adaptés à la disparition de `inconnue` (le générateur d'exemptions n'a plus à l'exclure) ;
  - **`--expect`, égal** : on fait tourner le script sur le dossier fabriqué, on garde sa sortie comme référence, on relance avec `--expect` sur cette référence → code 0, sortie identique à la première, octet pour octet ;
  - **`--expect`, différent** : même référence avec un seul compte modifié → code non nul, message `sortie différente` ;
  - **`--expect`, fichier absent** → code non nul ;
  - **« premier nom hors liste »**, au niveau de la couche de classes compilée en couverture (le binaire de test est lié à l'arbre d'objets de couverture pour la couche de classes et ses dépendances) : une classe nommée `aa` instanciée deux fois → compte de `aa` = 2, compte d'un nom jamais vu = 0 ; liste `{aa}` → NULL ; liste vide → `aa` ; `register_named("bb", …)` → `bb` rendu avec la liste `{aa}` ; classe déclarée avec `fz="cc"` → `cc` rendu ; classe déclarée sans `fz` et nommée → l'emplacement `""` du registre `fz` n'est pas rendu ; classe sans nom de couverture instanciée → `""` rendu. Les registres sont vidés entre les cas avec les fonctions de support de test existantes.
- **Preuve jetable, consignée dans le message de commit, pas versionnée** : retirer une entrée de `KNOWN[]` (par ex. `jb`) → `make class-coverage-net` rend 1 avec « échec » et le nom `jb` dans le stderr rapporté ; changement annulé ensuite.
- **Neutralité** : `make test` vert, `make frames-check` conforme, `make class-coverage-net` code 0 et sortie égale à la référence.
- **Prior art** : `test_class_coverage` lui-même (script piloté sur un dossier fabriqué) ; `constants-check`, qui compare une sortie à une liste versionnée (`tools/constants_expected_ecarts.txt`) et échoue sur toute différence ; `test_pico8_entity_smoke` pour déclarer/instancier des classes dans un test.

## Critères du vérificateur

Chacun se traduit en `FAILED:` par le vérificateur de `prd-autopilot` (§7 du plan ; c'est le seul arrêt que `plan-pilot` honore).

- `FAILED:` si le diff modifie ou supprime un `*.sha256` du filet, ou retire une entrée de `FRAMES_EVENTS` ou de `RECORDINGS` (`git diff` sur le commit).
- `FAILED:` si `make test` ou `make frames-check` est rouge après **3 tentatives** de correction sur ce commit.
- `FAILED:` si la sortie de `make class-coverage-net` (sur le code final de l'étape, cible lancée sans `--expect` modifié à la main, lignes de compilation de make exclues) n'est pas, octet pour octet, les 31 lignes du Problem Statement — en particulier si un seul compte ou une seule colonne de rejeux diffère — ou si son code de sortie n'est pas 0.
- `FAILED:` si `tools/class_coverage_net_expected.txt` ne contient pas exactement ces 31 lignes (une référence « ajustée » à une sortie différente est un échec, pas une correction).
- `FAILED:` si `tools/class_coverage_exempt.txt` est modifié.
- `FAILED:` si la remesure faite au début de l'implémentation, avant tout changement, diffère des 31 lignes (le filet ou le moteur aurait bougé depuis le cadrage : décision humaine).
- `FAILED:` si une classe est convertie en struct, ou si `pico8_class_instantiate` change de signature.

Critères de réussite (en plus) : `KNOWN[]` ne contient que des noms ; plus aucune lecture de compte par pointeur de classe dans `src/` ; le binaire de rejeu est construit sans `-DPICO8_CLASS_COVERAGE` et le binaire de couverture uniquement depuis l'arbre de couverture ; la preuve jetable (entrée retirée de `KNOWN[]` → échec nommant le nom) est consignée dans le message de commit.

## Out of Scope

- Toute conversion de classe (1a.1 `jb` et suivantes), le déplacement du crochet dans un constructeur de struct, le pointeur de constructeur dans `pico8_class_t`.
- Compter les instances sans classe (`mo`, `je`, `fm`, enveloppes) : elles ne sont pas comptées aujourd'hui et ne le sont pas après.
- Changer le format des lignes de `class_coverage.sh`, son tri, ou la colonne des rejeux.
- Ajouter `class-coverage-net` à `make test` (§4.2 : hors suite ; ~4 min).
- Modifier `docs/plan-3.md` au-delà de ce que l'autopilote y consigne d'ordinaire en fin d'étape.
- Lancer l'appli raylib : jamais.

## Further Notes

- La mesure de cadrage a pris ~4 min (15 rejeux, un processus chacun). Le vérificateur doit la relancer au moins une fois sur le code final.
- Rares, à surveiller pendant toute la phase 1a (un seul rejeu les construit) : `nl` 1, `os` 1, `mx` 3, `fr` 5, `hj` 5.
- À partir de cette étape, chaque PRD de 1a peut écrire son critère §7 comme « `make class-coverage-net` rend 0 » : la comparaison à la porte 0.4 est dans la cible.
