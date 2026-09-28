# slipways-love

## À faire

Règle de cette liste : chaque todo est une seule phrase claire, lisible sans référence externe, de 30 mots au plus.

- [ ] Relire puis ouvrir la pull request de la branche `claude/plan-2-float-malloc-bfkd28`, qui retire toute virgule flottante du cœur du jeu sans changer son rendu hors corrections sourcées.
- [ ] Sur le vrai PICO-8, vérifier trois comportements : l'affichage de -0.0001, la couleur d'un cercle tracé avec une couleur nil, et sin/cos entre les multiples de 1/64.
- [ ] Avant d'écrire le prochain plan, trancher comment remplacer malloc : quelles allocations vivent une frame, lesquelles vivent toute la partie, et quelles limites de taille imposer.
- [ ] Ajouter une triche qui donne des points de score bonus, pour qu'un e2e court montre les étoiles allumées du panneau de score.
- [ ] Faire respecter la couverture de classes par `make e2e-cleanup`, qui propose de retirer `place_processor_ascension`, seul e2e à instancier le processeur et la porte d'ascension.
