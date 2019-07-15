/**
 *  record program's costs
 *      I/O
 *      cpu runtime
 */
#ifndef COST_H
#define COST_H

/* a struct for cost */
typedef struct
{
    long    point;
    long    page;
    long    cpu;
    long    io;
    long    lowerbound;
    long    search;
} Cost;
/* initialize a cost structure */
void CostInit(Cost *c);
/* cost plus */
void CostCombine(Cost *tgt, const Cost *src);
/* cost subtraction */
void CostSubtract(Cost *tgt, const Cost *src);
/* Cost multiply(division also) */
void CostMultiply(Cost *tgt, float num);
/* display a cost */
void CostDisplay(Cost c);

#endif
