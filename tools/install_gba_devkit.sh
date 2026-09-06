set -e

if [ ! -f /usr/share/keyring/devkitpro-pub.gpg ]; then
	sudo mkdir -p /usr/share/keyring
	sudo curl -sS -A "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0 Safari/537.36" -o /usr/share/keyring/devkitpro-pub.gpg https://apt.devkitpro.org/devkitpro-pub.gpg
fi

if [ ! -f /etc/apt/sources.list.d/devkitpro.list ]; then
	echo "deb [signed-by=/usr/share/keyring/devkitpro-pub.gpg] https://apt.devkitpro.org stable main" | sudo tee /etc/apt/sources.list.d/devkitpro.list > /dev/null
fi

sudo apt-get update
sudo apt-get install -y apt-transport-https devkitpro-pacman

sudo dkp-pacman -S gba-dev --needed --noconfirm

echo "devkitARM + libtonc installed under /opt/devkitpro"
echo "DEVKITPRO=/opt/devkitpro DEVKITARM=/opt/devkitpro/devkitARM"
