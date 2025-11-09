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

void geraConjuntoDeDadosBuf(float *C, int nc, int d)
{
    for (int i = 0; i < nc; ++i)
        for (int j = 0; j < d; ++j)
            C[i*d + j] = (float)rand() / (float)RAND_MAX;
}