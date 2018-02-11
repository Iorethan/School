/**
 * Soubor:  proj1.c
 * Datum:   16. 11. 2014
 * Autor:   Ondrej vales, xvales03@stud.fit.vutbr.cz
 * Projekt: Projekt 1 IZP - Vypocty v tabulce
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define MAX_DELKA 1024

/**< Prototypy vsech funkci vyskytujicich se v programu. */
int vyber_oblast(int radek_od, int radek_do, int sloupec_od, int sloupec_do, char *operace);
int inicializace(int *radek_do, int *sloupec_do, double *vysledek, char *operace);
int vyber_operace(char *operace, char *bunka, double *vysledek, int *pocet_bunek);
int nacti_radek(char *radek);
int overeni_arg(int argc, char **argv);
int overeni_cisla(char *bunka);
void hlaseni_chyb(int chyba);
void napoveda();

/** Kody chyb vyskytujicich se v programu. */
enum cislo_chyby
{
  CH_OK,            /**< Bez chyby. */
  CH_ARG,           /**< Chyba ve vstupnich argumentech. */
  CH_OVF,           /**< Radek presahujici maximalni delku. */
  CH_MIMO,          /**< Vyber bunek mimo tabulku. */
  CH_NECISLO,       /**< Operace nad bunkou s neciselnym obsahem. */
  CH_PRAZDNO,       /**< Radek neobsahuje ani jednu bunku. */
  CH_CHYBA = -1,    /**< Obecna chyba */
  CH_OVFAUX = -2    /**< Pomocna chyba pro vystup z funkce nacti_radek */
};

/**< Kody vyberu oblsati tabulky pro funkce overeni_arg a main. */
enum cislo_vyberu
{
  HELP,
  ROW,
  ROWS,
  COL,
  COLS,
  RANGE
};

/**
 * Hlavni program.
 */
int main(int argc, char **argv)
{
    int cislo_vyberu;
    int chyba = CH_OK;

    cislo_vyberu = overeni_arg(argc, argv);

    switch (cislo_vyberu)
    {
    case HELP:
        napoveda();
        break;
    case ROW:
        chyba = vyber_oblast(atoi(argv[3]), atoi(argv[3]), 1, 0, argv[1]);
        break;
    case ROWS:
        chyba = vyber_oblast(atoi(argv[3]), atoi(argv[4]), 1, 0, argv[1]);
        break;
    case COL:
        chyba = vyber_oblast(1, 0, atoi(argv[3]), atoi(argv[3]), argv[1]);
        break;
    case COLS:
        chyba = vyber_oblast(1, 0, atoi(argv[3]), atoi(argv[4]), argv[1]);
        break;
    case RANGE:
        chyba = vyber_oblast(atoi(argv[3]), atoi(argv[4]), atoi(argv[5]), atoi(argv[6]), argv[1]);
        break;
    default :
        chyba = CH_ARG;
    }
    if (chyba != CH_OK)
        hlaseni_chyb(chyba);
    return chyba;
}

/**
 * Funkce pro praci s jednotlivymi radky tabulky.
 * @param radek_od Cislo radku od ktereho se ma zpracovavat tabulka.
 * @param radek_do Cislo radku do ktereho se ma zpracovavat tabulka.
 * @param sloupec_od Cislo sloupce od ktereho se maji z radku vybirat bunky.
 * @param sloupec_do Cislo sloupce do ktereho se maji z radku vybirat bunky.
 * @param operace Pole zanku reprezentujici operaci, ktera se ma provest.
 * @return Vraci kody chyb z enum cislo_chyby.
 */
int vyber_oblast(int radek_od, int radek_do, int sloupec_od, int sloupec_do, char *operace)
{
    char radek[MAX_DELKA + 2];
    char *bunka;
    int pocet_znaku;
    double vysledek = 0;
    int pocet_bunek = 0;
    int nekonecne_cteni;

    nekonecne_cteni = inicializace(&radek_do, &sloupec_do, &vysledek, operace);

    int i_radek_od = 1;
    for (; i_radek_od < radek_od; i_radek_od++)  //Preskoceni nepotrebnych radku.
    {
        pocet_znaku = nacti_radek(radek);
        if (pocet_znaku == EOF)
            return CH_MIMO;
        if (pocet_znaku == CH_OVFAUX)
            return CH_OVF;
    }

    for (; i_radek_od <= radek_do; i_radek_od++)    //Nacitani radku se kterymi se ma pracovat.
    {
        pocet_znaku = nacti_radek(radek);
        if (pocet_znaku == EOF && nekonecne_cteni == 1 && radek_od != i_radek_od)
            break;
        if (pocet_znaku == EOF)
            return CH_MIMO;
        if (pocet_znaku == CH_OVFAUX)
            return CH_OVF;
        if (*radek == '\n')
            return CH_PRAZDNO;

        bunka = radek;
        int i_sloupec_od = 1;                                   //Promena pro iteraci pres bunky radku. Puvodni hodnotu sloupec_od nutno zachovat!
        for (; i_sloupec_od < sloupec_od; i_sloupec_od++)       //Preskoceni nepotrebnych bunek.
        {
            if (*bunka == '\n')
                return CH_MIMO;
            bunka += strlen(bunka) + 1;
        }
        for (; i_sloupec_od <= sloupec_do; i_sloupec_od++)      //Vyber bunek se kterymi se ma pracovat.
        {
            if (*bunka == '\n' && nekonecne_cteni == 1 && radek_do != INT_MAX)
                break;
            if (*bunka == '\n')
                return CH_MIMO;
            if (vyber_operace(operace, bunka, &vysledek, &pocet_bunek) == 4)        //Predani pozadovanych bunek funkci provadejici operace.
                return CH_NECISLO;
            bunka += strlen(bunka) + 1;
        }
    }

    if (strcmp(operace, "avg") == 0)                    //Vypis vysledku.
        printf("%.10g\n", vysledek / pocet_bunek);
    else if (strcmp(operace, "select") == 0)            //Vypis vysledku u operace select provadi funkce vyber_operace.
        ;
    else
        printf("%.10g\n", vysledek);
    return CH_OK;
}

/**
 * Inicializace hodnot vysledku podle operace.
 * Nastaveni maxilamni hodnoty do ktere cist radky a sloupce.
 * @param radek_do Hodnota do ktere se ctou radky.
 * @param sloupec_do Hodnota do ktere se ctou sloupce.
 * @param vysledek Promena do ktere se uklada vysldek matematickych operaci.
 * @param operace Retezec reprezentujici vybranou operaci..
 * @return Vraci 0 pokud je omezeno kam az se ma tablka cist, 1 pokud to omezeno neni.
 */
int inicializace(int *radek_do, int *sloupec_do, double *vysledek, char *operace)
{
    if (strcmp(operace, "min") == 0)
        *vysledek = 1.0/0.0;
    if (strcmp(operace, "max") == 0)
        *vysledek = -1.0/0.0;

    if (*sloupec_do == 0)
    {
        *sloupec_do = INT_MAX;
        return 1;
    }
    if (*radek_do == 0)
    {
        *radek_do = INT_MAX;
        return 1;
    }
    return 0;
}

/**
 * Provedeni prislusne operace s bunkou z tabulky. Ulozeni vysledne hodnoty do promene vysledek nebo vypis na stdout.
 * @param operace Pole znaku reprezentujici pozadovanou operace.
 * @param bunka Pole znaku reprezentujici bunku tabulky.
 * @param vysledek Promena do ktere se uklada vysldek matematickych operaci.
 * @param pocet_bunek Pomocna promena pro operaci aritmeticky prumer.
 * @return Vraci CH_NECISLO pokud je provadena matematicka operace s necislenou bunkou, jinak CH_OK.
 */
int vyber_operace(char *operace, char *bunka, double *vysledek, int *pocet_bunek)
{
    if (overeni_cisla(bunka) == CH_NECISLO && strcmp(operace, "select") != 0)       //S neciselnou bunkou pracuje poze operace select.
            return CH_NECISLO;

    if (strcmp(operace, "select") == 0)
    {
        if (overeni_cisla(bunka) == CH_NECISLO)     //Neciselna hodnota vypsana jako retzec.
            printf("%s\n", bunka);
        if (overeni_cisla(bunka) == CH_OK)          //Ciselna hodnota vypsana jako cislo.
            printf("%.10g\n", atof(bunka));
    }else if (strcmp(operace, "min") == 0)
    {
        if (atof(bunka) < *vysledek)
            *vysledek = atof(bunka);
    }else if (strcmp(operace, "max") == 0)
    {
        if (atof(bunka) > *vysledek)
            *vysledek = atof(bunka);
    }else if (strcmp(operace, "sum") == 0)
    {
        *vysledek += atof(bunka);
    }else if (strcmp(operace, "avg") == 0)
    {
        *vysledek += atof(bunka);
        (*pocet_bunek)++;
    }
    return CH_OK;
}

/**
 * Cteni znaku ze stdin ukoncene znakem \n nebo EOF a jejich zapis do pole znaku.
 * Odstraneni vicenasobnych bilych znaku mezi bunkami a bilich znaku pred prvni bunkou.
 * Za bile znaky povazuje funkce mezeru a tabulator.
 * Jednotlive bunky jsou oddelene znakem \0. Toto umoznuje tisknuti bunky jako retezec a pouziti funkce strlen.
 * Znak konce radku je v samostatne bunce.
 * @param radek Pole znaku kam jsou zapisovany znaky ze stdin.
 * @return Vraci delku nacteneho retezce nebo CH_CHYBA pokud je radek ukoncen EOF nebo CH_OVFAUX pokud je prekrocena maximalni delka radku.
 */
int nacti_radek(char *radek)
{
    int pozice_znaku = 0;
    int pocet_bilych = 1;       //Indikuje zda predchazejici nacteny znak byl bily.
    int posledni_znak;

    do
    {
        do
        {
            posledni_znak = getchar();
            if (posledni_znak == '\r')      //Prevedeni ukonceni radku CRLF na LF.
                posledni_znak = getchar();
        } while ((posledni_znak == ' ' || posledni_znak == '\t') && pocet_bilych == 1);
        if (posledni_znak != ' ' && posledni_znak != '\t')  //Zapis normalnich znaku.
        {
            pocet_bilych = 0;
            radek[pozice_znaku] = posledni_znak;
        }
        else                                                //Nahrazeni bileho znaku znakem \0.
        {
            pocet_bilych = 1;
            radek[pozice_znaku] = '\0';
        }
        pozice_znaku++;
        if (pozice_znaku > MAX_DELKA)
            return CH_OVFAUX;
    } while (posledni_znak != '\n' && posledni_znak != EOF);    //Cteni znaku je ukonceno \n nebo EOF.

    if (pozice_znaku > 1 && radek[pozice_znaku - 2] != '\0')    //Posunuti \n do vlastni bunky.
    {
        radek[pozice_znaku] = '\n';
        radek[pozice_znaku - 1] = 0;
        pozice_znaku++;
    }

    if (posledni_znak == EOF)
        return CH_CHYBA;
    return pozice_znaku;
}

/**
 * Overeni platnosti argumentu a jejich zpracovani.
 * @param argc Pocet argumentu funkce.
 * @param argv Pole retezcu reprezentujicich argumenty.
 * @return Vraci prislusny kod argumentu z enum cislo_vyberu nebo CH_CHYBA pokud argumenty nejsou platne.
 */
int overeni_arg(int argc,char **argv)
{
    for (int i = 3; i < argc; i++)
        if (overeni_cisla(argv[i]) == CH_NECISLO)
            return CH_CHYBA;
    if (argc == 1)
        return CH_CHYBA;
    if (argc == 2 && (strcmp(argv[1], "--help") == 0))
        return HELP;
    if (strcmp(argv[1], "select") != 0 && strcmp(argv[1], "min") != 0 && strcmp(argv[1], "max") != 0 && strcmp(argv[1], "sum") != 0 && strcmp(argv[1], "avg") != 0)
        return CH_CHYBA;
    if (argc == 4 && strcmp(argv[2], "row") == 0 && atoi(argv[3]) > 0)
        return ROW;
    if (argc == 5 && strcmp(argv[2], "rows") == 0 && atoi(argv[3]) > 0 && atoi(argv[4]) >= atoi(argv[3]))
        return ROWS;
    if (argc == 4 && strcmp(argv[2], "col") == 0 && atoi(argv[3]) > 0)
        return COL;
    if (argc == 5 && strcmp(argv[2], "cols") == 0 && atoi(argv[3]) > 0 && atoi(argv[4]) >= atoi(argv[3]))
        return COLS;
    if (argc == 7 && strcmp(argv[2], "range") == 0 && atoi(argv[3]) > 0 && atoi(argv[4]) >= atoi(argv[3]) && atoi(argv[5]) > 0 && atoi(argv[6]) >= atoi(argv[5]))
        return RANGE;
    return CH_CHYBA;
}

/**
 * Kontrola obsahu bunky.
 * @param bunka Pole znaku reprezentujici jednu bunku tabulky.
 * @return Vraci CH_OK pokud bunka obsahuje cislo, CH_NECISLO pokud ne.
 */
int overeni_cisla(char *bunka)
{
    if (strlen(bunka) == 1 && (bunka[0] < '0' || bunka[0] > '9'))       //Pokud tvori cislo jediny znak musi to byt cislice.
        return CH_NECISLO;
    else
    {
         if (bunka[0] != '-' && bunka[0] != '+' && (bunka[0] < '0' || bunka[0] > '9'))      //Prvni znak muze byt i + nebo -.
              return CH_NECISLO;
         for (unsigned int i = 1; i < strlen(bunka); i++)
         {
              if (bunka[i] < '0' || bunka[i] > '9')
                  return CH_NECISLO;
         }
    }
    return CH_OK;
}

/**
 * Vytiskne chybove hlaseni na chybovy vystup.
 */
void hlaseni_chyb(int chyba)
{
    const char *hlaseni[] ={
        "neplatne argumenty",
        "moc dlouhy radek",
        "operace mimo tabulku",
        "matematicka operace nad neciselnou bunkou",
        "prazdny radek"};

    fprintf(stderr, "Chyba: %s!\n", hlaseni[chyba - 1]);
}

/**
 * Vytiskne na standardni vystup text s napovedou.
 */
void napoveda()
{
    printf("Tabulkovy kalkulator\n"
            "Se zadanou tabulkou provadi program jednu z nasledujicich operaci:\n"
            "urceni maxima, urceni minima, vypocet sumy, vypocet prumeru, vypis obsahu.\n"
            "Vyjma vypisu lze operace provadet jen nad bunkami s celociselnym obsahem.\n"
            "Program cte tabulku ze stdin.\n\n"
            "Program se spousti s argumenty:\noperace vyber_bunek\n"
            "operace:\n"
            "max\t\t vyhledani nejvetsi hodnoty\n"
            "min\t\t vyhledani nejmensi hodnoty\n"
            "sum\t\t urceni souctu vsech hodnot\n"
            "avg\t\t urceni aritmetickedo prumeru hodnot\n"
            "select\t\t vypis obsahu bunek\n"
            "vyber_bunek:\n"
            "row X\t\t vyber bunek na radku X (X > 0)\n"
            "col X\t\t vyber bunek ve sloupci X (X > 0)\n"
            "rows X Y\t vyber bunek na radcich X az Y (0 < X <= Y)\n"
            "cols X Y\t vyber bunek ve slupcich X az Y (0 < X <= Y)\n"
            "range A B X Y\t vyber bunek na radich A az B ve sloupcich X az Y\n"
            "\t\t (0 < A <= B, 0 < X <= Y)\n");
}
