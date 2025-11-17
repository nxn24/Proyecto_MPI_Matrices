#ifndef OPERACIONES_MATRICES_H
#define OPERACIONES_MATRICES_H


#include <stdbool.h>


// ============================================================================
// FUNCIONES BÁSICAS DE MATRICES - SEMANA 1
// ============================================================================


double* crear_matriz(int n);
void liberar_matriz(double* matriz);
void llenar_matriz(double* matriz, int n);
void imprimir_matriz(const double* matriz, int n);
void multiplicar_matrices_secuencial(const double* A, const double* B, double* C, int n);


// ============================================================================
// FUNCIONES DE VERIFICACIÓN
// ============================================================================


bool verificar_correccion_matriz(const double* C_secuencial, const double* C_paralelo, int n, double tolerancia);
double calcular_suma_matriz(const double* matriz, int n);


#endif

