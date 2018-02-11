// Soubor: h2o.h
// Reseni IOS-Projekt 2
// Datum: 3. 5. 2015
// Autor: Ondrej Vales, xvales03
// Fakulta: FIT VUTBR

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <semaphore.h>
#include <unistd.h>

#define SEM_CNT 9

enum sem_names {ATOMIC, CNTH, CNTO, BOND, BARRIER, WRTF, FINISH, AUX, CNTB};
enum wait_times {GH, GO, B};

typedef struct {
    unsigned count;
    useconds_t utime[3];
} args_t;

typedef struct {
	unsigned line_cnt;
	unsigned queue_h;
	unsigned queue_o;
	unsigned h;
	unsigned bnd;
	FILE *fd;
} data_t;

typedef struct {
	int shmid;
	sem_t *mtx;
	data_t *ptr;
} shmem_t;

int process_args (int argc, char *argv[], args_t *pargs);
void create_o(args_t args, shmem_t shmem);
void create_h(args_t args, shmem_t shmem);
void process_o(args_t args, shmem_t shmem, unsigned serial_o);
void process_h(args_t args, shmem_t shmem, unsigned serial_h);
void bond(args_t args, shmem_t shmem, char type, unsigned serial);
void cleanup_fail(shmem_t shmem);
