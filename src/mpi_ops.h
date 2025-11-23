#ifndef MPI_OPS_H
#define MPI_OPS_H


#include <stdbool.h>


// ============================================================================
// CONFIGURACIÓN
// ============================================================================
#define TOLERANCIA_VERIFICACION_MPI 1e-9


// ============================================================================
// FUNCIONES DE MULTIPLICACIÓN PARALELA CON MPI - SEMANA 2
// ============================================================================


void multiplicar_matrices_mpi_scatter(const double* A, const double* B, double* C, int n);
void multiplicar_matrices_mpi_broadcast(const double* A, const double* B, double* C, int n);


// ============================================================================
// FUNCIONES AUXILIARES MPI
// ============================================================================


bool comparar_rendimiento_mpi(int n);
void ejecutar_pruebas_rendimiento(void);


#endif
