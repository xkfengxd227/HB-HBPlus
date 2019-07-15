/**
 *  declarations of basic operations for Clustering using KMeans in the yael library
 */
#ifndef CLUSTERING_H
#define CLUSTERING_H
#include "common.h"

typedef struct
{
    // ���´���
	int 				ncenter;		// ��������
	int					niter;			// �����������
	int					nthread;		// ����������������ʱ��
	int 				seed;			// �漴�����
	int 				nredo;			// kmeansִ�д�����Ĭ��Ϊ1��
	// ���½��
	float 			    *centroid;	    // ��������
	int                 *assign;        // the belonging cluster of each data
	int                 *nassign;       // number of members in each cluster
	int 				**member;		// ���ݵ�Ծ���Ĺ���

	DoubleIndex         **innerLB;      // ÿһ�����������ݵ����С�߽����
} Clustering;

/// �����ʼ��
void C_Init(Clustering *c);
/// ����ɾ������
void C_Delete(Clustering *c);
/// ���о���
void C_Clustering(Clustering *c, fDataSet *ds, fDataSet *lds);
/// �����������������ݵ��߽����С���룬����
void C_InnerLBDistance(Clustering *c, fDataSet *ds);
#endif // CLUSTERING_H
