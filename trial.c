#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(){
    const char* p = "pogggers!";
    const char* itr = p;
    while( *itr != '\0' ){
        printf("%c\n",*(itr++));
    }
}

