// init: The initial user-level program

#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fcntl.h"

int
main(void)
{
  if(open("console", O_RDWR) < 0){
    mknod("console", 1, 1);   // usar valores explícitos
    open("console", O_RDWR);
  }
  dup(0);  // stdout
  dup(0);  // stderr

  // --- primero corre yosoytupadre ---
  char *argv1[] = { "yosoytupadre", 0 };
  int pid = fork();
  if(pid == 0){
    exec("yosoytupadre", argv1);
    printf("init: exec yosoytupadre failed\n");
    exit(1);
  }
  wait((int*)0);

  // --- después corre el shell como siempre ---
  char *argv2[] = { "sh", 0 };
  for(;;){
    printf("init: starting sh\n");
    pid = fork();
    if(pid < 0){
      printf("init: fork failed\n");
      exit(1);
    }
    if(pid == 0){
      exec("sh", argv2);
      printf("init: exec sh failed\n");
      exit(1);
    }

    while(wait((int*)0) >= 0)
      ;
  }
}
