# Notes de projet

## Le portage

- Cible active : **PC via raylib, rendu logiciel** — voir `docs/plan.md`. Portage littéral, fonction par fonction, de `pico8/lua.lua` (la source décompilée du cart), pas une réécriture depuis le document de conception.
- PSX est **en pause**, pas abandonné : on y revient quand une version PC/C est jouable de bout en bout. `psx-hello/` est un projet de test du toolchain — voir `docs/psx-hello-world.md`.
- L'audio (`sfx`, `music`) est **hors périmètre**, pas reporté.

## Ce qu'on ne fait pas

- **Ne jamais lancer l'appli raylib en interactif** (aucune fenêtre) avant la phase 5 de `docs/plan.md`. La vérification se fait sans fenêtre : N frames pilotées par un `events.json` scripté, puis dump texte du framebuffer.

## Build

- `make test` est la suite complète. La légende des cibles est en tête du `Makefile`.
- `make raylib` compile et linke le binaire interactif sans jamais l'exécuter ; il faut `make raylib-vendor-setup` d'abord — voir `docs/raylib-vendoring.md`.

## Commentaires : ce qui a le droit de rester dans le code

Un commentaire va dans une de ces destinations, sinon il part :

- **Le sens d'un nom minifié** (`jo`, `bv`, `cg`, `kq`…) → `docs/glossaire-noms.md`, une ligne par symbole. Les identifiants C portent déjà le nom d'origine (`pico8_bv`, `pico8_jo_kj`) ; c'est le sens qui manquait, pas le nom.
- **Un écart connu avec le cart, ou un morceau volontairement non fait** → `docs/trous-de-portage.md`, numéroté. Le code s'y réfère par un `/* TROU-7 */` nu, rien de plus.
- **Un piège qu'un lecteur futur pourrait réintroduire** → un `assert(cond && "…")` porteur de message si la condition est vérifiable à l'exécution, sinon **40 mots maximum**. Pas de récit, pas de trace ASan, pas d'historique des correctifs.
- **Un contrat C que ni le nom ni le glossaire ne portent** — qui possède la mémoire, qui la libère, ce que rend la fonction quand le champ est absent → une phrase en en-tête.
- **`TODO`** reste autorisé : c'est l'aveu qu'un cas n'est pas traité.

Dans un `Makefile`, la seule destination est l'aide à la lecture d'une commande.

Tout le reste — pourquoi telle décision a été prise, ce qui avait été essayé avant, quelle revue a signalé quoi — va dans le **message de commit**. Git le garde, et personne ne le paie en jetons à chaque lecture du fichier.

Le code Lua d'origine ne se recopie pas en commentaire : `pico8/lua.lua` est dans le dépôt, et les tests de rejeu (`make test`, 9 729 frames) sont ce qui atteste la fidélité.
