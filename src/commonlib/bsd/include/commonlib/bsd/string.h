/* SPDX-License-Identifier: BSD-3-Clause */

#ifndef _COMMONLIB_BSD_STRING_H_
#define _COMMONLIB_BSD_STRING_H_

#include <stddef.h>
#include <stdint.h>

size_t strlen(const char *src);
size_t strnlen(const char *str, size_t maxlen);
char *strcat(char *dst, const char *src);
char *strncat(char *dst, const char *src, size_t n);

unsigned int skip_atoi(char **ptr);

#endif  /* _COMMONLIB_BSD_STRING_H_ */
