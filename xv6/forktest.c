// Test that fork fails gracefully.
// Tiny executable so that the limit can be filling the proc table.

#include "types.h"
#include "stat.h"
#include "user.h"
#if 0
void
printf(int fd, char *s, ...)
{
  write(fd, s, strlen(s));
}

void
forktest(void)
{
  int n, pid;

  printf(1, "fork test\n");

  for(n=0; n<1000; n++){
    pid = fork();
    if(pid < 0)
      break;
    if(pid == 0)
      exit();
  }
  
  if(n == 1000){
    printf(1, "fork claimed to work 1000 times!\n");
    exit();
  }
  
  for(; n > 0; n--){
    if(wait() < 0){
      printf(1, "wait stopped early\n");
      exit();
    }
  }
  
  if(wait() != -1){
    printf(1, "wait got too many\n");
    exit();
  }
  
  printf(1, "fork test OK\n");
}

int
main(void)
{
  forktest();
  exit();
}
#endif

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
