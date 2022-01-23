#ifndef PREDICT_H_
#define PREDICT_H_
#define MAX_ORDER  5
#include "data.h"

int polyfit(const double* const dependentValues,
            const double* const independentValues,
            unsigned int        countOfElements,
            unsigned int        order,
            double*             coefficients);

void setX(Data * head, double * X );

void setY(Data * head, double (*Y)[MAX_DAYS],int columns);

int polyval(double * coef,int order,double x);

char *  format_date(char * date,int interval);
#endif
