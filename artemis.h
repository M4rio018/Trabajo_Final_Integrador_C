/*
 * artemis.h - Tipos, constantes y funciones del simulador de ascenso del SLS.
 *
 * Aca se declara TODO lo que otros modulos pueden usar de la simulacion;
 * la implementacion esta en artemis.c.
 */
#ifndef ARTEMIS_H
#define ARTEMIS_H

#define G_TIERRA 9.80
#define LIMITE_G_TRIP 3.0
/* [CORREGIDO 8] Con dt=1 s y h=1000 km se generan ~1600 filas; con 500 se desbordaba
 * el arreglo (y de hecho nunca se guardaba nada). Ademas ahora se controla el limite. */
#define MAX_REGISTROS 2000

/* Etapas del vuelo */
typedef enum {
    ASCENSO_SRB,
    SOLO_NUCLEO,
    MOTORES_APAGADOS,
    CAIDA_LIBRE,
    APOGEO
} Fase;

/* Datos que ingresa el usuario */
typedef struct {
    double a1, t1, a2, h_km, dt;
} Mision;

/* Estado del cohete en un instante */
typedef struct {
    double t, y, v, a, a_pico;
    Fase fase;
} Estado;

/* Resultados (se usa tanto para el calculo analitico como para el simulado) */
typedef struct {
    double v1, y1, v2, t2, hmax, g_max;
    int hmax_simulada;
} Resultados;

/* Registro de cada fila de la simulacion */
typedef struct {
    Estado registros[MAX_REGISTROS];
    int n;
} Telemetria;

/* Todo el estado del programa */
typedef struct {
    Mision mision;
    Resultados analiticos;
    Resultados simulados;
    Telemetria telemetria;
    int simulada, completada;
} Sesion;

/* Calculo analitico y validacion de la mision */
int  mision_valida(const Mision *m);
void calcular_analitico(const Mision *m, Resultados *r);

/* Simulacion (los punteros permiten modificar el Estado directamente, sin copiarlo) */
void imprimir_estado(const Estado *e);
int  avanzar_turno(const Mision *m, Estado *e, double dt, Resultados *r);
int  avanzar_balistico(Estado *e, double dt);
void simular_ascenso(Sesion *s);

/* Opciones del menu */
void configurar_mision(Sesion *s);
void mostrar_analisis(const Sesion *s);

#endif /* ARTEMIS_H */
