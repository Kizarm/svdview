PRJ = cmsis
CC  = clang
CXX = clang++
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
OBJS = main.o tinyxml2.o parser.o utils.o
OBJS+= cmsistree.o devicetree.o stm32hack.o
OBJS+= startup_template.o
OBJS+= $(PRINTER).o
# ... atd.
BOBJS = $(addprefix $(BLD),$(OBJS))
# linker
$(PRJ): $(BLD) $(BOBJS)
	$(CXX) -o $(PRJ) $(BOBJS)
# preloz co je potreba
$(BLD)%.o: %.s
	as  $< -o $@
$(BLD)%.o: %.c
	$(CC) -std=gnu17 -c $(CFLAGS) $< -o $@
$(BLD)%.o: %.cpp
	$(CXX) -std=c++17 -c $(CFLAGS) $< -o $@
# vycisti
clean:
	$(DEL) $(BLD)* *.lst *~
.PHONY: all clean
