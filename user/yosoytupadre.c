#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  printf("Mi padre es: %d\n", getppid());
  printf("Mi abuelo es: %d\n", getancestor(2));
  exit(0);
}
