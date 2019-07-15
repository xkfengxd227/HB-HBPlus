/**
 *  declarations of basic operations for HBPlus
 */
#ifndef HBPLUS_H
#define HBPLUS_H
#include "common.h"
#include "cost.h"
#include "clustering.h"
#include "heap.h"

/// store the index of HBPlus into file
void HBPlus_IndexIntofile(Clustering *c, char *indexfolder, fDataSet *ds);
/// excute knn search based on HBPlus
void HBPlus_Search(Clustering *c, fDataSet *qds, int m, float alpha, float *R, float *r_centroid, char *indexfolder, int nk, DoubleIndex **knnset, Cost *cost);
/********************* basic operations *******************/
/// figure out the LB for each centroid, using R to estimate, and alpha to filter seperate hyperplane bound
void HBPlus_LowerBound(DoubleIndex *lowerbound, const float *query, const float *r_c, const float *centroid, DoubleIndex **innerLB, int ncenter, int d, int m, float alpha);
/// load data points from cluster file, with a limitation for LB
int HBPlus_ClusterFromFile(const char *filename, int num, int d, float *set, int *set_num, double lblimit);
/// generate the rotate matrc
void HBPlus_GenerateRotate(float *R, int m, int d);
/// rotate centroids with R
void HBPlus_RotateCentroid(float *r_c, float *c, float *R, int k, int d, int m);
#endif

