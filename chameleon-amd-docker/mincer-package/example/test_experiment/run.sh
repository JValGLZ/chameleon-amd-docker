#!/bin/bash
set -e

export LD_LIBRARY_PATH=/opt/papi/lib:$LD_LIBRARY_PATH
export CPATH=/opt/papi/include:$CPATH

echo "Compiling row-major version with PAPI..."
gcc -I/opt/papi/include -L/opt/papi/lib -lpapi -o matmul_row_papi matrix_mul_rowmajor.c

echo "Compiling column-major version with PAPI..."
gcc -I/opt/papi/include -L/opt/papi/lib -lpapi -o matmul_col_papi matrix_mul_colmajor.c

echo ""
echo "=== Running Row-Major Version ==="
./matmul_row_papi

echo ""
echo "=== Running Column-Major Version ==="
./matmul_col_papi
