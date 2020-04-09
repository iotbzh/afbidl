// genutils.c
#include <string.h>
#include "genutils.h"

int _str_is_in(const char *value, const char* array[], size_t len) {
    for(size_t i=0; i<len; i++) {
        if (!strncmp(array[i], value, strlen(array[i]))) {
            return 1; // true
        }
    }
    return 0; // false
}

