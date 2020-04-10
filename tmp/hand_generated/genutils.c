// genutils.c
#include <string.h>
#include <regex.h>
#include "genutils.h"

#include <assert.h>
#include <stdio.h>

#include <afb/afb-binding.h>

afb_api_t api;

bool _str_is_in(const char *value, const char* array[], size_t len) {
    for(size_t i=0; i<len; i++) {
        if (!strncmp(array[i], value, strlen(array[i]))) {
            return 1; // true
        }
    }
    return 0; // false
}

bool check_pattern(const char* pattern, const char* string) {
    regex_t regex;
    int reti;
    char msgbuf[100];

    /* Compile regular expression */
    reti = regcomp(&regex, pattern, /*0*/REG_EXTENDED);
    assert(reti == 0);

    /* Execute regular expression */
    reti = regexec(&regex, string, 0, NULL, 0);
    regfree(&regex);

    //assert((reti == 0) || (reti == REG_NOMATCH));
#ifndef NDEBUG
    if ( ! ((reti == 0) || (reti == REG_NOMATCH))) {
        regerror(reti, &regex, msgbuf, sizeof(msgbuf));
        /*fprintf(stderr,*/
        AFB_API_NOTICE(api, "Regex match failed: %s\nThis should never happend, please fix", msgbuf);
        abort();
    }
#endif
    return (reti==0);
}

/*
 * test code
 *

int main(void) {
    char *pattern = "a{3,5}b+c*";

    char *strings[] = {
        "abc",
        "aaaaaaaaaabcc",
        "aaaabcc",
        "aaaac",
        "aaaabbc",
        "aaabbbb",
        "aaaa"
    };

    int nb_string = sizeof(strings)/sizeof(strings[0]);
    printf("pattern = %s nb_string == %d\n", pattern, nb_string);
    for (int i=0; i<nb_string; i++) {
        printf("%s %s\n", strings[i], check_pattern(pattern, strings[i])?"matches.":"doesn't match.");
    }

    return 0;
}
*/
