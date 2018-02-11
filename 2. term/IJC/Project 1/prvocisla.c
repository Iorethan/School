// prvocisla.c
// Reseni IJC-DU1, priklad a), 20.3.2015
// Autor: Ondrej Vales, FIT
// Prelozeno: gcc 4.8.4
// Program nekontroluje jestli do datoveho typu definovaneho v bit-array.h lze ulozit cislo N
// Pokud je cislo N vetsi nez maximalni hodnota ulozitelna do poziteho datoveho typu, program nebude fungovat spravne

#include <stdio.h>
#include <stdlib.h>

#include "error.h"
#include "bit-array.h"
#include "eratosthenes.h"

#define N 201000000

int main()
{
	BA_create(pole,N + 1);
	Eratosthenes(pole);

	int count = 0;
	type index = BA_size(pole);
	/* hledam poslednich 10 prvocisel */
	while ((count < 10) && (index > 2))
	{
		index--;
		if (BA_get_bit(pole,index) == 0)
		{
			count++;
		}
	}
	/* vypis prvocisel ve vzestupnem poradi */
	for(; count > 0; count--)
	{
		while(BA_get_bit(pole,index) != 0)
		index++;
		printf("%ld\n", (long int)index);
		index++;
	}

	return 0;
}
