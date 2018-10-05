#include "myThread.h"

/**
 *  Defines for interacting with the thread context struct
 *
 *  NB: _BP would be 3
 */
#define _SP 4
#define _PC 5

/**
 *  Linux hides addresses in jump buffers by xoring them with gs:0x18 and then
 *  rotating them left 9.  This code does the same, so that we can put our own
 *  addresses into a jump buffer
 *
 *  NB: gcc inline assembly "=r" indicates an in/out parameter
 */
int xor_and_rol(void* p)
{
    int ret = (int)p;
#ifndef __sun__
#warning "Linux"
    asm volatile ("xorl %%gs:0x18, %0;\n roll $0x9, %0;" : "=r"(ret));
#endif
    return ret;
}

const unsigned THREADS     = 10;         // total number of threads
unsigned   curr_thread = 0;         // the current thread
char*           stack[THREADS * 8192];   // stack for each thread
jmp_buf        context[THREADS];        // jump buffer for each thread
unsigned threadID[THREADS];             //store the ID of each thread
int ID = 1;                  //the curr thread ID
unsigned joinTo[THREADS];                      //the next thread
unsigned numThread = 1;                 //the number of current threads
unsigned next = 0;                          //the next thread that should be execed
unsigned valid[THREADS];                //mark the valid threads
unsigned wait[THREADS];              //if this thread is waitting
unsigned waited[THREADS];              //if this thread is waited by others
void* (*func_ptr[THREADS])(void *);             //store pointers
void* args_array[THREADS];              //store args
bool no_handler = 0;                //can handler work

void uthread_exit(void * val_ptr){
    no_handler = 1;
    numThread --;
    valid[curr_thread] = 0; 
    wait[waited[curr_thread]] = -1;
    waited[curr_thread] = -1;
    printf("%d exited\n", threadID[curr_thread]);
    no_handler = 0;
    signal_handler(-1);
}

int uthread_join(unsigned t, void ** status){
    printf("%d wait %d by join operation\n", threadID[curr_thread], t);
    wait[curr_thread] = t;
    waited[t] = curr_thread;
    return 0;
}

void signal_handler(int i){
    if(no_handler || setjmp(context[curr_thread]) == 403) return ;
    next = scheduler();
    curr_thread = next;
    longjmp(context[curr_thread], 403);
}

void init(){
    struct sigaction act;
    struct itimerval timer;
    memset(valid,0,sizeof(valid));
    memset(wait,-1,sizeof(wait));
    memset(waited,-1,sizeof(waited));
    valid[0] = 1;

    memset(&act, 0, sizeof(act));

    act.sa_handler = &signal_handler;
    act.sa_flags = SA_NODEFER;

    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = INTERVAL;
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = INTERVAL;
    setitimer(ITIMER_REAL, &timer, 0);

    sigaction(SIGALRM, &act, NULL);
}

int threadManager(){
    (*func_ptr[curr_thread])(args_array[curr_thread]);
    uthread_exit(0);
}

int uthread_create(unsigned* t, void* (*start)(void*), void* args){
    printf("%d want to create\n",*t);
    no_handler = 1;
    if(numThread == 1) init();
    numThread ++;
    if(setjmp(context[curr_thread]) == 403) return 0;
    curr_thread = getFree();
    if(curr_thread == -1) return -1;
    valid[curr_thread] = 1; 
    
    func_ptr[curr_thread] = start;
    args_array[curr_thread] = args;
    
    ((unsigned*)context[curr_thread])[_SP] = xor_and_rol(&stack[(curr_thread + 1) * 8192]);
    ((unsigned*)context[curr_thread])[_PC] = xor_and_rol((void*)threadManager);
    *t = ID;
    threadID[curr_thread] = ID ++;
    no_handler = 0;
    longjmp(context[curr_thread], 403);
}

int uthread_self(){
    return threadID[curr_thread];
}
unsigned getFree(){
    for(unsigned i = 0;i < THREADS;++ i) 
        if(!valid[i]) return i;
    return -1;
}

unsigned scheduler(){
    int now;
    for(unsigned i = 1;i < THREADS;++ i){
        now = (i + curr_thread) % THREADS;
        if(valid[now] && wait[now] == -1) return now;
    }
    return curr_thread;
}

void uthread_yield(){
    printf("%d yield to other thread\n",curr_thread);
    signal_handler(-1);
}
