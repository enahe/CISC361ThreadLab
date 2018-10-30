#include "t_lib.h"
#include "ud_thread.h"

tcb *running;
tcb *ready;

void t_yield()
{
  tcb *runningHead = running;
  tcb *readyHead = ready;
  tcb *tmp = ready;
  runningHead->next = NULL;
  printf("%d\n", ready->thread_id);
  while (tmp->next) {
     tmp = tmp->next;
     printf("traversing the ready queue");
  }
  tmp->next = runningHead;
  running = ready;
  ready = ready->next;
  swapcontext(&(tmp->thread_context), &(running->thread_context));
}

void t_init()
{
  tcb *initialSetup;
  initialSetup  = (tcb *) malloc(sizeof(tcb));
 
  getcontext(&(initialSetup->thread_context));    /* let tmp be the context of main() */
  initialSetup -> thread_id = 0;
  initialSetup -> thread_priority = 0;
  initialSetup -> next = NULL;
  printf("Starting new context at %p\n", (void *) &(initialSetup->thread_context));
  running = initialSetup;
  printf("Starting new running thread at %p, with thread id %d \n", (void *) running);
}

void t_create(void(*function)(int), int id, int priority)
{
  size_t sz = 0x10000;
  tcb *addTcb;
  tcb *tempTcb = ready;
  addTcb = (tcb *) malloc(sizeof(tcb));
  getcontext(&(addTcb->thread_context));
/***
  uc->uc_stack.ss_sp = mmap(0, sz,
       PROT_READ | PROT_WRITE | PROT_EXEC,
       MAP_PRIVATE | MAP_ANON, -1, 0);
***/
  addTcb->thread_context.uc_stack.ss_sp = malloc(sz);  /* new statement */
  addTcb->thread_context.uc_stack.ss_size = sz;
  addTcb->thread_context.uc_stack.ss_flags = 0;
  addTcb->thread_context.uc_link = &(running->thread_context); 
  makecontext(&(addTcb->thread_context), (void (*)(void)) function, priority, id);
  addTcb -> thread_id = id;
  addTcb -> thread_priority = priority;
  addTcb -> next = NULL;
  printf("Starting new context at %p\n", (void *) &(addTcb->thread_context));
  if (ready == NULL) {
     ready = addTcb;
     printf("Starting new ready thread at %p, with thread id %d \n", (void *) ready, ready->thread_id);
  }
  else {
     while (tempTcb -> next != NULL) {
     tempTcb = tempTcb -> next;
     }
     tempTcb-> next = addTcb;
     printf("adding to the end of the tcb, with thread at %p and thread id %d \n", (void *) addTcb, addTcb->thread_id);
  }

}
