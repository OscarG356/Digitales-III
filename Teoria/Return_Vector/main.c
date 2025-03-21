#include <stdio.h>

// Función para escalar el vector
void scaled_vector(int *pin, int ion, int scale, int *pout) { 
    for (int i = 0; i < ion; i++) {
        pout[i] = pin[i] * scale;  // Escalamos y guardamos en pout
    }
}

// Función para imprimir el vector en formato [x,y,z]
void print_vector(int *pout, int ion) {
    printf("[");
    for (int i = 0; i < ion; i++) {
        printf("%d", pout[i]);
        if (i < ion - 1) {
            printf(",");  // Agregamos coma entre elementos
        }
    }
    printf("]\n");
}

int main() {
    int pin[] = {8, 32, 45};  // Vector de entrada
    int ion = sizeof(pin) / sizeof(pin[0]);  // Tamaño del vector
    int scale = 2;  // Factor de escala
    int pout[ion];  // Vector de salida

    scaled_vector(pin, ion, scale, pout);  // Escalamos el vector
    print_vector(pout, ion);  // Imprimimos el vector escalado

    return 0;

      
  
  
}