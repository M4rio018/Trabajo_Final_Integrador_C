
#ifndef VALIDACIONES_H
#define VALIDACIONES_H

/* Pide un numero real (con punto decimal) hasta que este entre min y max.
 * Rechaza letras, texto sobrante ("12abc"), "nan" e "inf". Si la entrada se
 * cierra (Ctrl+D) avisa y termina el programa. */
double leer_double(const char *msg, double min, double max);

/* Igual que leer_double pero para enteros. */
int leer_entero(const char *msg, int min, int max);

#endif /* VALIDACIONES_H */
