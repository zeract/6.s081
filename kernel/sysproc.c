#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  if(argint(0, &n) < 0)
    return -1;
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  if(argaddr(0, &p) < 0)
    return -1;
  return wait(p);
}

uint64
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;
  
  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  backtrace();
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

//lab4 traps
uint64
sys_sigreturn(void)
{
  myproc()->trapframe->epc = myproc()->save_epc;
  myproc()->trapframe->a0 = myproc()->save_a0;
  myproc()->trapframe->a1 = myproc()->save_a1;
  myproc()->trapframe->a2 = myproc()->save_a2;
  myproc()->trapframe->a3 = myproc()->save_a3;
  myproc()->trapframe->a4 = myproc()->save_a4;
  myproc()->trapframe->a5 = myproc()->save_a5;
  myproc()->trapframe->a6 = myproc()->save_a6;
  myproc()->trapframe->a7 = myproc()->save_a7;
  myproc()->trapframe->s1 = myproc()->save_s1;
  myproc()->trapframe->s0 = myproc()->save_s0;
  myproc()->trapframe->ra = myproc()->save_ra;
  myproc()->trapframe->sp = myproc()->save_sp;
  myproc()->on_handler=0;
  //trap_handler=0;
  return 0;
}
//lab4 traps
uint64
sys_sigalarm(void)
{
  int tick;
  uint64 p;

  if(argint(0, &tick)<0){
    return -1;
  }
  if(argaddr(1,&p)<0){
    return -1;
  }
  myproc()->interval = tick;
  myproc()->handler = p;
  return 0;
}