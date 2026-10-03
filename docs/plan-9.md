# Plan 9 — Toolchain PSX et premier `.bin/.cue`

Brouillon, à passer en `plan-converge` avant toute exécution. Troisième des cinq plans vers un `.bin/.cue` PSX (plans 7 à 11).

## 1. Objectif

Dans ce sandbox : `psx-hello` compilé, emballé en `.bin/.cue`, et démarré dans un émulateur sans fenêtre, avec une preuve lisible par un agent.

## 2. Ce qui existe

- `psx-hello/` et `tools/install_psn00bsdk_toolchain.sh`, jamais exécutés (`docs/psx-hello-world.md`).
- Le piège connu d'`elf2x`, et son contournement.
- Le réseau du sandbox bloque les miroirs GNU et les émulateurs aujourd'hui.

## 3. Étapes

1. **Réseau.** Lister les hôtes nécessaires (toolchain, PSn00bSDK, outil d'image CD, émulateur, BIOS) et les faire ouvrir par l'humain. Préférer les binaires précompilés à une compilation de GCC.
2. **Toolchain.** `make psn00bsdk-toolchain-setup` installe tout dans un sandbox neuf, puis `make psx-hello` produit un `.exe`.
3. **Émulateur sans fenêtre.** Démarrer le `.exe` en mode headless et en sortir une preuve : une capture de VRAM en fichier, que le plan 10 réutilise, plus la sortie TTY si elle aide. La fenêtre reste interdite, comme pour raylib. Si aucun émulateur ne convient, arrêt et retour à l'humain : les plans 10 et 11 en dépendent.
4. **Image CD.** Une cible `make` produit `psx-hello.bin/.cue` (outil type `mkpsxiso`), avec licence et `SYSTEM.CNF`, démarrée comme à l'étape 3.
5. **Cible de contrôle.** Une cible `make`, hors de `make test-all`, enchaîne image et démarrage, et échoue si la preuve diffère de sa référence (sha256 de capture, comme les e2e).
6. **Doc.** `docs/psx-hello-world.md` réécrit sur ce qui marche vraiment.

## 4. Point ouvert

Le choix de l'émulateur se fait à l'étape 3 : il faut un mode sans fenêtre qui démarre un `.exe` puis une image CD et rend une trace exploitable.
