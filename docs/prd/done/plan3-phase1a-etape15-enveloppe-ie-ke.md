---
title: "Plan 3, étape 1a.15 : l'enveloppe `ie.ke`, instance sans classe à `cls` non nul, devient une vue struct sans parent qui porte `rr`/`s`/`me`/`od`"
description: "Pour lire un champ sur la classe-valeur que tient `self.ke` (`self.ke.rr`, `self.ke.s`, `dl(self.ke)`), `ie` construit sur la pile une instance sans classe (tag `INSTANCE`, `fields` vide partagé, `cls` = la classe tenue) : un artefact du portage pour atteindre la chaîne de défauts d'une classe à travers la chaîne d'une instance. Elle devient une struct sans parent, à en-tête, dont les quatre membres `rr`, `s`, `me`, `od` sont remplis depuis la chaîne de la classe-valeur par un nouveau lecteur de chaîne de classe ; le membre `ke` de `ie` garde la classe-valeur pour `ie:ldown`."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Étapes de la phase 1a ») :

> 1a.15 — enveloppe `ie.ke` (`research_icon.c:68`, `cls` non nul), lit `rr`/`s`.

Règles du §5 qui s'appliquent à ce commit :

> Chaque classe devient un `struct` dont le premier membre est la struct de sa classe parente ; chaque champ de sa chaîne (défauts et champs écrits sur instance) devient un membre de type `pico8_value_t`, **nom minifié conservé**. Nil reste `pico8_value_ref(NULL)`.
>
> **Instances sans classe** : converties comme des classes à part entière, sans parent.
>
> **Coexistence** : une instance convertie porte un nouveau tag (`PICO8_TYPE_TAG_STRUCT`) et un pointeur vers sa table de méthodes, dans le même premier emplacement qu'aujourd'hui. […]
>
> **Héritage entre formes et classes-valeurs** : une classe convertie garde sa `pico8_class_t` enregistrée (registres `fz` et nommé compris) jusqu'à la dernière étape de 1a, qui les supprime toutes avec `pico8_class.c`. Dans le commit de conversion, sa `pico8_class_t` reçoit un pointeur vers le constructeur de struct, et `pico8_class_instantiate` délègue à ce constructeur quand il est posé : `gx[]`, `ie.ke`, `lookup_named` et `kv[hp.cw]` construisent ainsi la forme struct sans être touchés avant la conversion de leur propre classe. […]
>
> **Accès directs aux défauts** : tout site hors `pico8_class.c` qui lit ou écrit `<classe>->defaults` […] migre dans le commit de cette classe : […] la `pico8_class_t` retenue n'en garde qu'une copie en lecture pour les descendants non convertis.

§6 : l'étape ne nomme aucun test de régression ; aucun ne lit l'enveloppe ; ils passent tous avec un diff vide. §7 : le commit passe `make class-coverage-net` avec une sortie identique à `tools/class_coverage_net_expected.txt` (`ie  133  15/15` inchangé). L'enveloppe n'est pas une classe du registre : aucun nom dans `KNOWN[]`, rien enregistré au crochet de couverture. Un site découvert en cours d'étape et couvert par une règle existante du §5 est ajouté à la liste des étapes dans le commit, avec « découvert à 1a.15 » dans le message.

### Ce que 1a.1 à 1a.14 ont posé, réutilisé tel quel

- `PICO8_TYPE_TAG_STRUCT`, `pico8_struct_header_t`, descripteur `pico8_struct_class_t` (méthodes indexées par l'enum, lecteur, écrivain). `pico8_entity_get_field`/`pico8_entity_set_field` sont à double forme ; l'écrivain asserte si le descripteur n'en a pas ; `pico8_instance_get_field` asserte s'il reçoit une struct.
- **Modèle des instances sans classe** : `je` (1a.7, ec01e86), `mo` (1a.9, 9aa4812), enveloppe `mz.jj` (1a.11, 3f62fe5). En-tête seul, sans `pico8_ke_t`, descripteur `static const` sans méthode, écrivain NULL, aucun nom de couverture, accesseur public du descripteur, caractérisation figée sur l'ancien code puis vérification de forme ajoutée après. `jj` a introduit le cas « porte des champs » : le lecteur ne connaît que ses membres et rend 0 pour tout autre nom.
- **1a.14 (4622efb)** : `ie` est une struct `pico8_ie_t` fille de `ke`. Son membre `ke` tient une REF vers la **`pico8_class_t`** d'une classe de `gx[]` (tag `UNKNOWN`, vérifié par l'assert de `ie_self_ke_class`). `ie:ldown` instancie cette classe par `pico8_class_instantiate(ke_cls, {id="s"..id})`. `ie_ke_wrapper`, `ie_ke_field` et `g_ie_empty_fields` sont restés à l'identique, réservés à cette étape.

### Ce que fait le cart

`pico8/lua.lua` :

```
:926-931  function ie:hb()if not self.kh and oj(self.ke.rr)then
            self.ra,self.kh=ie.gy,true  ie.gy-=v(10,0) end end
:932-935  function ie:ldown(mp) ie.id+=1 return self.ke({id="s"..ie.id}) end
:936-940  function ie:kj(p)if self.kh then
            palt(3,false)spr(self.ke.s,p.x,p.y)if self.m_ then
            dl(self.ke)self.jb:ol(0,0)end palt(3,true)end end
:560-567  function dl(self)if self.me=="kz" then … if self.et then … t=self.et.qs …
            elseif self.od then t=self.od.."$" end io(t or "",…) end end
:1181-1185 local gx={qh,os,nr,fr,nl,mx,hj}
            for i,b in pairs(gx)do ie({ke=b,jb=jb(dr[i])})end
:1464     function oj(px) return not px or gu(fm.invented,px) end
```

**Ce qu'est vraiment l'enveloppe.** En Lua, `self.ke` **est** la table de classe (`qh`, `os`, …) ; `self.ke.rr` lit la classe, puis sa `__index` remonte vers les parentes. Il n'y a aucune enveloppe dans le cart. Dans le port, une `pico8_class_t` n'a pas de lecteur de chaîne propre : le seul lecteur de chaîne est `pico8_instance_get_field`, qui part d'une instance. L'enveloppe est donc un artefact du portage : une instance vide dont `cls` est la classe tenue, pour que la lecture tombe tout de suite sur les défauts de la classe puis de ses parentes. `fields` est vide et partagé (`g_ie_empty_fields`), parce que personne n'écrit jamais dessus.

**Valeurs lues, mesurées dans les specs du cart** (aucune n'est réaffectée sur la classe : les seules écritures de `s`/`od`/`me` du cart visent des instances, dont `nr:hb` `self.s=…` `:1114` et `self.od=` `:623`) :

| `gx[i]` | classe | `rr` | `s` | `od` | `me` (hérité de `qh` `:908`) |
|---|---|---|---|---|---|
| 1 | `qh` | absent | 174 | 15 | `"kz"` |
| 2 | `os` | absent | 100 | 15 | `"kz"` |
| 3 | `nr` | `"slipgates"` | 170 | 10 | `"kz"` |
| 4 | `fr` | `"infraspace"` | 78 | 25 | `"kz"` |
| 5 | `nl` | `"trade league"` | 68 | 20 | `"kz"` |
| 6 | `mx` | `"void synthesis"` | 66 | 40 | `"kz"` |
| 7 | `hj` | `"ascension"` | 166 | 60 | `"kz"` |

Aucune classe de ces chaînes (`ke`, `jo`, `qh` et les sept) n'a de défaut `et`.

### Emprise réelle de l'enveloppe (mesurée sur 4622efb)

| Aspect | Mesure |
|---|---|
| Construction | `ie_ke_wrapper` (`research_icon.c:76-82`) : par valeur, sur la pile, tag `INSTANCE`, `fields` = `&g_ie_empty_fields`, `cls` = la classe tenue par `self->ke`. **Aucune enveloppe persistante** ; une par lecture. Jamais dans `hv`/`cv`/`cg`, jamais atteinte par `dg`, `he`, `dc`, `gw`, `jz`, `ui:bh`. |
| Combien | 7 `ie` (un par `gx[i]`, `eu`, `cart.c`), donc 7 classes-valeurs distinctes ; l'enveloppe est reconstruite à chaque `hb` (par `hy:lj`) et à chaque `kj` d'une icône révélée, deux fois quand `m_` est vrai. |
| Libération | aucune (pile). `g_ie_empty_fields` est statique. |
| Lecteurs | `ie_ke_field(self,"rr")` dans `pico8_ie_hb` (`:117`) ; `ie_ke_field(self,"s")` dans `pico8_ie_kj` (`:203`) ; `pico8_dl(gfx,mem,&wrapper)` dans `pico8_ie_kj` (`:216-217`), qui lit `me` (`probe.c:207`), `et` (`probe.c:214`) et `od` (`probe.c:230`) par **`pico8_entity_get_field`, déjà à double forme**. |
| Écrivains qui l'atteignent | aucun : `dl` ne fait que lire. |
| Méthodes appelées dessus | aucune (`dl` est appelé directement, pas par `mv`). |
| Qui garde la classe-valeur | le **membre `ke` de `ie`**, pas l'enveloppe : `ie:ldown` (`:163-177`) et `pico8_ie_kj` (`:213`) le lisent par `ie_self_ke_class`. |
| `->defaults` | aucun dans le module (1a.14 les a retirés). |
| Tests qui la voient | aucun directement. `test_pico8_ie` la traverse : `DRAW_HASH` (sprite `s`, étiquette `od$` de `dl` sur la première icône révélée), `RECORDED_HASH`/`HUB_HASH`/`LONG_HASH`/`PLAYTHROUGH_HASH` (révélations par `rr`), `DIRECTED_HASH`. Les assertions `->cls == get_ref(…,"ke")` (`:481`, `:558`, `:564`) portent sur l'instance **construite par `ldown`**, pas sur l'enveloppe. |
| Commentaires qui la nomment | `research_icon.h:17-19`, `research_icon.c:31-33`, `:62-94`, `:209-211`, `class.h:40-43` (liste des modules qui construisent une instance sans classe à la main). |
| Couverture | aucun nom. `ie  133  15/15` inchangé attendu. |
| Clé dynamique | aucune. |

## Solution

1. **Nouvelle lecture de chaîne sur une classe-valeur, dans la couche** (`pico8_class`) : une fonction publique qui, pour une `pico8_class_t` et un nom, parcourt `defaults` de la classe puis de chaque `parent` et rend 1 + la valeur, ou 0 sans écrire. `pico8_instance_get_field` s'en sert pour la partie « chaîne » de sa recherche (après ses propres `fields`), sans changement de comportement. C'est la lecture Lua `classe.nom` que le port n'avait pas ; elle disparaît avec `pico8_class.c` en 1a.42.
2. **Struct de l'enveloppe** (publique dans le module de l'icône de recherche, à côté de `pico8_ie_t`) : un `pico8_struct_header_t`, puis quatre membres `pico8_value_t`, noms minifiés conservés : `rr`, `s`, `me`, `od`. Pas de `pico8_ke_t` (sans parent), pas de pointeur de classe : ce n'est plus la classe-valeur qu'elle transporte, ce sont les quatre champs de sa chaîne que quelqu'un lit. `et` n'est pas un membre : aucune classe des sept chaînes ne l'a.
3. **Construction** : une fonction du module qui, pour un `ie`, rend l'enveloppe **par valeur** : tag `STRUCT`, descripteur de l'enveloppe, et chaque membre lu sur la classe-valeur tenue par `self->ke` par la lecture de chaîne du point 1 (absent → `pico8_value_ref(NULL)`). Même durée de vie qu'aujourd'hui : sur la pile, une par lecture. Quand `self->ke` est absent, tous les membres restent nil (même effet qu'aujourd'hui : `hb` voit `rr` absent, `kj` dessine le sprite 0, `dl` n'est pas appelé).
4. **Descripteur `static const`** : aucune méthode ; un lecteur qui rend `rr`, `s`, `me`, `od` quand ils ne sont pas nil, et 0 pour tout autre nom (dont `et`) ; écrivain **NULL** (aucun écrivain ne l'atteint : une écriture ferait partir l'assert de `pico8_entity_set_field`). Un accesseur public du descripteur, comme `pico8_mo_placeholder_struct_class()`. L'accesseur de l'enveloppe est aussi public, pour la vérification de forme.
5. **Lecteurs** :
   - `pico8_ie_hb` lit le membre `rr` de l'enveloppe au lieu de `ie_ke_field(self,"rr")`. `pico8_oj` reçoit la même valeur (nil pour `qh`/`os`).
   - `pico8_ie_kj` construit l'enveloppe **une fois**, lit le membre `s` pour le sprite, et passe son adresse à `pico8_dl` quand `m_` est vrai et que `self->ke` est présent. `pico8_dl` n'est pas modifié : il lit `me`/`et`/`od` par `pico8_entity_get_field`, déjà à double forme ; l'enveloppe lui est passée avec le même transtypage que `pico8_dl_dispatch`.
6. **Supprimés** : `ie_ke_wrapper`, `ie_ke_field`, `g_ie_empty_fields`. `ie_self_ke_class` **reste** : `ie:ldown` l'utilise, et la construction de l'enveloppe aussi.
7. **Inchangés** : la struct `pico8_ie_t` et son membre `ke` (classe-valeur), `pico8_ie_ldown` (toujours `pico8_class_instantiate(ke_cls, …)`, qui délèguera au constructeur de struct de chaque classe de `gx[]` à sa conversion, 1a.18-1a.25), le constructeur d'`ie`, son lecteur, son écrivain, `probe.c`, `cart.c`, les classes de `gx[]` et leurs specs.
8. **Plan** : ajouter à la ligne 1a.15 de `docs/plan-3.md` « Découvert à 1a.15 : `dl(self.ke)` lit `me`/`et`/`od` sur l'enveloppe (`probe.c:207,214,230`, déjà à double forme) ; `me`/`od` deviennent membres, `et` absent de toutes les chaînes de `gx[]` ; lecture de chaîne sur une classe-valeur ajoutée à `pico8_class` ». Aucun défaut muté ne s'ajoute au §5.

## User Stories

1. En tant qu'autopilote de la phase 1a, je veux que l'enveloppe `ie.ke` soit la seule chose convertie par ce commit, afin de respecter « une classe par commit » (§7).
2. En tant qu'autopilote, je veux que le PRD dise ce qu'est l'enveloppe (un artefact du portage pour lire la chaîne d'une classe-valeur), afin de ne pas la convertir en une chose que le cart n'a pas.
3. En tant qu'autopilote, je veux l'emprise mesurée (construction, nombre, lecteurs, écrivains, tests, commentaires), afin que chaque site touché soit vérifiable.
4. En tant que lecteur du code, je veux que l'enveloppe soit une struct à en-tête avec les membres `rr`, `s`, `me`, `od`, noms minifiés conservés, afin que « chaque champ lu de sa chaîne est un membre » tienne.
5. En tant que lecteur du code, je veux qu'elle n'ait ni `pico8_ke_t` ni pointeur de classe, afin que « sans parent » (§5) se lise dans le type.
6. En tant que lecteur du code, je veux que `et` ne soit pas un membre, afin qu'aucun champ sans défaut ni écriture n'entre dans une struct.
7. En tant que mainteneur, je veux une lecture de chaîne sur une classe-valeur dans la couche, afin qu'aucun `->defaults` n'apparaisse dans le module et qu'aucune instance sans classe ne soit fabriquée pour lire une classe.
8. En tant que mainteneur, je veux que `pico8_instance_get_field` réutilise cette lecture, afin qu'il n'y ait qu'un parcours de chaîne dans la couche.
9. En tant que mainteneur, je veux que le membre `ke` de `ie` garde la classe-valeur, afin que `ie:ldown` instancie toujours la bonne classe par `pico8_class_instantiate`.
10. En tant qu'auteur des étapes 1a.18 à 1a.25, je veux que `ie:ldown` passe toujours par `pico8_class_instantiate`, afin que la délégation au constructeur de struct construise la forme struct sans toucher `ie`.
11. En tant qu'auteur des étapes 1a.18 à 1a.25, je veux que l'enveloppe lise `rr`/`s`/`me`/`od` par la lecture de chaîne de la classe-valeur, afin que la copie en lecture des défauts que garde chaque `pico8_class_t` convertie (§5) suffise à la faire marcher.
12. En tant qu'auteur des étapes 1a.18 à 1a.25, je veux un test figé qui lit ces quatre valeurs pour chacune des sept classes, afin qu'une conversion qui retirerait une valeur de la copie en lecture échoue tout de suite.
13. En tant que gardien du filet, je veux que `ie:hb` révèle une icône exactement quand `oj(rr)` est vrai, afin que les icônes de `qh`/`os` apparaissent d'emblée et les autres à l'invention de leur recherche.
14. En tant que gardien du filet, je veux que `ie:kj` dessine le sprite `s` de la classe tenue, afin que chaque icône ait son image.
15. En tant que gardien du filet, je veux que `dl(self.ke)` dessine l'étiquette `od$` au survol, afin que le coût de construction s'affiche comme avant.
16. En tant que gardien du filet, je veux que `dl` voie `me == "kz"` sur l'enveloppe, afin qu'il passe son test d'état comme sur la classe Lua.
17. En tant que gardien du filet, je veux que `dl` voie `et` absent sur l'enveloppe, afin qu'il ne dessine ni sprite 31 ni `qs`.
18. En tant que mainteneur, je veux que l'enveloppe n'ait pas d'écrivain, afin qu'une écriture jamais mesurée arrête l'exécution au lieu de disparaître dans une copie de pile.
19. En tant que mainteneur, je veux que l'enveloppe reste construite sur la pile à chaque lecture, afin que sa durée de vie soit la même qu'aujourd'hui et qu'aucune allocation ne s'ajoute.
20. En tant que mainteneur, je veux que `kj` ne construise l'enveloppe qu'une fois par appel, afin que `s` et `dl` lisent la même copie.
21. En tant que mainteneur, je veux qu'un `ie` sans `ke` donne une enveloppe aux membres nil, afin que le comportement actuel (sprite 0, pas de `dl`, `rr` absent) reste identique.
22. En tant que gardien de la couverture, je veux qu'aucun nom ne s'ajoute à `KNOWN[]` et que l'enveloppe n'enregistre rien, afin que `make class-coverage-net` rende une sortie identique.
23. En tant que mainteneur, je veux un accesseur public du descripteur et de l'enveloppe, afin que les tests vérifient la forme sans connaître le module.
24. En tant que vérificateur, je veux une caractérisation figée sur l'ancien code, par icône, de la révélation par `hb` et du dessin par `kj` (avec et sans survol), afin de prouver ce que `DRAW_HASH` ne voit que pour une seule icône.
25. En tant que gardien des tests, je veux que toutes les valeurs figées de `test_pico8_ie` restent identiques, afin que la caractérisation de 1a.14 garde son objet.
26. En tant que gardien des tests de régression, je veux qu'ils passent tous avec un diff vide, afin que la conversion ne se valide pas en affaiblissant un test.
27. En tant que gardien du filet, je veux qu'aucun `.sha256`, aucune entrée de `FRAMES_EVENTS`/`RECORDINGS` ni aucune ligne des références de couverture ne bouge, afin que l'étape ne se valide pas en déplaçant sa cible.
28. En tant que lecteur du plan, je veux que la ligne 1a.15 nomme les sites découverts (`dl` lit `me`/`et`/`od`, lecture de chaîne ajoutée), afin que la liste des étapes dise ce que l'étape a touché.
29. En tant que lecteur du code, je veux que les commentaires rendus faux soient corrigés et que les ajouts ne portent qu'un contrat C ou un piège (CLAUDE.md), afin de ne laisser aucun contrat mensonger.
30. En tant qu'auteur de 1a.42, je veux que le PRD note que la classe-valeur tenue par `ie.ke` et la lecture de chaîne disparaîtront avec `pico8_class_t`, afin que ce remplacement soit prévu à cette étape-là.

## Implementation Decisions

Aucun outil d'agent n'était disponible dans cette session pour lancer un agent `guideline` frais. J'ai chargé le skill `guideline` et je l'ai appliqué moi-même : hypothèses explicites, simplicité, changement chirurgical, critères vérifiables. Les décisions tranchées ainsi sont marquées (G).

- **Ce que l'enveloppe est, et ce qu'elle devient.** Le port avait besoin d'une instance pour lire la chaîne d'une classe. La struct de l'étape porte les **valeurs** que les lecteurs lisent, pas la classe. La classe-valeur reste dans le membre `ke` de `ie`, seul endroit où le cart la garde (`ie({ke=b,…})`), et `ie:ldown` continue de l'appeler.
- **(G) Membres copiés depuis la chaîne, plutôt qu'une struct qui garde `cls` et relit la chaîne à la demande.** La règle du §5 veut des membres. Une struct qui garderait un `pico8_class_t *` resterait une instance sans classe déguisée, avec un lecteur générique de chaîne. La copie est exacte : les quatre défauts ne sont jamais réaffectés sur une classe (mesuré), et la copie est refaite à chaque lecture comme l'enveloppe actuelle. Rejeté aussi : rendre toute la chaîne par le lecteur. Seuls `rr`, `s`, `me`, `et`, `od` sont lus (mesuré), et le modèle `jj` rend 0 pour le reste.
- **(G) Lecture de chaîne dans `pico8_class`, plutôt qu'un parcours `->defaults` dans le module ou une `pico8_instance_t` cachée dans la construction.** Le parcours dans le module rajouterait un site `->defaults` que le §5 fait disparaître. L'instance cachée garderait l'instance sans classe que l'étape supprime. La fonction ajoutée vit dans le fichier que 1a.42 supprime. `pico8_instance_get_field` la réutilise, et c'est son seul changement.
- **(G) Enveloppe sur la pile, rendue par valeur.** C'est la durée de vie actuelle. Rejetés : un objet par `ie` (ajouterait à `pico8_ie_t` un membre qui n'est pas un champ du cart) ; un tableau statique de sept vues indexé par classe (état et cycle init/shutdown en plus, pour rien).
- **`pico8_dl` n'est pas modifié.** Il lit déjà par `pico8_entity_get_field`. Changer sa signature (`pico8_instance_t *` → pointeur générique) toucherait `slipway.c` et les autres appelants de `bv`/`dl`/`d_` : hors étape. L'enveloppe lui est passée avec le transtypage de `pico8_dl_dispatch`.
- **Écrivain NULL** (modèle `mo`/`jj`) : l'assert de `pico8_entity_set_field` est la garde.
- **Règle du §5 qui couvre l'étape** : « Instances sans classe » pour la forme. « Défauts mutés » ne s'applique pas : rien n'est réaffecté ni modifié en place.
- **Tenue à travers 1a.18-1a.25** : la conversion d'une classe de `gx[]` pose son constructeur de struct et garde sa `pico8_class_t` avec une copie en lecture de ses défauts (§5, « Héritage entre formes et classes-valeurs » et « Accès directs aux défauts »). L'enveloppe ne lit que cette copie, par la lecture de chaîne. `ie:ldown` passe par `pico8_class_instantiate`, qui délègue. Rien à faire dans ces étapes pour l'enveloppe, à condition que la copie garde `rr`, `s`, `me`, `od`. Le scénario figé de cette étape le vérifie à chaque commit suivant.
- **Commentaires** (CLAUDE.md) à corriger parce que le changement les rend faux :
  - `research_icon.h:17-19` (« through the transient wrapper … »).
  - `research_icon.c:31-33` (`g_ie_empty_fields`, supprimé), `:62-94` (contrats de l'enveloppe instance), `:209-211` (le commentaire sur `dl(self.ke)` reste vrai sur le fond, `me="kz"` hérité de `qh` ; il est ajusté seulement s'il nomme l'ancienne enveloppe).
  - `class.h:40-43` : `ie.ke` sort de la liste des instances sans classe construites à la main.
  À ajouter : le contrat de la construction de l'enveloppe (par valeur, membres copiés de la chaîne de la classe tenue, nil sans `ke`) et celui de la lecture de chaîne (rend 0 sans écrire si le nom est absent de toute la chaîne ; la classe est empruntée). Tout le reste va dans le message de commit.

## Implementation Chunks

**Un seul commit** (§7). Chaque morceau est un point d'arrêt TDD qui laisse `make test` et `make frames-check` verts. `make class-coverage-net` est lancé au moins sur le code final.

1. **Caractérisation sur l'ancien code.** Nouveau scénario dans `test_pico8_ie` (voir Testing), valeurs figées **avant** toute modification de `research_icon.c`/`.h` et `class.c`/`.h`.
2. **Lecture de chaîne sur une classe-valeur** dans `pico8_class`, réutilisée par `pico8_instance_get_field`. Test unitaire de la fonction. Rien d'autre ne change.
3. **Enveloppe convertie.** Struct, descripteur, accesseurs, construction, `hb`/`kj` passés aux membres, suppression de `ie_ke_wrapper`/`ie_ke_field`/`g_ie_empty_fields`, contrats, vérification de forme, ligne 1a.15 du plan. La caractérisation passe **sans changer ses valeurs figées**.

## Testing Decisions

- **Un bon test observe le comportement** : révélation (`kh`, `ra`, `gy`) après `hb`, pixels du framebuffer après `kj`, valeurs rendues par `pico8_entity_get_field`, compte de couverture. Jamais l'agencement mémoire au-delà du tag et du descripteur.
- **Nouveau scénario `test_ke_view` dans `test_pico8_ie`** (morceau 1), dans un fils forké (`in_fresh_child`), figé sur l'ancien code, lu seulement par l'API publique et à double forme :
  1. Après `eu`, pour chacune des 7 icônes dans l'ordre de `gx[]` : état de `kh` après un `hb` avec `fm.invented` vide (attendu : révélées `qh`, `os` seulement), puis après avoir ajouté le `rr` de sa classe à `fm.invented` (l'implémenteur trouve la façon, par exemple un helper de test existant ; sinon il le note comme omis dans le commit, pas une valeur à inventer).
  2. Pour chaque icône révélée : empreinte de `draw_alone` sans survol, puis avec `m_` vrai (sprite `s` + étiquette `od$` de `dl`), cumulées dans une constante `KE_VIEW_HASH`.
  Les empreintes par icône élargissent `DRAW_HASH`, qui ne voit qu'une icône.
- **Forme** (ajoutée au morceau 3, dans `test_ie_is_a_struct` ou à côté) : pour chaque icône, l'enveloppe est taguée `STRUCT` avec le descripteur de l'enveloppe ; `pico8_entity_get_field` y rend les valeurs du tableau du Problem Statement (`rr` absent pour `qh`/`os`, `s`, `od`, `me` = `"kz"`) et 0 pour `et`, `ke`, `nu`, `hq` ; dans un fils, `pico8_entity_set_field(&vue, "od", …)` fait `SIGABRT`. Le membre `ke` de chaque icône est toujours la classe `gx[i]` (déjà figé par `test_construction`).
- **Lecture de chaîne** (morceau 2) : sur des classes déclarées dans le test (racine, fille qui surcharge, fille qui hérite), nom trouvé sur la classe, sur une parente, absent partout (0, sortie intacte). `test_pico8_entity_smoke` et tous les tests qui passent par `pico8_instance_get_field` restent verts sans changement.
- **Constantes figées existantes** de `test_pico8_ie` (`CONSTRUCTION_HASH`, `START_HASH`, `RECORDED_HASH`, `HUB_HASH`, `LONG_HASH`, `PLAYTHROUGH_HASH`, `DIRECTED_HASH`, `DRAW_HASH`, `RESTART_HASH`, comptes de frames) : identiques.
- **Mutations** : l'implémenteur remet localement au moins 6 erreurs plausibles et note dans le commit celles qui sont attrapées :
  - `rr` lu sur `ke`/`jo` au lieu de la classe tenue (ou toujours nil) ;
  - `s` de la mauvaise classe (par exemple toujours celle de `qh`) ;
  - `od` absent (étiquette vide au survol) ;
  - `me` absent (`dl` ne dessine rien) ;
  - lecteur qui rend aussi un `et` ;
  - lecture de chaîne qui s'arrête à la classe sans remonter `parent` (`me` perdu) ;
  - `ldown` qui instancie depuis l'enveloppe au lieu du membre `ke`.
  Un mutant équivalent est justifié dans le commit.
- **Tests de régression** : tous passent avec un diff vide.
- **`test_pico8_mz`, `test_pico8_mo`, `test_pico8_je`, `test_pico8_jc`, `test_pico8_bg`** et les autres caractérisations de 1a : diff vide, valeurs figées identiques.
- **Couverture** : `make class-coverage-net` rend 0, sortie identique à la référence (`ie  133  15/15`, et les sept classes de `gx[]` inchangées).
- **Prior art** : `test_pico8_mz` (`test_jj_wrapper`, forme ajoutée après, écriture qui abandonne dans un fils), `test_pico8_mo`, `test_pico8_je`, `test_pico8_ie` (`draw_alone`, `first_icon_with_kh`, `in_fresh_child`).

## Critères du vérificateur

Chacun se traduit en `FAILED:` par le vérificateur de `prd-autopilot` (§7 ; seul arrêt que `plan-pilot` honore).

- `FAILED:` si `make test` ou `make frames-check` est encore rouge après **3 tentatives** de correction sur ce commit.
- `FAILED:` si le diff modifie ou supprime un `*.sha256` du filet, ou retire une entrée de `FRAMES_EVENTS` ou de `RECORDINGS`.
- `FAILED:` si `make class-coverage-net`, relancé sur le code final, ne rend pas 0 ou si sa sortie n'est pas identique à `tools/class_coverage_net_expected.txt`.
- `FAILED:` si `tools/class_coverage_net_expected.txt`, `tools/class_coverage_exempt.txt` ou `KNOWN[]` est modifié, ou si l'enveloppe enregistre un nom au crochet de couverture.
- `FAILED:` si un champ de l'enveloppe ne tombe sous aucune règle du §5 : un membre autre que `rr`, `s`, `me`, `od` sans lecture mesurée, ou un membre `et`, ou un pointeur de classe gardé dans l'enveloppe.
- `FAILED:` si `ie:ldown` n'instancie plus la classe-valeur du membre `ke` par `pico8_class_instantiate`, ou si le membre `ke` d'un `ie` ne tient plus la `pico8_class_t` de `gx[i]`.
- `FAILED:` si un test de régression de bug (§6) a un diff non vide, ou devrait changer d'intention pour passer.
- `FAILED:` si une autre classe ou instance sans classe change de forme. En particulier : `ie` (hors de ses lecteurs `hb`/`kj`), une classe de `gx[]` (`qh`, `os`, `nr`, `fr`, `nl`, `mx`, `hj`), `jo`, `ke`, `mz.jj`, `jc`, le `{}` de `bv`, un provisoire, les enveloppes de `pz()`. Aussi si `probe.c`, `cart.c`, `planet*.c` ou une spec de `pico8_class_declare` est modifié.
- `FAILED:` si une valeur figée de `test_pico8_ie` (constantes `*_HASH`, comptes, valeurs de `test_construction`), du nouveau scénario `test_ke_view`, ou d'une autre caractérisation de 1a (`test_pico8_mz`, `test_pico8_mo`, `test_pico8_je`, `test_pico8_iv`, `test_pico8_qb`, `test_pico8_nf`, `test_pico8_fq`, `test_pico8_mh`, `test_pico8_jc`, `test_pico8_bg`) a été changée après la conversion.

Critères de réussite (en plus) :

- grep : plus aucun `ie_ke_wrapper`, `ie_ke_field`, `g_ie_empty_fields`, `PICO8_TYPE_TAG_INSTANCE` ni `pico8_instance_get_field` dans `research_icon.c` ; aucun `->defaults` ajouté hors de `pico8_class.c`.
- La struct de l'enveloppe n'a que `pico8_struct_header_t`, `rr`, `s`, `me`, `od`. Son descripteur n'a aucune méthode et son écrivain est NULL.
- L'enum de méthodes est inchangé.
- Hors de `research_icon.c`/`.h`, le diff de `src/` se limite à la lecture de chaîne de `pico8_class.c`/`.h` et à la correction du commentaire de `class.h`.
- Le diff de `tests/` se limite à `test_pico8_ie.c` (nouveau scénario, forme) et au test de la lecture de chaîne.
- La ligne 1a.15 de `docs/plan-3.md` nomme les sites découverts, et le message de commit dit « découvert à 1a.15 ».

## Out of Scope

- La struct `ie` (1a.14), hors de ses lecteurs `hb`/`kj` de l'enveloppe.
- Les classes de `gx[]` (1a.18-1a.25) et `jo` (1a.17) ; leurs specs et leurs `->defaults`.
- La signature de `pico8_dl`/`bv`/`d_` et leurs autres appelants.
- Le remplacement de la classe-valeur tenue par `ie.ke` et de la lecture de chaîne quand `pico8_class_t` disparaît : 1a.42 (voir Further Notes).
- Le typage C des membres (`const char *` pour `rr`/`me`, fixe pour `s`/`od`) : phase 1b.
- L'audio, et lancer l'appli raylib (jamais).

## Further Notes

- Sans le morceau 3 correctement câblé, un `kj` au survol passerait une struct à `dl` : c'est sans risque, `dl` est déjà à double forme. Le vrai risque est sémantique : une copie qui oublie la remontée vers `qh` perd `me`, et `dl` n'affiche plus rien. `KE_VIEW_HASH` et `DRAW_HASH` le voient.
- **1a.42** : quand toutes les `pico8_class_t` disparaissent, `ie.ke` ne pourra plus tenir une classe-valeur ni l'enveloppe la lire par la chaîne. Il faudra un objet de classe converti qui porte à la fois le constructeur (pour `ie:ldown`) et les défauts `rr`/`s`/`me`/`od` (pour l'enveloppe). Ce n'est pas ajouté au plan ici : c'est dans le périmètre déclaré de 1a.42 (grep de sortie `pico8_class_t` vide). L'implémenteur peut le mentionner dans le message de commit.
- Mesures faites par grep sur `src/` et `tests/` à 4622efb, en lisant `pico8/lua.lua:384-400`, `:552-574`, `:905-940`, `:998-1125`, `:1172-1186`, `:1464-1466`, ainsi que `pico8_research_icon.c`/`.h`, `pico8_probe.c` (`pico8_dl`), `pico8_class.c`/`.h` (`pico8_instance_get_field`), `pico8_struct.h`, `tests/test_pico8_ie.c`, et les PRD de 1a.11 et 1a.14.
