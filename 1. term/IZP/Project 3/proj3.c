/*
 * Projekt 3 IZP, FIT 2014
 * PRUCHOD BLUDISTEM
 * jmeno: Ondrej Vales
 * login: xvales03
 * kontakt: xvales03@stud.fit.vutbr.cz
 * datum vytvoreni: 14. 12. 2014
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/**< Kody chybovych stavu. */
enum errors{
    OK,                 //vporadku
    ERR_ARG,            //chybne argumenty
    ERR_FILEPATH,       //neplatna cesta k souboru
    ERR_FILE,           //neplatny obsah souboru
    ERR_START,          //neplatna pocatecni pozice
    ERR_ALLOC,          //chyba pri alokaci pameti
    VALID = -1,         //pomocne stavy pro testovani mapy
    INVALID = -2};

/**< Kody operaci. */
enum opearions{
    HELP,
    TEST,
    LPATH,
    RPATH,
    SPATH,
    FAIL};

/**< Kody smeru v bludisti. */
enum directions{
    LEFT,
    DOWN,
    RIGHT,
    UP};

/**< Kody hranic policek. */
enum borders{
    L_BOR,
    R_BOR,
    X_BOR};     //horni nebo dolni

typedef struct {
    int rows;
    int cols;
    unsigned char *cells;
} Map;

typedef struct {
    unsigned int max_distance;
    int max_count;
    unsigned int *distance;
} Dist;

int process_params(int argc, char *argv[]);
bool chceck_params(char *argv[]);
int test_map(Map *map, char *filepath);
bool isborder(Map *map, int r, int c, int border);
int start_border(Map *map, int r, int c, int leftright);
int find_path(Map *map, char *argv[], int leftright);
int open_file(char *path, FILE **f);
int get_dimensions(Map *map, FILE *f);
int alloc_map(Map *map);
int get_cells(Map *map, FILE *f);
void free_map(unsigned char *cells);
bool iswhite(char c);
bool isnumber(char c);
int print_test(int state);
void print_err(int errnum);
void print_help();

int find_short_path(Map *map, char *argv[]);
void find_exits(Map *map, Dist *dist);
void eval_adjacent(Map *map, Dist *dist);
void print_path(Map *map, Dist *dist, int *r, int *c);

/**
 * Hlavni program.
 */
int main(int argc, char *argv[])
{
    int args;
    int state = OK;
    Map map = {0, 0, NULL};

    args = process_params(argc, argv);          //zpracovani prikazu z argumentu programu

    switch (args)
    {
    case HELP:
        print_help();
        break;
    case TEST:
        state = test_map(&map, argv[2]);        //testovani pouze vypise valid/invalid
        if (state < OK)
            state = print_test(state);
        break;
    case LPATH:
        state = test_map(&map, argv[4]);        //pokud test probehl neuspesne, program se ukonci
        if (state == VALID)
            state = find_path(&map, argv, LEFT);
        break;
    case RPATH:
        state = test_map(&map, argv[4]);        //pokud test probehl neuspesne, program se ukonci
        if (state == VALID)
            state = find_path(&map, argv, RIGHT);
        break;
    case SPATH:
        state = test_map(&map, argv[4]);        //pokud test probehl neuspesne, program se ukonci
        if (state == VALID)
            state = find_short_path(&map, argv);
        break;
    default:                                    //pokud nebyla vybrana zadna operace program se ukonci s chybou argumentu
        state = ERR_ARG;
    }

    if (map.cells != NULL)                      //dealokace pameti pro ulozeni mapy
        free_map(map.cells);

    if (state != OK)                            //pokud doslo k chybe vypsat chybovy stav
        print_err(state);

    return state;
}

/**
 * Pruchod bludistem podel steny. Leftright nabyva hodnot 0 a 2, protoze pruchod podel prave steny
 * je stejny jako podel leve akorat zrcadlove prevraceny.
 * @param map Struktura obsahujici udaje o bludisti.
 * @param argv Pole retezcu zadanych jako argumenty programu.
 * @param leftright Rozliseni pruchodu podel leve a prave steny.
 * @return Vraci OK pokud nedoslo k chybe nebo prislusny chybovy kod.
 */
int find_path(Map *map, char *argv[], int leftright)
{
    int r = atoi(argv[2]);
    int c = atoi(argv[3]);
    int dir;

    dir = start_border(map, r, c, leftright);       //urceni steny, ktera se ma nasledovat
    if (dir == INVALID)
        return ERR_START;

    while (r > 0 && r <= map->rows && c > 0 && c <= map->cols)          //cestu hledam dokud se nedostanu mimo bludiste
    {
        if (dir == LEFT && !isborder(map, r - 1, c - 1, L_BOR))            //leva stena
        {
            printf("%d,%d\n", r ,c);
            c--;
        } else if (dir == DOWN && !isborder(map, r - 1, c - 1, X_BOR) && (c + r) % 2 == 1)            //dolni stena
        {
            printf("%d,%d\n", r ,c);
            r++;
        } else if (dir == RIGHT && !isborder(map, r - 1, c - 1, R_BOR))            //prava stena
        {
            printf("%d,%d\n", r ,c);
            c++;
        } else if (dir == UP && !isborder(map, r - 1, c - 1, X_BOR) && (c + r) % 2 == 0)            //horni stena
        {
            printf("%d,%d\n", r ,c);
            r--;
        } else
            dir += 2;                           //pozadovana stena nebyla pruchozi, musim zmenit stenu co nasleduju
        dir = (dir + 1 + leftright) % 4;        //zmena orientace po vyhodnoceni pruchodnosti steny
    }
    return OK;
}

/**
 * Zpracovani argumentu z prikazove radky.
 * @param argc Pocet argumentu.
 * @param argv Pole retezcu zadanych jako argumenty programu.
 * @return Vraci kod zpracovane argumentu nebo FAIL pokud argumenty neodpovidaly zadnemu zpusobu spusteni.
 */
int process_params(int argc, char *argv[])
{
    if (argc == 2 && strcmp(argv[1], "--help") == 0)
        return HELP;
    if (argc == 3 && strcmp(argv[1], "--test") == 0)
        return TEST;
    if (argc == 5 && strcmp(argv[1], "--lpath") == 0 && chceck_params(argv))        //overeni cisel pocatecni pozice
        return LPATH;
    if (argc == 5 && strcmp(argv[1], "--rpath") == 0 && chceck_params(argv))        //overeni cisel pocatecni pozice
        return RPATH;
    if (argc == 5 && strcmp(argv[1], "--shortest") == 0 && chceck_params(argv))     //overeni cisel pocatecni pozice
        return SPATH;
    return FAIL;
}

/**
 * Overeni 2 a 3 agrumentu. Tyto argumenty musi byt cisla.
 * @param argv Pole retezcu zadanych jako argumenty programu.
 * @return Vraci true pokud jsou to cisla jinak false.
 */
bool chceck_params(char *argv[])
{
    for (unsigned int i = 2; i < 4; i++)
        for (unsigned int j = 0; j < strlen(argv[i]); j++)
        {
            if (!isnumber(argv[i][j]))
                return false;
        }
    return true;
}

/**
 * Overeni 2 a 3 agrumentu. Tyto argumenty musi byt cisla.
 * @param map Struktura obsahujici udaje o bludisti.
 * @param filepath Nazev souboru s bludistem.
 * @return Vraci chybovy stav pokud se nepodarilo zpracovat soubor s bludistem nebo VALID/INVALID v zavislosti na obasahu souboru.
 */
int test_map(Map *map, char *filepath)
{
    int state;
    FILE *f;

    state = open_file(filepath, &f);            //otevreni souboru

    if (state == OK)
        state = get_dimensions(map, f);         //nacteni velikosti mapy

    if (state == OK)
        state = alloc_map(map);                 //alokace pameti pro mapu

    if (state == OK)
        state = get_cells(map, f);              //nacteni zbytku znaku

    if (f != NULL)
        fclose(f);

    if (state == OK)
    {
        for (int r = 0; r < map->rows ; r++)        //pruchod tabulkou
            for (int c = 0; c < map->cols; c++)
            {
                if (c < map->cols - 1)
                    if (isborder(map, r, c, R_BOR) != isborder(map, r, c + 1, L_BOR))       //overeni hranice s bunkou vpravo
                        return INVALID;
                if (r < map->rows - 1 && (r + c) % 2 == 1)
                    if (isborder(map, r, c, X_BOR) != isborder(map, r + 1, c, X_BOR))       //s bunkou dole
                        return INVALID;
            }
        return VALID;
    }
    return state;
}

/**
 * Overeni hranice bunky.
 * @param map Struktura obsahujici udaje o bludisti.
 * @param r Radek ve ketrem je bunka.
 * @param c Sloupec ve kterem je bunka.
 * @param border Ozanaceni hranice bunky.
 * @return Vraci true pokud je prislusna hranice nepruchozi, false pokud je pruchozi.
 */
bool isborder(Map *map, int r, int c, int border)
{
    switch (border)
    {
    case L_BOR:
        if ((map->cells[r * map->cols + c] & 1) == 1)
            return true;
        break;
    case R_BOR:
        if ((map->cells[r * map->cols + c] & 2) == 2)
            return true;
        break;
    case X_BOR:
        if ((map->cells[r * map->cols + c] & 4) == 4)
            return true;
    }
    return false;
}

/**
 * Urceni hranice, ktera se ma nasledovat po vstupu do bludiste.
 * @param map Struktura obsahujici udaje o bludisti.
 * @param r Radek ve kterem se ma vstoupit do bludiste.
 * @param c Sloupec ve kterem se ma vstoupit do bludiste.
 * @param leftright Urcuje podel jake steny se ma bludiste prochazet.
 * @return Vraci true pokud je prislusna hranice nepruchozi, false pokud je pruchozi.
 */
int start_border(Map *map, int r, int c, int leftright)
{
    if (r == 1 && c <= map->cols && c > 0 && (c + r) % 2 == 0 && !isborder(map, r - 1, c - 1, X_BOR))       //vstup shora
    {
        if (leftright == 0)
            return RIGHT;
        else
            return LEFT;
    }

    if (r == map->rows && c <= map->cols && c > 0 && (c + r) % 2 == 1 && !isborder(map, r - 1, c - 1, X_BOR))   //vstup zdola
    {
        if (leftright == 0)
            return LEFT;
        else
            return RIGHT;
    }

    if (c == 1 && r <= map->rows && r > 0 && !isborder(map, r - 1, c - 1, L_BOR))       //vstup zleva
    {
        if (leftright == 0)
        {
            if (r % 2 == 1)
                return UP;
        }
        else if (r % 2 == 0)
            return DOWN;
        return RIGHT;
    }

    if (c == map->cols && r <= map->rows && r > 0 && !isborder(map, r - 1, c - 1, R_BOR))       //vstup zprava
    {
        if (leftright == 0)
        {
            if ((r + c) % 2 == 1)
                return DOWN;
        }
        else if ((r + c) % 2 == 0)
            return UP;
        return LEFT;
    }

    return INVALID;     //zadana bunka neni na hranici nebo na ni neni mozny vstup z venku
}

/**
 * Otevreni souboru s bludistem.
 * @param path Nazev souboru.
 * @param f Adresa otevreneho souboru.
 * @return Vraci OK nebo ERR_FILEPATH pokud nebylo mozne soubor otevrit.
 */
int open_file(char *path, FILE **f)
{
    *f = fopen(path, "r");
    if (*f == NULL)             //otevreni se nepodarilo
        return ERR_FILEPATH;
    return OK;
}

/**
 * Nacteni rozmeru bludiste.
 * @param map Struktura obsahujici udaje o bludisti.
 * @param f Adresa otevreneho souboru.
 * @return Vraci OK nebo ERR_FILE pokud na zacatku souboru nejsou dve cisla.
 */
int get_dimensions(Map *map, FILE *f)
{
    int c = fgetc(f);
    int dim[] = {0, 0};

    for (int i = 0; i < 2; i++) //nacteni dvou cisel
    {
        while (iswhite(c))      //preskoceni bilych znaku
        c = fgetc(f);

        while (!iswhite(c))     //prevedeni retezce na cislo
        {
            if (!isnumber(c))
                return ERR_FILE;
            dim[i] = dim[i] * 10;
            dim[i] += c - '0';
            c = fgetc(f);
            if (feof(f))
                return ERR_FILE;
        }
    }

    map->rows = dim[0];
    map->cols = dim[1];

    return OK;
}

/**
 * Alokace pameti pro mapu.
 * @param map Struktura obsahujici udaje o bludisti.
 * @return Vraci OK nebo ERR_ALLOC pokud se nezdarila alokace.
 */
int alloc_map(Map *map)
{
    map->cells = malloc(map->rows * map->cols * sizeof(unsigned char));
    if (map->cells == NULL)     //nepodarilo se alokovat
        return ERR_ALLOC;
    return OK;
}

/**
 * Nacteni zbytku souboru s bludistem.
 * @param map Struktura obsahujici udaje o bludisti.
 * @param f Adresa otevreneho souboru.
 * @return Vraci OK nebo ERR_FILE pokud je obsah souboru neocekavany.
 */
int get_cells(Map *map, FILE *f)
{
    int c = fgetc(f);
    for (int i = 0; i < map->cols * map->rows ; i++)    //potrebuju zaplnit celou mapu, pro kazdou polozku jedno cislo
    {
        while (iswhite(c))          //preskoceni bilych znaku
            c = fgetc(f);
        if (c < '0' || c > '7')     //polozky musi mit hodnotu 0 az 7
            return ERR_FILE;
        map->cells[i] = c - '0';
        c = fgetc(f);
        if (!iswhite(c) && !feof(f))    //kdyz soubor skonci predcasne chyba
            return ERR_FILE;
    }
    return OK;
}

/**
 * Uvolneni mapy.
 * @param map Struktura obsahujici udaje o bludisti.
 */
void free_map(unsigned char *cells)
{
    free(cells);
}

/**
 * Rozhodnuti, zda zank je bily nebo ne.
 * @param c Overovany znak.
 * @return Vraci true pokud znak je bily, jinak false.
 */
bool iswhite(char c)
{
    if (c == ' ' || c == '\t' || c == '\n' || c == '\r')
        return true;
    return false;
}

/**
 * Rozhodnuti, zda zank je cislice.
 * @param c Overovany znak.
 * @return Vraci true pokud znak je cislice, jinak false.
 */
bool isnumber(char c)
{
    if (c < '0' || c > '9')
        return false;
    return true;
}

/**
 * Vypis vysledku overeni.
 * @param state Vysledek overeni.
 * @return Vraci OK.
 */
int print_test(int state)
{
    if (state == VALID)
        printf("Valid\n");
    if (state == INVALID)
        printf("Invalid\n");
    return OK;
}

/**
 * Vytiskne chybove hlaseni na chybovy vystup.
 */
void print_err(int errnum)
{
    if (errnum == INVALID)
        errnum = ERR_FILE;
    const char *err_string[] = {
        "neplatne argumenty",
        "neplatna cesta k souboru",
        "neplatny obsah souboru",
        "neplatna pocatecni pozice",
        "nepodarila se alokace pameti",};
    fprintf(stderr, "Chyba: %s!\n", err_string[errnum - 1]);
}

/**
 * Vytiskne na standardni vystup text s napovedou.
 */
void print_help()
{
    printf("Bludiste\n"
           "IZP - projekt 3\n\n"

           "Program hleda cestu skrz bludiste s trojuhelnikovymi policky, ktere prochazi\n"
           "podel sten. Mapu bludiste ocekava v souboru. Jmeno je zadano jako argument.\n\n"

           "Syntax spusteni programu:\n"
           "--test name\t\tOveri, zda soubor name obsahuje platnou mapu bludiste.\n"
           "--lpath r c name\tProjde bludiste v souboru name podel leve steny. Vypise\n"
           "\t\t\tsouradnice kazdeho pouziteho policka na zvlastni radek.\n"
           "\t\t\tPocatecni radek oznacuje r, pocatecni sloupec c.\n"
           "--rpath r c name\tProjde bludiste v souboru name podel prave steny. Vypise\n"
           "\t\t\tsouradnice kazdeho pouziteho policka na zvlastni radek.\n"
           "\t\t\tPocatecni radek oznacuje r, pocatecni sloupec c.\n"
           "--shortest r c name\tNajde nejkratsi cestu z bludiste. Vypise souradnice\n"
           "\t\t\tkazdeho pouziteho policka na zvlastni radek.\n"
           "\t\t\tPocatecni radek oznacuje r, pocatecni sloupec c.\n\n"

           "Autor:\t\tOndrej Vales\n"
           "Datum vyvoreni:\t7. 12 . 2014\n");
}

/**
 * Nalezeni nejkratsi cesty. Program hleda cestu od vychodu k zadane bunce.
 * Vsem bunkam priradi hodnotu odpovidajici poctu policek, ktere je nutno projit,
 * pri vychdu z bludiste z dane bunky.
 * Vsem vychodum priradi hodnotu jedna. Vsem bunkam do kterych se da dostat pres
 * volnou hranici z jiz ohodnocene bunky priradi hodnotu o jedna vetsi, nez jakou
 * mela jiz ohodnocena bunka. Toto se opakuje dokud neni ohodnocena bunka kterou zadal
 * uzivatel nebo pokud nejsou ohodnoce vsechny bunky, do kterych se lze dostat alespon
 * z jednoho vychodu.
 * Pote ze zadane bunky program hleda cestu ven. Vzdy se presune do bunky s o jedna
 * mensim ohodnocenim dokud se nedostane k vychodu.
 * Program vypise cestu nebo hlaseni ze cesta neexistuje.
 * @param map Struktura obsahujici udaje o bludisti.
 * @param argv Pole retezcu zadanych jako argumenty programu.
 * @return Vraci OK.
 */
int find_short_path(Map *map, char *argv[])
{
    int r = atoi(argv[2]);
    int c = atoi(argv[3]);
    Dist dist = {0, 0, NULL};

    if (r < 1 || r > map->rows || c < 1 || c > map->cols)
        return ERR_START;

    dist.distance = malloc(map->rows * map->cols * sizeof(unsigned int));
    if (dist.distance == NULL)     //nepodarilo se alokovat
        return ERR_ALLOC;

    for (int i = 0; i < (map->rows * map->cols); i++)       //vyplneni cele mapy 0
        dist.distance[i] = 0;

    find_exits(map, &dist);         //najiti vsech vychodu z bludiste

    while (dist.distance[(r - 1) * map->cols + c - 1] == 0 && dist.max_count > 0)       //Vsem bunkam priradi hodnotu odpovidajici jejich vzdalenosti od nejblizsiho vychodu
    {
        dist.max_count = 0;
        dist.max_distance++;
        eval_adjacent(map, &dist);
    }

    if (dist.max_count == 0)            //cesta ven nemusi existovat
        printf("Ze zadaneho policka neexistuje cesta ven.\n");
    else for (;dist.max_distance > 0; dist.max_distance--)             //zpetny vychod z bludiste po nalezeni nejkratsi cesty
    {
        print_path(map, &dist, &r, &c);
    }

    free(dist.distance);        //uvolneni pameti

    return OK;
}

/**
 * Nalezeni vsech vychodu z bludiste.
 * @param map Struktura obsahujici udaje o bludisti.
 * @param dist Struktura obsahujici udaje o ohodnocenych bunkach.
 */
void find_exits(Map *map, Dist *dist)
{
    for (int c = 0; c < map->cols; c += 2)
    {
        if (!isborder(map, 0, c, X_BOR))
        {
            dist->distance[c] = 1;
            dist->max_count++;
        }
        if (c + (map->rows % 2) < map->cols && !isborder(map, map->rows - 1, c + (map->rows % 2), X_BOR))
        {
            dist->distance[(map->rows - 1) * map->cols + c] = 1;
            dist->max_count++;
        }
    }

    for (int r = 0; r < map->rows; r++)
    {
        if (!isborder(map, r, 0, L_BOR))
        {
            dist->distance[r * map->cols] = 1;
            dist->max_count++;
        }
        if (!isborder(map, r, map->cols - 1, R_BOR))
        {
            dist->distance[(r + 1) * map->cols - 1] = 1;
            dist->max_count++;
        }
    }
    dist->max_distance++;
}

/**
 * Ohodnoceni bunek sousedicich s jiz ohodnocenymi bunkami.
 * @param map Struktura obsahujici udaje o bludisti.
 * @param dist Struktura obsahujici udaje o ohodnocenych bunkach.
 */
void eval_adjacent(Map *map, Dist *dist)
{
    for (int row = 0; row < map->rows; row++)
        for (int col = 0; col < map->cols; col++)
        {
            int i = row * map->cols + col;
            if (dist->distance[i] == dist->max_distance - 1)
            {
                if (col != 0 && !isborder(map, row, col, L_BOR) && dist->distance[i - 1] == 0)
                {
                    dist->distance[i - 1] = dist->max_distance;
                    dist->max_count++;
                }
                if (i < map->rows * map->cols - 1 && col != map->cols - 1 && !isborder(map, row, col, R_BOR) && dist->distance[i + 1] == 0)
                {
                    dist->distance[i + 1] = dist->max_distance;
                    dist->max_count++;
                }
                if ((row + col) % 2 == 0 && !isborder(map, row, col, X_BOR) && i >= map->cols && dist->distance[i - map->cols] == 0)
                {
                    dist->distance[i - map->cols] = dist->max_distance;
                    dist->max_count++;
                }
                if ((row + col) % 2 == 1 && !isborder(map, row, col, X_BOR) && i < (map->rows - 1) * map->cols && dist->distance[i + map->cols] == 0)
                {
                    dist->distance[i + map->cols] = dist->max_distance;
                    dist->max_count++;
                }
            }
        }
}

/**
 * Vytisteni nejkreatsi cesty.
 * @param map Struktura obsahujici udaje o bludisti.
 * @param dist Struktura obsahujici udaje o ohodnocenych bunkach.
 * @param r Radek na kterem lezi zadana bunka.
 * @param c Sloupec na kterem lezi zadana bunka.
 */
void print_path(Map *map, Dist *dist, int *r, int *c)
{
        printf("%d,%d\n", *r , *c);
        if (*c > 1 && !isborder(map, *r - 1, *c - 1, L_BOR) && dist->distance[(*r - 1) * map->cols + *c - 2] == dist->max_distance - 1)     //bunka vlevo ma o jedna nizsi hodnotu
        {
            (*c)--;
        }
        else if (*c < map->cols && !isborder(map, *r - 1, *c - 1, R_BOR) && dist->distance[(*r - 1) * map->cols + *c] == dist->max_distance - 1)        //bunka vpravo ma o jedna nizsi hodnotu
        {
            (*c)++;
        }
        else if ((*r + *c) % 2 == 0 && *r > 1 && !isborder(map, *r - 1, *c - 1, X_BOR) && dist->distance[(*r - 2) * map->cols + *c -1] == dist->max_distance - 1)       //bunka nahore ma o jedna nizsi hodnotu
        {
            (*r)--;
        }
        else if ((*r + *c) % 2 == 1 && *r > 1 && !isborder(map, *r - 1, *c - 1, X_BOR) && dist->distance[*r * map->cols + *c - 1] == dist->max_distance - 1)        //bunka dole ma o jedna nizsi hodnotu
        {
            (*r)++;
        }
}
