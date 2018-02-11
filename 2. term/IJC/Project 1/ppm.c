// ppm.c
// Reseni IJC-DU1, priklad b), 20.3.2015
// Autor: Ondrej Vales, FIT
// Prelozeno: gcc 4.8.4

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "error.h"
#include "bit-array.h"
#include "ppm.h"

struct ppm * ppm_read(const char * filename)
{
    FILE *f = fopen(filename, "r");
    if (f == NULL)
        FatalError("Soubor %s nenalezen", filename);

    /* nacteni a overeni prvnich 3 znaku P6<ws> */
    if((fgetc(f) != 'P') || (fgetc(f) != '6') || (!isspace(fgetc(f))))
    {
        Warning("Chybny format souboru");
        fclose(f);
        return NULL;
    }

    /* nacteni 3 cisel ze souboru - rozmenry obrazku a cislo 255 */
    unsigned dim[3] = {0, 0, 0};
    int c;
    for (int i = 0; i < 3; i++)
    {
        while(isspace(c = fgetc(f)));
        while (isdigit(c))
        {
            dim[i] = dim[i] * 10 + (c - '0');
            c = fgetc(f);
        }
        if (!isspace(c))
        {
            Warning("Chybny format souboru");
            fclose(f);
            return NULL;
        }
    }

    /* treti nactene cislo musi byt 255 */
    if(dim[2] != 255)
    {
        Warning("Chybny format souboru");
        fclose(f);
        return NULL;
    }

    /* alokace struktury do ktere budu ukladat nactena data */
    struct ppm *new_ppm = malloc(sizeof(struct ppm) + sizeof(char) * 3 * dim[0] * dim[1]);
    if (new_ppm == NULL)
    {
        fclose(f);
        FatalError("Alokace pameti selhala");
    }
    /* do struktury ulozim rozmery obrazku */
    new_ppm->xsize = dim[0];
    new_ppm->ysize = dim[1];
    dim[2] = dim[0] * dim[1] * 3;

    /* nactani RGB dat */
    for (unsigned i = 0; i < dim[2]; i++)
    {
        new_ppm->data[i] = c = fgetc(f);
        if (c == EOF)
        {
            Warning("Chybny format souboru");
            fclose(f);
            free(new_ppm);
            return NULL;
        }
    }

    /* po posledni RGB bajtu nasleduje EOF*/
    if(fgetc(f) != EOF)
    {
        Warning("Chybny format souboru");
        fclose(f);
        free(new_ppm);
        return NULL;
    }
    fclose(f);

    return new_ppm;
}

int ppm_write(struct ppm *p, const char * filename)
{

    FILE *f = fopen(filename, "w");
    if(f == NULL)
    {
        Warning("Nepodarilo se otevrit soubor");
        return -1;
    }

    /* zapis hlavicky */
    fputs("P6\n", f);
    fprintf(f, "%u %u\n", p->xsize, p->ysize);
    fputs("255\n", f);

    /* zapis dat */
    unsigned size = p->xsize * p->ysize * 3;
    for (unsigned i = 0; i < size; i++)
    {
        fputc(p->data[i], f);
    }
    fclose(f);
    return 0;
}
