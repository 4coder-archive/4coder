#!/bin/bash
#!/bin/sh
# Linux packaging script by insofaras
#
# Usage:
# 
# * Put all the .txt, .ttf, etc stuff in TEMPLATE_DIR and it'll get copied over
#   (including the 3rd party folder)
#
# * Put the makefile (included at the end of this file) in the dir above 4coder/code
#
# * Change the other dir variables as appropriate
#

TEMPLATE_DIR="$HOME/Desktop/4coder/release_template/"
CODE_DIR="$HOME/Desktop/4coder"
TMP_DIR="/tmp/4coder"
OUT_ZIP="$HOME/Desktop/4coder-linux.zip"
OUT_ZIP_SUPER="$HOME/Desktop/4coder-linux-super.zip"

echo "template: $TEMPLATE_DIR"
echo "base:     $CODE_DIR"
echo "temp:     $TMP_DIR"
echo "out:      $OUT_ZIP"

rm -rf "$OUT_ZIP"
rm -rf "$OUT_ZIP_SUPER"

mkdir -p "$TMP_DIR"
mkdir -p "$TMP_DIR/alpha"
mkdir -p "$TMP_DIR/super"

cat << EOF > "$TMP_DIR/readme.txt"
This is a linux 4coder release.
Brought to you by Mr4thDimention and insofaras.

Enjoy!
EOF

pushd "$CODE_DIR"

echo "Alpha User"

make clean
make release32
cp -r "${TEMPLATE_DIR}" "$TMP_DIR/alpha/32"
cp ./4ed ./4ed_app.so "$TMP_DIR/alpha/32/"

echo " "

make clean
make release
cp -r "${TEMPLATE_DIR}" "$TMP_DIR/alpha/64"
cp ./4ed ./4ed_app.so "$TMP_DIR/alpha/64/"

echo " "

cp "$TMP_DIR/readme.txt" "$TMP_DIR/alpha/readme.txt"
pushd "$TMP_DIR"
zip -r "$OUT_ZIP" "$(basename alpha)"
popd

echo " "



echo "Super User"

make clean
make release32
cp -r "${TEMPLATE_DIR}" "$TMP_DIR/super/32"
cp ./4ed ./4ed_app.so ./code/4coder_*.h ./code/4coder_*.cpp "$TMP_DIR/super/32/"

echo " "

make clean
make release
cp -r "${TEMPLATE_DIR}" "$TMP_DIR/super/64"
cp ./4ed ./4ed_app.so ./code/4coder_*.h ./code/4coder_*.cpp "$TMP_DIR/super/64/"

echo " "

cp "$TMP_DIR/readme.txt" "$TMP_DIR/super/readme.txt"
pushd "$TMP_DIR"
zip -r "$OUT_ZIP_SUPER" "$(basename super)"
popd

echo " "



make clean

rm -rf "$TMP_DIR"

popd

echo "Created linux 4coder package: $OUT_ZIP"

exit

