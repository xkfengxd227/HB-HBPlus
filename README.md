(current folder ~/code/hbplus)

Structure of this package
=========================

This package contains all necessary codes for the HB and HB+ (or HBPlus) algorithm to do exact NN search in high-dimensions.

The meaning of all the files/folders are as follows:

- HB.*			codes for the HB algorithm
- HBPlus.*		codes for the HB+ algorithm
- clustering.* 	clustering codes based on KMeans in the yael library
- common.*		some common declarations
- cost.*		record the NN search cost, mainly include IO and time
- heap.*		the min-heap and max-heap
- main.c 		the main program to run HB and HB+

- HBindex, HBPlusindex		folders to store index structures for HB and HB+
- yael_v438/ 	One of the core operation of HB and HB+ is KMeans clustering, we use the efficient 
				yael library to do KMeans clustering


How to use this package
=======================

The codes is writen under Linux system, our system version is Ubuntu 14.04.


1. Compile the yael library
---------------------------

Since our HB and HB+ codes is based on the yael library, you need to download and compile the yael library first. We have provided a version of the yael library in this package, so you can directly compile it. 

The compile of yael is mainly about installing several essential libraries on your computer, including gfortran, libblas3gf, liblapack3gf and swig. You can refer to this post (https://blog.csdn.net/xd227/article/details/48860635) as a direction.

If you do not compile it, you will encounter the following error
```
/usr/bin/ld: cannot find -lyael
collect2: error: ld returned 1 exit status
```


2. Setting parameters in the main.c file
----------------------------------------

Parameters of our algorithm mainly include dataset related parameters and algorithm related parameters.

For dataset, we use the format of the popular .fvecs and .ivecs binary format. It is a very intuitive and simple format, you can visit http://corpus-texmex.irisa.fr/ for detailed knowledge.

All the parameters should be set in the main.c file before compiling. This main.c program contains necesary loops for multiple parameter tunning. If you are not used to this manner, you can adjust the program to the pass-call manner. 


3. Compile our codes
--------------------
After you set all the parameter values, you can just compile and run to get the results.

```
# make
# ./main 		# run
```



Example results
===============
Following is a result of successfully running the algorithm. The upper part is the process of kmeans clustering (100 iterations, 120 clusters). The below part is the performance results of performing 
k=100-NN search in HB and HB+ (m=3, alpha=0.08).

```
root@ws-unsw:~/code/hbplus# ./main
d: 1000, MAXK: 100, read: 200200
<><><><> kmeans / run 0 <><><><><>
 -> 0.048 -> 0.037 -> 0.034 -> 0.032 -> 0.032 -> 0.031 -> 0.031 -> 0.031 ->
 0.030 -> 0.030 -> 0.030 -> 0.030 -> 0.030 -> 0.030 -> 0.030 -> 0.030 -> 0.030 
 -> 0.030 -> 0.030 -> 0.030 -> 0.030 -> 0.030 -> 0.030 -> 0.030 -> 0.030 -> 
 0.030 -> 0.030 -> 0.030 -> 0.030 -> 0.030 -> 0.029 -> 0.029 -> 0.029 -> 0.029 
 -> 0.029 -> 0.029 -> 0.029 -> 0.029 -> 0.029 -> 0.029 -> 0.029 -> 0.029 ->
 0.029 -> 0.029 -> 0.029 -> 0.029 -> 0.029 -> 0.029 -> 0.029 -> 0.029 -> 0.029 
 -> 0.029 -> 0.029 -> 0.029 -> 0.029 -> 0.029 -> 0.029 -> 0.029 -> 0.029 -> 0.029
 -> 0.029 -> 0.029 -> 0.029 -> 0.029 -> 0.029 -> 0.029 -> 0.029 -> 0.029 -> 0.029
 -> 0.029 -> 0.029 -> 0.029 -> 0.029 -> 0.029 -> 0.029 -> 0.029 -> 0.029 -> 0.029
 -> 0.029 -> 0.029 -> 0.029 -> 0.029 -> 0.029 -> 0.029 -> 0.029 -> 0.029 -> 0.029 -> 0.029 
 -> 0.029 -> 0.029 -> 0.029 -> 0.029 -> 0.029 -> 0.029 -> 0.029 -> 0.029 -> 0.029 -> 0.029 
 -> 0.029 -> 0.029
Total number of iterations: 100
Unbalanced factor of last iteration: 1.1137
>>> clustering, quantization error: 0.029165
total: 67040

################# k=100-NN, cluster: 120 ##########################


------------------ HB ---------------
IO	(page	points)	time(us)	(lb	io	search)	accuracy
53	18	11404	1513		268	878	366	1.000000

--------------- HBPlus: m=3, alpha=0.080000 -----------------------
IO	(page	points)	time(us)	(lb	io	search)	accuracy
42	18	7946	1369		248	849	270	1.000000
```
