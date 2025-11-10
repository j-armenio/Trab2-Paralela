#!/bin/bash
#SBATCH --job-name=exp3_knn
#SBATCH --output=saida_exp3.out
#SBATCH --nodes=4
#SBATCH --ntasks-per-node=2
#SBATCH --time=00:04:00
#SBATCH --exclusive

echo "=============================================="
echo "Executando Experiência 3 (8 processos em 4 nós — 2 por nó)"
echo "=============================================="

mpirun --map-by ppr:2:node ./knn-mpi nq=128 npp=400000 d=300 k=1024

