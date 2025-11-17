cmake_minimum_required(VERSION 3.20)
project(Proyecto_MPI_Matrices C)


set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)


# Para compilación en Windows, usar compilador normal
if(WIN32)
    message(STATUS "Windows detected - using standard C compiler")
else()
    # En WSL/Linux, buscar MPI
    find_package(MPI)
    if(MPI_FOUND)
        message(STATUS "MPI found: ${MPI_VERSION}")
        set(CMAKE_C_COMPILER ${MPI_C_COMPILER})
        include_directories(${MPI_C_INCLUDE_PATH})
    else()
        message(STATUS "MPI not found - using sequential version")
    endif()
endif()


# Crear ejecutable
add_executable(matrix_multiply src/main.c src/matrix_ops.c)


# Enlazar con MPI si está disponible
if(MPI_FOUND)
    target_link_libraries(matrix_multiply ${MPI_C_LIBRARIES})
    message(STATUS "Linking with MPI libraries")
endif()


# Configurar flags de compilación
target_compile_options(matrix_multiply PRIVATE -Wall -Wextra -O2)

