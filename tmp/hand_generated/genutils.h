#ifndef _GENUTILS_H_
#define _GENUTILS_H_

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/



#define str_is_in(value, array)  _str_is_in(value, array, sizeof(array)/sizeof(array[0]))


// 0 : value not found i array
// 1 : value found
int _str_is_in(const char *value, const char* array[], size_t len);


#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/


#endif /*_GENUTILS_H_*/
