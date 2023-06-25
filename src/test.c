#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define READ (1<<1)
#define WRITE (1<<2)
#define CLOSE (1<<3)
#define ALL (READ|WRITE|CLOSE)

void func(int* ptr){
    free(ptr);
}
int main(){
    int* a = malloc(sizeof(*a));
    func(a);
}