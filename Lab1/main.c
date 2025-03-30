/**
 * @file graceful_permutations.c
 * @brief Generación de permutaciones gráciles.
 * 
 * Este programa genera y cuenta todas las permutaciones gráciles de un conjunto
 * de números del 1 al n, asegurando que las diferencias entre elementos
 * consecutivos sean únicas y válidas.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <sys/timeb.h> // Biblioteca para medir tiempo en milisegundos

#define MAX_N 50  ///< Valor máximo permitido para n
#define MIN_N 1   ///< Valor mínimo permitido para n

int permutation[MAX_N]; ///< Arreglo para almacenar la permutación actual
bool used[MAX_N] = {false}; ///< Arreglo para marcar los números usados
bool diff_used[MAX_N] = {false}; ///< Arreglo para marcar las diferencias usadas
int n; ///< Tamaño de la permutación
int count = 0; ///< Contador de permutaciones gráciles encontradas

struct timeb start_time; ///< Tiempo de inicio del programa

/**
 * @brief Obtiene el tiempo transcurrido desde el inicio del programa en milisegundos.
 * @return Tiempo en milisegundos.
 */
double get_elapsed_time() {
    struct timeb end_time;
    ftime(&end_time);
    return (end_time.time - start_time.time) * 1000.0 + (end_time.millitm - start_time.millitm);
}

/**
 * @brief Genera permutaciones gráciles de forma recursiva.
 * @param index Índice actual en la permutación.
 */
void generate_graceful(int index) {
    if (index == n) {
        count++;
        return;
    }
    
    for (int i = 1; i <= n; i++) {
        if (!used[i]) {
            if (index > 0) {
                int diff = abs(permutation[index - 1] - i);
                if (diff_used[diff] || diff >= n || diff < 1) {
                    continue;
                }
                diff_used[diff] = true;
            }
            
            permutation[index] = i;
            used[i] = true;
            
            generate_graceful(index + 1);
            
            used[i] = false;
            if (index > 0) {
                diff_used[abs(permutation[index - 1] - i)] = false;
            }
        }
    }
}

/**
 * @brief Función principal del programa.
 * @return Código de salida.
 */
int main() {
    while (true) {
        printf("Ingrese el valor de n (o 0 para salir): ");
        scanf("%d", &n);

        if (n == 0) {
            printf("Saliendo del programa.\n");
            break; // Termina el bucle si el usuario ingresa 0
        }

        if (n > MAX_N) {
            printf("El valor de n es demasiado grande. Intente nuevamente.\n");
            continue; // Pide otro valor de `n`
        }
        if (n < MIN_N) {
            printf("El valor de n es demasiado pequeño. Intente nuevamente.\n");
            continue; // Pide otro valor de `n`
        }

        ftime(&start_time); // Captura el tiempo inicial
        count = 0; // Reinicia el contador de permutaciones
        generate_graceful(0);
        double elapsed_time = get_elapsed_time();

        printf("Número de permutaciones gráciles para n = %d: %d\n", n, count);
        printf("Tiempo de ejecución: %.3f ms\n", elapsed_time); // Imprime en milisegundos
        printf("Tiempo de ejecución: %.3f s\n", elapsed_time / 1000.0); // Imprime en segundos
        printf("Tiempo de ejecución: %.3f min\n", elapsed_time / 60000.0); // Imprime en minutos
        printf("Tiempo de ejecución: %.3f h\n", elapsed_time / 3600000.0); // Imprime en horas
    }

    return 0;
}