// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem,kmem1,kmem2;

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  initlock(&kmem1.lock,"kmem");
  initlock(&kmem2.lock,"kmem");
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);
  push_off();
  int id = cpuid();
  pop_off();
  if(id==0){
    r = (struct run*)pa;

    acquire(&kmem.lock);
    r->next = kmem.freelist;
    kmem.freelist = r;
    release(&kmem.lock);
  }
  else if(id==1){
    r = (struct run*)pa;
    acquire(&kmem1.lock);
    r->next = kmem1.freelist;
    kmem1.freelist = r;
    release(&kmem1.lock);
  }
  else{
    r = (struct run*)pa;
    acquire(&kmem2.lock);
    r->next = kmem2.freelist;
    kmem2.freelist = r;
    release(&kmem2.lock);
  }
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;
  push_off();
  int id = cpuid();
  pop_off();
  //printf("%d",id);
  if(id==0){
    acquire(&kmem.lock);
  r = kmem.freelist;
  if(!r){
    if(kmem1.freelist){
      acquire(&kmem1.lock);
      r = kmem1.freelist;
      kmem1.freelist = r->next;
      release(&kmem1.lock);
      release(&kmem.lock);
      memset((char*)r,5,PGSIZE);
      return (void*)r;
    }
    if(kmem2.freelist){
      acquire(&kmem2.lock);
      r = kmem2.freelist;
      kmem2.freelist = r->next;
      release(&kmem2.lock);
      release(&kmem.lock);
      memset((char*)r,5,PGSIZE);
      return (void*)r;
    }
  }
  if(r)
    kmem.freelist = r->next;
  
  release(&kmem.lock);

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
  }
  else if(id==1){
    acquire(&kmem1.lock);
  r = kmem1.freelist;
  if(!r){
    if(kmem.freelist){
      acquire(&kmem.lock);
      r = kmem.freelist;
      kmem.freelist = r->next;
      release(&kmem.lock);
      release(&kmem1.lock);
      memset((char*)r,5,PGSIZE);
      return (void*)r;
    }
    if(kmem2.freelist){
      acquire(&kmem2.lock);
      r = kmem2.freelist;
      kmem2.freelist = r->next;
      release(&kmem2.lock);
      release(&kmem1.lock);
      memset((char*)r,5,PGSIZE);
      return (void*)r;
    }
  }
  if(r)
    kmem1.freelist = r->next;
  release(&kmem1.lock);

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
  }
  else{
    acquire(&kmem2.lock);
  r = kmem2.freelist;
  if(!r){
    if(kmem.freelist){
      acquire(&kmem.lock);
      r = kmem.freelist;
      kmem.freelist = r->next;
      release(&kmem.lock);
      release(&kmem2.lock);
      memset((char*)r,5,PGSIZE);
      return (void*)r;
    }
    if(kmem1.freelist){
      acquire(&kmem1.lock);
      r = kmem1.freelist;
      kmem1.freelist = r->next;
      release(&kmem1.lock);
      release(&kmem2.lock);
      memset((char*)r,5,PGSIZE);
      return (void*)r;
    }
  }
  if(r)
    kmem2.freelist = r->next;
  release(&kmem2.lock);

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
  }
}
