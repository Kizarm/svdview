PRJ = cmsis
MLB = libcmsis.a
AR  = ar
CC  = gcc
CXX = g++
DEL = rm -f
BLD = ./build/
#DEL = del
VPATH = . ./tinyxml2
PRINTER = PrinterHpp

CFLAGS = -MMD -Wall -Os -DPRINTER="$(PRINTER)"
#CFLAGS+= -fno-exceptions
#CFLAGS+= -Wa,-adhlns=$(@:%.o=%.lst)

all: $(PRJ)
$(BLD):
	mkdir $(BLD)
-include $(BLD)*.d
# zdrojaky
OBJS = cmsis.o tinyxml2.o parser.o utils.o
OBJS+= cmsistree.o devicetree.o stm32hack.o
OBJS+= startup_template.o
OBJS+= $(PRINTER).o
# ... atd.
BOBJS = $(addprefix $(BLD),$(OBJS))
# linker
$(MLB): $(BLD) $(BOBJS)
	$(AR) rcs $(MLB) $(BOBJS) 
$(PRJ): $(MLB) main.cpp
	$(CXX) -Wall -Os -o $(PRJ) main.cpp -L. -lcmsis
# preloz co je potreba
$(BLD)%.o: %.s
	as  $< -o $@
$(BLD)%.o: %.c
	$(CC) -std=gnu17 -c $(CFLAGS) $< -o $@
$(BLD)%.o: %.cpp
	$(CXX) -std=c++17 -c $(CFLAGS) $< -o $@
# vycisti
clean:
	$(DEL) $(BLD)* *.lst *~ $(MLB)
.PHONY: all clean
