---
title: "Plan 3, étape 0.4 : porte de couverture des classes sur les rejeux du filet"
status: done
---

## Verdict

`make class-coverage-net` sur master 450fb80 : `couverture: OK -- chaque classe non exemptée est instanciée (15 rejeu(x))`. Seule `ke` (exemptée) reste à 0.

## Livré par

- #27 (fde59b3) : les pilotes headless rejouent les enregistrements raylib depuis l'écran titre (`--from-title`, liste `RECORDINGS`) ; cible `make class-coverage-net` sur `FRAMES_EVENTS`. Avant ce correctif, le pré-clic de `bo()[2]` désynchronisait tout enregistrement raylib : la porte voyait `fr hj mx nl nr os pm` à 0.
- #28 (450fb80) : enregistrement humain `trading_hub_session_events.json`, qui construit le seul `nl` du filet.

## Pour la relance en sortie de 1a

Même commande, `make class-coverage-net`. Les classes rares, à surveiller : `nl` 1 (1 rejeu), `os` 1 (1), `fq` 3, `mx` 3, `pm` 4, `fr` 5, `hj` 5 (1 rejeu chacune), `nr` 6.
