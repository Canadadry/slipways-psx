# Suivi du portage — Phase 3

Liste de suivi fonction par fonction pour la Phase 3 de [`docs/plan.md`](plan.md) (portage de `pico8/lua.lua`). Fichier séparé du plan pour ne pas l'alourdir avec ~140 entrées.

Groupée par vague de dépendances (ce qui est appelé avant ce qui appelle) ; une vague = un lot au sens du checkpoint de Phase 3 (`docs/plan.md`). Mise à jour au fil du portage — pas remplie à l'avance.

Gabarit de table, une section `## Vague N` par vague. Colonne « Module cible » ajoutée après le constat `warn-review` de fin de Phase 1 (un module — `pico8_gfx.c` — avait grossi disproportionnellement sans que ça se voie avant une revue de fin de phase) : elle rend visible, vague après vague, la taille relative de chaque module au fur et à mesure du portage plutôt qu'après coup.

| Fonction (nom minifié) | Module cible | Statut | Notes |
| --- | --- | --- | --- |
| `ex.` | `ex_module.c` | portée / restante | ex. dépend de `ob()` |

*(vide pour l'instant — la Phase 3 n'a pas commencé ; Phase 1 et 2 sont préalables.)*
