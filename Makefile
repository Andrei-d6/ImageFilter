CC = mpicc -Wall
PROGRAM_NAME = tema3
all: build

build: tema3

tema3: main.o readWriteImage.o filter.o
	$(CC) -o tema3 $^

main.o: main.c helper.h
	$(CC) -c main.c

readWriteImage.o: readWriteImage.c helper.h
	gcc -c readWriteImage.c

filter.o: filter.c helper.h
	gcc -c filter.c

clean:
	rm *.o $(PROGRAM_NAME)
