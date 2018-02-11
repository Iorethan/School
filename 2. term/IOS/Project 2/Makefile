# Soubor: Makefile
# Reseni IOS-Projekt 2
# Datum: 3. 5. 2015
# Autor: Ondrej Vales, xvales03
# Fakulta: FIT VUTBR

all: h2o

h2o: h2o.o h2o.h o.o h.o
	gcc -std=gnu99 -Wall -Wextra -Werror -pedantic -pthread h2o.o h2o.h o.o h.o -o h2o

h2o.o: h2o.c h2o.h
	gcc -std=gnu99 -Wall -Wextra -Werror -pedantic -pthread -c h2o.c
	
o.o: o.c h2o.h
	gcc -std=gnu99 -Wall -Wextra -Werror -pedantic -pthread -c o.c
	
h.o: h.c h2o.h
	gcc -std=gnu99 -Wall -Wextra -Werror -pedantic -pthread -c h.c
	
clean:
	rm *.o
	
cleanall:
	rm h2o *.o