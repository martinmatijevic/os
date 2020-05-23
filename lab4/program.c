#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <stdlib.h>
#include "slucajni_prosti_broj.h"

#define MASKA(bitova)	(-1+(1<<(bitova)))
#define UZMIBITOVE(broj,prvi,bitova)	(((broj)>>(64-(prvi)))&MASKA(bitova))
#define N	5
#define M	3
#define BROJ_J_ITERACIJA	300000000

struct gmp_pomocno gg;
uint64_t velicina_grupe = 1;
uint64_t generiraj_dobar_broj (struct gmp_pomocno *g);
uint64_t zbrckanost (uint64_t x);
void procjeni_velicinu_grupe ();
sem_t sPrazni;
sem_t sKO;
sem_t sPuni;

uint64_t MS[5];
uint64_t ULAZ = 0;
uint64_t IZLAZ = 0;
uint64_t BROJAC = 0;
uint64_t kraj = 0;
uint64_t KRAJ_RADA = 1;

void stavi_u_MS (uint64_t broj)
{
	MS[ULAZ] = broj;
	ULAZ = (ULAZ + 1) % 5;
	BROJAC++;
	if( BROJAC > 5 ) {
		BROJAC--;
		IZLAZ = (IZLAZ + 1) % 5;
	}
}

uint64_t uzmi_iz_MS ()
{
	uint64_t broj = MS[IZLAZ];
	if( BROJAC > 0 ) {
		IZLAZ = (IZLAZ + 1) % 5;
		BROJAC--;
	}
	return broj;
}

uint64_t zbrckanost (uint64_t x)
{
	uint64_t z = 0;
	uint64_t i, j, b1, pn;
	for(i = 0; i < 64-4; i++){
		b1 = 0;
		pn = UZMIBITOVE(x, i+4, 4);
		for(j = 0; j < 4; j++) if ((1<<j)&pn) b1++;
		if (b1 > 2) z += b1-2;
		else if (b1 < 2) z += 2-b1;
	}
	return z;
}

uint64_t generiraj_dobar_broj (struct gmp_pomocno *g)
{
	uint64_t najbolji_broj = 0;
	uint64_t najbolja_zbrckanost = 120;
	uint64_t broj, z, i;
	for(i = 0; i < velicina_grupe; i++){
		broj = daj_novi_slucajan_prosti_broj(g);
		z = zbrckanost(broj);
		if (z < najbolja_zbrckanost)
		{
			najbolja_zbrckanost = z;
			najbolji_broj = broj;
		}
	}
	return najbolji_broj;
}

void procjeni_velicinu_grupe ()
{
	uint64_t R = 1000;
	uint64_t k = 0;
	uint64_t SEKUNDI = 5;
	uint64_t i, brojeva_u_sekundi, broj;
	time_t t;
	t = time(NULL);
	while (time(NULL) < (t+SEKUNDI)){
		k++;
		for(i = 0; i < R-1; i++){
			broj = generiraj_dobar_broj(&gg);
			stavi_u_MS(broj);
		}
		ULAZ = 0;
		IZLAZ = 0;
	}
	brojeva_u_sekundi = k*R/SEKUNDI;
	velicina_grupe = brojeva_u_sekundi*2/5;
}

void *radna_dretva (void *id)
{
	struct gmp_pomocno lg;
	uint64_t x;
	int *ID = id;
	int iid = *((int*) id);
	unsigned long brojac = 0;
	unsigned long j;
	if ( iid == N ) {
		struct sched_param prio;
		prio.sched_priority = 1;
		if (pthread_setschedparam (pthread_self(), SCHED_RR, &prio)) {
			perror ( "Greska: pthread_setschedparam (dozvole?)" );
			exit (1);
		}
	}
	inicijaliziraj_generator(&lg, *ID);
	do {
		x = generiraj_dobar_broj(&lg);
		sem_wait(&sPrazni);
		sem_wait(&sKO);
		stavi_u_MS(x);
		printf ("Dretva %d radna ", iid);
		printf("stavio %lx\n", x);
		sem_post(&sKO);
		sem_post(&sPuni);
		for (j = 0; j < BROJ_J_ITERACIJA; j++)
				;
		if ( iid == N ) {
			brojac++;
			if (brojac%5 == 0) sleep(3);
		}
	}
	while (kraj != KRAJ_RADA);
	obrisi_generator(&lg);
	return NULL;
}

void *neradna_dretva (void *id)
{
	uint64_t y;
	int iid = *((int*) id);
	unsigned long brojac = 0;
	unsigned long j;
	if ( iid == M ) {
		struct sched_param prio;
		prio.sched_priority = 1;
		if (pthread_setschedparam (pthread_self(), SCHED_RR, &prio)) {
			perror ( "Greska: pthread_setschedparam (dozvole?)" );
			exit (1);
		}
	}
	do {
		sem_wait(&sPuni);
		sem_wait(&sKO);
		y = uzmi_iz_MS();
		printf ("Dretva %d neradna ", iid);
		printf("uzeo %lx\n", y);
		sem_post(&sKO);
		sem_post(&sPrazni);
		for (j = 0; j < BROJ_J_ITERACIJA; j++)
				;
		if ( iid == M ) {
			brojac++;
			if (brojac%5 == 0) sleep(3);
		}
	}
	while (kraj != KRAJ_RADA);
	return NULL;
}

int main(int argc, char *argv[])
{
	int BR1[N], BR2[M], i;
	pthread_t t[8];
	sem_init(&sPrazni, 0, 5);
	sem_init(&sKO, 0, 1);
	sem_init(&sPuni, 0, 0);
	inicijaliziraj_generator (&gg, 0);
	procjeni_velicinu_grupe();
	for (i = 0; i < N; i++) {
		BR1[i] = i;
		pthread_create( &t[i], NULL, radna_dretva, &BR1[i]);
	}
	for (i = 0; i < M; i++) {
		BR2[i] = i + N;
		pthread_create( &t[i+N], NULL, neradna_dretva, &BR2[i]);
	}
	sleep(20);
	kraj = KRAJ_RADA;
	sem_post(&sPuni);
	sem_post(&sPuni);
	sem_post(&sPuni);
	sem_post(&sPrazni);
	sem_post(&sPrazni);
	sem_post(&sPrazni);
	sem_post(&sPrazni);
	sem_post(&sPrazni);
	obrisi_generator(&gg);
	sem_destroy(&sPrazni);
	sem_destroy(&sKO);
	sem_destroy(&sPuni);
	return 0;
}
