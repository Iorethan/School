all: steg-decode prvocisla prvocisla-inline

steg-decode: steg-decode.o eratosthenes.o error.o ppm.o bit-array.h eratosthenes.h error.h ppm.h
	gcc -std=c99 -Wall -Wextra -pedantic -O2 -lm steg-decode.o eratosthenes.o error.o ppm.o -o steg-decode

prvocisla: prvocisla.o eratosthenes.o error.o bit-array.h eratosthenes.h error.h
	gcc -std=c99 -Wall -Wextra -pedantic -O2 -lm prvocisla.o eratosthenes.o error.o -o prvocisla

prvocisla-inline: prvocisla-inline.o eratosthenes.o error.o bit-array.h eratosthenes.h error.h
	gcc -std=c99 -Wall -Wextra -pedantic -O2 -lm prvocisla-inline.o eratosthenes.o error.o -o prvocisla-inline -DUSE_INLINE

steg-decode.o: steg-decode.c bit-array.h eratosthenes.h error.h ppm.h
	gcc -c -std=c99 -Wall -Wextra -pedantic -O2 -lm steg-decode.c -o steg-decode.o

prvocisla.o: prvocisla.c bit-array.h eratosthenes.h error.h
	gcc -c -std=c99 -Wall -Wextra -pedantic -O2 -lm prvocisla.c -o prvocisla.o

prvocisla-inline.o: prvocisla.c bit-array.h eratosthenes.h error.h
	gcc -c -std=c99 -Wall -Wextra -pedantic -O2 -lm prvocisla.c -o prvocisla-inline.o -DUSE_INLINE

eratosthenes.o: eratosthenes.c bit-array.h error.h
	gcc -c -std=c99 -Wall -Wextra -pedantic -O2 -lm eratosthenes.c -o eratosthenes.o

error.o: error.c error.h
	gcc -c -std=c99 -Wall -Wextra -pedantic -O2 -lm error.c -o error.o

ppm.o: ppm.c bit-array.h error.h ppm.h
	gcc -c -std=c99 -Wall -Wextra -pedantic -O2 -lm ppm.c -o ppm.o