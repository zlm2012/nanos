#include "kernel.h"
#include "multiboot2.h"

typedef long Align;

typedef union header {
  struct {
    union header *ptr;
    volatile unsigned size;
  } s;
  Align x;
} Header;

static Header base;
static Header *freep = 0;
struct multiboot_tag_elf_sections *elf;
SectionHeader *sh;

void *kmalloc(size_t nbytes) {
  Header *p, *prevp;
  unsigned int lowend=0, i;
  unsigned nunits;

  nunits=(nbytes+sizeof(Header)-1)/sizeof(Header)+1;
  printk("kmalloc units: %d\n", nunits);
  if ((prevp = freep) == 0) {
    elf=*(void **)(pa_to_va(ELF_INFO_PTR));
    printk("elf info: %p\n", elf);
    sh=(SectionHeader*)(&elf->sections);
    for (i=0; i<elf->entsize; i++)
      if (sh[i].addr != 0 && sh[i].addr+sh[i].size>lowend) {
        lowend=sh[i].addr+sh[i].size;
        printk("shaddr: %x, shsize: %x\n", sh[i].addr, sh[i].size);
      }
    lowend>>=12;
    lowend++;
    lowend<<=12;
    printk("lowend: %x\n", lowend);
    base.s.ptr=(Header*)lowend;
    freep=prevp=&base;
    base.s.size=0;
    base.s.ptr->s.ptr=(Header*)lowend;
    lowend-=0xc0000000;
    base.s.ptr->s.size=(KRN_MEM-lowend-sizeof(Header))/sizeof(Header);
  }

  if(nbytes==0)
    return 0;
  for (p=prevp->s.ptr;;prevp=p, p=p->s.ptr) {
    //printk("try to kmalloc size: %x, pointer: %p\n", p->s.size, p+1);
    if (p->s.size >=nunits) {
      if (p->s.size == nunits)
	prevp->s.ptr=p->s.ptr;
      else {
	p->s.size-=nunits;
	p+=p->s.size;
	p->s.size =nunits;
      }
      freep=prevp;
      //printk("kmalloc size: %x, pointer: %p\n", p->s.size, p+1);
      memset(p+1, 0, nbytes);
      return (void*)(p+1);
    }
    if (p == freep)
      return 0;
  }
}

void kfree(void *ap) {
  Header *bp, *p;
  bp=(Header *)ap-1;
  for (p=freep;!(bp>p&&bp<p->s.ptr);p=p->s.ptr)
    if (p>=p->s.ptr && (bp>p || bp< p->s.ptr))
      break;
  if (bp+bp->s.size == p->s.ptr) {
    bp->s.size += p->s.ptr->s.size;
    bp->s.ptr=p->s.ptr->s.ptr;
  } else
    bp->s.ptr=p->s.ptr;
  if (p+p->s.size==bp) {
    p->s.size+=bp->s.size;
    p->s.ptr=bp->s.ptr;
  }else
    p->s.ptr=bp;
  freep=p;
}
