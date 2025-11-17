# ============================================================================
# COMPILATION CONFIGURATION - WEEK 1 OPTIMIZED (C11)
# ============================================================================


CC = mpicc
CFLAGS = -Wall -Wextra -Wpedantic -O2 -std=c11 -lm
TARGET = matrix_multiply


SRC_DIR = src
SOURCES = $(SRC_DIR)/main.c $(SRC_DIR)/matrix_ops.c


# ============================================================================
# MAIN RULES
# ============================================================================


$(TARGET): $(SOURCES)
   @echo "Compiling Week 1 project (C11)..."
   $(CC) $(CFLAGS) -o $(TARGET) $(SOURCES)
   @echo "Executable created: $(TARGET)"


# ============================================================================
# TEST AND VERIFICATION RULES
# ============================================================================


clean:
   rm -f $(TARGET)


run: $(TARGET)
   @echo "Executing with 4 processes..."
   mpirun -np 4 ./$(TARGET)


test-verification: $(TARGET)
   @echo "Running verification tests..."
   @echo "=== Test with 2x2 matrix ==="
   mpirun -np 2 ./$(TARGET) 2
   @echo "=== Test with 4x4 matrix ==="
   mpirun -np 4 ./$(TARGET) 4
   @echo "=== Test with 8x8 matrix ==="
   mpirun -np 4 ./$(TARGET) 8


test-performance: $(TARGET)
   @echo "Running performance tests..."
   @echo "=== Matrix 256x256 ==="
   mpirun -np 1 ./$(TARGET) 256
   @echo "=== Matrix 512x512 ==="
   mpirun -np 1 ./$(TARGET) 512


valgrind-test: $(TARGET)
   @echo "Running valgrind for memory leak detection..."
   mpirun -np 2 valgrind --leak-check=full --error-exitcode=1 ./$(TARGET) 4


info:
   @echo "WEEK 1 - Sequential Matrix Multiplication + MPI (C11 Standard)"
   @echo "Target: $(TARGET)"
   @echo "Flags: $(CFLAGS)"
   @echo "Features: Numerical verification, Robust error handling"


.PHONY: clean run test-verification test-performance valgrind-test info
