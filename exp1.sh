#!/bin/bash
#SBATCH --job-name=exp1_knn
#SBATCH --output=saida_exp1.out
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --time=00:04:00
#SBATCH --exclusive


echo "=============================================="
echo "Executando Experiência 1 (1 processo MPI)"
echo "=============================================="

mpirun --map-by ppr:1:node ./knn-mpi nq=128 npp=400000 d=300 k=1024

