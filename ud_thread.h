/* 
 * thread library function prototypes
 */
#include <ucontext.h>
void t_create(void(*function)(int), int thread_id, int priority);
void t_yield(void);
void t_init(void);
void t_shutdown(void);
void t_terminate(void);

struct tcb {
	  int         thread_id;
          int         thread_priority;
	  ucontext_t  thread_context;
	  struct tcb *next;
       };

typedef struct tcb tcb;
