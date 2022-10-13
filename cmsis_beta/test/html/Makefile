CMS = ./bin/cmsis
PRJ = $(CMS).js
CC  = emcc
CXX = em++
DEL = rm -f
BLD = ./bld/
#DEL = del
VPATH = . ../.. ../../tinyxml2
PRINTER = PrinterHTML

CFLAGS = -MMD -Wall -Oz -DPRINTER="$(PRINTER)"
CFLAGS+= -I. -I../..
#CFLAGS+= -Wa,-adhlns=$(@:%.o=%.lst)
LFLAGS = -O3 --bind -s WASM=1 -s ALLOW_MEMORY_GROWTH=1
LFLAGS+= -s EXPORTED_RUNTIME_METHODS=['addOnPostRun'] --no-heap-copy
#LDLIBS = --preload-file ./files

all: $(PRJ)
$(BLD):
	mkdir $(BLD)
-include $(BLD)*.d
# zdrojaky
OBJS = html.o
OBJS+= tinyxml2.o parser.o utils.o
OBJS+= cmsistree.o devicetree.o stm32hack.o
OBJS+= decompress.o deflate.o data.o
#OBJS+= startup_template.o
OBJS+= $(PRINTER).o
# ... atd.
BOBJS = $(addprefix $(BLD),$(OBJS))
# linker
$(PRJ): $(BLD) $(BOBJS)
	$(CXX) $(LFLAGS) -o $(PRJ) $(BOBJS) $(LDLIBS)
# preloz co je potreba
$(BLD)%.o: %.s
	as  $< -o $@
$(BLD)%.o: %.c
	$(CC) -std=gnu17 -c $(CFLAGS) $< -o $@
$(BLD)%.o: %.cpp
	$(CXX) -std=c++17 -c $(CFLAGS) $< -o $@
# vycisti
clean:
	$(DEL) $(BLD)* *.lst *~ $(PRJ) $(CMS).wasm
.PHONY: all clean
