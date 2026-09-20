# Suivi du portage — Phase 3

Liste de suivi fonction par fonction pour la Phase 3 de [`docs/plan.md`](plan.md) (portage de `pico8/lua.lua`). Fichier séparé du plan pour ne pas l'alourdir avec ~140 entrées.

Groupée par vague de dépendances (ce qui est appelé avant ce qui appelle) ; une vague = un lot au sens du checkpoint de Phase 3 (`docs/plan.md`). Mise à jour au fil du portage — pas remplie à l'avance.

Gabarit de table, une section `## Vague N` par vague. Colonne « Module cible » ajoutée après le constat `warn-review` de fin de Phase 1 (un module — `pico8_gfx.c` — avait grossi disproportionnellement sans que ça se voie avant une revue de fin de phase) : elle rend visible, vague après vague, la taille relative de chaque module au fur et à mesure du portage plutôt qu'après coup.

| Fonction (nom minifié) | Module cible | Statut | Notes |
| --- | --- | --- | --- |
| `ex.` | `ex_module.c` | portée / restante | ex. dépend de `ob()` |

## Vague 1 — Chunk 1 : « Generic pure helpers, list/data glue, and the missing audio stubs » (PRD [phase3-etape1-function-by-function-c-port.md](prd/triage/phase3-etape1-function-by-function-c-port.md))

~27 fonctions + littéraux de données, sans dépendance mutuelle interne au chunk — le PRD autorise explicitement 2-3 sous-lots par sous-thème. Sous-lot 1 (cette session) ci-dessous ; le reste reste `restante` pour un sous-lot suivant.

| Fonction (nom minifié) | Module cible | Statut | Notes |
| --- | --- | --- | --- |
| `ft` | `pico8_collect.c`/`.h` | portée | Glue de bucket-liste `rc[nt]=rc[nt]or{}; add(rc[nt],pu)`. Deux points d'entrée C (`pico8_ft_index`/`pico8_ft_name`) pour la clé polymorphe (nombre vs chaîne) d'un seul `ft` Lua. |
| `qj` | `pico8_collect.c`/`.h` | portée | Fusion `mr` DANS `o` (mr gagne sur une clé partagée — sens déjà confirmé par `pico8_ob.c`). Duplique volontairement la boucle de fusion déjà inline dans `pico8_ob()` plutôt que de faire grossir ce module déjà proche du seuil de 430-540 lignes (538 lignes avant ce lot) — duplication documentée, pas silencieuse. |
| `mw` | `pico8_collect.c`/`.h` | portée | `qj({},o)` — copie superficielle. |
| `gu` | `pico8_collect.c`/`.h` | portée | Test d'appartenance via `all()`+`==` (`pico8_value_equal`). **Limite documentée** : au moins un vrai site d'appel (`ho()`, restant dans ce chunk) compare des chaînes Lua et a besoin d'une égalité PAR CONTENU (les chaînes Lua sont internées, donc `==` y est en pratique une égalité de contenu) — mais `pico8_value_equal` compare `PICO8_VALUE_STRING` par identité de pointeur, décision Phase 2 déjà actée et testée (`test_equal_string_by_pointer_identity_not_content`). Pas de site d'appel réel dans CE sous-lot n'exerce cet écart ; laissé au sous-lot qui portera `ho()`/exercera Chunk 11 pour trancher (étendre `pico8_value_equal` ou adapter l'appelant), pas deviné ici. |
| `jp` | `pico8_collect.c`/`.h` | portée | Filtre générique sur `pico8_list_t`, prédicat + ctx (motif déjà établi par `pico8_list_foreach_fn`). |
| `nv` | `pico8_collect.c`/`.h` | portée | Map générique ; `kp(e)or nil` porté via le motif "écrit *out_value, retourne 1/0" déjà utilisé par `pico8_fi()` (pas de genre nil dans `pico8_value_t`). |
| `la` | `pico8_collect.c`/`.h` | portée | Concat de deux listes via `pico8_list_foreach` (fidèle à l'original qui utilise `foreach`, pas une boucle nue). |
| `oo` | `pico8_collect.c`/`.h` | portée | Invocation de constructeur positionnel (`fn(a[1]..a[5])or a`) sur `pico8_ob_table_t`/`pico8_ob_constructor_fn`. Logique identique déjà inline dans la branche `identifier(...)` de `pico8_ob_value_for_token` (Phase 2) — non refactorée pour partager le code (éviter de faire grossir `pico8_ob.c`, déjà proche du seuil), duplication documentée. Note pour le futur portage de `bl()` (Chunk 2) : si le registre `pi[e.fn]` à clé chaîne enregistre des adaptateurs `pico8_ob_constructor_fn`-compatibles pour `rk`/`io`/`eq`/`lq.ol`/`spr`, cette même `pico8_oo` est directement réutilisable telle quelle. **Signalé (vérification Chunk 1, sous-lot 1)** : le vrai `fn and fn(...) or a` retombe sur `a` si `fn(...)` renvoie un résultat *falsy* (pas seulement si `fn` est nil), mais ce port ne gère que le cas `fn==NULL` — même simplification que la duplication inline déjà existante dans `pico8_ob.c` (donc pas un écart nouveau), non exercée aujourd'hui (aucun constructeur enregistré ne peut renvoyer falsy), à revisiter quand un builtin void comme `spr` sera enregistré comme constructeur (Chunk 2). |
| `ne` | — | restante | Formatage de nombre signé (`+`/couleur). |
| `ij` | — | restante | Plus proche voisin dans une liste de points. |
| `rn` (+ `pico8_sfx`/`pico8_music`) | — | restante | `rn` référence le global `ih` (pas encore assigné avant Chunk 21) — traitement « référence avant, compile, pas encore exerçable » comme `bg:ldown`. Stubs audio no-op à créer ici, seul et unique endroit où ils existent. |
| `jf` | — | restante | Fabrique de fermeture (marche aléatoire bornée), utilise `rnd`/`abs`/`max` (Phase 2). |
| `bu` | — | restante | Glue bouton souris (`stat(34)`+`band`), lit `qn.on` précédent. |
| `qz` | — | restante | À enregistrer dans le registre `char`-keyed existant de `pico8_ob` sous `'b'` (`pico8_ob_register_constructor`) — si porté dans un sous-lot séparé de celui qui touche `pico8_ob.c`, laisser la note ici pour le sous-lot qui s'en charge. |
| `gk` | — | restante | Seuil `mp`/`rp` selon `e.fa`. |
| `na` | — | restante | Oscillation verticale (`self.ra.y+=sin(...)`), assignée plus tard comme `qh.ly` (Chunk 15, hors scope ici — juste la fonction standalone). |
| `ma` | — | restante | Décroissance + décalage polaire (`qm`), utilisé pour l'auréole de particules. |
| `gd` | — | restante | Test de visibilité écran — référence le futur singleton `mo` (pas encore réel avant Chunk 6) — même traitement « référence avant ». |
| `cu` | — | restante | Construit la table de palette en mémoire (`poke`/`sget`/`bor`). |
| `dv` | — | restante | `memcpy` de la palette (astuce PICO-8 `0x5f00`). |
| `rk` | — | restante | `rectfill` bande horizontale pleine largeur. |
| `eq` | — | restante | Cadre + 2 sprites (bouton d'aide). |
| `io` | — | restante | Texte avec contour (`er`, un des littéraux de données de ce chunk). |
| `unstash` | — | restante | Décodage RLE-par-caractère depuis `peek()`/`gt` (alphabet de 44 caractères) — à lire et porter littéralement (algorithme confirmé en lisant `pico8/lua.lua:79-84`, pas deviné). |
| `gn` | — | restante | Générateur pondéré par `.r` avec exclusion des 3 derniers tirages (`nm[3-#oc%3]`). |
| `ho` | — | restante | Fermeture de renommage de champs positionnels→nommés (via `r`), retour de fonction Lua — en C, exposée comme un contexte + une fonction `pico8_list_foreach_fn` séparée (pas de fermeture native). Dépend de `gu` (portée) et du global `r` (construit via `unstash`, même sous-lot restant). |
| `gt` | — | restante | Constante chaîne (alphabet de 44 caractères pour `unstash`). |
| `er` | — | restante | Littéral `ob([[...]])`, offsets de contour pour `io()`. |
| `es` | — | restante | Littéral `ob([[1,0,1,2,]])`. |
| `ct` | — | restante | Littéral `ob([[-0.333,0,0.25,0.5,]])`. |
| `b_` | — | restante | Littéral `ob([[0,100,200,400,]])`. |
| `co` | — | restante | Littéral `ob([[-5,0,1,1,]])`. |
| `bn`/`bq` | — | restante | Constantes numériques (`6,0.0977`) — pas via `ob()`, affectation directe. |
| `fb` | — | restante | Littéral `ob([[2,2,5,5,7,7,10,10,14,14,14,19,19,19,]])`. |
| `r`/`iz` | — | restante | `ob(unstash(8192))`/`ob(unstash(8259))` — dépendent de `unstash` (même sous-lot restant). |

**Sous-lot 1 (cette session)** : `ft`, `qj`, `mw`, `gu`, `jp`, `nv`, `la`, `oo` — module `src/pico8_collect.c`/`src/pico8_collect.h` (nouveau, 169+155 lignes, sous le seuil de 430-540 lignes). Aucune de ces fonctions n'est encore appelée par un binaire du Makefile (aucun consommateur n'existe encore dans ce chunk) : compilation propre vérifiée manuellement (`cc -std=c99 -Wall -Wextra -Werror -c`, 0 warning) plutôt que via `make test`, conformément à la politique « pas de test unitaire par fonction » de cette phase ; le sous-lot/chunk qui appellera réellement ces fonctions ajoutera le rattachement Makefile nécessaire (même schéma que `pico8_class.c`, qui n'est compilé que via `test_pico8_entity_smoke`).

**Vérification module cible** : `pico8_ob.c` (Phase 2) reste à 538 lignes, déjà au-dessus du repère ~430-540 signalé par `warn-review` — non touché par ce sous-lot (décision explicite ci-dessus de ne pas y ajouter `oo` pour ne pas l'alourdir davantage). Aucun autre module existant modifié.

**Restant pour le(s) sous-lot(s) suivant(s)** : `ne`, `ij`, `rn`+stubs `pico8_sfx`/`pico8_music`, `jf`, `bu`, `qz`, `gk`, `na`, `ma`, `gd`, `cu`, `dv`, `rk`, `eq`, `io`, `unstash`, `gn`, `ho`, plus les littéraux `gt`, `er`, `es`, `ct`, `b_`, `co`, `bn`/`bq`, `fb`, `r`/`iz`.
