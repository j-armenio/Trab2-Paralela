#include "max-heap.h"
#include <stdio.h>
#include <math.h>

#define MAX_HEAP_SIZE (1024*1024)

// ------- FUNCOES AUXILIARES ------- //

void printNSpaces(int n) 
{
    for (int i=0; i < n; i++) 
        printf(" ");
}

__attribute__((always_inline)) inline void swapFloat(float *a, float *b)
{
    float temp = *a;
    *a = *b;
    *b = temp;
}

__attribute__((always_inline)) inline void swapInt(int *a, int *b)
{
    int temp = *a;
    *a = *b;
    *b = temp;
}

#define parent(pos) ((pos-1)/2)

void maxHeapify(float heap[], int size, int i, int idx[])
{
    while (1) {
        int largest = i;
        int left    = 2 * i + 1;
        int right   = 2 * i + 2;

        if (left < size && heap[left] > heap[largest])
            largest = left;

        if (right < size && heap[right] > heap[largest])
            largest = right;

        if (largest != i) {
            swapFloat(&heap[i], &heap[largest]);
            swapInt(&idx[i], &idx[largest]);
            i = largest;
        } else {
            break;
        }
    }
}

void heapifyUp(float heap[], int pos, int idx[])
{
    float val = heap[pos];
    int id = idx[pos];

    while (pos > 0 && val > heap[parent(pos)]) {
        heap[pos] = heap[parent(pos)];
        idx[pos] = idx[parent(pos)];
        pos = parent(pos);
    }
    heap[pos] = val;
    idx[pos] = id;
}

// ------- OPERACOES MAX-HEAP ------- //

void decreaseMax(float heap[], int size, float new_value, int idx[], int new_idx)
{
    if (size == 0) // heap vazia
        return;

    if (heap[0] > new_value) {
        heap[0] = new_value;
        idx[0] = new_idx;
        maxHeapify(heap, size, 0, idx);
    }
}

void insert(float heap[], int *size, float element, int idx[], int new_idx)
{
    *size += 1;
    int last = *size - 1;

    heap[last] = element;
    idx[last] = new_idx;
    heapifyUp(heap, last, idx);
}

int isMaxHeap(float heap[], int size)
{
    for (int i=1; i < size; i++){
        if (heap[i] <= heap[parent(i)])
            continue;
        else {
            return 0;
        }
    }
    return 1;
}

// Constrói uma heap a partir de um vetor arbitrário de valores
void buildMaxHeap(float heap[], int size, int idx[])
{
    for (int i = size / 2 - 1; i >= 0; i--)
        maxHeapify(heap, size, i, idx);
}

// ordena os k vizinhos em ordem crescente com heapsort
void heapSortMax(float heap[], int size, int idx[])
{
    for (int end = size - 1; end > 0; --end) {
        swapFloat(&heap[0], &heap[end]);
        swapInt(&idx[0], &idx[end]);
        maxHeapify(heap, end, 0, idx);
    }
}