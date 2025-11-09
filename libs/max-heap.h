#ifndef MAX_HEAP_H
#define MAX_HEAP_H

void decreaseMax(float heap[], int size, float new_value, int idx[], int new_idx);
void insert(float heap[], int *size, float element, int idx[], int new_idx);
int isMaxHeap(float heap[], int size);
void buildMaxHeap(float heap[], int size, int idx[]);
void heapSortMax(float heap[], int size, int idx[]);

#endif