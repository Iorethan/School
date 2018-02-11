// error.h
// Reseni IJC-DU1, priklad b), 20.3.2015
// Autor: Ondrej Vales, FIT
// Prelozeno: gcc 4.8.4

#ifndef ERROR_H_INCLUDED
#define ERROR_H_INCLUDED


void Warning(const char *fmt, ...);
void FatalError(const char *fmt, ...);


#endif // ERROR_H_INCLUDED
