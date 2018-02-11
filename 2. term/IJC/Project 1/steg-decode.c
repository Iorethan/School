// steg-decode.c
// Reseni IJC-DU1, priklad b), 20.3.2015
// Autor: Ondrej Vales, FIT
// Prelozeno: gcc 4.8.4

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "error.h"
#include "bit-array.h"
#include "eratosthenes.h"
#include "ppm.h"

#define N 75000000

int main(int argc, char **argv)
{
    /* overeni poctu argumentu a vypocet bitove pole s prvocisly */
	if (argc != 2)
		FatalError("Chybne argumenty");
	BA_create(pole,N);
	Eratosthenes(pole);

	struct ppm *img = ppm_read(argv[1]);

    /* limit velikosti obrazku je 5000 * 5000 pixel, celkem tedy 25 mil pixel */
    	if(img->xsize * img->ysize * 3 > N)
	{
	    FatalError("Maximalni velikost obrazku je %ld pixelu", N / 3);
	}

	char c = 0;
	unsigned count = 0;
	unsigned long int dim = img->xsize * img->ysize * 3;
	/* pruchod pres ppm data */
	for (unsigned long int index = 2; index <= dim && count < CHAR_BIT; index++)
	{
        /* vyber znaku na prvociselne pozici */
		if(!BA_get_bit(pole, index))
		{
			c = c | ((img->data[index]&(char)1)<<count);
			count++;
		}
        /* tisk znaku po naplneni */
		if(count == CHAR_BIT)
		{
			if(!isprint(c) && c != 0)
			{
				free(img);
				FatalError("Chybny format");
			}
			putchar(c);
			if(c != 0)
			{
				c = 0;
				count = 0;
			}
		}
	}
	/* zprava je zakoncena nulou */
	if(c != 0)
	{
		free(img);
		FatalError("Chybny format");
	}
	printf("\n");

	free(img);

	return 0;
}
