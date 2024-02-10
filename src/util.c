#include <stdlib.h>
#include <string.h>
#include "util.h"
#include <c-log/log.h>

char* format_name(char *first, char *middle, char *last) {
    // Figure out how many characters are in each string
    size_t char_count = 0;

    if (first) {
        char_count += strlen(first);
    }
    if (middle) {
        char_count += strlen(middle);
    }
    if (last) {
        char_count += strlen(last);
    }

    // Account for spaces between parts
    char_count += (first && (middle || last)) + (middle && last);

    char *buffer = malloc(char_count + 1);
    bzero((void*) buffer, char_count + 1);

    if (first) {
        strcat(buffer, first);
        if (middle || last) {
            strcat(buffer, " ");
        }
    }

    if (middle) {
        strcat(buffer, middle);
        if (last) {
            strcat(buffer, " ");
        }
    }

    if (last) {
        strcat(buffer, last);
    }

    return buffer;
}
