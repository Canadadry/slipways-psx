---
title: "Plan 2, Phase C0 : classement des 76 casts, table de propriété, observation, fraîcheur de l'inventaire"
description: "Avant de convertir le moindre site flottant, chaque cast doit porter une population sourcée, chaque jeton de l'inventaire un propriétaire unique (ou une décision ouverte nommée), et l'inventaire versionné ne doit plus pouvoir périmer en silence."
status: needs-triage
---

## Problem Statement

La Phase B a livré trois rapports versionnés : l'inventaire (165 jetons flottants du cœur, dont 76 casts `(int)pico8_fixed_to_double(…)`), la couverture sous le filet et la garde `-mgeneral-regs-only`. Elle s'est arrêtée volontairement avant le classement : les 76 casts portent tous « à auditer » et les comptes des populations (a), (b), (c) valent 0.

La Phase C ne peut pas commencer en l'état, pour quatre raisons.

1. **Aucun cast n'a de population.** C1 ne convertit que les sites (a) et (b) dont le signe est prouvé ≥ 0. D1 prend (c) et les sites (a) qui peuvent recevoir un négatif. Tant que personne n'a rattaché chaque cast à un `flr(` de `pico8/lua.lua` ou écrit sa preuve de signe, C1 n'a pas de liste de travail. Or classer au jugé violerait le garde-fou du §9 : « aucune valeur non sourcée ». Et conserver la troncature par défaut n'est pas neutre non plus : « ne pas choisir est aussi un choix ».
2. **Des jetons n'ont pas de propriétaire.** Aucune étape de C, D ou E ne revendique les 2 `sin`/`cos` de la libc dans `pico8_fixed_cos`/`_sin`, ni les définitions de `pico8_fixed_from_double`/`to_double`. Le §5 interdit de réécrire ces fonctions. Le §6 et la Phase E exigent zéro flottant et le retrait de `-lm`. Le plan consigne cette contradiction comme **décision ouverte, à trancher en C0 par l'humain**. D'autres jetons (les mots de commentaire, les 18 constantes dont la valeur ne change pas) n'ont pas d'étape nommée. Un jeton qu'on découvre sans propriétaire en Phase E, c'est une phase qui ne ferme pas.
3. **L'inventaire peut périmer.** `make float-inventory` est hors de la suite. Une modification du cœur qui ajoute, retire ou déplace un jeton laisse `tools/float_inventory.txt` faux, et rien ne le signale. Les sorties de phase de C et D s'énoncent en « compte de X = 0 d'après l'inventaire » : un rapport périmé les rendrait fausses.
4. **La clé d'identité ne survit pas à C1.** L'inventaire identifie un site par (fichier, fonction, catégorie, rang). Convertir un cast décale le rang de ses frères de la même fonction : `pico8_mz_kj` en a 8, `pico8_jo_kj` 7, `pico8_pi_spr_constructor` 5. Une annotation indexée sur ce rang glisserait sur le mauvais cast au premier commit de C1.

S'y ajoute un constat fait en préparant cette étape : 8 des 76 casts se trouvent dans des aides locales du type `pico8_*_to_int(x)`. Ces aides sont appelées de 2 à une dizaine de sites chacune (`pico8_ui_to_int` existe dans deux fichiers). Pour ces casts, ni la correspondance avec un `flr(` ni la preuve de signe ne peuvent se lire au site du cast : elles portent sur l'ensemble des appelants.

## Solution

C0 ne modifie **aucun site flottant**. Il produit quatre choses, et le code du jeu en sort inchangé : `frames-check` reste identique, l'inventaire aussi, à la colonne population près.

1. **Un fichier d'annotation versionné, édité à la main.** Il contient une ligne par cast : sa clé stable (fichier, fonction, extrait, occurrence parmi les extraits identiques de la fonction), sa population, son étape propriétaire et une ligne de justification. L'inventaire le lit. Il remplit la colonne population et les comptes (a), (b), (c), « à auditer », et ajoute un compte par étape pour les casts. Une annotation orpheline, un cast non annoté, une population (a) sans renvoi vérifiable à un `flr(` du Lua ou une étape incompatible avec la population font échouer l'inventaire.
2. **Un build d'observation, hors suite.** Il rejoue tout le filet (`FRAMES_EVENTS`) avec une instrumentation compilée seulement sous un drapeau dédié, sur le modèle de `-DPICO8_REF_AUDIT`. Il consigne, pour chaque cast, la valeur brute minimale vue et le nombre de passages, dans un rapport versionné joint à l'inventaire comme l'est la couverture. Le rapport signale en erreur toute contradiction : un cast annoté (b) vu négatif.
3. **Le classement effectif des 76 casts**, écrit dans le fichier d'annotation. Chaque classement est sourcé ou reste « à auditer ».
4. **La table de propriété**, consignée dans `docs/plan-2.md`. Elle affecte chaque catégorie ou sous-groupe de l'inventaire à exactement une étape (C1, C2b, D1–D4, E), et la somme par étape vaut 165. Les jetons de la décision ouverte y figurent sur une ligne explicite **« sans propriétaire — décision ouverte »**, sans que C0 la tranche.

Enfin, `make test` gagne une **vérification de fraîcheur** : il régénère l'inventaire dans `tmp/` et le compare octet par octet à la copie versionnée. Un rapport périmé fait échouer la suite, avec un message qui nomme la commande à relancer.

## User Stories

1. En tant qu'agent qui ouvre C1, je veux une liste des casts de population (a) ou (b) dont l'étape propriétaire est C1, pour convertir exactement ces sites et aucun autre.
2. En tant qu'agent qui ouvrira D1, je veux la liste des casts dont l'étape propriétaire est D1 ((c), et (a) à signe non prouvé), pour savoir d'avance ce que D1 fera diverger.
3. En tant qu'agent, je veux que chaque cast porte exactement une population parmi (a), (b), (c) et « à auditer », pour qu'aucun cast ne soit classé deux fois ni oublié.
4. En tant qu'agent, je veux qu'un cast classé (a) cite une ligne de `pico8/lua.lua` qui contient `flr(`, et que l'outil le vérifie, pour que la population (a) soit sourcée par construction et non affirmée.
5. En tant qu'agent, je veux qu'un cast classé (b) porte une preuve de signe en une ligne (compteur, `#t`, borne de boucle, identifiant de sprite, valeur déjà passée par `max(0,…)`…), pour que la bascule de C1 repose sur un raisonnement relisible.
6. En tant qu'agent, je veux qu'une observation « jamais négatif sur le filet » ne suffise jamais à classer un cast (b), pour qu'un indice empirique ne se déguise pas en preuve. Un filet ne voit que les parties qu'on a jouées.
7. En tant qu'agent, je veux qu'un cast sans preuve disponible reste « à auditer » avec une justification qui dit ce qui manque, pour que l'absence de source reste visible au lieu d'être comblée par une supposition.
8. En tant qu'agent, je veux qu'un cast NON COUVERT par le filet reste « à auditer » jusqu'à ce qu'un test ou une fixture l'atteigne (règle de B2, reprise telle quelle par C0), même quand un raisonnement le rattache à une population candidate. Le candidat peut être noté dans la justification.
9. En tant qu'agent, je veux que chaque cast (c) dise pourquoi il en est un (Lua muet, et la valeur peut être négative : coordonnée écran, delta, observation d'un négatif), pour que D1 hérite d'une raison et non d'un reste.
10. En tant qu'agent, je veux qu'un cast (a) dont l'argument peut être négatif soit affecté à D1 et non à C1, comme le plan le prescrit, parce que passer de la troncature à `flr` y est une correction et non une transformation neutre.
11. En tant qu'agent, je veux que la clé d'une annotation soit (fichier, fonction, extrait, occurrence parmi les extraits identiques de la fonction), pour que convertir un cast en C1 ne fasse pas glisser les annotations de ses frères.
12. En tant qu'agent, je veux qu'une annotation qui ne correspond plus à aucun cast soit une erreur, pour qu'un commit de C1 qui convertit un cast retire aussi son annotation dans le même commit.
13. En tant qu'agent, je veux qu'un cast présent dans le code sans annotation soit une erreur, pour qu'un cast ajouté ou réécrit ne passe pas inaperçu.
14. En tant qu'agent, je veux que l'étape portée par une annotation soit contrôlée contre sa population ((b) → C1, (c) → D1, (a) → C1 ou D1, « à auditer » → aucune), pour qu'une incohérence se voie à la génération et non en revue.
15. En tant qu'agent qui classe un cast placé dans une aide `pico8_*_to_int`, je veux que la justification couvre tous les appelants de l'aide (le signe et la correspondance Lua de chacun), pour qu'une preuve faite sur un appelant ne vaille pas pour les dix.
16. En tant qu'agent, je veux que les casts qui servent de clé de table (`key.as.fixed` dans les `get_by_value`/`set_by_value`, lookups…) portent une mention « clé de table » dans leur justification, pour que D4 retrouve sa liste de revue sans refaire la recherche.
17. En tant qu'agent, je veux que l'inventaire imprime la population et l'étape lues dans l'annotation à la place de « à auditer » codé en dur, et des comptes réels pour (a), (b), (c), « à auditer » et par étape, pour énoncer les sorties de C1 et D1 en « compte = 0 » d'après l'inventaire.
18. En tant qu'agent, je veux un build d'observation qui consigne, pour chaque cast, la valeur brute minimale reçue sur tout le filet et son nombre de passages, pour trier « vu négatif » / « jamais vu négatif » avant d'écrire les preuves.
19. En tant qu'agent, je veux que l'observation n'édite aucun site flottant et soit compilée hors du build par défaut, dans son propre arbre d'objets, pour que `frames-check` et l'inventaire restent identiques.
20. En tant qu'agent, je veux que le rapport d'observation soit joint à l'inventaire et signale en erreur tout cast annoté (b) vu négatif, pour qu'une preuve fausse soit réfutée par la mesure dès qu'elle l'est.
21. En tant qu'agent, je veux que deux casts d'une même ligne partagent leur observation et que le rapport le dise, pour qu'une valeur mesurée sur l'un ne soit pas attribuée à l'autre en silence.
22. En tant qu'agent, je veux qu'un cast jamais atteint sorte en NON COUVERT dans le rapport d'observation, comme dans celui de couverture, et jamais comme « jamais négatif ».
23. En tant que mainteneur, je veux que `make test` échoue quand l'inventaire versionné diffère de celui que produiraient les sources et l'annotation actuelles, et que le message nomme `make float-inventory`, pour qu'un rapport périmé ne survive pas à un commit.
24. En tant que mainteneur, je veux que la couverture, l'observation et la garde restent hors de `make test`, pour que la suite reste rapide et verte pendant C et D. La couverture est citée par chaque PRD d'étape.
25. En tant qu'humain qui doit trancher la décision ouverte, je veux la voir dans la table de propriété sous la forme d'une ligne « sans propriétaire — décision ouverte », avec ses 16 jetons comptés et les deux options du plan, pour trancher sur des chiffres et non sur une prose.
26. En tant qu'agent qui mènera E, je veux que la somme des comptes par étape de la table de propriété égale le total de l'inventaire, pour que rien ne soit découvert sans propriétaire en fermeture.
27. En tant qu'agent, je veux que la table de propriété dise, pour chaque affectation que le plan ne nomme pas mot pour mot, de quelle phrase du plan elle se déduit, pour qu'une déduction puisse être contestée au lieu de passer pour une décision.
28. En tant que mainteneur, je veux que le nouvel outil de jointure de l'observation et l'extension de l'inventaire soient testés sur des entrées fabriquées qui exercent ce qu'ils doivent empêcher, pour qu'une régression de l'outillage ne rende pas le classement aveugle.
29. En tant que mainteneur, je veux que la légende en tête du `Makefile` mentionne la nouvelle cible d'observation (« hors suite ») et la vérification de fraîcheur dans `make test`.
30. En tant qu'agent, je veux que `docs/plan-2.md` reçoive un « État de C0 » (comptes par population et par étape, nombre de casts « à auditer » et pourquoi), sur le modèle de l'« État de B2 », pour que C1 parte d'un état consigné.

## Implementation Decisions

- **Aucun site flottant modifié.** Aucune ligne du cœur ne change, sauf, si l'observation l'exige, une ligne sans jeton flottant qui protège la définition de `pico8_fixed_to_double` de l'expansion de la macro d'observation. Contrôle d'acceptation : `make frames-check` identique, et l'inventaire identique à la colonne population, aux comptes et à l'en-tête près.

- **Fichier d'annotation.** C'est un fichier texte versionné sous `tools/`, à côté de l'inventaire, et le seul des rapports flottants qu'on édite à la main (son en-tête le dit). Il contient une ligne par cast, avec le séparateur ` | ` des autres rapports : `fichier | fonction | extrait | occurrence | population | étape | justification`. Les lignes `#` sont des commentaires.
  - *Clé* : fichier, fonction, extrait (l'argument du cast, tel que l'inventaire l'imprime), occurrence (1, 2… parmi les casts **de même extrait** dans la même fonction). L'occurrence ne sert qu'à départager, par exemple les deux `s_val.as.fixed` de `pico8_jo_kj` ou les deux `tl` de `pico8_cr`. Le rang par catégorie de l'inventaire n'entre pas dans la clé.
  - *Population* : `(a)`, `(b)`, `(c)` ou `a auditer`.
  - *Étape* : `C1`, `D1` ou `-`. Règles vérifiées : (b) → C1, (c) → D1, (a) → C1 si la justification prouve aussi le signe ≥ 0, sinon D1, « à auditer » → `-`.
  - *Justification* : une ligne, non vide. Pour (a), elle contient un renvoi `lua.lua:<ligne>`, et l'outil vérifie que la ligne citée de `pico8/lua.lua` contient `flr(`. `lua.lua` est la source décompilée figée, ses numéros de ligne ne dérivent pas. Pour (b), c'est la preuve de signe. Pour (c), la raison. Pour « à auditer », ce qui manque (preuve introuvable, NON COUVERT, appelant non prouvé…).

- **Jointure dans l'inventaire, pas à côté.** Le script d'inventaire prend le fichier d'annotation en premier argument, et le `Makefile` le lui passe. La colonne population existe déjà dans l'inventaire, comme les lignes `compte population …`. Les y remplir est le changement minimal. Surtout, la vérification de fraîcheur de `make test` contrôle alors aussi les annotations : orphelines, manquantes, (a) non sourcé, étape incohérente. L'inventaire imprime en outre l'étape de chaque cast, avec une ligne `compte etape C1 N` / `D1 N` / `a auditer N`. Toute incohérence d'annotation sort en erreur avec un code distinct de ceux qui existent (2 : jeton hors catégorie ; 3 : somme fausse), et le rapport versionné n'est pas remplacé (la cible écrit à côté puis renomme, comme aujourd'hui). La justification n'est pas recopiée dans l'inventaire : elle vit dans l'annotation seule.

- **Aides `pico8_*_to_int`.** Le cast d'une aide est **un** site d'inventaire, donc une seule annotation. Sa population est la pire de ses appelants : (a) seulement si chaque appelant correspond à un `flr(` du Lua, (b) seulement si chaque appelant est prouvé ≥ 0, sinon (c) ou « à auditer ». C0 ne scinde pas les aides : ce serait modifier des sites, et c'est à C1 ou D1 d'en décider s'ils le jugent utile.

- **Valeur entière par construction.** Certains arguments sont déjà entiers, par exemple `pico8_fixed_flr(ratio)` dans `pico8_mz_kj`. Pour eux, troncature et `flr` coïncident quel que soit le signe. Le plan ne définit pas de population pour ce cas, et C0 n'en ajoute pas (§11.5 fige la découpe en trois). Le fait est noté dans la justification. Le cast suit la définition : (a) si le Lua dit `flr(` à cet endroit, (b) si le signe est prouvé, sinon (c). En (c), il part en D1, où `flr` le laisse inchangé : aucun résultat faux n'en découle.

- **Clés de table.** La conversion du jeton flottant d'un cast-clé suit sa population (C1 ou D1). Le plan route (b) vers C1 sans exception. La revue de la sémantique des clés fractionnaires (D4) ne supprime aucun jeton de l'inventaire : dans la table de propriété, D4 possède 0 jeton et reçoit la liste des casts marqués « clé de table ». Cette lecture est consignée dans la table comme une déduction, que l'humain peut contester.

- **Build d'observation.** Il a son propre drapeau de compilation, son propre arbre d'objets (règle « un arbre d'objets par jeu de flags » du `Makefile`) et son propre binaire de rejeu, lié aux mêmes sources que `pico8_replay`. La cible est hors suite et rejoue chaque entrée de `FRAMES_EVENTS` depuis la variable elle-même, comme `float-coverage`.
  - *Instrumentation sans toucher aux sites* : sous le drapeau seulement, l'en-tête du module fixe remplace `pico8_fixed_to_double(x)` par un appel d'enregistrement qui reçoit fichier, fonction et ligne, puis rend la même valeur. Le build par défaut est textuellement inchangé. La définition dans le module fixe est protégée de l'expansion sans éditer une ligne qui porte un jeton flottant.
  - *Enregistrement* : par (fichier, fonction, ligne), le minimum de la valeur brute 16.16 reçue et le nombre de passages. Il est déversé à la sortie vers un fichier nommé par une variable d'environnement, sur le modèle de `PICO8_REF_AUDIT_OUT`, et les déversements de plusieurs rejeux s'additionnent.
  - *Jointure* : un script POSIX `sh`/`awk` joint l'inventaire (casts seulement, avec leur population) aux enregistrements par (fichier, ligne). Il écrit un rapport versionné sous `tools/`, avec une ligne par cast : minimum brut en hexa, passages, et `vu negatif` / `jamais negatif` / `NON COUVERT`, plus `(ligne partagee)` quand deux casts partagent la ligne. Il se termine par des comptes. Il sort en erreur si un cast annoté (b) est `vu negatif` (CONTRADICTION) ou si l'inventaire ne contient aucun cast. Les appels non-cast de `pico8_fixed_to_double` (`pico8_dj`, `pico8_value_tostr`, `sin`/`cos`) sont enregistrés mais écartés par la jointure.
  - *Granularité* : la ligne, comme la couverture. Deux casts d'une même ligne (`pico8_lq_ol_item_draw`, `pico8_jo_kj`…) reçoivent le minimum commun. C'est prudent pour « jamais négatif », ambigu pour « vu négatif », et le rapport le dit.
  - *Statut de l'observation* : c'est un indice, pas une preuve. Une justification (b) peut citer l'observation en complément, jamais à sa place. La règle figure dans l'en-tête du fichier d'annotation et se vérifie en revue : l'outil ne peut pas juger une preuve. Le seul contrôle mécanique est la contradiction (b) × vu négatif.
  - Le rapport est reproductible (le rejeu est déterministe, §3.3) et nomme le compilateur, comme celui de couverture.

- **Fraîcheur dans `make test`.** Une cible ajoutée aux prérequis de `test` régénère l'inventaire (sources du cœur et annotation) dans `tmp/` et le compare à `tools/float_inventory.txt`. En cas d'écart, elle affiche un `diff` court et « relancer make float-inventory ». Elle dépend du script, de l'annotation et des sources du cœur.

- **Pas de cliquet sur la garde.** Le plan le laisse facultatif (« éventuellement »), et C0 ne le pose pas. C1 et C2 ne font que retirer du flottant, donc aucune étape proche ne risque d'en réintroduire dans un fichier propre. B2 a déjà écarté un cliquet dans `make test` pour la même raison. Compiler 58 fichiers de plus à chaque `make test` coûterait sans rien protéger. La garde reste relancée en début de phase, comme la « Règle pour C et D » le prescrit.

- **Table de propriété**, consignée dans `docs/plan-2.md` au §7 Phase C sous C0. Elle part de l'inventaire actuel. Les comptes de casts par étape viennent des lignes `compte etape` de l'inventaire annoté, pas d'un recopiage.

  | Sous-groupe de l'inventaire | Jetons | Étape | Source |
  |---|---|---|---|
  | constantes `from_double(<littéral>)` | 61 | C2b | Phase C : « les 34 constantes remplacées par les entiers de `probe_out.txt` », et C1 est « constantes inchangées ». C2 est la seule étape de C qui touche aux constantes. Les 18 valeurs inchangées y sont remplacées sans divergence : c'est une déduction |
  | casts, étape C1 | selon l'annotation | C1 | (a) à signe prouvé, (b) |
  | casts, étape D1 | selon l'annotation | D1 | (c), (a) à signe non prouvé |
  | casts « à auditer » | selon l'annotation | aucune, **bloqués** | règle de B2 : test ou fixture d'abord |
  | `%.4f`, et `double` + `to_double` de `pico8_value_tostr` | 3 | D2 | Phase D, `tostr` |
  | `pow`, `from_double(ec_d)`, et `double` + 2 `to_double` de `pico8_dj` | 5 | D3 | Phase D, `^0.75` |
  | revue des clés de table | 0 | D4 | ne retire aucun jeton ; hérite de la liste des casts « clé de table » |
  | 4 mots de commentaire (`float`, `double-free`, `pico8_fixed_from_double`, `double comma`) | 4 | E | déduction : le §6.1, sortie de E, les voit |
  | `sin`/`cos` libc, et `double`/`to_double`/`from_double` de `pico8_fixed_cos`/`_sin` | 2 + 8 | **sans propriétaire — décision ouverte** | §4/§5 contre §6/E ; TROU-1 |
  | définitions de `pico8_fixed_from_double`/`_to_double` | 6 | **sans propriétaire — décision ouverte** | idem |

  Total : 61 + 76 + 3 + 5 + 4 + 16 = 165. La ligne « décision ouverte » reprend les deux options du plan (ajuster §6/E, ou lever la clôture du §5 pour ces sites) et ne choisit pas. La note « Décision ouverte » de la Phase E y renvoie. Le titre de la Phase D (« les 5 sites flottants ») ne décrit que 3 des 5 réécrits : la table le signale comme une conséquence de la décision ouverte, sans corriger le titre.

- **Commentaires.** Les nouveaux scripts, en-têtes et cibles suivent `CLAUDE.md` : seuls restent le contrat d'un outil (ce qu'il rend, ses codes de sortie), les pièges et l'aide à la lecture d'une commande de `Makefile`. Le récit de chantier va dans les messages de commit.

## Implementation Chunks

1. **Fraîcheur de l'inventaire.** La cible de comparaison, ajoutée aux prérequis de `test`, et la légende du `Makefile`. Acceptation : `make test` vert sur l'arbre actuel, puis rouge si l'on ajoute un cast factice dans une copie du cœur (vérifié par un test sur sources fabriquées, jamais en éditant `src/`). Ce chunk vient en premier : sans lui, les chunks suivants peuvent laisser l'inventaire versionné périmé.
2. **Format d'annotation et jointure dans l'inventaire.** Le script d'inventaire prend l'annotation en argument, la clé stable, les contrôles (orpheline, manquante, (a) non sourcé contre `lua.lua`, étape incohérente), la colonne population et étape, les comptes. Le fichier d'annotation initial contient les 76 casts en `a auditer | - | non classe`. L'inventaire régénéré est alors identique à l'actuel, aux nouvelles colonnes et comptes près, et les tests existants sont mis à jour pour la nouvelle signature. Dépend du chunk 1.
3. **Build d'observation.** Le drapeau, l'arbre d'objets, l'enregistrement, la cible hors suite, le script de jointure, le rapport versionné et son test sur entrées fabriquées. Acceptation : `frames-check` identique, et deux exécutions donnent le même rapport. Dépend du chunk 2 (il lit les populations dans l'inventaire).
4. **Classement des 76 casts.** On remplit le fichier d'annotation cast par cast. On rattache la fonction C à son homologue Lua via `docs/glossaire-noms.md`, on cherche un `flr(` à l'endroit correspondant, sinon on prouve le signe, sinon (c) ou « à auditer ». Les aides `*_to_int` sont classées sur tous leurs appelants. Les 16 casts NON COUVERT restent « à auditer ». Puis on régénère l'inventaire et l'observation : zéro contradiction. Aucun compte attendu n'est fixé d'avance : ce que le Lua et les preuves donnent fait foi. Dépend des chunks 2 et 3.
5. **Table de propriété et consignation.** On écrit dans `docs/plan-2.md` la table ci-dessus (avec les comptes C1/D1/à auditer tirés de l'inventaire annoté), la ligne « sans propriétaire — décision ouverte » et un « État de C0 », et on met à jour la note de la Phase E pour qu'elle renvoie à la table. Acceptation : la somme de la table vaut le `compte total` de l'inventaire. Dépend du chunk 4.

## Testing Decisions

- Un bon test porte sur ce que l'outil rend (le rapport, le code de sortie) à partir d'entrées qu'il fabrique. Il n'inspecte ni les expressions `awk` ni les fichiers intermédiaires, et il exerce d'abord ce que l'outil doit empêcher.
- **Inventaire annoté** (extension de `tests/test_float_inventory.c`), sur une source fabriquée et un `lua.lua` fabriqué :
  - une annotation orpheline donne une erreur, et le rapport n'est pas écrit en entier ;
  - un cast non annoté donne une erreur ;
  - un (a) qui cite une ligne sans `flr(` donne une erreur ; un (a) qui cite une ligne avec `flr(` passe ;
  - un (b) avec l'étape D1 donne une erreur, un (c) avec C1 aussi ;
  - la clé est stable : on retire le premier de deux casts d'une même fonction, l'annotation du second s'applique toujours et celle du premier devient orpheline ;
  - deux casts de même extrait dans une fonction sont départagés par l'occurrence ;
  - les comptes par population et par étape sont justes ;
  - deux exécutions donnent la même sortie.
- **Fraîcheur** : le test pilote la comparaison sur un inventaire fabriqué et versionné, puis sur une source modifiée. Il vérifie l'échec, et que le message nomme `make float-inventory`. Précédent : `tests/test_frames_check.c`, un test C qui pilote un script sur des fichiers fabriqués.
- **Jointure d'observation** (nouveau test, dans `unit`), sur des enregistrements et un inventaire annoté fabriqués :
  - un minimum négatif donne `vu negatif`, un positif `jamais negatif` ;
  - une ligne absente des enregistrements donne `NON COUVERT` ;
  - deux casts d'une même ligne sont marqués `(ligne partagee)` ;
  - un (b) vu négatif donne une CONTRADICTION et une sortie en échec ;
  - un inventaire sans cast donne une erreur ;
  - des enregistrements de deux rejeux sont additionnés (passages) et « minimisés » (valeur).
- **L'instrumentation elle-même** n'a pas de test unitaire dédié. Son acceptation : le build d'observation compile et produit un rapport non vide sur le filet, `frames-check` reste identique dans le build par défaut, et l'inventaire ne change pas du fait de l'instrumentation. Précédent : `-DPICO8_REF_AUDIT`, `src/pico8_ref_audit.h`, `tools/ref_audit_report.sh`, dont la jointure « statique × dynamique » avec NOT COVERED est le modèle.
- **Le classement** n'est pas testé par un test unitaire, puisqu'un test ne peut pas juger une preuve. Ce qui est mécanique est vérifié par l'inventaire à chaque `make test` : complétude, (a) sourcés, cohérence étape/population. La contradiction avec la mesure est vérifiée par l'observation. Le reste relève de la revue du commit, qui porte le fichier d'annotation.
- Les tests des outils entrent dans `unit`, donc dans `make test`. Le build d'observation réel en reste dehors, comme `float-coverage` et `ref-audit`.

## Out of Scope

- **Toute conversion de site** : C1 (casts (a)/(b)), C2a/C2b (constantes), D1–D4. En particulier, aucun `(int)pico8_fixed_to_double` n'est remplacé et aucune constante n'est corrigée.
- **Le build d'audit de C1** (égalité ancienne/nouvelle expression, `x >= 0` asserté à chaque site (b)). L'observation de C0 en est le mode relevé : elle consigne, elle n'asserte pas l'égalité.
- **Trancher la décision ouverte** (`sin`/`cos` de `pico8_fixed`, définitions `from_double`/`to_double`). C0 la rend visible et comptée ; c'est à l'humain de la trancher.
- **Écrire des tests ou des fixtures** pour atteindre les 16 casts NON COUVERT. Ils restent « à auditer ». C'est à l'étape qui les convertit de les atteindre, par la règle de B2.
- **Scinder les aides `pico8_*_to_int`** par appelant.
- **Un cliquet de la garde dans `make test`** (voir Implementation Decisions).
- **Mettre la couverture dans `make test`** : elle reste manuelle, citée par chaque PRD d'étape.
- **Le volet `malloc`** (§8).
- `pico8_fixed_sin`/`cos`/`rnd` (clôture du §5), et lancer l'application raylib.

## Further Notes

- Répartition connue avant classement, d'après `tools/float_coverage.txt` : 60 casts couverts, 16 NON COUVERT (`pico8_ho_apply`, `pico8_hy_kt`, `pico8_cr` ×2, `pico8_ha2_my_dispatch`, `pico8_jo_hu`, `pico8_ga_kj` ×5, `pico8_research_lookup`, `pico8_pj_ph_map` ×2, les `set_by_value`/`get_by_value` de `economy_tiers` et `slipway_display`). Ces 16 resteront « à auditer » à la sortie de C0, et la table de propriété les montrera bloqués.
- `pico8/lua.lua` compte 17 `flr(` pour 76 casts. La population (a) est donc bornée à peu de sites, et une même ligne `flr(` peut sourcer plusieurs casts C (par exemple une aide et son appelant).
- Le minimum observé doit être relevé en valeur brute 16.16 (hexa), dans l'esprit du §5 : « bits bruts, jamais décimal ».
- L'occurrence dans la clé d'annotation repose sur l'extrait textuel. Si C1 réécrit l'argument d'un cast sans le convertir, son annotation devient orpheline, et c'est voulu : un argument changé demande une nouvelle justification.
