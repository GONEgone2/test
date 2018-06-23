#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>

static int exec_command1(void)
{
  errno = 0; // system last err
  int pid;
  int status;
  int code;
  pid_t result;
  
  /* echo */
  pid = fork();
  if(pid < 0){
	  perror("fork() failed.");
	  return (EXIT_FAILURE);
  }
  
  if(pid == 0){
	  /* 子プロセス */
	  printf("execl() before \n");  
	  execl("/bin/echo", "/bin/echo", "hage", NULL);
	  printf("execl() after \n");
	  if(errno != 0){
		  perror(strerror(errno));
	  }else{
		  //正常時は復帰しない		  
	  }
  }

  /* 親プロセス */
  result = wait(&status); //子プロセス終了待ち
  if( result < 0){
	  perror("wait() failed.");
	  return (EXIT_FAILURE);
  }
  
  if(WIFEXITED(status)){
	  code = WEXITSTATUS(status);
	  printf("子プロセス終了コード : %d\n", code);
  }else{
	  printf("wait失敗終了コード : %d\n", status);
  }
  
  return EXIT_SUCCESS;	
}

static int exec_command2(void)
{
  errno = 0; // system last err
  int pid;
  int status;
  int code;
  pid_t result;
  
  /* echo */
  pid = fork();
  if(pid < 0){
	  perror("fork() failed.");
	  return (EXIT_FAILURE);
  }
  
  if(pid == 0){
	  /* 子プロセス */
	  printf("execl() before \n");  
	  execl("/bin/ls", "/bin/ls", "-l", NULL);	  
	  printf("execl() after \n");
	  if(errno != 0){
		  perror(strerror(errno));
	  }else{
		  //正常時は復帰しない		  
	  }
  }

  /* 親プロセス */
  result = wait(&status); //子プロセス終了待ち
  if( result < 0){
	  perror("wait() failed.");
	  return (EXIT_FAILURE);
  }
  
  if(WIFEXITED(status)){
	  code = WEXITSTATUS(status);
	  printf("子プロセス終了コード : %d\n", code);
  }else{
	  printf("wait失敗終了コード : %d\n", status);
  }
  
  return EXIT_SUCCESS;	
}

int main(){
	exec_command1();
	exec_command2();
	
  return EXIT_SUCCESS;

}
