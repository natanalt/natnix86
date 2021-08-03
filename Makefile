
NASM     ?= nasm
INCLUDES := include

NASMFLAGS = 					\
	-f bin 						\
	$(addprefix -I,$(INCLUDES))

OUTFILES = \
	out/boot.bin \
	out/kernel.bin

TARFILES = \
	kernel.bin

.PHONY: all clean prepdirs

all: prepdirs out out/live.flp

clean:
	-rm -r out
	-rm -r deps

prepdirs:
	@mkdir -p out
	@mkdir -p deps

out: $(OUTFILES)

out/boot.bin: boot/boot.asm
	$(NASM) $(NASMFLAGS) $< -o out/boot.bin -MD deps/boot.d

out/kernel.bin: kernel/kernel.asm
	$(NASM) $(NASMFLAGS) $< -o out/kernel.bin -MD deps/kernel.d

out/live.flp: out
	-cd out && rm live.tar
	cd out && tar -H ustar -cvf live.tar $(TARFILES)
	dd if=/dev/zero of=$@ bs=512 count=2880
	dd if=out/boot.bin of=$@ bs=512 conv=notrunc
	dd if=out/live.tar of=$@ bs=512 conv=notrunc seek=8

-include deps/boot.d
