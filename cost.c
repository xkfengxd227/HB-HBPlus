/**
 *  implement for cost
 */
#include "cost.h"
#include <stdio.h>

/* initialize a cost structure */
void CostInit(Cost *c)
{
    c->point = 0;
    c->page = 0;
    c->cpu = 0;
    c->io = 0;
    c->lowerbound = 0;
    c->search = 0;
}
/* cost plus */
void CostCombine(Cost *tgt, const Cost *src)
{
    tgt->point = tgt->point + src->point;
    tgt->page = tgt->page + src->page;

    tgt->cpu = tgt->cpu + src->cpu;
    tgt->io = tgt->io + src->io;
    tgt->lowerbound = tgt->lowerbound + src->lowerbound;
    tgt->search = tgt->search + src->search;
}
/* cost subtraction */
void CostSubtract(Cost *tgt, const Cost *src)
{
    tgt->point = tgt->point - src->point;
    tgt->page = tgt->page - src->page;

    tgt->cpu = tgt->cpu - src->cpu;
    tgt->io = tgt->io - src->io;
    tgt->lowerbound = tgt->lowerbound - src->lowerbound;
    tgt->search = tgt->search - src->search;
}
/* Cost multiply(division also) */
void CostMultiply(Cost *tgt, float num)
{
    tgt->point = (long)(tgt->point * num);
    tgt->page = (long)(tgt->page * num);

    tgt->cpu = (long)(tgt->cpu * num);
    tgt->io = (long)(tgt->io * num);
    tgt->lowerbound = (long)(tgt->lowerbound * num);
    tgt->search = (long)(tgt->search * num);
}
void CostDisplay(Cost c)
{
    printf("page:\t%ld\npoint:\t%ld\n", c.page, c.point);
    printf("total:\t%ldus\n", c.cpu);
    printf("\t|__lb:\t%ldus\n", c.lowerbound);
    printf("\t|__io:\t%ldus\n", c.io);
    printf("\t|__search:\t%ldus\n", c.search);
    printf("\t|__cpu:\t%ldus\n", c.cpu-c.io);
}
