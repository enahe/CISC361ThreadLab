/* 
 * thread library function prototypes
 */
#include <ucontext.h>
#include <string.h>
struct tcb {
	  int         thread_id;
          int         thread_priority;
	  ucontext_t  thread_context;
	  struct tcb *next;
       };

typedef struct tcb tcb;

struct sem_t{
         int count;
         tcb *q;
       };
typedef struct sem_t sem_t;

struct messageNode {
         char *message;     // copy of the message 
         int  len;          // length of the message 
         int  sender;       // TID of sender thread 
         int  receiver;     // TID of receiver thread 
         struct messageNode *next; // pointer to next node 
       };
typedef struct messageNode messageNode;

struct mbox {
	  struct messageNode  *msg;       // message queue
	  sem_t               *mbox_sem;
       };
typedef struct mbox mbox;

//t_create: Creates a new thread with a given thread id and priority
void t_create(void(*function)(int), int thread_id, int priority);
//t_yield: yields the cpu and swaps out the currently running process with the 
//first of the ready processes. It puts the currently running process on the end of it
void t_yield(void);
//t_init: creates the main running thread. 
void t_init(void);
//t_shutdown: shuts down all the running and ready processes, and frees everything
void t_shutdown(void);
//t_terminate: frees the running process. 
void t_terminate(void);
//int sem_init: creates a new semaphore pointed to by sp with a count value of sem_count
int sem_init(sem_t **sp, int sem_count);
//void sem_wait: current thread does a sait on the specified semaphore
void sem_wait(sem_t *sp);
// void sem_signal: current thread does a signal on the specified semaphore
void sem_signal(sem_t *sp);
// void sem_destroy: destroy(free) any state realted to the specifed semaphore
void sem_destroy(sem_t **sp);
 int mbox_create(mbox **mb); /* Create a mailbox pointed to by mb. */
void mbox_destroy(mbox **mb); /* Destroy any state related to the mailbox pointed to by mb. */
void mbox_deposit(mbox *mb, char *msg, int len); /* Deposit message msg of length len into the mailbox pointed to by mb. */
void mbox_withdraw(mbox *mb, char *msg, int *len); /* Withdraw the first message from the mailbox pointed to by mb into msg and set the message's length in len accordingly. The caller of mbox_withdraw() is responsible for allocating the space in which the received message is stored. If there is no message in the mailbox, len is set to 0. mbox_withdraw() is not blocking. Even if more than one message awaits the caller, only one message is returned per call to mbox_withdraw(). Messages are withdrew in the order in which they were deposited. */




