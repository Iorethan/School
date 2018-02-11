// Soubor: o.c
// Reseni IOS-Projekt 2
// Datum: 3. 5. 2015
// Autor: Ondrej Vales, xvales03
// Fakulta: FIT VUTBR

#include "h2o.h"

void create_o(args_t args, shmem_t shmem)
{
	/* Proces vytvarejici atomy O */
	time_t t;
	pid_t pidp[args.count];
	srand((unsigned) time(&t));
	for (unsigned i = 0; i < args.count; i++)		// Cyklus vytvarejici procesy O.
	{
		useconds_t period_o;
		period_o = rand()%(1000*(args.utime)[GO]);
		usleep(period_o);		// Uspani na dobu GO.
		
		pidp[i] = fork();
		if (pidp[i] == 0)		// Novy proces.
		{
			process_o(args, shmem, i + 1);
			_Exit(0);
		}
		else if (pidp[i] < 0)		// Osetreni chyby pri fork.
		{
			for (unsigned j = i; j > 0; j--)
				kill(pidp[j], SIGKILL);
			return;
		}
	}
	for (unsigned i = 0; i < args.count; i++)		// Cekani na vsechny potomky.		
		waitpid(pidp[i], NULL, 0); 
	return;
}

void process_o(args_t args, shmem_t shmem, unsigned serial_o)
{	
	/* Proces kyslik */
	sem_wait(shmem.mtx + WRTF);		// Atomicky zapis do souboru.
	fprintf((shmem.ptr)->fd, "%u\t: O %u\t: started\n", (shmem.ptr)->line_cnt, serial_o);
	fflush((shmem.ptr)->fd);
	(shmem.ptr)->line_cnt++;
	sem_post(shmem.mtx + WRTF);
	
	sem_wait(shmem.mtx + ATOMIC);
	(shmem.ptr)->queue_o++;
	if ((shmem.ptr)->queue_h >= 2)
	{
		sem_wait(shmem.mtx + WRTF);		// Atomicky zapis do souboru.
		fprintf((shmem.ptr)->fd, "%u\t: O %u\t: ready\n", (shmem.ptr)->line_cnt, serial_o);
		fflush((shmem.ptr)->fd);
		(shmem.ptr)->line_cnt++;
		sem_post(shmem.mtx + WRTF);	
		
		(shmem.ptr)->queue_h -= 2;
		(shmem.ptr)->queue_o--;
		sem_init(shmem.mtx + BARRIER, 1, 0);
		(shmem.ptr)->h = 0;
		sem_post(shmem.mtx + CNTH);
		sem_post(shmem.mtx + CNTH);
		sem_post(shmem.mtx + CNTO);		
	}
	else
	{
		sem_wait(shmem.mtx + WRTF);		// Atomicky zapis do souboru.
		fprintf((shmem.ptr)->fd, "%u\t: O %u\t: waiting\n", (shmem.ptr)->line_cnt, serial_o);
		fflush((shmem.ptr)->fd);
		(shmem.ptr)->line_cnt++;
		sem_post(shmem.mtx + WRTF);	
		sem_post(shmem.mtx + ATOMIC);		
	}
	
	sem_wait(shmem.mtx + CNTO);
	bond(args, shmem, 'O', serial_o);
	
	sem_wait(shmem.mtx + AUX);
	(shmem.ptr)->bnd++;		
	if((shmem.ptr)->bnd == 3)
	{
		(shmem.ptr)->bnd = 0;
		sem_post(shmem.mtx + CNTB);	
		sem_post(shmem.mtx + CNTB);
		sem_post(shmem.mtx + CNTB);	
	}
	sem_post(shmem.mtx + AUX);
	sem_wait(shmem.mtx + CNTB);
	
	sem_wait(shmem.mtx + WRTF);		// Atomicky zapis do souboru.
	fprintf((shmem.ptr)->fd, "%u\t: O %u\t: bonded\n", (shmem.ptr)->line_cnt, serial_o);
	fflush((shmem.ptr)->fd);
	(shmem.ptr)->line_cnt++;
	sem_post(shmem.mtx + WRTF);
	
	sem_wait(shmem.mtx + BARRIER);
	sem_destroy(shmem.mtx + BARRIER);
	
	sem_post(shmem.mtx + ATOMIC);
	
	if (serial_o == args.count)
		sem_post(shmem.mtx + FINISH);
	
	sem_wait(shmem.mtx + FINISH);
	sem_post(shmem.mtx + FINISH);
	
	sem_wait(shmem.mtx + WRTF);		// Atomicky zapis do souboru.
	fprintf((shmem.ptr)->fd, "%u\t: O %u\t: finished\n", (shmem.ptr)->line_cnt, serial_o);
	fflush((shmem.ptr)->fd);
	(shmem.ptr)->line_cnt++;
	sem_post(shmem.mtx + WRTF);
}
