#include <stdio.h>
#include <stdlib.h>
#include "knn.h"
#include "max-heap.h"

/*
    a,b: pontos no espaço
    D: número de dimensões 
*/
float squaredDist(const float *a, const float *b, int D)
{
    float sum = 0.0f;

    for (int i=0; i < D; ++i) {
        float diff = a[i] - b[i];
        sum += diff * diff;
    }
    return sum;
}

/*
    nq = numero de pontos em Q
    n  = numero de pontos em P
    D  = número de dimensoes dos pontos
    k  = tamanho dos conjuntos de pontos vizinhos buscados
*/
int **knnSequencial(float **Q, int nq, float **P, int n, int D, int k)
{
    // aloca matriz de resultados R
    int **R = (int **)malloc(nq * sizeof(int *));
    for (int i=0; i < nq; ++i)
        R[i] = (int *)malloc(k * sizeof(int));

    // heap de distancias
    float *heap = (float *)malloc(k* sizeof(float));

    // Para cada ponto em Q[i]
    for (int i = 0; i < nq; ++i) {
        // inicializa heap com os k primeiros pontos de P
        for (int j = 0; j < k; ++j) {
            heap[j] = squaredDist(Q[i], P[j], D);
            R[i][j] = j;
        }

        // constroi a max-heap inicial
        buildMaxHeap(heap, k, R[i]);

        // Para cada ponto restante em P
        for (int j = k; j < n; ++j) {
            float dist = squaredDist(Q[i], P[j], D);

            // se encontra uma distancia menor que a maxima, ela substitui a max
            if (dist < heap[0])
                decreaseMax(heap, k, dist, R[i], j);
        }

        //ordena os k vizinhos
        heapSortMax(heap, k, R[i]);
    }

    free(heap);
    return R;
}

int **knnMPI()
{

}