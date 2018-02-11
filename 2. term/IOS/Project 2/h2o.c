// Soubor: h2o.c
// Reseni IOS-Projekt 2
// Datum: 3. 5. 2015
// Autor: Ondrej Vales, xvales03
// Fakulta: FIT VUTBR

#include "h2o.h"

int main(int argc, char *argv[])
{
	/* Zpracovani argumentu */
    args_t args;
    if (process_args(argc, argv, &args) != 0)		// Argumenty neodpovidaji predepsanemu formatu.
    {
        fprintf(stderr, "CHYBA: spatne argumenty!\n");
        exit(1);
    }
	
	shmem_t shmem;
	pid_t pid, pid2;
	
	/* Prideleni sdilene pameti. */
	if ((shmem.shmid = shmget(IPC_PRIVATE, sizeof(data_t), IPC_CREAT | 0666)) == -1)		// Nepodarilo se pridelit sdilenou pamet.
	{
        fprintf(stderr, "CHYBA: nelze pridelit sdilenou pamet!\n");
        exit(2);
	}
	if ((shmem.ptr = shmat(shmem.shmid, NULL, 0)) == (void *) -1)		// Nelze se pripojit ke sdilene pameti.
	{
        fprintf(stderr, "CHYBA: nelze pridelit sdilenou pamet!\n");
		shmctl(shmem.shmid, IPC_RMID, NULL);
        exit(2);		
	}
	if ((shmem.mtx = mmap(NULL, SEM_CNT * sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0)) == (void *) -1)		// Nepodarilo se pridelit sdilenou pamet.
	{
        fprintf(stderr, "CHYBA: nelze pridelit sdilenou pamet!\n");
		shmdt(shmem.ptr);
		shmctl(shmem.shmid, IPC_RMID, NULL);
        exit(2);	
	}
    
	/* Inicializace semaforu */
	if (sem_init(shmem.mtx + ATOMIC, 1, 1) == -1)
		cleanup_fail(shmem);
	if (sem_init(shmem.mtx + CNTH, 1, 0) == -1)
		cleanup_fail(shmem);
	if (sem_init(shmem.mtx + CNTO, 1, 0) == -1)
		cleanup_fail(shmem);
	if (sem_init(shmem.mtx + BOND, 1, 1) == -1)
		cleanup_fail(shmem);
	if (sem_init(shmem.mtx + WRTF, 1, 1) == -1)
		cleanup_fail(shmem);
	if (sem_init(shmem.mtx + FINISH, 1, 0) == -1)
		cleanup_fail(shmem);
	if (sem_init(shmem.mtx + AUX, 1, 1) == -1)
		cleanup_fail(shmem);
	if (sem_init(shmem.mtx + CNTB, 1, 0) == -1)
		cleanup_fail(shmem);
	
	/* Inicializace promennych */
	if (((shmem.ptr)->fd = fopen("h2o.out", "w")) == NULL)
		cleanup_fail(shmem);
	(shmem.ptr)->line_cnt = 1;
	(shmem.ptr)->queue_h = 0;
	(shmem.ptr)->queue_o = 0;
	
	/* Zacatek programu */
	pid = fork();
	if (pid == 0)
	{
		create_o(args, shmem);		// Proces tvorici atomy O.
		_Exit(0);
	}
	else if (pid < 0)		// Chyba pri fork.
	{	
		sem_destroy(shmem.mtx + ATOMIC);
		sem_destroy(shmem.mtx + CNTH);
		sem_destroy(shmem.mtx + CNTO);
		sem_destroy(shmem.mtx + BOND);
		sem_destroy(shmem.mtx + WRTF);
		sem_destroy(shmem.mtx + FINISH);
		sem_destroy(shmem.mtx + AUX);
		sem_destroy(shmem.mtx + CNTB);	
		cleanup_fail(shmem);		
	}
	else	// Pokracovani hlavniho programu.
	{
		{
			pid2 = fork();
			if (pid2 == 0)		// Proces tvorici atomy H.
			{
				create_h(args, shmem);
				_Exit(0);
			}
			else if (pid2 < 0)		// Chyba pri fork.	
			{	
				sem_destroy(shmem.mtx + ATOMIC);
				sem_destroy(shmem.mtx + CNTH);
				sem_destroy(shmem.mtx + CNTO);
				sem_destroy(shmem.mtx + BOND);
				sem_destroy(shmem.mtx + WRTF);
				sem_destroy(shmem.mtx + FINISH);
				sem_destroy(shmem.mtx + AUX);
				sem_destroy(shmem.mtx + CNTB);	
				cleanup_fail(shmem);
			}
		}
	}	
	
	waitpid(pid, NULL, 0); 
	waitpid(pid2, NULL, 0); 
	
	/* Odstraneni semaforu */
	sem_destroy(shmem.mtx + ATOMIC);
	sem_destroy(shmem.mtx + CNTH);
	sem_destroy(shmem.mtx + CNTO);
	sem_destroy(shmem.mtx + BOND);
	sem_destroy(shmem.mtx + WRTF);
	sem_destroy(shmem.mtx + FINISH);
	sem_destroy(shmem.mtx + AUX);
	sem_destroy(shmem.mtx + CNTB);	
	
	/* Uvolneni sdilene pameti */
	fclose((shmem.ptr)->fd);
	munmap(shmem.mtx, SEM_CNT * sizeof(sem_t));
	shmdt(shmem.ptr);
	shmctl(shmem.shmid, IPC_RMID, NULL);
	exit(0);
}

int process_args (int argc, char *argv[], args_t *pargs)
{
    long aux;
    char *ptr = NULL;
    if (argc == 5)
    {
        aux = strtol(argv[1], &ptr, 10);		// Kontrola a prevod prvniho argumentu.
        if (*ptr != '\0' || aux < 1)
            return 1;
        pargs->count = (unsigned)aux;

        for (int i = 2; i < argc; i++)		// Kontrola a prevod zbyvajicich argumentu.
        {
            aux = strtol(argv[i], &ptr, 10);
            if (*ptr != '\0' || aux < 0 || aux > 5000)
                return 1;
			if (aux == 0)
				aux++;
            pargs->utime[i - 2] = (unsigned)aux;
        }
        return 0;
    }
    return 1;		// Argumenty neodovidaji predepsanemu formatu.
}

void bond(args_t args, shmem_t shmem, char type, unsigned serial)
{
	/* Proces tvorby molekul h2o */
	sem_wait(shmem.mtx + WRTF);		// Atomicky zapis do souboru.
	fprintf((shmem.ptr)->fd, "%u\t: %c %u\t: begin bonding\n", (shmem.ptr)->line_cnt, type, serial);
	fflush((shmem.ptr)->fd);
	(shmem.ptr)->line_cnt++;
	sem_post(shmem.mtx + WRTF);
	
	useconds_t period_h;
	period_h = rand()%(1000*(args.utime)[B]);
	usleep(period_h);			// Uspani na dobu tvoreni molekuly B.
}

void cleanup_fail(shmem_t shmem)
{
	/* Uvolneni pameti a vypis hlaseni na chybovy vystup */
	fprintf(stderr, "CHYBA: selhala operace se semaforem nebo systemove volani!\n");
	munmap(shmem.mtx, SEM_CNT * sizeof(sem_t));
	shmdt(shmem.ptr);
	shmctl(shmem.shmid, IPC_RMID, NULL);
	exit(2);
}
