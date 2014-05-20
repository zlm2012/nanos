#include "stdio.h"
#include "stdlib.h"

int main(int argc, char** argv) {
	printf("Hello World!\n");
	printf("Just test for arguement: %d\n", 200);
        char tmp[2][20]={"Test!", "Can't you see?"};
        char *ar[2]={tmp[0], tmp[1]};
        int i;
        int pid;
        if (argc==1) {
          printf("argc==1, now test fork and exec()...\n");
          if ((pid=fork())==0)
            exec(3, 2, ar);
          else {
            waitpid(pid);
            printf("Child finished. from parent\n");
          }
        } else {
          printf("init by exec(). Now print argv:\n");
          for (i=0; i<argc; i++)
            printf("%s\n", argv[i]);
        }

	return 0;
}
