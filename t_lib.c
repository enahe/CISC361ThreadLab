#include "t_lib.h"
#include "ud_thread.h"

tcb *running;
tcb *ready;

void t_yield()
{
  ucontext_t *tmp;

  tmp = running;
  running = ready;
  ready = tmp;

  swapcontext(ready, running);
}

void t_init()
{
  tcb *initialSetup;
  initialSetup  = (tcb *) malloc(sizeof(tcb));
  ucontext_t *tmp;
  tmp = (ucontext_t *) malloc(sizeof(ucontext_t));
  initialSetup -> thread_id = 1;
  initialSetup -> thread_priority = 1;
  initialSetup -> thread_context = *tmp;
  initialSetup -> next = NULL;
  getcontext(&(initialSetup->thread_context));    /* let tmp be the context of main() */
  printf("Starting new context at %p\n", (void *) &(initialSetup->thread_context));
  running = initialSetup;
  printf("Starting new running thread at %p, with thread id %d \n", (void *) running);
}

void t_create(void(*function)(int), int thread_id, int priority)
{
  size_t sz = 0x10000;

  ucontext_t *uc;
  uc = (ucontext_t *) malloc(sizeof(ucontext_t));

  getcontext(uc);
/***
  uc->uc_stack.ss_sp = mmap(0, sz,
       PROT_READ | PROT_WRITE | PROT_EXEC,
       MAP_PRIVATE | MAP_ANON, -1, 0);
***/
  uc->uc_stack.ss_sp = malloc(sz);  /* new statement */
  uc->uc_stack.ss_size = sz;
  uc->uc_stack.ss_flags = 0;
  uc->uc_link = running; 
  makecontext(uc, (void (*)(void)) function, 1, thread_id);
  ready = uc;
}
