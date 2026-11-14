/*
 * ARTEMIS - Simulador de ascenso del cohete SLS (Situacion 1)
 * Lenguaje: C99
 *
 * main.c: solo contiene el menu y el ciclo principal. Toda la logica esta en
 * artemis.c y la lectura de datos en validaciones.c.
 */
#include <stdio.h>

#include "artemis.h"
#include "validaciones.h"

int main(void) {
    /* [MEJORA 10] Inicializadores designados (C99): se acaba el warning por llaves y campos
     * faltantes y queda claro que valor es cada cosa. Todo lo demas queda en cero. */
    static Sesion sesion = {
        .mision = {.a1 = 20.0, .t1 = 120.0, .a2 = 12.5, .h_km = 160.0, .dt = 10.0}
    };
    calcular_analitico(&sesion.mision, &sesion.analiticos);

    int op;
    do {
        printf("\n=== ARTEMIS - SLS ===\n");
        printf("1. Configurar mision\n");
        printf("2. Simular ascenso\n");
        printf("3. Ver analisis analitico\n");
        printf("0. Salir\n");
        op = leer_entero("Opcion: ", 0, 3);

        switch (op) {
            case 1: configurar_mision(&sesion); break;
            case 2: simular_ascenso(&sesion);   break;
            case 3: mostrar_analisis(&sesion);  break;
            default: break;
        }
    } while (op != 0);

    return 0;
}
