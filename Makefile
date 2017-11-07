practica2: practica2.o imc/PerceptronMulticapa.o imc/stats.o imc/util.h
	g++ practica2.o imc/PerceptronMulticapa.o imc/stats.o -o practica2 -O3 -g

practica2.o: practica2.cpp
	g++ -c practica2.cpp

imc/PerceptronMulticapa.o: imc/PerceptronMulticapa.cpp imc/PerceptronMulticapa.h
	g++ -c imc/PerceptronMulticapa.cpp -o imc/PerceptronMulticapa.o

imc/stats.o: imc/stats.cpp imc/stats.hpp
	g++ -c imc/stats.cpp -o imc/stats.o

clean:
	rm -f *.o
	rm -f imc/*.o
