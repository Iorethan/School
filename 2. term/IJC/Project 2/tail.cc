// Soubor: tail.cc
// Reseni IJC-DU2, priklad 1
// Datum: 24. 4. 2015
// Autor: Ondrej Vales, xvales03
// Fakulta: FIT VUTBR
// Prelozeno: gcc 4.8

#include <iostream>
#include <string>
#include <istream>
#include <fstream>
#include <cstdlib>
#include <queue>
#include <sstream>

using namespace std;

enum MODS {LAST, FROM};	// Rozliseni tisku poslednich n radku a vsech radku od radku n.

typedef struct{
    long int line;
    int mode;
} mode_s;

int process_args(int argc, char *argv[], mode_s& args, ifstream& file);
void print_last (mode_s args, ifstream& file);
void print_from (mode_s args, ifstream& file);
void read_file (queue<string>& lines, string& line, ifstream& file);


int main(int argc, char *argv[])
{
    ios::sync_with_stdio(false);
    mode_s args= {10, LAST};	// Implicitne tisknu 10 poslednich radku.
    ifstream file;
    if (argc != process_args(argc, argv, args, file))	// Overeni spravnosti argumentu.
    {
        cerr << "CHYBA: spatne argumenty!" << endl;
        return 1;
    }

    switch (args.mode)	// Zvoleni pozadovane funkce tail
    {
    case LAST:
        if (args.line != 0)
            print_last(args, file);
        break;
    case FROM:
        print_from(args, file);
        break;
    }


    if (file.is_open())	// Zavreni souboru
        file.close();

    return 0;
}

int process_args(int argc, char *argv[], mode_s& args, ifstream& file)
{
    int counter = 1;		// Ciatc zpracovanych arumentu.
    char *ptr;

    if (argc > 2 && string(argv[1])  == "-n")	// Overeni pritomnosti dvojice argumentu -n [+]cislo.
    {
        if (argv [2][0] == '+')
        {
            args.mode = FROM;		// Zmena modu.
        }
        args.line = abs(strtol(argv[2], &ptr, 10));	// Prevod retezce na cislo.
        if (*ptr == '\0')
            counter += 2;		// Pokud prevod probehl uspesne zmenim pocet zpracovanych argumentu.
    }

    if (argc > counter)		// Dalsi argument je nazev souboru.
    {
        file.open(argv[counter]);		// Pokud se mi ho podari otevrit zpracoval sem dalsi argument.
        if (file.is_open())
            counter++;
    }

    return counter;
}

void print_last (mode_s args, ifstream& file)
{
    queue<string> lines;
    string line;

    read_file(lines, line, file);	// Nacteni vsech radku.

    while (lines.size() > static_cast <unsigned long> (args.line)) // Preskoceni radku co netisknu.
    {
        lines.pop();
    }

    while (!lines.empty())		// Tisk zbylych radku.
    {
        line = lines.front();
        cout << line << endl;
        lines.pop();
    }
}

void print_from (mode_s args, ifstream& file) // Preskoceni radku co netisknu.
{
    queue<string> lines;
    string line;

    read_file(lines, line, file);	// Nacteni vsech radku.

    for (int i = 1; i < args.line ; i++)
    {
        lines.pop();
    }

    while (!lines.empty())		// Tisk zbylych radku.
    {
        line = lines.front();
        cout << line << endl;
        lines.pop();
    }
}

void read_file (queue<string>& lines, string& line, ifstream& file) // Nacteni vsech radku ze souboru/cin
{
    if (file.is_open())
    {
        while (getline(file, line))
            lines.push(line);
    }
    else
    {
        while (getline(cin, line))
            lines.push(line);
    }
}
