CC = g++
#HEADERS = structure.h writeout.h

parser: parser.o structure.o
				$(CC) parser.o structure.o -o parser

parser.o: parser.cpp structure.h
				$(CC) -c parser.cpp

structure.o: structure.cpp structure.h
				$(CC) -c structure.cpp
