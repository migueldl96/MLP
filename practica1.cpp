//============================================================================
// Introducción a los Modelos Computacionales
// Name        : practica1.cpp
// Author      : Pedro A. Gutiérrez
// Version     :
// Copyright   : Universidad de Córdoba
//============================================================================

#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <unistd.h>
#include <iostream>
#include <ctime>    // Para cojer la hora time()
#include <cstdlib>  // Para establecer la semilla srand() y generar números aleatorios rand()
#include <string.h>
#include <math.h>
#include "imc/PerceptronMulticapa.h"
#include "imc/util.h"
#include "imc/stats.hpp"

#define ENTRIOPIA_CRUZADA 1
#define MSE 0
#define OFFLINE true
#define ONLINE false


using namespace imc;
using namespace std;

int main(int argc, char **argv) {
	// Argumentos
	char fichTrain[256], fichTest[256];
	bool sameTestAndTrain = true;
	int iteraciones = 1000;
	int numCapaOcultas = 5, numNeuronasCapa = 5;
	double eta = 0.1, mu = 0.9, validation = 0.0;
	int decremento = 1;
    double meanIterations = 0;
    bool trainMode = OFFLINE;
    int funcionError = MSE;
    bool softmaxOut = false;

	// Procesar los argumentos de la línea de comandos
	opterr=0;
	char c;
    while ((c = getopt (argc, argv, "t:T:i:l:h:e:m:v:d:ofs")) != -1)
    {
        switch (c)
        {
        case 't': // Nombre del fichero de TRAIN
        	strcpy(fichTrain, optarg);
        break;

        case 'T': // Nombre del fichero de TEST
        	strcpy(fichTest, optarg);
        	sameTestAndTrain = false;
        break;

        case 'i': // Número de iteraciones
        	if(atoi(optarg) > 0)
        		iteraciones = atoi(optarg);
        	else {
        		printf("WARNING: Las iteraciones debe ser positivas, utilizando valor 1000...\n");
        	}
        break;

        case 'l': // Número de capas ocultas
        	if(atoi(optarg) > 0)
        		numCapaOcultas = atoi(optarg);
        	else {
        		printf("WARNING: El número de capas ocultas debe ser positivo, utilizando valor 5...\n");
        	}
        break;

        case 'h': // Número de neuronas/capa
        	if(atoi(optarg) > 0)
        		numNeuronasCapa = atoi(optarg);
        	else {
        		printf("WARNING: El número de neuronas por capa debe ser positivo, utilizando valor 5...\n");
        	}
        break;

        case 'e': // Parámetro eta
        	if(atof(optarg) >= 0 && atoi(optarg) <=1)
        		eta = atof(optarg);
        	else {
        		printf("WARNING: El parámetro eta debe comprenderse en [0, 1], utilizando valor 0.1...\n");
        	}
        break;

        case 'm': // Parámetro mu
        	if(atof(optarg) >= 0 && atoi(optarg) <=1)
        		mu = atof(optarg);
        	else {
        		printf("WARNING: El parámetro mu debe comprenderse en [0, 1], utilizando valor 0.9...\n");
        	}
        break;

        case 'v': // Porcentaje de validación
        	if(atof(optarg) >= 0 && atoi(optarg) <=1)
        		validation = atof(optarg);
        	else {
        		printf("WARNING: El porcentaje de validación debe comprenderse en [0, 1], utilizando valor 0.0...\n");
        	}
        break;

        case 'd': // Factor de decremento
        	decremento = atoi(optarg);
        break;

        case 'o': // Online
            trainMode = ONLINE;
        break;

        case 'f': // Funcion de error = Entriopía cruzada
            funcionError = ENTRIOPIA_CRUZADA;
        break;

        case 's': // Softmax
            softmaxOut = true;
        break;


        // Algún error?
        case '?':
            if (optopt == 't')
                fprintf (stderr, "La opción %c requiere un argumento.\n", optopt);
            else if (isprint (optopt))
                fprintf (stderr, "Opción desconocida '-%c'.\n", optopt);
            else
                fprintf (stderr, "Caracter `\\x%x'.\n", optopt);
            return 1;
        default:
            abort ();
        }
    }
	// Objeto perceptrón multicapa
	PerceptronMulticapa mlp;

	// Parámetros del mlp
	mlp.dEta = eta;
	mlp.dMu = mu;
	mlp.dValidacion = validation;
	mlp.dDecremento = decremento;
    mlp.bOnline = trainMode;


	// Lectura de datos
	Datos * pDatosTrain, * pDatosTest;
	// Entrenamiento
	pDatosTrain = mlp.leerDatos(fichTrain);
	// Si test es igual a entrenamiento, nos ahorramos volver a leerlo
	(sameTestAndTrain) ? (pDatosTest = pDatosTrain) : (pDatosTest = mlp.leerDatos(fichTest));

	// Inicializar vector topología
	int *topologia = new int[numCapaOcultas+2];
	topologia[0] = pDatosTrain->nNumEntradas;
	for(int i=1; i<(numCapaOcultas+2-1); i++)
		topologia[i] = numNeuronasCapa;
	topologia[numCapaOcultas+2-1] = pDatosTrain->nNumSalidas;

	// Inicializar red con vector de topología
	mlp.inicializar(numCapaOcultas+2,topologia);

    // Semilla de los números aleatorios
    int semillas[] = {10,20,30,40,50};
    double *erroresTest = new double[5];
    double *erroresTrain = new double[5];
    double *ccrsTest = new double[5];
    double *ccrsTrain = new double[5];

    long double begin = clock();
    for(int i=0; i<5; i++){
    	cout << "**********" << endl;
    	cout << "SEMILLA " << semillas[i] << endl;
    	cout << "**********" << endl;
		srand(semillas[i]);
		mlp.ejecutarAlgoritmoOnline(pDatosTrain,pDatosTest,iteraciones,&(erroresTrain[i]),&(erroresTest[i]), fichTrain, &meanIterations, funcionError, ccrsTrain, ccrsTest);
		cout << "Finalizamos => Error de test final: " << erroresTest[i] << endl;
	}
    long double end = clock();
    long double tiempo = (end-begin)/(double)CLOCKS_PER_SEC;

    cout << "HEMOS TERMINADO TODAS LAS SEMILLAS" << endl;

    double mediaErrorTest = 0, desviacionTipicaErrorTest = 0;
    double mediaErrorTrain = 0, desviacionTipicaErrorTrain = 0;
    
    // Calcular medias y desviaciones típicas de entrenamiento y test
    mediaErrorTrain = mean(erroresTrain, 5);
    mediaErrorTest  = mean(erroresTest, 5);
    desviacionTipicaErrorTrain = sd(erroresTrain, 5);
    desviacionTipicaErrorTest  = sd(erroresTest, 5);

    cout << "INFORME FINAL" << endl;
    cout << "*************" << endl;
    cout << "Error de entrenamiento (Media +- DT): " << mediaErrorTrain << " +- " << desviacionTipicaErrorTrain << endl;
    cout << "Error de test (Media +- DT):          " << mediaErrorTest << " +- " << desviacionTipicaErrorTest << endl;
    cout << "Número de iteraciones medio:          " << meanIterations << endl;
    cout << "Tiempo necesario:                     " << tiempo << "s" << endl;
    return EXIT_SUCCESS;
}

