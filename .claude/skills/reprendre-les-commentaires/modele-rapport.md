# Modèle de rapport d'agent

À coller en fin de consigne, ou à faire lire à l'agent. L'orchestrateur en tire le
message de commit, les lots d'entrées de glossaire et d'écarts, et la décision de
relancer un module. Il lit **les rubriques 7 et 8 en premier** : ce sont elles qui
portent les résultats.

**Une rubrique vide n'est pas une rubrique ratée**, et ça vaut pour les neuf. Sur
un rattrapage de quelques fichiers, 3, 5 et 6 sont souvent vides par construction.
Meubler est le seul vrai échec.

---

## 1. Ratio

```
<module>.h   <N> → <n> lignes de commentaire   (<X> lignes de code)
<module>.c   <N> → <n>                          (<X>)

contrôle : diff préprocesseur avant/après <identique | écart, lequel>
            lignes de code <identiques | écart>
```

Le contrôle va ici, à côté du ratio : c'est la première chose que l'orchestrateur
doit lire, et la seule preuve que le nettoyage n'a rien cassé.

## 2. Ce que j'ai gardé, et sous quelle destination

Une ligne par commentaire conservé, avec sa destination (3 = piège, 4 = contrat) :

- `fonction()` — contrat : qui libère le tampon rendu.
- `autre()` — piège, 40 mots : <résumé en une ligne>.

## 3. Asserts posés

Vide sur un fichier de test, et vide sur un fichier qu'aucune cible lançable
n'atteint : dans les deux cas la passe n'en pose pas.

Un bloc par assert. **Le mode de preuve est obligatoire** : énumération des sites
d'appel, mesure, ou preuve locale sans hypothèse sur les appelants. « Je ne vois
pas comment ça pourrait arriver » n'est pas un mode de preuve.

```
site      : <fichier:ligne>
condition : assert(<cond> && "<message>")
remplace  : <le commentaire qui disait le piège>
preuve    : <énumération / mesure / locale> — <le détail qui l'établit>
```

## 4. Asserts envisagés puis écartés

Aussi important que la rubrique précédente. Un bloc par candidat :

```
condition envisagée : <cond>
pourquoi écarté     : <ce que l'examen a montré>
remplacé par        : <les 40 mots, ou rien>
```

Si le calcul de marge a dit non, donne les nombres. Ça a déjà révélé un bug.

## 5. Entrées de glossaire à ajouter ou corriger

Tu n'écris pas dans le glossaire. Propose, l'orchestrateur applique par lots.

- ajout : `<symbole>` — <sens, une ligne>
- correction : `<symbole>` — le glossaire dit « <X> », le code dit « <Y> », vérifié
  par <…>

## 6. Écarts à consigner

- TROU-? — <titre> — <2 à 4 lignes> — marqueur posé à `<fichier:ligne>`
- TROU-<n> existant — fichier à ajouter à la liste : `<chemin>`

## 7. Affirmations de statut vérifiées fausses

Une ligne chacune : ce que le commentaire affirmait, ce que la vérification a
montré, comment tu l'as vérifié.

## 8. Ce que j'ai hésité à supprimer

**Obligatoire. Ne pas remplir par politesse, ne pas omettre.**

Ce qui t'a fait hésiter et que tu as supprimé quand même, ou gardé sans en être
sûr : une justification longue mais reconstituable de nulle part ailleurs, une
connaissance qui risque de ne plus être écrite nulle part après ton passage, un
commentaire qui contredit le code, un bug annoncé corrigé dont tu ne retrouves
que la moitié du correctif. Dis ce qui se perd et à quel prix on le
reconstituerait.

C'est la rubrique qui, sur le chantier de référence, a fait remonter les vrais
écarts, les erreurs du glossaire et un bug déclaré corrigé qui ne l'était qu'à
moitié.

## 9. Ce qui reste en l'état, et pourquoi

Ce que tu n'as pas pu reprendre sans sortir de ton mandat (toucher une assertion,
modifier du code, arbitrer un écart). Avec ce qu'il faudrait décider.
