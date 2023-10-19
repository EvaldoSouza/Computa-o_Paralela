#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>

#define MAX_LENGHT 256
#define BUF_SIZE 65536

#define TRUE 1
#define FALSE 0
#define MASTER 0

int numDivisores(int num){
    int i;
    int resultado = 0;
    for(i = 2; i<=num; i++){
        if(num % i == 0){
            resultado++; //quantos divisores exatos o numero tem
        }
    }
    return resultado;
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
    FILE *arquivo = fopen("numeros.txt", "r");
    FILE *saida = fopen("saida.txt", "w");

    char buffer[MAX_LENGHT];
    int valores, num_linhas, base, extra_linhas, world_size, rank;
    double prev_time_value, time_value, time_diff;


    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Status *Stat;
    num_linhas = count_lines(arquivo);
    base = num_linhas/world_size;
    extra_linhas = num_linhas % world_size;

    /* Initial time */
    if(rank == MASTER){
        //armazenar essas linhas em vários vetores
        int *vetor_estupido = malloc(num_linhas * sizeof(int)); //mudar o nome do vetor...ou arrumar uma solução  melhor
        for(int i = 0; i<num_linhas; i++){
            fgets(buffer, MAX_LENGHT, arquivo);
            valores = atoi(buffer);
            vetor_estupido[i] = valores;
        }

        //Começar a contar o tempo depois das operações de leitura
        prev_time_value = MPI_Wtime();

        //enviar um vetor para cada escravo
        int *vetor_do_mestre = malloc((base + extra_linhas) * sizeof(int));
        int *vetor_do_escravo = malloc(base * sizeof(int) );
        int posicao_ve = 0;
        //loop do mestre
        for (int ve = 0; ve < (base + extra_linhas); ve++ ){
            vetor_do_mestre[ve] = vetor_estupido[ve];
            posicao_ve++;
        }
        //loop do escravo
        for(int num_e = 1; num_e < world_size; num_e++){
            for (int ve = 0; ve < base; ve++ ){
                vetor_do_escravo[ve] = vetor_estupido[posicao_ve];
                posicao_ve++;
            }
            //mandando o vetor, usando comunicação bloqueante, pois não sei se vou ter memoria o suficiente para mandar varios vetores desse tamanho
            MPI_Send(vetor_do_escravo, base, MPI_INT, num_e, 0, MPI_COMM_WORLD);

        }
        //processar eu mesmo um vetor
        int *resultados = malloc(num_linhas * sizeof(int));
        int posicao_receba = 0;
        printf("Recebendo de escravo %d\n", 0);

        for(int i = 0; i<(base + extra_linhas); i++){
            //salvar meus resultados em um vetor
            resultados[posicao_receba] = numDivisores(vetor_do_mestre[i]);
            printf("Posicao: %d, Numero: %d, Divisores: %d\n", posicao_receba,vetor_estupido[posicao_receba], resultados[posicao_receba]);
            posicao_receba++;
        }
        //receber o resultado dos escravos
        int *resultado_escravo = malloc((base) * sizeof(int)); //vou fazer mais simples, para não precisar definir MPI_Types
        for(int num_e = 1; num_e < world_size; num_e++){
            MPI_Recv(resultado_escravo, base, MPI_INT, num_e, MASTER, MPI_COMM_WORLD, Stat);
            printf("Recebendo de escravo %d\n", num_e);
            for(int i = 0; i<base; i++){
                resultados[posicao_receba] = resultado_escravo[i];
                printf("Posicao: %d, Numero: %d, Divisores: %d\n", posicao_receba,vetor_estupido[posicao_receba], resultados[posicao_receba]);

                posicao_receba++;
            }

        }
        //finalizar a tomada de tempo depois de receber todos os resultados
        time_value = MPI_Wtime();

        //escrever esses resultados em um arquivo
        for(int tudo = 0; tudo < num_linhas; tudo++){
            //printf("Numero: %d, Divisores: %d\n", vetor_estupido[tudo], resultados[tudo]);            

            fprintf(saida, "Numero: %d, Divisores: %d\n", vetor_estupido[tudo], resultados[tudo]);            
        }
        time_diff = time_value - prev_time_value;
        
        fclose(arquivo);
        fclose(saida);
        printf("TERMINADO!\nTempo da tarefa: %f\n", time_diff);
    }else{
        //preciso receber um vetor de numeros do mestre
        int *num_recebidos = malloc(base * sizeof(int)); //vou fazer mais simples, para não precisar definir MPI_Types
        MPI_Recv(num_recebidos, base, MPI_INT, MASTER, 0, MPI_COMM_WORLD, Stat);

        //processar esses numeros
        int *num_processados = malloc(base * sizeof(int)); //vou fazer mais simples, para não precisar definir MPI_Types
        for(int i =0; i<base; i++){
        //salvar meus resultados em um vetor
            num_processados[i] = numDivisores(num_recebidos[i]);

        }

        //enviar meu vetor para o mestre
        MPI_Send(num_processados, base, MPI_INT, MASTER, MASTER, MPI_COMM_WORLD);
        //free(num_recebidos);
        //free(num_processados);
    }

    MPI_Finalize();
    return 0;
}