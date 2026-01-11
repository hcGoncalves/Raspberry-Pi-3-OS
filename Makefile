CFILES = $(wildcard src/*.c)
ASMFILES = $(wildcard src/*.S)
OFILES = $(patsubst src/%.c, build/%.o, $(CFILES))
OSMFILES = $(patsubst src/%.S, build/%.o, $(ASMFILES))

LIBFDTFILES = $(wildcard libfdt/*.c)
OLIBFDTFILES = $(patsubst libfdt/%.c, build/%.o, $(LIBFDTFILES))


GCCFLAGS = -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles -mstrict-align -Iinclude -Ilibfdt/include
GCCPATH = C:/Users/Henrique/Documents/arm-gnu-toolchain-14.3.rel1-mingw-w64-x86_64-aarch64-none-elf/bin

all: clean kernel8.img

#%.o: %.s
#	$(GCCPATH)/aarch64-none-elf-gcc.exe $(GCCFLAGS) -c $< -o $@

#%.o: %.c
#	$(GCCPATH)/aarch64-none-elf-gcc.exe $(GCCFLAGS) -c $< -o $@

build/%.o: src/%.S
	$(GCCPATH)/aarch64-none-elf-gcc.exe $(GCCFLAGS) -c $< -o $@


build/%.o: src/%.c
	$(GCCPATH)/aarch64-none-elf-gcc.exe $(GCCFLAGS) -c $< -o $@

build/%.o: libfdt/%.c
	$(GCCPATH)/aarch64-none-elf-gcc.exe $(GCCFLAGS) -c $< -o $@

kernel8.img: $(OSMFILES) $(OFILES) $(OLIBFDTFILES)
	$(GCCPATH)/aarch64-none-elf-ld.exe -nostdlib $(OSMFILES) $(OFILES) $(OLIBFDTFILES) -T link.ld -o kernel8.elf
	$(GCCPATH)/aarch64-none-elf-objcopy.exe -O binary kernel8.elf kernel8.img

clean:
	/bin/rm kernel8.elf *.o *.img > /dev/null 2> /dev/null || true