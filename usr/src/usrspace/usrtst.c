#include "stdio.h"
#include "stdlib.h"

int main() {
	printf("Hello World!\n");
	printf("Just test for arguement: %d\n", 200);
	int a;
	a=fork();
	if (a) printf("Forked. from parent.\n");
        else printf("Forked. from child.\n");

	return 0;
}
