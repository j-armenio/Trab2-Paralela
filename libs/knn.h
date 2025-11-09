#ifndef KNN_H
#define KNN_H

void printMatInt(int **M, int linhas, int colunas);
void printMatFloat(float **M, int linhas, int colunas);

float squaredDist(const float *a, const float *b, int D);
int **knn(float **Q, int nq, float **P, int n, int D, int k);

#endif