/**
 *  some declaration for heap and some operations for max-heap
 */

#ifndef HEAP_H
#define HEAP_H
#include "common.h"

typedef struct
{
    int             MaxNum;           // maximum member of heap
    int             length;             // number of current members
    DoubleIndex     *elem;              // elements
} Heap;

/// basic pointer operations
void Heap_Init(Heap *h, int max_num);
void Heap_Delete(Heap *h);
int Heap_Left(int i);
int Heap_Right(int i);
int Heap_Parent(int i);
/// exchange two elements
void Heap_Exchange(Heap *h, int i, int j);
/// display a heap
void Heap_Display(Heap *h);

/**************************** operations for max-heap ******************************/
/// maintain a max-heap
void MaxHeap_Heapify(Heap *h, int i);
/// build a max-heap, or adjust into a max-heap
void MaxHeap_Build(Heap *h);
/// update value of a key(new key is larger)
void MaxHeap_IncreaseKey(Heap *h, int i, DoubleIndex *di);
/// insert a new element into the max-heap, if full, replace the max element
void MaxHeap_Insert(Heap *h, DoubleIndex *di);
/// extract the maximum element
void MaxHeap_Extract(Heap *h, DoubleIndex *di);

/**************************** operations for min-heap ******************************/
/// maintain a min-heap
void MinHeap_Heapify(Heap *h, int i);
/// build a min-heap, or adjust into a min-heap
void MinHeap_Build(Heap *h);
/// update value of a key(new key is larger)
void MinHeap_DecreaseKey(Heap *h, int i, DoubleIndex *di);
/// insert a new element into the max-heap, if full, replace the max element
void MinHeap_Insert(Heap *h, DoubleIndex *di);
/// extract the maximum element
void MinHeap_Extract(Heap *h, DoubleIndex *di);

#endif
