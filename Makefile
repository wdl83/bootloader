all: bootloader.Makefile
	make -f bootloader.Makefile

clean:
	rm *.bin -f
	rm *.elf -f
	rm *.hex -f
	rm *.lst -f
	rm *.map -f
