CPP_FILES  := $(wildcard *.cpp) $(wildcard **/*.cpp)
C_FILES    := $(wildcard *.c) $(wildcard **/*.c)
H_FILES    := $(wildcard *.h) $(wildcard **/*.h)

WARNINGS   := -Wno-write-strings
FLAGS      := -D_GNU_SOURCE -fPIC -fpermissive

debug: FLAGS += -DDEV_BUILD
debug: ../build/build

package: FLAGS += -DPACKAGE
package: ../build/build

site: FLAGS += -DSITE_BUILD
site: ../build/build

../build/build: $(CPP_FILES) $(C_FILES) $(H_FILES)
	g++ $(WARNINGS) $(FLAGS) build.cpp -g -o $@
	../build/build

clean:
	$(RM) ../build/build ../build/fsmgen ../build/metagen ../build/4ed_app.so ../build/4ed
