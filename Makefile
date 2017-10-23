practica1: practica1.o imc/PerceptronMulticapa.o imc/stats.o imc/util.h
	g++ practica1.o imc/PerceptronMulticapa.o imc/stats.o -o practica1 -O3 -g

practica1.o: practica1.cpp
	g++ -c practica1.cpp

imc/PerceptronMulticapa.o: imc/PerceptronMulticapa.cpp imc/PerceptronMulticapa.h
	g++ -c imc/PerceptronMulticapa.cpp -o imc/PerceptronMulticapa.o

imc/stats.o: imc/stats.cpp imc/stats.hpp
	g++ -c imc/stats.cpp -o imc/stats.o

clean:
	rm -f *.o
	rm -f imc/*.o
