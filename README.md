# Proyecto 2 — Multiplicación de Matrices Paralela con MPI
**Computación de Alto Rendimiento (HPC) — 2025**  
*Universidad Nacional Mayor de San Marcos*

---

## 👥 Integrantes

| Código   | Apellidos y Nombres        | Email                        |
|----------|----------------------------|------------------------------|
| 20200053 | Sánchez Villalta, Nixon    | nixon.sanchez@unmsm.edu.pe   |
| 22200136 | Ttito Carhuas, Carolhay    | carolhay.ttito@unmsm.edu.pe  |
| 21200085 | Vilca García, Wendy        | wendy.vilca@unmsm.edu.pe     |

---

## 📋 1. Resumen (Abstract)

Este proyecto implementa, analiza y compara distintos enfoques de multiplicación de matrices densas utilizando **OpenMPI** bajo el modelo de **memoria distribuida**.

- En la **Semana 1**, se construyó la versión secuencial, la validación numérica y la estructura modular del proyecto.
- En la **Semana 2**, se desarrollaron **dos estrategias de paralelización**:
    - **MPI_Scatterv + MPI_Bcast + MPI_Gatherv** → estrategia principal del laboratorio.
    - **MPI_Bcast + MPI_Reduce** → estrategia adicional para análisis comparativo.

La Semana 3 se centrará en mediciones completas de rendimiento: tiempo, speedup, eficiencia y comportamiento de escalabilidad.

---

## 🎯 2. Introducción

La multiplicación de matrices es esencial en:

- Simulaciones físicas y modelos matemáticos
- Métodos numéricos
- Ciencia de datos y machine learning
- Operaciones fundamentales de álgebra lineal

Al tener una complejidad cúbica:
\[
\mathcal{O}(N^3)
\]
la paralelización es necesaria para manejar tamaños grandes de matriz.

El proyecto explora técnicas de **descomposición de datos** y **comunicación entre procesos** usando MPI, siguiendo buenas prácticas propuestas por Gropp, Lusk, Thakur y Dongarra.

---

## 📘 3. Fundamento Teórico

### 3.1. Algoritmo de Multiplicación Matricial

\[
C[i][j] = \sum_{k=0}^{N-1} A[i][k] \cdot B[k][j]
\]

El algoritmo requiere:
- \(N^3\) multiplicaciones
- \(N^3 - N^2\) sumas

---

### 3.2. Modelo de Comunicación MPI

| Operación MPI | Función en el algoritmo                             |
|---------------|------------------------------------------------------|
| `MPI_Scatterv`| Distribución de filas de A (bloques irregulares)     |
| `MPI_Bcast`   | Difusión completa de la matriz B                     |
| `MPI_Gatherv` | Recolección ordenada de los bloques parciales de C   |
| `MPI_Reduce`  | Combinación de resultados entre procesos             |

---

## 🏗️ 4. Diseño e Implementación

### 4.1. Descomposición por Filas

La matriz A se divide entre los procesos de forma balanceada:

\[
A = [A_0, A_1, \dots, A_{p-1}]
\]

Cada proceso calcula:

\[
C_i = A_i \cdot B
\]

---

## 4.2 Estrategia 1 — **MPI_Scatterv + MPI_Bcast + MPI_Gatherv**

### Flujo:
1. **Distribución:**  
   `MPI_Scatterv` reparte porciones de \(A\), incluso si \(N \mod p \neq 0\).

2. **Difusión:**  
   `MPI_Bcast` envía la matriz \(B\) completa a todos los procesos.

3. **Cálculo local:**  
   Cada proceso computa su bloque \(C_i\).

4. **Recolección:**  
   `MPI_Gatherv` reconstruye la matriz final \(C\) en el proceso root.

### Ventajas:
- Distribución balanceada incluso en casos irregulares.
- Excelente para el patrón striping por filas.
- Escalabilidad adecuada en entornos distribuidos.

---

## 4.3 Estrategia 2 — **MPI_Bcast + MPI_Reduce** (adicional)

### Flujo:
1. `MPI_Bcast` difunde A y B completas.
2. Cada proceso calcula un subconjunto de filas.
3. `MPI_Reduce` combina los resultados finales.

### Pros y Contras:
| Ventaja              | Desventaja                          |
|----------------------|-------------------------------------|
| Fácil de implementar | Toma más memoria                    |
| Buena para pruebas   | Más comunicación que Scatter/Gather |

Se usa para comparación y análisis.

---


## 🧱 5. Estructura del Proyecto — Semana 2 


```
Proyecto_MPI_Matrices/
├── src/
│ ├── main.c # Control del programa + rendimiento
│ ├── matrix_ops.h # Funciones secuenciales
│ ├── matrix_ops.c # Multiplicación secuencial
│ ├── mpi_ops.h # Funciones MPI
│ └── mpi_ops.c # Implementación Scatter/Bcast/Gather + Reduce
├── Makefile
├── README.md
└── .gitignore
```


---


## ⚙️ 6. Compilación y Ejecución


### 6.1. Compilación
```bash
make clean && make
```


### 6.2. Ejecución
```bash
mpirun -np 4 ./matrix_multiply 4
mpirun -np 8 ./matrix_multiply 512
mpirun --oversubscribe -np 8 ./matrix_multiply 1024
```


---


## 📊 7. Resultados de Correctitud — Semana 2

En esta etapa del proyecto se verificó lo siguiente:

- ✔ **Correspondencia entre los resultados secuenciales y paralelos**, utilizando ambas estrategias MPI implementadas.
- ✔ **Tolerancia de comparación numérica:**  
  \[
  1 \times 10^{-9}
  \]
- ✔ **Validación de funcionalidad en diferentes tamaños:**  
  \[
  N = 64,\ 128,\ 256,\ 512
  \]

> La medición de **speedup**, **eficiencia** y **escalabilidad** se realizará en la **Semana 3**, una vez completada la fase final del laboratorio.

---

## 🧪 8. Avances por Semana

### ✔ Semana 1
- Implementación secuencial completa.
- Modularización del proyecto en `src/`.
- Generación y liberación segura de matrices.
- Verificación numérica con tolerancia estricta.
- `Makefile` funcional y automatizado.
- Configuración inicial del entorno MPI.

### ✔ Semana 2
- Implementación completa de la estrategia **Scatter/Bcast/Gatherv**.
- Implementación adicional de la estrategia **Broadcast + Reduce**.
- Manejo seguro de procesos sin carga (`filas_local = 0`).
- Rediseño del `main.c` para soportar pruebas paralelas y validaciones.
- Implementación de funciones de rendimiento con `MPI_Wtime`.
- Comentarios añadidos en las funciones clave del código.
- Actualización integral del README con teoría y estructura.

---

## 🧭 9. Conclusiones y Trabajo Futuro

### Conclusiones
- **`MPI_Scatterv`** permite una distribución eficiente incluso con matrices que no se dividen equitativamente entre los procesos.
- Las dos estrategias paralelas implementadas (**Scatter/Gather** y **Broadcast/Reduce**) producen resultados correctos y verificables.
- Se estableció una base sólida para realizar análisis de rendimiento en la próxima semana.

### Trabajo Futuro
- Comparación detallada de **speedup** y **eficiencia**.
- Evaluación de **escalabilidad fuerte y débil**.
- Implementación de la multiplicación **por bloques**.
- Uso de **tipos derivados MPI** para optimizar aún más la comunicación.

---

## 📚 Referencias

1. Dongarra et al. (2022). *The Singular Value Decomposition: A Tour of Its Historical Development and Current State-of-the-Art*.
2. Gropp, Lusk & Thakur (2021). *Using MPI-3: Portable Parallel Programming with the Message-Passing Interface*. MIT Press.
3. Demmel & Hoemmen (2023). *Communication-Avoiding Numerical Linear Algebra*.

