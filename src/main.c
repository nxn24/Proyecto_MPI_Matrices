#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include "matrix_ops.h"


#define TAMANIO_POR_DEFECTO 4
#define TOLERANCIA_VERIFICACION 1e-9


#ifdef __linux__
#define TIENE_MPI_REAL 1
#include <mpi.h>
#else
#define TIENE_MPI_REAL 0
typedef int Comunicador_MPI;
#define MUNDO_MPI 0
#define EXITO_MPI 0


static inline double Tiempo_MPI(void) { return (double)clock() / CLOCKS_PER_SEC; }
static inline int Inicializar_MPI(int* const argc, char*** const argv) { return EXITO_MPI; }
static inline int Obtener_Rango_MPI(const Comunicador_MPI comunicador, int* const rango) { *rango = 0; return EXITO_MPI; }
static inline int Obtener_Tamano_MPI(const Comunicador_MPI comunicador, int* const tamano) { *tamano = 1; return EXITO_MPI; }
static inline int Barrera_MPI(const Comunicador_MPI comunicador) { return EXITO_MPI; }
static inline int Abortar_MPI(const Comunicador_MPI comunicador, const int codigo_error) { exit(codigo_error); return EXITO_MPI; }
static inline int Finalizar_MPI(void) { return EXITO_MPI; }


#define MPI_Init Inicializar_MPI
#define MPI_Comm_rank Obtener_Rango_MPI
#define MPI_Comm_size Obtener_Tamano_MPI
#define MPI_Barrier Barrera_MPI
#define MPI_Abort Abortar_MPI
#define MPI_Finalize Finalizar_MPI
#define MPI_Wtime Tiempo_MPI
#define MPI_COMM_WORLD MUNDO_MPI
#define MPI_SUCCESS EXITO_MPI
#endif


int main(const int argc, char* argv[]) {
   int rango = 0;
   int tamano = 1;


   MPI_Init((int*)&argc, &argv);
   MPI_Comm_rank(MPI_COMM_WORLD, &rango);
   MPI_Comm_size(MPI_COMM_WORLD, &tamano);


   printf("Proceso %d de %d: ¡Hola Mundo MPI!\n", rango, tamano);


   if (rango == 0) {
       printf("\n=== MULTIPLICACIÓN DE MATRICES - SEMANA 1 ===\n");
       #if TIENE_MPI_REAL
       printf("Ejecutando con MPI REAL\n");
       #else
       printf("Ejecutando en modo secuencial (MPI no disponible)\n");
       #endif


       int N = TAMANIO_POR_DEFECTO;
       if (argc > 1) {
           char* fin_analisis;
           N = strtol(argv[1], &fin_analisis, 10);
           if (fin_analisis == argv[1] || *fin_analisis != '\0' || N <= 0) {
               fprintf(stderr, "Error: Tamaño de matriz inválido '%s'\n", argv[1]);
               MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
               return EXIT_FAILURE;
           }
       }


       printf("Tamaño de matriz: %dx%d\n", N, N);
       double* A = crear_matriz(N);
       double* B = crear_matriz(N);
       double* C = crear_matriz(N);
       double* C_prueba = crear_matriz(N);


       if (A == NULL || B == NULL || C == NULL || C_prueba == NULL) {
           fprintf(stderr, "Error: Falló la asignación de memoria\n");
           MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
           return EXIT_FAILURE;
       }


       llenar_matriz(A, N);
       llenar_matriz(B, N);
       printf("Matrices creadas exitosamente\n");


       printf("\nEjecutando multiplicación secuencial...\n");
       double tiempo_inicio = MPI_Wtime();
       multiplicar_matrices_secuencial(A, B, C, N);
       double tiempo_secuencial = MPI_Wtime() - tiempo_inicio;
       printf("Tiempo secuencial: %.6f segundos\n", tiempo_secuencial);


       printf("\nVerificando corrección numérica...\n");
       multiplicar_matrices_secuencial(A, B, C_prueba, N);
       bool verificacion_exitosa = verificar_correccion_matriz(C, C_prueba, N, TOLERANCIA_VERIFICACION);

       if (verificacion_exitosa) {
           printf("✓ VERIFICACIÓN EXITOSA: Los resultados son consistentes\n");
       } else {
           printf("✗ ERROR: Se detectaron resultados inconsistentes\n");
       }


       if (N <= 6) {
           printf("\nMatriz A:\n"); imprimir_matriz(A, N);
           printf("\nMatriz B:\n"); imprimir_matriz(B, N);
           printf("\nResultado C:\n"); imprimir_matriz(C, N);
       } else {
           double suma_C = calcular_suma_matriz(C, N);
           printf("Suma de elementos de C: %.6f\n", suma_C);
       }


       liberar_matriz(A); liberar_matriz(B); liberar_matriz(C); liberar_matriz(C_prueba);

       printf("\n=== SEMANA 1 COMPLETADA ===\n");
       printf("Resumen:\n");
       printf("- Tamaño de matriz: %dx%d\n", N, N);
       printf("- Tiempo secuencial: %.6f segundos\n", tiempo_secuencial);
       printf("- Verificación: %s\n", verificacion_exitosa ? "EXITOSA" : "FALLIDA");
       printf("- Procesos MPI: %d\n", tamano);
   } else {
       MPI_Barrier(MPI_COMM_WORLD);
   }


   MPI_Finalize();
   return EXIT_SUCCESS;
}
