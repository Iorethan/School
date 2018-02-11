// Soubor: tail.c
// Reseni IJC-DU2, priklad 1
// Datum: 24. 4. 2015
// Autor: Ondrej Vales, xvales03
// Fakulta: FIT VUTBR
// Prelozeno: gcc 4.8

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 510

enum MODS {LAST, FROM};	// Ozanaceni ruznych funkci tail.

typedef struct{
    long int line;
    int mode;
    FILE *f;
} mode_s;

int process_args(int argc, char *argv[], mode_s *args);
void print_last (mode_s args);
void print_from (mode_s args);


int main(int argc, char *argv[])
{
    mode_s args= {10, LAST, stdin};	// Implicitni nastaveni.
    if (argc != process_args(argc, argv, &args))
    {
        fprintf(stderr, "CHYBA: spatne argumenty!\n");
        return 1;
    }

    switch (args.mode)
    {
    case LAST:
        if (args.line != 0)
            print_last(args);
        break;
    case FROM:
        print_from(args);
        break;
    }

    return 0;
}

int process_args(int argc, char *argv[], mode_s *args)
{
    char *ptr = NULL;
    int count = 1;

    if (argc > 2 && strcmp(argv[1], "-n") == 0)		// Zpracovani argumentu ozanacujich pocet radku k tisku.
    {
        if (argv [2][0] == '+')
        {
            args->mode = FROM;
        }
        args->line = abs(strtol(argv[2], &ptr, 10));
        count += 2;		// Pricteni poctu zpracovanych argumentu.
        if (*ptr != '\0')
            count = 0;
    }

    if (argc > count)		// Pripadne otevreni souboru.
    {
        args->f = fopen(argv[count], "r");
        count++;		// Pricteni poctu zpracovanych argumentu.
        if (args->f == NULL)
            count = 0;
    }

    return count;
}

void print_last (mode_s args)
{
    char buffer [args.line][N];
    int state = 0;
    int i, c;
    for (i = 0; i < args.line; i++)
        buffer[i][0] = '\0';

    for (i = 0; fgets(buffer[i], N, args.f) != NULL; i++, i = i % args.line) // Cyklicke prepisovani bufferu poslednimy nactebymi radky, starsi a nepotrebne radky sou nahrazeny novymi.
    {
        if (strlen(buffer[i]) == N - 1 && buffer[i][N - 2] != '\n')	// Overeni zda byl nacten cely radek.
        {
            buffer[i][N - 2] = '\n';
            while ((c = getc(args.f)) != '\n' && c != EOF)		// Nacteni zbytku dloheho radku.
                ;
            if (state == 0)
            {
                fprintf(stderr, "CHYBA: prilis dlouhe radky!\n");
                state = 1;
            }
        }
    }

    for (int j = 0; j < args.line; j++, i++, i = i % args.line)		// Tisk radku z bufferu.
    {
        printf("%s", buffer[i]);
    }
}

void print_from (mode_s args)
{
    char buffer[N];
    int state = 0;
    int c;
    for (int i = 1; i < args.line; i++)		// Preskoceni nepotrebnych radku.
        while ((c = fgetc(args.f)) != '\n' && c != EOF);
    while (fgets(buffer, N, args.f) != NULL)		// Tisk zbytku souboru.
    {
        if (strlen(buffer) == N - 1 && buffer[N - 2] != '\n')		// Overeni zda byl nacten cely radek.
        {
            buffer[N - 2] = '\n';
            while ((c = getc(args.f)) != '\n' && c != EOF)		// Nacteni zbytku dlouheho radku.
                ;
            if (state == 0)
            {
                fprintf(stderr, "CHYBA: prilis dlouhe radky!\n");
                state = 1;
            }
        }
        printf("%s", buffer);
    }
}
