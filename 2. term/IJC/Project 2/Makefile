# Soubor: Makefile
# Reseni IJC-DU2
# Datum: 24. 4. 2015
# Autor: Ondrej Vales, xvales03
# Fakulta: FIT VUTBR

CFLAGS=-std=c99 -Wall -Wextra -pedantic
CCFLAGS=-std=c++11 -Wall -Wextra -pedantic
EXECS= tail tail2 wordcount wordcount-dynamic
LIBS= htable_clear.o htable_foreach.o htable_free.o htable_init.o htable_lookup.o htable_remove.o htable_statistics.o hash_function.o

all: $(EXECS)

tail: tail.c
	gcc $(CFLAGS) $< -o $@
	
tail2: tail.cc
	g++ $(CCFLAGS) $< -o $@
	
wordcount: wordcount.o io.o io.h htable.h libhtable.a
	gcc $(CFLAGS) -static $< io.o libhtable.a -o $@
	
wordcount-dynamic: wordcount.o io.o io.h htable.h libhtable.so
	gcc $(CFLAGS) $< io.o libhtable.so -o $@	
	
wordcount.o: wordcount.c io.h
	gcc -c $(CFLAGS) -c $< -o $@
	
io.o: io.c io.h
	gcc -c $(CFLAGS) -c $< -o $@

libhtable.a: $(LIBS)
	ar crs libhtable.a $(LIBS)

libhtable.so: $(LIBS)
	gcc $(CFLAGS) -shared -fPIC -o libhtable.so $(LIBS)

hash_function.o: hash_function.c htable.h
	gcc $(CFLAGS) -fPIC -c $< -o $@

htable_clear.o: htable_clear.c htable.h
	gcc $(CFLAGS) -fPIC -c $< -o $@

htable_foreach.o: htable_foreach.c htable.h
	gcc $(CFLAGS) -fPIC -c $< -o $@

htable_free.o: htable_free.c htable.h
	gcc $(CFLAGS) -fPIC -c $< -o $@

htable_init.o: htable_init.c htable.h
	gcc $(CFLAGS) -fPIC -c $< -o $@

htable_lookup.o: htable_lookup.c htable.h
	gcc $(CFLAGS) -fPIC -c $< -o $@

htable_remove.o: htable_remove.c htable.h
	gcc $(CFLAGS) -fPIC -c $< -o $@

htable_statistics.o: htable_statistics.c htable.h
	gcc $(CFLAGS) -fPIC -c $< -o $@

cleanall:
	rm -f tail tail2 wordcount wordcount-dynamic libhtable.so libhtable.a *.o

clean:
	rm -f *.o
