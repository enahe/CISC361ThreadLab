#include "t_lib.h"
#include "ud_thread.h"

tcb *running;
tcb *ready;
mbox *messageQueue;

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
  mbox_create(&(messageQueue)); 
  sem_init(&(messageQueue->mbox_sem),0);
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

int mbox_create(mbox **mb) {
 struct mbox* newMbox = (mbox *) malloc(sizeof(mbox));
    newMbox->msg = NULL;
    newMbox->mbox_sem = NULL;
    *mb = newMbox;
    return 1;
}

void mbox_destroy(mbox **mb) {
struct mbox * tempBox = *mb;
free(tempBox->msg);
free(tempBox);
}

void mbox_deposit(mbox *mb, char *msg, int len) {
struct messageNode * newMessage =(messageNode *) malloc(sizeof(messageNode));
struct messageNode * headMessage = mb->msg;
newMessage->message = malloc(len+1);
strcpy(newMessage->message, msg);
newMessage->len = len;
newMessage->next = NULL;
if (mb->msg == NULL) {
    mb->msg = newMessage;
   printf("First message added\n");
  }
else {
    while (headMessage->next) {
           headMessage = headMessage -> next;
    }
    headMessage->next = newMessage;
    printf("Message added to the end of the mailbox\n");
}
}

void mbox_withdraw(mbox *mb, char *msg, int *len) {
struct messageNode * headMessage = mb->msg;
if (headMessage == NULL) {
    len = 0;
}
else {
   strcpy(msg, headMessage->message);
   len = headMessage->len;
}
if (mb->msg != NULL) {
   mb->msg = mb->msg->next;
   free(headMessage -> message);
   free(headMessage);
}
}

void send(int tid, char *msg, int len) {
struct messageNode * newMessage =(messageNode *) malloc(sizeof(messageNode));
struct messageNode * headMessage = messageQueue->msg;
newMessage->message = malloc(len+1);
strcpy(newMessage->message, msg);
newMessage->len = len;
newMessage->receiver = tid;
newMessage->sender = running->thread_id;
newMessage->next = NULL;


if (messageQueue->msg == NULL) {
    messageQueue->msg = newMessage;
   printf("First message added\n");
  }
else {
    while (headMessage->next) {
           headMessage = headMessage -> next;
    }
    headMessage->next = newMessage;
    printf("Message added to the end of the mailbox\n");
}
sem_signal(&(messageQueue->mbox_sem));
}


void receive(int *tid, char *msg, int *len) {
struct messageNode * headMessage = messageQueue->msg;
struct messageNode * otherHead = messageQueue->msg;
struct messageNode * tempMessage;
sem_wait(&(messageQueue->mbox_sem));
if (headMessage == NULL) {
    len = 0;
}
else {
     if (((headMessage->receiver) == (running -> thread_id)) && (headMessage->sender == *tid) || (*tid == 0)) { 
      strcpy(msg, headMessage->message);
      *len = headMessage->len;
      if (headMessage != NULL) {
      messageQueue->msg = headMessage -> next;
      free(headMessage -> message);
      free(headMessage);
    }
    else {
    while (headMessage->next) {
      if (((headMessage->receiver) == (running -> thread_id)) && (headMessage->sender == *tid)) {
      strcpy(msg, headMessage->message);
      *len = headMessage->len;
      if (headMessage != NULL) {
      tempMessage = headMessage;
      tempMessage ->next = headMessage->next;
      free(headMessage -> message);
      free(headMessage);
      break; 
      }
     }
    else {
    headMessage = headMessage -> next;
    }
    }
   }
}
}
} 




