// Soubor: wordcount.c
// Reseni IJC-DU2, priklad 2
// Datum: 24. 4. 2015
// Autor: Ondrej Vales, xvales03
// Fakulta: FIT VUTBR
// Prelozeno: gcc 4.8

#include <stdio.h>
#include <stdlib.h>

#include "htable.h"
#include "io.h"

#define HTAB_SIZE 7001
#define WORD_MAX_LENGHT 127

// Pro velikost tabulky bylo vybrano 7001, protoze pri praci s tabulkou s
// prvociselnou velikosti by melo dojit k lepsimu rozptyleni hodnot.
// Tato velikost umoznuje rychlou praci s texty s poctem ruznych slov
// v radu desetitisicu a zaroven nema prilis velke naroky na pamet.
// Pro praci s textem s vetsim poctem ruznych slov by bylo potreba tuto
// hodnotu navisit (pri praci s dlouhymi seznamy bude program pomaly).

void print(const char* key, unsigned int value);


int main()
{
    htab_t *tab = htab_init(HTAB_SIZE);
    if (tab == NULL)
	{
		fprintf(stderr, "CHYBA: Nepodarilo se naalokovat pamet!\n");
		return 1;
	}
	
    if (tab->htab_size == 0)	// Program potrebuje tabulku s nejmene jednou polozkou, jinak nema zadny pocatecni odkaz ze ktereho vytvaret seznam
	{
		fprintf(stderr, "CHYBA: Nelze pokracovat s tabulkou o velikosi 0!\n");
		return 1;
	}

    int max = WORD_MAX_LENGHT;
    char str[max];
    hashtab_listitem *item;
    while (fgetw(str, max, stdin) != EOF)	// Nacitam az do konce souboru.
    {
        item = htab_lookup(tab, str);
		if (item == NULL)	// Nepodarilo se najit a vytvorit novou polozku.
		{
			fprintf(stderr, "CHYBA: Nepodarilo se naalokovat pamet!\n");
			return 1;
		}
        item->data++;
    }

	// Vypis vsech slov a prislusnych cetnosti
    htab_foreach(tab, print);

    htab_free(tab);
    return 0;
}


// Funkce pro vypis slova a cetnosti.
void print(const char* key, unsigned int value)
{
    printf("%s\t%u\n", key, value);
}
