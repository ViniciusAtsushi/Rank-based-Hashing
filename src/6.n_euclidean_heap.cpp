/*
   
   Para rodar esse programa são necessários os arquivos:

   -> rank_d_id
   -> feat-matrix.txt 

    O arquivo gerado como saída é:

    -> hash+euclidiana.bin

*/

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define n_images 1360                    
#define n_neighbors 80                 
#define max_neighbors 1000
#define image_class 80                  
#define features 4096                    

void maxHeapify(float arr[], int n, int i, int id_arr[]) {
    int largest = i; 
    int left = 2 * i + 1;
    int right = 2 * i + 2;

    if (left < n && arr[left] > arr[largest]) {
        largest = left;
    }

    if (right < n && arr[right] > arr[largest]) {
        largest = right;
    }

    if (largest != i) {
        // Trocar a distância
        float temp_dist = arr[i];
        arr[i] = arr[largest];
        arr[largest] = temp_dist;

        // Trocar o ID
        int temp_id = id_arr[i];
        id_arr[i] = id_arr[largest];
        id_arr[largest] = temp_id;

        maxHeapify(arr, n, largest, id_arr);
    }
}

void buildMaxHeap(float arr[], int n, int id_arr[]) {
    for (int i = n / 2 - 1; i >= 0; i--) {
        maxHeapify(arr, n, i, id_arr);
    }
}

void heapSort(float arr[], int n, int id_arr[]) {
    buildMaxHeap(arr, n, id_arr);
    for (int i = n - 1; i > 0; i--) {
        // Troca a distânciar
        float temp_dist = arr[0];
        arr[0] = arr[i];
        arr[i] = temp_dist;

        // Trocar o ID
        int temp_id = id_arr[0];
        id_arr[0] = id_arr[i];
        id_arr[i] = temp_id;

        maxHeapify(arr, i, 0, id_arr);
    }
}



int main(void)
{
    // --------------------------------------------------------------------------------------------------------------------------

    int i,j,k,m;
    float soma,sub;

    float **matrix,                      
          **matrix_dist;                 

    int **matrix_dist_id;                

    matrix = (float**) malloc (n_images*sizeof(float*));
    matrix_dist = (float**) malloc(n_images * sizeof(float*));
    matrix_dist_id = (int**) malloc (n_images * sizeof(int*));

    for(i = 0; i < n_images; i++)
    {
        matrix[i] = (float*) malloc (features * sizeof(float));
        matrix_dist[i] = (float*) malloc (max_neighbors * sizeof(float));
        matrix_dist_id[i] = (int*) malloc (max_neighbors * sizeof(int));
    }

    // --------------------------------------------------------------------------------------------------------------------------

    FILE *file;

    file = fopen("./files/feat-matrix.txt","rb");

    FILE *file_id;

    file_id = fopen("./files/rank_d_id","rb");

    FILE *report;

    report = fopen("./report/report.txt","a");

    for(i=0;i<n_images;i++)
    {
        for(j=0;j<features;j++)
        {
            fscanf(file,"%f,",&matrix[i][j]);
        }
    }

    for(i=0;i<n_images;i++)
    {
        for(j=0;j<n_neighbors;j++)
        {
            fscanf(file_id,"%d,",&matrix_dist_id[i][j]);
        }

        for(j=0;j<max_neighbors - n_neighbors;j++)               
        {
            fscanf(file_id,"%d",&k);
        }
    }

    // --------------------------------------------------------------------------------------------------------------------------

    // Calculo de tempo da distancia euclidiana

    double tempo_gasto = 0.0;
	clock_t begin  = clock();
	srand(time(0));

    for(i=0;i<n_images;i++)
    {
        for(j=0;j<n_neighbors;j++)
        {
            m = matrix_dist_id[i][j];
            for(k=0;k<features;k++)
            {
                sub = matrix[i][k] - matrix[m][k];
                sub = pow(sub,2);
                soma += sub; 
            }
            soma = sqrt(soma);
            matrix_dist[i][j] = soma;
            soma = 0;
        }
    }

    // --------------------------------------------------------------------------------------------------------------------------

    FILE *file_exe;
    FILE *file_exe2;

    file_exe = fopen("./files/hash+euclidiana.txt","w+");
    file_exe2 = fopen("./files/hash+euclidiana_dist.txt","w+");

    int aux_id;
    float aux_dist;


    for(i=0;i<n_images;i++)
    {
	heapSort(matrix_dist[i], n_neighbors, matrix_dist_id[i]);
    }

    clock_t end  = clock();
	tempo_gasto += (double)(end-begin)/CLOCKS_PER_SEC;

	printf("\nTempo gasto (Calcular distancias): %lf segundos\n",tempo_gasto);

    fprintf(report,"6. Euclidean Distance: %f segundos\n",tempo_gasto);


    for(i=0;i<n_images;i++)
    {
        for(j=0;j<image_class;j++)
        {
            fprintf(file_exe,"%d ",matrix_dist_id[i][j]);
            fprintf(file_exe2,"%f ",matrix_dist[i][j]);          // Descomentar para gerar o arquivo de distancias 
        }
        fprintf(file_exe,"\n");
        fprintf(file_exe2,"\n");                                 // Descomentar para gerar o arquivo de distancias 
    }
    // --------------------------------------------------------------------------------------------------------------------------


    for(int i = 0; i < n_images; i++)
    {
        free(matrix[i]);
        free(matrix_dist[i]);
        free(matrix_dist_id[i]);
    }
    free(matrix);
    free(matrix_dist);
    free(matrix_dist_id);


    fclose(file);
    fclose(file_id);
    fclose(file_exe);
    fclose(file_exe2);
    fclose(report);
    // --------------------------------------------------------------------------------------------------------------------------

}
