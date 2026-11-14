/*
 * artemis.c - Calculo analitico, simulacion y opciones de menu del simulador SLS.
 *
 * Las declaraciones estan en artemis.h. La lectura de datos del usuario esta en
 * validaciones.c.
 */
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "artemis.h"
#include "validaciones.h"

/* Funciones internas de este modulo (no se exportan) */
static void mover(Estado *e, double dt);
static void registrar(Sesion *s, Estado *e);

/* ------------------------------------------------------------------ */
/* Calculo analitico                                                   */
/* ------------------------------------------------------------------ */

/* [CORREGIDO 3] El original no validaba la combinacion de datos. Si los SRB solos
 * (y1 = 1/2*a1*t1^2) ya superan la altitud final, la simulacion no corta en el objetivo:
 * llegaba a 2700 km con objetivo de 1000 km y el tiempo RETROCEDIA (300 s -> 196 s).
 * En el analitico el "if (disc < 0) disc = 0" tapaba el problema en silencio. */
int mision_valida(const Mision *m) {
    double y1 = 0.5 * m->a1 * m->t1 * m->t1;
    return y1 < m->h_km * 1000.0;
}

void calcular_analitico(const Mision *m, Resultados *r) {
    double h = m->h_km * 1000.0;
    r->v1 = m->a1 * m->t1;
    r->y1 = 0.5 * m->a1 * m->t1 * m->t1;

    double disc = r->v1 * r->v1 + 2.0 * m->a2 * (h - r->y1);
    if (disc < 0.0) disc = 0.0;   /* defensivo: con mision_valida() no deberia ocurrir */

    r->v2 = sqrt(disc);
    r->t2 = m->t1 + (r->v2 - r->v1) / m->a2;
    r->hmax = h + (r->v2 * r->v2) / (2.0 * G_TIERRA);
    /* [MEJORA 4] Gmax es la aceleracion de la PRIMERA fase expresada en g (consigna, item d),
     * por eso se usa a1 y no el mayor entre a1 y a2. Si el docente pidiera la carga real sobre
     * la tripulacion en ascenso vertical, la formula seria (a1 + G_TIERRA) / G_TIERRA. */
    r->g_max = m->a1 / G_TIERRA;   /* aceleracion de la PRIMERA fase, en g (consigna, item d) */
    r->hmax_simulada = 0;
}

/* ------------------------------------------------------------------ */
/* Simulacion                                                          */
/* ------------------------------------------------------------------ */

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

/* Movimiento con aceleracion constante durante dt (cinematica exacta, no Euler). */
static void mover(Estado *e, double dt) {
    e->y += e->v * dt + 0.5 * e->a * dt * dt;
    e->v += e->a * dt;
    e->t += dt;
}

int avanzar_turno(const Mision *m, Estado *e, double dt, Resultados *r) {
    int eventos = 0;
    double restante = dt;
    double objetivo = m->h_km * 1000.0;

    while (restante > 1e-7 && (e->fase == ASCENSO_SRB || e->fase == SOLO_NUCLEO)) {
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
        } else { /* SOLO_NUCLEO */
            e->a = m->a2;
            /* [CORREGIDO 4] El original usaba "(v > 0) ? ... : 0": con v = 0 daba t_ev = 0
             * y cortaba los motores al instante. Se resuelve siempre la cuadratica
             * y/ + v*t + a*t^2/2 = objetivo, con el discriminante acotado (nunca NaN)
             * y t_ev nunca negativo (nunca retrocede el tiempo). */
            double disc = e->v * e->v + 2.0 * e->a * (objetivo - e->y);
            if (disc < e->v * e->v) disc = e->v * e->v;
            double t_ev = (-e->v + sqrt(disc)) / e->a;
            if (t_ev < 0.0) t_ev = 0.0;

            if (t_ev <= restante) {
                mover(e, t_ev);
                restante -= t_ev;
                e->y = objetivo;                 /* elimina el error de redondeo */
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
    /* [CORREGIDO 5] En el original, cuando el apogeo caia dentro del paso, se llamaba a
     * mover() ANTES de poner e->a = -g (con a = 0): la altura final salia ~2 veces
     * v^2/(2g) y no coincidia con el analitico. Ahora se fija la aceleracion primero. */
    e->a = -G_TIERRA;
    if (t_apo <= dt) {
        mover(e, t_apo);
        e->v = 0.0; e->a = 0.0; e->fase = APOGEO;
        return 1;
    }
    mover(e, dt);
    if (e->v < 0) e->v = 0;
    return 0;
}

/* [CORREGIDO 8] Antes: telemetria.n se ponia en 0 pero nunca se guardaba ningun registro,
 * y a_pico nunca se actualizaba. Ahora cada fila impresa se guarda (con control de limite). */
static void registrar(Sesion *s, Estado *e) {
    if (e->fase == ASCENSO_SRB && e->a > e->a_pico) e->a_pico = e->a;   /* g de la primera fase */
    if (s->telemetria.n < MAX_REGISTROS)
        s->telemetria.registros[s->telemetria.n++] = *e;
    imprimir_estado(e);
}

void simular_ascenso(Sesion *s) {
    Estado e = {0.0, 0.0, 0.0, s->mision.a1, 0.0, ASCENSO_SRB};
    int eventos;

    s->telemetria.n = 0;
    s->simulada = 0; s->completada = 0;
    memset(&s->simulados, 0, sizeof s->simulados);

    printf("\n t [s] | fase         | altitud km | velocidad | acelerac.\n");
    printf("-----------------------------------------------------------\n");
    registrar(s, &e);

    while (e.fase == ASCENSO_SRB || e.fase == SOLO_NUCLEO) {
        eventos = avanzar_turno(&s->mision, &e, s->mision.dt, &s->simulados);
        /* [CORREGIDO 7] El valor de retorno (eventos) se ignoraba: ahora se avisa de la
         * separacion de SRB y del corte de motores. */
        if (eventos & 1) printf("   >> Separacion de SRB (t = %.2f s)\n", e.t);
        if (eventos & 2) printf("   >> Altitud objetivo alcanzada: corte de motores\n");
        registrar(s, &e);
    }

    /* [CORREGIDO 6] El original terminaba al apagar los motores y avanzar_balistico()
     * nunca se llamaba: nunca se simulaba la subida inercial hasta el apogeo, asi que
     * no habia forma de comparar Hmax contra el valor analitico. */
    e.fase = CAIDA_LIBRE;
    while (e.fase != APOGEO) {
        avanzar_balistico(&e, s->mision.dt);
        registrar(s, &e);
    }
    printf("   >> Apogeo alcanzado\n");

    s->simulados.hmax = e.y;
    s->simulados.hmax_simulada = 1;
    s->simulados.g_max = e.a_pico / G_TIERRA;
    s->simulada = 1;
    s->completada = 1;
    if (s->telemetria.n >= MAX_REGISTROS)
        printf("(Aviso: telemetria truncada a %d registros)\n", MAX_REGISTROS);
}

/* ------------------------------------------------------------------ */
/* Menu                                                                */
/* ------------------------------------------------------------------ */

/* [MEJORA 5] La configuracion se lee en una Mision temporal y solo se aplica si es valida;
 * antes se pisaba la mision actual aunque quedara inconsistente. Ademas se invalida la
 * simulacion anterior (sus resultados ya no corresponden a los datos nuevos). */
void configurar_mision(Sesion *s) {
    Mision m;
    m.a1   = leer_double("Aceleracion a1 (1-60): ", 1, 60);
    m.t1   = leer_double("Tiempo t1 (10-300): ", 10, 300);
    m.a2   = leer_double("Aceleracion a2 (1-30): ", 1, 30);
    m.h_km = leer_double("Altitud final km (1-1000): ", 1, 1000);
    m.dt   = leer_double("Paso dt (1-60): ", 1, 60);

    if (!mision_valida(&m)) {
        printf("\nDatos incoherentes: solo con los SRB se llegaria a %.1f km, que supera la\n"
               "altitud final de %.1f km. Baje a1 o t1, o suba la altitud final.\n"
               "Se mantiene la mision anterior.\n",
               0.5 * m.a1 * m.t1 * m.t1 / 1000.0, m.h_km);
        return;
    }
    s->mision = m;
    calcular_analitico(&s->mision, &s->analiticos);
    s->simulada = 0;
    printf("Mision configurada.\n");
}

/* [MEJORA 3] El analisis ahora compara analitico vs simulado y usa LIMITE_G_TRIP
 * (en el original la constante estaba definida pero no se usaba). */
void mostrar_analisis(const Sesion *s) {
    const Resultados *a = &s->analiticos;
    const Resultados *n = &s->simulados;

    printf("\n-- ANALISIS ANALITICO --\n");
    printf("v1: %.2f m/s | y1: %.3f km\n", a->v1, a->y1 / 1000.0);
    printf("v2: %.2f m/s | t2: %.2f s\n", a->v2, a->t2);
    printf("Hmax: %.3f km | Gmax: %.2f g\n", a->hmax / 1000.0, a->g_max);
    if (a->g_max > LIMITE_G_TRIP)
        printf("ADVERTENCIA: se supera el limite de %.1f g para la tripulacion.\n", LIMITE_G_TRIP);

    if (!s->simulada) {
        printf("\n(Ejecute la simulacion con la opcion 2 para compararla con el analitico.)\n");
        return;
    }
    printf("\n-- COMPARACION ANALITICO vs SIMULADO --\n");
    printf("%-8s | %14s | %14s | %12s\n", "Magnitud", "Analitico", "Simulado", "Diferencia");
    printf("%-8s | %14.2f | %14.2f | %12.2e\n", "v1 [m/s]", a->v1, n->v1, n->v1 - a->v1);
    printf("%-8s | %14.3f | %14.3f | %12.2e\n", "y1 [km]", a->y1 / 1000.0, n->y1 / 1000.0, (n->y1 - a->y1) / 1000.0);
    printf("%-8s | %14.2f | %14.2f | %12.2e\n", "v2 [m/s]", a->v2, n->v2, n->v2 - a->v2);
    printf("%-8s | %14.2f | %14.2f | %12.2e\n", "t2 [s]", a->t2, n->t2, n->t2 - a->t2);
    printf("%-8s | %14.3f | %14.3f | %12.2e\n", "Hmax[km]", a->hmax / 1000.0, n->hmax / 1000.0, (n->hmax - a->hmax) / 1000.0);
    printf("%-8s | %14.2f | %14.2f | %12.2e\n", "Gmax [g]", a->g_max, n->g_max, n->g_max - a->g_max);
}

