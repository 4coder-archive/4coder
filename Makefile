CPP_FILES  := $(wildcard *.cpp) $(wildcard **/*.cpp)
C_FILES    := $(wildcard *.c) $(wildcard **/*.c)
H_FILES    := $(wildcard *.h) $(wildcard **/*.h)

WARNINGS   := -Wno-write-strings
FLAGS      := -D_GNU_SOURCE -fPIC

debug: FLAGS += -DDEV_BUILD
debug: ../build/build

../build/build: $(CPP_FILES) $(C_FILES) $(H_FILES)
	gcc $(WARNINGS) $(FLAGS) build.c -g -o $@
	../build/build

clean:
	$(RM) ../build/build ../build/fsmgen ../build/metagen ../build/4ed_app.so ../build/4ed



