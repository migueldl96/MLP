/*
 * util.h
 *
 *  Created on: 06/03/2015
 *      Author: pedroa
 */

#ifndef UTIL_H_
#define UTIL_H_
#include <cstdlib>  // Para establecer la semilla srand() y generar números aleatorios rand()
#include <cmath>

namespace util{
static int * vectorAleatoriosEnterosSinRepeticion(int minimo, int maximo, int cuantos){
        int total = maximo-minimo+1;
        int* numerosPorElegir = new int[total];
        int* numerosElegidos = new int[cuantos];
        // Inicializar la lista de elecciones posibles
        for(int i = 0; i < total; i++)
                numerosPorElegir[i] = minimo+i;

        for(int i=0; i < cuantos; i++)
        {
                int numeroElegido = rand() % (total-i);
                // Recordar el numero elegido
                numerosElegidos[i]=numerosPorElegir[numeroElegido];
                // Ponemos en numeros por elegir, el ultimo que era valido, asi
                // son todos validos hasta data.n-i-1
                numerosPorElegir[numeroElegido]=numerosPorElegir[total-i-1];
        }
        delete [] numerosPorElegir;
        return numerosElegidos;

}
static void copiaVector(double * source, double * destination, int tam) {
        int i;
        for(i=0;i<tam;i++) destination[i] = source[i];
}
static bool contiene(int * vector, int ele, int tam) {
        int i;
        for(i=0;i<tam;i++)
                if(vector[i] == ele)
                        return true;

        return false;
}

static int indiceMaxElemento(double * vector, int tam) {
        int i, indice=0;
        double max = vector[0];

        for(i=1;i<tam;i++) {
                if(vector[i]>max) {
                        max = vector[i];
                        indice = i;
                }
        }

        return indice;
}
}


#endif /* UTIL_H_ */
