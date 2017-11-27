# create headers
VIEW = ../../svdview
t1.c: ../test.h
t2.cpp: ../test.h
t3.c: ../stat.h ../defs.h
../test.h:
	$(VIEW) ../test.svd ../test.h
../stat.h:
	$(VIEW) -n ../test.svd ../stat.h
../defs.h:
	$(VIEW) -nd ../test.svd ../defs.h
