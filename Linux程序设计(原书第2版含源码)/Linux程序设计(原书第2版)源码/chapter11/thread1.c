#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main() {
    printf("POSIX version is set to %ld\n", _POSIX_VERSION);
    if (_POSIX_VERSION < 199506L) {
        if (_POSIX_C_SOURCE >= 199506L) {
            printf("Sorry, you system does not support POSIX.1c threads\n");
        }
        else {
            printf("Try again with -D_POSIX_C_SOURCE=199506L\n");
        }
    }
    else {
        printf("Your system supports POSIX1.c threads,\n");
        #ifdef _POSIX_THREAD_PRIORITY_SCHEDULING
            printf("including support for priority scheduling\n");
        #else
            printf("but does not support priority scheduling\n");
        #endif
    }
    exit(EXIT_SUCCESS);
}
