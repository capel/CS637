#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char **argv)
{
  int i;
	
  if(argc < 2){
    printf(2, "usage: fund pid numtickets\n");
    exit();
  }
  fund(atoi(argv[i]), atoi(argv[2]));
  exit();
}
