src1 = *.cpp
src2 = *.c

all:
	g++ $(src1) $(src2) -o server
all2:
	g++ -c server.cpp
	g++ -c logging.c
	g++ -o server server.o logging.o
