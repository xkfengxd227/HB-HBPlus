/**
 * Some common configs
 */

#ifndef COMMON_H
#define COMMON_H
#include <stdio.h>
#include <sys/time.h>

#define     INT_MAX 0x7FFFFFFF			// max signed integer
#define     FLOAT_MAX 9999999999.0 		// max float
#define     FLOAT_ZERO  1e-16 			// zero for float
#define     SHAKERANGE  10        		// in case the result knn not at the same position with groundtruth
#define     PAGESIZE    4096  			// page size: 4K
#define     PAGE_RAN_SE 10           	// 10 sequence pages equal 1 random page

#define     INDEX_PARA "index.para"
#define     CLUSTER_POXFIX ".cluster"

typedef enum{false, true} bool;
// dataset of float type
typedef struct
{
	int 				n;				// cardinality of the dataset
	int 				d;				// dimensionality of the dataset
	float				*data;			// data vectors
} fDataSet;

// an Index-Value pair: index is int type, value is double type
typedef struct
{
    int                 id;             // index
    double              val;            // value
} DoubleIndex;

// time diff
long timediff(struct timeval begin, struct timeval end);
// squared Euclidean distance between two d-dim vectors
float odistance_square(const float *a, const float *b, int d);
// Encludiean distance between two d-dim vectors
float odistance(const float *a, const float *b, int d);
// inner product for two vectors
float inner_product(float *a, float *b, int d);
// extract belongingness and members of the clusters, 
// and calculate the inner lowerbound distances in each cluster
void extract_members(int *assign, int *nassign, int **member, int n, int ncenter);
// compare two float number, return true when a > b
int f_bigger(float a, float b);
// open file with given format
FILE *open_file(char *filename, char *format);

/// MergeSort: sort DoubleIndex type data in ascending order according to their values
///
/// @param		di 		datas to be sort
/// @param 		l 		begin position
/// @param 		r 		ending position(including)
void DI_MergeSort(DoubleIndex *di, int l, int r);
void DI_Merge(DoubleIndex *di, int l, int m, int r);
/** load groundtruth from file
 *	format: 
 *		- it is a binary file obeying the [ivecs] format
 *			- it contains [nq] set of groundtruth results for [nq] queries
 *			- for each query, we maintain [MAXK] ids of its groundtruths
 */
void load_groundtruth(char *filename, int MAXK, int nq, DoubleIndex *groundtruth);
// verify the accuracy
float verify_knn(DoubleIndex **knnset, DoubleIndex *groundtruth, int nq, int nk, int dGT);
// calculate groundtruth for query in data
void calculate_groundtruth(float *data, float *query, int n, int d, int nq, int nk, DoubleIndex *groundtruth);
#endif
