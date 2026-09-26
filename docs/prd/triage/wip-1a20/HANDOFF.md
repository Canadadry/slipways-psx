# Reprise plan-pilot plan 3 — étape 1a.20 (`os`) en cours

## État

- Étapes gradées : 0.4, 1a.0 → 1a.19 (dont 1a.3 absorbée, 1a.16b). Branche rebasée sur master (#29, `make test` / `make e2e`).
- 1a.20 `os` : implémentée, **non vérifiée, non commitée**. PRD : `../plan3-phase1a-etape20-os-processeur.md`. Travail : `wip.patch` (s'applique sur la branche : `git apply docs/prd/triage/wip-1a20/wip.patch`). Caractérisation figée sur l'ancien code : `test_pico8_os.old-code.c`. Journal : `1a20-progress.md` (mutations faites, 20/21, `od` équivalent).
- Reste pour 1a.20 : appliquer le patch, `make test` + `make e2e` + `make class-coverage-net`, vérificateur indépendant (chercher une mutation oubliée : l'agent a été coupé juste après le banc de mutations), commit, `git mv` du PRD vers `done/`, supprimer `wip-1a20/`.
- Étape suivante : 1a.21 `hj`.

## Règles apprises (à mettre dans chaque prompt d'agent)

- `timeout` sur toute commande longue ; attendre un PID, jamais `pgrep -f` (il se trouve lui-même).
- Disque limité : chaque copie jetable du scratchpad est supprimée après usage (un disque plein a provoqué des redémarrages du conteneur).
- Journal de progression par étape dans le scratchpad (le conteneur redémarre ; une mutation est déjà restée en place à 1a.19).
- Mutations sur le seul binaire concerné. Commit via un fichier message écrit par l'implémenteur.
- Depuis #29 : `make test` = boucle rapide, `make e2e` = rejeux ; `prd-autopilot` lance `check-e2e` par PRD. Les nouveaux tests de caractérisation sont ajoutés à `UNIT_TESTS` (ceux à rejeux longs pourraient aller en e2e : à trancher par l'humain).
- Arbre jo : les fonctions de qh et jo restent à double forme jusqu'à 1a.26 ; filles construites sur le modèle de 1a.19 (`nl`).

## En attente de l'humain

- TROU-23 : garder, ou corriger vers le cart (change deux `.sha256`).
- Écart `0x5f80` dans `pico8_ui_gw` (le port écrit 0, le cart `max(peek-1,0)`), hors plan 3.
