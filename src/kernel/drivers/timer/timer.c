#include "kernel.h"
#include "x86/x86.h"
#include "hal.h"
#include "time.h"
#include "string.h"
#include "adt/list.h"

#define PORT_TIME 0x40
#define PORT_RTC  0x70
#define FREQ_8253 1193182
#define CURRENT_YEAR 2014
#define INTR_NOTIFY 200

pid_t TIMER;
static long jiffy = 0;
static int century_register = 0x00;
static Time rt;

typedef struct timer {
  int interval;
  pid_t src;
  bool va;
  ListHead h;
} Timer;

static Timer tmrs[100];
static ListHead th;
static void update_jiffy(void);
static void init_i8253(void);
static void init_rt(void);
static void timer_driver_thread(void);

enum {
  cmos_address = 0x70,
  cmos_data    = 0x71
};

static int get_update_in_progress_flag() {
  out_byte(cmos_address, 0x0A);
  return (in_byte(cmos_data) & 0x80);
}
 
 
static unsigned char get_RTC_register(int reg) {
  out_byte(cmos_address, reg);
  return in_byte(cmos_data);
}

void read_rtc() {
  unsigned char century;
  unsigned char last_second;
  unsigned char last_minute;
  unsigned char last_hour;
  unsigned char last_day;
  unsigned char last_month;
  unsigned char last_year;
  unsigned char last_century;
  unsigned char registerB;
 
  // Note: This uses the "read registers until you get the same values twice in a row" technique
  //       to avoid getting dodgy/inconsistent values due to RTC updates
 
  while (get_update_in_progress_flag());                // Make sure an update isn't in progress
  rt.second = get_RTC_register(0x00);
  rt.minute = get_RTC_register(0x02);
  rt.hour = get_RTC_register(0x04);
  rt.day = get_RTC_register(0x07);
  rt.month = get_RTC_register(0x08);
  rt.year = get_RTC_register(0x09);
  if(century_register != 0) {
    century = get_RTC_register(century_register);
  }
 
  do {
    last_second = rt.second;
    last_minute = rt.minute;
    last_hour = rt.hour;
    last_day = rt.day;
    last_month = rt.month;
    last_year = rt.year;
    last_century = century;
 
    while (get_update_in_progress_flag());           // Make sure an update isn't in progress
    rt.second = get_RTC_register(0x00);
    rt.minute = get_RTC_register(0x02);
    rt.hour = get_RTC_register(0x04);
    rt.day = get_RTC_register(0x07);
    rt.month = get_RTC_register(0x08);
    rt.year = get_RTC_register(0x09);
    if(century_register != 0) {
      century = get_RTC_register(century_register);
    }
  } while( (last_second != rt.second) || (last_minute != rt.minute) || (last_hour != rt.hour) ||
	   (last_day != rt.day) || (last_month != rt.month) || (last_year != rt.year) ||
	   (last_century != century) );
 
  registerB = get_RTC_register(0x0B);
 
  // Convert BCD to binary values if necessary
 
  if (!(registerB & 0x04)) {
    rt.second = (rt.second & 0x0F) + ((rt.second / 16) * 10);
    rt.minute = (rt.minute & 0x0F) + ((rt.minute / 16) * 10);
    rt.hour = ( (rt.hour & 0x0F) + (((rt.hour & 0x70) / 16) * 10) ) | (rt.hour & 0x80);
    rt.day = (rt.day & 0x0F) + ((rt.day / 16) * 10);
    rt.month = (rt.month & 0x0F) + ((rt.month / 16) * 10);
    rt.year = (rt.year & 0x0F) + ((rt.year / 16) * 10);
    if(century_register != 0) {
      century = (century & 0x0F) + ((century / 16) * 10);
    }
  }
 
  // Calculate the full (4-digit) year
 
  if(century_register != 0) {
    rt.year += century * 100;
  } else {
    rt.year += (CURRENT_YEAR / 100) * 100;
    if(rt.year < CURRENT_YEAR) rt.year += 100;
  }
}

void init_timer(void) {
  init_i8253();
  init_rt();
  add_irq_handle(0, update_jiffy);
  PCB *p = create_kthread("timer", timer_driver_thread);
  TIMER = p->pid;
  wakeup(p);
  list_init(&th);
  memset(tmrs, 0, sizeof(tmrs));
  hal_register("timer", TIMER, 0);
}

static void
timer_driver_thread(void) {
  static Msg m;
  int i;
  ListHead *itr;
  Timer *t;
	
  while (true) {
    receive(ANY, &m);
		
    switch (m.type) {
    case NEW_TIMER:
      i=0;
      while(tmrs[i].va && i<100) i++;
      tmrs[i].va=true;
      tmrs[i].src=m.src;
      tmrs[i].interval=m.i[0];
      list_add_before(&th, &(tmrs[i].h));
      break;
    case INTR_NOTIFY:
      list_foreach(itr, &th) {
	t=list_entry(itr, Timer, h);
	t->interval-=1;
	if (t->interval==0) {
	  t->va=false;
	  list_del(itr);
	  m.src=TIMER;
	  m.dest=t->src;
	  send(t->src, &m);
	}
      }
      break;
    default: assert(0);
    }
  }
}

long
get_jiffy() {
  return jiffy;
}

static int
md(int year, int month) {
  bool leap = (year % 400 == 0) || (year % 4 == 0 && year % 100 != 0);
  static int tab[13] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  return tab[month] + (leap && month == 2);
}

static void
update_jiffy(void) {
  jiffy ++;
  Msg a;
  a.type=INTR_NOTIFY;
  if (jiffy % HZ == 0) {
    rt.second ++;
    if (rt.second >= 60) { rt.second = 0; rt.minute ++; }
    if (rt.minute >= 60) { rt.minute = 0; rt.hour ++; }
    if (rt.hour >= 24)   { rt.hour = 0;   rt.day ++;}
    if (rt.day >= md(rt.year, rt.month)) { rt.day = 1; rt.month ++; } 
    if (rt.month >= 13)  { rt.month = 1;  rt.year ++; }
    send(TIMER, &a);
  }
}

static void
init_i8253(void) {
  int count = FREQ_8253 / HZ;
  assert(count < 65536);
  out_byte(PORT_TIME + 3, 0x34);
  out_byte(PORT_TIME, count & 0xff);
  out_byte(PORT_TIME, count >> 8);	
}

static void
init_rt(void) {
  memset(&rt, 0, sizeof(Time));
  /* Insert code here to initialize current time correctly */
  read_rtc();
}

void 
get_time(Time *tm) {
  memcpy(tm, &rt, sizeof(Time));
}
