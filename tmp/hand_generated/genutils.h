#ifndef _GENUTILS_H_
#define _GENUTILS_H_

#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/



#define str_is_in(value, array)  _str_is_in(value, array, sizeof(array)/sizeof(array[0]))


// false : value not found in array
// true : value found...
bool _str_is_in(const char *value, const char* array[], size_t len);

bool check_pattern(const char* pattern, const char* string);


#ifndef TEST_UNIT
struct afb_api_t;
extern afb_api_t the_api;
#endif

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/


#endif /*_GENUTILS_H_*/
