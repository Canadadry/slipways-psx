---
title: "Prérequis d'en-têtes manquants dans le Makefile"
description: "Les cibles ne listent qu'une partie des en-têtes dont elles dépendent, si bien qu'éditer un en-tête porteur de sémantique ne reconstruit pas les binaires qui en dépendent."
status: needs-triage
---

## Problem Statement

Les cibles du Makefile listent leurs sources `.c` exhaustivement, mais leurs en-têtes de façon inégale : certaines en nomment quelques-uns, la plupart n'en nomment aucun. La conséquence est qu'éditer un en-tête ne provoque pas la reconstruction des binaires qui l'incluent. `make` considère la cible à jour, relie un objet compilé contre l'ancienne version, et la suite passe au vert sur du code qui n'est plus celui du dépôt.

Tant que les en-têtes ne contenaient que des déclarations, le risque restait théorique : une signature changée sans recompilation échoue à l'édition de liens, bruyamment. Ce n'est plus le cas. L'en-tête du tag de type introduit par le correctif du segfault `lq` contient de la **sémantique compilée dans chaque objet** : des constantes magiques et une fonction `static inline`. Changer une de ces constantes sans reconstruire produit un binaire où une partie du code teste l'ancienne valeur et l'autre la nouvelle — c'est-à-dire exactement la confusion de type que ce tag existe pour éliminer, réintroduite par le système de construction et invisible à la lecture du source.

Cet en-tête n'est aujourd'hui listé comme prérequis que de la seule cible ajoutée avec lui. Les autres cibles l'incluent pourtant, transitivement, par les en-têtes de liste, de table et de classe.

Ce n'est pas une régression : le Makefile a toujours été ainsi, et l'habitude est cohérente avec elle-même. C'est le contenu des en-têtes qui a changé de nature.

## Solution

Faire en sorte qu'éditer un en-tête reconstruise ce qui en dépend, sans imposer de maintenir à la main une liste de prérequis par cible — le travail manuel étant précisément ce qui a produit l'état actuel.

L'approche retenue est la génération automatique des dépendances par le compilateur : celui-ci sait déjà quels en-têtes chaque unité de compilation a réellement ouverts, y compris transitivement, et sait les écrire dans un fichier que `make` peut inclure. La liste cesse alors d'être une déclaration d'intention pour devenir un fait mesuré, et elle ne peut plus dériver du code.

Cela suppose de compiler vers des objets intermédiaires plutôt que de passer chaque cible du source à l'exécutable en un seul appel, ce qui est un changement de forme réel du Makefile. Le bénéfice secondaire est une reconstruction nettement plus rapide : plusieurs cibles partagent aujourd'hui la quasi-totalité du moteur et la recompilent intégralement chacune de leur côté.

Une variante minimale existe — ajouter les en-têtes porteurs de sémantique aux cibles qui les incluent — et doit être chiffrée avant d'être écartée : elle est plus petite, mais elle perpétue la liste manuelle qui a déjà dérivé une fois.

## User Stories

1. En tant que développeur du portage, je veux qu'éditer un en-tête reconstruise les binaires qui l'incluent, afin que la suite teste le code que je viens d'écrire.
2. En tant que développeur du portage, je veux que changer une constante du tag de type ne puisse pas produire un binaire mélangeant l'ancienne et la nouvelle valeur, afin que le système de construction ne réintroduise pas la famille de bugs que ce tag élimine.
3. En tant que développeur du portage, je veux que la liste des dépendances soit dérivée du code plutôt que maintenue à la main, afin qu'elle ne puisse pas dériver silencieusement.
4. En tant que développeur du portage, je veux ne pas avoir à penser aux prérequis en ajoutant une cible, afin que l'oubli cesse d'être possible.
5. En tant que développeur du portage, je veux une reconstruction incrémentale, afin de ne pas recompiler tout le moteur pour chaque cible à chaque modification.
6. En tant qu'agent instruisant un bug, je veux pouvoir faire confiance à un `make test` vert, afin de ne pas conclure à tort d'une suite qui a testé un objet périmé.
7. En tant que développeur du portage, je veux que le premier `make` après un dépôt frais fonctionne sans fichier de dépendances préexistant, afin que la construction reste autonome.
8. En tant que développeur du portage, je veux que les artefacts intermédiaires restent dans le répertoire temporaire déjà ignoré par git, afin de ne pas polluer le dépôt.
9. En tant que développeur du portage, je veux que la cible liée à raylib continue de fonctionner à l'identique, afin que le changement de forme n'affecte pas le pilote interactif.
10. En tant que développeur du portage, je veux pouvoir vérifier que le mécanisme fonctionne réellement, afin de ne pas remplacer une liste fausse par une liste automatique fausse.

## Implementation Decisions

- Les dépendances sont générées par le compilateur pendant la compilation, pas par une passe d'analyse séparée : une passe séparée peut diverger du compilateur, ce qui recrée le problème un niveau plus haut.
- Les fichiers de dépendances sont inclus de façon tolérante à leur absence, pour qu'une construction depuis un dépôt propre fonctionne sans étape préalable.
- Les objets intermédiaires et les fichiers de dépendances vont dans le répertoire temporaire déjà utilisé par les binaires de test et déjà ignoré par git.
- Les cibles gardent leurs noms et leur sémantique actuels. En particulier, la cible de vérification et celle qui agrège les tests unitaires continuent de se comporter comme aujourd'hui du point de vue de l'appelant.
- Les réglages de compilation spécifiques à certaines cibles — instrumentation mémoire pour le test de régression du tag, instrumentation de comportement indéfini pour le test d'arithmétique fixe — doivent survivre au passage aux objets intermédiaires. Un objet compilé sans instrumentation ne doit jamais être partagé avec une cible instrumentée : c'est le piège principal de ce changement, puisqu'il produirait un binaire partiellement instrumenté dont le silence ne prouve rien.
- La cible liée à raylib n'est pas construite dans les environnements sans dépendance vendorisée ; son écriture doit rester inchangée dans sa forme pour ne pas être cassée sans possibilité de la tester ici.

## Testing Decisions

- Le comportement à tester n'est pas du code du jeu mais celui du système de construction : la vérification consiste à provoquer la situation qui échoue aujourd'hui et à observer qu'elle ne se produit plus.
- Cas de vérification principal : construire la suite, modifier une constante du tag de type sans toucher à aucun `.c`, relancer la construction, et vérifier que les binaires concernés ont bien été reconstruits. Aujourd'hui, la plupart ne le sont pas.
- Cas de vérification secondaire : la même manipulation sur un en-tête purement déclaratif, pour confirmer que le mécanisme ne dépend pas du contenu de l'en-tête.
- Vérification de non-régression : une construction depuis un état propre produit une suite verte, et une seconde construction immédiate ne recompile rien.
- Vérification de l'instrumentation : le test de régression du tag doit continuer de signaler un débordement mémoire si le correctif est retiré, et le test d'arithmétique fixe doit continuer d'échouer sur un comportement indéfini. Un test instrumenté devenu silencieux parce qu'il a été relié à des objets non instrumentés est le mode d'échec propre à ce PRD, et il est indétectable autrement que par cette vérification explicite.

## Out of Scope

- Le remplacement du système de construction par un autre outil.
- La réorganisation des sources ou des inclusions entre modules.
- La construction de la cible raylib dans un environnement sans dépendance vendorisée.
- Le son, hors périmètre du portage.

## Further Notes

Ce PRD n'est pas urgent : rien n'est cassé aujourd'hui, et l'oubli ne se manifeste qu'après l'édition d'un en-tête. Il est en revanche peu coûteux et retire une catégorie entière de résultats trompeurs — un `make test` vert obtenu sur un objet périmé est exactement le genre de silence qui a déjà coûté cher dans ce portage, le journal de portage notant qu'une suite qui ne démarrait pas avait survécu à un cycle complet de développement avant d'être remarquée.
