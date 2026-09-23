---
title: "Plan 2, Phase D4 : revue des clés de table — chaque clé convertie reçoit un verdict sourcé"
description: "En Lua, t[1.5] est une clé distincte de t[1], mais le port C convertit la clé en int avant la recherche : chaque site où une valeur 16.16 devient une clé de table doit être établi entier (ou chaîne) par construction et/ou sans argument fractionnaire sur tout le filet, sinon consigné en trou de portage."
status: needs-triage
---

## Problem Statement

Texte de l'étape, recopié de `docs/plan-2.md` (§7, Phase D) :

> **D4 — clés de table.** Réduite aux clés déjà converties (16 en C1, 1 en D1) ; lue principalement dans le compte d'arguments fractionnaires de D0a (un site sans argument fractionnaire sur tout le filet est entier de fait, pas seulement par argument).

Contexte, §3.2 (bloc B du relevé) :

> **Découverte non anticipée, même bloc : les clés de table fractionnaires sont des clés distinctes.** `t[1.5]="a"` puis `t[1]` → `nil`, et `t[2]="b"` puis `t[2.5]` → `nil`. Le C, lui, convertit la clé en `int` *avant* la recherche (`pico8_score_panel.c:110`, `pico8_planet.c:309`, `economy_formulas.c:55,66`…) : sur une clé fractionnaire il trouverait une entrée là où Lua ne trouve rien. À vérifier site par site pendant l'audit — une clé toujours entière par construction rend le point sans objet, mais ça doit être établi, pas supposé.

Risque, §10 :

> | Clés de table fractionnaires (§3.2) | Le C trouverait une entrée là où Lua renvoie `nil` | À établir site par site pendant l'audit — « toujours entier par construction » doit être démontré, pas supposé |

Et la décision actée 14 : « Les clés de table fractionnaires sont des clés distinctes en Lua/PICO-8 ; le C convertit avant la recherche. À établir site par site. »

**Où en est-on.** Tous les casts `(int)pico8_fixed_to_double` sont convertis (C1 : 47 sites après D0b, D1 : 29). `tools/float_annotation.txt` n'a plus aucune ligne de cast : les justifications « cle de table » qui y figuraient ne vivent plus que dans son historique git. D0b a déjà rendu le verdict de clé de ses 11 sites (6 entiers par construction, 5 chaînes par construction, aucun trou ouvert). Il reste :

- **16 sites C1** dont la justification portait « cle de table » (commit `ecef59d`, retirés en C1) : leur preuve portait sur le **signe** (≥ 0), pas sur le caractère entier de la clé. Un signe prouvé ne dit rien d'une clé `2.5`.
- **1 site D1** (`pico8_mz_kj`, `pico8_fixed_flr(ratio)`, `kk[flr(sr/self.oi*8)]`, lua.lua:790).
- **Les clés que l'étiquette n'a pas vues.** L'étiquette « cle de table » a été posée à la main en C0, par cast. Un cast qui passe par une aide (`pico8_particle_to_int`, `pico8_hud_to_int`, `pico8_ui_to_int`, `pico8_bg_to_int`, `pico8_probe_to_int`, `pico8_jc_display_to_int`, `ie_to_int`…) ou dont le résultat est retouché avant de servir d'indice (`pico8_menu.c`, `pico8_cr` : `pico8_fixed_flr_int(tl) - 1` vers `ek` ; `pico8_particle.c` : `pico8_particle_to_int(pico8_fixed_flr(new_l))` vers `cs`) a pu échapper au tri. Tant qu'aucun balayage ne l'a exclu, le §10 n'est pas levé.

Tant que ces sites n'ont pas de verdict, le risque « clés de table fractionnaires » reste ouvert dans le plan, et la ligne « revue des clés de table » de la table de propriété n'est pas soldée.

## Solution

D4 est une **revue, pas une conversion** : aucune ligne de `src/` ne change. Elle produit, dans `docs/plan-2.md`, un « État de D4 » qui donne à **chaque site de clé de table issu d'une valeur 16.16** un verdict sourcé, et lève (ou requalifie) le risque du §10.

1. **Établir la liste des sites**, en partant de tout le cœur, pas seulement de l'étiquette :
   - les 28 sites étiquetés « cle de table » dans l'historique de `tools/float_annotation.txt` (16 C1, 1 D1, 11 D0b) ;
   - un **balayage** de tous les appels de `pico8_fixed_flr_int` du cœur (76 sites d'audit, aides comprises) : pour chacun, le résultat — ou, pour une aide `*_to_int`, le résultat à **chacun de ses appelants** — sert-il d'indice à une lecture ou écriture de table (`pico8_ob_table_get_index`/`set_index`/`remove_index`, accès indexé d'un `pico8_list_t`, tout équivalent de `t[n]`) ? Chaque site rendu « pas une clé » l'est en une ligne.
   - En contrôle : aucune autre conversion 16.16 → `int` n'existe dans le cœur (grep des décalages `>> 16` et des conversions ad hoc ; à la rédaction, seul `pico8_value.c`, `tostr`, en contient une, et elle n'indexe rien).
2. **Rendre un verdict par site clé**, avec l'une de ces formes (plusieurs peuvent se cumuler) :
   - **entière par construction** — lignes de `pico8/lua.lua` (ou positions de l'unstash) citées qui montrent que la valeur ne peut être qu'entière : littéral, compteur `+=1`, `#t`, `flr(…)` explicite, constante du cart ;
   - **chaîne par construction** — idem, la branche numérique n'est jamais prise (forme de G5) ;
   - **0 fractionnaire sur tout le filet** — le compte `fractionnaires` de `make float-audit` pour la ligne du site, cité avec son nombre d'appels. C'est la lecture principale demandée par le texte de l'étape ; elle suffit seule pour un site que le filet atteint, et elle ne vaut rien pour un site non atteint (appels 0) ni pour une aide partagée dont le compte agrège tous les appelants (voir Implementation Decisions) ;
   - **fractionnaire possible** — la sémantique Lua peut amener une clé non entière au site. Le site devient une **nouvelle entrée de `docs/trous-de-portage.md`** (TROU-23 et suivants), décrivant l'écart (Lua manque, le C trouve l'entrée d'indice `flr`), sa source et son atteinte par le filet. **Il n'est pas corrigé ici.**
3. **Solder le plan** : ligne D4 marquée faite, « État de D4 » écrit, ligne « revue des clés de table » de la table de propriété marquée faite, ligne « Clés de table fractionnaires » du §10 barrée (**levé**) si aucun site n'est « fractionnaire possible », sinon requalifiée avec les renvois TROU.

## User Stories

1. En tant que mainteneur du port, je veux que chaque site où une valeur 16.16 devient une clé de table ait un verdict écrit, afin que le risque du §10 soit levé par une démonstration et non par supposition.
2. En tant que mainteneur, je veux que le verdict d'un site C1 dise pourquoi la clé est **entière**, pas seulement positive, afin qu'une preuve de signe ne soit pas prise pour une preuve d'intégralité.
3. En tant que mainteneur, je veux que la revue cite le compte `fractionnaires` de `make float-audit` pour chaque site atteint, afin que « entier de fait sur le filet » soit une mesure reproductible.
4. En tant que mainteneur, je veux que les comptes cités viennent d'une passe de `make float-audit` faite pendant D4, sur l'arbre de D4, afin qu'ils ne soient pas recopiés d'un état antérieur (les comptes ont bougé depuis D1 : 32 corrections non vues au lieu de 28, 5 551 169 passages).
5. En tant que mainteneur, je veux qu'un site non atteint par le filet ne soit jamais déclaré sûr sur la foi de « 0 fractionnaire », afin que « 0 sur 0 appel » ne passe pas pour une preuve.
6. En tant que mainteneur, je veux qu'une aide `*_to_int` partagée soit jugée **par appelant** quand l'un d'eux indexe une table, afin que les millions d'arguments fractionnaires d'un tracé ne masquent pas, ni n'incriminent, une clé bien entière (ou l'inverse).
7. En tant que mainteneur, je veux que la liste des sites soit le résultat d'un balayage de tous les appels de `pico8_fixed_flr_int`, afin qu'une clé que l'étiquette de C0 a manquée soit trouvée.
8. En tant que mainteneur, je veux que chaque appel balayé et jugé « pas une clé » ait sa ligne (coordonnée, couleur, sprite, rayon, borne de boucle…), afin que le balayage soit relisible et que l'exclusion ne soit pas silencieuse.
9. En tant que mainteneur, je veux que les 6 verdicts convertis de D0b et les 5 verdicts chaîne de G5 soient repris par renvoi à l'« État de D0b », afin de ne pas refaire ni recopier une revue déjà faite.
10. En tant que mainteneur, je veux que le site D1 `pico8_mz_kj` `flr(ratio)` ait son verdict (flr explicite du Lua, lua.lua:790 ; 0 fractionnaire à l'audit), afin que la seule clé de D1 soit couverte.
11. En tant que mainteneur, je veux qu'un site « fractionnaire possible » devienne une entrée TROU sourcée, afin que l'écart soit connu sans que D4 change le comportement du port.
12. En tant que mainteneur, je veux que D4 ne modifie aucun fichier de `src/`, afin que `make frames-check` reste identique sans `frames-baseline` et que la revue ne puisse rien casser.
13. En tant que mainteneur, je veux que `make test` reste vert après D4, afin que les contrôles de fraîcheur (inventaire, constantes, audit) confirment qu'aucun fichier suivi n'a dérivé.
14. En tant que mainteneur, je veux que la table de revue vive dans `docs/plan-2.md` (« État de D4 »), comme les états des autres étapes, afin de ne pas ajouter un rapport et un contrôle de plus pour une revue qui ne se rejoue pas.
15. En tant que mainteneur, je veux que chaque ligne de la table donne fichier, fonction, expression Lua de la table et de la clé (avec ligne de `lua.lua`), provenance (C1, D1, D0b, balayage), verdict, source et compte d'audit, afin qu'une ligne se vérifie seule.
16. En tant que relecteur de la Phase E, je veux que la ligne « revue des clés de table » de la table de propriété soit marquée faite avec un renvoi à l'« État de D4 », afin que la fermeture ne trouve pas de ligne sans étape soldée.
17. En tant que relecteur, je veux que le §10 dise « levé » ou renvoie aux TROU ouverts, afin que le tableau des risques reflète l'état réel.
18. En tant que mainteneur, je veux que l'écriture d'une table avec une clé convertie (et pas seulement la lecture) soit aussi revue, afin que `t[1.5]=v` écrit en `t[1]` par le C soit couvert.
19. En tant que mainteneur, je veux que le décalage `- 1` ou `+ 1` appliqué par le C entre la conversion et l'indice (base 0/1) soit noté quand il existe, afin qu'un lecteur ne prenne pas l'indice C pour la clé Lua.
20. En tant qu'agent qui exécute cette PRD, je veux ne jamais lancer l'appli raylib, afin de respecter `CLAUDE.md` : toute mesure passe par `make float-audit` et `make frames-check`, sans fenêtre.
21. En tant que mainteneur, je veux que si un site « fractionnaire possible » semble trivialement corrigeable, la PRD ne le corrige quand même pas, afin que toute correction divergente passe par la règle d'attribution d'une étape D, avec sa propre PRD.
22. En tant que mainteneur, je veux que les commentaires éventuellement touchés suivent la règle de `CLAUDE.md` (un `/* TROU-n */` nu au site, rien d'autre), afin de ne pas réintroduire de récit de chantier dans les sources ; D4 n'en ajoute que si un TROU est ouvert.

## Implementation Decisions

- **Pas de code.** D4 ne touche ni `src/`, ni les outils, ni `Makefile`. Seuls changent `docs/plan-2.md` et, si un site est « fractionnaire possible », `docs/trous-de-portage.md` (plus le `/* TROU-n */` nu au site, seule exception tolérée dans `src/`, sans effet sur la compilation).
- **Où vit la table.** Dans une section « État de D4 » de `docs/plan-2.md`, au format des états précédents (puces, puis un tableau). Pas de rapport versionné sous `tools/`, pas de contrôle dans `make test` : la revue est une lecture de code et une citation de mesures, elle n'a rien à rejouer. (Choix de simplicité ; un rapport + contrôle ne se justifierait que si la revue devait être refaite à chaque changement, ce que le plan ne demande pas.)
- **Colonnes de la table** : fichier et fonction C ; table et clé côté Lua avec ligne de `lua.lua` ; provenance (C1 / D1 / D0b / balayage) ; lecture ou écriture ; verdict (entière par construction, chaîne par construction, 0 fractionnaire sur le filet, fractionnaire possible) ; source (lignes Lua ou unstash) ; audit (`appels N | fractionnaires M` de la ligne du site). Les 11 de D0b peuvent tenir en une ligne par groupe avec renvoi.
- **Les 16 sites C1 à revoir**, repris de l'historique de l'annotation (`git log -p -- tools/float_annotation.txt`, commit `ecef59d`) : `pico8_hy_hk` (`po[r.k]`), `get_by_value` et `set_by_value` de `pico8_economy_formulas.c`, `get_by_value` de `pico8_economy_score.c`, `pico8_da` (`es[pt.mi]`), `pico8_db` ×2 (`ct[pt.mi]`, `ct[oz.mi]`), `sum_by_mi_visit` (`b_[pt.mi]`, `co[pt.mi]`), `bb_visit` (`po[min(pt.mi,#po)]`), `pico8_dc` (`fl[qc.fa]`), `pico8_jo_pq` (`gb[self.hp]`), `pico8_jo_gh` (`jo.dz[self.mi]`), `pico8_jo_kj` `mi` (`jo.ka[self.mi]`), `get_by_value` de `pico8_score_panel.c` (`fm.po[r.k]`), `pico8_pj_hb` (`ef[fm.lo]`), `pico8_gi` (`fm.fu[t.nk]`). Leurs justifications de C0 contiennent déjà la plupart des sources d'intégralité (constantes du cart, `mi` dans {0,1,2} ou `min`/`mid` de compteurs `#`, littéraux de classe, `lo` = 1 + littéraux) : la revue les **reformule en preuve d'intégralité** et les vérifie contre `lua.lua`, elle ne les recopie pas aveuglément. Point d'attention : un `mid`/`min` de compteurs entiers reste entier, mais toute division, tout `*0.5`, toute constante décimale dans la chaîne de la valeur casse la preuve.
- **Le site D1** : `pico8_mz_kj`, `pico8_fixed_flr(ratio)`, `kk[flr(sr/self.oi*8)]` — `flr` explicite du Lua ; à l'audit, ligne 108, 3 080 appels, 0 fractionnaire (mesure de rédaction, à refaire).
- **Lecture de l'audit.** `make float-audit` (dans `make test`, ~15 s) donne par (fichier, ligne) : appels, négatifs, fractionnaires. Trois limites à appliquer telles quelles :
  - *site non atteint* (appels 0 : les 5 de G5) → le compte ne prouve rien ; seul le verdict par construction vaut ;
  - *aide partagée* (`pico8_particle_to_int` : 2 094 431 appels, 1 157 455 fractionnaires à la rédaction) → le compte agrège tous les appelants et ne dit rien de l'appelant qui indexe ; verdict par construction à ce site d'appel (ex. `cs[flr(l)]` : `flr` explicite) ;
  - *ligne partagée* (deux appels sur une ligne) → le compte est commun ; le signaler.
- **Balayage.** Les ~74 appels textuels de `pico8_fixed_flr_int` du cœur (76 sites d'audit) et tous les appelants des aides `*_to_int`. Critère « clé » : le `int` rendu (éventuellement décalé de ±1) arrive, directement ou via une variable locale, à un indice de table ou de liste. Les appels `pico8_ob_table_*_index` à indice littéral ou compteur de boucle C sont hors revue (aucune valeur 16.16 en amont) ; ceux à indice issu de la clé positionnelle de l'unstash (`key.index`) aussi, entière par construction du format.
- **Verdict « fractionnaire possible » → TROU, pas correctif.** L'étape est une revue (texte du plan). Une correction changerait le comportement (Lua `nil` là où le C trouvait une entrée) et relèverait de la règle d'attribution d'une étape D (S, A, F, complément neutre, variante suffisante), qui n'a pas sa place dans une revue. L'entrée TROU donne : site, clé Lua, source montrant la valeur fractionnaire possible, comportement Lua vs C, atteinte par le filet (compte d'audit), et « non corrigé ».
- **Aucun faux positif par prudence.** Un site n'est « fractionnaire possible » que si une source montre un chemin où la valeur est non entière ; l'absence de preuve d'intégralité sans chemin exhibé est consignée comme « non établi » dans la table, avec ce qui manque — et le §10 n'est alors pas levé pour ce site.
- **Chiffres de sortie à consigner** : nombre de sites clé revus par provenance et par verdict, nombre d'appels balayés « pas une clé », passe `make float-audit` de référence (totaux : sites, passages), `make frames-check` identique sur les 12 fichiers sans `frames-baseline`, `make test` vert.

## Testing Decisions

- **Pas de nouveau test.** Aucune ligne de code ne change ; un test n'aurait rien à fixer. Les protections existantes suffisent et sont les portes de sortie :
  - `make test` vert (dont `float-inventory-check`, `constants-check`, `float-audit` : la liste C1 et les sites D inchangés, 0 écart, 0 nouveau ≠ `flr`) ;
  - `make frames-check` identique sur les 12 fichiers de `FRAMES_EVENTS`, **sans** `frames-baseline` ;
  - `git diff --stat` limité à `docs/plan-2.md`, éventuellement `docs/trous-de-portage.md` et des `/* TROU-n */` nus.
- **Ce qui fait une bonne vérification ici** : chaque verdict se vérifie seul, par une ligne de `lua.lua` citée ou un compte de l'audit cité avec son nombre d'appels — pas par un jugement du relecteur. Un compte cité doit être retrouvable dans la sortie de `make float-audit` sur l'arbre de D4.
- **Prior art** : la colonne « Clé de table » de la table de l'« État de D0b » et la section « Non protégés par le filet (D0b, G5) » sont le modèle exact du verdict attendu ; les puces « Corrections non vues par le filet (D1) » sont le modèle de la lecture des comptes d'audit par site.

## Out of Scope

- Toute conversion ou correction de site, y compris d'un site trouvé « fractionnaire possible » (→ TROU, correction dans une étape ultérieure avec sa PRD).
- Tout nouvel outil, rapport sous `tools/`, contrôle dans `make test`, fixture ou test.
- Les clés chaîne en général (dispatch `pico8_ob` par nom), sauf les branches numériques des 5 sites G5 déjà revues en D0b.
- La Phase E (définitions `from_double`/`to_double`, mots de commentaire, `-lm`), le volet `malloc`, l'audio.
- Lancer l'appli raylib (interdit par `CLAUDE.md`).
- `tools/float_observe.txt` (outil en panne depuis D1, constat pour E).

## Further Notes

- Mesure faite à la rédaction (`make float-audit`, arbre courant) : 80 lignes de sites, 32 corrections non vues, 4 lignes partagées, 5 551 169 passages ; les 16 sites C1 et le site D1 cités ont tous 0 fractionnaire, avec des nombres d'appels de 2 (`pico8_jo_pq`) à 1 211 558 (`pico8_dc`). Plusieurs sites C1 sont très peu atteints (`pico8_jo_pq` 2, `pico8_db` 4 et 4, `pico8_hy_kt` 4) : c'est pour eux que le verdict par construction porte l'essentiel, le compte d'audit n'étant qu'un complément.
- Candidats connus hors étiquette, à trancher par le balayage : `pico8_cr` → `ek[flr(tl)-1]` côté C (vérifier la clé Lua exacte), `pico8_particle.c` → `cs[flr(l)]` via `pico8_particle_to_int` (flr explicite), `pico8_score_panel_display.c` → `gs[3426-flr(yr)]` (déjà revu en D0b, G2).
- Les numéros de ligne de §3.2 (`pico8_score_panel.c:110`, `pico8_planet.c:309`, `economy_formulas.c:55,66`) datent du cadrage et ont dérivé ; la table de D4 identifie les sites par fichier + fonction, et ne cite la ligne que comme indice avec la mesure de l'audit.
- Un seul chunk : la revue ne se découpe pas utilement (une table, un commit docs).
