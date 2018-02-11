// Soubor: io.c
// Reseni IJC-DU2, priklad 2
// Datum: 24. 4. 2015
// Autor: Ondrej Vales, xvales03
// Fakulta: FIT VUTBR
// Prelozeno: gcc 4.8

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

int fgetw(char *s, int max, FILE *f)
{
	static int state_of_strlen_fgetw = 0;
    max--;	// Musim si nechta misto na koncovou nulu.
    int c, i;
    while (isspace(c = fgetc(f))); // Preskoceni mezer pred slovem.

    for (i = 0; i < max && !isspace(c) && c != EOF; i++) // Cteni znaku dokud nevycerpam buffer nebo nejsem na konci slova.
    {
        s[i] = c;
        c = fgetc(f);
    }
    s[i] = '\0';
    if (i == max && !isspace(c) && state_of_strlen_fgetw == 0)	// Overeni zda se slovo precetli cele
	{
        fprintf(stderr, "CHYBA: prilis dlouhe slovo!\n");
        state_of_strlen_fgetw = 1;
	}
	
    while (!isspace(c) && c != EOF)		// Precteni zbytku dlouheho slova
        c = fgetc(f);

    if (c == EOF)		// Pokud byl text ukoncen primo znakem EOF (bez noveho radku) funkce precte slovo a EOF vrati na stdin (a precte jej az pri pristim volani).
    {
        if (i == 0)
            return EOF;
        else
        {
            ungetc(c, f);
            return i;
        }
    }
    return i;
}
