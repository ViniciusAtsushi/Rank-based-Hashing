/*

    Recebe os arquivos matrix_normalizada[0.1000].txt, rank_f_id para gerar um arquivo com todas as IMAGENS normalizadas
    Cria o arquivo all_images.bin
*/

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <iostream>
#include <vector>
#include <omp.h>

#define n 1360                   
#define features 4096            

using namespace std;

//  ---------------------------------- MAIN ----------------------------------------------------

int main(void)
{
    FILE *arq,
         *arq2,
         *arq_ft;

    FILE *report;

    report = fopen("./report/report.txt","a");

    int i,j,k;

    arq = fopen("./files/matriz_normalizada[0.1000].txt","r");
    if(arq == NULL)
    {
        printf("Arquivo não pode ser aberto");
        getchar();
        exit(1);
    }

    arq2 = fopen("./files/rank_f_id","r");
    if(arq2 == NULL)
    {
        printf("Arquivo não pode ser aberto");
        getchar();
        exit(1);
    }

    float **rank_f;
    int **rank_id;
    float **new_matriz;

    rank_f = (float**) malloc(features * sizeof(float*));
    rank_id = (int**) malloc(features * sizeof(int*));

    new_matriz = (float**) malloc(n * sizeof(float*));

    for(i=0;i<features;i++)
    {
        rank_f[i] = (float*) malloc(n*sizeof(float));
        rank_id[i] = (int*) malloc(n * sizeof(int));
    }

    for(i=0;i<n;i++)
    {
        new_matriz[i] = (float*) malloc(features * sizeof(float));
        for(j=0;j<features;j++)
        {
            new_matriz[i][j] = 0.0;
        }
    }

    for(i=0;i<features;i++)
    {
        for(j=0;j<n;j++)
        {
            fscanf(arq,"%f ",&rank_f[i][j]);
            fscanf(arq2,"%d ",&rank_id[i][j]);           
        }
    }



    int aux;

    double tempo_gasto = 0.0;
    clock_t begin  = clock();
	srand(time(0));

    arq_ft = fopen("./files/all_images.bin","wb");
    if(arq_ft)
    {
        printf("\nCreating new file... ");
        printf("\n->>Sucess");
        for(k=0;k<features;k++)
        { 
            for(i=0;i<n;i++)
            {
                aux = rank_id[k][i];
                new_matriz[aux][k] = rank_f[k][i];
            }
        }

        clock_t end  = clock();
        tempo_gasto += (double)(end-begin)/CLOCKS_PER_SEC;

        printf("\nTempo gasto: %lf segundos\n",tempo_gasto);

        fprintf(report,"3. New image search:   %f segundos\n",tempo_gasto);

        for(i=0;i<n;i++)
        {
            for(j=0;j<features;j++)
            {
                fprintf(arq_ft,"%.4f ",new_matriz[i][j]);
            }
            fprintf(arq_ft,"\n");
        }
    }
    

    for(i=0;i<features;i++)
    {
        free(rank_f[i]);
        free(rank_id[i]);
    }

    free(rank_f);
    free(rank_id);

    for(i=0;i<n;i++)
    {
        free(new_matriz[i]);
    }

    free(new_matriz);

    // --------------------------------------------------------------------------------------------------------------------------------------------
    fclose(arq);
    fclose(arq2);
    fclose(arq_ft);
    fclose(report);
    return 0;
}
