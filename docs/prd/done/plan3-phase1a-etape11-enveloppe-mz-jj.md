---
title: "Plan 3, étape 1a.11 : l'enveloppe `mz.jj`, instance sans classe, devient une struct sans parent qui porte `qs`"
description: "La table de classe `mz.jj` (`{qs=\"-too far-\"}`) est encore vue par le code à travers une instance sans classe (tag `INSTANCE`, `cls` NULL, `fields` = table `jj` du module). Elle devient une struct statique à en-tête, avec un seul membre `qs`, un lecteur qui ne connaît que `qs` et aucun écrivain. La table `jj` du module disparaît, le membre `jj` de chaque `mz` pointe sur cette struct, et le seul lecteur générique de l'enveloppe (`et.qs` dans `dl`) passe au lecteur à double forme."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Étapes de la phase 1a ») :

> 1a.11 — enveloppe `mz.jj` : sans classe (`wormhole.c:293`).

Règles du §5 qui s'appliquent à ce commit :

> Chaque classe devient un `struct` dont le premier membre est la struct de sa classe parente ; chaque champ de sa chaîne (défauts et champs écrits sur instance) devient un membre de type `pico8_value_t`, **nom minifié conservé**. Nil reste `pico8_value_ref(NULL)`.
>
> **Défauts mutés sur la classe** : […] deviennent un état statique partagé de la classe, jamais un champ d'instance. Même traitement pour tout autre défaut réaffecté après l'initialisation **ou dont la table pointée est modifiée en place** […].
>
> **Instances sans classe** : converties comme des classes à part entière, sans parent.
>
> **Coexistence** : une instance convertie porte un nouveau tag (`PICO8_TYPE_TAG_STRUCT`) et un pointeur vers sa table de méthodes, dans le même premier emplacement qu'aujourd'hui. `hv`/`cv`/`cg`, `dc`, `mv` traitent les deux formes pendant la migration ; l'ordre d'enregistrement est préservé.
>
> **Accès directs aux défauts** : tout site hors `pico8_class.c` qui lit ou écrit `<classe>->defaults` […] migre dans le commit de cette classe.

§6 : l'étape ne nomme aucun test de régression. Aucun test de régression ne lit l'enveloppe. Ils doivent tous passer avec un diff vide. §7 : le commit passe `make class-coverage-net` avec une sortie identique à `tools/class_coverage_net_expected.txt`. L'enveloppe n'est pas une classe du registre, elle n'a aucun nom dans `KNOWN[]` et n'enregistre rien au crochet de couverture. Un site découvert en cours d'étape et couvert par une règle existante du §5 est ajouté à la liste des étapes dans le commit, avec « découvert à 1a.11 » dans le message.

### Ce que 1a.1 à 1a.10 ont posé, réutilisé tel quel

- `PICO8_TYPE_TAG_STRUCT`, `pico8_struct_header_t`, descripteur `pico8_struct_class_t` (méthodes indexées par l'enum, lecteur de champ, écrivain). `pico8_entity_get_field` / `pico8_entity_set_field` sont à double forme. L'écrivain asserte s'il n'y a pas d'écrivain dans le descripteur ou si le nom n'a pas de membre. `pico8_instance_get_field` asserte s'il reçoit une struct.
- **Modèle des instances sans classe : le `je` provisoire (1a.7, ec01e86) et le `mo` provisoire (1a.9, 9aa4812).** Une struct à en-tête seul, sans `pico8_ke_t` (sans parent). Un descripteur `static const` sans méthode. Pas d'écrivain (NULL) : une écriture qui l'atteint arrête l'exécution. Un unique objet statique initialisé à la compilation, jamais alloué ni libéré. Un accesseur public du descripteur. Aucun nom de couverture. Une caractérisation figée sur l'ancien code, lue seulement par l'API à double forme, puis une vérification de forme ajoutée après la conversion. L'enveloppe `jj` diffère sur un seul point : elle **porte un champ**, `qs`, donc son lecteur connaît `qs`, là où ceux de `je`/`mo` ne trouvaient jamais rien.
- **1a.10 (968e059)** : `mz` est une struct `pico8_mz_t` dont le membre `jj` est une REF vers la table `jj` possédée par le module (`g_mz_jj`, parsée depuis la spec par `pico8_wormhole_init`, libérée par `pico8_wormhole_shutdown`). `mz:hq` lit `jj` **sur la classe** : il pose en `et` l'enveloppe du module (`pico8_mz_jj_wrapper()`), jamais le membre. L'enveloppe est aujourd'hui un `pico8_instance_t` alloué par `malloc` dans `pico8_wormhole_init` (`wormhole.c:388-391`), avec le tag `INSTANCE`, `cls` NULL et `fields` = `g_mz_jj`. Elle est libérée par un `free` simple au shutdown, avant la table. `test_pico8_mz` vérifie que `fields` de l'enveloppe et le membre `jj` sont la même table.

### Ce que fait le cart

`pico8/lua.lua` :

```
:552-558  function bv(self,mp,rp)self.ra,self.p=mp,mp/128
            local nq,c=ij(self.p,cg.np)self.et= nq<self.dw and c or
            jp(cg.jc,function(sw) return be(sw.qu.p,sw.re.p,self.p,0.0032)end)[1]or
            not ui.hw(rp)and {}
            if (self.et)self.et.fo=self end
:559-567  function dl(self)if self.me=="kz" then local mp,t=je.ra
            if self.et then t=self.et.qs if (not t)spr(31,…) elseif self.od then t=self.od.."$"end
            io(t or "",…)end end
:569      function d_(self)if not self.et and fm:lu(self.od)then …
:761      jj=o(qs="-too far-"),
:767-771  function mz:hq(...)bv(self,...)local nq
            nq,self.ij=ij(self.p,self.ox)if nq>ov("space folding",0.19,0.27)then
            self.et=mz.jj end end
:783-784  function mz:kj(p) … if not self.et then circ(…) end …
```

Points décisifs :

- **`jj` n'est jamais modifiée en place.** C'est la question posée par l'étape : `bv` écrit-il `et.fo` dans l'enveloppe ? Non. `bv` pose `fo` sur l'`et` **qu'il vient de calculer**, c'est-à-dire la planète de `cg.np`, la voie de `cg.jc` ou le `{}` neuf. Ces trois valeurs viennent de `ij`, `jp` ou d'un constructeur de table, et aucune ne peut être `mz.jj`. `mz:hq` remplace `et` par `jj` **après** le retour de `bv`, sans rien y écrire. Au `hq` suivant, `bv` recalcule `et` avant d'écrire `fo`, et l'ancien `et` (éventuellement `jj`) n'est plus touché. Le cart n'a aucune autre écriture sur `et` ou sur `mz.jj`. Les seuls accès à l'enveloppe sont donc : une lecture de `qs` (`dl`), trois tests de vérité de `self.et` (`dl`, `d_`, `mz:kj`), qui ne la déréférencent pas, et l'affectation `self.et=mz.jj`.
- `jj` est un **défaut de la classe `mz`**, partagé par tous les `mz`. Une seule table existe pour tout le programme. C'est déjà un état de classe, et il est **immuable**.
- `jc.et` (`slipway.c:129`, dont le `{qs="-too far-"}` de `jc:ce`, `slipway.c:255`) est **un autre champ, sur une autre entité** : `jc:hq` y pose le résultat de `jc:ce` et y écrit `fo`. L'enveloppe `mz.jj` n'y entre jamais. Le `{}` de `planet_family_trade.c:94` est l'argument `s` passé à `nr:gv` par `nr:ldown`, pas un `et`. Aucun des deux n'est touché ici (1a.13, 1a.37, 1a.39).

### Emprise réelle de l'enveloppe (mesurée sur 968e059)

| Aspect | Mesure |
|---|---|
| Construction | 1 site : `pico8_wormhole_init` (`wormhole.c:388-391`), `malloc`, tag `INSTANCE`, `fields` = `g_mz_jj`, `cls` NULL. Pas de `pico8_class_t`, pas de `lt` : **jamais dans `hv`/`cv`/`cg`**. Elle n'est donc jamais atteinte par `dg`, `he`, `dc`, `gw`, `jz` ni `ui:bh`. |
| Libération | `pico8_wormhole_shutdown`, par un `free` simple, avant `pico8_ob_table_free(g_mz_jj)`. |
| Champs | `qs` = `"-too far-"`, et rien d'autre, pendant toute sa vie. |
| Méthodes | aucune. |
| `->defaults` | aucun. La lecture `->defaults` de `jj` a disparu en 1a.10. |
| Qui la pose | `pico8_mz_hq` (`wormhole.c:103`) : `self->et = pico8_value_ref(pico8_mz_jj_wrapper())`. C'est le seul site. |
| Lecteurs de l'enveloppe | `pico8_dl` (`probe.c:217-220`) : `pico8_instance_get_field(et_inst, "qs", …)` sur l'`et` d'un `mz`, d'un `qh` ou d'un `g_`. **Lecteur à simple forme : il assertera sur une struct.** Les `et` qui y arrivent sont une planète `jo`, une voie `jc`, le `{}` jeté de `bv` (tous encore instances), `false`, ou l'enveloppe. |
| Tests de vérité de `et` (sans déréférencement) | `pico8_dl` (`probe.c:214`), `pico8_d_` (`probe.c:269`), `pico8_mz_kj` (`wormhole_display.c:68`). Ils ne changent pas. |
| Écrivains qui l'atteignent | **aucun**. `pico8_bv` (`probe.c:181-186`) écrit `fo` par `et_inst->fields` sur l'`et` qu'il a calculé, qui n'est jamais l'enveloppe. Son assert (« et is an instance ») reste vraie. `pico8_jc_hq` (`slipway.c:130-136`) écrit sur `jc.et`, qui n'est jamais l'enveloppe. |
| Lecteurs de `fo` | `slipway_display.c:208` (sur une voie `jc`) et `planet_display.c:564` (sur une planète). Aucun ne lit l'enveloppe. |
| Membre `jj` de `mz` | REF vers `g_mz_jj` (`wormhole.c:341`), rendu par `pico8_mz_field`. Aucun code de `src/` ne lit ce membre. Seul `test_pico8_mz` le lit (`:321-325`, `:762`). |
| Tests qui lisent l'enveloppe | `test_pico8_mz` uniquement. `et_kind` (`:171`) fait une comparaison d'identité. `fold_hq` (`:402-405`) lit `qs` par `pico8_entity_get_field`, donc déjà à double forme. `test_construction` (`:321-325`) lit `jj.qs` par `pico8_ob_table_get_name` sur le membre, puis vérifie `fields == jj`. `test_mz_is_a_struct` (`:760-762`) vérifie la forme `INSTANCE`/`cls` NULL/`fields` == membre. `test_pico8_playability.c:9` n'en parle que dans un commentaire. |
| Scénarios figés qui traversent l'enveloppe | `HQ_HASH` (hq loin de toute ancre, `ET_JJ`, `qs` replié), `EY_HASH` (`ey` à `t`≥15 avec `et` = enveloppe : refus, crédits inchangés), `DRAW_HASH` (étiquette « -too far- » dessinée par `dl`), `PLAYTHROUGH_HASH` (genre de `et` par frame). |
| Couverture | Aucun nom. `mz  146  9/15` inchangé attendu. |
| Clé dynamique | Aucune. |

## Solution

1. **Struct de l'enveloppe** (publique dans le module du trou de ver, à côté de `pico8_mz_t`) : un `pico8_struct_header_t`, puis un seul membre `qs` (`pico8_value_t`). Elle n'a pas de `pico8_ke_t` : c'est une instance sans classe, sans parent, et elle n'est jamais une entité.
2. **Un seul objet statique, initialisé à la compilation** (modèle `je`/`mo`) : tag `PICO8_TYPE_TAG_STRUCT`, descripteur de l'enveloppe, `qs` = chaîne `"-too far-"`. Il n'est jamais alloué ni libéré. C'est **l'état de classe** qui représente `mz.jj`, partagé par tous les `mz`, et il est immuable.
3. **Descripteur `static const`** : aucune méthode. Un lecteur qui rend `qs`, et 0 pour tout autre nom (dont `fo`). Un écrivain **NULL**, parce qu'aucun écrivain générique ne l'atteint (mesuré). Une écriture qui l'atteindrait ferait partir l'assert de `pico8_entity_set_field`, et cette assertion est le garde-fou de la règle ci-dessous. Un accesseur public du descripteur s'ajoute, comme `pico8_mo_placeholder_struct_class()`.
4. **`pico8_mz_jj_wrapper()`** rend l'adresse de cet objet, avec le type de la struct de l'enveloppe au lieu de `pico8_instance_t *`. Il n'est **jamais NULL** et ne dépend plus de `pico8_wormhole_init`. Son contrat d'en-tête est corrigé en conséquence.
5. **La table `jj` du module disparaît.** `g_mz_jj` n'est plus parsée ni libérée. La spec parsée par `pico8_wormhole_init` ne garde que `nu` et `kk`. La spec de la `pico8_class_t` de `mz` **ne change pas** : elle reste la copie en lecture jamais lue. Le membre `jj` de chaque `mz` devient une REF vers l'objet statique de l'enveloppe. Le membre et `et` après un « trop loin » désignent donc **le même objet**, comme en Lua où `self.et` et `mz.jj` sont la même table.
6. **`pico8_mz_hq`** : même affectation (`et` = REF vers l'enveloppe), seul le type de l'accesseur change.
7. **`pico8_wormhole_init` / `_shutdown`** : l'init ne construit plus l'enveloppe. Le shutdown ne la libère plus et ne libère plus `g_mz_jj`. Il libère la classe, puis `nu` et `kk`. Il reste idempotent et sûr avant la première init.
8. **Coexistence, le seul lecteur générique** : dans `pico8_dl`, la lecture de `qs` sur `et` passe de `pico8_instance_get_field` à `pico8_entity_get_field`, et le type local suit (pointeur générique). Le résultat est identique sur une planète, une voie, un `{}` jeté (instances). Sur l'enveloppe, il rend `qs` par le lecteur de la struct. Rien d'autre ne change dans `probe.c` : les tests de vérité de `et`, l'écriture de `fo` par `bv` et son assert restent tels quels. `slipway.c`, `slipway_display.c`, `planet_display.c` et `planet_family_trade.c` ne changent pas.
9. **Règle appliquée à un éventuel `fo` sur l'enveloppe** : c'est « défaut dont la table pointée est modifiée en place » (§5, défauts mutés) : état statique partagé de la classe, jamais un champ de chaque `mz`. Il est prouvé plus haut qu'aucune écriture de ce genre n'existe. Si l'implémenteur en trouvait une (assert de l'écrivain NULL sur un rejeu ou un test), il appliquerait cette règle : membre `fo` dans l'objet statique, écrivain qui n'accepte que `fo`, `mz.jj` ajouté à la liste des défauts modifiés en place du §5 et site nommé à la ligne 1a.11. Ce n'est pas un arrêt, mais c'est un écart au PRD à justifier dans le commit.
10. **`test_pico8_mz`** : trois endroits changent, et seulement eux.
    - `test_construction` : la lecture de `jj.qs` passe à `pico8_entity_get_field` sur le membre `jj`. La valeur figée `"-too far-"` ne change pas. L'identité « `fields` de l'enveloppe == membre `jj` » devient « enveloppe == membre `jj` ».
    - `test_mz_is_a_struct` : la vérification de forme de l'enveloppe (tag `INSTANCE`, `cls` NULL, `fields` == membre) devient : tag `STRUCT`, descripteur de l'enveloppe, enveloppe == membre `jj`.
    - Le commentaire qui annonce « the `jj` wrapper stays a classless instance » (`:736-737`) est corrigé.
    Aucune constante `*_HASH` ni aucun compte figé ne bouge.
11. **Plan** : ajouter à la ligne 1a.11 de `docs/plan-3.md` le texte « Découvert à 1a.11 : `et.qs` lu par `dl` (`probe.c:219`) ». Aucun défaut modifié en place ne s'ajoute au §5 (sauf application du point 9).

## User Stories

1. En tant qu'autopilote de la phase 1a, je veux que l'enveloppe `mz.jj` soit la seule chose convertie par ce commit, afin de respecter « une classe par commit » (§7).
2. En tant qu'autopilote, je veux que l'emprise de l'enveloppe soit mesurée dans le PRD (construction, libération, champs, qui la pose, lecteurs, écrivains, tests, scénarios figés), afin que chaque site touché soit vérifiable.
3. En tant que lecteur du code, je veux que l'enveloppe soit une struct à en-tête avec le seul membre `qs`, nom minifié conservé, afin que la traçabilité vers `jj=o(qs="-too far-")` tienne.
4. En tant que lecteur du code, je veux que l'enveloppe n'ait pas de `pico8_ke_t`, afin que « sans parent » (§5) se lise dans le type.
5. En tant que mainteneur, je veux un unique objet statique initialisé à la compilation, afin qu'il n'y ait ni allocation ni libération à suivre, comme pour `je` et `mo`.
6. En tant que mainteneur, je veux que la table `jj` du module disparaisse, afin qu'il ne reste qu'une seule représentation de `mz.jj` au lieu d'une table et d'une enveloppe qui l'emprunte.
7. En tant que mainteneur, je veux que le membre `jj` de chaque `mz` pointe sur l'objet de l'enveloppe, afin que « chaque champ de sa chaîne est un membre » tienne et que le membre et `et` désignent le même objet, comme en Lua.
8. En tant que gardien du filet, je veux que `mz:hq` pose toujours en `et` l'enveloppe quand l'ancre la plus proche est trop loin, afin que « -too far- » s'affiche exactement comme avant.
9. En tant que gardien du filet, je veux que `dl` lise `qs` sur l'enveloppe struct par le lecteur à double forme, afin que l'étiquette soit dessinée sans faire partir l'assert de la couche.
10. En tant que gardien du filet, je veux que `dl` rende le même texte, ou le même sprite 31, sur un `et` planète, voie ou `{}` jeté, afin que `qh` et `g_` ne bougent pas.
11. En tant que gardien du filet, je veux que `d_` refuse (`rn(57)`, `p_`) quand `et` est l'enveloppe, sans la déréférencer, afin que le refus hors de portée soit identique.
12. En tant que gardien du filet, je veux que `mz:kj` n'efface pas l'anneau quand `et` est l'enveloppe, afin que le rendu soit identique au pixel près.
13. En tant que gardien du filet, je veux que `bv` continue à poser `fo` sur la cible qu'il vient de calculer et jamais sur l'enveloppe, afin que les voies survolées s'allument comme avant.
14. En tant que mainteneur, je veux que l'enveloppe n'ait pas d'écrivain, afin qu'une écriture jamais mesurée arrête l'exécution au lieu de muter en silence un défaut partagé.
15. En tant que mainteneur, je veux que la règle qui couvrirait un `fo` écrit sur l'enveloppe soit nommée d'avance (défaut modifié en place, état statique de classe), afin que l'implémenteur sache quoi faire sans improviser un membre par `mz`.
16. En tant que mainteneur, je veux que le lecteur de l'enveloppe ne rende que `qs`, afin qu'un `fo`, un `ra` ou une méthode lus dessus restent absents comme sur la table d'origine.
17. En tant qu'auteur des étapes 1a.13, 1a.37 et 1a.39, je veux que `jc.et`, le `{qs="-too far-"}` de `jc:ce` et le `{}` de `nr:ldown` restent des instances, afin que chacun reste une étape à part.
18. En tant que gardien de la couverture, je veux qu'aucun nom ne s'ajoute à `KNOWN[]` et que l'enveloppe n'enregistre rien au crochet, afin que `make class-coverage-net` rende une sortie identique.
19. En tant que gardien de la mémoire, je veux que le shutdown ne libère plus que la classe, `nu` et `kk`, et qu'il reste idempotent, afin que les cycles init/shutdown des tests restent propres sous ASan.
20. En tant que mainteneur, je veux que `pico8_mz_jj_wrapper()` ne soit jamais NULL et que son contrat le dise, afin qu'aucun appelant ne teste une nullité devenue impossible.
21. En tant que mainteneur, je veux un accesseur public du descripteur de l'enveloppe, afin que les tests vérifient la forme sans connaître l'objet statique.
22. En tant que gardien des tests, je veux que `test_pico8_mz` ne change que son lecteur de `jj.qs`, son identité enveloppe/membre et sa vérification de forme, avec toutes ses constantes figées identiques, afin que la caractérisation garde son objet.
23. En tant que vérificateur, je veux une caractérisation de l'enveloppe figée sur l'ancien code (`qs`, absence de `fo` après des `hq` successifs, absence d'autres noms), afin de prouver ce que les empreintes existantes ne voient qu'indirectement.
24. En tant que gardien des tests de régression, je veux qu'ils passent tous avec un diff vide, afin que la conversion ne se valide pas en affaiblissant un test.
25. En tant que gardien du filet, je veux qu'aucun `.sha256`, aucune entrée de `FRAMES_EVENTS`/`RECORDINGS` ni aucune ligne des références de couverture ne bouge, afin que l'étape ne se valide pas en déplaçant sa cible.
26. En tant que lecteur du plan, je veux que la ligne 1a.11 nomme le site découvert (`dl`, `probe.c:219`), afin que la liste des étapes dise ce que l'étape a réellement touché.
27. En tant que lecteur du code, je veux que les commentaires rendus faux par le changement soient corrigés et que les ajouts ne portent qu'un contrat C ou un piège (CLAUDE.md), afin de ne laisser aucun contrat mensonger.

## Implementation Decisions

Aucun outil d'agent n'était disponible dans cette session pour lancer un agent `guideline` frais. J'ai chargé le skill `guideline` et je l'ai appliqué moi-même : hypothèses explicites, simplicité, changement chirurgical, critères vérifiables. Les décisions tranchées ainsi sont marquées (G).

- **Forme : en-tête + `qs`, et rien d'autre.** `qs` est le seul champ que la table porte et que quelqu'un lit. `fo` n'y est pas, parce qu'aucune écriture n'existe (preuve au Problem Statement). Ajouter un membre `fo` « au cas où » créerait un champ qui ne tombe sous aucune règle, donc un arrêt §7.
- **(G) Objet statique à la compilation, plutôt qu'alloué dans l'init.** C'est le modèle `je`/`mo`, et l'objet est une donnée de classe immuable. Rejeté : garder `malloc` dans `pico8_wormhole_init` pour préserver « NULL avant l'init ». Personne ne teste cette nullité (grep), et un `mz` n'existe jamais avant l'init. La nullité n'était qu'un effet de l'allocation.
- **(G) Fusion de la table et de l'enveloppe.** Rejeté : garder `g_mz_jj` à côté d'une struct qui recopie `qs`. On aurait deux représentations de `mz.jj`, avec un membre `jj` qui pointerait sur une table que plus rien ne lit. Rejeté aussi : supprimer le membre `jj` de `mz`. C'est un défaut de la chaîne (§5), et 1a.10 l'a gardé pour cette raison.
- **Écrivain NULL** (modèle `mo`) : l'assert de `pico8_entity_set_field` est la garde. La règle qui s'appliquerait sinon est écrite au point 9 de la Solution.
- **Règle du §5 qui couvre l'étape** : « Instances sans classe » pour la forme. « Défauts mutés… modifiée en place » ne s'applique **pas**, parce que rien ne mute. L'objet est pourtant unique et partagé, parce que c'est un défaut de classe et non un champ d'instance. Aucun ajout à la liste des défauts mutés connus.
- **Lecteurs et écrivains de `et`, un par un** :
  - `bv` (`probe.c:179-186`) : il écrit `et` sur `self` (déjà à double forme) et `fo` sur la cible calculée (planète, voie ou `{}`, jamais l'enveloppe). **Inchangé.**
  - `dl` (`probe.c:214`, test de vérité) : inchangé. `dl` (`probe.c:219`, `et.qs`) : **passe à double forme**, seul site modifié hors du module.
  - `d_` (`probe.c:269`, test de vérité) : inchangé.
  - `mz:hq` (`wormhole.c:103`) : même affectation, le type de l'accesseur change.
  - `mz:kj` (`wormhole_display.c:68`, test de vérité) : inchangé.
  - `jc:hq` (`slipway.c:129-136`, `jc.et` et `fo`) et `jc:ce` (`slipway.c:255`) : un autre champ, jamais l'enveloppe. **Inchangés.**
  - `slipway_display.c:207-208` (`jc.et`/`jc.fo`, tests de vérité) et `slipway_display.c:269` (`jc.fo=nil`) : inchangés.
  - `planet_display.c:564,584` (`fo` d'une planète) : jamais l'enveloppe. Inchangés.
  - `nr:ldown` (`planet_family_trade.c:94`, `{}` passé en `s` à `nr:gv`) : pas un `et`. Inchangé.
  - Tests : voir le point 10 de la Solution.
- **(G) Changement chirurgical dans `dl`** : seul l'appel du lecteur de `qs` et le type de la variable locale changent. La garde « `et` est une REF non nulle » reste telle quelle.
- **`test_pico8_entity_smoke`, `bg.c`, `cart.c`** : non touchés. `cart.c` appelle toujours `pico8_wormhole_init`/`_shutdown` et asserte le parent de `mz`.
- **Commentaires** (CLAUDE.md) à corriger parce que le changement les rend faux :
  - `wormhole.h:18-19` : « `nu`, `kk` and `jj` point at the tables the module owns ». `jj` pointe maintenant sur l'enveloppe.
  - `wormhole.h:42-44` : contrat du shutdown, qui ne libère plus ni l'enveloppe ni la table `jj`.
  - `wormhole.h:57-62` : contrat de `pico8_mz_jj_wrapper()`. La struct statique n'est jamais libérée, jamais NULL et sans écrivain. « classless instance », « `fields` are the module's `jj` table » et « NULL before init » deviennent faux.
  - `wormhole.c:398-399` : « Plain `free()` on the wrapper… ».
  - Le message de l'assert de `pico8_mz_spec_table` (« nu, kk and jj »).
  - `test_pico8_mz.c:736-737`.
  À ajouter : le contrat de l'objet statique (unique, jamais alloué ni libéré, sans écrivain : une écriture qui l'atteint arrête l'exécution), dans l'esprit de celui de `g_mo_placeholder`. Tout le reste va dans le message de commit.

## Implementation Chunks

**Un seul commit** (§7). Chaque morceau est un point d'arrêt TDD qui laisse `make test` et `make frames-check` verts. `make class-coverage-net` est lancé au moins sur le code final.

1. **Caractérisation de l'enveloppe sur l'ancien code.** Un nouveau scénario dans `test_pico8_mz` (voir Testing), dont les valeurs sont figées **avant** toute modification de `wormhole*.c`/`.h` et `probe.c`.
2. **Coexistence, sans rien convertir.** `dl` lit `et.qs` par `pico8_entity_get_field`. Le filet et la couverture sont trivialement identiques.
3. **Enveloppe convertie.** Struct, descripteur, objet statique, accesseurs, membre `jj`, suppression de `g_mz_jj` et de la construction et libération de l'enveloppe, contrats, les trois endroits de `test_pico8_mz` (point 10), la vérification de forme, la ligne 1a.11 du plan. La caractérisation passe **sans changer ses valeurs figées**.

## Testing Decisions

- **Un bon test observe le comportement** : ce que rendent `pico8_entity_get_field` et `pico8_mv`, le genre de `et`, les crédits de `fm`, les pixels du framebuffer, le compte de couverture. Jamais l'agencement mémoire au-delà du tag, du descripteur et de l'identité de l'objet.
- **Nouveau scénario `test_jj_wrapper` dans `test_pico8_mz`** (morceau 1), lu **uniquement** par l'API à double forme, dans un fils forké juste après `pico8_cart_load()` (`in_fresh_child`). Il est figé sur l'ancien code :
  1. Sur `pico8_mz_jj_wrapper()` : `qs` présent, chaîne `"-too far-"`. `fo`, `ra`, `p`, `me`, `t`, `nu`, `et` absents. `pico8_mv(wrapper, "kj"|"ev"|"hq", …)` ne trouve pas de méthode.
  2. Un `mz` reçoit un `hq` loin de toute ancre (`ET_JJ`), puis un `hq` sur une voie (`ET_JC`, si la partie en a une, sinon noté comme omis), puis un `hq` dans le vide (`ET_THROWAWAY`), puis de nouveau un `hq` loin (`ET_JJ`). Après chaque `hq`, l'enveloppe n'a toujours pas de `fo` et son `qs` est inchangé. La cible `jc`/`{}` a bien `fo` == le `mz`.
  3. Deux `mz` vivants en `ET_JJ` ont **le même** `et` (identité de la REF). Le membre `jj` n'est pas lu ici : sur l'ancien code, c'est une table nue, que `pico8_entity_get_field` ne sait pas lire. L'identité « membre `jj` == enveloppe » est une assertion de forme, ajoutée au morceau 3.
- **Forme** (ajoutée au morceau 3, dans `test_mz_is_a_struct`) : l'enveloppe est taguée `STRUCT` avec le descripteur de l'enveloppe (accesseur). Le membre `jj` d'un `mz` est l'enveloppe. Dans un fils, `pico8_entity_set_field(wrapper, "fo", …)` fait `SIGABRT`.
- **Constantes figées existantes** : `OX_HASH`, `HQ_HASH`, `KZ_*`, `EY_HASH`, `PR_*`, `DRAW_HASH`, `PLAYTHROUGH_*`, `GEOHARVEST_*` sont identiques. `HQ_HASH`, `EY_HASH`, `DRAW_HASH` et `PLAYTHROUGH_HASH` traversent l'enveloppe et sont la preuve principale.
- **Mutations** : l'implémenteur remet localement au moins 6 erreurs plausibles et note dans le commit celles qui sont attrapées :
  - `qs` erroné ou absent ;
  - lecteur qui rend aussi `fo` ou un autre nom ;
  - `dl` revenu à `pico8_instance_get_field` ;
  - membre `jj` pointé sur un autre objet que l'enveloppe ;
  - écrivain qui accepte et jette ;
  - `mz:hq` qui pose une enveloppe neuve à chaque appel ;
  - `bv` qui écrit `fo` après le remplacement par `jj`.
  Un mutant équivalent (par exemple un objet alloué par `malloc` au lieu du statique : fuite seulement) est justifié dans le commit.
- **Tests de régression** : tous passent avec un diff vide, en particulier `playability`, `geoharvest_burst` et `slipway_drag`.
- **`test_pico8_iv`, `test_pico8_qb`, `test_pico8_nf`, `test_pico8_je`, `test_pico8_mo`, `test_pico8_fq`, `test_pico8_mh`** : diff vide, valeurs figées identiques.
- **Couverture** : `make class-coverage-net` rend 0, avec une sortie identique à la référence (`mz  146  9/15`).
- **Prior art** : `test_pico8_mo` et `test_pico8_je` (objet statique, forme ajoutée après, écriture qui abandonne dans un fils), `test_pico8_mz` (fils forkés, `et_kind`, `fold_hq`).

## Critères du vérificateur

Chacun se traduit en `FAILED:` par le vérificateur de `prd-autopilot` (§7 ; seul arrêt que `plan-pilot` honore).

- `FAILED:` si `make test` ou `make frames-check` est encore rouge après **3 tentatives** de correction sur ce commit.
- `FAILED:` si le diff modifie ou supprime un `*.sha256` du filet, ou retire une entrée de `FRAMES_EVENTS` ou de `RECORDINGS`.
- `FAILED:` si `make class-coverage-net`, relancé sur le code final, ne rend pas 0 ou si sa sortie n'est pas identique à `tools/class_coverage_net_expected.txt`.
- `FAILED:` si `tools/class_coverage_net_expected.txt`, `tools/class_coverage_exempt.txt` ou `KNOWN[]` est modifié, ou si l'enveloppe enregistre un nom au crochet de couverture.
- `FAILED:` si un champ de l'enveloppe ne tombe sous aucune règle du §5. En particulier : un membre autre que `qs` ajouté sans écriture mesurée, ou une écriture trouvée sur l'enveloppe et traitée autrement que par la règle des défauts modifiés en place (point 9), par exemple un `fo` recopié dans chaque `mz`.
- `FAILED:` si le membre `jj` d'un `mz` et l'`et` posé par `mz:hq` ne désignent pas le même objet, ou si `mz:hq` lit `jj` sur l'instance au lieu de la classe.
- `FAILED:` si un test de régression de bug (§6) a un diff non vide, ou devrait changer d'intention pour passer.
- `FAILED:` si une autre classe ou instance sans classe change de forme. En particulier : `mz`, `jc`, le `{}` jeté de `bv`, le `{qs="-too far-"}` de `jc:ce`, le `{}` de `nr:ldown`, l'enveloppe `ie.ke`, `qh`, `g_`, `jo`, `nr`, `bg`, ou un provisoire. Aussi si `slipway.c`, `slipway_display.c`, `planet_display.c`, `planet_family_trade.c` ou `bg.c` est modifié.
- `FAILED:` si une valeur figée de `test_pico8_mz` (constantes `*_HASH`, comptes de frames et de ticks, valeurs de `test_construction`), du nouveau scénario de l'enveloppe, ou de `test_pico8_iv`, `test_pico8_qb`, `test_pico8_nf`, `test_pico8_je`, `test_pico8_mo`, `test_pico8_fq` ou `test_pico8_mh` a été changée après la conversion.

Critères de réussite (en plus) :

- grep : plus aucun `g_mz_jj\b`, `PICO8_TYPE_TAG_INSTANCE` ni `malloc`/`free` de l'enveloppe dans `wormhole.c`. Plus aucun `pico8_instance_get_field` dans `pico8_dl`. Plus aucun `->fields` ni `->cls` sur `pico8_mz_jj_wrapper()` dans `tests/`.
- La struct de l'enveloppe n'a que `pico8_struct_header_t` et `qs`. Son descripteur n'a aucune méthode et son écrivain est NULL.
- L'enum de méthodes est inchangé (`OL`, `PH`, `KJ`, `EV`, `LDOWN`, `MD`, `HQ`, `EY`, `KZ`, `PR`).
- La spec de `pico8_class_declare` de `mz` et son `pico8_class_register_named("mz", …)` sont inchangés.
- Hors de `wormhole.c`/`.h`, le diff de `src/` se limite à la lecture de `qs` dans `pico8_dl` (`probe.c`).
- Le diff de `tests/` se limite à `test_pico8_mz.c` : le nouveau scénario, les trois endroits du point 10 et les assertions de forme.
- La ligne 1a.11 de `docs/plan-3.md` nomme le site découvert, et le message de commit dit « découvert à 1a.11 ».

## Out of Scope

- `jc` (1a.13), dont `jc.et`, `jc.fo` et le `{qs="-too far-"}` de `jc:ce` (1a.39).
- Le `{}` jeté de `bv` (1a.38) et le `{}` de `nr:ldown` (1a.37).
- L'assert de `bv` sur `et` INSTANCE : elle reste vraie, et sa réécriture viendra avec la première cible convertie (`jc`, `jo`).
- L'enveloppe `ie.ke` (1a.15), qui a elle aussi `bv`/`dl`/`d_` pour méthodes.
- `mz` lui-même (fait en 1a.10), hors de son membre `jj`.
- Le typage C de `qs` (`const char *`) : phase 1b. Les tables `nu`/`kk` en tableaux fixes : phase D.
- L'audio, et lancer l'appli raylib (jamais).

## Further Notes

- Sans le morceau 2, le premier « trop loin » d'un `mz` ferait partir, après conversion, l'assert de `pico8_instance_get_field` dans `dl`, dès le premier `dc("ev")` qui suit. `DRAW_HASH` et `PLAYTHROUGH_HASH` le verraient, et le filet aussi si un rejeu survole le vide loin d'une ancre avec un `mz` en main.
- Le seul risque de sémantique est `fo` : en Lua, une écriture `jj.fo=…` persisterait sur la table de classe, donc pour tous les `mz`. L'écrivain NULL transforme cette hypothèse en arrêt visible au lieu d'un écart silencieux. Le scénario 2 prouve sur l'ancien code qu'elle n'arrive pas.
- Mesures faites par grep sur `src/` et `tests/` à 968e059, en lisant `pico8/lua.lua:552-574`, `:615-680`, `:752-796`, ainsi que `pico8_wormhole.c`/`.h`, `pico8_wormhole_display.c`, `pico8_probe.c`, `pico8_slipway.c`, `pico8_slipway_display.c`, `pico8_planet_display.c`, `pico8_planet_family_trade.c`, `pico8_entity.c` (`g_mo_placeholder`), `pico8_struct.h`, `pico8_class.c` (`pico8_instance_get_field`) et `tests/test_pico8_mz.c`.
