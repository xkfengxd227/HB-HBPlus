/**
 *  the implementation for heap
 */
#include "heap.h"
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/// basic pointer operations
void Heap_Init(Heap *h, int max_num){
    h->MaxNum = max_num;
    h->elem = (DoubleIndex*)malloc(sizeof(DoubleIndex)*max_num);
    h->length = 0;
}
void Heap_Delete(Heap *h){
    free(h->elem);
    h->MaxNum = -1;
    h->length = -1;
}
int Heap_Left(int i)    {   return (2*i+1);     }
int Heap_Right(int i)   {   return ((i+1)*2);   }
int Heap_Parent(int i)  {   return ((i-1)/2);   }
/// exchange two elements
void Heap_Exchange(Heap *h, int i, int j){
    DoubleIndex temp;
    memcpy(&temp, &h->elem[i], sizeof(DoubleIndex));
    memcpy(&h->elem[i], &h->elem[j], sizeof(DoubleIndex));
    memcpy(&h->elem[j], &temp, sizeof(DoubleIndex));
}
/// display a heap
void Heap_Display(Heap *h)
{
    int i;
    putchar('[');
    for(i = 0; i < h->length; i++)
    {
        printf("%d:%d-%f, ", i, h->elem[i].id, h->elem[i].val);
    }
    putchar(']');
    putchar('\n');
}

/**************************** operations for max-heap ******************************/
/// maintain a max-heap
void MaxHeap_Heapify(Heap *h, int i)
{
    int l = Heap_Left(i);
    int r = Heap_Right(i);
    int large = i;
    // select the maximum element among the i-th, l, r element
    if(l < h->length && f_bigger(h->elem[l].val, h->elem[i].val))
    {
        large = l;
    }
    else
    {
        large = i;
    }
    if(r < h->length && f_bigger(h->elem[r].val, h->elem[large].val))
    {
        large = r;
    }

    if(large != i)
    {
        Heap_Exchange(h, i, large);
        MaxHeap_Heapify(h, large);
    }
}
/// build a max-heap, or adjust into a max-heap
void MaxHeap_Build(Heap *h)
{
    if(h->length <= 0)
    {
        printf("error: no elements yet");
        exit(-1);
    }
    int i;
    for(i = h->length / 2; i >= 0; i--)
    {
        MaxHeap_Heapify(h, i);
    }
}
/// update value of a key(new key is larger)
void MaxHeap_IncreaseKey(Heap *h, int i, DoubleIndex *di)
{
    if(i >= h->length){
        printf("error: index exceeds element number\n");
        exit(-1);
    }
    if(f_bigger(h->elem[i].val, di->val)){
        printf("error: increase value is small\n");
        exit(-1);
    }
    memcpy(&h->elem[i], di, sizeof(DoubleIndex));
    while(i > 0 && f_bigger(h->elem[i].val, h->elem[Heap_Parent(i)].val))
    {
        Heap_Exchange(h, i, Heap_Parent(i));
        i = Heap_Parent(i);
    }
}
/// insert a new element into the max-heap, if full, replace the max element
void MaxHeap_Insert(Heap *h, DoubleIndex *di)
{
    if(h->length < h->MaxNum)
    {// heap is not full
        h->elem[h->length].val = -FLOAT_MAX;
        h->length = h->length + 1;
        MaxHeap_IncreaseKey(h, h->length-1,  di);
    }
    else
    {// heap is full, replace the maximum element
        memcpy(&h->elem[0], di, sizeof(DoubleIndex));
        MaxHeap_Heapify(h, 0);
    }
}
/// extract the maximum element
void MaxHeap_Extract(Heap *h, DoubleIndex *di)
{
    if(h->length <= 0)
    {
        printf("error: no elements in heap\n");
        exit(-1);
    }
    memcpy(di, &h->elem[0], sizeof(DoubleIndex));
    memcpy(&h->elem[0], &h->elem[h->length-1], sizeof(DoubleIndex));
    h->length = h->length - 1;
    MaxHeap_Heapify(h, 0);
}

/**************************** operations for min-heap ******************************/
/// maintain a min-heap
void MinHeap_Heapify(Heap *h, int i)
{
    int l = Heap_Left(i);
    int r = Heap_Right(i);
    int small = i;
    if(l < h->length && f_bigger(h->elem[i].val, h->elem[l].val))
    {
        small = l;
    }
    else
    {
        small = i;
    }
    if(r < h->length && f_bigger(h->elem[small].val, h->elem[r].val))
    {
        small = r;
    }

    if(small != i)
    {
        Heap_Exchange(h, i, small);
        MinHeap_Heapify(h, small);
    }
}
/// build a min-heap, or adjust into a min-heap
void MinHeap_Build(Heap *h)
{
    if(h->length <= 0)
    {
        printf("error: no elements yet");
        exit(-1);
    }
    int i;
    for(i = h->length / 2; i >= 0; i--)
    {
        MinHeap_Heapify(h, i);
    }
}
/// update value of a key(new key is larger)
void MinHeap_DecreaseKey(Heap *h, int i, DoubleIndex *di)
{
    // make sure a smaller key value
    if(i >= h->length)
    {
        printf("error: index exceeds length\n");
        exit(-1);
    }
    if(f_bigger(di->val, h->elem[i].val))
    {
        printf("error: make sure a smaller value\n");
        exit(-1);
    }
    memcpy(&h->elem[i], di, sizeof(DoubleIndex));
    while(i > 0 && f_bigger(h->elem[Heap_Parent(i)].val, h->elem[i].val))
    {
        Heap_Exchange(h, i, Heap_Parent(i));
        i = Heap_Parent(i);
    }
}
/// insert a new element into the max-heap, if full, replace the max element
void MinHeap_Insert(Heap *h, DoubleIndex *di)
{
    if(h->length < h->MaxNum)
    {// enough space
        h->elem[h->length].val = FLOAT_MAX;
        h->length = h->length + 1;
        MinHeap_DecreaseKey(h, h->length-1, di);
    }
    else
    {// heap is full
        memcpy(&h->elem[0], di, sizeof(DoubleIndex));
        MinHeap_Heapify(h, 0);
    }
}
/// extract the maximum element
void MinHeap_Extract(Heap *h, DoubleIndex *di)
{
    if(h->length <= 0)
    {
        printf("error: no elements yet\n");
        exit(-1);
    }
    memcpy(di, &h->elem[0], sizeof(DoubleIndex));
    memcpy(&h->elem[0], &h->elem[h->length-1], sizeof(DoubleIndex));
    h->length = h->length - 1;
    MinHeap_Heapify(h, 0);
}
