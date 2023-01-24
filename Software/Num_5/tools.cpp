#include <string.h>
#include <grx20.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int spocitej_delku(char *soubor)
{
  FILE *infile; long int c=0; int r=0; double h;
  infile=fopen(soubor, "r");
  if (infile==NULL) { printf("\nnepodarilo se otevrit soubor\n"); exit(-2); }

  do {
          r=fscanf(infile,"%lf", &h);
          c++;

  } while ( ( r==1 ) && (c<300000) );
  fclose(infile);
  return c-1;
}



double* alokuj_vektor (long int pocet)
{ double *o;
  o=(double*)malloc(sizeof(double)*pocet);
  if(o==NULL ) { printf("\nnepodarilo se alokovat pamet\n"); exit(-2); }
  return o;
}

int nacti_vektor (double* vec, long int pocet, char *soubor)
{
  FILE *infile; long int c; long int r;

  infile=fopen(soubor, "r");
  if (infile==NULL) { printf("\nnepodarilo se otevrit soubor\n"); return(-2); }
  c=0; r=0;
  do {
          r=fscanf(infile,"%lf", &vec[c]);
          c++;

  } while ( ( r==1 ) && (c<pocet) );
  fclose(infile);
  c-=1;
  return 0;

}


void konvoluce (double *vstupni_signal, double *vystupni_signal,
                    long int delka_signalu, double *jadro, long int delka_jadra)
{
  double s;  long int i, j;


  for(i=delka_jadra-1; i<delka_signalu; i++)
  {  s=0.0;
     for(j=0; j<delka_jadra; j++) {
        //if ( (i-j)<0 ) t=vstupni_signal[0]; else t=vstupni_signal[i-j];
        s += jadro[j] * vstupni_signal[i-j];
     }
     //vystupni_signal[i-delka_jadra/2] = s;
     vystupni_signal[i] = s;

  }
}


void minmax (double *vekt, long int delka, double *min, double *max)
{ long int i;
  *max=vekt[0]; *min=vekt[0];
  for(i=1; i<delka; i++) {
    if (vekt[i]>*max) *max=vekt[i];
    if (vekt[i]<*min) *min=vekt[i];
  }
}
