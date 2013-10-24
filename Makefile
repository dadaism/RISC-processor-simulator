CC = gcc
CFLAGS = -O3

all:	main

clean:
	-rm bin/mips_asm
	-rm bin/mips_emulator
	-rm bin/mips_emulator_forwarding

main:
	$(CC) $(CFLAGS) -o bin/mips_asm src/mips_asm.c
	$(CC) $(CFLAGS) -o bin/mips_emulator src/mips_emulator.c
	$(CC) $(CFLAGS) -o bin/mips_emulator_forwarding src/mips_emulator_forwarding.c
