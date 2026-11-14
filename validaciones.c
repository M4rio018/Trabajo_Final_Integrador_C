/*
 * validaciones.c - Lectura segura de datos por teclado.
 *
 * [CORREGIDO 1] Se lee SIEMPRE una linea completa con fgets. Con scanf el codigo original:
 *   - se quedaba en bucle infinito si stdin se cerraba (EOF, Ctrl+D / entrada redirigida),
 *   - aceptaba "12abc" como 12 y dejaba "abc" en el buffer para la lectura siguiente.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

#include "validaciones.h"

/* Lee una linea completa en buf. Devuelve 0 si la entrada se cerro (EOF o error). */
static int leer_linea(const char *msg, char *buf, size_t n) {
    printf("%s", msg);
    fflush(stdout);
    if (fgets(buf, (int)n, stdin) == NULL) return 0;
    if (strchr(buf, '\n') == NULL) {            /* linea mas larga que buf: se descarta el resto */
        int c;
        while ((c = getchar()) != '\n' && c != EOF) { }
    }
    return 1;
}

double leer_double(const char *msg, double min, double max) {
    char buf[64];
    for (;;) {
        if (!leer_linea(msg, buf, sizeof buf)) {
            printf("\nEntrada cerrada. Saliendo.\n");
            exit(EXIT_FAILURE);
        }
        char *fin;
        double val = strtod(buf, &fin);
        int hay_numero = (fin != buf);
        while (isspace((unsigned char)*fin)) fin++;
        /* [CORREGIDO 2] "nan" pasaba el filtro original porque toda comparacion con NaN
         * da falso; isfinite() lo rechaza (y tambien "inf"). Ademas se exige que no
         * sobre texto despues del numero. Ojo: el separador decimal es el punto. */
        if (hay_numero && *fin == '\0' && isfinite(val) && val >= min && val <= max)
            return val;
        /* [MEJORA 6] El original repetia el prompt en silencio; ahora se explica el error. */
        printf("  Valor invalido: ingrese un numero (con punto decimal) entre %g y %g.\n", min, max);
    }
}

int leer_entero(const char *msg, int min, int max) {
    char buf[64];
    for (;;) {
        if (!leer_linea(msg, buf, sizeof buf)) {
            printf("\nEntrada cerrada. Saliendo.\n");
            exit(EXIT_FAILURE);
        }
        char *fin;
        long val = strtol(buf, &fin, 10);
        int hay_numero = (fin != buf);
        while (isspace((unsigned char)*fin)) fin++;
        if (hay_numero && *fin == '\0' && val >= min && val <= max)
            return (int)val;
        printf("  Valor invalido: ingrese un entero entre %d y %d.\n", min, max);
    }
}
