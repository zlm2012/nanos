/*
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Chris Torek.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
#include "common.h"
#include "string.h"

#define to_char(val)	((val) + '0')
#define to_digit(val)	((val) - '0')
#define is_digit(val)	((unsigned)to_digit(val) <= 9)

#define LONG_MAX 0xffffffff
#define INT_MAX  0xffffffff

/* Convert an unsigned long to ASCII for vfprintf */
static char *_ultoa(unsigned long val, char *endp, int base, int octzero, char *xdigs) {
  char *cp = endp;
  *cp = '\0';
  unsigned long sval;

  switch (base) {
  case 10:
    if (val < 10) {
      *--cp = to_char(val);
      return cp;
    }
    if (val > LONG_MAX) {
      *--cp = to_char(val % 10);
      sval = val /10;
    } else
      sval = val;
    do {
      *--cp = to_char(sval % 10);
      sval /= 10;
    } while (sval != 0);
    break;
  case 8:
    do {
      *--cp = to_char(val & 7);
      val >>= 3;
    } while (val);
    if (octzero && *cp != '0')
      *--cp = '0';
    break;
  case 16:
    do {
      *--cp = xdigs[val & 15];
      val >>= 4;
    } while (val);
    break;
  default:
    break;
  }
  return cp;
}

void* memchr(const void *s, unsigned char c, size_t n) {
  if (n != 0) {
    const unsigned char *p = s;
    do {
      if (*p++ == c)
	return ((void *)(p - 1));
    } while (--n != 0);
  }
  return (NULL);
}

/* Flags used during conversion */
#define ALT		0x01	/* alternate form */
#define HEXPREFIX	0x02	/* add 0x or 0X prefix */
#define LADJUST		0x04	/* left adjustment */
#define LONGDBL		0x08	/* long double */
#define LONGINT		0x10	/* long integer */
#define SHORTINT	0x20	/* short integer */
#define ZEROPAD		0x40	/* zero (as opposed to blank) pad */
#define FPT		0x80	/* floating point number */

#define BUF		68

/*
 * Newly implemented vfprintf, adapted from Berkeley.
 * To be as simple as possible, this version has no
 * floating number and quad integer support.
 */
int vfprintf(void (*printer)(char), const char *fmt0, void ** args) {
  char *fmt;			/* format string */
  int ch;			/* charcter from fmt */
  int n, n2;			/* handy integer (short term usage) */
  char *cp;			/* handy char pointer (short term usage) */
  int flags;			/* flags as above */
  int ret;			/* return value accumulator */
  int width;			/* width from format, or 0 */
  int prec;			/* precision from format, or -1 */
  char sign;			/* sign prefix */
  unsigned long ulval = 0;	/* integer args */
  int base;			/* base for [diouxX] conversion */
  int dprec;			/* a copy of prec if [diouxX], 0 otherwise */
  int realsz;			/* field size expanded by dprec, sign, etc */
  int size;			/* size of converted field or string */
  int prsize;			/* max size of printed field */
  char *xdigs = NULL;		/* digits for [xX] conversion */
  char buf[BUF];		/* space for %c, %[diouxX] */
  char ox[2];			/* space for 0x hex-prefix */
  int nextarg;			/* arg index */
  void **argtable = (void**)args;
  int i;

#define GETARG(type)				\
  ((type)(argtable[nextarg++]))

#define SARG()					\
  (flags&LONGINT ? GETARG(long) :		\
   flags&SHORTINT ? (long)(short)GETARG(int) :	\
   (long)GETARG(int))
  
#define UARG()								\
  (flags&LONGINT ? GETARG(unsigned long) :				\
   flags&SHORTINT ? (unsigned long)(unsigned short)GETARG(int) :	\
   (unsigned long)GETARG(unsigned int))
  
  /*
   * Get * arguments, including the form *nn$. Preserve the nextarg
   * that the argument can be gotten once the type is determined.
   */
#define GETASTER(val)				\
  n2 = 0;					\
  cp = fmt;					\
  while (is_digit(*cp)) {			\
    n2 = 10 * n2 + to_digit(*cp);		\
    cp++;					\
  }						\
  if (*cp == '$') {				\
    int hold = nextarg;				\
    nextarg = n2 - 1;				\
    val = GETARG(int);				\
    nextarg = hold;				\
    fmt = ++cp;					\
  } else {					\
    val = GETARG(int);				\
  }
  
#define PADSIZE 16
  static char blanks[PADSIZE] =
    {' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' '};
  static char zeroes[PADSIZE] =
    {'0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0'};

#define PAD(howmany, with) {				\
    if ((n = (howmany)) > 0) {				\
      while (n > PADSIZE) {				\
	for(i=0; i<PADSIZE; i++) printer(with[i]);	\
	n -= PADSIZE;					\
      }							\
      for(i=0; i<n; i++) printer(with[i]);		\
    }							\
  }
      

  fmt = (char *)fmt0;
  nextarg = 0;
  ret = 0;

  for (;;) {
    for (cp = fmt; (ch = *fmt) != '\0' && ch != '%'; fmt++);
    if((n = fmt - cp) != 0) {
      if ((unsigned)ret + n > INT_MAX)
	goto error;
      for(; cp < fmt; cp++) printer(*cp);
      ret += n;
    }
    if (ch == '\0')
      goto done;
    fmt++;

    flags = 0;
    dprec = 0;
    width = 0;
    prec = -1;
    sign = '\0';

  rflag:
    ch = *fmt++;
  reswitch:
    switch (ch) {
    case ' ':
      if (!sign)
	sign = ' ';
      goto rflag;
    case '#':
      flags |= ALT;
      goto rflag;
    case '*':
      GETASTER(width);
      if (width >= 0)
	goto rflag;
      width = -width;
      /* FALLTHROUGH */
    case '-':
      flags |= LADJUST;
      goto rflag;
    case '+':
      sign = '+';
      goto rflag;
    case '.':
      if ((ch = *fmt++) == '*') {
	GETASTER (n);
	prec = n < 0 ? -1 : n;
	goto rflag;
      }
      n = 0;
      while (is_digit(ch)) {
	n = 10 * n + to_digit(ch);
	ch = *fmt++;
      }
      prec = n < 0 ? -1 : n;
      goto reswitch;
    case '0':
      flags |= ZEROPAD;
      goto rflag;
    case '1': case '2': case '3': case '4': case '5':
    case '6': case '7': case '8': case '9':
      n = 0;
      do {
	n = 10 * n + to_digit(ch);
	ch = *fmt++;
      } while (is_digit(ch));
      if (ch == '$') {
	nextarg = n-1;
	goto rflag;
      }
      width = n;
      goto reswitch;
    case 'h':
      flags |= SHORTINT;
      goto rflag;
    case 'l':
      /* no quad support now, %ll ignored... */
      flags |= LONGINT;
      goto rflag;
    case 'c':
      *(cp = buf) = GETARG(int);
      size = 1;
      sign = '\0';
      break;
    case 'D':
      flags |= LONGINT;
      /*FALLTHROUGH*/
    case 'd':
    case 'i':
      ulval = SARG();
      if ((long)ulval < 0) {
	ulval = (~ulval)+1;
	sign = '-';
      }
      base = 10;
      goto number;
    case 'n':
      if (flags & LONGINT)
	*GETARG(long *) = ret;
      else if (flags & SHORTINT)
	*GETARG(short *) = ret;
      else
	*GETARG(int *) = ret;
      continue; /* no output */
    case 'O':
      ulval = UARG();
      base = 8;
      goto nosign;
    case 'p':
      ulval = (unsigned long)GETARG(void *);
      base = 16;
      xdigs = "0123456789abcedf";
      flags = flags | HEXPREFIX;
      ch = 'x';
      goto nosign;
    case 's':
      if ((cp = GETARG(char *)) == NULL)
	cp = "(null)";
      if (prec >= 0) {
	char *p = memchr(cp, 0, (size_t)prec);

	if (p != NULL) {
	  size = p - cp;
	  if (size > prec)
	    size = prec;
	} else
	  size = prec;
      } else
	size = strlen(cp);
      sign = '\0';
      break;
    case 'U':
      flags |= LONGINT;
      /* FALLTHROUGH */
    case 'u':
      ulval = UARG();
      base = 10;
      goto nosign;
    case 'X':
      xdigs = "0123456789ABCDEF";
      goto hex;
    case 'x':
      xdigs = "0123456789abcdef";
    hex:
      ulval = UARG();
      base = 16;
      /* leading 0x/X ONLY IF non-zero */
      if (flags & ALT && (ulval != 0))
	flags |= HEXPREFIX;

      /* unsigned conversions */
    nosign:
      sign = '\0';
    number:
      if ((dprec = prec) >= 0)
	flags &= ~ZEROPAD;

      cp = buf + BUF;
      if (ulval != 0 || prec != 0)
	cp = _ultoa(ulval, cp, base, flags & ALT, xdigs);
      size = buf + BUF - cp;
      break;
    default:
      if (ch == '\0')
	goto done;
      cp = buf;
      *cp = ch;
      size = 1;
      sign = '\0';
      break;
    }

    /* Padding... */
    realsz = dprec > size ? dprec : size;
    if (sign)
      realsz++;
    else if (flags & HEXPREFIX)
      realsz += 2;

    prsize = width > realsz ? width : realsz;
    if ((unsigned)ret + prsize > INT_MAX)
      goto error;

    if ((flags & (LADJUST|ZEROPAD)) == 0)
      PAD(width - realsz, blanks);

    if (sign) {
      printer(sign);
    } else if (flags & HEXPREFIX) {
      ox[0] = '0';
      ox[1] = ch;
      printer(ox[0]);
      printer(ox[1]);
    }

    if ((flags & (LADJUST|ZEROPAD)) == ZEROPAD)
      PAD(width - realsz, zeroes);

    PAD(dprec - size, zeroes);

    for(i=0; i < size; i++) printer(cp[i]);

    if (flags & LADJUST)
      PAD(width - realsz, blanks);

    ret += prsize;
  }
 done:
 error:
  return (ret);
}

extern void serial_printc(char);

/* __attribute__((__noinline__))  here is to disable inlining for this function to avoid some optimization problems for gcc 4.7 */
void __attribute__((__noinline__)) 
printk(const char *ctl, ...) {
	void **args = (void **)&ctl + 1;
	vfprintf(serial_printc, ctl, args);
}
