#include "kernel.h"
#include "tty.h"

static int tty_idx = 1;

static void
getty(void) {
	char name[] = "tty0", buf[256];
	int i;
	lock();
	name[3] += (tty_idx ++);
	unlock();

	while(1) {
		/* Insert code here to do these:
		 * 1. read key input from ttyd to buf (use dev_read())
		 * 2. convert all small letters in buf into capitcal letters
		 * 3. write the result on screen (use dev_write())
		 */
		memset(buf, 0, 256);
		dev_read(name, TTY, buf, 0, 256);
		for(i=0; i<strlen(buf); i++)
			if (buf[i]<='z' && buf[i]>='a')
				buf[i]+='A'-'a';
		buf[i]='\r';
		buf[i+1]='\n';
		dev_write(name, TTY, buf, 0, strlen(buf));
	}
}

void
init_getty(void) {
	int i;
	for(i = 0; i < NR_TTY; i ++) {
		wakeup(create_kthread(getty));
	}
}


