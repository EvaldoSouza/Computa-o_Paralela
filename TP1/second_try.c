#include <stdio.h>
#include <string.h>
#include <stdlib.h>
// #include "sc_monotic.c"
// #define __STDC_FORMAT_MACROS
// #include <inttypes.h>
#include <mpi.h>
#include <math.h>

#define MAX_LENGHT 256
#define BUF_SIZE 65536

#define TRUE 1
#define FALSE 0
#define MASTER 0

//fazer um .h depois
typedef struct resultadoDivisores
{
    int numero;
    //int qnt_divisores_raiz;
    int qnt_divisores_total;

}Resultado;


Resultado numDivisores(int num){
    double raiz = sqrt(num);
    int i;
    int resultado = 0;
    Resultado retorno;

    if(num == 4){ //4 é um caso especial dessa função, pois a qnt de divisores totais é igual a divisores da raiz
        retorno.numero = num;
        //retorno.qnt_divisores_raiz = 1;
        retorno.qnt_divisores_total = 1;
        return retorno;
    }

    for(i = 2; i<=raiz; i++){
        if(num % i == 0){
            resultado++; //quantos divisores exatos o numero tem
        }
    }

    retorno.numero = num;
    //retorno.qnt_divisores_raiz = resultado;
    retorno.qnt_divisores_total = resultado*2;



    return retorno;
}

int numDivisores(int num){
    if(num <=0){
        printf("Num corrompido!");
        return -1;
    }
    double raiz = sqrt(num);
    int i;
    int resultado = 0;
    for(i = 2; i<=raiz; i++){
        if(num % i == 0){
            resultado++; //quantos divisores exatos o numero tem
        }
    }

    return resultado;
}

//encontra o tamanho do arquivo em bytes
long int findSize(char file_name[]) 
{ 
	// opening the file in read mode 
	FILE* fp = fopen(file_name, "r"); 

	// checking if the file exist or not 
	if (fp == NULL) { 
		printf("File Not Found!\n"); 
		return -1; 
	} 

	fseek(fp, 0L, SEEK_END); 

	// calculating the size of the file 
	long int res = ftell(fp); 

	// closing the file 
	fclose(fp); 

	return res; 
} 

int count_lines(FILE* file)
{    
    char buf[BUF_SIZE];
    int counter = 0;
    for(;;)
    {
        size_t res = fread(buf, 1, BUF_SIZE, file);

        if (ferror(file))
            return -1;

        int i;
        for(i = 0; i < res; i++)
            if (buf[i] == '\n')
                counter++;

        if (feof(file))
            break;
    }

    fseek(file, 0L, SEEK_SET); //voltando o ponteiro para a inicio do arquivo
    return counter;
}

int main(int argc, char **argv){

    MPI_Init(&argc, &argv);

    char buffer[MAX_LENGHT];
    int valores, num_linhas, base, extra_linhas, world_size, rank;
    double start, end;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    
    if (argc != 3) {
        if (rank == 0) {
            printf("Usage: %s <input_file> <output_file>\n", argv[0]);
        }
        MPI_Finalize();
        return 1;
    }

    char* input_filename = argv[1];
    char* output_filename = argv[2];
    FILE* arquivo = fopen(input_filename, "r");
    FILE* saida = fopen(output_filename, "w");
    if (arquivo == NULL) {
            printf("Error opening the input file.\n");
            MPI_Finalize();
            return 1;
        }
        if (saida == NULL) {
            printf("Error opening the output file.\n");
            fclose(arquivo);
            MPI_Finalize();
            return 1;
    }
    
    MPI_Status *Stat;
    
    num_linhas = count_lines(arquivo);
    base = num_linhas/world_size;
    extra_linhas = num_linhas % world_size; // o máximo de linhas extras é world_size - 1

    int *parte = NULL; //fazer como struct depois
    parte = malloc((base + 1 )* sizeof(int)); // fazendo a alocação de 6
    int* vetor_estupido, *vetor_mais_estupido_ainda;
    int *valores_para_vetores, *deslocamento;

    valores_para_vetores = calloc(world_size, sizeof(int));
    deslocamento = calloc(world_size, sizeof(int));  

    if(rank == MASTER){
        printf("Processo Master\n");
        vetor_estupido = malloc(num_linhas * sizeof(int));
        vetor_mais_estupido_ainda = malloc(num_linhas * sizeof(int));
        for (int i = 0; i < num_linhas; i++) {
            fgets(buffer, MAX_LENGHT, arquivo);
            valores = atoi(buffer);
            vetor_estupido[i] = valores;
        }


        valores_para_vetores[0] = base;
        int linhas_sobrando = extra_linhas;
        for(int j= 1; j<world_size; j++){
            if(linhas_sobrando > 0){
                valores_para_vetores[j] = base + 1;
                linhas_sobrando--;
            }else{
                valores_para_vetores[j] = base;
            }
        }

        int sum = 0;
        for (int i = 0; i < world_size; i++) {
            deslocamento[i] = sum;
            sum += valores_para_vetores[i];
        }

        MPI_Scatterv(vetor_estupido, valores_para_vetores,deslocamento, MPI_INT, parte, valores_para_vetores[rank], MPI_INT, MASTER, MPI_COMM_WORLD ); 
        printf("Processo Master\n");
    }    
    start = MPI_Wtime();
    if(rank != MASTER){
        MPI_Scatterv(NULL, NULL, NULL,MPI_INT, parte, valores_para_vetores[rank], MPI_INT, MASTER, MPI_COMM_WORLD  );
    }    

    //MPI_Barrier(MPI_COMM_WORLD);
    printf("Rank: %d", rank);
    int divisores = 0;
    if(rank == 0){
        printf("meu loop: %d",valores_para_vetores[rank] );
    }

    for(int i = 0; i < valores_para_vetores[rank]; i++){
            divisores = numDivisores(parte[i]);
            parte[i] = divisores;
    }
    
    MPI_Gatherv(parte, valores_para_vetores[rank], MPI_INT, vetor_mais_estupido_ainda, valores_para_vetores, deslocamento, MPI_INT, MASTER, MPI_COMM_WORLD);

    printf("Cheguei na linha 207, Processo %d\n", rank);
    MPI_Barrier(MPI_COMM_WORLD);
    printf("Cheguei na linha 208, Processo %d\n", rank);

    end = MPI_Wtime();

    if(rank == MASTER){
    printf("Cheguei na linha 212, Processo %d\n", rank);

        for(int i = 0; i < num_linhas; i++){
            fprintf(saida, "Numero: %d, Quantidade de Divisores: %d\n", vetor_estupido[i], vetor_mais_estupido_ainda[i]);
        }
        double runtime = end - start;
        printf("Runtime: %fs\n", runtime);

        free(vetor_estupido);
        free(vetor_mais_estupido_ainda);
        free(valores_para_vetores);
    }


    fclose(saida);
    fclose(arquivo);
    free(parte);



    return EXIT_SUCCESS;
}