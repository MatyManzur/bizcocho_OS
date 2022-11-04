include Makefile.inc

KERNEL=kernel.bin
SOURCES=$(wildcard c/*/*.c)
SOURCES_ASM=$(wildcard asm/*.asm)
OBJECTS=$(SOURCES:.c=.o)
OBJECTS_ASM=$(SOURCES_ASM:.asm=.o)
LOADERSRC=loader.asm
GCCDEFS=
LOADEROBJECT=$(LOADERSRC:.asm=.o)
STATICLIBS=

buddy: GCCDEFS= -D BUDDY
buddy: all

all:	$(LOADEROBJECT) $(OBJECTS) $(STATICLIBS) $(OBJECTS_ASM)
	$(LD) $(LDFLAGS) -T kernel.ld -o $(KERNEL) $(LOADEROBJECT) $(OBJECTS) $(OBJECTS_ASM) $(STATICLIBS)
	$(LD) $(LDFLAGS) -T kernel.ld --oformat=elf64-x86-64 $(LOADEROBJECT) $(OBJECTS) $(OBJECTS_ASM) $(STATICLIBS) -o kernel.elf

%.o: %.c
	$(GCC) $(GCCFLAGS) $(GCCDEFS) -I./include -c $< -o $@

%.o : %.asm
	$(ASM) $(ASMFLAGS) $< -o $@

$(LOADEROBJECT):
	$(ASM) $(ASMFLAGS) $(LOADERSRC) -o $(LOADEROBJECT)

clean:
	rm -rf asm/*.o c/*/*.o *.o *.bin 

.PHONY: all clean
