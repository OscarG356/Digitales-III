#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>

#define MAX_N 50
#define MIN_N 1

int permutation[MAX_N];
bool used[MAX_N] = {false};
bool diff_used[MAX_N] = {false};
int n;
int count = 0;
time_t start_time;

double get_elapsed_time() {
    return difftime(time(NULL), start_time);
}

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

int main() {
    printf("Ingrese el valor de n: ");
    scanf("%d", &n);
    
    if (n > MAX_N) {
        printf("El valor de n es demasiado grande.\n");
        return 1;
    }
    if(n<MIN_N){
        printf("El valor de n es demasiado pequeño.\n");
        return 1;
    }
    
    start_time = time(NULL);
    generate_graceful(0);
    double elapsed_time = get_elapsed_time();
    
    printf("Número de permutaciones gráciles para n = %d: %d\n", n, count);
    printf("Tiempo de ejecución: %.6f segundos\n", elapsed_time);
    
    return 0;
}