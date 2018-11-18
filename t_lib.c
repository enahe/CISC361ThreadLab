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
  //printf("%d\n", ready->thread_id);
  if(tmp != NULL) {
  while (tmp->next) {
     tmp = tmp->next;
  }
  tmp->next = runningHead;
  running = ready;
  ready = ready->next;
  swapcontext(&(runningHead->thread_context), &(running->thread_context));
  }
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
  if (ready == NULL) {
     ready = addTcb;
  }
  else {
     while (tempTcb -> next != NULL) {
     tempTcb = tempTcb -> next;
     }
     tempTcb-> next = addTcb;
  }

}
void t_terminate() {
   struct tcb * tmp = running;
   tcb * readyHead = ready;
   if (readyHead != NULL) {
   readyHead -> next == NULL;
   }
   running = readyHead;
   if (ready != NULL) {
   ready = ready->next;
   }
   free(tmp->thread_context.uc_stack.ss_sp);
   free(tmp);
   setcontext(&(running->thread_context));
}

void t_shutdown() {
   struct tcb * readyHead = ready;
   struct tcb * next;
   
   if(running != NULL) {
   free(running->thread_context.uc_stack.ss_sp);
   free(running);
   running = NULL;
   }
   
   while(readyHead != NULL) {
   next = readyHead -> next;
   free(readyHead->thread_context.uc_stack.ss_sp);
   free(readyHead);
   readyHead = next;
   }
   ready = NULL;
 
  
}

int sem_init(sem_t **sp, int sem_count) {
    struct sem_t* newSem = (sem_t *) malloc(sizeof(sem_t));
    newSem->count = sem_count;
    newSem->q = NULL;
    *sp = newSem;
    return sem_count;
}

void sem_wait(sem_t *sp) {
    sighold();
    sp->count = sp->count -1;
    tcb * semQueue = sp->q;
    tcb * readyHead = ready;
    tcb *runningHead = running;
    tcb * tmp = sp->q;
    if (runningHead != NULL) {
       runningHead->next = NULL;
    }
    if (sp->count < 0) {
     if (sp->q == NULL) {
        sp->q = runningHead; 
        printf("thread to be added to the head of sp queue: %d\n", sp->q->thread_id);
     }
     else {
        while (tmp -> next) {
           tmp = tmp ->next;
         }
     tmp -> next = runningHead;
     printf("thread to be added to the end of sp queue: %d\n", tmp->next->thread_id);
     }
     running = ready;
     ready = ready->next;
     swapcontext(&(runningHead->thread_context), &(running->thread_context));
   sigrelse();
   //sleep(1000);
   
    }
    else {
      sigrelse();
    }
}

void sem_signal(sem_t *sp) {
   sighold();
   sp->count = sp->count + 1; 
   if (sp->count <= 0) {
    tcb *semQueue = sp->q;
    tcb *readyHead = ready;
    tcb *tmp = ready;
    sp->q = sp->q->next;
    if (semQueue != NULL) {
    semQueue->next = NULL;
    }
    while (tmp->next) {
        tmp = tmp->next;
     }
    tmp->next = semQueue;
    printf("Removed thread id %d\n", semQueue->thread_id);
   sigrelse();
   
}
}

void sem_destroy(sem_t ** sp) {
    sem_t * spHead = * sp;
    tcb * next;
    
    while (spHead->q != NULL ) {
   next = spHead->q-> next;
   free(spHead->q->thread_context.uc_stack.ss_sp);
   free(spHead->q);
   spHead = spHead->q;
    }
    free(spHead);
}




