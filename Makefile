# ============================================================================
# COMPILATION CONFIGURATION - WEEK 2 PARALLEL MPI (C11)
# ============================================================================


CC = mpicc
CFLAGS = -Wall -Wextra -Wpedantic -O2 -std=c11 -lm
TARGET = matrix_multiply


SRC_DIR = src
SOURCES = $(SRC_DIR)/main.c $(SRC_DIR)/matrix_ops.c $(SRC_DIR)/mpi_ops.c  # 🟡 NUEVO ARCHIVO


# ============================================================================
# MAIN RULES
# ============================================================================


$(TARGET): $(SOURCES)
	@echo "Compiling Week 2 project - Parallel MPI (C11)..."
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES)
	@echo "Executable created: $(TARGET)"


# ============================================================================
# TEST AND VERIFICATION RULES - WEEK 2
# ============================================================================


clean:
	rm -f $(TARGET)


run: $(TARGET)
	@echo "Executing Week 2 parallel version with 4 processes..."
	mpirun -np 4 ./$(TARGET)


run-large: $(TARGET)
	@echo "Executing with large matrix (256x256) and 4 processes..."
	mpirun -np 4 ./$(TARGET) 256


test-comparison: $(TARGET)
	@echo "Running MPI strategy comparison tests..."
	@echo "=== Small matrix (8x8) ==="
	mpirun -np 2 ./$(TARGET) 8
	@echo "=== Medium matrix (64x64) ==="
	mpirun -np 4 ./$(TARGET) 64
	@echo "=== Large matrix (128x128) ==="
	mpirun -np 4 ./$(TARGET) 128


test-scaling: $(TARGET)
	@echo "Testing MPI scaling with different process counts..."
	@echo "=== 2 processes ==="
	mpirun -np 2 ./$(TARGET) 128
	@echo "=== 4 processes ==="
	mpirun -np 4 ./$(TARGET) 128
	@echo "=== 8 processes ==="
	mpirun -np 8 ./$(TARGET) 128


valgrind-mpi: $(TARGET)
	@echo "Running valgrind for MPI memory leak detection..."
	mpirun -np 2 valgrind --leak-check=full --error-exitcode=1 ./$(TARGET) 8


benchmark: $(TARGET)
	@echo "Running comprehensive benchmark..."
	@echo "=== Benchmark 64x64 ==="
	mpirun -np 4 ./$(TARGET) 64
	@echo "=== Benchmark 128x128 ==="
	mpirun -np 4 ./$(TARGET) 128
	@echo "=== Benchmark 256x256 ==="
	mpirun -np 4 ./$(TARGET) 256


info:
	@echo "WEEK 2 - Parallel Matrix Multiplication with MPI (C11 Standard)"
	@echo "Target: $(TARGET)"
	@echo "Flags: $(CFLAGS)"
	@echo "Features:"
	@echo "  - Two MPI strategies: Scatter/Gather and Broadcast"
	@echo "  - Performance comparison and speedup analysis"
	@echo "  - Numerical verification"
	@echo "  - Robust error handling for MPI"


.PHONY: clean run run-large test-comparison test-scaling valgrind-mpi benchmark info
