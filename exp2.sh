#!/bin/bash
#SBATCH --job-name=exp2_knn
#SBATCH --output=saida_exp2.out
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=8
#SBATCH --time=00:04:00
#SBATCH --exclusive

echo "=============================================="
echo "Executando Experiência 2 (8 processos no mesmo nó)"
echo "=============================================="

mpirun --map-by ppr:8:node ./knn-mpi nq=128 npp=400000 d=300 k=1024

