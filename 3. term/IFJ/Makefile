CC=gcc
CFLAGS += -std=c11 -pedantic -O3 -DNDEBUG
LDFLAGS=
debug: CFLAGS = -std=c11 -pedantic -Wall -Wextra -O0 -g
opt: CFLAGS += -O3 -DNDEBUG
prof: CFLAGS += -pg
prof: LDFLAGS += -pg

EXEC = ifj15			#name of executable
OFILES =	$(patsubst %.c,%.o,$(wildcard *.c))
LINKED = -lm			#all libraries to be linked

TESTS = lextest tabletest arraytest insttest

.PHONY: all gen clean debug pack all

all: $(EXEC)

insttest: tests/insttest.o varstack.o instructions.o inst_support.o \
dynamic_array.o inst_stack.o ial.o
	$(CC) -o $@ $^ -g $(LINKED)

lextest: tests/lextest.o $(filter-out main.o, $(OFILES))
	$(CC) -o $@ $^ $(LINKED)

tabletest: tests/tabletest.o ial.o
	$(CC) -o $@ $^ $(LINKED)

arraytest: tests/arraytest.o dynamic_array.o error.o
	$(CC) -o $@ $^ $(LINKED)

tests/%.o: tests/%.c
	$(CC) -c $(CFLAGS) $< -o $@

$(EXEC): $(OFILES)
	$(CC) $(LDFLAGS) -o $@ $^ $(LINKED)

gen:
	./gen_vectors.sh

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -f $(EXECS) $(TESTS) *.o tests/*.o
	
debug: all

opt: all

prof: all

pack: gen all
	zip xzenca00 Makefile *.c *.h
