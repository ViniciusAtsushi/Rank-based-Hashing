/*
    Recebe o arquvio:
        feat-matrix.txt 
        
    Gera os arquivos de features ordenadas:
        (rank_f) + arquivo de id(rank_f_id)
*/

#include <stdlib.h>
#include <sys/timeb.h>
#include <iostream>
#include <vector>
#include <omp.h> 

using namespace std;

// --------------------------------------- HEAP SORT ------------------------------------------------------------------------

void exchange(vector<float>& distances, vector<int>& curRk, int i, int j) {
    //Distances
    float t = distances[i];
    distances[i] = distances[j];
    distances[j] = t;
    //Ranked Lists
    int trk = curRk[i];
    curRk[i] = curRk[j];
    curRk[j] = trk;
}

void downheapDist(vector<float>& distances, vector<int>& curRk, int n, int v) {
    int w = 2 * v + 1; //first descendant of v
    while (w < n) {
        if (w + 1 < n) {
            if (distances[w + 1] > distances[w]) {
                w++;
            }
        } 
        if (distances[v] >= distances[w]) {
            return;
        }
        exchange(distances, curRk, v, w);
        v = w;
        w = 2 * v + 1;
    }
}

void downheapSim(vector<float>& distances, vector<int>& curRk, int n, int v) {
    int w = 2 * v + 1; //first descendant of v
    while (w < n) {
        if (w + 1 < n) {
            if (distances[w + 1] < distances[w]) {
                w++;
            }
        }
        if (distances[v] <= distances[w]) {
            return;
        }
        exchange(distances, curRk, v, w);
        v = w;
        w = 2 * v + 1;
    }
}

void buildheap(vector<float>& distances, vector<int>& curRk, int n, string type) {
    for (int v = n / 2 - 1; v >= 0; v--) {  // não sei se v = n ou v = features
        if (type == "SIM") {
            downheapSim(distances, curRk, n,  v);
        } else { //DIST
            downheapDist(distances, curRk, n, v);
        }
    }
}

void heapsort(vector<float>& distances, vector<int>& curRk, int n, string type) {
    buildheap(distances, curRk, n,type);
    while (n > 1) {
        n--;
        exchange(distances, curRk, 0, n);
        if (type == "SIM") {
            downheapSim(distances, curRk, n,  0);
        } else { //DIST
            downheapDist(distances, curRk, n, 0);
        }
    }
}


void mainHeapSort(string type, float*& matrix, vector<float>& feat_list, vector<int>& rkLists, int n, int features) {
    for (int rk = 0; rk < features; rk++)
    {
        vector<float> distances(n);
        vector<int> curRk(n);
        #pragma omp parallel for
        for (int j = 0; j < n; j++) {
            curRk[j] = j;
            distances[j] = matrix[features*j+rk];
        }
        heapsort(distances, curRk, n,type);
        
        for (int j = 0; j < n; j++) {
            rkLists[n*rk+j] = curRk[j];
            feat_list[n*rk+j] = distances[j];                
        }        
    }
}
// --------------------------------------------------------------------------------------------------------------------------

float* initMatrix(int n,int features) {
    cout << "\t - Intializing Feature Matrix ..." << endl;

    float* matrix = NULL;
    long int nN = ((long int) n)*features;
    matrix = new float[nN]();  //initialize all values to zero, but allocate memory immediately

    cout << "\t - Matrix Successfully Initialized ..." << endl;

    return matrix;
}
// --------------------------------------------------------------------------------------------------------------------------

int main() 
{
    int i,j;
    int n = 1360;               // Numero de imagens - 1360 (Flowers)   / 5000 (Corel5K)
    int features = 4096;        // Numero de features - 4096 (Flowers)  / 2048 (Corel5K)
    float* matrix = initMatrix(n,features);
    FILE *arq,
         *arq2,
         *arq3;
    
    FILE *report;

    report = fopen("./report/report.txt","w");

    // init ranked lists
    //vector<int> rkLists;

    vector<int> rkLists(n*features);
    vector<float> feat_list(n*features);
    
    for (int i = 0; i < features; i++) {
        for (int j = 0; j < n; j++) {
            rkLists[i*n+j] = j;
        }
    }

    //  ------------------------- init matrix with FEAT-matrix values --------------------------------------------


    arq = fopen("./files/feat-matrix.txt","r");         // Arquivo com as features extraidas
    if(arq == NULL)
    {
        printf("Arquivo não pode ser aberto");
        getchar();
        exit(1);
    }

    struct timeb start,end;
	float diff;

    ftime(&start);

    for (int i = 0; i < n*features; i++) {
        fscanf(arq,"%f,",&matrix[i]);
    }

    mainHeapSort("DIST", matrix, feat_list, rkLists, n, features);

    ftime(&end);

    diff = (float) (1000* (end.time - start.time) + (end.millitm - start.millitm));
	diff = diff/1000; 

	printf("\nTempo gasto (Heap-sort das features): %lf segundos\n",diff);

    fprintf(report,"REPORT:\n\n1. Ranked Features:    %f segundos\n",diff);

    arq2 = fopen("./files/rank_f_id","w");                            
    if(arq2== NULL)
    {
        printf("Arquivo não pode ser aberto");
        getchar();
        exit(1);
    }

    arq3 = fopen("./files/rank_f","w");                             
    if(arq3== NULL) 
    {
        printf("Arquivo não pode ser aberto");
        getchar();
        exit(1);
    }

    for(int m=0;m<features;m++)
    {
       for(int k=0;k<n;k++)
       {
           fprintf(arq2,"%d ",rkLists[k+m*n]);
           fprintf(arq3,"%.4f ",feat_list[k+m*n]);
       }
       fprintf(arq2,"\n");
       fprintf(arq3,"\n");
    }
    
    fclose(arq);
    fclose(arq2);
    fclose(arq3);
    fclose(report);
    return 0;
}

