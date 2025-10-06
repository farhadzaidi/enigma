name=$1

if [ -z "$name" ]; then
    echo "Please specify a name to assign this version"
    exit 1
fi

mkdir -p ./versions
cp ./build/enigma ./versions/"$name"

echo "Saved version '$name'"