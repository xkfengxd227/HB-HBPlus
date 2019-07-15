/*
 * declarations of basic operations for HB
 */
#ifndef HB_H
#define HB_H
#include "common.h"
#include "cost.h"
#include "clustering.h"

// store HB index structure into file
void HB_IndexIntoFile(const Clustering *c, const char *folder, const fDataSet *ds);
// search nk-NN for all queries in queryset based on hb
void HB_Search(const Clustering *c, const fDataSet *queryset, char *folder, int nk, DoubleIndex **knnset, Cost *cost);
// write knnset into file
void HB_KnnIntoFile(const char *filename, DoubleIndex **knnset, int nq, int nk);
/********************* basic operations *******************/
// calculate all the lower bounds between query and all clusters
void HB_LowerBound(DoubleIndex *lowerbound, const float *query, const float *centroid, DoubleIndex **innerLB, int ncenter, int d);
// load data points from cluster
void HB_ClusterFromFile(const char *filename, int num, int d, float *set, int *set_num);
#endif
