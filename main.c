/*----------------------------------------------------------
 * This is a program running both HB and HBPlus
 *---------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "HB.h"
#include "HBPlus.h"
#include "common.h"
#include "cost.h"
#include "clustering.h"
#include "heap.h"
#include "yael_v438/yael/vector.h"
#include "yael_v438/yael/machinedeps.h"

int main(int argc, char *argv[])
{
    /*----------------------------------------------------------
     *    datasets
     *---------------------------------------------------------*/
    /// CoorHistogram    
    char		datafile[255] 	= 	    "/home/xikafe/dataset/color/bin/color_base.fvecs";
    char        learnfile[255]  =       "/home/xikafe/dataset/color/bin/color_base.fvecs";  
                                                // for small dataset, we use the base set as the learning set
    char        queryfile[255]  =       "/home/xikafe/dataset/color/bin/color_query.fvecs";
    char        groundtruthfile[255] =  "/home/xikafe/dataset/color/bin/color_gt.ivecs";
    int                     d   =       32;
    int                     n   =       67040;
    int                     n_l =       67040;      // number of learning (or training) data set

    /// Aerial
/*
    char		datafile[255] 	= 	    "/home/xikafe/dataset/aerial/bin/aerial_base.fvecs";
    char        learnfile[255]  =       "/home/xikafe/dataset/aerial/bin/aerial_base.fvecs";
    char        queryfile[255]  =       "/home/xikafe/dataset/aerial/bin/aerial_query.fvecs";
    char        groundtruthfile[255] =  "/home/xikafe/dataset/aerial/bin/aerial_gt.ivecs";
    int                     d   =       60;
    int                     n   =       274465;
    int                     n_l =       100000;
*/

    /// WT
/*
    char        datafile[255]   =       "/home/xikafe/dataset/wt/bin/wt_base.fvecs";
    char        learnfile[255]  =       "/home/xikafe/dataset/wt/bin/wt_learn.fvecs"; 
                                                    // for large dataset, we can use a learning set
    char        queryfile[255]  =       "/home/xikafe/dataset/wt/bin/wt_query.fvecs";
    char        groundtruthfile[255] =  "/home/xikafe/dataset/wt/bin/wt_gt.ivecs";
    int                     d   =       128;
    int                     n   =       269648;
    int                     n_l =       161789;
*/


    // 基本数据
	char  hb_indexfolder[255]   =       "HBindex";         // index folder for HB
    char hbplus_indexfolder[255]=       "HBPlusindex";      // index folder for HBPlus
	int nq = 200;                                  // 	number of query
	fDataSet 		    ds;	                        // storing base dataset
	fDataSet            lds;                        // storing learning set
    fDataSet            qds;                        // storing query set
    ds.d = d;
    ds.n = n;
    lds.d = d;  
    lds.n = n_l;    
    qds.d = d;
    qds.n = nq;

    /*----------------------------------------------------------
     *    Clustering related parameters
     *---------------------------------------------------------*/
    Clustering c;
    C_Init(&c);
  	c.niter                    =		100;	              // iteration number in clustering
	c.nthread                  =		1; //count_cpu();    // number of threads to using during clustering
	c.seed                     =		0;                   // seeds for clustering
	c.nredo                    =		1;                   // clustering times, usually use 1 clustering progress


    /*----------------------------------------------------------
     *    Parameters tuning: including k-NN search parameters and HB and HBPlus parameters
     *---------------------------------------------------------*/

    /// nk: number of NNs to return
    // int     nk_set[11]          =       {1,10,20,30,40,50,60,70,80,90,100};
    int     MAXK                =       100;
    int     nk_set[1]           =       {100};
    int     nk_num              =       1;  // how many [nk] to test, equal to the actual values in the nk_set array
    int     nk                  =       nk_set[0];
    int     nki                 =       -1;

    /// number of clusters to generate during the Clustering
    int     ncenter_set[1]      =       {120};
    int     ncenter_num         =       1;
    c.ncenter                   =		ncenter_set[0];
    int     nci                 =       -1;


    /// [m] parameter in HBPlus
    int     m_set[1]            =       {3};
    int     m_num               =       1;
    int     m                   =       m_set[0];
    int     mi                  =       -1;

    /// [alpha] parameter in HBPlus
    float   alpha_set[1]        =       {0.08};
    int     alpha_num           =       1;
    float   alpha               =       alpha_set[0];
    int     alphai              =       -1;


    float   *R;
    float   *hbplus_centroid;
    int i;

    /*----------------------------------------------------------
     *    k-NN search
     *---------------------------------------------------------*/

	/* loading groundtruth, base, learn and query set */
    DoubleIndex **hb_knnset, **hbplus_knnset;
    DoubleIndex *groundtruth = (DoubleIndex*)malloc(sizeof(DoubleIndex)*nq * MAXK);
    load_groundtruth(groundtruthfile, MAXK, nq, groundtruth);


	ds.data = fvec_new(n*d);
	lds.data = fvec_new(n_l*d);
	qds.data = fvec_new(nq*d);
	fvecs_read(datafile, d, n, ds.data);
	fvecs_read(learnfile, d, n_l, lds.data);
    fvecs_read(queryfile, d, nq, qds.data);

    /*  performance recorders */
    float hb_accuracy, hbplus_accuracy;
    Cost hb_cost, hbplus_cost;


	for(nci = 0; nci < ncenter_num; nci++)
	{
	    /// specify a cluster number and do Clustering, and store the index into file
	    c.ncenter = ncenter_set[nci];
	    C_Clustering(&c, &ds, &lds);
        HB_IndexIntoFile(&c, hb_indexfolder, &ds);
        HBPlus_IndexIntofile(&c, hbplus_indexfolder, &ds);


	    for(nki = 0; nki < nk_num; nki++)
	    {
	        /// specify a number of returned NNs and do [nk]-NN search using HB and HBPlus
            nk = nk_set[nki];

            if(MAXK < nk){
                printf("error: MAXK should be bigger than nk\n");
                return -1;
            }

            printf("\n################# k=%d-NN, cluster: %d ##########################\n", 
                nk, ncenter_set[nci]);

            /***************** search with HB ******************/
            hb_knnset = (DoubleIndex **)malloc(sizeof(DoubleIndex*)*nq);
            CostInit(&hb_cost);
            HB_Search(&c, &qds, hb_indexfolder, nk, hb_knnset, &hb_cost);
            hb_accuracy = verify_knn(hb_knnset, groundtruth, nq, nk, MAXK);

            printf("\n\n------------------ HB ---------------\n");
            // printf("accuracy:\t%f\n", hb_accuracy);
            // printf("IO:\t%ld\n", hb_cost.page + hb_cost.point / ( PAGESIZE/(d*sizeof(float)) ) / PAGE_RAN_SE);
            // CostDisplay(hb_cost);
            printf("IO\t(page\tpoints)\ttime(us)\t(lb\tio\tsearch)\taccuracy\n");
            printf("%ld\t%ld\t%ld\t%ld\t\t%ld\t%ld\t%ld\t%f\n",
                    hb_cost.page + hb_cost.point / ( PAGESIZE/(d*sizeof(float)) ) / PAGE_RAN_SE, hb_cost.page, hb_cost.point,
                    hb_cost.cpu, hb_cost.lowerbound, hb_cost.io, hb_cost.search,
                    hb_accuracy);

            // release memory
            for(i = 0; i < nq; i++)
            {
                free(hb_knnset[i]); hb_knnset[i] = NULL;
            }
            free(hb_knnset); hb_knnset = NULL;


            /***************** search with HBPlus ******************/
            for(mi = 0; mi < m_num; mi++)
            {
                /// specify a value of [m]
                m = m_set[mi];


                /// generate the transformation (or rotating) matrix [R]
                R = fvec_new(m*d);
                HBPlus_GenerateRotate(R, m, d);

                hbplus_centroid = fvec_new(c.ncenter*m);
                HBPlus_RotateCentroid(hbplus_centroid, c.centroid, R, c.ncenter, d, m);

                for(alphai = 0; alphai < alpha_num; alphai++)
                {
                    alpha = alpha_set[alphai];

                    hbplus_knnset = (DoubleIndex **)malloc(sizeof(DoubleIndex*)*nq);
                    CostInit(&hbplus_cost);
                    HBPlus_Search(&c, &qds, m, alpha, R, hbplus_centroid, hbplus_indexfolder, nk, hbplus_knnset, &hbplus_cost);
                    hbplus_accuracy = verify_knn(hbplus_knnset, groundtruth, nq, nk, MAXK);

                    printf("\n--------------- HBPlus: m=%d, alpha=%f -----------------------\n",
                        m, alpha);
                    // printf("accuracy:\t%f\n", hbplus_accuracy);
                    // printf("IO:\t%ld\n", hbplus_cost.page + hbplus_cost.point / ( PAGESIZE/(d*sizeof(float)) ) / PAGE_RAN_SE);
                    // CostDisplay(hbplus_cost);
                    printf("IO\t(page\tpoints)\ttime(us)\t(lb\tio\tsearch)\taccuracy\n");
                    printf("%ld\t%ld\t%ld\t%ld\t\t%ld\t%ld\t%ld\t%f\n",
                        hbplus_cost.page + hbplus_cost.point / ( PAGESIZE/(d*sizeof(float)) ) / PAGE_RAN_SE, hbplus_cost.page, hbplus_cost.point,
                        hbplus_cost.cpu, hbplus_cost.lowerbound, hbplus_cost.io, hbplus_cost.search,
                        hbplus_accuracy);

                    // release memory
                    for(i = 0; i < nq; i++)
                    {
                        free(hbplus_knnset[i]); hbplus_knnset[i] = NULL;
                    }
                    free(hbplus_knnset); hbplus_knnset = NULL;
                }

                free(R); R = NULL;
                free(hbplus_centroid); hbplus_centroid = NULL;
            }
	    }

	    // release memory
        C_Delete(&c);
	}

    /// 999. release memory
    free(ds.data); ds.data = NULL;
    free(lds.data); lds.data = NULL;
    free(qds.data); qds.data = NULL;
	free(groundtruth); groundtruth = NULL;
    return 0;
}
