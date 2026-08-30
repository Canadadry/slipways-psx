LOVE = /Applications/love.app/Contents/MacOS/love
LOVE_FILE = slipways.love


build:
	cd game && zip -r ../$(LOVE_FILE) .

run:
	$(LOVE) game
test:
	cd game && lua ../tests/test_all.lua

font:
	python3 tools/ttf_to_pixelfont.py "game/assets/kenney_kenney-fonts/Fonts/Kenney Mini.ttf" 8 " ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789" -o game/assets/font.png
