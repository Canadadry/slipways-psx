---
name: reprendre-les-commentaires
description: Ramène un dépôt noyé sous le récit de chantier (numéros de phase, renvois aux specs, code source recopié, historiques de correctifs abandonnés) à des commentaires qui ne portent plus que ce que le code ne dit pas — phase 1 séquentielle qui extrait ce qu'on ne pourrait plus reconstituer, phase 2 avec un agent par module sous une règle de quatre destinations. Use when le ratio commentaires/code est hors de proportion, quand on veut supprimer massivement du commentaire sans perdre ce qu'il contenait, quand un portage ou une génération par agents a laissé du journal de bord dans les sources, quand l'utilisateur parle de dégraisser, tailler, reprendre ou nettoyer les commentaires d'un dépôt, ou quand quelques fichiers écrits sous l'ancien régime arrivent dans un dépôt déjà repris (merge, branche longue, contribution extérieure) et qu'il faut les rattraper.
---

# Reprendre les commentaires

Un dépôt écrit par vagues d'agents accumule du **récit de chantier** dans ses commentaires : numéros de phase et de lot, renvois à des PRD, code de référence recopié, justifications de découpage, historiques de correctifs abandonnés. Ça se lit une fois et ça se paie à chaque lecture de fichier ensuite.

Cette skill ramène ce volume à ce qui a une raison de vivre dans le code, **sans changer un comportement**.

Exemple travaillé de bout en bout (un port C d'un cart PICO-8, 18 544 → 2 966 lignes de commentaire, 47 % → 12 %) : [exemple-travaille.md](exemple-travaille.md). Le reste de ce fichier est la méthode, à appliquer ailleurs.

## Deux régimes d'emploi — choisir avant de commencer

Un dépôt ne se reprend qu'une fois ; le rattrapage, lui, revient à chaque merge. Les deux usages ne suivent pas le même chemin.

**Campagne complète** — le dépôt n'a jamais été repris, le récit est partout. Phase 1 puis phase 2, dans l'ordre, ratio mesuré sur le dépôt entier.

**Rattrapage ponctuel** — le dépôt a déjà été repris, et quelques fichiers neufs sont arrivés sous l'ancien régime : un merge, une branche longue, une contribution extérieure. C'est l'usage le plus fréquent une fois la campagne passée. Alors :

- **phase 1 sautée.** Le glossaire et le fichier d'écarts existent : ils se consultent en lecture seule, et comme en campagne l'agent n'y écrit pas — il propose ses ajouts et ses corrections dans son rapport.
- **la règle des quatre destinations est déjà dans le fichier d'instructions du dépôt** (`CLAUDE.md` ou équivalent). L'agent la lit là-bas ; la consigne n'a pas à la recopier, seulement à y renvoyer.
- **ratio mesuré par fichier**, jamais sur le dépôt : sur deux fichiers, la mesure globale ne bouge pas et ne dit rien.
- **un seul agent** si les fichiers tiennent dans ce qu'une vague lui donnerait. La règle « l'en-tête et le corps ensemble » vaut toujours.

Dépôt à moitié repris — un glossaire mais pas de fichier d'écarts, ou l'inverse : c'est un rattrapage quand même. On ne fabrique pas la moitié manquante par symétrie ; on la crée le jour où un écart se présente vraiment.

## Prérequis, non négociable

**Une suite de tests qui atteste le comportement, et qui est verte avant de commencer.** La méthode ajoute des `assert` ; un assert faux fait abandonner le processus. Sur le chantier de référence, un agent en a posé un qui faisait tomber toute la suite (voir plus bas). C'est le filet qui l'a attrapé, pas la relecture.

Sans suite de tests : n'ajoutez aucun assert, la destination 3 se réduit à sa variante prose. Dites-le à l'utilisateur plutôt que de bricoler.

**C'est une obligation de l'orchestrateur, pas une case que l'agent coche** : l'agent n'a pas le droit de lancer la suite (le répertoire de build est partagé). Le dire explicitement dans la consigne, sinon l'agent croit travailler sans filet — ou pire, croit en avoir un là où il n'y en a pas.

**Et il y a des fichiers que la suite n'atteint pas.** Sur le chantier de référence, deux d'entre eux incluaient une bibliothèque graphique non vendorisée, dans un dépôt qui interdit par ailleurs d'ouvrir une fenêtre : aucune cible lançable ne les construit. Un assert faux y survivrait à toute la suite et n'exploserait qu'entre les mains d'un humain. **Dans un fichier qu'aucune cible lançable n'atteint, on ne pose aucun assert, même établi.** Recensez ces fichiers avant d'ouvrir la phase 2 et dites-le à leurs agents.

Relevez le ratio de départ, il sert de mesure de progrès à chaque vague :

```sh
# à adapter au langage ; ici commentaires C/C++ vs lignes de code
cloc --by-file src/ | tail -5
```

**Donnez cette commande aux agents, elle est la définition.** « Ligne de commentaire » a au moins trois sens (ligne entièrement commentaire, ligne en contenant, définition propre à `cloc`) : sans commande canonique, chaque agent écrit son compteur et les chiffres ne sont comparables ni entre eux ni avec le vôtre. C'est la seule mesure que la méthode présente comme objective ; qu'elle dépende de l'outil de chacun la vide de son sens.

## La règle : quatre destinations

Tout commentaire rencontré va dans **une** de ces quatre destinations, sinon il part.

1. **Le sens d'un nom** → un fichier de glossaire, une ligne par symbole. Le code ne le répète pas.
2. **Un écart connu avec la référence**, ou un morceau volontairement non fait → une entrée numérotée dans un fichier d'écarts. Le code n'y renvoie que par un **marqueur nu**, `/* TROU-7 */`, seul sur sa ligne au-dessus de la déclaration concernée — pas de résumé à côté, il périmerait.
3. **Un piège qu'un lecteur futur réintroduirait** → `assert(cond && "message court")` au site exact si c'est vérifiable à l'exécution, **sinon 40 mots maximum**.
4. **Un contrat que ni le nom ni le glossaire ne portent** — qui possède la mémoire, qui la libère, ce que rend la fonction quand le champ est absent, ce qu'un NULL déclenche → une phrase en en-tête. Une exception, à nommer pour qu'elle ne serve pas de passe-droit : **un format de fichier ou un protocole que quelque chose d'extérieur au dépôt doit pouvoir lire** est un contrat long légitime — il n'a nulle part ailleurs où vivre. Tout le reste tient en une phrase, ou part dans un `docs/format-*.md`.

Tout le reste va dans le **message de commit**. Git le garde, et personne ne le paie en jetons à chaque lecture du fichier. `TODO` reste autorisé : c'est l'aveu qu'un cas n'est pas traité.

## Phase 1 — campagne complète seulement, séquentielle, pendant que la prose est encore là

L'orchestrateur la fait seul, avant tout agent. Elle extrait ce qu'on ne pourrait plus reconstituer une fois les commentaires taillés.

1. **Le glossaire.** Utile quand les identifiants ne portent pas leur sens : code minifié translittéré, symboles hérités d'un format binaire, abréviations d'un domaine. Le construire en lisant tous les en-têtes, une ligne par symbole, et vérifier mécaniquement que chaque symbole cité existe encore. *Transposition* : si les noms sont déjà parlants, ce fichier n'a pas lieu d'être — ne le fabriquez pas par symétrie. Il peut devenir un glossaire de vocabulaire métier, ou rien.
2. **Les écarts numérotés.** Une entrée par divergence avec la référence (autre implémentation, spec, système remplacé) ou par manque assumé. **Revérifier chaque entrée contre le code** plutôt que recopier le commentaire : c'est là qu'on trouve les affirmations périmées, et parfois un écart devenu atteignable que personne n'a noté.
3. **Supprimer les journaux de chantier** devenus redondants avec git, et patcher tous les renvois pendants.
4. **Recâbler le processus qui produisait la prose** — sinon elle repousse. Concrètement : écrire la règle des quatre destinations dans le fichier d'instructions du dépôt (`CLAUDE.md` ou équivalent), et **corriger les skills qui écrivaient du récit**. Sur le chantier de référence, une skill de correction de bug ajoutait un paragraphe de journal à chaque passage ; l'étape a été retirée, le constat va au message de commit.

Commiter la phase 1 avant d'ouvrir la phase 2.

## Le récit ne vit pas que dans les commentaires

Une chaîne destinée à un humain — bannière, message d'aide, ligne d'état — est de la documentation qui s'exécute, et elle périme exactement comme un commentaire. Sur le chantier de référence, un mode de lecture rejeté avait disparu des commentaires **et** du code, mais survivait dans la bannière que le programme imprime à chaque lancement : la seule phrase que lit l'humain avant de regarder ce qu'il va signer lui promettait un comportement qui n'existe plus.

D'où la règle, à mettre dans chaque consigne : **une chaîne destinée à l'humain se vérifie comme un commentaire, et une divergence se signale — elle ne se corrige jamais dans cette passe.** C'est du code : le diff cesserait d'être purement soustractif, et le correctif mérite son propre commit `fix:`.

## Phase 2 — un agent par module

En rattrapage ponctuel, c'est ici qu'on entre directement.

Un agent, un module, et **l'en-tête et le corps ensemble** (`.h` + `.c`, interface + implémentation) : le piège est souvent décrit dans l'en-tête alors que l'assert qui le remplace doit vivre dans le corps. Un agent qui ne voit qu'un des deux fichiers perd la moitié de la transformation.

La consigne à copier : [consigne-agent.md](consigne-agent.md). Le rapport à exiger : [modele-rapport.md](modele-rapport.md).

Grouper les modules par vagues cohérentes (4 à 6 agents), une vague à la fois.

## La règle centrale : un assert doit être vérifiable, pas seulement plausible

Les agents ne peuvent pas lancer la compilation — ils partagent le répertoire de build — donc ils **raisonnent au lieu de mesurer**, et un raisonnement sérieux encode parfois une hypothèse plutôt qu'un invariant.

Ce qui est arrivé : un agent a posé un assert exigeant qu'une spec de données se termine par une virgule. La règle était vraie, et c'était le piège le plus coûteux du dépôt. Il l'avait validée sérieusement — réimplémentation de la machine à états du parseur hors ligne, passée sur les 58 specs recensées. Le premier `make test` a dit le contraire : mesurées, les specs décodées depuis la mémoire finissent par **deux** virgules et l'index n'y atterrit pas au même endroit. La condition était fausse d'un cran. Assert retiré, règle remise en prose.

D'où la consigne, à mettre en gras dans la prompt de chaque agent :

> **S'il ne peut pas établir qu'un assert tient autrement qu'en raisonnant, il écrit 40 mots.** Un assert de trop coûte plus cher qu'un commentaire de trop.

Établir, ça veut dire : énumérer exhaustivement les sites d'appel, mesurer une longueur ou un contenu hors du langage cible, ou prouver l'invariant **localement, sans hypothèse sur les appelants**. Pas : « je ne vois pas comment ça pourrait arriver ».

Les vagues suivantes ont refusé cinq asserts pour cette raison, et l'un d'eux a produit la meilleure trouvaille du chantier : l'agent a calculé la marge avant d'écrire (66 tickets dans un pool pour 90 tirages possibles), le calcul a dit non — et ce « non » était un bug de lecture hors bornes que personne n'avait vu. **Demander explicitement la liste des asserts envisagés puis écartés** : elle vaut autant que ceux qui sont posés.

## Les autres règles, toutes vérifiées sur le terrain

- **Vérifier toute affirmation de statut avant de la croire.** « Pas encore porté », « jamais exercé », « aucun lecteur », « code mort » : une cinquantaine se sont révélées fausses. Elles ne mentaient pas à l'écriture, elles ont vieilli — et c'est ce qui les rend dangereuses, plusieurs détournaient le regard du seul endroit qu'il fallait examiner. Une affirmation de statut se vérifie (grep, compte d'appelants) ou se supprime.
- **Un fichier de test a sa propre règle.** Son commentaire porte *pourquoi cette valeur-là*, ce qu'aucun commentaire de code ne porte : une assertion sur un pixel, une fourchette, un index de frame est irrécupérable sans sa raison. Le taux de conservation y est légitimement plus haut. **Tu ne modifies ni ne supprimes une assertion existante** : ni sa valeur, ni sa présence. Changer ce qu'un test prouve dans un commit de nettoyage, c'est le faire passer sans relecture. En **ajouter** une est permis si elle est prouvée comme n'importe quel assert : elle est listée au rapport, et l'orchestrateur en fait un commit `test:` à côté du commit `docs:`.
- **Les agents n'écrivent ni dans le glossaire ni dans le fichier d'écarts.** Plusieurs se les partageraient. Ils *signalent* les entrées manquantes et les erreurs dans leur rapport, l'orchestrateur applique par lots. Sur le chantier de référence : une quarantaine d'entrées ajoutées ainsi, et **quatre erreurs de l'orchestrateur lui-même** trouvées par ses agents.
- **La rubrique « ce que tu as hésité à supprimer » est la plus utile du rapport.** C'est elle qui fait remonter les vrais écarts, les erreurs du glossaire et les bugs déclarés corrigés qui ne le sont qu'à moitié. La demander explicitement, avec la consigne de **ne pas la remplir par politesse ni de l'omettre**.

## Rôle de l'orchestrateur

0. **Le contrôle mécanique marche partout, y compris là où la compilation échoue.** `gcc -fpreprocessed -dD -E -P` ne résout pas les `#include` : il se contente de retirer les commentaires. Un fichier qui n'est pas compilable ici — dépendance non vendorisée, en-tête absent — se vérifie donc quand même, octet pour octet. Ne dites jamais à un agent de « relire le diff à la main » pour ces fichiers-là : vous lui retirez sa seule preuve. Second filet gratuit : le nombre de lignes de code doit être identique avant et après.
1. **Compiler chaque module en isolation** dès que son agent rend la main (`make src/obj/module.o` ou l'équivalent). Ça attrape l'`#include <assert.h>` manquant et les commentaires imbriqués.
2. **Ne lancer la suite complète que sur un arbre stabilisé** — tous les agents de la vague rendus. Une compilation sur un arbre à moitié réécrit ne dit rien, et c'est précisément ce qui a permis d'attribuer correctement la panne de l'assert.
3. **Commiter module par module**, avec le récit dans le message : le ratio avant/après, les asserts posés et leur mode de preuve, les affirmations périmées démontées, les corrections du glossaire.
4. **Séparer les commits qui ajoutent des `abort()`** (un assert en est un) de ceux qui ne font que supprimer du texte. Un commit purement soustractif doit pouvoir se relire comme tel, et se révoquer sans rien emporter.
5. **Recommander le remplacement par plage de lignes**, qui refuse si la plage contient autre chose que du commentaire, plutôt que le remplacement de chaînes exactes recopiées depuis une lecture. Le premier ne peut pas toucher au code et échoue bruyamment ; le second se trompe d'un retour à la ligne et colle du texte approximatif.
6. **Relever le ratio à chaque vague.** C'est la mesure de progrès utile, et la seule qui ne dépende pas d'un jugement.

## Ce que la méthode ne garantit pas

Elle a produit une panne : un assert faux, qui faisait abandonner toute la suite de tests. Il a été écrit par un agent consciencieux, qui avait validé sa règle hors ligne avant de la poser. Ce n'est pas la relecture qui l'a attrapé, c'est le premier `make test` de la vague suivante.

La méthode est donc sûre **dans la mesure exacte où la suite de tests l'est**. Si elle ne couvre pas un chemin, un assert faux sur ce chemin survivra jusqu'à un utilisateur. C'est l'argument pour le prérequis en tête de ce fichier, et pour la règle de l'assert vérifiable.
