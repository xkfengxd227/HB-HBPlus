/*
 * Some implemetation for common configs
 */
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "yael_v438/yael/vector.h"
#include <sys/time.h>

long timediff(struct timeval begin, struct timeval end)
{
    return ((end.tv_sec - begin.tv_sec) * 1000000 + (end.tv_usec - begin.tv_usec));
}

float odistance_square(const float *a, const float *b, int d)
{
    float dis = 0;
    int i;
    for(i = 0; i < d; i++)
    {
        dis = dis + (a[i]-b[i]) * (a[i]-b[i]);
    }
    return dis;
}

// Encludiean distance between two d-dim vectors
float odistance(const float *a, const float *b, int d)
{
    return sqrt(odistance_square(a, b, d));
}

float inner_product(float *a, float *b, int d)
{
    float rslt = 0;
    int i;
    for(i = 0; i < d; i++)
    {
        rslt = rslt + a[i] * b[i];
    }
    return rslt;
}
// 析出数据点对各个聚类的归属, and calculate the minimum distance between each member and the bound
void extract_members(int *assign, int *nassign, int **member, int n, int ncenter)
{
	int i;
	int *pos = ivec_new_0(ncenter);

	// 分配空间 
	int total = 0;
	for(i = 0; i < ncenter; i++)
	{
		member[i] = ivec_new_set(nassign[i], -1);

		total = total + nassign[i];
	}
	printf("total: %d\n", total);

	// 提取出属于每一个聚类的数据点ID
	int icenter = -1;
	for(i = 0; i < n; i++)
	{
		// 第i个数据点属于第icenter个聚类
		icenter = assign[i];
		// 第icenter个聚类添加第i个数据点ID到icenter目前的列表末尾
		member[icenter][pos[icenter]] = i;
		// 第icenter个聚类的位置+1
		pos[icenter] ++;
	}

	// 验证提取后每个聚类的成员个数与之前结果的匹配性，并打印分配结果
	FILE *fp = fopen("member.txt", "w");
	int j;
	for(i = 0; i < ncenter; i++)
	{
		fprintf(fp, "%d:%d\t", i, pos[i]);
		for(j = 0; j < pos[i]; j++)
		{
			fprintf(fp, " %d", member[i][j]);
		}
		fputc('\n', fp);
	}
	fclose(fp);

	free(pos); pos = NULL;
}
// compare two float number, return true when a > b
int f_bigger(float a, float b)
{
    return ((a-b > FLOAT_ZERO) ? 1:0);
}

FILE *open_file(char *filename, char *format)
{
    FILE *fp;
    fp = fopen(filename, format);
    if(fp == NULL)
    {
        printf("failed to open file %s\n", filename);
        exit(-1);
    }
    return fp;
}


void DI_MergeSort(DoubleIndex *di, int l, int r)
{
	int m;
    if (l < r)                                                                                                          //继续划分
    {
        m = (l + r) / 2;                                                                                            //中间位置
        DI_MergeSort(di, l, m);                                                                                     //归并排序左半边
        DI_MergeSort(di, m + 1, r);                                                                                 //归并排序右半边
        DI_Merge(di, l, m, r);                                                                                      //合并两边
    }
}

void DI_Merge(DoubleIndex *di, int l, int m, int r)
{
	int i;
    ///
    /// 计算左右长度
    ///
    int llen = m - l + 1;
    int rlen = r - m;
    DoubleIndex *llb = (DoubleIndex*)malloc(sizeof(DoubleIndex) * (llen+1));
    DoubleIndex *rlb = (DoubleIndex*)malloc(sizeof(DoubleIndex) * (rlen+1));

    ///
    /// 将左右边数据读出来，存储来，利于排序
    ///
    for (i = 0; i < llen; i++)
    {
    	memcpy(&llb[i], &di[l+i], sizeof(DoubleIndex));
    }
    for (i = 0; i < rlen; i++)
    {
    	memcpy(&rlb[i], &di[m + 1 + i], sizeof(DoubleIndex));
    }
    ///
    /// 在左右分组的末尾添加一个“极大值”用作本分组结束的标志，2*PI
    ///
    llb[llen].id = llen + 1;
    llb[llen].val = FLOAT_MAX;
    rlb[rlen].id = rlen + 1;
    rlb[rlen].val = FLOAT_MAX;

    int li = 0,
        ri = 0,
        k = 0;
    for (i = l; i <= r; i++)
    {
        if (f_bigger(rlb[ri].val, llb[li].val))                                            //左边较小
        {
        	memcpy(&di[l+k], &llb[li], sizeof(DoubleIndex));
            li++;
            k++;
        }
        else
        {
        	memcpy(&di[l+k], &rlb[ri], sizeof(DoubleIndex));
            ri++;
            k++;
        }
    }

    free(rlb); rlb = NULL;
    free(llb); llb = NULL;
}

void load_groundtruth(char *filename, int MAXK, int nq, DoubleIndex *groundtruth)
{
    FILE *fp;
    fp = open_file(filename, "rb");
    int i, j, nret;
    int *stream = NULL;


    /// read actual dimension of groundtruth
    int d;
    nret = fread(&d, sizeof(int), 1, fp);
    if (d < MAXK){
        printf("not enough groundtruth\n");
        fclose(fp);
        exit(-1);
    }


    /// read groundtruth into buffer
    fseek(fp, 0, SEEK_SET);
    stream = ivec_new(nq*(d+1));
    nret = fread(stream, sizeof(int), (nq*(d+1)), fp);
    fclose(fp);
    printf("d: %d, MAXK: %d, read: %d\n", d, MAXK, nret);


    /// convert buffer into groundtruth
    for(i = 0; i < nq; i++)
    {
        for(j = 0; j < MAXK; j++)
        {
            groundtruth[i*MAXK + j].id = stream[i*(d+1) + j + 1];
            groundtruth[i*MAXK + j].val = -1;
        }

        // printf("%d: %d\n", i, groundtruth[i*MAXK].id);
    }

    free(stream);   stream = NULL;
}

/*
float verify_knn(DoubleIndex **knnset, DoubleIndex *groundtruth, int nq, int nk)
{
    int i, j, count = 0, k;
    float accuracy = 0;
    bool match = false;

    int *status = ivec_new(nq*nk);
    bool direct = false;

    for(i = 0; i < nq; i++)
    {
        // re-order
        DI_MergeSort(knnset[i], 0, nk-1);

        for(j = 0; j < nk; j++)
        {
            match = false;
            direct = true;

            if(knnset[i][j].id == groundtruth[i*nk+j].id)
            {
                match = true;
            }
            else
            {// miss match, shake range
                for(k = j - SHAKERANGE; k <= j+SHAKERANGE; k++)
                {
                    if(k >= 0 && k < nk && k != j)
                    {
                        if(knnset[i][j].id == groundtruth[i*nk+k].id)
                        {
                            match = true;
                            direct = false;
                        }
                    }
                }
            }
            count = count + (match ? 1 : 0);
            if(!match)
            {
                status[i*nk+j] = 0;
            }
            else if(direct)
            {
                status[i*nk+j] = 1;
            }
            else
            {
                status[i*nk+j] = 9;
            }
        }
    }

    printf("match:%d\n", count);
    accuracy = (float)count / (nq * nk);

    free(status); status = NULL;
    return accuracy;
}
*/

float verify_knn(DoubleIndex **knnset, DoubleIndex *groundtruth, int nq, int nk, int dGT)
{

    int     count;
    int     qi, ki, gi;
    int     gtid;

    count = 0;
    for(qi = 0; qi < nq; qi++){
        /* for the qi-th query */
        for(gi = 0; gi < nk; gi++){
            gtid = groundtruth[qi*dGT+gi].id;        /* id of the gi-th groundtruth */

            /* check whether the gi-th groundtruth is in the kNN retults by matching id one by one */
            for(ki = 0; ki < nk; ki++){
                if(knnset[qi][ki].id == gtid){
                    count++;
                    break;
                }
            }
        }

        
    }

    return (float)count / nq / nk;
}

void calculate_groundtruth(float *data, float *query, int n, int d, int nq, int nk, DoubleIndex *groundtruth)
{
    DoubleIndex *knn = (DoubleIndex*)malloc(sizeof(DoubleIndex)*n);
    float *q = fvec_new(d);
    float *x = fvec_new(d);
    int i, j;
    for(i = 0; i < nq; i++)
    {
        memcpy(q, query+i*d, sizeof(float)*d);
        for(j = 0; j < n; j++)
        {
            memcpy(x, data+j*d, sizeof(float)*d);
            knn[j].id = j;
            knn[j].val = odistance(q, x, d);
        }
        DI_MergeSort(knn, 0, n-1);
        memcpy(groundtruth+i*nk, knn, sizeof(DoubleIndex)*nk);
    }
    free(knn); knn = NULL;
    free(q); q = NULL;
    free(x); x = NULL;

    FILE *fp = open_file("1000.groundtruth", "w");
    for(i = 0; i < nq; i++)
    {
        // fprintf(fp, "%d", d);
        for(j = 0; j < nk; j++)
        {
            fprintf(fp, "%d ", groundtruth[i*nk+j].id);
        }
        fputc('\n', fp);
    }
    fclose(fp);
}
