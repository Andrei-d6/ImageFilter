#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "helper.h"
float A[] = { 1, 1, 1, 1, 1, 1, 1, 1, 1}; // * 1/9  Smooth
float B[] = { 1, 2, 1, 2, 4, 2, 1, 2, 1}; // * 1/16 Gaussian Blur
float C[] = { 0,-2, 0,-2,11,-2, 0,-2, 0}; // * 1/3  Sharpen
float D[] = {-1,-1,-1,-1, 9,-1,-1,-1,-1}; //	    Mean removal
float E[] = { 0, 1, 0, 0, 0, 0, 0,-1, 0}; // 		Emboss

//Smoothing filter
unsigned char smooth(unsigned char * aux, int maxval) 
{	
	float result = 0;
	unsigned char final_result;

	for(int i = 0; i < 9; i++) {
		result += ((aux[i]) * (A[8-i] / 9.0f));
	}

	if(result > maxval)
		result = maxval;

	if(result < 0)
		result = 0;

	final_result = (unsigned char)result;
	return final_result;
}

//Approximative Gaussian Blur filter
unsigned char gaussianBlur(unsigned char * aux, int maxval) 
{	
	float result = 0;
	unsigned char final_result;

	for(int i = 0; i < 9; i++) {
		result += ((aux[i]) * (B[8-i] / 16.0f));
	}

	if(result > maxval)
		result = maxval;

	if(result < 0)
		result = 0;

	final_result = (unsigned char)result;
	return final_result;
}


//Sharpen filter
unsigned char sharpen(unsigned char * aux, int maxval) 
{	
	float result = 0;
	unsigned char final_result;

	for(int i = 0; i < 9; i++) {
		result += aux[i] * (C[8-i] / 3.0f);
	}

	if(result > maxval)
		result = maxval;

	if(result < 0)
		result = 0;

	final_result = (unsigned char)result;
	return final_result;
}


//Mean removal filter
unsigned char meanRemoval(unsigned char * aux, int maxval) 
{	
	float result = 0;
	unsigned char final_result;

	for(int i = 0; i < 9; i++) {
		result += aux[i] * D[8-i];
	}

	if(result > maxval)
		result = maxval;

	if(result < 0)
		result = 0;

	final_result = (unsigned char)result;
	return final_result;
}

//Emboss filter
unsigned char emboss(unsigned char * aux, int maxval) 
{	
	float result = 0;
	unsigned char final_result;

	for(int i = 0; i < 9; i++) {
		result += aux[i] * E[8-i];
	}

	if(result > maxval)
		result = maxval;

	if(result < 0)
		result = 0;

	final_result = (unsigned char)result;
	return final_result;
}

void filterBwImage(int width, int height, int maxval, BW * bwImage, BW * bwFilteredImage, char *filter)
{	

	/*
	A - Smoothing filter
	B - Approximative Gaussian Blur filter
	C - Sharpen
	D - Mean removal
	E - Emboss
	*/

	unsigned char *aux = (unsigned char*)malloc(3*3*sizeof(unsigned char));
	int index;

	//Parcurgerea "matriceala" a vectorului de pixeli
	for(int i = 0; i < height; i++) {
		for(int j = 0; j < width; j++) {
			
			index = 0;
			int left, right;
			int up, down;

			//Extremitatile coordonatelor vecinilor
			left  = j-1;
			right = j+1;
			up 	  = i-1;
			down  = i+1; 

			//Parcurgerea vecinilor pixelului curent
			for(int i2 = up; i2 <= down; i2++) {
				for(int j2 = left; j2 <= right; j2++) {
					if(i2 >= 0 && j2 >= 0 && i2 < height && j2 < width)
						aux[index] = bwImage[i2*width + j2].pixel;
					else
						aux[index] = 0;
					index++;
				}
			}

			//Aplicarea filtrului corespunzator
			if(strcmp(filter, "smooth") == 0) {
				bwFilteredImage[i*width + j].pixel = smooth(aux, maxval);

			} else if(strcmp(filter, "blur") == 0) {
				bwFilteredImage[i*width + j].pixel = gaussianBlur(aux, maxval);

			} else if(strcmp(filter, "sharpen") == 0) {
				bwFilteredImage[i*width + j].pixel = sharpen(aux, maxval);

			} else if(strcmp(filter, "mean") == 0) {
				bwFilteredImage[i*width + j].pixel = meanRemoval(aux, maxval);

			} else if(strcmp(filter, "emboss") == 0) {
				bwFilteredImage[i*width + j].pixel = emboss(aux, maxval);

			} else {
				memcpy(bwFilteredImage, bwImage, width*height);
				free(aux);
				return;
			}
		}
	}

	//Eliberarea memoriei folosite
	free(aux);
}

void filterRgbImage(int width, int height, int maxval, RGB * rgbImage, RGB * rgbFilteredImage, char *filter)
{	

	/*
	A - Smoothing filter
	B - Approximative Gaussian Blur filter
	C - Sharpen
	D - Mean removal
	E - Emboss
	*/

	unsigned char *auxR = (unsigned char*)malloc(3*3*sizeof(unsigned char));
	unsigned char *auxG = (unsigned char*)malloc(3*3*sizeof(unsigned char));
	unsigned char *auxB = (unsigned char*)malloc(3*3*sizeof(unsigned char));
	int index;

	//Parcurgerea "matriceala" a vectorului de pixeli
	for(int i = 0; i < height; i++) {
		for(int j = 0; j < width; j++) {
			
			index = 0;
			int left, right;
			int up, down;
			
			//Extremitatile coordonatelor vecinilor
			left  = j-1;
			right = j+1;
			up 	  = i-1;
			down  = i+1; 

			//Parcurgerea vecinilor pixelului curent
			for(int i2 = up; i2 <= down; i2++) {
				for(int j2 = left; j2 <= right; j2++) {
					if(i2 >= 0 && j2 >= 0 && i2 < height && j2 < width) {
						auxR[index] = rgbImage[i2*width + j2].r;
						auxG[index] = rgbImage[i2*width + j2].g;
						auxB[index] = rgbImage[i2*width + j2].b;
					}
					else {
						auxR[index] = 0;
						auxG[index] = 0;
						auxB[index] = 0;
					}
					index++;
				}
			}

			//Aplicarea filtrului corespunzator
			if(strcmp(filter, "smooth") == 0) {
				rgbFilteredImage[i*width + j].r = smooth(auxR, maxval);
				rgbFilteredImage[i*width + j].g = smooth(auxG, maxval);
				rgbFilteredImage[i*width + j].b = smooth(auxB, maxval);

			} else if(strcmp(filter, "blur") == 0) {
				rgbFilteredImage[i*width + j].r = gaussianBlur(auxR, maxval);
				rgbFilteredImage[i*width + j].g = gaussianBlur(auxG, maxval);
				rgbFilteredImage[i*width + j].b = gaussianBlur(auxB, maxval);

			} else if(strcmp(filter, "sharpen") == 0) {
				rgbFilteredImage[i*width + j].r = sharpen(auxR, maxval);
				rgbFilteredImage[i*width + j].g = sharpen(auxG, maxval);
				rgbFilteredImage[i*width + j].b = sharpen(auxB, maxval);

			} else if(strcmp(filter, "mean") == 0) {
				rgbFilteredImage[i*width + j].r = meanRemoval(auxR, maxval);
				rgbFilteredImage[i*width + j].g = meanRemoval(auxG, maxval);
				rgbFilteredImage[i*width + j].b = meanRemoval(auxB, maxval);

			} else if(strcmp(filter, "emboss") == 0) {
				rgbFilteredImage[i*width + j].r = emboss(auxR, maxval);
				rgbFilteredImage[i*width + j].g = emboss(auxG, maxval);
				rgbFilteredImage[i*width + j].b = emboss(auxB, maxval);

			} else {
				memcpy(rgbFilteredImage, rgbImage, width*height);
				free(auxR);
				free(auxG);
				free(auxB);
				return;
			}
		}
	}

	//Eliberarea memoriei folosite
	free(auxR);
	free(auxG);
	free(auxB);
}