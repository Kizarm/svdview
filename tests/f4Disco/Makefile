GPATH = ../..
GHEAD = stm32f40x.h
GFILE = STM32F40x.svd
all: $(GFILE) $(GPATH)/svdview
	$(GPATH)/svdview $(GFILE) $(GHEAD)
	patch -p0 <f4.patch
	mv $(GHEAD) ./inc/$(GHEAD)
$(GFILE):
	wget https://raw.githubusercontent.com/posborne/cmsis-svd/master/data/STMicro/STM32F40x.svd
$(GPATH)/svdview:
	cd $(GPATH) && qmake-qt4 && make -j4
