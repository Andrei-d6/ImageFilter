#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>
#include "helper.h"
#define  MASTER		0

int main(int argc, char **argv) 
{	
	//Imaginea de intrare si imagea filtrata pentru PGM
	BW *bwImage = NULL, *bwFilteredImage = NULL;
	//Imaginea de intrare si imagea filtrata pentru PNM
	RGB *rgbImage = NULL, *rgbFilteredImage = NULL;

	//Imaginea rezultata in urma aplicarii unui filtru
	BW *bwFinalImage = NULL;
	RGB *rgbFinalImage = NULL;

	//Tipul imaginii primite la intrare: PGM sau PNM
	int imageType;

	//Imaginea de intrare
	char *inputFile = argv[1];
	//Imaginea de iesire
	char *outputFile = argv[2];

	//Valoarea maxima a unui pixel
	int maxval;

	//Numarul de randuri asociate unui proces
	int rowsToFilter;

	//Dimensiunile originale ale imaginii
	int globalWidth, globalHeight;
	//Dimensiunile cu care lucreaza fiecare proces
	int width, height;
	
	// P - numarul de procese
	// rank - rank-ul fiecarui proces (MASTER este procestul cu rank-ul 0)
	int P, rank;
    
    //tag-ul folosit in comunicare
	int tag = 0;
    MPI_Status status;

    //Semafor pentru retinerea locului in care se afla imaginea finala
	// 0 -> imaginea finala se afla in FinalImage
	// 1 -> imaginea finala sa afla in image
	int finalImageLocation = 0;

	//Verificarea faputlui ca a fost aplicat cel putin un filtru
	int appliedFilter = 0;

    //Initializare
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &P);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	//Tratarea cazului in care exista un singur proces
	if(P == 1) {
		//Citirea imaginii data spre prelucrare
		imageType = readImage(inputFile, &globalWidth, &globalHeight, &maxval, &bwImage, &rgbImage);

		if(imageType == 5)
			bwFinalImage = (BW*)malloc(globalWidth*globalHeight*sizeof(BW));
		else if(imageType == 6)
			rgbFinalImage = (RGB*)malloc(globalWidth*globalHeight*3*sizeof(RGB));


		//Aplicarea filtrelor asupra imaginii
		for(int filter = 3; filter < argc; filter++) {
			appliedFilter = 1;
			
			/*
				Pentru folosirea eficienta a memoriei, spatiul de memorie folosit pentru
			    retinerea rezultatului aplicarii unui filtru va fi folosit ca intrare pentru 
			    aplicarea urmatorului filtru
			
				finalImageLocation == 0 => imaginea peste care trebuie aplicat filtrul curent
										   se afla in bwImage, respectiv rgbImage

				finalImageLocation == 1 => imaginea peste care trebuie aplicat filtrul curent
										   se afla in bwFinalImage, respectiv rgbFinalImage
			*/
			if(finalImageLocation == 0) {
				if(imageType == 5)
					filterBwImage(globalWidth, globalHeight, maxval, bwImage, bwFinalImage, argv[filter]);
				else if(imageType == 6)
					filterRgbImage(globalWidth, globalHeight, maxval, rgbImage, rgbFinalImage, argv[filter]);
			}
			else {

				if(imageType == 5)
					filterBwImage(globalWidth, globalHeight, maxval, bwFinalImage, bwImage, argv[filter]);	
				else 
					filterRgbImage(globalWidth, globalHeight, maxval, rgbFinalImage, rgbImage, argv[filter]);
			}
			finalImageLocation = (finalImageLocation + 1) % 2;
		}
		
		if(appliedFilter == 0) {
			//Daca nu a fost aplicat nici un filtru, imaginea noua va fi identica celei initiale
			writeImage(outputFile, globalWidth, globalHeight, maxval, imageType, bwImage, rgbImage);
		} else {

			if(finalImageLocation == 1) {
				writeImage(outputFile, globalWidth, globalHeight, maxval, imageType, bwFinalImage, rgbFinalImage);
			} else {
				writeImage(outputFile, globalWidth, globalHeight, maxval, imageType, bwImage, rgbImage);
			}	
		}
			
		//Eliberarea memoriei folosite
		if(imageType == 5) {
			free(bwImage);
			free(bwFinalImage);
		} else if(imageType == 6) {
			free(rgbImage);
			free(rgbFinalImage);
		}

		MPI_Finalize();
		return 0;
	}

	//Variabile pentru stabilirea sursei si destinatiei in comunicare
	int dest, source;
	//Ofsset-ul fata de primul pixel din imagine
	int offset;

	//Procestul MASTER (rank 0) se ocupa de citirea imaginii
	//precum si de trimiterea paramterilor initiali
	if(rank == MASTER) {
		//Citirea imaginii de intrare
		imageType = readImage(inputFile, &globalWidth, &globalHeight, &maxval, &bwImage, &rgbImage);
		
		//Trimiterea informatiilor initiale: width, height, maxval si imageType
		for(dest = 1; dest < P; dest++) {
			rowsToFilter = globalHeight/P;

			if(dest < globalHeight % P)
				rowsToFilter++;

			if(dest == P-1)
				rowsToFilter += 1;	//Ultimul proces primeste un fragment inferior al matricii
									//=> necesita un singur rand aditional (marginea inferioara ar fi 0)
			else
				rowsToFilter += 2; //Celelalte procese (fara MASTER) necesita
									//atat un rand suplimentar superior cat si inferior

			Pachet *sendBuffer = (Pachet*)malloc(sizeof(Pachet));
			sendBuffer->width = globalWidth;
			sendBuffer->height = rowsToFilter;
			sendBuffer->maxval = maxval;
			sendBuffer->imageType = imageType;
			MPI_Send(sendBuffer, 4, MPI_INT, dest, tag, MPI_COMM_WORLD);
		}

		//Determinarea propriilor parametrii (acelasi width dar numarul de randuri difera)
		width = globalWidth;
		height = globalHeight/P;
		if(globalHeight % P != 0)
			height++;
		//Primul proces necesita inca un rand suplimentar (marginea inferioara)
		height += 1;

		if(imageType == 5) {
			bwFinalImage = (BW*)malloc(globalWidth*globalHeight*sizeof(BW));
		}
		else if(imageType == 6) {
			rgbFinalImage = (RGB*)malloc(globalWidth*globalHeight*3*sizeof(RGB));
		}

	} else {
		//Receptionarea datelor initiale
		Pachet *recvBuffer = (Pachet*)malloc(sizeof(Pachet));
		MPI_Recv(recvBuffer, 4, MPI_INT, MASTER, tag, MPI_COMM_WORLD, &status);
		width = recvBuffer->width;		//latimea fragmentului ce urmeaza a fi primit (latimea originala a imaginii)
		height = recvBuffer->height;	//inaltimea fragmentului ce urmeaaza a fi primit (numarul de randuri total - cu tot cu randurile supliemntare)
		maxval = recvBuffer->maxval;	
		imageType = recvBuffer->imageType;	// tipul imaginii 5-PGM 6-PNM

		//Alocarea de memorie in functie de tipul imaginii
		if(imageType == 5) {
			bwImage = (BW*)malloc(width*height*sizeof(BW));
			bwFilteredImage = (BW*)malloc(width*height*sizeof(BW));
		} else if(imageType == 6) {
			rgbImage = (RGB*)malloc(width*height*3*sizeof(RGB));
			rgbFilteredImage = (RGB*)malloc(width*height*3*sizeof(RGB));
		}
		//printf("Process %d has received:\nWidth: %d\nHeight: %d\nMaxval: %d\nImageType: %d\n\n", rank, width, height, maxval, imageType);
	}

	for(int filter = 3; filter < argc; filter++) {
		appliedFilter = 1;
		
		//Trimiterea fragmentelor de imagine
		if(rank == MASTER) {
			//offset - offset-ul fata de inceputul imaginii de unde trebuie trimise randurile
			//	(la cate randuri fata de inceput se afla bucata de imagine ce trebuie trimisa unui anumit proces)
			offset = globalHeight/P;

			//in cazul in care inaltimea nu se imparte perfect la numarul de procese
			//ultimul proces ar avea de procesat mai multe randuri ale imaginii decat celelalte procese
			// => si procesul MASTER va lua un rand in plus
			if(globalHeight % P != 0)
				offset++;
			offset++;

			for(dest = 1; dest < P; dest++) {
				//rowsToFilter - numarul de randuri ce trebuie trimise catre procesare (aplicare de filtre)
				//se incearca o distribuire cat mai echilibrata a numarului de randuri
				rowsToFilter = globalHeight/P;

				//pentru a nu supraincarca ultimul proces ca numar de randuri ce trebuie procesate
				//procesele cu rank-ul < restul de randuri ce ar reveni ultimului proces, acestea vor lua un rand in plu
				if(dest < globalHeight % P)
					rowsToFilter++;

				if(dest == P-1)
					rowsToFilter += 1;	//ultimul proces necesita un rand aditional pentru procesare corecta
										//marginea superioara a randurilor ce ii revin
				else
					rowsToFilter += 2; /*
										celelalte procese (fara MASTER) necesita doua randuri aditionale penrtu 
										procesare corecta - randul imediat superior randurilor aferente respectivului
										prces, repsectiv randul imediat inferior acelor randuri
										*/
				//Alinierea pentru trimiterea fragmentului curent
				offset -= 2;
				
				//Trimiterea fragmentelor de imagine catre celelalte procese (in functie de tipul si locul imaginii)
				if(imageType == 5) {
					if(finalImageLocation == 0)
						MPI_Send(bwImage + offset*width, rowsToFilter*width, MPI_UNSIGNED_CHAR, dest, tag, MPI_COMM_WORLD);
					else
						MPI_Send(bwFinalImage + offset*width, rowsToFilter*width, MPI_UNSIGNED_CHAR, dest, tag, MPI_COMM_WORLD);
				}
				else if(imageType == 6) {
					if(finalImageLocation == 0)
						MPI_Send(rgbImage + offset*width, rowsToFilter*width*3, MPI_UNSIGNED_CHAR, dest, tag, MPI_COMM_WORLD);
					else
						MPI_Send(rgbFinalImage + offset*width, rowsToFilter*width*3, MPI_UNSIGNED_CHAR, dest, tag, MPI_COMM_WORLD);
				}

				//offset-ul se deplaseaza cu randurile corespunzatoare procesului curent
				offset += rowsToFilter;
			}
		} else {
			//Receptionarea fragmentelor de imagine
			if(imageType == 5) {
				MPI_Recv(bwImage, width*height, MPI_UNSIGNED_CHAR, MASTER, tag, MPI_COMM_WORLD, &status);
			}
			else if(imageType == 6) { 
				MPI_Recv(rgbImage, width*height*3, MPI_UNSIGNED_CHAR, MASTER, tag, MPI_COMM_WORLD, &status);
			}
		}

		//Aplicarea filtrelor asupra imaginii
		if(rank == MASTER) {
			//in functie de locul unde se afla imaginea peste care trebuie aplicat filtrul
			//MASTER decide ordinea corecta a parametrilor 
			// (rezultatul aplicarii filtrului sa se afle in locul unde vor fi receptionate fragmentele de la celelalte procese)
			if(finalImageLocation == 0) {
				if(imageType == 5)
					filterBwImage(width, height, maxval, bwImage, bwFinalImage, argv[filter]);
				else if(imageType == 6)
					filterRgbImage(width, height, maxval, rgbImage, rgbFinalImage, argv[filter]);
			}
			else {
				if(imageType == 5)
					filterBwImage(width, height, maxval, bwFinalImage, bwImage, argv[filter]);
				else if(imageType == 6)
					filterRgbImage(width, height, maxval, rgbFinalImage, rgbImage, argv[filter]);
			}
		} else { //celelalte procese vor calcula imaginea asupra careia a fost aplicat filtru in FilteredImage (bw sau rgb)
			if(imageType == 5)
				filterBwImage(width, height, maxval, bwImage, bwFilteredImage, argv[filter]);
			else if(imageType == 6)
				filterRgbImage(width, height, maxval, rgbImage, rgbFilteredImage, argv[filter]);
		}
		//locul unde se va forma imaginea finala se schimba dupa fiecare filtru (Image->FinalImage->Image->FinalImage...)
		finalImageLocation = (finalImageLocation + 1) % 2;


		//Trimiterea fragmentelor filtrate ale imaginii
		if(rank != MASTER) {
			if(rank != P-1) {
				/*
				Procesele care nu sunt MASTER si nici ultimul (ca rank) au primit 
				doua randuri suplimentare, astfel ca acestea detin height-2 informatie utila
				
				bwFilteredImage+width / rgbFilteredImage+width - se ignora primul rand
				(height-2)*width - nu se trimite si ultimul rand (tinand cont ca se trimite de la + width)
				*/
				if(imageType == 5)
					MPI_Send(bwFilteredImage + width, (height-2)*width, MPI_UNSIGNED_CHAR, MASTER, tag, MPI_COMM_WORLD);
				else if(imageType == 6)
					MPI_Send(rgbFilteredImage + width, (height-2)*width*3, MPI_UNSIGNED_CHAR, MASTER, tag, MPI_COMM_WORLD);
			
			} else if(rank == P-1) {

				/*
				Ultimul proces (cu rank-ul P-1) detine un rand suplimentar (cel aflat in imediata 
				vecinatate superioara a primului rand) astfel incat va trimite cu un rand mai putin

				bwFilteredImage+width / rgbFilteredImage+width - se ignora primul rand
				(height-1)*width - se trimite cu un rand mai putin
				*/
				if(imageType == 5)
					MPI_Send(bwFilteredImage + width, (height-1)*width, MPI_UNSIGNED_CHAR, MASTER, tag, MPI_COMM_WORLD);
				else if(imageType == 6)
					MPI_Send(rgbFilteredImage + width, (height-1)*width*3, MPI_UNSIGNED_CHAR, MASTER, tag, MPI_COMM_WORLD);
			}

		} else {
			//Procesul MASTER aduna fragmentele asupra carora a fost aplicat filtrul
			
			/*
			Conform logicii de distribuire, procesul MASTER determina cate randuri
			trebuie sa primeasca de la fiecare proces si locul acestora in imaginea finala
			*/
			offset = globalHeight/P;
			if(globalHeight % P != 0)
				offset++;

			for(source = 1; source < P; source++) {
				rowsToFilter = globalHeight/P;

				if(source < globalHeight % P)
					rowsToFilter++;

				if(imageType == 5) {

					if(finalImageLocation == 1)
						MPI_Recv(bwFinalImage + offset*width, rowsToFilter*width, MPI_UNSIGNED_CHAR, source, tag, MPI_COMM_WORLD, &status);
					else
						MPI_Recv(bwImage + offset*width, rowsToFilter*width, MPI_UNSIGNED_CHAR, source, tag, MPI_COMM_WORLD, &status);
				}
				else if(imageType == 6) {

					if(finalImageLocation == 1)
						MPI_Recv(rgbFinalImage + offset*width, rowsToFilter*width*3, MPI_UNSIGNED_CHAR, source, tag, MPI_COMM_WORLD, &status);
					else
						MPI_Recv(rgbImage + offset*width, rowsToFilter*width*3, MPI_UNSIGNED_CHAR, source, tag, MPI_COMM_WORLD, &status);
				}
				offset += rowsToFilter;
			}
		}
	}

	//Scrierea imaginii finale (dupa aplicarea filtrelor)
	if(rank == MASTER) {
		if(appliedFilter == 0)
			//In cazul in care nu a fost aplicat nici un filtru, se copiaza imaginea originala 
			writeImage(outputFile, globalWidth, globalHeight, maxval, imageType, bwImage, rgbImage);
		else {
			if(finalImageLocation == 1)
				writeImage(outputFile, globalWidth, globalHeight, maxval, imageType, bwFinalImage, rgbFinalImage);
			else
				writeImage(outputFile, globalWidth, globalHeight, maxval, imageType, bwImage, rgbImage);
		}
			
		//Eliberarea memoriei folosite
		if(imageType == 5) {
			free(bwImage);
			free(bwFinalImage);
		} else if(imageType == 6) {
			free(rgbImage);
			free(rgbFinalImage);
		}

	} else {

		//Eliberarea memoriei folosite
		if(imageType == 5) {
			free(bwImage);
			free(bwFilteredImage);
		} else if(imageType == 6) {
			free(rgbImage);
			free(rgbFilteredImage);
		}
	}
	
	MPI_Finalize();
	return 0;
}