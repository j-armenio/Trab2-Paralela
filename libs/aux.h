#ifndef AUX_H
#define AUX_H

void printMatInt(int **M, int linhas, int colunas);
void printMatFloat(float **M, int linhas, int colunas);
void geraConjuntoDeDados(float *C, int nc, int d);
float **make2D(float *buf, int rows, int D);

#endif