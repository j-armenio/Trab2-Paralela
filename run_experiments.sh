#!/bin/bash

# Configurações
NQ=128
NP=400000
DIM=300
K=1024
REPETITIONS=3  # ✅ REDUZIR PARA TESTE - depois volta para 10

echo "Compilando..."
make clean
make

echo "=== INICIANDO EXPERIMENTOS ==="

# Experiência 1: 2 processos MPI × 8 threads = 16 núcleos
echo "EXPERIÊNCIA 1: 2 MPI × 8 threads"
for i in $(seq 1 $REPETITIONS); do
    echo "Execução $i/$REPETITIONS:"
    mpirun -np 2 ./knn_mpi_omp nq=$NQ npp=$NP d=$DIM k=$K -t=8
    echo "----------------------------------------"
done

# Experiência 2: 4 processos MPI × 4 threads = 16 núcleos  
echo "EXPERIÊNCIA 2: 4 MPI × 4 threads"
for i in $(seq 1 $REPETITIONS); do
    echo "Execução $i/$REPETITIONS:"
    mpirun -np 4 ./knn_mpi_omp nq=$NQ npp=$NP d=$DIM k=$K -t=4
    echo "----------------------------------------"
done

echo "EXPERIMENTOS CONCLUÍDOS!"
