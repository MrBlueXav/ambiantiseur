#
#       !!!! Do NOT edit this makefile with an editor which replace tabs by spaces !!!!    
#
##############################################################################################
# 
# On command line:
#
# make all = Create project
#
# make clean = Clean project files.
#
# To rebuild project do "make clean" and "make all".
#

##############################################################################################
# Start of default section
#

TRGT = arm-none-eabi-
CC   = $(TRGT)gcc
CP   = $(TRGT)objcopy
AS   = $(TRGT)gcc -x assembler-with-cpp
BIN  = $(CP) -O ihex 

MCU  = cortex-m4

# List all default C defines here, like -D_DEBUG=1
DDEFS = -D__arm__ -D__ASSEMBLY__ -DUSE_STDPERIPH_DRIVER -DSTM32F4XX -DHSE_VALUE=8000000 -D__FPU_PRESENT=1  -DSTM32F407VG

# List all default ASM defines here, like -D_DEBUG=1
DADEFS = -D__arm__ -D__ASSEMBLY__ -DUSE_STDPERIPH_DRIVER -DSTM32F4XX -DHSE_VALUE=8000000 -D__FPU_PRESENT=1  -DSTM32F407VG

# List all default directories to look for include files here
DINCDIR = 

# List the default directory to look for the libraries here
DLIBDIR = C:\GNUToolsARMEmbedded\4.7-2012q4\arm-none-eabi\lib\armv7e-m\fpu

# List all default libraries here
DLIBS = -lgcc -lc -lm

#
# End of default section
##############################################################################################

##############################################################################################
# Start of user section
#

# 
# Define project name and Ram/Flash mode here
PROJECT        = ambiantiseur
HEAP_SIZE      = 8192
STACK_SIZE     = 2048

#
# Define linker script file here
#
LDSCRIPT = ./prj/link.ld
FULL_PRJ = $(PROJECT)_flash


#
# Define FPU settings here
#
FPU = -mfloat-abi=hard -mfpu=fpv4-sp-d16 -D__FPU_USED=1

# List all user C define here, like -D_DEBUG=1
UDEFS = 

# Define ASM defines here
UADEFS = 

# List C source files here
SRC  = ./src/main.c \
./src/minblep_tables.c \
./src/phaser2.c \
./src/pitchTables.c \
./src/sequences.c \
./src/stm32f4_discovery.c \
./src/stm32f4_discovery_audio_codec.c \
./src/stm32f4xx_it.c \
./src/synth.c \
./cmsis/device/misc.c \
./cmsis/device/stm32f4xx_adc.c \
./cmsis/device/stm32f4xx_dac.c \
./cmsis/device/stm32f4xx_dma.c \
./cmsis/device/stm32f4xx_exti.c \
./cmsis/device/stm32f4xx_gpio.c \
./cmsis/device/stm32f4xx_i2c.c \
./cmsis/device/stm32f4xx_rcc.c \
./cmsis/device/stm32f4xx_rng.c \
./cmsis/device/stm32f4xx_rtc.c \
./cmsis/device/stm32f4xx_spi.c \
./cmsis/device/stm32f4xx_syscfg.c \
./cmsis/device/stm32f4xx_tim.c \
./cmsis/device/system_stm32f4xx.c

# List ASM source files here
ASRC = ./cmsis/device/startup_stm32f4xx.s

# List all user directories here
UINCDIR = ./inc \
          ./cmsis/core \
          ./cmsis/device

# List the user directory to look for the libraries here
ULIBDIR =

# List all user libraries here
ULIBS = 

# Define optimisation level here
OPT = -O2
#OPT = -O2 -falign-functions=16 -fno-inline -fomit-frame-pointer

#
# End of user defines
##############################################################################################


INCDIR  = $(patsubst %,-I%,$(DINCDIR) $(UINCDIR))
LIBDIR  = $(patsubst %,-L%,$(DLIBDIR) $(ULIBDIR))

DEFS    = $(DDEFS) $(UDEFS) -DRUN_FROM_FLASH=1


ADEFS   = $(DADEFS) $(UADEFS) -D__HEAP_SIZE=$(HEAP_SIZE) -D__STACK_SIZE=$(STACK_SIZE)
OBJS    = $(ASRC:.s=.o) $(SRC:.c=.o)
LIBS    = $(DLIBS) $(ULIBS)
MCFLAGS = -mthumb -mcpu=$(MCU) $(FPU)

ASFLAGS  = $(MCFLAGS) $(OPT) -g -gdwarf-2 -Wa,-amhls=$(<:.s=.lst) $(ADEFS)

CPFLAGS  = $(MCFLAGS) $(OPT) -gdwarf-2 -Wall -Wstrict-prototypes -fverbose-asm 
CPFLAGS += -ffunction-sections -fdata-sections -Wa,-ahlms=$(<:.c=.lst) $(DEFS)

LDFLAGS  = $(MCFLAGS) -nostartfiles -T$(LDSCRIPT) -Wl,-Map=$(FULL_PRJ).map,--cref,--gc-sections $(LIBDIR)

# Generate dependency information
CPFLAGS += -MD -MP -MF .dep/$(@F).d

#--------------------------------------------------------------------------------------------------------------
# makefile rules
#--------------------------------------------------------------------------------------------------------------

all: $(OBJS) $(FULL_PRJ).elf $(FULL_PRJ).hex size


%.o : %.c
	$(CC) -c $(CPFLAGS) -I . $(INCDIR) $< -o $@

%.o : %.s
	$(AS) -c $(ASFLAGS) $< -o $@

%elf: $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) $(LIBS) -o $@ 
  
%hex: %elf
	$(BIN) $< $@

size : $(FULL_PRJ).elf
	@echo 'Invoking: ARM Windows GNU Print Size'
	arm-none-eabi-size  --format=berkeley -t $(FULL_PRJ).elf
	@echo 'Finished building: $@'
	@echo ' '

clean:
	rm -f $(OBJS)
	rm -f $(FULL_PRJ).elf
	rm -f $(FULL_PRJ).map
#	rm -f $(FULL_PRJ).hex
	rm -f $(SRC:.c=.c.bak)
	rm -f $(SRC:.c=.lst)
	rm -f $(ASRC:.s=.s.bak)
	rm -f $(ASRC:.s=.lst)
	rm -fR .dep

# 
# Include the dependency files, should be the last of the makefile
#
-include $(shell mkdir .dep 2>/dev/null) $(wildcard .dep/*)

# *** EOF ***