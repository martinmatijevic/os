#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include "slucajni_prosti_broj.h"

#define MASKA(bitova)	(-1+(1<<(bitova)))
#define UZMIBITOVE(broj,prvi,bitova)	(((broj)>>(64-(prvi)))&MASKA(bitova))
#define N	5
#define M	3

struct gmp_pomocno gg;
uint64_t velicina_grupe = 1;
uint64_t generiraj_dobar_broj (struct gmp_pomocno *g);
uint64_t zbrckanost (uint64_t x);
void procjeni_velicinu_grupe ();
pthread_mutex_t m;
pthread_cond_t rPrazni;
pthread_cond_t rPuni;

uint64_t MS[5];
uint64_t ULAZ = 0;
uint64_t IZLAZ = 0;
uint64_t BROJAC = 0;
uint64_t kraj = 0;
uint64_t KRAJ_RADA = 1;
uint64_t br_punih = 0;
uint64_t br_praznih = 5;

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
	inicijaliziraj_generator(&lg, *ID);
	do {
		x = generiraj_dobar_broj(&lg);
		pthread_mutex_lock(&m);
		while (br_praznih == 0) {
			pthread_cond_wait(&rPrazni, &m);
			if (kraj == KRAJ_RADA) return NULL;
		}
		stavi_u_MS(x);
		printf("stavio %lx\n", x);
		br_punih++;
		br_praznih--;
		pthread_cond_signal (&rPuni);
		pthread_mutex_unlock(&m);
	}
	while (kraj != KRAJ_RADA);
	obrisi_generator(&lg);
	return NULL;
}

void *neradna_dretva (void *id)
{
	uint64_t y;
	do {
		sleep(3);
		pthread_mutex_lock(&m);
		while (br_punih == 0) {
			pthread_cond_wait(&rPuni, &m);
			if (kraj == KRAJ_RADA) return NULL;
		}
		y = uzmi_iz_MS();
		printf("uzeo %lx\n", y);
		br_praznih++;
		br_punih--;
		pthread_cond_signal (&rPrazni);
		pthread_mutex_unlock(&m);
	}
	while (kraj != KRAJ_RADA);
	return NULL;
}

int main(int argc, char *argv[])
{
	int BR1[N], BR2[M], i;
	pthread_t t[8];
	pthread_mutex_init(&m, NULL);
	pthread_cond_init(&rPrazni, NULL);
	pthread_cond_init(&rPuni, NULL);
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
	pthread_cond_signal (&rPuni);
	pthread_cond_signal (&rPuni);
	pthread_cond_signal (&rPuni);
	pthread_cond_signal (&rPrazni);
	pthread_cond_signal (&rPrazni);
	pthread_cond_signal (&rPrazni);
	pthread_cond_signal (&rPrazni);
	pthread_cond_signal (&rPrazni);
	obrisi_generator(&gg);
	pthread_mutex_destroy(&m);
	pthread_cond_destroy(&rPrazni);
	pthread_cond_destroy(&rPuni);
	return 0;
}
