set -e

sudo apt-get update
sudo apt-get install -y build-essential cmake git

git submodule update --init --recursive -- tools/mcp-mgba

cd tools/mcp-mgba
bash native/build.sh
npm install
cd node_modules/koffi && node ./cnoke.cjs -P . -D src/koffi --release
cd ../..
npm run build

echo "mcp-mgba built and ready."
echo "Registered in .mcp.json as the 'mgba' server - restart/reload Claude Code to pick it up."
echo "Manual start: node tools/mcp-mgba/dist/index.js"
