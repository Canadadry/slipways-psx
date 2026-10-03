# Plan 8 — Toolchain PSX et premier `.bin/.cue`

Brouillon, à passer en `plan-converge` avant toute exécution. Troisième des cinq plans vers un `.bin/.cue` PSX (plans 6 à 10).

## 1. Objectif

Dans ce sandbox : `psx-hello` compilé, emballé en `.bin/.cue`, et démarré dans un émulateur sans fenêtre, avec une preuve lisible par un agent.

## 2. Ce qui existe

- `psx-hello/` et `tools/install_psn00bsdk_toolchain.sh`, jamais exécutés (`docs/psx-hello-world.md`).
- Le piège connu d'`elf2x`, et son contournement.
- Le réseau du sandbox bloque les miroirs GNU et les émulateurs aujourd'hui.

## 3. Étapes

1. **Réseau.** Lister les hôtes nécessaires (toolchain, PSn00bSDK, outil d'image CD, émulateur) et les faire ouvrir par l'humain. Préférer les binaires précompilés à une compilation de GCC.
2. **Toolchain.** `make psx-hello` produit un `.exe`.
3. **Image CD.** Une cible `make` produit `psx-hello.bin/.cue` (outil type `mkpsxiso`), avec licence et `SYSTEM.CNF`.
4. **Émulateur sans fenêtre.** Démarrer l'image en mode headless et en sortir une preuve : capture de VRAM en fichier, ou sortie TTY. La fenêtre reste interdite, comme pour raylib.
5. **Cible de contrôle.** Une cible `make` enchaîne image et démarrage, et échoue si la preuve manque.
6. **Doc.** `docs/psx-hello-world.md` réécrit sur ce qui marche vraiment.

## 4. Point ouvert

Le choix de l'émulateur dépend de l'étape 4 : il faut un mode sans fenêtre qui démarre une image CD et rend une trace exploitable.
