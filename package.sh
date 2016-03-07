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

mkdir -p "$TMP_DIR"

pushd "$CODE_DIR"

make clean
make release32
cp -r "${TEMPLATE_DIR}" "$TMP_DIR/32"
cp ./4ed ./4ed_app.so ./code/4coder_*.h ./code/4coder_*.cpp "$TMP_DIR/32/"

make clean
make release
cp -r "${TEMPLATE_DIR}" "$TMP_DIR/64"
cp ./4ed ./4ed_app.so ./code/4coder_*.h ./code/4coder_*.cpp "$TMP_DIR/64/"

make clean

cat << EOF > "$TMP_DIR/readme.txt"
This is a linux 4coder release.
Brought to you by Mr4thDimention and insofaras.

Enjoy!
EOF

pushd "$TMP_DIR/.."
zip -r "$OUT_ZIP" "$(basename $TMP_DIR)"
popd

rm -rf "$TMP_DIR"

popd

echo "Created linux 4coder package: $OUT_ZIP"

exit

## The makefile, copy it into a file called "makefile" one dir above the code/ dir.

CPP_FILES := $(wildcard *.cpp) $(wildcard **/*.cpp)
H_FILES := $(wildcard *.h) $(wildcard **/*.h)
WARNINGS := -Wno-write-strings 
PLAT_LINKS := -L/usr/local/lib -lX11 -lpthread -lm -lrt -lGL -ldl 
FLAGS := -fPIC -fno-threadsafe-statics -pthread

all: 4ed_app.so 4ed

4ed_app.so: $(CPP_FILES) $(H_FILES)
	g++ $(WARNINGS) $(FLAGS) -std=gnu++0x -shared code/4ed_app_target.cpp -iquoteforeign -o 4ed_app.so

4ed: $(CPP_FILES) $(H_FILES)
	g++ $(WARNINGS) $(FLAGS) -std=gnu++0x code/linux_4ed.cpp -iquoteforeign $(PLAT_LINKS) -o $@

clean:
	$(RM) -f 4ed_app.so 4ed
	
release: FLAGS += -U_FORTIFY_SOURCE -fno-stack-protector -Wl,--wrap=memcpy code/linux_release_compat.c -Wl,-s
release: 4ed_app.so 4ed
	strip -R .comment $^
	
release32: FLAGS += -U_FORTIFY_SOURCE -fno-stack-protector -Wl,-s -m32
release32: 4ed_app.so 4ed
	strip -R .comment $^

.PHONY: clean release release32

