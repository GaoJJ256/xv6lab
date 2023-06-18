#include "types.h"
#include "riscv.h"
#include "param.h"
#include "defs.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
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
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
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


  argint(0, &n);
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// #define LAB_PGTBL // --------------------------------------------------- << here !
#ifdef LAB_PGTBL
int
sys_pgaccess(void)
{
  //接收参数
    int len;
    uint64 addr;
    int bitmask;
    argaddr(0, &addr);
    argint(1, &len);
    if(len > 32 || len < 0){
        printf("too many pages to check\n");
        exit(1);
    }
    argint(2, &bitmask);

    int ans = 0x0;
    struct proc * p = myproc();

    for(int i = 0; i < len; i ++)
    {
        int va = addr + i * PGSIZE;
        if(va >= MAXVA)
            goto bad;

        pte_t *pte = walk(p->pagetable, va, 0);
        if(pte == 0)
            goto bad;
        
        if((*pte & PTE_A) != 0)
            ans |= (1 << i);
        *pte &= ~PTE_A;  //将PTE_A位置0 
    }

    if(copyout(p->pagetable, bitmask, (char *)&ans, sizeof(ans)) < 0)
        return -1;

    return 0;

    bad:
        printf("some problems in va2pte\n");
        return 1;
}
#endif

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
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
