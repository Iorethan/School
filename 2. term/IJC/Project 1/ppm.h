// ppm.h
// Reseni IJC-DU1, priklad b), 20.3.2015
// Autor: Ondrej Vales, FIT
// Prelozeno: gcc 4.8.4

#ifndef PPM_H_INCLUDED
#define PPM_H_INCLUDED


struct ppm {
    unsigned xsize;
    unsigned ysize;
    char data[]; // RGB bajty, celkem 3*xsize*ysize
};

struct ppm * ppm_read(const char * filename);
int ppm_write(struct ppm *p, const char * filename);


#endif // PPM_H_INCLUDED
