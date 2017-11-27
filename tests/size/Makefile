VIEW = ../../svdview

all: size_c size_cpp

size_c: size.c ../test.h
	gcc -Os -I.. size.c -o size_c
size_cpp: size.c ../test.h
	g++ -std=c++11 -Os -I.. size.c -o size_cpp
../test.h:
	$(VIEW) ../test.svd ../test.h

clean:
	rm -f size_c size_cpp
