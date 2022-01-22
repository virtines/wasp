#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>

int libc_sd = -1;

extern int main(int argc, char** argv, char **env);
extern void __libc_init_array(void);
extern void __libc_fini_array (void);
extern int _init_signal(void);
extern char** environ;



int libc_start(int argc, char** argv, char** env)
{
   int ret;

   /* call init function */
   __libc_init_array();

   /* register a function to be called at normal process termination */
   atexit(__libc_fini_array);

   /* optind is the index of the next element to be processed in argv */
   optind = 0;

   if (env)
      environ = env;

   /* initialize simple signal handling */
   //_init_signal();

   ret = main(argc, argv, env);

   /* call exit from the C library so atexit gets called, and the
      C++ destructors get run. This calls our exit routine below    
      when it's done. */
   exit(ret);

   /* we should never reach this point */
   return 0;
}

