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


Este proyecto implementa y analiza la multiplicación de matrices densas mediante una estrategia de paralelización basada en *striping* por filas utilizando **OpenMPI** en un entorno de **memoria distribuida**.


Durante la **Semana 1** se desarrolló la implementación secuencial, la modularización del código, el `Makefile` y la configuración inicial de MPI.
En semanas posteriores se evaluarán *speedup*, *eficiencia* y *sobrecarga de comunicación*, empleando `MPI_Scatter`, `MPI_Bcast` y `MPI_Gather`.


---


## 🎯 2. Introducción


La multiplicación de matrices es un componente esencial en:


- Simulaciones físicas
- Álgebra lineal numérica
- Machine Learning
- Procesamiento numérico


Debido a su costo $\mathcal{O}(N^3)$, la paralelización se vuelve necesaria para tamaños grandes.
Siguiendo a **Dongarra et al. (2022)**, la optimización de operaciones de álgebra lineal continúa siendo un reto en HPC.
Este proyecto se basa también en lineamientos de **Gropp et al. (2021)**.


---


## 📘 3. Fundamento Teórico


### 3.1. Algoritmo de Multiplicación


$$
C[i][j] = \sum_{k=0}^{N-1} A[i][k] \cdot B[k][j]
$$


**Complejidad:** $\mathcal{O}(N^3)$


### 3.2. Modelo de Comunicación MPI


| Comando      | Rol                                      |
|--------------|------------------------------------------|
| `MPI_Scatter`| Distribuye filas de $A$                  |
| `MPI_Bcast`  | Transmite la matriz $B$ completa         |
| `MPI_Gather` | Recolecta los bloques parciales de $C$   |


---


## 🏗️ 4. Diseño e Implementación


### 4.1. Descomposición


$A$ se divide por filas entre los procesos:


$$
A = [A_0, A_1, \dots, A_{p-1}]
$$


Cada proceso calcula:
$$
C_i = A_i \times B
$$


### 4.2. Flujo de Control


🔹 **Proceso Raíz (rank 0)**
- Genera matrices $A$ y $B$
- Distribuye $A$ (`MPI_Scatter`)
- Difunde $B$ (`MPI_Bcast`)
- Recolecta $C$ (`MPI_Gather`)
- Valida resultados con tolerancia $10^{-9}$


🔹 **Workers**
- Reciben su porción $A_i$
- Calculan $C_i$
- Envían resultados al root


---


## 🧱 5. Estructura del Proyecto — Semana 1 (Avanzada)


```
Proyecto_MPI_Matrices/
├── src/
│   ├── main.c         # Punto de entrada + MPI_Init
│   ├── matrix_ops.h   # Prototipos
│   └── matrix_ops.c   # Implementación secuencial
├── Makefile           # Compilación automática
├── README.md          # Este archivo
└── .gitignore         # Exclusión de binarios
```


---


## ⚙️ 6. Compilación y Ejecución


### 6.1. Compilación
```bash
make
```
O manualmente:
```bash
mpicc -Wall -Wextra -O2 -std=c11 -lm -o multiplicar_matrices src/main.c src/matrix_ops.c
```


### 6.2. Ejecución
```bash
mpirun -np 4 ./multiplicar_matrices 4
mpirun -np 4 ./multiplicar_matrices 8
```


---


## 📊 7. Resultados Esperados (Semanas 2–3)


| Métrica     | Fórmula                          | Utilidad                    |
|-------------|----------------------------------|-----------------------------|
| **Speedup** | $S(p) = \dfrac{T_1}{T_p}$       | Aceleración general         |
| **Eficiencia** | $E(p) = \dfrac{S(p)}{p}$      | Uso del paralelismo         |
| **Overhead** | $T_p = T_{\text{comp}} + T_{\text{comm}}$ | Costo de comunicación |


---


## 🧪 8. Avances — Semana 1


✅ Implementación secuencial
✅ Generación aleatoria de matrices
✅ Validación numérica ($10^{-9}$)
✅ Modularización (`src/`, headers)
✅ Configuración inicial MPI
✅ Medición con `MPI_Wtime`
✅ Makefile funcional
✅ README elaborado y completo


---


## 🧭 9. Conclusiones y Trabajo Futuro


### Conclusiones
- La versión secuencial sirve como *baseline* confiable.
- La modularización facilita el desarrollo paralelo.
- MPI es adecuado para memoria distribuida y escalamiento.


### Trabajo Futuro
- Implementación paralela completa
- Versión por bloques
- Tipos derivados MPI
- Análisis de escalabilidad fuerte y débil


---


## 📚 Referencias


1. Dongarra et al. (2022). *The Singular Value Decomposition: A Tour of Its Historical Development and Current State-of-the-Art*.
2. Gropp, Lusk & Thakur (2021). *Using MPI-3: Portable Parallel Programming with the Message-Passing Interface*. MIT Press.
3. Demmel & Hoemmen (2023). *Communication-Avoiding Numerical Linear Algebra*.
