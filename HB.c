#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "yael_v438/yael/vector.h"
#include <assert.h>
#include "HB.h"
#include "common.h"
#include "cost.h"
#include "clustering.h"
#include "heap.h"

void HB_IndexIntoFile(const Clustering *c, const char *folder, const fDataSet *ds)
{
    char filename[255] = {'\0'};
    FILE *fp;
    int nci, di, i;
    int d = ds->d;

    // store the parameter of index(center, inner distance, member count)
    sprintf(filename, "%s/%s", folder, INDEX_PARA);
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
        sprintf(filename, "%s/%d%s", folder, nci, CLUSTER_POXFIX);
        fp = open_file(filename, "wb");
        for(i = 0; i < c->nassign[nci]; i++)
        {
            fwrite(&c->member[nci][i], sizeof(int), 1, fp);                                         // dimension
            fwrite(ds->data+d*c->member[nci][i],sizeof(float), d, fp);     // data
        }
        fclose(fp);
    }
}

void HB_Search(const Clustering *c, const fDataSet *queryset, char *folder, int nk, DoubleIndex **knnset, Cost *cost)
{
    char filename[256];
    int d = queryset->d,
        nq = queryset->n,
        qi, i, set_i;
    int cid;
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
        HB_LowerBound(lowerbound, query, c->centroid, c->innerLB, c->ncenter, d);
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
            HB_ClusterFromFile(filename, set_num, d, set, set_id);
            gettimeofday(&tve_io, NULL);
            costi.io = costi.io + timediff(tvb_io, tve_io);
            costi.page = costi.page + 1;
            costi.point = costi.point + set_num;

            for(set_i = 0; set_i < set_num; set_i++)
            {// calculate real distance between all candidates and query
                candidate.id = set_id[set_i];
                memcpy(set_vector, set+set_i*d, sizeof(float)*d);
                candidate.val = odistance(query, set_vector, d);
                if(heap.length < heap.MaxNum || f_bigger(heap.elem[0].val, candidate.val))
                {// heap is not full or new value is smaller, insert
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

void HB_KnnIntoFile(const char *filename, DoubleIndex **knnset, int nq, int nk)
{
    int i, j;
    FILE *fp = fopen(filename, "w");
    for(i = 0; i < nq; i++)
    {
        // fprintf(fp, "%f", knnset[i][0].val);     // the distance to the nearest neighbor
        for(j = 0; j < nk; j++)
        {
            fprintf(fp, "%d ", knnset[i][j].id);
        }
        fputc('\n', fp);
    }
    fclose(fp);
}
/*************************** basic operations ***************************/
void HB_LowerBound(DoubleIndex *lowerbound, const float *query, const float *centroid, DoubleIndex **innerLB, int ncenter, int d)
{
    int i, j, nci, otheri, idi, idj;
    float max_dis, temp_dis;
    float *center = fvec_new(d);
    float *ocenter = fvec_new(d);
    DoubleIndex *sqdis_query_centroid;                // square distance between query and all centroids
    float *centroid_distance_map = fvec_new(ncenter*ncenter);

    /// prepare the query centroid square distance map
    sqdis_query_centroid = (DoubleIndex*)malloc(sizeof(DoubleIndex)*ncenter);
    for(nci = 0; nci < ncenter; nci++)
    {
        sqdis_query_centroid[nci].id = nci;
        memcpy(center, centroid+nci*d, sizeof(float)*d);
        sqdis_query_centroid[nci].val = odistance_square(query, center, d);
    }
    // sort
    DI_MergeSort(sqdis_query_centroid, 0, ncenter-1);

    /// prepare the center distance map
    for(nci = 0; nci < ncenter; nci++)
    {
        memcpy(center, centroid+nci*d, sizeof(float)*d);
        for(otheri = 0; otheri <= nci; otheri++)
        {
            memcpy(ocenter, centroid+otheri*d, sizeof(float)*d);
            centroid_distance_map[nci*ncenter+otheri] = odistance(center, ocenter, d);
            centroid_distance_map[otheri*ncenter+nci] = centroid_distance_map[nci*ncenter+otheri];
        }
    }

    /// calculate the lower bounds
    lowerbound[0].id = sqdis_query_centroid[0].id;
    lowerbound[0].val = FLOAT_ZERO;
    for(i = 1; i < ncenter; i++)
    {
        idi = sqdis_query_centroid[i].id;
        // work out the (i-th id)-th cluster's lower bound
        max_dis = FLOAT_ZERO;
        for(j = 0; j < i; j++)
        {
            idj = sqdis_query_centroid[j].id;
            temp_dis = (sqdis_query_centroid[i].val - sqdis_query_centroid[j].val)
                     / (2*centroid_distance_map[idi*ncenter+idj]);
            if(f_bigger(temp_dis, max_dis))
            {// a larger lower bound distance
                max_dis = temp_dis;
            }
        }
        lowerbound[i].id = idi;
        lowerbound[i].val = max_dis + innerLB[idi][0].val;
    }
    /// sort lower bounds
    DI_MergeSort(lowerbound, 0, ncenter-1);

    free(center); center = NULL;
    free(ocenter); ocenter = NULL;
    free(sqdis_query_centroid); sqdis_query_centroid = NULL;
    free(centroid_distance_map); centroid_distance_map = NULL;
}
void HB_ClusterFromFile(const char *filename, int num, int d, float *set, int *set_num)
{
    int i, nret;
    FILE *fp = fopen(filename, "rb");
    if(fp == NULL)
    {
        printf("error to open file %s\n", filename);
        exit(-1);
    }
    for(i = 0; i < num; i++)
    {
        nret = fread(set_num+i, sizeof(int), 1, fp);
        nret = fread(set+i*d, sizeof(float), d, fp);
    }
    fclose(fp);
}
