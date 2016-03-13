SHELL:=/bin/bash -O extglob
XXD = xxd
CC = gcc
GNUARM = arm-elf-
CFLAGS = -Wextra -Wno-unused-parameter -Wall -Werror -Ofast -I/usr/local/include/ -L /usr/local/lib/ -I include/ -lcurl -lz -lusb-1.0 lib/partial.c lib/libflursch.c -F/Library/Frameworks -framework CoreFoundation -F/System/Library/PrivateFrameworks -framework MobileDevice

all:
		@echo Valid options :
		@echo '"make payload"'
		@echo '"make inject"'
		@echo '"make tether"'
		@echo '"make arm7go"'
		@echo '"make clean"'

inject:
		$(CC) utils/inject.c $(CFLAGS) -o utils/inject

tether:
		$(CC) utils/tether.c $(CFLAGS) -o utils/tether

arm7go:
		$(CC) utils/arm7go.c $(CFLAGS) -o utils/arm7go

clean:
		rm -f utils/!(*.c)
		rm -rf include/steaks4uce.h

steaks4uce:
		$(GNUARM)as -march=armv6 -mthumb -o exploits/steaks4uce_payload.o exploits/steaks4uce.S
		$(GNUARM)objcopy -O binary exploits/steaks4uce_payload.o exploits/steaks4uce.payload
		$(XXD) -i exploits/steaks4uce.payload | sed 's/exploits_steaks4uce_payload/steaks4uce_payload/g' > include/steaks4uce.h
		rm -rf exploits/steaks4uce_payload.o
		rm -rf exploits/steaks4uce.payload

limera1n:
		$(GNUARM)as -mthumb -o exploits/limera1n_payload.o exploits/limera1n.S
		$(GNUARM)objcopy -O binary exploits/limera1n_payload.o exploits/limera1n.payload
		$(XXD) -i exploits/limera1n.payload | sed 's/exploits_limera1n_payload/limera1n_payload/g' > include/limera1n.h
		rm -rf exploits/limera1n_payload.o
		rm -rf exploits/limera1n.payload