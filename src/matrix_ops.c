#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "matrix_ops.h"


// ============================================================================
// IMPLEMENTACIÓN BÁSICA - SEMANA 1
// ============================================================================


double* crear_matriz(int n) {
   if (n <= 0) {
       fprintf(stderr, "Error: Tamaño de matriz inválido (%d)\n", n);
       exit(EXIT_FAILURE);
   }
   return (double*)calloc(n * n, sizeof(double));
}


void liberar_matriz(double* matriz) {
   free(matriz);
}


void llenar_matriz(double* matriz, int n) {
   if (!matriz) return;

   static bool semilla_establecida = false;
   if (!semilla_establecida) {
       srand(42);
       semilla_establecida = true;
   }

   for (int i = 0; i < n * n; i++) {
       matriz[i] = (double)rand() / RAND_MAX * 100.0;
   }
}


void imprimir_matriz(const double* matriz, int n) {
   if (!matriz || n > 8) return;

   printf("Matriz %dx%d:\n", n, n);
   for (int i = 0; i < n; i++) {
       for (int j = 0; j < n; j++) {
           printf("%8.2f ", matriz[i * n + j]);
       }
       printf("\n");
   }
}


void multiplicar_matrices_secuencial(const double* A, const double* B, double* C, int n) {
   if (!A || !B || !C) return;

   memset(C, 0, n * n * sizeof(double));

   for (int i = 0; i < n; i++) {
       for (int j = 0; j < n; j++) {
           double suma = 0.0;
           for (int k = 0; k < n; k++) {
               suma += A[i * n + k] * B[k * n + j];
           }
           C[i * n + j] = suma;
       }
   }
}


double calcular_suma_matriz(const double* matriz, int n) {
   if (!matriz) return 0.0;
   double suma = 0.0;
   for (int i = 0; i < n * n; i++) suma += matriz[i];
   return suma;
}


bool verificar_correccion_matriz(const double* C_secuencial, const double* C_paralelo, int n, double tolerancia) {
   if (!C_secuencial || !C_paralelo) return false;

   for (int i = 0; i < n * n; i++) {
       if (fabs(C_secuencial[i] - C_paralelo[i]) > tolerancia) {
           return false;
       }
   }
   return true;
}
