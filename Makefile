SHELL=bash
CC=gcc
CFLAGS="-O3"


all: oneway oneway-static

clean: 
	rm -rf oneway || :

oneway: oneway.c
	$(CC) $(CFLAGS) -o oneway oneway.c

# we use -w to disable the warning below (gcc loads NSS dynamiclly at run time)
# warning: Using 'getpwnam' in statically linked applications requires at runtime the shared libraries from the glibc version used for linking
oneway-static: oneway.c
	$(CC) $(CFLAGS) -w -static -s -o oneway-static oneway.c
