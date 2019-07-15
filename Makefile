.SUFFIXES: .c .o .swg .cpp

# the yael library
include ./yael_v438/makefile.inc
YAELPATH=./yael_v438/yael


all: main

ifeq "$(USEARPACK)" "yes"
  YAELLDFLAGS+=$(ARPACKLDFLAGS)
  EXTRAMATRIXFLAG=-DHAVE_ARPACK
endif

ifeq "$(USEOPENMP)" "yes"
  EXTRAMATRIXFLAG+=-fopenmp
  YAELLDFLAGS+=-fopenmp
endif

cost: cost.o
	$(CC) $(FLAGS) $(EXTRACFAGS) $(YAELCFLAGS) -lm

common: common.o
	$(CC) $(FLAGS) $(EXTRACFAGS) $(YAELCFLAGS) -lm

heap: heap.o common.o
	$(CC) $(FLAGS) $(EXTRACFAGS) $(YAELCFLAGS) -lm

clustering: clustering.o common.o
	$(CC) $(FLAGS) $(EXTRACFAGS) $(YAELCFLAGS) -lm

HB: HB.o common.o cost.o clustering.o heap.o
	$(CC) $(FLAGS) $(EXTRACFAGS) $(YAELCFLAGS) -lm

HBPlus: HBPlus.o common.o cost.o heap.o clustering.o
	$(CC) $(FLAGS) $(EXTRACFAGS) $(YAELCFLAGS) -lm

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@ $(FLAGS) $(EXTRACFLAGS) $(YAELCFLAGS) -lm

main: main.o HB.o HBPlus.o common.o cost.o clustering.o heap.o
	$(CC) -o $@ $^ $(LDFLAGS) $(LAPACKLDFLAGS) $(THREADLDFLAGS) $(YAELLDFLAGS) -lm


#############################
# Dependencies

# for i in *.c ; do cpp -I.. -MM $i; done

cost.o: cost.c cost.h
common.o: common.c common.h $(YAELPATH)/vector.h
heap.o: heap.c heap.h common.h
clustering.o: clustering.c clustering.h common.h $(YAELPATH)/vector.h $(YAELPATH)/machinedeps.h $(YAELPATH)/kmeans.h
HB.o: HB.h HB.c common.h cost.h clustering.h heap.o $(YAELPATH)/vector.h
HBPlus.o: HBPlus.h HBPlus.c common.h cost.h heap.h clustering.h $(YAELPATH)/vector.h
main.o: main.c HB.h HBPlus.h common.h cost.h clustering.h heap.h $(YAELPATH)/vector.h $(YAELPATH)/machinedeps.h

clean:
	rm -f  *.o common HB HBPlus main cost clustering heap
	rm -f  *.gch common HB HBPlus main cost clustering heap


