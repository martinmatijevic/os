#include <stdio.h>
#include <time.h>

#include "slucajni_prosti_broj.h"
#define MASKA(bitova)									(-1+(1<<(bitova)))
#define UZMIBITOVE(broj,prvi,bitova)	(((broj)>>(64-(prvi)))&MASKA(bitova))
struct gmp_pomocno p;
uint64_t velicina_grupe=1;
uint64_t MS[5];
uint64_t ULAZ=0;
uint64_t IZLAZ=0;

void stavi_u_MS(uint64_t broj)
{
	MS[ULAZ] = broj;
	ULAZ = (ULAZ + 1) % 5;
}

uint64_t uzmi_iz_MS()
{
	uint64_t broj = MS[IZLAZ];
	IZLAZ = (IZLAZ + 1) % 5;
	return broj;
}

uint64_t zbrckanost (uint64_t x)
{
	uint64_t z=0, i, j, b1, pn;
	for(i=0; i<64-4; i++)
	{
		b1=0;
		pn=UZMIBITOVE(x, i+4, 4);
		for(j=0; j<4; j++) if ((1<<j)&pn) b1++;
		if (b1>2) z+=b1-2;
		else if (b1<2) z+=2-b1;
	}
	return z;
}

uint64_t generiraj_dobar_broj()
{
	uint64_t najbolji_broj = 0, broj;
	uint64_t najbolja_zbrckanost = 120, z;
	uint64_t i;
	for(i=0; i<velicina_grupe; i++)
	{
		broj = daj_novi_slucajan_prosti_broj(&p);
		z = zbrckanost(broj);
		if (z < najbolja_zbrckanost)
		{
			najbolja_zbrckanost = z;
			najbolji_broj = broj;
		}
	}
	return najbolji_broj;
}

uint64_t procjeni_velicinu_grupe()
{
	uint64_t M = 1000;
	uint64_t SEKUNDI = 5;
	time_t t;
	t = time(NULL);
	uint64_t k = 0, i, brojeva_u_sekundi, broj;
	while (time(NULL)<(t+SEKUNDI))
	{
		k++;
		for(i = 0; i<M-1; i++)
		{
			broj = generiraj_dobar_broj();
			stavi_u_MS(broj);
		}
		ULAZ=0;
		IZLAZ=0;
	}
	brojeva_u_sekundi = k * M / SEKUNDI;
	velicina_grupe = brojeva_u_sekundi*2/5;
	return velicina_grupe;
}

int main(int argc, char *argv[])
{
	uint64_t broj, broj_ispisa = 0;
	time_t t=time(NULL);
	FILE *fp;
	inicijaliziraj_generator (&p, 0);
	velicina_grupe=procjeni_velicinu_grupe();
	fp=fopen("readme.txt","w");	
	while (broj_ispisa<10)
	{
		broj = generiraj_dobar_broj();
		stavi_u_MS(broj);
		if (time(NULL)!=t)
		{
			broj=uzmi_iz_MS();
			printf("%lx\n", broj);
			fprintf(fp,"%lx\n", broj);	
			broj_ispisa++;
			t = time(NULL);
		}
	}
	fclose(fp);
	obrisi_generator (&p);
	return 0;
}
