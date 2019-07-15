/**
 *  implement of basic operations for HBPlus
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "yael_v438/yael/vector.h"
#include <assert.h>
#include <math.h>
#include "HBPlus.h"
#include "common.h"
#include "cost.h"
#include "clustering.h"
#include "heap.h"

void HBPlus_IndexIntofile(Clustering *c, char *indexfolder, fDataSet *ds)
{
    char filename[255] = {'\0'};
    FILE *fp;
    int nci, di, i;
    int d = ds->d;

    // store the parameter of index(center, inner distance, member count)
    sprintf(filename, "%s/%s", indexfolder, INDEX_PARA);
    fp = open_file(filename, "w");
    for(nci = 0; nci < c->ncenter; nci++)
    {
        fprintf(fp, "%f\t%d", c->innerLB[nci][0].val, c->nassign[nci]);          // inner lowerbound, member number
        for(di = 0; di < d; di++)
        {                                                                   // centroids
            fprintf(fp, "\t%f", c->centroid[nci*d+di]);
        }
        fputc('\n', fp);
    }
    fclose(fp);

    // store all clusters
    for(nci = 0; nci < c->ncenter; nci++)
    {
        // the nci-th cluster
        memset(filename, 0, 255);
        sprintf(filename, "%s/%d%s", indexfolder, nci, CLUSTER_POXFIX);
        fp = open_file(filename, "wb");
        for(i = 0; i < c->nassign[nci]; i++)
        {
            fwrite(&c->innerLB[nci][i].id, sizeof(int), 1, fp);             // id of this data point
            fwrite(&c->innerLB[nci][i].val, sizeof(double), 1, fp);         // distance to boundary
            fwrite(ds->data+d*c->innerLB[nci][i].id, sizeof(float), d, fp); // this data point
        }
        fclose(fp);
    }
}

void HBPlus_Search(Clustering *c, fDataSet *queryset, int m, float alpha, float *R, float *r_centroid, char *folder, int nk, DoubleIndex **knnset, Cost *cost)
{
    char filename[256];
    int d = queryset->d,
        nq = queryset->n,
        qi, i, set_i;
    int cid, point_num;
    float knn_R;
    float *set;
    int *set_id;
    int set_num;
    float *set_vector = NULL;
    float *query = fvec_new(d);
    DoubleIndex candidate;
    DoubleIndex *lowerbound = (DoubleIndex*)malloc(sizeof(DoubleIndex)*c->ncenter);
                                                                // lower bounds between query and all centers
    Cost costi;
    struct timeval tvb, tve, tvb_lb, tve_lb, tvb_io, tve_io;
    Heap heap;
    for(qi = 0; qi < nq; qi++)
    {
        /// initialize the cost recorder
        CostInit(&costi);
        gettimeofday(&tvb, NULL);

        /// the qi-th query
        memcpy(query, queryset->data+qi*d, sizeof(float)*d);
        knnset[qi] = (DoubleIndex*)malloc(sizeof(DoubleIndex)*nk);
        /// calculate the lower bounds between query and all clusters to get the right order
        gettimeofday(&tvb_lb, NULL);

        HBPlus_LowerBound(lowerbound, query, r_centroid, c->centroid, c->innerLB, c->ncenter, d, m, alpha);
        gettimeofday(&tve_lb, NULL);
        costi.lowerbound = timediff(tvb_lb, tve_lb);

        /// search for knn
        set_vector = fvec_new(d);
        knn_R = FLOAT_MAX;
        i = 0;
        Heap_Init(&heap, nk);
        while(i < c->ncenter)
        {
            cid = lowerbound[i].id;
            // the i-th cluster
            if(f_bigger(lowerbound[i].val, knn_R))
            {
                break;
            }
            // knn_R > lowerbound[i], means there are candidates in the i-th cluster
            set_num = c->nassign[cid];
            set = fvec_new(set_num*d);
            set_id = ivec_new(set_num);
            sprintf(filename, "%s/%d.cluster", folder, cid);
            gettimeofday(&tvb_io, NULL);
            point_num = HBPlus_ClusterFromFile(filename, set_num, d, set, set_id, knn_R-lowerbound[i].val);
            gettimeofday(&tve_io, NULL);
            costi.io = costi.io + timediff(tvb_io, tve_io);
            costi.page = costi.page + 1;
            costi.point = costi.point + point_num;
            for(set_i = 0; set_i < point_num; set_i++)
            {// calculate real distance between all candidates and query
                candidate.id = set_id[set_i];
                memcpy(set_vector, set+set_i*d, sizeof(float)*d);
                candidate.val = odistance(query, set_vector, d);
                if(heap.length < heap.MaxNum || f_bigger(heap.elem[0].val, candidate.val))
                {// heap is not full or a smaller candidate
                    MaxHeap_Insert(&heap, &candidate);
                }
            }
            knn_R = heap.elem[0].val;
            i++;

            // free
            free(set); set = NULL;
            free(set_id); set_id = NULL;
        }// end of search loop
        // printf("%d ", i);//
        memcpy(knnset[qi], heap.elem, sizeof(DoubleIndex)*heap.length);

        gettimeofday(&tve, NULL);
        costi.cpu = timediff(tvb, tve);
        costi.search = costi.cpu - costi.lowerbound - costi.io;

        /// sum new cost
        CostCombine(cost, &costi);
        Heap_Delete(&heap);
    }

    CostMultiply(cost, 1/(float)nq);

    free(set_vector); set_vector = NULL;
    free(query); query = NULL;
    free(lowerbound); lowerbound = NULL;
}

/***************************** basic operations ********************************/
void HBPlus_LowerBound(DoubleIndex *lowerbound, const float *query, const float *r_c, const float *centroid, DoubleIndex **innerLB, int ncenter, int d, int m, float alpha)
{
    int i, j, ci, cj, nci, otheri;
    int T = (int)ceil(alpha * ncenter);
    float *x = fvec_new(d);
    float *c = fvec_new(d);
    float *rx = fvec_new(m);
    float *rc = fvec_new(m);
    float *centroid_distance_map = fvec_new(ncenter*ncenter);

    /// 1. prepare distances between query and all clusters, and sort
    DoubleIndex *qcsdis = (DoubleIndex*)malloc(sizeof(DoubleIndex)*ncenter);
    float max, temp;

    for(i = 0; i < ncenter; i++){
        memcpy(x, centroid+i*d, sizeof(float)*d);
        qcsdis[i].id = i;
        qcsdis[i].val = odistance_square(query, x, d);
    }
    DI_MergeSort(qcsdis, 0, ncenter-1);

    /// 2. prepare the estimate distance map between each centroids
    for(nci = 0; nci < ncenter; nci++)
    {
        memcpy(rx, r_c+nci*m, sizeof(float)*m);
        for(otheri = 0; otheri <= nci; otheri++)
        {
            memcpy(rc, r_c+otheri*m, sizeof(float)*m);
            centroid_distance_map[nci*ncenter+otheri] = odistance(rx, rc, m);
            centroid_distance_map[otheri*ncenter+nci] = centroid_distance_map[nci*ncenter+otheri];
        }
    }

    /// 3. for each cluster, figure out the lower bound distance
    lowerbound[0].id = qcsdis[0].id;
    lowerbound[0].val = 0;
    Heap heap;
    DoubleIndex di;
    for(i = 1; i < ncenter; i++)
    {
        ci = qcsdis[i].id;
        max = FLOAT_ZERO;
        if(i < T)
        {// seperate bounds not exceeds T
            for(j = 0; j < i; j++)
            {
                cj = qcsdis[j].id;
                memcpy(x, centroid+ci*d, sizeof(float)*d);
                memcpy(c, centroid+cj*d, sizeof(float)*d);
                temp = (qcsdis[i].val - qcsdis[j].val) / (2*odistance(x, c, d));
                if(f_bigger(temp, max))
                {
                    max = temp;
                }
            }
            lowerbound[i].id = ci;
            lowerbound[i].val = max;
        }
        else
        {// seperate bounds exceeds T
            Heap_Init(&heap, T);
            for(j = 0; j < i; j++)
            {
                // estimate the q H distance
                cj = qcsdis[j].id;
                di.id = j;
                di.val = (qcsdis[i].val - qcsdis[j].val)
                                / (2*centroid_distance_map[ci*ncenter+cj]);
                if(j < T || f_bigger(di.val, heap.elem[0].val))
                {// insert a larger value
                    MinHeap_Insert(&heap, &di);
                }
            }
            for(j = 0; j < T; j++)
            {
                cj = qcsdis[heap.elem[j].id].id;
                memcpy(x, centroid+ci*d, sizeof(float)*d);
                memcpy(c, centroid+cj*d, sizeof(float)*d);
                temp = (qcsdis[i].val - qcsdis[heap.elem[j].id].val) / (2*odistance(x, c, d));
                if(f_bigger(temp, max))
                {
                    max = temp;
                }
            }
            lowerbound[i].id = ci;
            lowerbound[i].val = max;

            Heap_Delete(&heap);
        }

    }
    DI_MergeSort(lowerbound, 0, ncenter-1);

    free(x); x = NULL;
    free(c); c = NULL;
    free(rc); rc = NULL;
    free(rx); rx = NULL;
    free(qcsdis); qcsdis = NULL;
    free(centroid_distance_map); centroid_distance_map = NULL;
}

int HBPlus_ClusterFromFile(const char *filename, int num, int d, float *set, int *set_num, double lblimit)
{
    int i, nret;
    double lb;
    FILE *fp = fopen(filename, "rb");
    if(fp == NULL)
    {
        printf("error to open file %s\n", filename);
        exit(-1);
    }
    for(i = 0; i < num; i++)
    {
        nret = fread(set_num+i, sizeof(int), 1, fp);
        nret = fread(&lb, sizeof(double), 1, fp);
        if(f_bigger(lb, lblimit))
        {// quit when lb exceeds the lb limit
            break;
        }
        else{
            nret = fread(set+i*d, sizeof(float), d, fp);
        }
    }
    fclose(fp);

    return i;
}

void HBPlus_GenerateRotate(float *R, int m, int d)
{
    float constant = sqrt(3.0);
    int i = 0;
    int seed;
    srand(time(NULL));

    for(i = 0; i < m*d; i++)
    {
        seed = rand() % 6;
        if(seed == 0){
            R[i] = constant;
        }
        else if(seed == 5){
            R[i] = -constant;
        }
        else{
            R[i] = 0;
        }
    }
}

void HBPlus_RotateCentroid(float *r_c, float *c, float *R, int k, int d, int m)
{
    int ki, mi;
    float *xc, *xr;
    xc = fvec_new(d);
    xr = fvec_new(d);
    for(ki = 0; ki < k; ki++)
    {
        memcpy(xc, c+ki*d, sizeof(float)*d);
        for(mi = 0; mi < m; mi++)
        {
            memcpy(xr, R+mi*d, sizeof(float)*d);
            r_c[ki*m+mi] = 1/sqrt(m)*inner_product(xc, xr, d);      // according to the equation
        }
    }
    free(xc); xc = NULL;
    free(xr); xr = NULL;
}
