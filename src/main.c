#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include "matrix_ops.h"
#include "mpi_ops.h"


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
static inline int Inicializar_MPI(void) { return EXITO_MPI; }
static inline int Obtener_Rango_MPI(int* rango) {
   *rango = 0;
   return EXITO_MPI;
}
static inline int Obtener_Tamano_MPI(int* tamano) {
   *tamano = 1;
   return EXITO_MPI;
}
static inline int Barrera_MPI(void) { return EXITO_MPI; }
static inline int Abortar_MPI(int codigo_error) {
   exit(codigo_error);
   return EXITO_MPI;
}
static inline int Finalizar_MPI(void) { return EXITO_MPI; }


#define MPI_Init(argc, argv) Inicializar_MPI()
#define MPI_Comm_rank(comm, rank) Obtener_Rango_MPI(rank)
#define MPI_Comm_size(comm, size) Obtener_Tamano_MPI(size)
#define MPI_Barrier(comm) Barrera_MPI()
#define MPI_Abort(comm, code) Abortar_MPI(code)
#define MPI_Finalize() Finalizar_MPI()
#define MPI_Wtime() Tiempo_MPI()
#define MPI_COMM_WORLD MUNDO_MPI
#define MPI_SUCCESS EXITO_MPI
#endif


void mostrar_info_mpi(int rango, int tamano) {
   if (rango == 0) {
       printf("\n=== SISTEMA MPI ===\n");
       printf("Procesos totales: %d\n", tamano);
       #if TIENE_MPI_REAL
       printf("Implementación: MPI REAL\n");
       #else
       printf("Implementación: Modo secuencial (MPI simulado)\n");
       #endif
       printf("Proceso maestro: %d\n", rango);
   }
}


int procesar_argumentos(int argc, char* argv[], int rango) {
   int N = TAMANIO_POR_DEFECTO;


   if (argc > 1) {
       char* fin_analisis;
       N = strtol(argv[1], &fin_analisis, 10);
       if (fin_analisis == argv[1] || *fin_analisis != '\0' || N <= 0) {
           if (rango == 0) {
               fprintf(stderr, "Error: Tamaño de matriz inválido '%s'\n", argv[1]);
           }
           MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
           return -1;
       }
   }


   return N;
}


double medir_tiempo_mpi_wrapper(const double* A, const double* B, double* C, int n,
                              void (*funcion_multiplicacion)(const double*, const double*, double*, int)) {
   MPI_Barrier(MPI_COMM_WORLD);
   double inicio = MPI_Wtime();


   funcion_multiplicacion(A, B, C, n);


   MPI_Barrier(MPI_COMM_WORLD);
   return MPI_Wtime() - inicio;
}


void ejecutar_demo_paralela(int N, int rango, int tamano) {
   double tiempo_secuencial = 0.0;
   double tiempo_scatter = 0.0;
   double tiempo_bcast = 0.0;


   // 🟡 CORREGIDO: Declarar punteros aquí para todos los procesos
   double* A = NULL;
   double* B = NULL;
   double* C_secuencial = NULL;
   double* C_paralelo_scatter = NULL;
   double* C_paralelo_bcast = NULL;


   if (rango == 0) {
       printf("\n=== MULTIPLICACIÓN PARALELA MPI - SEMANA 2 ===\n");
       printf("Tamaño de matriz: %dx%d\n", N, N);
       printf("Procesos MPI: %d\n", tamano);


       A = crear_matriz(N);
       B = crear_matriz(N);
       C_secuencial = crear_matriz(N);
       C_paralelo_scatter = crear_matriz(N);
       C_paralelo_bcast = crear_matriz(N);


       if (!A || !B || !C_secuencial || !C_paralelo_scatter || !C_paralelo_bcast) {
           fprintf(stderr, "Error: Falló la asignación de memoria\n");
           MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
           return;
       }


       llenar_matriz(A, N);
       llenar_matriz(B, N);
       printf("Matrices creadas exitosamente\n");


       printf("\n🔴 EJECUTANDO MULTIPLICACIÓN SECUENCIAL...\n");
       double tiempo_inicio = MPI_Wtime();
       multiplicar_matrices_secuencial(A, B, C_secuencial, N);
       tiempo_secuencial = MPI_Wtime() - tiempo_inicio;
       printf("Tiempo secuencial: %.6f segundos\n", tiempo_secuencial);
   } else {
       // 🟡 CORREGIDO: Otros procesos NO crean matrices dummy
       // Las funciones MPI se encargarán de la memoria necesaria
       A = NULL;
       B = NULL;
       C_paralelo_scatter = NULL;
       C_paralelo_bcast = NULL;
   }


   MPI_Barrier(MPI_COMM_WORLD);


   // 🟡 CORREGIDO: Solo el proceso 0 necesita matrices de resultado
   if (rango == 0) {
       printf("\n🟡 EJECUTANDO MULTIPLICACIÓN MPI SCATTER...\n");
   }


   // Todos los procesos participan, pero solo el proceso 0 necesita el resultado
   double* C_scatter_temp = (rango == 0) ? C_paralelo_scatter : crear_matriz(1);
   tiempo_scatter = medir_tiempo_mpi_wrapper(A, B, C_scatter_temp, N, multiplicar_matrices_mpi_scatter);


   if (rango == 0) {
       printf("Tiempo MPI Scatter: %.6f segundos\n", tiempo_scatter);


       bool scatter_correcto = verificar_correccion_matriz(C_secuencial, C_paralelo_scatter, N, TOLERANCIA_VERIFICACION);
       printf("Verificación Scatter: %s\n", scatter_correcto ? "✓ EXITOSA" : "✗ FALLIDA");
   } else {
       liberar_matriz(C_scatter_temp); // 🟡 Liberar matriz temporal de otros procesos
   }


   MPI_Barrier(MPI_COMM_WORLD);


   if (rango == 0) {
       printf("\n🟡 EJECUTANDO MULTIPLICACIÓN MPI BROADCAST...\n");
   }


   double* C_bcast_temp = (rango == 0) ? C_paralelo_bcast : crear_matriz(1);
   tiempo_bcast = medir_tiempo_mpi_wrapper(A, B, C_bcast_temp, N, multiplicar_matrices_mpi_broadcast);


   if (rango == 0) {
       printf("Tiempo MPI Broadcast: %.6f segundos\n", tiempo_bcast);


       bool bcast_correcto = verificar_correccion_matriz(C_secuencial, C_paralelo_bcast, N, TOLERANCIA_VERIFICACION);
       printf("Verificación Broadcast: %s\n", bcast_correcto ? "✓ EXITOSA" : "✗ FALLIDA");


       printf("\n=== ANÁLISIS DE RENDIMIENTO ===\n");
       if (tiempo_scatter > 0 && tiempo_secuencial > 0) {
           double speedup_scatter = tiempo_secuencial / tiempo_scatter;
           printf("Speedup Scatter: %.2fx\n", speedup_scatter);
       }
       if (tiempo_bcast > 0 && tiempo_secuencial > 0) {
           double speedup_bcast = tiempo_secuencial / tiempo_bcast;
           printf("Speedup Broadcast: %.2fx\n", speedup_bcast);
       }


       if (N <= 6) {
           printf("\nMatriz A:\n");
           imprimir_matriz(A, N);
           printf("\nMatriz B:\n");
           imprimir_matriz(B, N);
           printf("\nResultado Secuencial:\n");
           imprimir_matriz(C_secuencial, N);
           printf("\nResultado MPI Scatter:\n");
           imprimir_matriz(C_paralelo_scatter, N);
       } else {
           double suma_secuencial = calcular_suma_matriz(C_secuencial, N);
           double suma_scatter = calcular_suma_matriz(C_paralelo_scatter, N);
           printf("Suma elementos - Secuencial: %.6f\n", suma_secuencial);
           printf("Suma elementos - Scatter:    %.6f\n", suma_scatter);
       }
   } else {
       liberar_matriz(C_bcast_temp); // 🟡 Liberar matriz temporal de otros procesos
   }


   // 🟡 CORREGIDO: Solo el proceso 0 libera las matrices principales
   if (rango == 0) {
       liberar_matriz(A);
       liberar_matriz(B);
       liberar_matriz(C_secuencial);
       liberar_matriz(C_paralelo_scatter);
       liberar_matriz(C_paralelo_bcast);
   }
   // 🟡 NOTA: Los otros procesos no tienen matrices que liberar (son NULL)
}


int main(int argc, char* argv[]) {
   int rango = 0;
   int tamano = 1;


   MPI_Init(&argc, &argv);
   MPI_Comm_rank(MPI_COMM_WORLD, &rango);
   MPI_Comm_size(MPI_COMM_WORLD, &tamano);


   mostrar_info_mpi(rango, tamano);


   int N = procesar_argumentos(argc, argv, rango);
   if (N == -1) {
       MPI_Finalize();
       return EXIT_FAILURE;
   }


   ejecutar_demo_paralela(N, rango, tamano);


   // 🟡 CORREGIDO: Todos los procesos deben llamar a comparar_rendimiento_mpi
   if (N >= 64) {
       if (rango == 0) {
           printf("\n=== EJECUTANDO PRUEBAS DE RENDIMIENTO ADICIONALES ===\n");
       }
       comparar_rendimiento_mpi(N);
   }


   if (rango == 0) {
       printf("\n=== SEMANA 2 COMPLETADA ===\n");
       printf("Resumen MPI Paralelo:\n");
       printf("- Implementadas 2 estrategias MPI: Scatter/Gather y Broadcast\n");
       printf("- Tamaño de matriz: %dx%d\n", N, N);
       printf("- Procesos utilizados: %d\n", tamano);
       printf("- Verificación numérica incluida\n");
       printf("- Análisis de speedup realizado\n");
   }


   MPI_Finalize();
   return EXIT_SUCCESS;
}
