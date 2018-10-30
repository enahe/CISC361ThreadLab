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
 
  getcontext(tmp);    /* let tmp be the context of main() */
  initialSetup -> thread_id = 0;
  initialSetup -> thread_priority = 0;
  initialSetup -> thread_context = *tmp;
  initialSetup -> next = NULL;
  printf("Starting new context at %p\n", (void *) &(initialSetup->thread_context));
  running = initialSetup;
  printf("Starting new running thread at %p, with thread id %d \n", (void *) running);
}

void t_create(void(*function)(int), int id, int priority)
{
  printf("Here");
  size_t sz = 0x10000;
  tcb *addTcb;
  tcb *tempTcb = ready;
  addTcb = (tcb *) malloc(sizeof(tcb));
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
  uc->uc_link = &(running->thread_context); 
  makecontext(uc, (void (*)(void)) function, 1, id);
  addTcb -> thread_id = id;
  addTcb -> thread_priority = priority;
  addTcb -> thread_context = *uc;
  addTcb -> next = NULL;
  printf("Starting new context at %p\n", (void *) &(addTcb->thread_context));
  if (ready == NULL) {
     ready = addTcb;
     printf("Starting new ready thread at %p, with thread id %d \n", (void *) ready);
  }
  else {
     while (tempTcb -> next != NULL) {
     tempTcb = tempTcb -> next;
     }
     tempTcb-> next = addTcb;
     printf("adding to the end of the tcb");
  }

}
