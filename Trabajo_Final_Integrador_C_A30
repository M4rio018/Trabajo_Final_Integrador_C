/*
 * ARTEMIS - Simulador de ascenso del cohete SLS
 * Lenguaje: C99
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>

#define G_TIERRA 9.80
#define LIMITE_G_TRIP 3.0
#define MAX_REGISTROS 500

typedef enum {
    ASCENSO_SRB,
    SOLO_NUCLEO,
    MOTORES_APAGADOS,
    CAIDA_LIBRE,
    APOGEO
} Fase;

typedef struct {
    double a1, t1, a2, h_km, dt;
} Mision;

typedef struct {
    double t, y, v, a, a_pico;
    Fase fase;
} Estado;

typedef struct {
    double v1, y1, v2, t2, hmax, g_max;
    int hmax_simulada;
} Resultados;

typedef struct {
    Estado registros[MAX_REGISTROS];
    int n;
} Telemetria;

typedef struct {
    Mision mision;
    Resultados analiticos;
    Resultados simulados;
    Telemetria telemetria;
    int simulada, completada, desafio_hecho, aciertos;
} Sesion;

/* Prototipos básicos */
double leer_double(const char *msg, double min, double max);
int leer_entero(const char *msg, int min, int max);
void calcular_analitico(const Mision *m, Resultados *r);
void imprimir_estado(const Estado *e);
int avanzar_turno(const Mision *m, Estado *e, double dt, Resultados *r);
int avanzar_balistico(Estado *e, double dt);

double leer_double(const char *msg, double min, double max) {
    double val;
    do {
        printf("%s", msg);
        if (scanf("%lf", &val) != 1) {
            val = min - 1;
            int c; while ((c = getchar()) != '\n' && c != EOF);
        }
    } while (val < min || val > max);
    return val;
}

int leer_entero(const char *msg, int min, int max) {
    int val;
    do {
        printf("%s", msg);
        if (scanf("%d", &val) != 1) {
            val = min - 1;
            int c; while ((c = getchar()) != '\n' && c != EOF);
        }
    } while (val < min || val > max);
    return val;
}

void calcular_analitico(const Mision *m, Resultados *r) {
    double h = m->h_km * 1000.0;
    r->v1 = m->a1 * m->t1;
    r->y1 = 0.5 * m->a1 * m->t1 * m->t1;
    
    double disc = r->v1 * r->v1 + 2.0 * m->a2 * (h - r->y1);
    if (disc < 0.0) disc = 0.0;

    r->v2 = sqrt(disc);
    r->t2 = m->t1 + (r->v2 - r->v1) / m->a2;
    r->hmax = h + (r->v2 * r->v2) / (2.0 * G_TIERRA);
    r->g_max = fmax(m->a1, m->a2) / G_TIERRA;
    r->hmax_simulada = 0;
}

void imprimir_estado(const Estado *e) {
    const char *fase_str = "Desconocida";
    switch (e->fase) {
        case ASCENSO_SRB:      fase_str = "SRB"; break;
        case SOLO_NUCLEO:      fase_str = "Nucleo"; break;
        case MOTORES_APAGADOS: fase_str = "Motores OFF"; break;
        case CAIDA_LIBRE:      fase_str = "Caida libre"; break;
        case APOGEO:           fase_str = "Apogeo"; break;
    }
    printf("%8.2f | %-12s | %10.3f | %10.2f | %8.2f\n",
           e->t, fase_str, e->y / 1000.0, e->v, e->a);
}

void mover(Estado *e, double dt) {
    e->y += e->v * dt + 0.5 * e->a * dt * dt;
    e->v += e->a * dt;
    e->t += dt;
}

int avanzar_turno(const Mision *m, Estado *e, double dt, Resultados *r) {
    int eventos = 0;
    double restante = dt;
    double objetivo = m->h_km * 1000.0;

    while (restante > 1e-7 && e->fase != MOTORES_APAGADOS) {
        if (e->fase == ASCENSO_SRB) {
            e->a = m->a1;
            if (e->t < m->t1) {
                double t_ev = m->t1 - e->t;
                if (t_ev <= restante) {
                    mover(e, t_ev);
                    restante -= t_ev;
                    e->fase = SOLO_NUCLEO;
                    e->a = m->a2;
                    if (r) { r->v1 = e->v; r->y1 = e->y; }
                    eventos |= 1;
                } else {
                    mover(e, restante);
                    restante = 0.0;
                }
            } else {
                e->fase = SOLO_NUCLEO;
            }
        } else if (e->fase == SOLO_NUCLEO) {
            e->a = m->a2;
            double t_ev = (e->v > 0) ? (-e->v + sqrt(e->v * e->v + 2.0 * e->a * (objetivo - e->y))) / e->a : 0;
            
            if (t_ev <= restante) {
                mover(e, t_ev);
                restante -= t_ev;
                e->y = objetivo;
                e->fase = MOTORES_APAGADOS;
                e->a = 0.0;
                if (r) { r->v2 = e->v; r->t2 = e->t; }
                eventos |= 2;
            } else {
                mover(e, restante);
                restante = 0.0;
            }
        }
    }
    return eventos;
}

int avanzar_balistico(Estado *e, double dt) {
    if (e->v <= 0.0) { e->v = 0.0; e->a = 0.0; e->fase = APOGEO; return 1; }
    double t_apo = e->v / G_TIERRA;
    if (t_apo <= dt) {
        mover(e, t_apo);
        e->v = 0.0; e->a = 0.0; e->fase = APOGEO;
        return 1;
    }
    e->a = -G_TIERRA;
    mover(e, dt);
    if (e->v < 0) e->v = 0;
    return 0;
}

void simular_ascenso(Sesion *s) {
    Estado e = {0.0, 0.0, 0.0, s->mision.a1, 0.0, ASCENSO_SRB};
    s->telemetria.n = 0;
    s->simulada = 0; s->completada = 0;

    printf("\n t [s] | fase         | altitud km | velocidad | acelerac.\n");
    printf("-----------------------------------------------------------\n");
    imprimir_estado(&e);

    while (e.fase != MOTORES_APAGADOS) {
        avanzar_turno(&s->mision, &e, s->mision.dt, &s->simulados);
        imprimir_estado(&e);
    }

    s->simulada = 1;
    s->completada = 1;
    printf("\n¡Altitud objetivo alcanzada!\n");
}

int main(void) {
    Sesion sesion = {
        {20.0, 120.0, 12.5, 160.0, 10.0},
        {0}, {0}, {0}, 0, 0, 0, 0
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
            case 1:
                sesion.mision.a1 = leer_double("Aceleracion a1 (1-60): ", 1, 60);
                sesion.mision.t1 = leer_double("Tiempo t1 (10-300): ", 10, 300);
                sesion.mision.a2 = leer_double("Aceleracion a2 (1-30): ", 1, 30);
                sesion.mision.h_km = leer_double("Altitud final km (1-1000): ", 1, 1000);
                sesion.mision.dt = leer_double("Paso dt (1-60): ", 1, 60);
                calcular_analitico(&sesion.mision, &sesion.analiticos);
                break;
            case 2:
                simular_ascenso(&sesion);
                break;
            case 3:
                printf("\n-- ANALISIS --\n");
                printf("v1: %.2f m/s | y1: %.3f km\n", sesion.analiticos.v1, sesion.analiticos.y1 / 1000.0);
                printf("v2: %.2f m/s | t2: %.2f s\n", sesion.analiticos.v2, sesion.analiticos.t2);
                printf("Hmax: %.3f km | Gmax: %.2f g\n", sesion.analiticos.hmax / 1000.0, sesion.analiticos.g_max);
                break;
        }
    } while (op != 0);

    return 0;
}
