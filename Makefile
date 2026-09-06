LOVE = /Applications/love.app/Contents/MacOS/love
LOVE_FILE = slipways.love

.PHONY: build run test font gba-devkit-setup gba-hello gba-hello-clean install mcp-mgba-test


build:
	cd game && zip -r ../$(LOVE_FILE) .

run:
	$(LOVE) game
test:
	cd game && lua ../tests/test_all.lua

font:
	python3 tools/ttf_to_pixelfont.py "game/assets/kenney_kenney-fonts/Fonts/Kenney Mini.ttf" 8 " ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789" -o game/assets/font.png

gba-devkit-setup:
	sh tools/install_gba_devkit.sh

gba-hello:
	DEVKITPRO=/opt/devkitpro DEVKITARM=/opt/devkitpro/devkitARM $(MAKE) -C gba-hello

gba-hello-clean:
	DEVKITPRO=/opt/devkitpro DEVKITARM=/opt/devkitpro/devkitARM $(MAKE) -C gba-hello clean

install:
	sh tools/install_mcp_mgba.sh

mcp-mgba-test:
	sudo apt-get install -y binutils-arm-none-eabi
	cd tools/mcp-mgba && bash native/build.sh --test
	cd tools/mcp-mgba && npm test
