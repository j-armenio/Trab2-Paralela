#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include "libs/knn.h"
#include "libs/aux.h"

// #define Q_POINTS_AMT 128
// #define P_POINTS_AMT 400000
// #define DIMENSIONS 300
// #define K_NEIGHBORS 1024

// trata o input e faz a valoração dos parametros
int parse_arg(const char *name, int def, int argc, char **argv)
{
    size_t name_lenght = strlen(name);

    for (int i=1; i < argc; ++i) {
        if (strncmp(argv[i], name, L) == 0 && argv[i][L] == '=') {
            return atoi(argv[i] + (L + 1));
        }
    }
    return def;
}

int main(int argc, char **argv)
{
    MPI_INIT(&argc, &argv);
    int rank = 0, nprocs = 1;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    // valores default
    int nq = Q_POINTS_AMT; // número de pontos em Q
    int n = P_POINTS_AMT;  // número de pontos em P
    int D = DIMENSIONS;    // número de dimensões
    int k = K_NEIGHBORS;   // tamanho do conj de vizinhos

    if (rank == 0) {
        nq  = parse_int_arg("nq",  nq,  argc, argv);
        npp = parse_int_arg("npp", npp, argc, argv);
        D   = parse_int_arg("d",   D,   argc, argv);
        k   = parse_int_arg("k",   k,   argc, argv);
        if (nq <= 0 || npp <= 0 || D <= 0 || k <= 0) {
            printf("Parâmetros inválidos.\n");
            return 1;
        }

        int params[4] = {nq, npp, D, k};
    }

    MPI_Bcast(params, 4, MPI_INT, 0, MPI_COMM_WORLD);

    nq  = params[0];
    npp = params[1];
    D   = params[2];
    k   = params[3];

    float *Q = NULL;
    float *P = NULL;

    P = (float *)malloc((size_t)n * D * sizeof(float));
    if (!P) { printf("Erro: malloc(P) falhou.\n"); return 1; }

    if (rank == 0) { // Somente o processo 0 deve gerar os conjuntos Q e P
        Q = (float *)malloc((size_t)nq * D * sizeof(float));
        if (!Q) { printf("Erro: malloc(Q) falhou.\n"); return 1; }

        geraConjuntoDeDados(P, n, D);
        geraConjuntoDeDados(Q, nq, D);
    }

    // =========== BROADCAST ===========
    // root(0) manda P para cada processo
    MPI_Bcast(P, (int)((size_t)n * D), MPI_FLOAT, 0, MPI_COMM_WORLD);
    // =================================
    
    // =========== SCATTER ===========
    // root faz a divisão de quanto de Q cada processo recebe

    // 1. root vai calcular  enviar quantos pontos de Q cada um vai processar
    int *pts_amount = NULL; // quantos elementos cada processo recebe
    int *pts_offset = NULL; // offset inicial de cada processo dentro de Q

    if (rank == 0) {
        pts_amount = (int *)malloc(nprocs * sizeof(int));
        pts_offset = (int *)malloc(nprocs * sizeof(int));
        if (!pts_amount || !pts_offset) { printf("Erro: malloc (pts_) falhou.\n"); return 1; }

        int base = nq / nprocs;
        int rem = nq % nprocs; // resto
        int offset = 0;
        for (int p=0; p < nprocs; ++p) {
            pts_amount[p] = base + (p < rem ? 1 : 0);
            pts_offset[p] = offset;
            offset += pts_amount[p]
        }
    }

    int local_nq = 0; // quantidade de pontos de Q que recebe
    MPI_SCATTER(pts_amount, 1, MPI_INT, &local_nq, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // 2. agora ele vai espalhar/enviar os dados em si de Q
    float *localQ = (float *)malloc((size_t)local_nq * D * sizeof(float));
    if (!localQ) { printf("Erro: malloc(localQ) falhou.\n"); return 1; }

    if (rank == 0) {
        int *pts_amount_f = (int *)malloc(nprocs * sizeof(int));
        int *pts_offset_f = (int *)malloc(nprocs * sizeof(int));
        if (!pts_amount_f || !pts_offset_f) { printf("Erro: malloc (pts_f) falhou.\n"); return 1; }
        
        // converte os pontos para float, para saber o tamanho que tem q mandar
        for (int p=0; p < nprocs; ++p) {
            pts_amount_f[p] = pts_amount[p] * D;
            pts_offset_f[p] = pts_offset[p] * D;
        }
        
        MPI_Scatterv(Q, pts_amount_f, pts_offset_f, MPI_FLOAT,
                    localQ, local_nq * D, MPI_FLOAT, 0, MPI_COMM_WORLD);
        
        free(pts_amount_f);
        free(pts_offset_f);
    } else {
        MPI_Scatterv(NULL, NULL, NULL, MPI_FLOAT,
                    localQ, local_nq * D, MPI_FLOAT, 0, MPI_COMM_WORLD);
    }
    // ===============================

    // =========== TESTE ===========
    // Evita muita saída: cada rank mostra poucos valores
    if (local_nq > 0) {
        float v0 = localQ[0];
        float vlast = localQ[(size_t)local_nq * D - 1];
        printf("[rank %d/%d] local_nq=%d, exemplo=(%.3f ... %.3f)\n",
               rank, nprocs, local_nq, v0, vlast);
        fflush(stdout);
    } else {
        printf("[rank %d/%d] local_nq=0\n", rank, nprocs);
        fflush(stdout);
    }
    // =================================

    // monta float** para usar na funcao knn
    float **P_data = make2D(dataP, n, D);
    float **localQ_data = make2D(localQ, local_nq, D);

    // barreira obrigatória pre-knn
    MPI_Barrier(MPI_COMM_WORLD);
    double t0 = MPI_Wtime();

    float **R = knnSequencial(Q, nq, P, n, D, k);

    double t1 = MPI_Wtime();

    // =========== GATHER ===========
    // ===============================

    if (rank == 0) {
        printf("Tempo KNN: %.f s\n", t1 - t0);
    }


    return 0;
}