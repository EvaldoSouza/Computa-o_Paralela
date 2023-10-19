//Somar um numero a cada elemento de um vetor de numeros
//Fazer primeiro linearmente
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "sc_monotic.c"
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <math.h>

#define MAX_LENGHT 256
#define BUF_SIZE 65536

//fazer um .h depois
typedef struct resultadoDivisores
{
    int numero;
    int qnt_divisores_raiz;
    int qnt_divisores_total;

}Resultado;

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

Resultado numDivisores(int num){
    double raiz = sqrt(num);
    int i;
    int resultado = 0;
    Resultado retorno;

    if(num == 4){ //4 é um caso especial dessa função, pois a qnt de divisores totais é igual a divisores da raiz
        retorno.numero = num;
        retorno.qnt_divisores_raiz = 1;
        retorno.qnt_divisores_total = 1;
        return retorno;
    }

    for(i = 2; i<=raiz; i++){
        if(num % i == 0){
            resultado++; //quantos divisores exatos o numero tem
        }
    }

    retorno.numero = num;
    retorno.qnt_divisores_raiz = resultado;
    retorno.qnt_divisores_total = resultado*2;



    return retorno;
}

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

int main(int argc, char **argv){

    
    FILE *arquivo = fopen("numeros.txt", "r");
    FILE *saida = fopen("saida.txt", "w");

    char buffer[MAX_LENGHT];
    int valores, world_size;
    uint64_t prev_time_value, time_value;
    uint64_t time_diff;
    int num_linhas = count_lines(arquivo);
    printf("O arquivo tem %d linhas \n", num_linhas);
    Resultado resultado;

    /* Initial time */
    printf("COMECANDO!\n");
    prev_time_value = get_posix_clock_time ();
    // while(fgets(buffer, MAX_LENGHT, arquivo)){ //primeira operação, leitura dos dados
    //     valores = atoi(buffer);
    //     if(num_is_prime(valores)){//segunda operação, determinar se é primo
    //         printf("Primo encontrado! %d\n", valores);
    //         fprintf(saida, "%d\n", valores);//terceira operação, escrita dos resultados
    //     }
    // }

    while(fgets(buffer, MAX_LENGHT, arquivo)){ //primeira operação, leitura dos dados
        valores = atoi(buffer);
        resultado = numDivisores(valores);
        fprintf(saida, "Numero: %d, Divisores: %d, %d\n", resultado.numero, resultado.qnt_divisores_raiz, resultado.qnt_divisores_total); //div*2 só não funciona para o 4 :(
    }
    
    time_value = get_posix_clock_time ();
    /* Time difference */
    time_diff = time_value - prev_time_value;
    
    fclose(arquivo);
    fclose(saida);
    printf("TERMINADO!\nTempo da tarefa: ");
    int segundos = time_diff / 1000000;
    int milisegundos = time_diff%1000;
    printf("Tempo gasto: %d:%d\n", segundos, milisegundos);
    printf("Tempo em microsegundos:""%"PRIu64"\n", time_diff);
    //printf("%"PRIu64":%"PRIu64"\n", time_diff, time_diff);
    return 1;
}