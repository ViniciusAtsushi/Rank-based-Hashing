/*
    Recebe o arquivo rank_f(arquivo de features ordenadas) e gera o arquivo matrix_normalizada[0.1000].txt
*/

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <time.h>

using namespace std;

#define n 1360                  
#define features 4096           

float* initMatrix() {
    cout << "\t - Intializing Feature Matrix ..." << endl;

    float* matrix = NULL;
    long int nN = ((long int) n)*features;
    matrix = new float[nN]();  //initialize all values to zero, but allocate memory immediately

    cout << "\t - Matrix Successfully Initialized ..." << endl;

    return matrix;
}

int main(void)
{
    FILE *arq = NULL,
         *new_arq = NULL,
         *report = NULL;

    report = fopen("./report/report.txt","a");
    
    int i,j,k;  
    float min,max;   
    float *matrix;
    
    arq = fopen("./files/rank_f","rb");
    if(arq == NULL)
    {
        cout << "Erro." << endl;
    }

    fseek(arq,0,SEEK_SET);

    double tempo_gasto = 0.0;
    clock_t begin  = clock();
	srand(time(0));

    matrix = initMatrix();

    for(i=0;i<features;i++)
    {
        for(j=0;j<n;j++)
        {
            fscanf(arq,"%f",&matrix[j+i*n]);
            if(j==0)
            {
                max = matrix[i*n+j];
                min = matrix[i*n+j];
            }

            if(matrix[i*n+j] > max)
            {
                max = matrix[i*n+j];
            }

            if(matrix[i*n+j] < min)
            {
                min = matrix[i*n+j];
            }
        }
        if(min != max)
        {
            for(k=0;k<n;k++)
            {
                matrix[i*n+k]= ((matrix[i*n+k] - min)/(max-min));
            }
        }
    }

    clock_t end  = clock();
    tempo_gasto += (double)(end-begin)/CLOCKS_PER_SEC;

    printf("\nTempo gasto: %lf segundos\n",tempo_gasto);

    fprintf(report,"2. Matriz normal[0.1]: %f segundos\n",tempo_gasto);

    new_arq = fopen("./files/matriz_normalizada[0.1000].txt","w");
    if(new_arq == NULL)
    {
        cout << "Erro." << endl;
    }

    for(i=0;i<features;i++)
    {
        for(j=0;j<n;j++)
        {
            fprintf(new_arq,"%.4f ",1000*matrix[j+i*n]);
        }
        fprintf(new_arq,"\n");
    }
    

    free(matrix);
    
    fclose(arq);
    fclose(new_arq);
    fclose(report);
}    

