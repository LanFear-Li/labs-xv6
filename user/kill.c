#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/signo.h"
#include "user/user.h"

int
main(int argc, char **argv)
{
  int i, signo;

  if(argc < 2){
    fprintf(2, "usage: kill -signal pid...\n");
    exit(1);
  }
  signo = atoi(argv[1] + 1);
  for(i=2; i<argc; i++)
    kill(atoi(argv[i]), signo);
  exit(0);
}
