CPP_FILES  := $(wildcard *.cpp) $(wildcard **/*.cpp)
H_FILES    := $(wildcard *.h) $(wildcard **/*.h)
WARNINGS   := -Wno-write-strings
PLAT_LINKS := -L/usr/local/lib -lX11 -lpthread -lm -lrt -lGL -ldl -lXfixes -lfreetype -lfontconfig
FLAGS      := -fPIC -fno-threadsafe-statics -pthread -I../foreign $(shell pkg-config --cflags freetype2)

# main stuff

debug: FLAGS += -DFRED_INTERNAL=1 -DFRED_SUPER=1 -g -O0
debug: ../4ed_app.so ../4ed

../4ed_app.so: $(CPP_FILES) $(H_FILES)
	g++ $(WARNINGS) $(FLAGS) -shared 4ed_app_target.cpp -iquoteforeign -o $@

../4ed: $(CPP_FILES) $(H_FILES)
	g++ $(WARNINGS) $(FLAGS) linux_4ed.cpp -iquoteforeign $(PLAT_LINKS) -o $@

clean:
	$(RM) ../4ed_app.so ../4ed

# releases

alpha: FLAGS += -U_FORTIFY_SOURCE -fno-stack-protector -Wl,--wrap=memcpy linux_release_compat.c -Wl,-s
alpha: ../4ed_app.so ../4ed
	strip -R .comment $^

alpha32: FLAGS += -U_FORTIFY_SOURCE -fno-stack-protector -Wl,-s -m32
alpha32: ../4ed_app.so ../4ed
	strip -R .comment $^

super: FLAGS += -DFRED_SUPER=1
super: alpha

super32: FLAGS += -DFRED_SUPER=1
super32: alpha32

# packaging

PACKAGE_FILES := ../4ed ../4ed_app.so README.txt TODO.txt

../4coder_super.zip:   PACKAGE_FILES += 4coder_*.h 4coder_*.cpp buildsuper.sh SUPERREADME.txt
../4coder_super32.zip: PACKAGE_FILES += 4coder_*.h 4coder_*.cpp buildsuper.sh SUPERREADME.txt

../4coder_%.zip: %
	zip -j $@ $(PACKAGE_FILES)
	(cd ../release_template && zip -g -r $@ .)
	$(MAKE) clean

package: clean
	@echo -e "\e[1;32m      ==== Creating Alpha package ==== \e[0m"
	$(MAKE) ../4coder_alpha.zip
	@echo -e "\e[1;32m      ==== Creating Super package ==== \e[0m"
	$(MAKE) ../4coder_super.zip
#	$(MAKE) ../4coder_alpha32.zip
#	$(MAKE) ../4coder_super32.zip

.PHONY: debug alpha alpha32 super super32 package clean


