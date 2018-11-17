/* 
 * thread library function prototypes
 */
#include <ucontext.h>
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




