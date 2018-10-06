#include <string.h>

// Link to the memcpy symbol with version 2.2.5 instead of the newer 2.14 one
// since they are functionaly equivalent, but using the old one allows 4coder
// to run on older distros without glibc >= 2.14

extern "C" {
    asm (".symver memcpy, memcpy@GLIBC_2.2.5");
    void *__wrap_memcpy(void *dest, const void *src, size_t n){
        return memcpy(dest, src, n);
    }
}
