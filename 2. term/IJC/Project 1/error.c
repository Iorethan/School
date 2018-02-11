// error.c
// Reseni IJC-DU1, priklad b), 20.3.2015
// Autor: Ondrej Vales, FIT
// Prelozeno: gcc 4.8.4

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

void Warning(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    fprintf(stderr, "CHYBA: ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    va_end (ap);
}

void FatalError(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    fprintf(stderr, "CHYBA: ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    va_end (ap);
    exit(1);
}
