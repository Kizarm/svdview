PRJ  = hello

VPATH  = . ./common/
BLD    = ./bld/
DFLAGS = -d
LFLAGS = -Wl,-Map=$(@:%.elf=%.map),-gc-sections
LDLIBS = -L./common -T script.ld
BFLAGS = --strip-unneeded

CFLAGS = -MMD -Wall -g -fno-exceptions -ffunction-sections -fdata-sections
#CFLAGS+= -fshort-enums
CFLAGS+= -I. -I./common
CCPU   = -mcpu=cortex-m0
MCPU   = -mthumb $(CCPU)

include Makefile.clang
#include Makefile.gcc

all: $(BLD) $(PRJ).elf
# zdrojaky
OBJS   = main.o system.o usart.o gpio.o
OBJS  += STM32F0x1_startup.o
OBJS  += abi.o
BOBJS = $(addprefix $(BLD),$(OBJS))

# ... atd.
# linker
$(BLD)main.o: STM32F0x1.h
$(BLD)system.o: STM32F0x1.h
$(BLD)usart.o: STM32F0x1.h
$(BLD)gpio.o: STM32F0x1.h
-include $(BLD)*.d
# místo MAKE v adresáři get lze jednoduše použít "unzip svd.zip", výrobce tam má stejně chybu.
# Je to tady takhle, protože je tam adresář STM, odkud lze SVD získat. Blbě se to hledá.
STM32F0x1.h STM32F0x1_startup.c: ../../cmsis
	@echo wget from STMicro
	cd get && $(MAKE) all
	../../cmsis STM32F0x1.svd -new
../../cmsis:
	cd ../../ && $(MAKE) -j4 all
$(PRJ).elf: $(BOBJS)
	-@echo [LD $(COM)] $@
	@$(LD) $(LFLAGS) -o $(PRJ).elf $(BOBJS) $(LDLIBS)
	-@echo "size:"
	$(SIZE) $(PRJ).elf
	-@echo "listing:"
	$(DUMP) $(DFLAGS) $(PRJ).elf > $(PRJ).lst
	-@echo "OK."
	$(COPY) $(BFLAGS) -O binary $(PRJ).elf $(PRJ).bin
# preloz co je potreba
$(BLD)%.o: %.c
	-@echo [CC $(COM)] $@
	@$(CC) -c $(CFLAGS) $< -o $@
$(BLD)%.o: %.cpp
	-@echo [CX $(COM)] $@
	@$(CX) -std=c++14 -fno-rtti -c $(CFLAGS) $< -o $@
$(BLD):
	mkdir $(BLD)
# vycisti
clean:
	$(DEL) $(BLD)* *.lst *.bin *.elf *.map *~ STM32F0x1.svd
	cd get && $(MAKE) clean
.PHONY: all clean
