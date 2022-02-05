KERNEL := ntoskrnl.elf

CC = cc
LD = ld
AS = nasm

CFLAGS = -O3 -pipe
LDFLAGS =

INTERNALLDFLAGS :=         \
	-Tlinker.ld            \
	-nostdlib              \
	-zmax-page-size=0x1000 \
	-static                \
	--no-dynamic-linker    \
	-ztext

INTERNALCFLAGS  :=       \
	-I.                  \
	-w	\
	-std=gnu11           \
	-ffreestanding       \
	-fno-stack-protector \
	-fno-pic -fpie       \
	-mno-80387           \
	-mno-mmx             \
	-mno-3dnow           \
	-mno-sse             \
	-mno-sse2            \
	-mno-red-zone 

CFILES := $(shell find ./ -type f -name '*.c' -not -path "initramfs/*")
ASMFILES = $(shell find ./ -type f -name '*.asm' -not -path "initramfs/*")
OBJ = $(CFILES:.c=.o)
OBJ += $(ASMFILES:.asm=_asm.o)

.PHONY: all clean

all: $(KERNEL)

stivale2.h:
	wget https://github.com/stivale/stivale/raw/master/stivale2.h

$(KERNEL): $(OBJ)
	$(LD) $(OBJ) $(LDFLAGS) $(INTERNALLDFLAGS) -o $@

%_asm.o: %.asm
	$(AS) $^ -f elf64 -o $@

%.o: %.c stivale2.h
	$(CC) $(CFLAGS) $(INTERNALCFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ)