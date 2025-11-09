#include <stdio.h>
#include <stdlib.h>
#include "aux.h"

void printMatInt(int **M, int linhas, int colunas)
{
    for (int i = 0; i < linhas; i++) {
        for (int j = 0; j < colunas; j++)
            printf("%d ", M[i][j]);
        printf("\n");
    }
}

void printMatFloat(float **M, int linhas, int colunas)
{
    for (int i = 0; i < linhas; i++) {
        for (int j = 0; j < colunas; j++)
            printf("%f ", M[i][j]);
        printf("\n");
    }
}

void geraConjuntoDeDados(float *C, int nc, int d)
{
    for (int i = 0; i < nc; ++i)
        for (int j = 0; j < d; ++j)
            C[i*d + j] = (float)rand() / (float)RAND_MAX;
}

// Cria um vetor de ponteiros 2D apontando pra um buf contiguo
float **make2D(float *buf, int rows, int D) 
{
    float **M = (float **)malloc(rows * sizeof(float *));
    for (int i=0; i < rows; ++i)
        M[i] = &buf[(size_t)i * D];
    
    return M;
}