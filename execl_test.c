#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>

int main(){
  errno = 0; // system last err
  int pid;
  int status;
  int code;
  pid_t result;
  
  /* echo */
  pid = fork();
  if(pid < 0){
	  perror("fork() failed.");
	  exit(EXIT_FAILURE);
  }
  
  if(pid == 0){
	  /* 子プロセス */
	  printf("execl() before \n");  
	  execl("/bin/echo", "/bin/echo", "hoge", "fuga", NULL);
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
	  exit(EXIT_FAILURE);
  }
  
  if(WIFEXITED(status)){
	  code = WEXITSTATUS(status);
	  printf("子プロセス終了コード : %d\n", code);
  }else{
	  printf("wait失敗終了コード : %d\n", status);
  }
  
  /* ls */
  pid = fork();
  if(pid == 0){
	  /* 子プロセス */
	  printf("execl2() before \n");  
	  execl("/bin/ls", "/bin/ls", "-l", NULL);
	  printf("execl2() after \n");
	  if(errno != 0){
		  perror(strerror(errno));
	  }else{
		  //正常時は復帰しない		  
	  }
  }

  /* 親プロセス */
  result = wait(&status);
  if(WIFEXITED(status)){
	  code = WEXITSTATUS(status);
	  printf("子プロセス終了コード : %d\n", code);
  }else{
	  printf("wait失敗終了コード : %d\n", status);
  }
  
  return EXIT_SUCCESS;

}
