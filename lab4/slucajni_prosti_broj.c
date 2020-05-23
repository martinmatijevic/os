#include <stdio.h>
#include <gmp.h>
#include <time.h>

#include "slucajni_prosti_broj.h"

void inicijaliziraj_generator(struct gmp_pomocno *p, unsigned id)
{
	mpz_init (p->slucajan_broj);
	mpz_init (p->prosti_broj);
	gmp_randinit_default (p->stanje);
	gmp_randseed_ui (p->stanje, (unsigned long int) time(NULL) + id);
}

void obrisi_generator(struct gmp_pomocno *p)
{
	mpz_clear(p->slucajan_broj);
	mpz_clear(p->prosti_broj);
	gmp_randclear (p->stanje);
}

uint64_t daj_novi_slucajan_prosti_broj (struct gmp_pomocno *p)
{
	uint64_t broj;
	mpz_urandomb (p->slucajan_broj, p->stanje, sizeof(uint64_t)*8);
	mpz_nextprime (p->prosti_broj, p->slucajan_broj);
	mpz_export (&broj, NULL, 0, sizeof(uint64_t), 0, 0, p->prosti_broj);
	return broj;
}
