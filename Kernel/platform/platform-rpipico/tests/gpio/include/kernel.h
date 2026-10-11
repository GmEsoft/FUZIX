#ifndef TEST_KERNEL_H
#define TEST_KERNEL_H
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
typedef uint16_t uarg_t;
struct u_data { int u_error; };
extern struct u_data udata;
int uget(const void *, void *, unsigned);
int uput(const void *, void *, unsigned);
#endif
