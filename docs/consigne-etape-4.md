# Consigne des agents de l'étape 4 du plan 5

**Base : `consigne-agent.md` et `modele-rapport.md` du skill `reprendre-les-commentaires`, lus tels quels ; là où ils se contredisent, ce fichier gagne.** Il ne liste que les points levés pour l'étape 4 (`docs/plan-5.md`, §6, entrée 4 et « Méthode de l'étape 4 »). Le skill n'est pas modifié : il sert aussi hors de ce chantier. Ce fichier vit tant que l'étape 4 reste à faire ; 4l le supprime dans son dernier commit.

Pour l'orchestrateur : remplacer `<MODULE>` et les chemins de la consigne du skill ; `<docs/glossaire.md>` est `docs/glossaire-noms.md` (lecture seule, point 3) ; `<docs/ecarts.md>` est `docs/trous-de-portage.md` (lecture seule, point 4) ; `<LA RÉFÉRENCE>` n'existe plus : le C est la référence (§2 du plan).

## 1. Ce que l'agent touche

- L'agent ne réécrit **que des commentaires**.
- Il ne touche ni code, ni chaîne, ni message d'`assert`, ni nom. Pour chacun, il propose dans son rapport le texte ou le nom neuf ; l'orchestrateur les applique dans les commits de chaînes et de locales.
- Sa preuve reste donc la plus simple : sortie du préprocesseur identique **sans exception** (la clause « aux `assert` près » du skill tombe) et nombre de lignes de code inchangé.

## 2. Langue et cart

- Commentaires en anglais (§2 du plan). **Ne jamais citer le cart** : un nom Lua devient le nom C (`docs/renommage.tsv`, colonnes `ancien` → `nouveau`, en lecture seule).
- « the cart does X » devient « X » ou disparaît. « nil », « Lua's true/truth » se disent en C : NULL, 0, absent, non nul.
- Un renvoi à un fichier, une fonction ou un champ disparu (`pico8_*.h`, `pico8_X_kind_new`…) est vérifié par `grep`, puis corrigé ou supprimé.
- Un nom court du cart encore porté par une locale ou un paramètre conservé (`ra`, `qa`, `me`…, registre D-29) n'est pas cité entre backticks : le compteur anti-cart le compterait. Le commentaire décrit son rôle (« the cursor position », « the year »).
- *Amendé par 4a.* Le relevé des citations compte en `short` tout mot nu d'une ou deux lettres qui est un ancien nom, même quand la prose ne pense pas au cart : un indice de boucle (« `states[i]`, i looked up… »), un nom de module (« the ui hover »), un mot de `awk` (`NR`), une marque de pluriel collée à un backtick (« `` `malloc`s `` »). On réécrit la phrase (« calls `malloc` », « game/ui.c »), on ne propose pas d'exception.
- *Amendé par 4b.* Une mention entre backticks d'un nom court qu'une ligne `renommé` du TSV porte comme ancien nom (`` `mem` ``, `` `gfx` `` depuis 4b, `` `ok` ``…) compte en `backtick`, même quand elle désigne un paramètre conservé d'une autre fonction : décrire le rôle (« PICO-8 memory », « the graphics state ») plutôt que citer le paramètre (D-36). Un nom de paramètre d'une ou deux lettres cité nu (`n`, `b`, `s`, `lo`/`hi`) se réécrit aussi (« the count », « the divisor »).
- *Amendé par 4a.* Remplacer un digramme par le nom C allonge souvent le commentaire (`entity_class.h` : 40 → 48 lignes). À l'étape 4, le ratio se relève mais n'est pas un objectif : la cible est le zéro des compteurs et un texte juste ; le récit de chantier part quand même au message de commit.

## 3. Destination 1 supprimée

Il n'y a plus de nom minifié. Le glossaire (`docs/glossaire-noms.md`) se lit seulement pour comprendre l'ancien commentaire, **jamais pour y écrire**. Un nom dont le sens ne se lit pas devient une proposition de renommage (rubrique 5 du rapport, ci-dessous), pas une ligne de glossaire.

## 4. Destination 2 : l'étiquette ne change pas

`/* TROU-n */` reste tel quel jusqu'à l'étape 5 (pas de `GAP-n`). Un écart nouveau se marque `/* TROU-? */` et se propose au rapport. `docs/trous-de-portage.md` est en lecture seule.

## 5. Destination 3 sans `assert` neuf

- **Aucun `assert` n'est ajouté à l'étape 4** : il changerait la sortie du préprocesseur et ne relève d'aucune des trois sortes de commit. La destination 3 se réduit à ses 40 mots. Un piège qui mériterait un `assert` va dans la rubrique 4 (« envisagés ») et, s'il est sérieux, dans le registre `docs/decisions-plan-5.md` par l'orchestrateur.
- Un `assert` existant garde sa condition. Son message fautif (français, cart) se propose réécrit, avec ses lecteurs (`must_abort_saying`, `expect_fatal`, `grep` du texte).
- Un `assert` nu dont le commentaire voisin serait le message reste une proposition, comme dans le skill.

## 6. Chaîne destinée à l'humain

- Une chaîne française ou qui cite le cart **se corrige** à l'étape 4, mais par l'orchestrateur, selon la règle de propriété des chaînes (§6 du plan : la sous-étape qui tient le site écrivain la traduit à tous ses sites).
- L'agent la liste avec trois choses : sa proposition, ses lecteurs (`grep`), et si un test la lit.
- Une chaîne qui décrit un comportement disparu reste un `fix:` à part, comme dans le skill.
- *Amendé par 4c1.* Le compteur de français ne voit pas les mots de contenu (`pleine`, `limite`, `octets`, `essai`, `bascule`…) : ses mots-outils seuls laissent passer « zone … pleine, limite 4 places ». L'agent lit donc chaque chaîne et chaque message d'`assert` de ses fichiers, et propose aussi ceux que le compteur ne marque pas.
- *Amendé par 4c1.* Les lecteurs d'un message fatal composé (`p8_fatal`, `expect_fatal`) se cherchent par `grep` de chaque fragment du gabarit, sur tout `tests/` : la PRD de 4c1 avait manqué `tests/test_frame_zone.c`, trouvé ainsi avant le rouge.

## 7. Tests

- Le commentaire d'un test garde « pourquoi cette valeur-là ».
- Une assertion ne change ni de condition ni de présence. Son message peut seulement être proposé réécrit (point 5).
- Aucune assertion n'est ajoutée à l'étape 4 (la permission du skill d'en ajouter une prouvée tombe).
- *Amendé par 4c1.* Un nom de cas d'un test à processus enfant (`CASES[]`, lu par les `expect_*` de `main`) est le nom de sa fonction sans `case_` ; un nom français ou du cart se propose sous cette règle, à la rubrique 10.

## 8. Contrôles de l'agent

Sans `make` (le répertoire de build est partagé) : l'agent appelle les scripts directement.

- *Lignes de commentaire*, la seule définition, mesurée avant et après :

  ```sh
  echo $(( $(grep -c . F) - $(gcc -fpreprocessed -dD -E -P F | grep -c .) ))
  ```

  Lignes non vides du fichier moins lignes non vides du préprocesseur.
- *Preuve* : sortie de `gcc -fpreprocessed -dD -E -P` identique à l'octet à celle du fichier d'avant. Pour un script ou une donnée de `tools/`, les lignes qui ne sont pas des commentaires sont identiques (*amendé par 4a*, la commande) :

  ```sh
  diff <(grep -v '^[[:space:]]*#' AVANT) <(grep -v '^[[:space:]]*#' F)
  ```

  Elle ne voit ni un commentaire en fin de ligne de code (`cmd  # …`) ni un `#` dans un here-document : un tel commentaire se laisse tel quel ou se signale à l'orchestrateur.
- *Hauteur des blocs* (*amendé par 4b*) : dans un `.c` de `src/` cité par `tools/line_coverage_reference.txt` (`cut -d: -f1 tools/line_coverage_reference.txt | sort -u`), chaque bloc de commentaire garde son nombre de lignes : `e2e-gap` compare des numéros de ligne, et un bloc qui grandit ou rétrécit décale tout le code qui suit (72 lignes de plus en défaut à 4b). Contrôle : les lignes qui ne sont pas des commentaires restent aux mêmes numéros, et `make e2e-gap` rend la même sortie à l'octet. Les en-têtes et les tests ne sont pas concernés.
- *Fichier d'avant* (*amendé par 4a*, pour l'orchestrateur) : quand le commit de chaînes et le commit de commentaires touchent le même fichier, l'« avant » des commentaires est l'état d'après les chaînes ; l'orchestrateur en garde une copie sous `tmp/` avant de lancer les agents, puisque les deux commits ne sont faits qu'à la fin.
- *Compteurs*, colonnes `backtick` et `comment` à zéro sur ses fichiers, hors messages d'`assert` qu'il a proposés :

  ```sh
  python3 tools/rename_count.py --table docs/renommage.tsv \
      --occurrences tmp/ast_rename/occurrences.tsv --root . --module <module>
  python3 tools/french_count.py --root . --exceptions tools/french_exceptions.txt \
      --substeps tools/rename_substeps.tsv --module <module> --sites
  python3 tools/cart_citations.py --table docs/renommage.tsv --root . --module <module>
  ```

## 9. Rapport

Au format de `modele-rapport.md`, avec ces changements de rubriques :

- rubrique 3 (asserts posés) : vide par construction ;
- rubrique 5 : devient « **noms à renommer** » — locales candidates de `make cryptic-locals`, statiques locales, noms au sens illisible, chacun avec un nom proposé (jamais une ligne de glossaire) ;
- rubrique 10, ajoutée : « **chaînes et messages d'`assert` à réécrire** » — site, texte actuel, texte proposé, lecteurs, test qui le lit ;
- rubrique 11, ajoutée : « **exceptions proposées** » — faux positifs du compteur de français, avec la ligne d'exception prête (`colonne<TAB>module<TAB>mot`, `tools/french_exceptions.txt`).

La rubrique 8 (« ce que j'ai hésité à supprimer ») reste obligatoire.

*Amendé par 4a.* La rubrique 7 (affirmations vérifiées fausses) vaut aussi pour les affirmations qu'on traduit : un nom du cart ne se remplace pas mot pour mot sans relire le code. À 4a, « un constructeur qui ne passe pas par `ke` » (classe racine) était faux pour deux des constructeurs listés (`mouse_new`, `structure_icon_new` appellent `entity_construct`) ; la traduction littérale aurait gardé l'erreur.
