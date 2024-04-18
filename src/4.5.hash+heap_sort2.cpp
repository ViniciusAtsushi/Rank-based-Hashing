/*

    ->> Este código junta o calculo de pesos das tabelas hash com o heap sort de ordenação. (Optimazed hash + Ranked Distances : Passos 4 e 5)
	->> Faz o calculo e ordenação de pesos UMA IMAGEM POR VEZ EM MEMÓRIA

    Fazendo passo a passo:

        1. Cria todas as tabelas hash em memória.
        2. Calcula os vizinhos encontrados pela hash UMA IMAGEM POR VEZ.
        3. Após calcular os vizinhos dessa imagem, chama o heap-sort e Ordena.
        4. Chama a função que escreve os vizinhos dessa imagem em arquivo.
        5. Limpa o vetor acumulador de pesos.

        6. Volta para o passo 2 para a próxima imagem.

    Desta forma existe somente um vetor de pesos por vez em memória.

    Arquivo necessários para execução:

        -> all_images.bin
        -> matriz_normalizada[0.1000].txt
        -> ranf_f_id

	Gera arquivos:

        -> rank_d
        -> rank_d_id

*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <time.h>
#include <math.h>
//#include <omp.h>
#include <sys/timeb.h>
#include <string.h>

#define TAM_MAX 1021					// Hash number
#define n_images 1360					
#define features 4096 					
#define div_matriz 1360                 		// Batch Size
#define max_neighbors 1000				// Number of neighbors per image to write - output_file config

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
    for (int v = n / 2 - 1; v >= 0; v--) {
        if (type == "SIM") {
            downheapSim(distances, curRk, n, v);
        } else { //DIST
            downheapDist(distances, curRk, n, v);
        }
    }
}

void heapsort(vector<float>& distances, vector<int>& curRk, int n, string type) {
    buildheap(distances, curRk, n, type);
    while (n > 1) {
        n--;
        exchange(distances, curRk, 0, n);
        if (type == "SIM") {
            downheapSim(distances, curRk, n, 0);
        } else { //DIST
            downheapDist(distances, curRk, n, 0);
        }
    }
}


void mainHeapSort(string type, float*& vetor, vector<float>& dist_list, vector<int>& rkLists, int n) {

     for (int j = 0; j < n; j++) {     
            rkLists[j] = j;
            dist_list[j] = vetor[j];
        }
        heapsort(dist_list, rkLists, n, type);
    
}

// ----------------------------------------------------- used by heap-sort -----------------------------------------------------------------------

float* initVector(int n) {
    cout << "\t - Intializing Distance Matrix ..." << endl;

    float* vector = NULL;
    long int nN = ((long int) n);
    vector = new float[nN]();  //initialize all values to zero, but allocate memory immediately

    cout << "\t - Matrix Successfully Initialized ..." << endl;

    return vector;
}


// ----------------------------------------------------- used by hash -----------------------------------------------------------------------------

struct feature {
	float item;
	int id; 
	struct feature *prox; 
};
typedef struct feature node;

float* initMatrix() {

    float* matrix = NULL;
    long int nN = ((long int) n_images)*features;
    matrix = new float[nN]();  //initialize all values to zero, but allocate memory immediately
    return matrix;
}

int* initMatrix2() {
 
    int* matrix = NULL;
    long int nN = ((long int) n_images)*features;
    matrix = new int[nN]();  //initialize all values to zero, but allocate memory immediately
    return matrix;
}

// -------------------------------------------------- HASH FUNCTIONS ---------------------------------------------------------------------------

int hash_function(float); 						
void initialize(node **);		 				// Initialize hash tables
void add_value_function(float,int, node **); 				// Insert value into hash table
void free_function(node **); 						// Free hash table memory

void inserelista(float,int, node **); 			
int buscalista(float, node **); 			

void ranked_all_neighbors(float*,float*,float,node**,int,int);
void write_all_neighbors_file(float **, int **);


// ----------------------------------------------------------------------------------------------------------------------------------------------


int main(int argc, char *argv[]) {

	// Defining matrices
	float *matriz_ft = initMatrix();
	int *matriz_id = initMatrix2();
	float **current_image;

	// Auxiliary variables
	int i,j,k,x,n_neighbors;
	float item;
	
    // Heapsort params
    int n = n_images;               
    float *vetor_peso = initVector(n);

    // init ranked lists
    vector<int> rkLists;
    vector<float> dist_list;

    rkLists.resize(n);
    dist_list.resize(n);

    // -------------------------------------------------------  Reading files  ---------------------------------------------------------------
	
    FILE *arq_ft = NULL,
         *arq_id = NULL,
         *arq_all_images = NULL,
         *file_exe = NULL,
         *file_exe_id = NULL,
         *report = NULL;
         

    arq_id = fopen("./files/rank_f_id","rb");                
    arq_ft = fopen("./files/matriz_normalizada[0.1000].txt","rb");
    arq_all_images = fopen("./files/all_images.bin","rb");
    file_exe = fopen("./files/rank_d","w+b");        
    file_exe_id = fopen("./files/rank_d_id","w+b");  
    report = fopen("./report/report.txt","a");       
    fclose(file_exe);
    fclose(file_exe_id);

    // ----------------------------------------------------------------------------------------------------------------------------------------------
    
	node ***HASH; 				// Creating initial hash

	HASH = (node***)malloc(features* sizeof(node**));

	for(int r =0;r <features;r++)
	{
		HASH[r] = (node**)malloc(TAM_MAX *sizeof(node*));
		initialize(HASH[r]);
	}

	current_image = (float**) malloc(n_images*sizeof(float*));
	for(int r = 0;r< n_images;r++)
	{
		current_image[r] = (float*) malloc (features*sizeof(float));
	}		

	system("clear");

	//  --------------------------------------------------- Reading files -------------------------------------------------------------------------
	
        for(i=0;i<features;i++)
	{
		for(j=0;j<n_images;j++)
		{
			fscanf(arq_ft,"%f ",&matriz_ft[j+i*n_images]);
            fscanf(arq_id,"%d ",&matriz_id[j+i*n_images]);
		}	
	}
        
	for(i=0;i<n_images;i++)
	{
		for(j=0;j<features;j++)
		{
			fscanf(arq_all_images,"%f ",&current_image[i][j]);
		}
	}

	double tempo_gasto1 = 0.0;
	clock_t begin1  = clock();
	srand(time(0));

	//  -------------------------------  Creating Hash Tables  -------------------------------------------------------------------
	for(k=0; k<features; k++)
    {
		for(j=0;j<n_images;j++)                                            
		{
			add_value_function(matriz_ft[j+k*n_images],matriz_id[j+k*n_images],HASH[k]);
		}
	}

	clock_t end1  = clock();
	tempo_gasto1 += (double)(end1-begin1)/CLOCKS_PER_SEC;

	//  ---------------------------------   Exec. Parameters from argv -----------------------------------------------------------------------
	
	if(argc != 2)    	
	{
		exit(1);
	}

	n_neighbors = atoi(argv[1]);
	printf("Number of neighbors: %d\n",n_neighbors);

	//  --------------------------------------------------------------------------------------------------------------------------------------------------

	struct timeb start,end;
	float diff;

	ftime(&start);

	double write_t = 0.0;
	if(n_neighbors > 0)  // Valid "n" neigbhors variable
	{

        int r;

        float **matriz_atual ;                   
        int   **matriz_atual_id ;                
        matriz_atual = (float**) malloc(n_images*sizeof(float*));
        matriz_atual_id = (int**) malloc(n_images*sizeof(int*));
        for(i = 0;i<n_images;i++)
        {
            matriz_atual[i] = (float*) malloc(div_matriz*sizeof(float));
            matriz_atual_id[i] = (int*)malloc(div_matriz*sizeof(int));
        }

		float rank_all_neighbors[n_images];			

		float var_vector[n_neighbors];

		for(i = 1;i <= n_neighbors;i++)
		{
			var_vector[i-1] = (n_neighbors - i) * pow(0.99,i);        // Weight function         			  
		}

		for(r=0;r<n_images;r++)                     
                    rank_all_neighbors[r] = 0.0;            

	
	
        for(int z = 0;z <n_images;z+=div_matriz)                        // For each batch
        {
            for(i=z;i<z+div_matriz;i++)                                 
            {
                j = i-z;                                                
							
                for (k=0; k<features; k++)
                {
                    if(matriz_ft[k*n_images] == matriz_ft[k*n_images+(n_images-1)])			// Check if current feature vector is valid (last and first values are different) 
                    {
                        //printf("\n\nHash n: %d Invalid",i+1);
                    }else
                    {
                        ranked_all_neighbors(var_vector,rank_all_neighbors,current_image[i][k],HASH[k],n_neighbors,i);
                    }

                }

                for(int r=0;r<n_images;r++)
                {
                    vetor_peso[r] = rank_all_neighbors[r];                    
                }

                mainHeapSort("DIST", vetor_peso,dist_list, rkLists, n);        


                for(r=0;r<n_images;r++)                     
                {
                    rank_all_neighbors[r] = 0.0;            
                }

                for(r = 0;r< div_matriz;r++)                
                {
                    matriz_atual[j][r] = dist_list[r];
                    matriz_atual_id[j][r] = rkLists[r];
                }
				
            }
            clock_t begin1  = clock();
	    srand(time(0));
          	
            write_all_neighbors_file(matriz_atual,matriz_atual_id);                    
            
            clock_t end1  = clock();
	    write_t += (double)(end1-begin1)/CLOCKS_PER_SEC;
            
        }
	}


    fclose(arq_id);
    fclose(arq_ft);
    fclose(arq_all_images);
	


	for(int r =0;r <features;r++)
	{
		free(HASH[r]);
	}
	free(HASH);
	
	ftime(&end);

        diff = (float) (1000* (end.time - start.time) + (end.millitm - start.millitm));
	diff = diff/1000; 

	printf("\nTempo gasto (Calcular pesos, ordenar e escrever em arquivo): %lf segundos\n",diff-write_t);
	printf("\nTempo gasto (Criar tabelas hash): %lf segundos\n",tempo_gasto1);

	fprintf(report,"4. Criar Tabelas Hash:   %f segundos\n",tempo_gasto1);
	fprintf(report,"5. Queries e ordenar pesos:   %f segundos\n",diff-write_t);

	for(i=0;i<n_images;i++)
	{
		free(current_image[i]);
	}

	free(current_image);

	fclose(report);
}


//  ------------------------------------------------ HASH FUNCTIONS --------------------------------------------------------------------------------

int hash_function(float key) {
	return (labs(key)) % TAM_MAX;
}

void initialize(node **HASH) {
	int i;
	for(i=0; i<TAM_MAX; i++)
		HASH[i]=NULL;		 
}

void add_value_function(float item,int id, node **HASH) { 
	
	int pos = hash_function(item); 
	
	
	inserelista(item,id,HASH);
}

void inserelista(float item,int id, node **HASH) {
	int pos = hash_function(item);
	node **list = &HASH[pos];
	
	if (*list == NULL) {
		*list = (node *) malloc(sizeof(node));
  		if (*list == NULL) {
			printf("\nErro alocacao memoria!");
			exit(1);
  		}
  		(*list)->item=item;
		(*list)->id = id;
  		(*list)->prox=NULL;
  		
 	}
	else { // Se ocorreu colisao
	
	    node *saved= HASH[pos]; // guardando posicao inicial ponteiro
	
		while ((*list)->prox != NULL) 
		
			*list=(*list)->prox; 
				
		(*list)->prox=(node *) malloc(sizeof(node));
		if ((*list)->prox == NULL) {
			printf("\nErro alocacao memoria!");
			exit(1);
  		}
  		
		*list=(*list)->prox;
  		(*list)->item=item;
		(*list)->id = id;
  		(*list)->prox=NULL; 
  		
  		HASH[pos]=saved; 
	}
	
}


int buscalista(float key, node **HASH) {		// Verifica se existe o elemento repetido na tabel hash retorna 1 ou 0
	int pos = hash_function(key);
	node *list = HASH[pos];
	while (list != NULL) {
		if (key == list->item) return 1;      // Se encontrou retorna verdadeiro
		list = list->prox;
	}
	return 0;
}

void free_function(node **HASH) {
	int i;
	node *next;
	for(i=0; i<TAM_MAX; i++) {
		node **list=&HASH[i]; 
		while(*list!= NULL) { 
  			next = (*list)->prox;                
  			free(*list);
  			*list = next; 
  		} 
  	}
  	free(next);  	
}



// --------------------------------------------- Functions for all images -------------------------------------------------------------------------------



void ranked_all_neighbors(float *var_vector ,float *rank_all_neighbors,float key, node**HASH, int n_neighbors,int image)
{
	int i,
		pos;
	int cont1,
		cont2;
	float var;

	pos = hash_function(key);
	
	node *list = HASH[pos];
	node *list2, *list3;

	i = 1;	

	list2 = list3 = list;
	cont1 = cont2 = pos;

	do{
		// ATRIBUINDO PESO:

		// var = (n_neighbors/i)/n_neighbors;						
		// var = n_neighbors - i;
		// var = pow(0.75,n_neighbors - i); 
		// var = (n_neighbors - i) * log(n_neighbors - i + 1);			  
		// var = pow(0.95,i); 
		// var = (1 - ( log(n_neighbors - i + 1) / log(n_neighbors)));

		//var = (n_neighbors - i) * pow(0.99,i);                 			  
		// Substituindo a linha acima pela linha:
		var = var_vector[i-1];

		if(list2==NULL)
		{
			while(list2 == NULL)
			{
			cont1 -=1;
			list2 = HASH[cont1]; 	
			}
		}
		
		if(list3 == NULL)
		{
			while(list3 == NULL)
			{
			cont2 +=1;
			list3 = HASH[cont2]; 
			}
		}

		
		if(list2 == list3)
		{	
			if(key == list->item)
			{

				rank_all_neighbors[list2->id] += var;
			}else{

				rank_all_neighbors[list2->id] += var;
			}
		}else 
		{
			rank_all_neighbors[list2->id] += var;
			rank_all_neighbors[list3->id] += var;
		}

		

		if(list2->prox != NULL)
		{
			list2 = list2->prox;
		}else
		{
			cont1 -=1;
			if(cont1 >=0 )
				list2 = HASH[cont1]; 
		}

		if(list3->prox != NULL)
		{
			list3 = list3->prox;
		}else
		{
			cont2 +=1;
			if(cont2 <=999)
				list3 = HASH[cont2]; 
		}

		i++;

	}while(i<= n_neighbors);
}

void write_all_neighbors_file(float **matriz_atual, int **matriz_atual_id)
{
	FILE *rank_d = NULL,
         *rank_d_id = NULL;

	rank_d = fopen("./files/rank_d","a+b");
    rank_d_id = fopen("./files/rank_d_id","a+b");

	int i,j;

    for(i = 0;i < div_matriz;i++)
    {
        for(j=0;j<max_neighbors;j++)             
        {
            fprintf(rank_d,"%f ",matriz_atual[i][j]);
            fprintf(rank_d_id,"%d ",matriz_atual_id[i][j]);
        }
        fprintf(rank_d,"\n");
        fprintf(rank_d_id,"\n");
    }


    fprintf(rank_d,"\n");
    fprintf(rank_d_id,"\n");

	fclose(rank_d);
    fclose(rank_d_id);
}
// --------------------------------------------- END of HASH FUNCTIONS ------------------------------------------------------------------------

