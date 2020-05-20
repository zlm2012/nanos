#ifndef __TYPES_H__
#define __TYPES_H__

/* USE THESE TYPES to specify integer varaiable size
   prefix "u" means unsigned, while suffix number indicates size */
typedef unsigned int   uint32_t;
typedef          int   int32_t;
typedef unsigned short uint16_t;
typedef          short int16_t;
typedef unsigned char  uint8_t;
typedef          char  int8_t;
typedef unsigned char  bool;

typedef          int   pid_t;
typedef          int   size_t;
typedef          int   off_t;

/* Structure of a ELF binary header */
typedef struct ELFHeader {
	unsigned int   magic;
	unsigned char  elf[12];
	unsigned short type;
	unsigned short machine;
	unsigned int   version;
	unsigned int   entry;
	unsigned int   phoff;
	unsigned int   shoff;
	unsigned int   flags;
	unsigned short ehsize;
	unsigned short phentsize;
	unsigned short phnum;
	unsigned short shentsize;
	unsigned short shnum;
	unsigned short shstrndx;
} ELFHeader;

/* Structure of program header inside ELF binary */
typedef struct ProgramHeader {
	unsigned int type;
	unsigned int off;
	unsigned int vaddr;
	unsigned int paddr;
	unsigned int filesz;
	unsigned int memsz;
	unsigned int flags;
	unsigned int align;
} ProgHeader;

typedef struct SectionHeader {
	unsigned int name;
	unsigned int type;
	unsigned int flags;
	unsigned int addr;
	unsigned int offset;
	unsigned int size;
	unsigned int link;
	unsigned int info;
	unsigned int addralign;
	unsigned int entsize;
} SectionHeader;

#endif
