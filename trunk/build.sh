#!/bin/sh

DEBUG="-DDEBUG -g"

g++ ${DEBUG} -O2 -Wall -c -o CEzFlashBase.o CEzFlashBase.cc
g++ ${DEBUG} -O2 -Wall -c -o CEzFlashFujistu.o CEzFlashFujistu.cc
g++ ${DEBUG} -Wall -O2 -c -o ezopen.o ezopen.cc
g++ ${DEBUG} -lusb -o ezopen *.o
