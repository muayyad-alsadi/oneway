SHELL=bash
CC=gcc
CFLAGS="-O3"


all: oneway

clean: 
	rm -rf oneway || :

oneway:
	$(CC) $(CFLAGS) -o oneway oneway.c

# you might need glibc-static
# warning: Using 'getpwnam' in statically linked applications requires at runtime the shared libraries from the glibc version used for linking
#oneway-static:
#	$(CC) -static -s -o oneway-static oneway.c
