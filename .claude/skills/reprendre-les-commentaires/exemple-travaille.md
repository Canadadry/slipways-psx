# L'exemple travaillé — un port C d'un cart PICO-8

Le chantier dont cette skill est tirée. À lire comme un exemple, pas comme la
définition : les noms de fichiers, la notion de glossaire et la référence
« le cart » sont propres à ce dépôt.

## Le point de départ

**18 544 lignes de commentaire pour 39 274 lignes de code — 47 %.** Presque tout
était du récit de chantier : numéros de phase et de « chunk », renvois aux PRD,
code Lua du cart recopié au-dessus de chaque fonction portée, justifications de
découpage de modules, historiques de correctifs abandonnés. Résultat :
**2 966 lignes, 12 %**, sans changer un comportement, suite de tests verte à
chaque étape.

Le code Lua recopié en commentaire avait une raison à l'époque : c'était la
méthode de vérification de la phase de portage. Elle a été remplacée par le rejeu
enregistré (9 729 frames comparées) — le commentaire ne vérifiait plus rien, il
coûtait seulement.

## Les trois artefacts produits

- `CLAUDE.md`, section « Commentaires » : la règle des quatre destinations, dans
  le fichier d'instructions que toute session relit.
- `docs/glossaire-noms.md` : 191 symboles, une ligne chacun. Le cart est minifié
  (`jo`, `bv`, `cg`) et le port translittère littéralement (`pico8_jo_kj`) : le
  **nom** était donc déjà dans le code, c'est le **sens** qui manquait, et il ne
  se déduisait ni du C ni du Lua.
- `docs/trous-de-portage.md` : les écarts numérotés, revérifiés contre le code un
  par un. Le code n'y renvoie que par `/* GAP-7 */`.

## Phase 1

Construite en lisant les 13 195 lignes d'en-tête pendant que la prose était encore
là. Trois constats sont tombés à ce moment-là, aucun n'aurait survécu à la
phase 2 :

- une affirmation périmée (« cette table est vide face au vrai `map.json` ») que
  le plan actait comme résolue depuis longtemps ;
- **GAP-4 devenu atteignable sans que personne le note** : le commentaire disait
  « ce sera un vrai trou le jour où `spr` sera enregistré comme constructeur », et
  l'initialisation l'enregistre bien ;
- **GAP-1 contredisant une décision actée** du plan (« pas de flottant natif »)
  alors que `pico8_fixed_cos/_sin` passent par le `cos()` de l'hôte.

Le recâblage du processus, dans le même commit : la règle dans `CLAUDE.md`, et
l'étape « Log the finding » retirée de la skill `fix-bug`, qui écrivait un
paragraphe de journal dans le dépôt à chaque correction de bug — et imposait un
second commit `docs:` pour ça. Sans ce point, la prose repousse.

## Phase 2 — ce que les vagues ont donné

Une vingtaine de modules, en vagues de 4 à 6 agents. Quelques résultats qui
illustrent chaque règle :

- **L'assert qui a cassé la suite** (module `ob`, le parseur de specs) : exigeait
  une virgule finale ; règle vraie, validée hors ligne sur 58 specs, fausse d'un
  cran en réalité — les specs décodées depuis la mémoire finissent par **deux**
  virgules. Retiré, règle remise en prose avec ce que la mesure a établi. Aucune
  tentative de rafistoler la condition : un second invariant mal posé coûte un
  autre tour, et la règle autorise la prose.
- **Le refus qui a trouvé un bug** (tirage pondéré des planètes) : l'agent voulait
  `assert(oc.count > 0)`, a calculé la marge avant d'écrire — 66 tickets pour
  90 planètes possibles — et a renoncé. En vérifiant, c'était pire que sa
  conclusion : pool vide, l'index tombe à 0, le code lit `items[0]` sur une liste
  dont le tableau peut être NULL. Consigné en GAP-16.
- **Les asserts écartés par la plomberie** : `i >= 1` dans `pico8_sub` alors que
  `unstash()` l'appelle avec 0 à chaque fin de chaîne ; `frames != NULL` dans
  `pico8_input_save` alors qu'un test passe NULL exprès. Les deux auraient cassé
  la suite. La rubrique « envisagés puis écartés » les a fait remonter.
- **Le seul assert établi par mesure** de sa vague : la longueur de l'alphabet de
  `unstash`, comptée hors du C — 51 caractères, saut de ligne compris. Se tromper
  d'un décale tout le décodage en silence.
- **Les fichiers de test** : 1 346 → 591 lignes, le taux de conservation le plus
  élevé du chantier, et c'est voulu. Trois fichiers laissés intacts. Deux
  affirmations qui se contredisaient elles-mêmes : un test annonçant « n'asserte
  rien » alors qu'il asserte six fois.
- **Les erreurs de l'orchestrateur trouvées par ses agents** : quatre, dont une
  entrée de glossaire qui attribuait un drapeau à l'écran de difficulté alors que
  son seul écrivain est le menu pause, et un en-tête annonçant 8 sites d'appel
  avec leurs numéros de ligne — il y en a 5.
- **Une perte assumée et signalée** : la dérivation d'un seuil de 5 000 crédits
  (34 lignes remontant quatre modules) réduite à sa conclusion, parce qu'elle est
  reconstituable depuis des sources nommées. Le plafond de 20 000 garde sa
  justification complète — le format 16.16 plafonne vers 32 768 — parce que
  celle-là n'est reconstituable de nulle part.

## Les ratios, vague par vague

Ils ne se ressemblent pas, et c'est normal : la plomberie portait peu de récit
(183 → 105), les modules de gameplay en portaient beaucoup (722 → 133 pour
`collect` + `ob`, 1 021 → 124 pour les trois modules de recherche), les tests
gardent leur « pourquoi cette valeur » (1 346 → 591). Un ratio bas n'est pas un
meilleur travail : c'est la nature du fichier.
