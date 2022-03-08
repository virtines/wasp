// #include <stdio.h>
// #include <pthread.h>

// #define NUM_THREADS 20

// void *thread_func(){
//     printf("this is a thread");
// }

// int main(int argc, char **argv) {
//     pthread_t c_thread[NUM_THREADS];

//     printf("Calling the threads.\n");
//     for(int i = 0; i < NUM_THREADS; i++){
//         pthread_create(&c_thread[i], NULL, thread_func, NULL);
//     }

//     printf("Joining the threads.\n");
//     for(int i = 0; i < NUM_THREADS; i++){
//         pthread_join(c_thread[i], NULL); 
//     }

//     printf("Done.\n");

//     return 0;
// }

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

void *print_message_function( void *ptr );

main()
{
     pthread_t thread1, thread2;
     char *message1 = "Thread 1";
     char *message2 = "Thread 2";
     int  iret1, iret2;

    /* Create independent threads each of which will execute function */

     iret1 = pthread_create( &thread1, NULL, print_message_function, (void*) message1);
     iret2 = pthread_create( &thread2, NULL, print_message_function, (void*) message2);

     /* Wait till threads are complete before main continues. Unless we  */
     /* wait we run the risk of executing an exit which will terminate   */
     /* the process and all threads before the threads have completed.   */

     pthread_join( thread1, NULL);
     pthread_join( thread2, NULL); 

     printf("Thread 1 returns: %d\n",iret1);
     printf("Thread 2 returns: %d\n",iret2);
     exit(0);
}

void *print_message_function( void *ptr )
{
     char *message;
     message = (char *) ptr;
     printf("%s \n", message);
}