# Consigne d'agent — à adapter et coller

Un agent, un module. Remplacer les `<...>`. Les passages en gras sont ceux qui ont
été payés cher : ne pas les adoucir.

---

Tu reprends les commentaires de **<MODULE>** : `<chemin/module.h>` et
`<chemin/module.c>`. **Les deux ensemble** — le piège est souvent énoncé dans
l'en-tête alors que le contrôle qui le remplace doit vivre dans le corps.

Ce dépôt a accumulé du récit de chantier dans ses commentaires : numéros de phase
et de lot, renvois à des documents de conception, code de référence recopié,
justifications de découpage, historiques de correctifs abandonnés. On le retire.
**Tu ne changes aucun comportement** : pas une signature, pas une valeur, pas un
ordre d'appel. Le seul code que tu as le droit d'ajouter est `#include <assert.h>`
et des `assert`, sous les conditions ci-dessous.

## Les quatre destinations

Tout commentaire que tu lis va dans une de ces quatre destinations, sinon il part.

1. **Le sens d'un nom** → `<docs/glossaire.md>`. Tu **n'écris pas** dans ce
   fichier (d'autres agents travaillent en parallèle) : tu listes dans ton rapport
   les entrées à ajouter ou à corriger, une ligne chacune. Le code ne répète pas
   le glossaire.
2. **Un écart connu avec <LA RÉFÉRENCE>**, ou un morceau volontairement non fait →
   une entrée dans `<docs/ecarts.md>`. Là non plus tu n'écris pas : tu proposes
   l'entrée dans ton rapport. Dans le code, tu ne laisses qu'un **marqueur nu**
   `/* TROU-n */`, seul sur sa ligne au-dessus de la déclaration concernée. Pas de
   résumé à côté : il périmerait. Pour un écart non encore numéroté, écris
   `/* TROU-? */` et signale-le.
3. **Un piège qu'un lecteur futur réintroduirait** → un
   `assert(cond && "message court")` au site exact **si tu peux établir qu'il
   tient**, sinon **40 mots maximum** de prose. Lis la section suivante avant
   d'écrire le moindre assert.
4. **Un contrat que ni le nom ni le glossaire ne portent** — qui possède la
   mémoire, qui la libère, ce que rend la fonction quand le champ est absent, ce
   qu'un NULL déclenche, le format exact d'une sortie sur laquelle un test
   s'appuie → **une phrase** en en-tête.

Tout le reste va dans le message de commit, que j'écris. Tu n'as pas à le
préserver dans le fichier : dis-le-moi dans ton rapport et il sera gardé par git.
`TODO` reste autorisé : c'est l'aveu qu'un cas n'est pas traité.

## La règle de l'assert — la plus importante de cette consigne

**Un assert doit être vérifiable, pas seulement plausible.** Tu ne peux pas lancer
la compilation ni les tests (le répertoire de build est partagé), donc tu es
tenté de raisonner au lieu de mesurer. Un raisonnement sérieux encode parfois une
hypothèse plutôt qu'un invariant.

C'est arrivé : un agent a posé un assert exigeant qu'une spec de données se
termine par une virgule. La règle était vraie et c'était le piège le plus coûteux
du dépôt ; il l'avait validée en réimplémentant le parseur hors ligne et en la
passant sur les 58 specs recensées. **Le premier lancement de la suite a dit le
contraire** : mesurées, les specs décodées depuis la mémoire finissent par deux
virgules et l'index n'y atterrit pas au même endroit. La condition était fausse
d'un cran, et toute la suite abandonnait.

Donc : tu poses un assert seulement si tu peux l'**établir**, c'est-à-dire

- énumérer exhaustivement les sites d'appel, et montrer que tous le respectent ; ou
- **mesurer** (compter une longueur, décoder un contenu, exécuter un script hors
  du langage cible) plutôt que déduire ; ou
- prouver l'invariant **localement, sans aucune hypothèse sur les appelants**.

« Je ne vois pas comment ça pourrait arriver » n'est pas une preuve. Si tu ne peux
pas établir, tu écris 40 mots. **Un assert de trop coûte plus cher qu'un
commentaire de trop.**

Calcule la marge avant d'écrire, quand il y en a une (bornes, tailles de pool,
compteurs). Si le calcul dit non, ne pose pas l'assert — et **dis-le-moi** : sur
ce chantier, un « non » de ce genre était un bug de lecture hors bornes que
personne n'avait vu.

Dans ton rapport, liste les asserts **envisagés puis écartés** avec la raison.
Cette liste vaut autant que ceux que tu poses.

**Si ton fichier n'est atteint par aucune cible lançable ici** — il inclut une
dépendance non vendorisée, aucun test ne le construit — alors **tu ne poses aucun
assert du tout**, même établi. Le filet qui rattrape les asserts faux, c'est la
suite de tests ; là où elle ne passe pas, c'est un humain qui le rattrapera, bien
plus tard. Dis-le dans ton rapport et écris 40 mots.

**Un assert préexistant sans message** (`assert(cond)` nu, avec à côté le
commentaire qui serait exactement son message) est un cas fréquent sur un dépôt
déjà repris : la première passe a posé l'assert, la seconde trouve le commentaire
orphelin. La fusion est la bonne transformation **et c'est une modification de
code** : tu ne la fais pas. Tu gardes la prose et tu le signales.

## Vérifie toute affirmation de statut avant de la croire

« Pas encore porté », « jamais exercé », « aucun lecteur », « code mort », « ce
sera un problème le jour où X » : ces phrases ne mentaient pas quand elles ont
été écrites, elles ont vieilli. Une cinquantaine se sont révélées fausses sur ce
dépôt, et plusieurs détournaient le regard du seul endroit qu'il fallait
examiner. Chacune se vérifie (grep, compte d'appelants, lecture de la référence)
ou se supprime. Une affirmation vérifiée fausse est un résultat : signale-la.

**Ça vaut aussi pour les chaînes destinées à l'humain** — bannière, message
d'aide, ligne d'état : c'est de la documentation qui s'exécute, et elle périme
comme un commentaire. Sur ce chantier, un mode de lecture rejeté avait disparu du
code et des commentaires mais survivait dans la bannière du programme. **Tu la
vérifies, et une divergence se signale — jamais ne se corrige ici** : c'est du
code, ça mérite son propre commit `fix:`.

## Si ton module est un fichier de test

Règle différente. Le commentaire d'un test porte **pourquoi cette valeur-là** —
ce qu'aucun commentaire de code ne porte. Une assertion sur un pixel, une
fourchette, un index est irrécupérable sans sa raison : tu gardes ces raisons, le
taux de conservation sera légitimement plus élevé. Tu retires le récit de chantier
et les affirmations périmées, rien d'autre.

**Tu ne modifies ni ne supprimes une assertion existante** : ni sa valeur, ni sa présence. Tu peux en **ajouter** une si tu l'as prouvée — même exigence de preuve qu'ailleurs. Liste-la au rapport : l'orchestrateur en fera un commit `test:` séparé du commit `docs:`.

Une assertion que tu juges **fausse, faible ou trompeuse** — elle ne prouve pas ce
que son nom annonce — se signale dans ton rapport, avec ce qu'elle prouve
réellement. Elle ne se corrige pas ici.

Le curseur, puisque « taux plus élevé » n'en est pas un : **une phrase reste si
elle explique une valeur littérale, un choix de cas ou un contrat de tampon ; elle
part si elle raconte d'où vient la décision.**

## Contrôle avant de rendre la main

- Aucune déclaration, aucune ligne de code perdue. Compare la sortie du
  préprocesseur (`<gcc -fpreprocessed -dD -E -P>`) avant/après : elle doit être
  identique, aux `assert` près. **Cette vérification marche même sur un fichier
  que tu ne peux pas compiler** : l'option ne résout pas les `#include`, elle
  retire les commentaires. Ne te rabats jamais sur une relecture à l'œil.
- Le nombre de lignes de code est identique avant et après. Second filet, gratuit.
- Pas de commentaire de bloc imbriqué (un `/* */` ne s'imbrique pas).
- `#include <assert.h>` présent si tu as posé un assert.
- Compte les lignes de commentaire avant et après avec **la commande que donne la
  consigne**, pas avec la tienne : sinon ton chiffre n'est comparable à aucun autre.
- Signale les fichiers du même lot que tu as croisés sans les avoir en charge. Un
  rattrapage qui en laisse la moitié derrière lui n'en est pas un.

## Rapport

Rends ton rapport au format de `modele-rapport.md`. **La rubrique « ce que j'ai
hésité à supprimer » n'est pas optionnelle** : ne la remplis pas par politesse, ne
l'omets pas non plus.
