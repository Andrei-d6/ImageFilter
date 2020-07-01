#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "helper.h"
#define SIZE	100

//Citirea imaginii, intoarce 5 pentru PGM si 6 pentru PNM (-1 pentru alt format)
int readImage(char *file, int *width, int *height, int *maxval, BW **bwImage, RGB **rgbImage)
{	
	/*
	PGM
	P5
	width height
	maxval
	height * widht * 1  - Poza alb-negru


	PNM
	P6
	width height
	maxval
	height * widht * 3  - Poza color
	*/

	//File descriptor
	FILE *fd;
	//Input buffer
	char buffer[3];
	char comment[SIZE];
	char c;

	//Numar de caractere citite la un moment dat
	int n;

	fd = fopen(file, "r");
	if(fd == 0)
		printf("File could not be openend\n");

	n = fscanf(fd, "%[^\n]", buffer);
	n = fscanf(fd, "%c", &c);

	//Citirea formatului fisierului
	if(!(strcmp(buffer, "P5") == 0 || strcmp(buffer, "P6") == 0))
		printf("Invalid file\n");

	//Tratarea cazului in care inputul contine o linie de forma #Created by GIMP...
	//fgets(comment, SIZE, fd);
	n = fscanf(fd, "%c", &c);
	while(c == '#') {
		n = fscanf(fd, "%[^\n]", comment);
		n = fscanf(fd, "%c", &c);
	}
	ungetc(c, fd);

	//Citirea dimensiunilor imaginii
	n  = fscanf(fd,"%d %d\n", width, height);

	//Marimea imaginii
	long size = (*width) * (*height);

	//Citirea valorii maxime
	n  = fscanf(fd,"%d\n", maxval);

	if(buffer[0] == 'P' && buffer[1] == '5') {

		//Alocare de memorie pentru width * height octeti
		*bwImage = (BW*)malloc(size*sizeof(BW));

		//Citirea continutului imaginii
		fread(*bwImage, sizeof(BW), size, fd);

		fclose(fd);
		//Imaginea a fost de tip PGM
		return 5;

	} else if(buffer[0] == 'P' && buffer[1] == '6') {
		
		//Alocarea de memorie pentru widht * height * 3 octeti
		*rgbImage = (RGB*)malloc(size*sizeof(RGB));

		//Citirea continutului imaginii
		fread(*rgbImage, sizeof(RGB), size, fd);

		//Imaginea a fost de tip PNM
		fclose(fd);
		return 6;

	}
	return -1;
}


//Scrierea imaginii de iesire
void writeImage(char *file, int width, int height, int maxval, int imageType, BW *bwImage, RGB *rgbImage)
{	
	//File descriptor
	FILE *fd;
	fd = fopen(file, "wb+");

	/*
	PGM
	P5
	width height
	maxval
	height * widht * 1  - Poza alb-negru


	PNM
	P6
	width height
	maxval
	height * widht * 3  - Poza color
	*/

	if(imageType == 5) {
		fprintf(fd, "P5\n");
		fprintf(fd, "%d %d\n%d\n", width, height, maxval);
		//Scrierea imaginii
		fwrite(bwImage, sizeof(BW), width*height, fd);
		//fprintf(fd, "\n");

	} else {
		fprintf(fd, "P6\n");
		fprintf(fd, "%d %d\n%d\n", width, height, maxval);

		//Scrierea imaginii
		fwrite(rgbImage, sizeof(RGB), width*height, fd);
		//fprintf(fd, "\n");
	}

	fclose(fd);
}