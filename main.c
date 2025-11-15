#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <omp.h>
#include "libs/knn.h"
#include "libs/aux.h"
#include "libs/verificaKNN.c"
#include "libs/max-heap.h"

#define Q_POINTS_AMT 128
#define P_POINTS_AMT 400000
#define DIMENSIONS 300
#define K_NEIGHBORS 1024

// Função auxiliar para calcular KNN de um único ponto
void knn_single_point(float *Q_point, float **P, int n, int D, int k, int *result) {
    float *heap = (float *)malloc(k * sizeof(float));
    
    // Inicializa heap com k primeiros pontos de P
    for (int j = 0; j < k; j++) {
        heap[j] = squaredDist(Q_point, P[j], D);
        result[j] = j;
    }
    
    // Constrói max-heap
    buildMaxHeap(heap, k, result);
    
    // Processa pontos restantes de P
    for (int j = k; j < n; j++) {
        float dist = squaredDist(Q_point, P[j], D);
        if (dist < heap[0]) {
            decreaseMax(heap, k, dist, result, j);
        }
    }
    
    // Ordena resultados
    heapSortMax(heap, k, result);
    
    free(heap);
}

// trata o input e faz a valoração dos parametros
int parse_arg(const char *name, int def, int argc, char **argv)
{
    size_t name_len = strlen(name);

    for (int i=1; i < argc; ++i) {
        if (strncmp(argv[i], name, name_len) == 0 && argv[i][name_len] == '=') {
            return atoi(argv[i] + (name_len + 1));
        }
    }
    return def;
}

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);
    int rank = 0, nprocs = 1;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    // =========== PARÂMETRO DE THREADS ===========
    int num_threads = 1;
    for (int i=1; i < argc; ++i) {
        if (strncmp(argv[i], "-t=", 3) == 0) {
            num_threads = atoi(argv[i] + 3);
            break;
        }
    }
    // ============================================

    // Get the name of the processor
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    MPI_Get_processor_name(processor_name, &name_len);

    // Print node info
    printf("Host %s has rank %d out of %d MPI processes\n",
           processor_name, rank, nprocs);

    // valores default
    int nq = Q_POINTS_AMT; // número de pontos em Q
    int npp = P_POINTS_AMT;  // número de pontos em P
    int D = DIMENSIONS;    // número de dimensões
    int k = K_NEIGHBORS;   // tamanho do conj de vizinhos

    int params[4];

    if (rank == 0) {
        nq  = parse_arg("nq",  nq,  argc, argv);
        npp = parse_arg("npp", npp, argc, argv);
        D   = parse_arg("d",   D,   argc, argv);
        k   = parse_arg("k",   k,   argc, argv);
        
        // Também verifica parâmetro -t no rank 0 para mostrar info
        for (int i=1; i < argc; ++i) {
            if (strncmp(argv[i], "-t=", 3) == 0) {
                num_threads = atoi(argv[i] + 3);
                break;
            }
        }
        
        printf("Configuração: %d processos MPI, %d threads por processo\n", nprocs, num_threads);
        printf("Total de núcleos: %d\n", nprocs * num_threads);
        printf("Parâmetros: nq=%d, npp=%d, D=%d, k=%d\n", nq, npp, D, k);
        
        if (nq <= 0 || npp <= 0 || D <= 0 || k <= 0) {
            printf("Parâmetros inválidos.\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        params[0] = nq; params[1] = npp; params[2] = D; params[3] = k;
    }

    MPI_Bcast(params, 4, MPI_INT, 0, MPI_COMM_WORLD);

    nq  = params[0];
    npp = params[1];
    D   = params[2];
    k   = params[3];

    const int n = npp; // só pq eu uso n daq p frente, nao npp

    int verify = 0;
    for (int i=0; i < argc; ++i) {
        if (strcmp(argv[i], "-v") == 0)
            verify = 1;
    }

    float *Q = NULL;
    float *P = NULL;

    P = (float *)malloc((size_t)n * D * sizeof(float));
    if (!P) { 
        printf("Erro: malloc(P) falhou.\n"); 
        MPI_Abort(MPI_COMM_WORLD, 1); 
    }

    if (rank == 0) { // Somente o processo 0 deve gerar os conjuntos Q e P
        Q = (float *)malloc((size_t)nq * D * sizeof(float));
        if (!Q) { 
            printf("Erro: malloc(Q) falhou.\n"); 
            MPI_Abort(MPI_COMM_WORLD, 1); 
        }

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
        if (!pts_amount || !pts_offset) { 
            printf("Erro: malloc (pts_) falhou.\n"); 
            MPI_Abort(MPI_COMM_WORLD, 1); 
        }

        int base = nq / nprocs;
        int rem = nq % nprocs; // resto
        int offset = 0;
        for (int p=0; p < nprocs; ++p) {
            pts_amount[p] = base + (p < rem ? 1 : 0);
            pts_offset[p] = offset;
            offset += pts_amount[p];
        }
    }

    int local_nq = 0; // quantidade de pontos de Q que recebe
    MPI_Scatter(pts_amount, 1, MPI_INT, &local_nq, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // 2. agora ele vai espalhar/enviar os dados em si de Q
    float *localQ = (float *)malloc((size_t)local_nq * D * sizeof(float));
    if (!localQ) { 
        printf("Erro: malloc(localQ) falhou.\n"); 
        MPI_Abort(MPI_COMM_WORLD, 1); 
    }

    if (rank == 0) {
        int *pts_amount_f = (int *)malloc(nprocs * sizeof(int));
        int *pts_offset_f = (int *)malloc(nprocs * sizeof(int));
        if (!pts_amount_f || !pts_offset_f) { 
            printf("Erro: malloc (pts_f) falhou.\n"); 
            MPI_Abort(MPI_COMM_WORLD, 1); 
        }
        
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

    // =========== EXECUÇÃO DO KNN ===========
    // monta float** para usar na funcao knn
    float **P_data = make2D(P, n, D);
    float **local_Q_data = make2D(localQ, local_nq, D);

    // barreira obrigatória pre-knn
    MPI_Barrier(MPI_COMM_WORLD);
    double t0 = MPI_Wtime();

    // MODIFICAÇÃO: KNN paralelizado com OpenMP
    int **local_R = (int **)malloc(local_nq * sizeof(int *));
    
    #pragma omp parallel num_threads(num_threads)
    {
        #pragma omp for schedule(static)
        for (int i = 0; i < local_nq; i++) {
            local_R[i] = (int *)malloc(k * sizeof(int));
            knn_single_point(local_Q_data[i], P_data, n, D, k, local_R[i]);
        }
    }

    // ===============================

    // =========== GATHER ===========
    // Reunir os valores calculados por todos nós

    // local_R é [local_nq][k] (int **), mas para ser usado no gather
    // precisa ser contiguo, então transformar em [local_nq*k]
    int local_elems = local_nq * k;
    int *local_R_cont = (int *)malloc((size_t)local_elems * sizeof(int));
    for (int i=0; i < local_nq; ++i) {
        for (int t=0; t < k; ++t)
            local_R_cont[i*k + t] = local_R[i][t];
    }

    int *pts_idx = NULL, *displs_idx = NULL, *R = NULL;
    if (rank == 0) {
        pts_idx = (int *)malloc(nprocs * sizeof(int));
        displs_idx = (int *)malloc(nprocs * sizeof(int));
        int off = 0;

        for (int p=0; p < nprocs; ++p) {
            pts_idx[p] = pts_amount[p] * k;
            displs_idx[p] = off;
            off += pts_idx[p];
        }
        R = (int *)malloc((size_t)nq * k * sizeof(int));
    }

    MPI_Gatherv(local_R_cont, local_elems, MPI_INT,
                R, pts_idx, displs_idx, MPI_INT,
                0, MPI_COMM_WORLD);
    // ===============================

    MPI_Barrier(MPI_COMM_WORLD);
    double t1 = MPI_Wtime();

    if (rank == 0) {
        printf("Tempo KNN: %.3f s\n", t1 - t0);
    }

    // Verificação caso -v em argv
    if (rank == 0 && verify == 1) {
        verificaKNN(Q, nq, P, n, D, k, R);
    }

    // libera memoria
    for (int i = 0; i < local_nq; ++i) 
        free(local_R[i]);
    free(local_R);
    free(local_R_cont);
    free(local_Q_data);
    free(P_data);
    free(localQ);
    free(P);     

    if (rank == 0) {
        free(R);
        free(pts_idx);
        free(displs_idx);
        free(Q);
        free(pts_amount);
        free(pts_offset);
    }

    MPI_Finalize();
    return 0;
}
