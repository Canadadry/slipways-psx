# raylib : vendoring et build

raylib n'est pas disponible en paquet système dans l'environnement de ce projet.
Il est donc cloné et construit depuis les sources, une fois, hors de `make test`.

## Mise en place

```sh
make raylib-vendor-setup      # clone + build, lent et réseau, une seule fois
make raylib                   # compile et LINK le binaire interactif
```

`raylib-vendor-setup` clone un tag de release figé (jamais une branche mouvante)
dans `third_party/raylib/` (gitignoré) et construit `libraylib.a` avec le build
system de raylib lui-même (`third_party/raylib/src/Makefile`, `PLATFORM_DESKTOP`).

`make raylib` ne lance jamais le binaire qu'il produit — voir `CLAUDE.md`. Il
exige que `third_party/raylib/src/libraylib.a` existe déjà : sans vendoring, il
échoue avec un « no rule to make target » ordinaire, comme `psx-hello` sans son
toolchain.

## Selon l'OS

Le script de vendoring et les flags de link du Makefile testent `uname -s`.

**Linux** — `apt-get` installe les en-têtes X11/GL, et le link ajoute
`-lGL -lX11 -lpthread -ldl -lrt`.

`RAYLIB_VENDOR_SKIP_APT=1 make raylib-vendor-setup` saute l'étape `sudo apt-get`,
pour qui a déjà les dépendances ou ne peut pas donner le sudo.

**macOS** — aucun paquet à installer, les Xcode Command Line Tools suffisent. Le
link utilise `-framework OpenGL -framework Cocoa -framework IOKit
-framework CoreAudio -framework CoreVideo`, l'ensemble que le `src/Makefile` de
raylib documente pour linker un programme externe PLATFORM_DESKTOP sur OSX.

## Fichiers concernés

`src/main_raylib.c`, `src/pico8_raylib_backend.c`,
`src/pico8_raylib_backend_coords.c`. Ce dernier est volontairement sans
dépendance à raylib et est testé par `tests/test_pico8_raylib_backend_coords.c`,
qui fait partie de `make test`.
