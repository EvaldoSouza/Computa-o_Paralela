#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main () {
   int i, n;
   time_t t;
   
   n = 1000000000; //um bilhao
   //n = 100;
   
   /* Intializes random number generator */
   srand((unsigned) time(&t));

   /* Print 5 random numbers from 0 to 49 */
   FILE *arquivo = fopen("muitos_numeros.txt", "w");

   for( i = 2 ; i <= n ; i++ ) {
      //printf("%d\n", rand() % 100000);
      //fprintf(arquivo, "%d\n", rand() % 100000);
      fprintf(arquivo, "%d\n", i);
   }
   fclose(arquivo);
   printf("Finalizado a geração e armazenamento de %d numeros aleatorios!\n", n);
   return(0);
}