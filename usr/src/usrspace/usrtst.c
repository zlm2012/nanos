#include "stdio.h"
#include "stdlib.h"

volatile int x;

int main(int argc, char** argv) {
	printf("Hello World!\n");
	printf("Just test for arguement: %d\n", 200);
        char tmp[2][20]={"Test!", "Can't you see?"};
        char *ar[2]={tmp[0], tmp[1]};
        int i;
        int pid;
        if (argc==1) {
          printf("argc==1, now test fork and exec()...\n");
          if ((pid=fork())==0) {
            x=getpid();
            printf("Test for data: %d, from child. Then exec()...\n", x);
            exec(0, 2, ar);
          } else {
            x=233;
            waitpid(pid);
            printf("Child finished. from parent\n");
            printf("P.S. test for data: %d, from parent.\n", x);
          }
        } else {
          printf("init by exec(). Now print argv:\n");
          for (i=0; i<argc; i++)
            printf("%s\n", argv[i]);
        }

	return 0;
}
