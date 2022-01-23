#include <Windows.h>
#include <stdio.h>
#include "predict.h"
#include "data.h"

static int months[]={0,31,29,31,30,31,30,31,31,30,31,30,31};

void setX(Data * head, double * X ){
    static int month, day, index, begin;
    index = 0;
    sscanf(head->date,"*%d/%d",&month,&day);
    begin = months[month -1] + day; 
    X[index++] = 1;
    while(head = head->next){
        sscanf(head->date,"*%d/%d",&month,&day);
        X[index++] = months[month -1] + day - begin + 1;
    }
};

void setY(Data * head, double (*Y)[MAX_DAYS],int columns){
    static int index;
    index = 0;

    while(head){
        for(int i = 0; i < columns -1; i++){
            Y[i][index] = head->items[i+2];
        }
        head = head->next;
        index ++;
    }
};
char *  format_date(char * date,int interval){
    static char new_date[20];
    static int month,day;
    sscanf(date,"*%d/%d",&month,&day);
    if( (day += interval) > months[month]){
        day -= months[month];
        month++;
    }
    sprintf(new_date,"#%d/%d",month,day);
    return new_date;
}

// 秦九韶算法求多项式的值
int polyval(double * coef, int order,double x){
    static double y; 
    y = coef[order];
    for(int i = order - 1; i >= 0; i--){
        y = y*x + coef[i];
    }
    return y;
}
//----------------------------------------------------
//
// METHOD:  polyfit
//
// INPUTS:  dependentValues[0..(countOfElements-1)]
//          independentValues[0...(countOfElements-1)]
//          countOfElements
//          order - Order of the polynomial fitting
//
// OUTPUTS: coefficients[0..order] - indexed by term
//               (the (coef*x^3) is coefficients[3])
//
//----------------------------------------------------

BOOL polyfit(const double* const dependentValues,
            const double* const independentValues,
            unsigned int        countOfElements,
            unsigned int        order,
            double*             coefficients)
{
    double B[MAX_ORDER+1] = {0.0f};
    double P[((MAX_ORDER+1) * 2)+1] = {0.0f};
    double A[(MAX_ORDER + 1)*2*(MAX_ORDER + 1)] = {0.0f};
    double x, y, powx;
    unsigned int ii, jj, kk;

    // Verify initial conditions....
    // ----------------------------------

    // This method requires that the countOfElements > 
    // (order+1) 
    if (countOfElements <= order)
        return FALSE;

    // This method has imposed an arbitrary bound of
    // order <= MAX_ORDER.  Increase MAX_ORDER if necessary.
    if (order > MAX_ORDER)
        return FALSE;

    // Begin Code...
    // ----------------------------------

    // Identify the column vector
    for (ii = 0; ii < countOfElements; ii++)
    {
        x    = dependentValues[ii];
        y    = independentValues[ii];
        powx = 1;

        for (jj = 0; jj < (order + 1); jj++)
        {
            B[jj] = B[jj] + (y * powx);
            powx  = powx * x;
        }
    }

    // Initialize the PowX array
    P[0] = countOfElements;

    // Compute the sum of the Powers of X
    for (ii = 0; ii < countOfElements; ii++)
    {
        x    = dependentValues[ii];
        powx = dependentValues[ii];

        for (jj = 1; jj < ((2 * (order + 1)) + 1); jj++)
        {
            P[jj] = P[jj] + powx;
            powx  = powx * x;
        }
    }

    // Initialize the reduction matrix
    //
    for (ii = 0; ii < (order + 1); ii++)
    {
        for (jj = 0; jj < (order + 1); jj++)
        {
            A[(ii * (2 * (order + 1))) + jj] = P[ii+jj];
        }

        A[(ii*(2 * (order + 1))) + (ii + (order + 1))] = 1;
    }

    // Move the Identity matrix portion of the redux matrix
    // to the left side (find the inverse of the left side
    // of the redux matrix
    for (ii = 0; ii < (order + 1); ii++)
    {
        x = A[(ii * (2 * (order + 1))) + ii];
        if (x != 0)
        {
            for (kk = 0; kk < (2 * (order + 1)); kk++)
            {
                A[(ii * (2 * (order + 1))) + kk] = 
                    A[(ii * (2 * (order + 1))) + kk] / x;
            }

            for (jj = 0; jj < (order + 1); jj++)
            {
                if ((jj - ii) != 0)
                {
                    y = A[(jj * (2 * (order + 1))) + ii];
                    for (kk = 0; kk < (2 * (order + 1)); kk++)
                    {
                        A[(jj * (2 * (order + 1))) + kk] = 
                            A[(jj * (2 * (order + 1))) + kk] -
                            y * A[(ii * (2 * (order + 1))) + kk];
                    }
                }
            }
        }
        else
        {
            // Cannot work with singular matrices
            return FALSE;
        }
    }

    // Calculate and Identify the coefficients
    for (ii = 0; ii < (order + 1); ii++)
    {
        for (jj = 0; jj < (order + 1); jj++)
        {
            x = 0;
            for (kk = 0; kk < (order + 1); kk++)
            {
                x = x + (A[(ii * (2 * (order + 1))) + (kk + (order + 1))] * B[kk]);
            }

            coefficients[ii] = x;
        }
        
    }
    return TRUE;
}