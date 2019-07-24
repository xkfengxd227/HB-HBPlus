/**
 *  declarations of basic operations for Clustering using KMeans in the yael library
 */
#ifndef CLUSTERING_H
#define CLUSTERING_H
#include "common.h"

typedef struct
{
    // clustering parameters
	int 				ncenter;		// number of cluster
	int					niter;			// number of iteration
	int					nthread;		// number of thread
	int 				seed;			// seed for random generation
	int 				nredo;			// number of repetation of clustering progress (default 1)
	
	// clustering results
	float 			    *centroid;	    // all centroid vectors
	int                 *assign;        // belongingness to clusters of all data
	int                 *nassign;       // number of members in each cluster
	int 				**member;		// members of each cluster

	DoubleIndex         **innerLB;      // inner lowerbounds for members of each cluster
} Clustering;

/// initlize a clustering
void C_Init(Clustering *c);
/// clear clustering results
void C_Delete(Clustering *c);
/// clustering
void C_Clustering(Clustering *c, fDataSet *ds, fDataSet *lds);
/// in each cluster, calculate the inner lower bounds of its member points, sort them in 
/// ascending order according to their inner lb
void C_InnerLBDistance(Clustering *c, fDataSet *ds);

#endif // CLUSTERING_H
