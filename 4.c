#include<stdio.h>
#include<stdio.h>
#include<errno.h>
#include<stdlib.h>
#include<stdarg.h>
#include<signal.h>
#define NUM_FRAMES 16
#define FRAME_SIZE 64

extern void 4_test(void);
char memory[NUM_FRAMES * FRAME_SIZE];
char mem_table[NUM_FRAMES];

void init_my_mem(void){ 
    for(size_t i = 0; i < NUM_FRAMES*FRAME_SIZE; i++){
        memory[i] = 0;
    }
    for(size_t i = 0; i < NUM_FRAMES; i++){
        mem_table[i] = 0;
    }   
};

void my_malloc_stats(void){
    printf("\n Memory Accocation table:");
    int temp = 0;
    for(size_t i = 0; i < NUM_FRAMES; i++){
        printf("\n %ld)  ", i);
        for(size_t ii = 0; ii < FRAME_SIZE; ii++){
            temp = (int)memory[64 * i + ii];
            if(temp > 31 && temp < 127)
                printf("%c", (char)memory[64 * i + ii]);
            else
                printf(".");
        }
    }
    printf("\n");
    for(size_t i = 0; i < NUM_FRAMES; i++){
        int sz = (int)mem_table[i];
        if (sz != 0){
            for(int ii = 0; ii < sz; ii++){
                printf("R:---- ");
            }
            i = (size_t)(sz+i-1);
        }
        else{
            printf("f:---- ");
        }
    }
    printf("\n");
};

void* my_malloc(size_t size){
    if(size == 0) return NULL;
    if(size > 1024) return NULL;
    size_t next_indx = 0, indx = 0, space = 0, notFound = 1;
    if(size % FRAME_SIZE == 0){
        space = size / FRAME_SIZE;
    }
    else{
        space = (size + FRAME_SIZE) / FRAME_SIZE;
    }
    while (notFound){    
        indx = next_indx;
        if(mem_table[indx] != 0){
            next_indx = indx + 1;
        }
        else{
            notFound = 0;
            for (size_t i = 0; i < space; i++){
                if( indx + i >= NUM_FRAMES){
                    errno = ENOMEM;
                    return NULL;
                }
                else if(mem_table[indx+i] != 0){
                    notFound = 1;
                    next_indx = indx + 1 + mem_table[indx+i];
                }
            }
        }
    }
    if(indx > NUM_FRAMES){
        errno = ENOMEM;
        return NULL;
    }
    mem_table[indx] = space;
    return &(memory[0 + FRAME_SIZE * indx]);
};

void my_free(void *ptr){
    void *head = &(memory[0]);
    if((ptr < head) || (ptr-head > NUM_FRAMES*FRAME_SIZE)){
        raise(SIGSEGV);
    }
    else if( (ptr - head) % FRAME_SIZE != 0){
        raise(SIGSEGV);
    }
    else{
        size_t indx = (ptr - head) / FRAME_SIZE;
        size_t lemf = mem_table[indx];
        if(lemf == 0){
            raise(SIGSEGV);
        }
        for(size_t i = 0; i < lemf; i++){
            mem_table[indx + i] = 0;
            for(size_t ii = 0; ii < FRAME_SIZE; ii++){
                memory[FRAME_SIZE * (indx + i) + ii] = 0;
            }
        }
    }
};

int my_asprintf(char **strp, const char *fmt, ... ){
    size_t len;
    va_list args;
    va_start(args,fmt);
    len = vsnprintf(NULL,0,fmt,args);
    va_end(args);
    if(len > 0){
        len++;
        char *strRes = (char*)my_malloc(len);
        if(strRes){
            va_start(args,fmt);
            len = vsnprintf(strRes, len, fmt, args);
            if (len > 0){
                *strp = strRes;
            }
            else{
                len = -1;
            }
            va_end(args);
        }
        else{
            len  =-1;
        }
    }   
    else{
        len = -1;
    }
    return(len);
};

int main(void){
    init_my_mem();
    4_test();
}

/*
compile by: gcc ‐‐std=c99 ‐Wall ‐O2 ‐o hw4 hw4.c hw4_test.c ‐pthread 
*/
