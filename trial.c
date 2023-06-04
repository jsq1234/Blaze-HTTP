#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(){
   char** message;
   message = (char**)malloc(sizeof(char*)*10);
   *message = "manan";
   *(message+1) = "poggers";
   printf("%s",*(++message));

}