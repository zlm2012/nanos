#include "kernel.h"
#include "adt/list.h"
#include "string.h"

extern pid_t TIMER;
extern pid_t FILEMAN;
void A () { 
	Msg m1, m2;
	m1.src = current->pid;
	int x = 0;
	while(1) {
		if(x % 10000000 == 0) {
			printk("a"); 
			send(4, &m1);
			receive(4, &m2);
		}
		x ++;
	}
}
void B () { 
	Msg m1, m2;
	m1.src = current->pid;
	int x = 0;
	receive(4, &m2);
	while(1) {
		if(x % 10000000 == 0) {
			printk("b"); 
			send(4, &m1);
			receive(4, &m2);
		}
		x ++;
	}
}
void C () { 
	Msg m1, m2;
	m1.src = current->pid;
	int x = 0;
	receive(4, &m2);
	while(1) {
		if(x % 10000000 == 0) {
			printk("c"); 
			send(4, &m1);
			receive(4, &m2);
		}
		x ++;
	}
}
void D () { 
	Msg m1, m2;
	m1.src = current->pid;
	receive(4, &m2);
	int x = 0;
	while(1) {
		if(x % 10000000 == 0) {
			printk("d"); 
			send(4, &m1);
			receive(4, &m2);
		}
		x ++;
	}
}
 
void E () {
	Msg m1, m2;
	m2.src = current->pid;
	char c;
	while(1) {
		receive(MSG_TRGT_ANY, &m1);
		if(m1.src == 0) {c = '|'; m2.dest = 1; }
		else if(m1.src == 1) {c = '/'; m2.dest = 2;}
		else if(m1.src == 2) {c = '-'; m2.dest = 3;}
		else if(m1.src == 3) {c = '\\';m2.dest = 0;}
		else assert(0);
 
		printk("\033[s\033[1000;1000H%c\033[u", c);
		send(m2.dest, &m2);
	}
 
}

void testForTimer() {
  unsigned char buf[32]={0x32, 0x38, 0xae, 0x14, 0x56, 0x90, 0xca, 0x78};
  Msg m;
  int i;
  m.src=current->pid;
  m.dest=TIMER;
  m.type=NEW_TIMER;
  m.i[0]=5;
  send(TIMER, &m);
  printk("Timer Message sent\n");
  receive(TIMER, &m);
  printk("Time UP!!!!\n");
  printk("Now Test Random Generator...\n");
  dev_write("random", current->pid, buf, 0, 8);
  dev_read("random", current->pid, buf, 0, 32);
  printk("The Random numbers #1:\n");
  for(i=0; i<32; i++)
    printk("%0x ", buf[i]);
  printk("\n----------------------\n");
  dev_write("random", current->pid, buf, 0, 32);
  m.src=current->pid;
  m.dest=TIMER;
  m.type=NEW_TIMER;
  m.i[0]=5;
  send(TIMER, &m);
  receive(TIMER, &m);
  dev_read("random", current->pid, buf, 0, 32);
  printk("The Random numbers #2:\n");
  for(i=0; i<32; i++)
    printk("%0x ", buf[i]);
  printk("\n----------------------\n");
  sleep();
}

#define NBUF 5
#define NR_PROD 3
#define NR_CONS 4
 
int buf[NBUF], f = 0, r = 0, g = 1;
int last = 0;
Sem empty, full, mutex;
 
void
test_producer(void) {
	while (1) {
		P(&empty);
		P(&mutex);
		if(g % 10000 == 0) {
			printk(".");	// tell us threads are really working
		}
		buf[f ++] = g ++;
		f %= NBUF;
		V(&mutex);
		V(&full);
	}
}
 
void
test_consumer(void) {
	int get;
	while (1) {
		P(&full);
		P(&mutex);
		get = buf[r ++];
		assert(last == get - 1);	// the products should be strictly increasing
		last = get;
		r %= NBUF;
		V(&mutex);
		V(&empty);
	}
}
 
void
test_setup(void) {
	create_sem(&full, 0);
	create_sem(&empty, NBUF);
	create_sem(&mutex, 1);
	int i;
	for(i = 0; i < NR_PROD; i ++) {
		wakeup(create_kthread(test_producer));
	}
	for(i = 0; i < NR_CONS; i ++) {
		wakeup(create_kthread(test_consumer));
	}
}

void testIDE(void) {
	int i;
	unsigned char buf[512];
	dev_read("hda", current->pid, buf, 0, 512);
	lock();
	printk("MBR INFO:\n----------------------------\n");
	for (i=0; i<512; i++)
		printk("%02x ", (int)buf[i]);
	printk("\n----------------------------\n\n");
	unlock();
	sleep();
}

void testRamdisk(void) {
	unsigned char buf[512];
	//dev_read("ram", current->pid, buf, 128*1024, 512);
	Msg m;
	m.src=current->pid;
	m.dest=FILEMAN;
	m.type=DO_READ;
	m.dev_id=1;
	m.buf=buf;
	m.offset=0;
	m.len=512;
	send(FILEMAN, &m);
	receive(FILEMAN, &m);
	printk("Ramdisk Reading Test:\n");
	printk("%s", buf);
	printk("--------------------\n\n");
	sleep();
}

