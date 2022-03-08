#include <stdio.h>
#include <pthread.h>

#include "wasp_js_x.h"

#define NUM_THREADS 20

#define JS_CODE "function retd(args) { return {'numd:':args.num}; }; var b64 = retd({'num': 99 }); hcall_return(b64);" 

void *thread_func(void *ptr){
    char* jscode = JS_CODE;
    js_x(jscode);
}

int main(int argc, char **argv) {
    pthread_t c_thread[NUM_THREADS];

    printf("Calling the threads.\n");
    for(int i = 0; i < NUM_THREADS; i++){
        pthread_create(&c_thread[i], NULL, thread_func, NULL);
    }

    printf("Joining the threads.\n");
    for(int i = 0; i < NUM_THREADS; i++){
        pthread_join(c_thread[i], NULL); 
    }

    printf("Done.\n");

    return 0;
}