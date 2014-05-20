#include "stdio.h"
#include "stdlib.h"

int main(int argc, char** argv) {
	printf("Hello World!\n");
	printf("Just test for arguement: %d\n", 200);
        char tmp[2][20]={"Test!", "Can't you see?"};
        char *ar[2]={tmp[0], tmp[1]};
        int i;
        printf("argc: %d, argv: %p\n", argc, argv);
        if (argc==1) {
          printf("argc==1, now test exec()...\n");
          exec(3, 2, ar);
        } else {
          printf("init by exec(). Now print argv:\n");
          for (i=0; i<argc; i++)
            printf("%s\n", argv[i]);
        }

	return 0;
}
