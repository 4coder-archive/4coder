CPP_FILES := $(wildcard *.cpp) $(wildcard **/*.cpp)
H_FILES := $(wildcard *.h) $(wildcard **/*.h)
WARNINGS := -Wno-write-strings 
PLAT_LINKS := -L/usr/local/lib -lX11 -lpthread -lm -lrt -lGL -ldl -lXfixes
FLAGS := -fPIC -fno-threadsafe-statics -pthread -I../foreign -g -O0

all: ../4ed_app.so ../4ed

../4ed_app.so: $(CPP_FILES) $(H_FILES)
	g++ $(WARNINGS) $(FLAGS) -std=gnu++0x -shared 4ed_app_target.cpp -iquoteforeign -o $@

../4ed: $(CPP_FILES) $(H_FILES)
	g++ $(WARNINGS) $(FLAGS) -std=gnu++0x linux_4ed.cpp -iquoteforeign $(PLAT_LINKS) -o $@

clean:
	$(RM) -f ../4ed_app.so ../4ed

release: FLAGS += -U_FORTIFY_SOURCE -fno-stack-protector -Wl,--wrap=memcpy linux_release_compat.c -Wl,-s
release: ../4ed_app.so ../4ed
	strip -R .comment $^

release32: FLAGS += -U_FORTIFY_SOURCE -fno-stack-protector -Wl,-s -m32
release32: ../4ed_app.so ../4ed
	strip -R .comment $^

release_super: FLAGS += -D FRED_SUPER 
release_super: release

release32_super: FLAGS += -D FRED_SUPER 
release32_super: release32

.PHONY: clean release release32 release_super release32_super


