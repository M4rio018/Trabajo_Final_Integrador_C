# ARTEMIS - Simulador de ascenso del cohete SLS

Trabajo Final Integrador C - Algoritmos y Estructuras de Datos - UTN FRRe
Situacion 1: ascenso del cohete SLS con desacople de los propulsores (SRB).

Programa de consola en C99 que calcula (de forma analitica) y simula (paso a paso) el ascenso
de un cohete en dos fases y compara ambos resultados.

## Modelo fisico

1. **Fase 1 (SRB + nucleo):** aceleracion constante `a1` durante `t1` segundos. Al final: `v1 = a1*t1`, `y1 = a1*t1^2/2`.
2. **Fase 2 (solo nucleo):** aceleracion constante `a2` hasta la altitud final `h`: `v2 = sqrt(v1^2 + 2*a2*(h - y1))` y `t2 = t1 + (v2 - v1)/a2`.
3. **Fase inercial:** con los motores apagados el cohete sube hasta el apogeo: `Hmax = h + v2^2/(2*g)`, con `g = 9,80 m/s^2`.
4. **Gmax:** aceleracion de la primera fase expresada en g (`a1/g`). Si supera 3 g se muestra una advertencia.

## Arquitectura

```mermaid
flowchart TD
    main["main.c<br/>menu y ciclo principal"] --> artemis["artemis.c / artemis.h<br/>calculo, simulacion, opciones del menu"]
    main --> val["validaciones.c / validaciones.h<br/>lectura segura de datos"]
    artemis --> val
```

| Archivo | Responsabilidad |
| --- | --- |
| `artemis.h` | Constantes, `enum Fase`, `struct` (`Mision`, `Estado`, `Resultados`, `Telemetria`, `Sesion`) y prototipos |
| `artemis.c` | `calcular_analitico`, `mision_valida`, `avanzar_turno`, `avanzar_balistico`, `simular_ascenso`, `configurar_mision`, `mostrar_analisis` |
| `validaciones.h` / `validaciones.c` | `leer_double` y `leer_entero`: leen una linea con `fgets`, la convierten con `strtod`/`strtol` y rechazan entradas invalidas |
| `main.c` | Menu (opciones 1, 2, 3 y 0) |
| `Makefile` | Compilacion modular con `make` |

## Compilar y ejecutar

```
make
./artemis
```

Sin `make`: `gcc -std=c99 -Wall -Wextra -pedantic main.c artemis.c validaciones.c -o artemis -lm`
(el `-lm` es necesario por `sqrt`). En Windows se puede usar MinGW o WSL.

Para borrar los archivos compilados: `make clean`.

## Uso

Menu: **1** configurar mision, **2** simular ascenso, **3** ver analisis (con comparacion contra la simulacion), **0** salir.
Datos por defecto (los del enunciado): `a1 = 20`, `t1 = 120`, `a2 = 12,5`, altitud final `160 km`, `dt = 10`.

Resultados esperados con esos datos:

| Magnitud | Valor |
| --- | --- |
| v1 / y1 | 2400,00 m/s / 144,000 km |
| v2 / t2 | 2481,93 m/s / 126,55 s |
| Hmax | 474,286 km |
| Gmax | 2,04 g |

## Validaciones

- El menu y los datos se leen como texto: se rechazan letras, `12abc`, `nan`, `inf`, valores vacios y valores fuera de rango, y se vuelve a pedir el dato explicando el error. El separador decimal es el **punto**.
- Si los SRB solos ya superan la altitud final (por ejemplo `a1 = 60`, `t1 = 300`, `h = 1000`), la mision se rechaza y se mantiene la anterior.
- Si la entrada se cierra (Ctrl+D), el programa avisa y termina; no queda en un bucle infinito.
- La telemetria tiene un maximo de 2000 registros y se controla el limite.

## Equipo

| Integrante | Rol | Que hizo |
| --- | --- | --- |
| Emanuel Yudiche | Coordinacion y analisis de resultados | Organizo el trabajo del equipo, analizo los resultados de la simulacion y los comparo con el calculo analitico |
| Lautaro Florentin | Arquitectura general y repositorio | Definio la division en modulos (`main`, `artemis`, `validaciones`), armo el `Makefile` y administro el repositorio de GitHub |
| Daniel Echenique | Logica de la simulacion | Programo `artemis.c`: calculo analitico, `avanzar_turno`, `avanzar_balistico` y `simular_ascenso` |
| Giordano Chaine Santiago | Validaciones y menu | Programo `validaciones.c` (lectura segura de datos) y el menu de `main.c` |
| Mario Alejandro Coffa | Casos de prueba | Diseno y ejecuto los casos de prueba: caso exitoso, entradas erroneas y mision incoherente |
| [Sexto integrante - completar] | README y registro de uso de IA | Escribio este README y llevo el registro de uso de inteligencia artificial |

## Registro de uso de inteligencia artificial

Usamos inteligencia artificial como ayuda para corregir, explicar y organizar el trabajo. Para el video no se uso IA para generar voces ni avatares: las voces y las caras son las del equipo.

| N | Herramienta | Para que la usamos | Que hicimos nosotros con el resultado |
| --- | --- | --- | --- |
| 1 | Gemini | Generar la primera version del programa en un solo archivo | Fue el punto de partida: lo leimos y lo probamos antes de corregirlo |
| 2 | Claude | **Corregir el codigo inicial**: entrada de datos con `scanf` (bucle infinito con EOF y aceptaba `12abc`), `nan`, mision incoherente, corte de motores, fase inercial que nunca se simulaba y calculo del apogeo | Revisamos cada correccion y comprobamos que el caso del enunciado diera los valores esperados |
| 3 | Claude | Explicarnos las correcciones: el flujo de calculo, la mejora de configurar la mision con una copia temporal, el uso de `LIMITE_G_TRIP` y por que `mision_valida` se usa antes del analitico | Lo usamos para entender el codigo |
| 4 | Claude | Marcar en el codigo cada cambio con comentarios `[CORREGIDO n]` y `[MEJORA n]` | **Dejamos esos comentarios en el codigo para guiarnos** y para saber que cambio y por que |
| 5 | Claude | Armar una version con el "Desafio de calculo" integrado como opcion 4 del menu | La probamos, pero no la incluimos en la entrega final, que tiene las opciones 1, 2, 3 y 0 |
| 6 | Claude | Dividir el programa en modulos (`main.c`, `artemis.h/.c`, `validaciones.h/.c`) y escribir el `Makefile` | Revisamos la division y verificamos que compile con `make` |
| 7 | Claude | Ajustar el calculo de Gmax para que use la aceleracion de la primera fase (`a1/g`), como pide la consigna | Comprobamos que con los datos del enunciado da 2,04 g |
| 8 | Claude | Verificar el programa con `gcc -Wall -Wextra -pedantic`, `valgrind` y sanitizers, y con entradas erroneas (`hola`, `abc`, `12abc`, `nan`, `999`, `-5`, mision incoherente y Ctrl+D) | Repetimos las pruebas de entradas erroneas nosotros |
| 9 | Claude | Leer la consigna y explicarnos que tiene que tener el video, el repositorio y la arquitectura | Armamos el trabajo siguiendo esos requisitos |
| 10 | Claude | Armar el guion del video para seis personas, con el texto de cada uno y lo que se muestra en pantalla | Cada uno adapto el texto con sus propias palabras |
| 11 | Claude | Crear una interfaz web de demostracion con la simulacion animada (tripulacion ficticia, explosion ante datos invalidos) | Es un complemento visual: no es parte del programa en C de la entrega |
| 12 | Claude | Crear la animacion de inicio del video y los carteles animados con el nombre de cada integrante | Las usamos en la edicion del video |

## Cierre

Las marcas `[CORREGIDO n]` y `[MEJORA n]` que quedaron en los comentarios indican cada cambio respecto de la version original.

## Correcciones respecto de la version original

Las marcas `[CORREGIDO n]` y `[MEJORA n]` en los comentarios del codigo indican cada cambio. Resumen:

- Lectura de datos con `fgets` (antes `scanf`: bucle infinito con EOF y aceptaba `12abc`).
- `nan` e `inf` rechazados con `isfinite`.
- Validacion de misiones incoherentes (antes el tiempo retrocedia en la simulacion).
- Corte de motores exacto al llegar a la altitud objetivo (cuadratica).
- Fase inercial hasta el apogeo (antes nunca se simulaba) y calculo de `Hmax` corregido.
- Telemetria guardada de verdad, con limite controlado; avisos de eventos (separacion de SRB, corte de motores, apogeo).
- Gmax calculado con la aceleracion de la primera fase.
