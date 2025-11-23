#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "matrix_ops.h"
#include "mpi_ops.h"


#ifdef __linux__
#include <mpi.h>
#else
// Definiciones para Windows (simuladas)
typedef int MPI_Datatype;
#define MPI_DOUBLE 0
#define MPI_INT 0
#define MPI_SUM 0


static inline int MPI_Scatterv(const void* sendbuf, const int* sendcounts, const int* displacements,
                             MPI_Datatype sendtype, void* recvbuf, int recvcount,
                             MPI_Datatype recvtype, int root, Comunicador_MPI comm) {
   if (root == 0 && recvbuf && sendbuf) {
       memcpy(recvbuf, (char*)sendbuf + displacements[0] * sizeof(double), recvcount * sizeof(double));
   }
   return 0;
}


static inline int MPI_Gatherv(const void* sendbuf, int sendcount, MPI_Datatype sendtype,
                            void* recvbuf, const int* recvcounts, const int* displacements,
                            MPI_Datatype recvtype, int root, Comunicador_MPI comm) {
   if (root == 0 && recvbuf && sendbuf) {
       memcpy((char*)recvbuf + displacements[0] * sizeof(double), sendbuf, sendcount * sizeof(double));
   }
   return 0;
}


static inline int MPI_Bcast(void* buffer, int count, MPI_Datatype datatype,
                          int root, Comunicador_MPI comm) {
   return 0;
}


static inline int MPI_Reduce(const void* sendbuf, void* recvbuf, int count,
                           MPI_Datatype datatype, int op, int root, Comunicador_MPI comm) {
   if (root == 0 && recvbuf && sendbuf) {
       memcpy(recvbuf, sendbuf, count * sizeof(double));
   }
   return 0;
}
#endif


#define TOLERANCIA_VERIFICACION 1e-9


// ============================================================================
// IMPLEMENTACIÓN SCATTER/GATHER - Distribución por filas
// ============================================================================


void multiplicar_matrices_mpi_scatter(const double* A, const double* B, double* C, int n) {
   int rango, tamano;
   MPI_Comm_rank(MPI_COMM_WORLD, &rango);
   MPI_Comm_size(MPI_COMM_WORLD, &tamano);


   // Calcular filas por proceso
   int filas_base = n / tamano;
   int filas_extra = n % tamano;
   int filas_local = filas_base + (rango < filas_extra ? 1 : 0);
   int desplazamiento = 0;


   // Calcular desplazamiento para este proceso
   for (int i = 0; i < rango; i++) {
       desplazamiento += filas_base + (i < filas_extra ? 1 : 0);
   }


   // Buffers locales
   double* A_local = NULL;
   double* B_local = NULL;
   double* C_local = NULL;


   // Solo asignar memoria si este proceso tiene trabajo que hacer
   if (filas_local > 0) {
       A_local = (double*)malloc(filas_local * n * sizeof(double));
       B_local = (double*)malloc(n * n * sizeof(double));
       C_local = (double*)calloc(filas_local * n, sizeof(double));
   }


   if ((filas_local > 0) && (!A_local || !B_local || !C_local)) {
       fprintf(stderr, "Proceso %d: Error en asignación de memoria\n", rango);
       MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
       return;
   }


   // Distribuir filas de A usando Scatterv
   int* sendcounts = NULL;
   int* displacements = NULL;


   if (rango == 0) {
       sendcounts = (int*)malloc(tamano * sizeof(int));
       displacements = (int*)malloc(tamano * sizeof(int));


       int offset = 0;
       for (int i = 0; i < tamano; i++) {
           int filas_proc = filas_base + (i < filas_extra ? 1 : 0);
           sendcounts[i] = filas_proc * n;
           displacements[i] = offset;
           offset += sendcounts[i];
       }
   }


   // Scatter de A (solo si este proceso tiene trabajo)
   if (filas_local > 0) {
       MPI_Scatterv(A, sendcounts, displacements, MPI_DOUBLE,
                   A_local, filas_local * n, MPI_DOUBLE,
                   0, MPI_COMM_WORLD);
   } else {
       // Proceso sin trabajo - participar en scatter con buffer dummy
       double dummy;
       MPI_Scatterv(A, sendcounts, displacements, MPI_DOUBLE,
                   &dummy, 0, MPI_DOUBLE, 0, MPI_COMM_WORLD);
   }


   // Broadcast de B completa a todos los procesos
   if (filas_local > 0) {
       // 🟡 CORREGIDO: Crear buffer no-const para Bcast
       double* B_temp = (double*)malloc(n * n * sizeof(double));
       if (rango == 0) {
           memcpy(B_temp, B, n * n * sizeof(double));
       }
       MPI_Bcast(B_temp, n * n, MPI_DOUBLE, 0, MPI_COMM_WORLD);
       memcpy(B_local, B_temp, n * n * sizeof(double));
       free(B_temp);
   } else {
       // Proceso sin trabajo - participar en broadcast con buffer dummy
       double dummy;
       MPI_Bcast(&dummy, 0, MPI_DOUBLE, 0, MPI_COMM_WORLD);
   }


   // Multiplicación local (solo si este proceso tiene trabajo)
   if (filas_local > 0) {
       for (int i_local = 0; i_local < filas_local; i_local++) {
           for (int j = 0; j < n; j++) {
               double suma = 0.0;
               for (int k = 0; k < n; k++) {
                   suma += A_local[i_local * n + k] * B_local[k * n + j];
               }
               C_local[i_local * n + j] = suma;
           }
       }
   }


   // Recopilar resultados con Gatherv
   if (filas_local > 0) {
       MPI_Gatherv(C_local, filas_local * n, MPI_DOUBLE,
                  C, sendcounts, displacements, MPI_DOUBLE,
                  0, MPI_COMM_WORLD);
   } else {
       // Proceso sin trabajo - participar en gather con buffer dummy
       double dummy;
       MPI_Gatherv(&dummy, 0, MPI_DOUBLE,
                  C, sendcounts, displacements, MPI_DOUBLE,
                  0, MPI_COMM_WORLD);
   }


   // Limpiar
   if (filas_local > 0) {
       free(A_local);
       free(B_local);
       free(C_local);
   }
   if (rango == 0) {
       free(sendcounts);
       free(displacements);
   }
}


// ============================================================================
// IMPLEMENTACIÓN BROADCAST - Todos tienen matrices completas
// ============================================================================


void multiplicar_matrices_mpi_broadcast(const double* A, const double* B, double* C, int n) {
   int rango, tamano;
   MPI_Comm_rank(MPI_COMM_WORLD, &rango);
   MPI_Comm_size(MPI_COMM_WORLD, &tamano);


   // Buffers locales para cada proceso
   double* A_local = (double*)malloc(n * n * sizeof(double));
   double* B_local = (double*)malloc(n * n * sizeof(double));
   double* C_local = (double*)calloc(n * n, sizeof(double));


   if (!A_local || !B_local || !C_local) {
       fprintf(stderr, "Proceso %d: Error en asignación de memoria\n", rango);
       MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
       return;
   }


   // Proceso 0 copia los datos, otros procesos reciben via broadcast
   if (rango == 0) {
       memcpy(A_local, A, n * n * sizeof(double));
       memcpy(B_local, B, n * n * sizeof(double));
   }


   // Broadcast de ambas matrices
   MPI_Bcast(A_local, n * n, MPI_DOUBLE, 0, MPI_COMM_WORLD);
   MPI_Bcast(B_local, n * n, MPI_DOUBLE, 0, MPI_COMM_WORLD);


   // Distribuir trabajo por filas
   int filas_base = n / tamano;
   int filas_extra = n % tamano;
   int inicio = 0;


   // Calcular rango de filas para este proceso
   for (int i = 0; i < rango; i++) {
       inicio += filas_base + (i < filas_extra ? 1 : 0);
   }
   int fin = inicio + filas_base + (rango < filas_extra ? 1 : 0);


   // Multiplicación de las filas asignadas
   for (int i = inicio; i < fin; i++) {
       for (int j = 0; j < n; j++) {
           double suma = 0.0;
           for (int k = 0; k < n; k++) {
               suma += A_local[i * n + k] * B_local[k * n + j];
           }
           C_local[i * n + j] = suma;
       }
   }


   // Reducir resultados al proceso 0
   MPI_Reduce(C_local, C, n * n, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);


   free(A_local);
   free(B_local);
   free(C_local);
}


// ============================================================================
// FUNCIONES AUXILIARES
// ============================================================================


double medir_tiempo_mpi_paralelo(const double* A, const double* B, double* C, int n,
                               void (*funcion_multiplicacion)(const double*, const double*, double*, int)) {
   MPI_Barrier(MPI_COMM_WORLD);
   double inicio = MPI_Wtime();


   funcion_multiplicacion(A, B, C, n);


   MPI_Barrier(MPI_COMM_WORLD);
   return MPI_Wtime() - inicio;
}


bool comparar_rendimiento_mpi(int n) {
   int rango;
   MPI_Comm_rank(MPI_COMM_WORLD, &rango);


   if (rango == 0) {
       printf("\n=== COMPARACIÓN DE RENDIMIENTO MPI - Matriz %dx%d ===\n", n, n);


       // Crear matrices de prueba
       double* A = crear_matriz(n);
       double* B = crear_matriz(n);
       double* C_scatter = crear_matriz(n);
       double* C_bcast = crear_matriz(n);
       double* C_secuencial = crear_matriz(n);


       if (!A || !B || !C_scatter || !C_bcast || !C_secuencial) {
           fprintf(stderr, "Error: No se pudieron crear matrices para prueba\n");
           return false;
       }


       llenar_matriz(A, n);
       llenar_matriz(B, n);


       // Medir tiempos
       double tiempo_secuencial = medir_tiempo_mpi_paralelo(A, B, C_secuencial, n, multiplicar_matrices_secuencial);
       double tiempo_scatter = medir_tiempo_mpi_paralelo(A, B, C_scatter, n, multiplicar_matrices_mpi_scatter);
       double tiempo_bcast = medir_tiempo_mpi_paralelo(A, B, C_bcast, n, multiplicar_matrices_mpi_broadcast);


       bool scatter_correcto = verificar_correccion_matriz(C_secuencial, C_scatter, n, TOLERANCIA_VERIFICACION);
       bool bcast_correcto = verificar_correccion_matriz(C_secuencial, C_bcast, n, TOLERANCIA_VERIFICACION);


       // Mostrar resultados
       printf("Secuencial:    %.6f segundos\n", tiempo_secuencial);
       printf("MPI Scatter:   %.6f segundos %s\n", tiempo_scatter,
              scatter_correcto ? "✓" : "✗");
       printf("MPI Broadcast: %.6f segundos %s\n", tiempo_bcast,
              bcast_correcto ? "✓" : "✗");


       // Calcular speedup
       if (tiempo_scatter > 0 && tiempo_secuencial > 0) {
           double speedup_scatter = tiempo_secuencial / tiempo_scatter;
           printf("Speedup Scatter: %.2fx\n", speedup_scatter);
       }
       if (tiempo_bcast > 0 && tiempo_secuencial > 0) {
           double speedup_bcast = tiempo_secuencial / tiempo_bcast;
           printf("Speedup Broadcast: %.2fx\n", speedup_bcast);
       }


       // Limpiar
       liberar_matriz(A);
       liberar_matriz(B);
       liberar_matriz(C_scatter);
       liberar_matriz(C_bcast);
       liberar_matriz(C_secuencial);


       return scatter_correcto && bcast_correcto;
   } else {
       // 🟡 CORREGIDO: Otros procesos participan sin crear matrices grandes
       // Usar matrices de tamaño 1 para evitar segmentation faults
       double A_dummy[1], B_dummy[1], C_dummy[1];


       // Participar en las mediciones con datos dummy
       multiplicar_matrices_mpi_scatter((const double*)A_dummy, (const double*)B_dummy, C_dummy, n);
       multiplicar_matrices_mpi_broadcast((const double*)A_dummy, (const double*)B_dummy, C_dummy, n);


       return true;
   }
}


void ejecutar_pruebas_rendimiento(void) {
   int rango;
   MPI_Comm_rank(MPI_COMM_WORLD, &rango);


   if (rango == 0) {
       printf("\n=== PRUEBAS DE RENDIMIENTO MPI ===\n");


       int tamanios[] = {64, 128, 256};
       int num_pruebas = sizeof(tamanios) / sizeof(tamanios[0]);


       for (int i = 0; i < num_pruebas; i++) {
           comparar_rendimiento_mpi(tamanios[i]);
       }
   } else {
       // Otros procesos también deben participar
       int tamanios[] = {64, 128, 256};
       int num_pruebas = sizeof(tamanios) / sizeof(tamanios[0]);


       for (int i = 0; i < num_pruebas; i++) {
           comparar_rendimiento_mpi(tamanios[i]);
       }
   }
}
