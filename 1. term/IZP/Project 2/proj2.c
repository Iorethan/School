/*
 * Projekt 2 IZP, FIT 2014
 * ITERACNI VYPOCTY
 * jmeno: Ondrej Vales
 * login: xvales03
 * kontakt: xvales03@stud.fit.vutbr.cz
 * datum vytvoreni: 26. 11. 2014
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define TAYLOR_LIMIT 13
#define FRAC_ACCURATE 8
#define MAX_HEIGHT 100.0

int calc_both(double height, double a, double b);
int process_params(int argc, char *argv[], double *height, int *arg_shift);
int check_params(int argc, char *argv[], int arg_shift);
int set_height(double *height, char *number);
int tan_compare(double x, unsigned int n, unsigned int m);
double taylor_tan(double x, unsigned int n);
double cfrac_tan(double x, unsigned int n);
double my_abs(double x);
void print_err(int errnum);
void print_help();

/**< Kody chybovych stavu. */
enum errors{
    OK,                     //vporadku
    ERR_ARG,                //chyba argumentu nebo nastaveni vysky
    ERR_RANGE,              //porovnani mimo vymezeny interval
    ERR_ALPHA,              //uhel alfa mimo limit
    ERR_BETA,               //uhel beta mimo limit
    ERR_AUX = -1};          //pomocna chyba

/**< Kody vyberu argumentu. */
enum arguments{
    HELP,
    COMPARE,
    DISTANCE,
    BOTH};                  //kod pro vypocet vzdalenosti i vysky

/**
 * Hlavni program.
 */
int main(int argc, char *argv[])
{
    int state = OK;                 //stav programu
    int args;
    double height = 1.5;            //vyska pristroje implicitnì 1,5m
    int arg_shift = 0;              //posun argumentu pokud bylo zadano -c X

    args = process_params(argc, argv, &height, &arg_shift);

    switch (args)
    {
    case HELP:
        print_help();
        break;
    case COMPARE:
        state = tan_compare(atof(argv[2]), atoi(argv[3]), atoi(argv[4]));
        break;
    case DISTANCE:
        state = calc_both(height, atof(argv[2 + arg_shift]), NAN);          //NAN argument nebyl zadan
        break;
    case BOTH:
        state = calc_both(height, atof(argv[2 + arg_shift]), atof(argv[3 + arg_shift]));
        break;
    default:
        state = ERR_ARG;
    }

    if (state != OK)                    //pri chyboven stavu vypsat chybu
        print_err(state);
    return state;
}

/**
 * Vypocet vzdalenosti a (pokud bylo zadano B) vysky objektu a vypsani vysledku.
 * @param height Vyska ve ktere je umisten pristroj.
 * @param a Uhel alfa v radianech.
 * @param b Uhel beta v radianech.
 * @return Vraci OK pokud nedoslo k chybe, ERR_ALPHA pokud byl uhel alfa mimo povoleny interval nebo ERR_BETA pokud uhel beta byl mimo povoleny interval.
 */
int calc_both(double height, double a, double b)        //vypocet vzdalenosti a vysky objektu
{
    double dist;
    if (a <= 0 || a > 1.4)                              //overeni zda uhly jsou v limitu
        return ERR_ALPHA;
    if ((b <= 0 || b > 1.4) && !isnan(b))
        return ERR_BETA;

    dist = height / cfrac_tan(a, FRAC_ACCURATE);        //vypocet vzdalenosti, FRAC_ACCURATE urcena hornota presnosti
    printf("%.10e\n", dist);

    if (!isnan(b))                                      //vypocet vysky pokud byl zadan druhy argument
        printf("%.10e\n", cfrac_tan(b, FRAC_ACCURATE) * dist + height);

    return OK;
}

/**
 * Zpracovani argumetu.
 * @param argc Pocet argumentu.
 * @param argv Pole retezcu argumentu.
 * @param height Vyska pristroje.
 * @param arg_shift Pocet jiz zpracovanych argumentu (-c X).
 * @return Vraci kod prislusne operace nebo ERR_AUX pokud argumenty neodpovidaji zadne operaci.
 */
int process_params(int argc, char *argv[], double *height, int *arg_shift)
{
    if (argc > 3 && strcmp(argv[1], "-c") == 0)                                     //osetreni volitelneho nastaveni vysky
        *arg_shift = set_height (height, argv[2]);
    if (argc == 2 && strcmp(argv[1], "--help") == 0)
        return HELP;
    if (argc == 5 && check_params(argc, argv, *arg_shift) == COMPARE)               //compare nepouziva vysku, nelze nastavovat + overeni cisel
        return COMPARE;
    if (argc == 3 + *arg_shift && check_params(argc, argv, *arg_shift) == BOTH)     //vypocet bez zadanecho beta, posun o dva pokud se zadavala vyska
        return DISTANCE;
    if (argc == 4 + *arg_shift && check_params(argc, argv, *arg_shift) == BOTH)     //kompletni vypocet
        return BOTH;
    return ERR_AUX;
}

/**
 * Overeni platnosti zadanych argumentu.
 * @param argc Pocet argumentu.
 * @param argv Pole retezcu argumentu.
 * @param arg_shift Pocet jiz zpracovanych argumentu (-c X).
 * @return Vraci COMPARE pokud se ma provadet porovnani, BOTH pokud se ma provadet vypocet a ERR_AUX pokud byly argumenty chybne.
 */
int check_params(int argc, char *argv[], int arg_shift)
{
    char *last_char = NULL;

    if (strcmp(argv[1], "--tan") == 0)                      //pro tan prvni argument double zbytek integer
    {
        strtod(argv[2], &last_char);
        if (*last_char != '\0')
            return ERR_AUX;
        for (int i = 3; i < argc; i++)
        {
            strtol(argv[i], &last_char, 10);
            if (*last_char != '\0')
                return ERR_AUX;
        }
        return COMPARE;
    }

    if (strcmp(argv[1 + arg_shift], "-m") == 0)             //pro vypocet jeden nebo dva double argumenty
    {
        for (int i = 2 + arg_shift; i < argc; i++)
        {
            strtod(argv[i], &last_char);
            if (*last_char != '\0')
                return ERR_AUX;
        }
        return BOTH;
    }
    return ERR_AUX;
}

/**
 * Zpracovani volitelneho argumentu a nastaveni vysky mericiho pristroje.
 * @param height Promenna ve ktere je ulozena vyska pristrije.
 * @param number Retezec znaku reprezentujici zadanou vysku pristroje.
 * @return Vraci pocet uspesne zpracovanych argumentu nebo 0 pokud nejaky argument neslo zpracovat.
 */
int set_height(double *height, char *number)
{
    char *last_char = NULL;

    *height = strtod(number, &last_char);                               //zadana vyska v intevalu (0; 1,4>
    if (*last_char != '\0' || *height <= 0.0 || *height > MAX_HEIGHT)
        return 0;
    return 2;                   //posunuti argumentu -m
}

/**
 * Porovnani vypoctu tangens pomoci funkce z math.h, taylorova polynomu a zretezenych zlomku
 * v zadanem intervalu.
 * @param x Uhel pro ktery srovnani probehnout.
 * @param n Dolni limit poctu iteraci.
 * @param m Horni limit poctu iteraci.
 * @return Vraci Ok pokud nedoslo k chybe, ERR_RANGE pokud byly pocty iteraci zadany mimo povoleny inteval.
 */
int tan_compare(double x, unsigned int n, unsigned int m)
{
    double aux_tan;
    double aux_ttan;
    double aux_cftan;

    if (n < 1 || n > m || m > TAYLOR_LIMIT)                             //pocet iteraci v rozmezi 1 az 13 vcetne
        return ERR_RANGE;

    for (; n <= m; n++)                                                 //iterace pro cele vybrane rozmezi
    {
        aux_tan = tan(x);
        aux_ttan = taylor_tan(x, n);
        aux_cftan = cfrac_tan(x, n);
        printf("%d %e %e %e %e %e\n", n, aux_tan, aux_ttan, my_abs(aux_tan - aux_ttan), aux_cftan, my_abs(aux_tan - aux_cftan));
    }
    return OK;
}

/**
 * Vypocet tangens pomoci taylorova polynomu.
 * @param x Uhel pro ktery se ma vypocitat tangens.
 * @param n Pocet iteraci vypoctu.
 * @return Vraci tangens zadaneho uhlu.
 */
double taylor_tan(double x, unsigned int n)
{
    const long long a[TAYLOR_LIMIT] = {1, 1, 2, 17, 62, 1382, 21844, 929569, 6404582, 443861162, 18888466084, 113927491862, 58870668456604};
    const long long b[TAYLOR_LIMIT] = {1, 3, 15, 315, 2835, 155925, 6081075, 638512875, 10854718875, 1856156927625, 194896477400625, 49308808782358125, 3698160658676859375};
    double sum = x;     //mezisoucet clenu
    double xn = x;      //citatel
    x = x * x;          //kazdy dalsi citatel x^2 nasobkem predchoziho
    for (unsigned int i = 1; i < n; i++)
    {
        xn *= x;                        //yvetseni citatele
        sum += xn * a[i] / b[i];        //vypocet clenu polynomu
    }
    return sum;
}

/**
 * Vypocet tangens pomoci zretezenych zlomku.
 * @param x Uhel pro ktery se ma vypocitat tangens.
 * @param n Pocet iteraci vypoctu.
 * @return Vraci tangens zadaneho uhlu.
 */
double cfrac_tan(double x, unsigned int n)
{
    double bn = 2 * n - 1;          //ve jmenovatelich zlomku licha cisla
    double lf = x / (bn + 2);       //prvni mezivysledek
    for (; bn > 0; bn -= 2)         //pouze licha cisla, ridici promenna se snizuje o 2 kazdou iteraci
        lf = 1.0 / ((bn / x) - lf); //vypocet mezivysledku
    return lf;
}

/**
 * Urceni absolutni hodnoty.
 * @param x Cislo ze ktereho se ma vypocitat absolutni hodnota.
 * @return Vraci absolutni hodnotu zadaneho cisla.
 */
double my_abs(double x)
{
    return x < 0 ? -x : x;
}

/**
 * Vytiskne chybove hlaseni na chybovy vystup.
 */
void print_err(int errnum)
{
    const char *err_mes[] = {
        "neplatne argumrnty",
        "porovnani v neplatnych mezich",
        "uhel alfa mimo limit",
        "uhel beta mimo limit"};

    fprintf(stderr, "Chyba: %s!\n", err_mes[errnum - 1]);
}

/**
 * Vytiskne na standardni vystup text s napovedou.
 */
void print_help()
{
    printf("Program srovnava presnost ruznych vypoctu tangens nebo ze zadanych uhlu pocita\n"
           "vzdalenost a vysku objektu.\n\n"
           "Srovnani presnosti se spousti s argumenty:\n"
           "--tan X N M\n"
           "X\tudava velikost uhlu pro ktery se maji hodnoty tangens pocitat\n"
           "N a M\tudava pocty iteraci vypoctu, N i M musi byt z intervalu 1 az 13 vcetne\n"
           "\ta N nesmi byt vetsi nez M\n\n"
           "Vypocet vzdalenosti a vysky se spousti s argumenty:\n"
           "[-c X] -m A [B]\n"
           "-c X\tnastavuje vysku pristroje, argument je volitelny\n"
           "A\tudava velikost uhlu alfa, musi byt z intevalu (0; 1,4>\n"
           "B\tudava velikost uhlu beta z intevalu (0; 1,4>, argument je volitelny\n\n"
           "Pokud je zadal pouze uhel alfa program spocita vzdalenost objektu.\n"
           "Pokud je zadan i uhel beta program spocita i vysku objektu.\n\n"
           "Autor: Ondrej Vales\n"
           "Datum vytvoreni: 26. 11. 2014\n");
}
