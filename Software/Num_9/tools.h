#ifndef TOOLS_H_INCLUDED
#define TOOLS_H_INCLUDED

int spocitej_delku(char *soubor);

double* alokuj_vektor (long int pocet);


int nacti_vektor (double* vec, long int pocet, char *soubor);


void konvoluce (double *vstupni_signal, double *vystupni_signal,
                    long int delka_signalu, double *jadro, long int delka_jadra);

void minmax (double *vekt, long int delka, double *min, double *max);



#endif // TOOLS_H_INCLUDED
