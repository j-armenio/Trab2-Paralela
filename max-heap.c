#include "max-heap.h"
#include <stdio.h>
#include <math.h>

// para compilar:
// gcc -O3 max-heap.c -o max-heap -lm

#define MAX_HEAP_SIZE (1024*1024)

// ------- FUNCOES AUXILIARES ------- //

void printNSpaces(int n) 
{
    for (int i=0; i < n; i++) 
        printf(" ");
}

void drawHeapTree(int heap[], int size, int n_levels)
{
    int offset = 0;
    int space = (int) pow(2, n_levels-1);
    int n_elements = 1;

    for (int level=0; level < n_levels; ++level) {
        // printa todos elementos nesse nivel
        for(int i=offset; i < size && i < (offset+n_elements); i++) {
            printNSpaces(((pow(2,n_levels-1-level))*2)-2);
            printf("[%2d]", heap[i]);
            printNSpaces(((pow(2,n_levels-1-level))*2)-2);
        }
        printf("\n");

        offset += n_elements;
        space = n_elements-1;
        n_elements *= 2;
    }
}

__attribute__((always_inline)) inline void swap(int *a, int *b)
{
    int temp = *a;
    *a = *b;
    *b = temp;
}

void maxHeapify(int heap[], int size, int i)
{
    while (1) {
        int largest = i;
        int left = 2*i+1;
        int right = 2*i+2;

        if (left < size && heap[left] > heap[largest])
            largest = left;
        if (right < size && heap[right] > heap[largest])
            largest = right;
        if (largest != i) {
            swap(&heap[i], &heap[largest]);
            i = largest;
        } else {
            break;
        }
    }
}

#define parent(pos) ((pos-1)/2)

void heapifyUp(int heap[], int pos)
{
    int val = heap[pos];
    while (pos > 0 && val > heap[parent(pos)]) {
        heap[pos] = heap[parent(pos)];
        pos = parent(pos);
    }
    heap[pos] = val;
}

// ------- OPERACOES MAX-HEAP ------- //

void decreaseMax(int heap[], int size, int new_value)
{
    if (size == 0) // heap vazia
        return;

    if (heap[0] > new_value) {
        heap[0] = new_value;
        maxHeapify(heap, size, 0);
    }
}

void insert(int heap[], int *size, int element)
{
    *size += 1;
    int last = *size - 1;

    heap[last] = element;
    heapifyUp(heap, last);
}

int isMaxHeap(int heap[], int size)
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