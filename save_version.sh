# Copies current binary over to the versions folder with the provided name
# Usage: bash save_version.sh <name>

name=$1

if [ -z "$name" ]; then
    echo "Please specify a name to assign this version"
    exit 1
fi

mkdir -p ./versions
cp ./build/enigma ./versions/"$name"

echo "Saved version '$name'"