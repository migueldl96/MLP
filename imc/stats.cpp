#include "stats.hpp"

double mean(double * data, int num) {
        double mean =0.0;
        for(int i=0;i<num;i++)
                mean += data[i];
        return mean/num;
}

double sd(double * data, int num) {
        double media, sd = 0.0;
        media = mean(data, num);
        for(int i=0;i<num;i++)
                sd += pow((data[i]-media),2);
        return sd/num;
}

