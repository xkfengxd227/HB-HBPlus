#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "clustering.h"
#include "common.h"
#include "yael_v438/yael/vector.h"
#include "yael_v438/yael/kmeans.h"
#include "yael_v438/yael/machinedeps.h"

void C_Init(Clustering *c)
{
	c->ncenter = -1;
	c->niter = 100;
	c->nthread = count_cpu();
	c->seed = 0;
	c->nredo = 1;
}

void C_Delete(Clustering *c)
{
	int i;
    if(c->centroid != NULL)
    {
        free(c->centroid);
        c->centroid = NULL;
        for(i = 0; i < c->ncenter; i++)
        {
            free(c->member[i]); c->member[i] = NULL;
            free(c->innerLB[i]); c->innerLB[i] = NULL;
        }
        free(c->assign); c->assign = NULL;
        free(c->nassign); c->nassign = NULL;
        free(c->member); c->member = NULL;
        free(c->innerLB); c->innerLB = NULL;
    }
}

void C_Clustering(Clustering *c, fDataSet *ds, fDataSet *lds)
{
    int i, j;
    int d = ds->d;
    int n = ds->n;
    int n_l = lds->n;

	/// allocate storage space
	c->centroid = fvec_new_set(c->ncenter*d, 0);
	c->member = (int**)malloc(sizeof(int*)*c->ncenter);
	c->assign = ivec_new_set(n, -1);
    c->innerLB = (DoubleIndex **)malloc(sizeof(DoubleIndex*)*c->ncenter);

	/// kmeans聚类
	float quantierror = kmeans(d, n_l, c->ncenter, c->niter, lds->data, c->nthread, c->seed, c->nredo, c->centroid, NULL, NULL, NULL);
	printf(">>> clustering, quantization error: %f\n", quantierror);
	fvecs_write_txt("centroid.txt", d, c->ncenter, c->centroid);

    // update assign according to the lastest centroids
    double min_id = -1;
    double min_dis = FLOAT_MAX;
    double temp_dis;
    float *mydis = fvec_new(n);
    float *vector = fvec_new(d);
    float *centroid = fvec_new(d);
    for(i = 0; i < n; i++)
    {
        min_id = -1;
        min_dis = FLOAT_MAX;
        memcpy(vector, ds->data+i*d, sizeof(float)*d);
        for(j = 0; j < c->ncenter; j++)
        {
            memcpy(centroid, c->centroid+d*j, sizeof(float)*d);
            temp_dis = odistance(vector, centroid, d);

            if(f_bigger(min_dis, temp_dis))
            {
                min_dis = temp_dis;
                min_id = j;
            }
        }
        c->assign[i] = min_id;
        mydis[i] = min_dis;
    }
    c->nassign = ivec_new_0(c->ncenter);
    for(i = 0; i < n; i++)
    {
        c->nassign[c->assign[i]] ++;
    }

    /// test assign
    FILE *fp = fopen("assign.txt", "w");
    for(i = 0; i < n; i++)
    {
        fprintf(fp, "%d\t%f\n", c->assign[i], mydis[i]);
    }
    fclose(fp);

	/// 提取聚类成员
	extract_members(c->assign, c->nassign, c->member, n, c->ncenter);
    /// 计算聚类中每一个数据点的边界距离，并排序
    C_InnerLBDistance(c, ds);

    free(vector); vector = NULL;
    free(centroid); centroid = NULL;
    free(mydis); mydis = NULL;
}

void C_InnerLBDistance(Clustering *c, fDataSet *ds)
{
    int i, j, nci, otheri;
    int d = ds->d;
    float dis = 0;
    float *xcenter = fvec_new(d);
    float *ocenter = fvec_new(d);
    float *x = fvec_new(d);
    float *centroid_dis_map = fvec_new_0(c->ncenter*c->ncenter);

    /// 事先计算任意两个聚类中心点的距离
    for(i = 0; i < c->ncenter; i++)
    {
        memcpy(xcenter, c->centroid+i*d, sizeof(float)*d);
        for(j = 0; j <= i; j++)
        {
            memcpy(ocenter, c->centroid+j*d, sizeof(float)*d);
            dis = odistance(xcenter, ocenter, d);
            centroid_dis_map[i*c->ncenter+j] = dis;
            if(i != j)
            {
                centroid_dis_map[j*c->ncenter+i] = dis;
            }
        }
    }

    // 初始化存放InnerLBDistance的空间
    for(nci = 0; nci < c->ncenter; nci++)
    {
        c->innerLB[nci] = (DoubleIndex*)malloc(sizeof(DoubleIndex) * c->nassign[nci]);
        for(i = 0; i < c->nassign[nci]; i++)
        {
            c->innerLB[nci][i].id = -1;
            c->innerLB[nci][i].val = FLOAT_MAX;
        }
    }

    for(nci = 0; nci < c->ncenter; nci++)
    {// 第nci个聚类中心
        // 准备第nci个聚类的中心点
        memcpy(xcenter, c->centroid+nci*d, sizeof(float)*d);
        for(otheri = 0; otheri < c->ncenter; otheri++)
        {// 选定另一个聚类中心点
            memcpy(ocenter, c->centroid+otheri*d, sizeof(float)*d);
            if(otheri != nci)
            {
                for(i = 0; i < c->nassign[nci]; i++)
                {
                    memcpy(x, ds->data+c->member[nci][i]*d, sizeof(float)*d);
                    dis = (odistance_square(x, ocenter, d) - odistance_square(x, xcenter, d)) / (2*centroid_dis_map[nci*c->ncenter+otheri]);
                    if(f_bigger(c->innerLB[nci][i].val, dis))
                    {// 更新为较小的距离
                        c->innerLB[nci][i].val = dis;
                        c->innerLB[nci][i].id = c->member[nci][i];          // id is the data point
                    }
                }
            }
        }
        // 第nci个聚类按照innerLB排序
        DI_MergeSort(c->innerLB[nci], 0, c->nassign[nci]-1);
    }

    free(centroid_dis_map); centroid_dis_map = NULL;
    free(ocenter); ocenter = NULL;
    free(xcenter); xcenter = NULL;
    free(x); x = NULL;
}
