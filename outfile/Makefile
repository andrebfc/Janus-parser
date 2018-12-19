CC = g++

out: out.o msgqueue.o
				$(CC) out.o msgqueue.o -o out -lpthread

out.o: out.cpp msgqueue.h
				$(CC) -c out.cpp

msgqueue.o: msgqueue.cpp msgqueue.h
				$(CC) -c msgqueue.cpp
