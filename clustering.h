/**
 *  declarations of basic operations for Clustering using KMeans in the yael library
 */
#ifndef CLUSTERING_H
#define CLUSTERING_H
#include "common.h"

typedef struct
{
    // 线下处理
	int 				ncenter;		// 聚类中心
	int					niter;			// 聚类迭代次数
	int					nthread;		// 处理器个数，聚类时用
	int 				seed;			// 随即数相关
	int 				nredo;			// kmeans执行次数（默认为1）
	// 线下结果
	float 			    *centroid;	    // 聚类中心
	int                 *assign;        // the belonging cluster of each data
	int                 *nassign;       // number of members in each cluster
	int 				**member;		// 数据点对聚类的归属

	DoubleIndex         **innerLB;      // 每一个聚类中数据点的最小边界距离
} Clustering;

/// 聚类初始化
void C_Init(Clustering *c);
/// 聚类删除数据
void C_Delete(Clustering *c);
/// 进行聚类
void C_Clustering(Clustering *c, fDataSet *ds, fDataSet *lds);
/// 计算聚类汇总所有数据到边界的最小距离，排序
void C_InnerLBDistance(Clustering *c, fDataSet *ds);
#endif // CLUSTERING_H
