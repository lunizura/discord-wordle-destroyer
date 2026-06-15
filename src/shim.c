#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#include <sys/stat.h>

int __mingw_sscanf(const char *s, const char *format, ...) {
    va_list args;
    va_start(args, format);
    int ret = vsscanf(s, format, args);
    va_end(args);
    return ret;
}

void sincosf(float x, float *s, float *c) {
    *s = sinf(x);
    *c = cosf(x);
}

void sincos(double x, double *s, double *c) {
    *s = sin(x);
    *c = cos(x);
}

FILE * __cdecl my_iob_func(unsigned i) {
    return (&_iob[i]);
}

void *imp_acrt __asm__("__imp____acrt_iob_func") = (void *)my_iob_func;

int stat64i32(const char *path, struct _stat *buffer) {
    return _stat(path, buffer);
}
