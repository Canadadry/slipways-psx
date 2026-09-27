# Rejeux actuels hors limite : minimisation et jugement

Critère de garde de `docs/prd/triage/e2e-rapide.md` : partir de l'écran
titre, tenir en 180 frames après minimisation (600 en exception listée),
ajouter au moins une ligne. Mesuré le 2026-09-27 sur le conteneur (4 cœurs).

## Partent du titre (RECORDINGS)

| rejeu | brut | étage mécanique | passe guidée | entre ? |
|---|---|---|---|---|
| trading_hub_session_events | 4 921 | 1 000, perd 385 lignes | 4 172, lignes identiques | non : 4 172 > 600 |
| events_recorded_1790198294 | 19 832 | 3 086, perd 1 242 lignes | non lancée | non |
| long_session_events | 27 273 | 5 962, perd 1 003 lignes | non lancée | non |
| session_events | 116 045 | 21 017, perd 805 lignes | non lancée | non |

- `trading_hub_session_events` : `make minimize MIN_CHUNK=16`, 464 candidats,
  42 min. L'étage mécanique change l'ensemble de lignes, la passe repart du
  brut ; chaque frame retirée décale le tirage `rnd` partagé, si bien que
  presque aucune tranche ne se retire sans changer l'ensemble. Sept fois la
  limite d'exception : n'entre pas.
- Les trois longs enregistrements : un appel d'oracle coûte 25 s, 32 s et
  149 s sur le brut, et l'étage mécanique perd lui aussi des lignes, donc la
  passe guidée repartirait du brut. Au rythme observé sur trading_hub
  (~0,85 frame gardée par frame brute), le résultat resterait des milliers de
  frames au-dessus de 600, pour des heures d'oracle. Non lancée.

Aucun de ces rejeux n'entre dans `tests/e2e/`. Leurs lignes restent dans
l'écart d'`e2e-gap`, à combler par des scénarios courts (catalogue).

## Partent du raccourci de difficulté

Hors de `tests/e2e/` par construction (tout e2e part du titre) : minimisés
à titre indicatif seulement (`SHORTCUT=1 MIN_CHUNK=4`, quelques secondes).

| rejeu | brut | étage mécanique | passe guidée |
|---|---|---|---|
| phase4_playthrough_events | 224 | 57 | 224 |
| slipway_drag_events | 226 | 59 | 147 |
| empty_planet_cleared_events | 229 | 35 | 108 |
| built_planet_panel_events | 278 | 72 | 260 |
| geoharvest_burst_events | 293 | 82 | 264 |

L'étage mécanique change l'ensemble de lignes de chacun. Ils seront
réenregistrés depuis le titre dans la boucle de scénarios. Les rejeux du
filet déjà sous 180 frames (`top_menubar`, `screencast_capture`,
`shop_button_hover`, `pause_menu_music`, `tech_screen_invent`, `game_end`)
ne sont pas concernés.
