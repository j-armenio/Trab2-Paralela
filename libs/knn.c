#include <stdio.h>

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