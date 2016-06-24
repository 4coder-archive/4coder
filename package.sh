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
OUT_ZIP="$HOME/Desktop/4coder-linux-64.zip"
OUT_ZIP_32="$HOME/Desktop/4coder-linux-32.zip"
OUT_ZIP_SUPER="$HOME/Desktop/4coder-linux-super-64.zip"
OUT_ZIP_SUPER_32="$HOME/Desktop/4coder-linux-super-32.zip"

echo "template: $TEMPLATE_DIR"
echo "base:     $CODE_DIR"
echo "temp:     $TMP_DIR"
echo "out:      $OUT_ZIP"

rm -rf "$OUT_ZIP"
rm -rf "$OUT_ZIP_SUPER"
rm -rf "$OUT_ZIP_32"
rm -rf "$OUT_ZIP_SUPER_32"

mkdir -p "$TMP_DIR"

pushd "$CODE_DIR"

echo "Alpha User"

# ALPHA-32
make clean
make release32
cp -r "${TEMPLATE_DIR}" "$TMP_DIR/alpha"
cp ./4ed ./4ed_app.so "$TMP_DIR/alpha/"
cp ./code/README.txt ./code/TODO.txt "$TMP_DIR/alpha/"

echo " "

pushd "$TMP_DIR"
zip -r "$OUT_ZIP_32" "$(basename alpha)"
popd
rm -rf "$TMP_DIR/alpha"

echo " "

# ALPHA-64
make clean
make release
cp -r "${TEMPLATE_DIR}" "$TMP_DIR/alpha"
cp ./4ed ./4ed_app.so "$TMP_DIR/alpha/"
cp ./code/README.txt ./code/TODO.txt "$TMP_DIR/alpha/"

echo " "

pushd "$TMP_DIR"
zip -r "$OUT_ZIP" "$(basename alpha)"
popd
rm -rf "$TMP_DIR/alpha"

echo " "



echo "Super User"

# SUPER-32
make clean
make release32_super
cp ./4ed ./4ed_app.so ./code/4coder_*.h ./code/4coder_*.cpp "$TMP_DIR/super/"
cp ./code/buildsuper.sh "$TMP_DIR/super/"
cp ./code/README.txt ./code/SUPERREADME.txt ./code/TODO.txt "$TMP_DIR/super/"
cp ./code/*.html "$TMP_DIR/super/"

echo " "

pushd "$TMP_DIR"
zip -r "$OUT_ZIP_SUPER_32" "$(basename super)"
popd
rm -rf "$TMP_DIR/super"

echo " "

# SUPER-64
make clean
make release_super
cp -r "${TEMPLATE_DIR}" "$TMP_DIR/super/"
cp ./4ed ./4ed_app.so ./code/4coder_*.h ./code/4coder_*.cpp "$TMP_DIR/super/"
cp ./code/buildsuper.sh "$TMP_DIR/super/"
cp ./code/README.txt ./code/SUPERREADME.txt ./code/TODO.txt "$TMP_DIR/super/"
cp ./code/*.html "$TMP_DIR/super/"

echo " "

pushd "$TMP_DIR"
zip -r "$OUT_ZIP_SUPER" "$(basename super)"
popd
rm -rf "$TMP_DIR/super"

echo " "



make clean

rm -rf "$TMP_DIR"

popd

echo "Created linux 4coder packages"

exit

