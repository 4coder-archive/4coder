/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 14.11.2017
 *
 * Debuging helper for fd leaking
 *
 */

// TOP

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>  

#define MAXPATHLEN 512

int
descriptor_open(int fd){
    int is_open = true;
    if (fd != 0){
        char t[1];
        if (pread(fd, t, 1, 0) == -1){
            if (errno == EBADF){
                is_open = false;
            }
        }
    }
    return(is_open);
}

void
print_open_file_descriptors(void){
    for (int fd = 1; fd < 256; ++fd){
        if (descriptor_open(fd)){
            char b[MAXPATHLEN + 1];
            fcntl(fd, F_GETPATH, b);
            LOGF("FD(%d) = \"%s\"\n", fd, b);
        }
    }
}

#define FD_CHECK() LOG("FD_CHECK\n"); print_open_file_descriptors()

// BOTTOM
