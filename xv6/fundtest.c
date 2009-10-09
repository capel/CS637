#include "types.h"
#include "stat.h"
#include "user.h"


int makekid()
{
  int pid = fork();
  int result;
  if (pid == 0)
  {
  	while(1)
	{
	  result += 12;
      result /= 55;
	}
  }
  else
	  return pid;
}


int
main(int argc, char **argv)
{
  int pid;
  pid = makekid();
  fund(pid, 200);
  pid = makekid();
  fund(pid, 400);
  exit();
}
