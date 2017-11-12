/*********************************************************************
 * File  : PerceptronMulticapa.cpp
 * Date  : 2017
 *********************************************************************/

#include "PerceptronMulticapa.h"
#include "util.h"


#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>  // Para establecer la semilla srand() y generar números aleatorios rand()
#include <limits>
#include <math.h>

#define ENTRIOPIA_CRUZADA 1
#define MSE 0


using namespace imc;
using namespace std;
using namespace util;

// ------------------------------
// CONSTRUCTOR: Dar valor por defecto a todos los parámetros
PerceptronMulticapa::PerceptronMulticapa(){
 	dEta = 0.1;
 	dMu = 0.9;
 	dValidacion = 0.0;
 	dDecremento = 1;
 	bOnline = true;
}

// ------------------------------
// Reservar memoria para las estructuras de datos
int PerceptronMulticapa::inicializar(int nl, int npl[]) {
	int i, j, k;
	nNumCapas = nl;
	pCapas = new Capa[nl];
	// Recorremos cada capa
	for(i = 0; i < nl ; i++) {
		pCapas[i].nNumNeuronas = npl[i];
		pCapas[i].pNeuronas    = new Neurona[npl[i]];

		// Recorremos cada neurona para crear el vector de pesos (si no es la primera capa...)
		if (i>0) {
			for(j = 0; j < npl[i] ; j++){
				pCapas[i].pNeuronas[j].w = new double[pCapas[i-1].nNumNeuronas + 1]; // + 1 => Sesgo
				pCapas[i].pNeuronas[j].wCopia = new double[pCapas[i-1].nNumNeuronas + 1]; // + 1 => Sesgo
				pCapas[i].pNeuronas[j].deltaW = new double[pCapas[i-1].nNumNeuronas + 1];
				pCapas[i].pNeuronas[j].ultimoDeltaW = new double[pCapas[i-1].nNumNeuronas + 1];
				for(k=0;k<pCapas[i-1].nNumNeuronas + 1;k++) {
					pCapas[i].pNeuronas[j].deltaW[k] = 0;
					pCapas[i].pNeuronas[j].ultimoDeltaW[k] = 0;
				}
			}

		}

	}
	
	return 1;
}


// ------------------------------
// DESTRUCTOR: liberar memoria
PerceptronMulticapa::~PerceptronMulticapa() {
	liberarMemoria();

}


// ------------------------------
// Liberar memoria para las estructuras de datos
void PerceptronMulticapa::liberarMemoria() {
	int i, j;
	for(i=1;i<nNumCapas;i++) {
		for(j=0;j<pCapas[i].nNumNeuronas;j++) {
			free(pCapas[i].pNeuronas[j].w);
			free(pCapas[i].pNeuronas[j].wCopia);
			free(pCapas[i].pNeuronas[j].deltaW);
			free(pCapas[i].pNeuronas[j].ultimoDeltaW);
		}
		free(pCapas[i].pNeuronas);
	}
	free(pCapas);
}

// ------------------------------
// Rellenar todos los pesos (w) aleatoriamente entre -1 y 1
void PerceptronMulticapa::pesosAleatorios() {
	int i, j, k;
	double pesoAleatorio;
	// Por cada capa (exepto la de entrada)
	for(i = 1; i < nNumCapas; i++) {
		// Por cada neurona
		for(j = 0; j < pCapas[i].nNumNeuronas; j++) {
			// Por cada peso (neuronas de la capa anterior)
			for(k=0; k<pCapas[i-1].nNumNeuronas+1; k++) {
				pesoAleatorio = ((rand()/(double)RAND_MAX)*2) - 1;
				pCapas[i].pNeuronas[j].w[k] = pesoAleatorio;
			}
		}
	}

	// Asignamos el vector de pesos de la capa de entrada a NULL para evitar problemas
	for(i = 0; i < pCapas[0].nNumNeuronas; i++)
		pCapas[0].pNeuronas[i].w = NULL;


}

// ------------------------------
// Alimentar las neuronas de entrada de la red con un patrón pasado como argumento
void PerceptronMulticapa::alimentarEntradas(double* input) {
	int i;
	// Primera capa: salida = entradas, no hay pesos
	for(i=0; i<pCapas[0].nNumNeuronas; i++) {
		pCapas[0].pNeuronas[i].x = input[i];
	}
}

// ------------------------------
// Recoger los valores predichos por la red (out de la capa de salida) y almacenarlos en el vector pasado como argumento
void PerceptronMulticapa::recogerSalidas(double* output)
{
	int i;
	for(i=0;i<pCapas[nNumCapas-1].nNumNeuronas; i++)
		output[i] = pCapas[nNumCapas-1].pNeuronas[i].x;

}

// ------------------------------
// Hacer una copia de todos los pesos (copiar w en copiaW)
void PerceptronMulticapa::copiarPesos() {
	int i, j, k;

	for(i=1;i<nNumCapas;i++) {
		for(j=0;j<pCapas[i].nNumNeuronas;j++) {
			for(k=0;k<pCapas[i-1].nNumNeuronas+1;k++) {
				pCapas[i].pNeuronas[j].wCopia[k] = pCapas[i].pNeuronas[j].w[k];
			}
		}
	}
}

// ------------------------------
// Restaurar una copia de todos los pesos (copiar copiaW en w)
void PerceptronMulticapa::restaurarPesos() {
	int i, j, k;
	for(i=1;i<nNumCapas;i++) {
		for(j=0;j<pCapas[i].nNumNeuronas;j++) {
			for(k=0;k<pCapas[i-1].nNumNeuronas+1;k++) {
				pCapas[i].pNeuronas[j].w[k] = pCapas[i].pNeuronas[j].wCopia[k];
			}
		}
	}
}

// ------------------------------
// Calcular y propagar las salidas de las neuronas, desde la primera capa hasta la última
void PerceptronMulticapa::propagarEntradas() {
	int i, j, k;
	double net = 0.0;

	// Propagamos hasta la última capa
	for(i=1; i<nNumCapas-1; i++) {
		// Bucle para cada neurona de la capa actual
		for(j=0;j<pCapas[i].nNumNeuronas; j++) {
			net = pCapas[i].pNeuronas[j].w[0];

			// Bucle para cada neurona de la capa anterior
			for(k=0;k<pCapas[i-1].nNumNeuronas; k++) {
				net += (pCapas[i-1].pNeuronas[k].x * pCapas[i].pNeuronas[j].w[k+1]);
			}
			
			pCapas[i].pNeuronas[j].x = 1/(1 + exp(-1*net));
		}
	}

	// Última capa, sigmoide o softmax?
	double netOuts[pCapas[nNumCapas-1].nNumNeuronas];
	double netsSum = 0.0;

	// Net de cada neurona de salidpa
	for(i=0;i<pCapas[nNumCapas-1].nNumNeuronas;i++) {
		net = 0;

		for(j=0;j<pCapas[nNumCapas-2].nNumNeuronas;j++)
			net += pCapas[nNumCapas-2].pNeuronas[j].x * pCapas[nNumCapas-1].pNeuronas[i].w[j+1];
		
		net += pCapas[nNumCapas-1].pNeuronas[i].w[0];


		// Softmax
		if(softmaxOut) {
			netOuts[i] = exp(net);
			netsSum += exp(net);
		}
		//Sigmoide
		else {
			pCapas[nNumCapas-1].pNeuronas[i].x = 1/(1+exp(-net));
		}

	}

	// Aplicamos softmax
	if(softmaxOut) {
		for(i=0;i<pCapas[nNumCapas-1].nNumNeuronas;i++)
			pCapas[nNumCapas-1].pNeuronas[i].x = netOuts[i]/netsSum;
	}



}

// ------------------------------
// Calcular el error de salida (MSE) del out de la capa de salida con respecto a un vector objetivo y devolverlo
double PerceptronMulticapa::calcularErrorSalida(double* target, int funcionError) {
	double error=0;
	int i;

	switch(funcionError) {

	case MSE:
		for(i=0;i<pCapas[nNumCapas-1].nNumNeuronas;i++)
			error += pow((target[i]-pCapas[nNumCapas-1].pNeuronas[i].x), 2);
		
		error /= pCapas[nNumCapas-1].nNumNeuronas;
	break;

	case ENTRIOPIA_CRUZADA:
		for(i=0;i<pCapas[nNumCapas-1].nNumNeuronas;i++) {
			error += (-1 * target[i] * log(pCapas[nNumCapas-1].pNeuronas[i].x));
		}

		error /= pCapas[nNumCapas-1].nNumNeuronas;
	break;

	}	
	return error;
}


// ------------------------------
// Retropropagar el error de salida con respecto a un vector pasado como argumento, desde la última capa hasta la primera
// funcionError=1 => EntropiaCruzada // funcionError=0 => MSE
void PerceptronMulticapa::retropropagarError(double* objetivo, int funcionError) {
	int i, j, k;
	double derivada;
	double outi, outj;
	// Capa de salida
	for(i=0;i<pCapas[nNumCapas-1].nNumNeuronas;i++) {
		outi = pCapas[nNumCapas-1].pNeuronas[i].x;

		// Sigmoide
		if(!softmaxOut) {
			switch(funcionError) {
				case MSE:
					derivada = (objetivo[i] - outi) * (outi) * (1 - outi);
				break;

				case ENTRIOPIA_CRUZADA:
					derivada = (objetivo[i] / outi) * (outi) * (1 - outi);
				break;

			}
		}

		// Softmax
		else {
			switch(funcionError) {
				case MSE:
					derivada = 0;
					for(j=0;j<pCapas[nNumCapas-1].nNumNeuronas;j++) {
						outj = pCapas[nNumCapas-1].pNeuronas[j].x;
						derivada += ((objetivo[j] - outj) * (outi) * ((j == i) ? (1 - outj) : (-outj)));
					}
				break;

				case ENTRIOPIA_CRUZADA:
					derivada = 0;
					for(j=0;j<pCapas[nNumCapas-1].nNumNeuronas;j++) {
						outj = pCapas[nNumCapas-1].pNeuronas[j].x;
						derivada += ((objetivo[j] / outj) * (outi) * ((j == i) ? (1 - outj) : (-outj)));

					}
				break;
			}
		}

		// Asignamos la derivada
		pCapas[nNumCapas-1].pNeuronas[i].dX = -derivada;
	}

	
	// Resto de capas
	for(i=nNumCapas-2; i>=0; i--) {
		for(j=0; j<pCapas[i].nNumNeuronas; j++) {
			derivada = 0;
			for(k=0; k<pCapas[i+1].nNumNeuronas; k++) {
				derivada = derivada + (pCapas[i+1].pNeuronas[k].w[j+1] * pCapas[i+1].pNeuronas[k].dX);
			}
			derivada = derivada * (pCapas[i].pNeuronas[j].x) * (1 - pCapas[i].pNeuronas[j].x);
			pCapas[i].pNeuronas[j].dX = derivada;
		}
	}
}

// ------------------------------
// Acumular los cambios producidos por un patrón en deltaW
void PerceptronMulticapa::acumularCambio() {
	int i, j, k;
	for(i=1;i<nNumCapas;i++) {
		for(j=0; j<pCapas[i].nNumNeuronas; j++) {

			// Sesgo
			pCapas[i].pNeuronas[j].deltaW[0] += pCapas[i].pNeuronas[j].dX;

			// Resto
			for(k=0; k<pCapas[i-1].nNumNeuronas;k++)
				pCapas[i].pNeuronas[j].deltaW[k+1] += pCapas[i].pNeuronas[j].dX * pCapas[i-1].pNeuronas[k].x;
			
		}
	}
}

// ------------------------------
// Actualizar los pesos de la red, desde la primera capa hasta la última
void PerceptronMulticapa::ajustarPesos() {
	int i, j, k;
	double deltaW, deltaWAnterior;
	for(i=1;i<nNumCapas;i++) {
		double eta = pow(dDecremento, -(nNumCapas-i))*dEta;
		for(j=0; j<pCapas[i].nNumNeuronas; j++) {
			for(k=0; k<pCapas[i-1].nNumNeuronas+1;k++) {
				deltaW = pCapas[i].pNeuronas[j].deltaW[k];
				deltaWAnterior = pCapas[i].pNeuronas[j].ultimoDeltaW[k];
				if(bOnline) 
					pCapas[i].pNeuronas[j].w[k] += ((-eta * deltaW) - dMu * (eta * deltaWAnterior));
				else
					pCapas[i].pNeuronas[j].w[k] += ((-eta * deltaW)/nNumPatronesTrain - (dMu * eta * deltaWAnterior)/nNumPatronesTrain);

			}
		}
	}
}

// ------------------------------
// Imprimir la red, es decir, todas las matrices de pesos
void PerceptronMulticapa::imprimirRed() {
	int i, j, k;
	for(i=1;i<nNumCapas;i++) {
		for(j=0;j<pCapas[i].nNumNeuronas;j++) {
			printf("Neurona[%d][%d]: ", i, j);
			for(k=0;k<pCapas[i-1].nNumNeuronas+1;k++) {
				printf("%f ", pCapas[i].pNeuronas[j].w[k]);
			}
			printf("\n");
		}
	}
}

// ------------------------------
// Actualizar los deltasW, los actuales a 0 y los anteriores=actuales
void PerceptronMulticapa::actualizarDeltasW() {
	int i, j, k;

	for(i=1;i<nNumCapas;i++) {
		for(j=0;j<pCapas[i].nNumNeuronas;j++) {
			for(k=0;k<pCapas[i-1].nNumNeuronas+1;k++) {
				pCapas[i].pNeuronas[j].ultimoDeltaW[k] = pCapas[i].pNeuronas[j].deltaW[k];
				pCapas[i].pNeuronas[j].deltaW[k] = 0;
			}
		}
	}

}

// ------------------------------
// Simular la red: propragar las entradas hacia delante, computar el error, retropropagar el error y ajustar los pesos
// entrada es el vector de entradas del patrón, objetivo es el vector de salidas deseadas del patrón.
// El paso de ajustar pesos solo deberá hacerse si el algoritmo es on-line
// Si no lo es, el ajuste de pesos hay que hacerlo en la función "entrenar"
// funcionError=1 => EntropiaCruzada // funcionError=0 => MSE
void PerceptronMulticapa::simularRed(double* entrada, double* objetivo, int funcionError) {

	if(bOnline) actualizarDeltasW();

	alimentarEntradas(entrada);

	propagarEntradas();

	retropropagarError(objetivo, funcionError);

	acumularCambio();

	if(bOnline) ajustarPesos();
}


// ------------------------------
// Entrenar la red para un determinado fichero de datos (pasar una vez por todos los patrones)
void PerceptronMulticapa::entrenar(Datos* pDatosTrain, int funcionError) {
	int i;

	if(!bOnline) actualizarDeltasW();

	for(i=0;i<pDatosTrain->nNumPatrones;i++) {
		simularRed(pDatosTrain->entradas[i], pDatosTrain->salidas[i], funcionError);
	}
	if(!bOnline) ajustarPesos();
}

// ------------------------------
// Leer una matriz de datos a partir de un nombre de fichero y devolverla
Datos* PerceptronMulticapa::leerDatos(const char *archivo) {
	Datos * dataSet = new Datos;
	int i, j;
	// Abrimos el archivo
	ifstream file(archivo);
	if(not file.is_open()) {
		printf("No se pudo abrir el archivo '%s'\n", archivo);
		exit(EXIT_FAILURE);
	}

	// Primera linea, información sobre los datos
	file >> dataSet -> nNumEntradas;
	file >> dataSet -> nNumSalidas;
	file >> dataSet -> nNumPatrones;

	// Preparamos las matrices
	double ** matrizEntradas = (double **)malloc((dataSet -> nNumPatrones) * sizeof(double *));
	double ** matrizSalidas  = (double **)malloc((dataSet -> nNumPatrones) * sizeof(double *));

	for(i = 0; i < (dataSet -> nNumPatrones); i++) {
		// Reservamos
		matrizEntradas[i] = (double *)malloc((dataSet -> nNumEntradas) * sizeof(double));
		matrizSalidas[i]  = (double *)malloc((dataSet -> nNumSalidas)  * sizeof(double));

		// Rellenamos
		for(j = 0; j < (dataSet -> nNumEntradas); j++ )
			file >> matrizEntradas[i][j];
		for(j = 0; j < (dataSet -> nNumSalidas); j++ )
			file >> matrizSalidas[i][j];
	}

	// Asignamos al conjunto de datos
	dataSet -> entradas = matrizEntradas;
	dataSet -> salidas  = matrizSalidas;

	file.close();

	return dataSet;
}

// Copiar conjunto de datos : ERROR DE MEMORIA
Datos * PerceptronMulticapa::creaCopia(const Datos * source) {
	int i;
	Datos * copy = new Datos;

	// Reservamos matriz de entradas y salidas para validacion
	copy->entradas = (double **)malloc((source -> nNumPatrones) * sizeof(double *));
	copy->salidas  = (double **)malloc((source -> nNumPatrones) * sizeof(double *));

	for(i=0 ; i < source -> nNumPatrones ; i++) {
		copy->entradas[i] = (double *)malloc((source -> nNumEntradas) * sizeof(double));
		copy->salidas[i]  = (double *)malloc((source -> nNumSalidas) * sizeof(double));
	}

	copy->nNumPatrones = source -> nNumPatrones;
	copy->nNumEntradas = source -> nNumEntradas;
	copy->nNumSalidas  = source -> nNumSalidas;

	// Copiamos las entradas y las salidas
	for(i=0;i<source -> nNumPatrones; i++) {
		copiaVector(source->entradas[i], copy->entradas[i], source -> nNumPatrones);
		copiaVector(source->salidas[i], copy->salidas[i], source -> nNumPatrones);
	}
	copy->nNumPatrones = source -> nNumPatrones;
	return copy;
}

// Dividir conjunto de entrenamiento en entrenamiento y validación, si es que se usa
void PerceptronMulticapa::splitDataSets(const Datos * originalPDatosTrain, Datos * newPDatosTrain, Datos * pDatosValidation, int * indices) {
	int i, j=0, k=0;

	// Nuevo número de patrones
	newPDatosTrain -> nNumPatrones = originalPDatosTrain->nNumPatrones - pDatosValidation->nNumPatrones;


	for(i=0; i< originalPDatosTrain->nNumPatrones ; i++) {
		if(contiene(indices, i, pDatosValidation->nNumPatrones)) {
			pDatosValidation->entradas[j] = originalPDatosTrain->entradas[i]; 
			pDatosValidation->salidas[j]  = originalPDatosTrain->salidas[i]; 
			j++;
		}

		else {
			newPDatosTrain->entradas[k] = originalPDatosTrain->entradas[i]; 
			newPDatosTrain->salidas[k]  = originalPDatosTrain->salidas[i];
			k++;
		}

	}
}

// ------------------------------
// Probar la red con un conjunto de datos y devolver el error MSE cometido
double PerceptronMulticapa::test(Datos* pDatosTest, int funcionError) {
	int i;
	double mse = 0.0;
	for(i=0;i<pDatosTest->nNumPatrones;i++) {
		alimentarEntradas(pDatosTest->entradas[i]);
		propagarEntradas();
		mse += calcularErrorSalida(pDatosTest->salidas[i], funcionError);
	}
	mse /= pDatosTest->nNumPatrones;
	return mse;
}

// Probar la red con un conjunto de datos y devolver el CCR
double PerceptronMulticapa::testClassification(Datos* pDatosTest, int ** confusionMatrix) {
	double ccr = 0.0;
	int i, j;
	double maxYDeseado, maxYObtenido;
	int posDeseado, posObtenido;

	for(i=0;i<pDatosTest->nNumPatrones;i++) {
		maxYDeseado = 0.0;
		maxYObtenido = 0.0;
		posDeseado = 0;
		posObtenido = 0;

		alimentarEntradas(pDatosTest->entradas[i]);
		propagarEntradas();


		for(j=0;j<pDatosTest->nNumSalidas;j++) {
			if(pDatosTest->salidas[i][j] > maxYDeseado) {
				maxYDeseado = pDatosTest->salidas[i][j];
				posDeseado = j;
			}

			if(pCapas[nNumCapas-1].pNeuronas[j].x > maxYObtenido) {
				maxYObtenido = pCapas[nNumCapas-1].pNeuronas[j].x;
				posObtenido = j;
			}
		}
		if(posDeseado == posObtenido)
			ccr++;
		if(confusionMatrix != NULL) confusionMatrix[posObtenido][posDeseado]++;
		
	}

	ccr = (ccr/pDatosTest->nNumPatrones);
	return ccr;
}

// ------------------------------
// Ejecutar el algoritmo de entrenamiento durante un número de iteraciones, utilizando pDatosTrain
// Una vez terminado, probar como funciona la red en pDatosTest
// Tanto el error MSE de entrenamiento como el error MSE de test debe calcularse y almacenarse en errorTrain y errorTest
void PerceptronMulticapa::ejecutarAlgoritmoOnline(const Datos * originalPDatosTrain, Datos * pDatosTest, int maxiter, double *errorTrain, double *errorTest, char * fichTrain, double * meanIterations, int funcionError, double *ccrTrain, double *ccrTest)
{
	int countTrain = 0;
	bool keepIterating = true;

	// Inicialización de pesos
	pesosAleatorios();

	// Matriz de confusión
	int ** confusionMatrix = NULL;

	/*confusionMatrix = (int **)malloc(6 * sizeof(int *));
	for(int i = 0; i < 6 ; i++) {
		confusionMatrix[i] = (int *)malloc(6 * sizeof(int));
		for(int j = 0; j < 6; j++) {
			confusionMatrix[i][j] = 0;
		}
	}*/

	double minTrainError = 0;
	int numSinMejorarTrain, numSinMejorarVal;
	double testError = 0;

	double valError=0, minValError;
	bool usingValidation = false;
	
	Datos * pDatosValidacion;

	nNumPatronesTrain = originalPDatosTrain->nNumPatrones;

	// Nuevo conjunto de entrenamiento
	//Datos * pDatosTrain = creaCopia(originalPDatosTrain);
	Datos * pDatosTrain = leerDatos(fichTrain);

	// Generar datos de validación (patrones aleatorios en una estructura Datos)
	if(dValidacion > 0 && dValidacion < 1){
		usingValidation = true;

		// Configuramos el nuevo conjunto de validacion
		pDatosValidacion = new Datos;
		pDatosValidacion -> nNumPatrones = (pDatosTrain->nNumPatrones) * dValidacion;
		pDatosValidacion -> nNumEntradas = pDatosTrain -> nNumEntradas;
		pDatosValidacion -> nNumSalidas = pDatosTrain -> nNumSalidas;
		int * patronesElegidos = vectorAleatoriosEnterosSinRepeticion(0, pDatosTrain->nNumPatrones - 1, pDatosValidacion->nNumPatrones);

		// Reservamos matriz de entradas y salidas para validacion
		pDatosValidacion->entradas = (double **)malloc((pDatosValidacion -> nNumPatrones) * sizeof(double *));
		pDatosValidacion->salidas  = (double **)malloc((pDatosValidacion -> nNumPatrones) * sizeof(double *));

		for(int i=0 ; i < pDatosValidacion -> nNumPatrones ; i++) {
			pDatosValidacion->entradas[i] = (double *)malloc((pDatosValidacion -> nNumEntradas) * sizeof(double));
			pDatosValidacion->salidas[i]  = (double *)malloc((pDatosValidacion -> nNumSalidas) * sizeof(double));
		}
		
		splitDataSets(originalPDatosTrain, pDatosTrain, pDatosValidacion, patronesElegidos);

	}

	// Aprendizaje del algoritmo
	do {

		entrenar(pDatosTrain, funcionError);

		double trainError = test(pDatosTrain, funcionError);
		if(usingValidation) valError = test(pDatosValidacion, funcionError);

		// Condición: Error de entrenamiento no varia en 50 iteraciones
		if(countTrain==0 || fabs(trainError - minTrainError) > 0.00001) {
			minTrainError = trainError;
			copiarPesos();
			numSinMejorarTrain = 0;
		}
		else{
			numSinMejorarTrain++;
		}

		if(numSinMejorarTrain==50) {
			restaurarPesos();
			keepIterating = false;
		}

		// Condición: Error de validación no varia en 50 iteraciones
		if(usingValidation) {
			if(countTrain==0 || fabs(valError - minValError) > 0.00001) {
				minValError = valError;
				numSinMejorarVal = 0;
			}
			else {
				numSinMejorarVal++;
			}

			if(numSinMejorarVal == 50){
				restaurarPesos();
				keepIterating = false;
			}
		}

		testError = test(pDatosTest, funcionError);
		// Controlamos el número de iteraciones
		(countTrain < maxiter && keepIterating) ? (countTrain++) : (keepIterating = false);

		// Comprobar condiciones de parada de validación y forzar
		cout << "Iteración " << countTrain << "\t Error de entrenamiento: " << trainError << "\t Error de test: " << testError;
		if(usingValidation) cout << "\t Error de validación: " << valError;
		cout << "\tCCR de entrenamiento: " << testClassification(pDatosTrain) << "\tCCR de test: " << testClassification(pDatosTest);
		cout << endl;

	} while (keepIterating);

	*meanIterations += countTrain/5;

	cout << "PESOS DE LA RED" << endl;
	cout << "===============" << endl;
	imprimirRed();


	cout << "Salida Esperada Vs Salida Obtenida (test)" << endl;
	cout << "=========================================" << endl;
	for(int i=0; i<pDatosTest->nNumPatrones; i++){
		double* prediccion = new double[pDatosTest->nNumSalidas];

		// Cargamos las entradas y propagamos el valor
		alimentarEntradas(pDatosTest->entradas[i]);
		propagarEntradas();
		recogerSalidas(prediccion);
		for(int j=0; j<pDatosTest->nNumSalidas; j++)
			cout << pDatosTest->salidas[i][j] << " -- " << prediccion[j] << " ";
		cout << endl;
		delete[] prediccion;

	}

	*errorTest=testError;
	*errorTrain=minTrainError;
	*ccrTest = testClassification(pDatosTest, confusionMatrix);
	*ccrTrain = testClassification(pDatosTrain);

	if(usingValidation) delete pDatosValidacion;

	/*
	for(int i = 0; i < 6 ; i++) {
		for(int j = 0; j < 6; j++) {
			cout << confusionMatrix[i][j] << " ";
		}
		cout << endl;
	}
	*/

}
