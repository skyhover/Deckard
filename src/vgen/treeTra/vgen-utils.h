/* This file contains extensions for C strings, etc. */

#ifndef _VGEN_UTILS_H_
#define _VGEN_UTILS_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/****** String Utilities ******/
char *stringclone(const char *var);
char *stringnclone(const char *var, unsigned int len);
int compare_string(const void *c1, const void *c2); /* a wrapper of strcmp. */

#ifdef __cplusplus
}
#endif

#endif
