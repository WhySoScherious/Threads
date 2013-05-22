/*
 * Threads.c
 * cmps 111
 *
 * CREATED: Paul Scherer    May  8, 2013 
 * CHANGED: Paul Scherer    May 15, 2013
 * CHANGED: Nicholas Smith  May 15, 2013
 * CHANGED: Maria Mishkova  May 16, 2013
 * CHANGED: Chase Wilson    May 16, 2013
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>
#include <ucontext.h>
#include <signal.h>

#define _XOPEN_SOURCE
#define NUM_THREADS 16
#define NUM_SLOTS 160

typedef void (*sighandler_t)(int);

static ucontext_t ctx[NUM_THREADS];
static int scheduler[NUM_SLOTS];

static void test_thread(void);
static int thread = 0;
void thread_exit(int);

/* 
 * void thread_handler(int signum)
 * Calls function thread_yield() 
 * Tells when the timer for each thread expired
 */
void thread_handler(int signum)
{
   thread_yield();
   printf("Timer expired!\n");
}

/* int main(void)
 * This is the main thread
 * Calls thread_create
 * Sets up interupts to make threads preemptive using signals
 */
int main(void)
{

    /*
     * The srand() function sets its argument as the seed for a new 
     * sequence of pseudo-random integers to be returned by rand(). 
     * We use rand() when implementing lottery scheduling below.
     */
  srand (time (NULL));
    
    printf("Main starting\n");
    printf("Main calling thread_create\n");
    
    /* 
     * Sets up struct for setitimer
     * Specifies the time for intervals
     * Calls setitimer virtual
     */
    struct itimerval itval;
    itval.it_interval.tv_sec = 1;
    itval.it_interval.tv_usec = 0;
    itval.it_value = itval.it_interval;
    setitimer(ITIMER_VIRTUAL,&itval,NULL); 
    signal(SIGVTALRM,thread_handler);
    
    /* Create 15 more threads */
int i;
    for (i = 1; i < NUM_THREADS; i++) {
        thread_create(&test_thread);
    }

    printf("Main returned from thread_create\n");

    /* Reset the starting thread to thread 0 */
    thread = 0;

    printf("Handing out lottery tickets to threads\n");
    for (i = 0; i < NUM_SLOTS; i++) {
        scheduler[i] = rand() % NUM_THREADS;
        //printf ("%d\n", scheduler[i]);
    }

    /* Loop, doing a little work then yielding to the other thread */
    while(1);

    /* We should never get here */
    exit(0);
}

/*
 * static void test_thread(void)
 * This is the thread that gets started by thread_create 
 */
static void test_thread(void) 
{
    printf("In test_thread\n");

    /* Loop, doing a little work then yielding to the other thread */
    while(1);    
    thread_exit(0);
}

/* 
 * int thread_yield()
 * Yield to another thread using lottery scheduling
 * Executes swapcontext
 */
int thread_yield() 
{
    int old_thread = thread;

    /* This is the lottery scheduler. */
    thread = old_thread;
    while(thread == old_thread){
      thread = scheduler[rand() % NUM_SLOTS];
    }
    printf("Thread %d yielding to thread %d\n", old_thread, thread);
    printf("Thread %d calling swapcontext\n", old_thread);

    /* This will stop us from running and restart the other thread */
    swapcontext(&ctx[old_thread], &ctx[thread]);

    /* The other thread yielded back to us */
    printf("Thread %d back in thread_yield\n", thread);
}

/*
 * int thread_create()
 * Creates a thread
 * Sets up stack of thread
 * Executes makecontext
 */
int thread_create(int (*thread_function)(void)) 
{
    int newthread = ++thread;

    printf("Thread %d in thread_create\n", thread);

    printf("Thread %d calling getcontext and makecontext\n", thread);

    /* First, create a valid execution context the same as the current one */
    getcontext(&ctx[newthread]);

    /* Now set up its stack */
    ctx[newthread].uc_stack.ss_sp = malloc(8192);
    ctx[newthread].uc_stack.ss_size = 8192;

    /*This is the context that will run when this thread exits */
    ctx[newthread].uc_link = &ctx[thread];

    /* Now create the new context and specify what function it should run */
    makecontext(&ctx[newthread], test_thread, 0);

    printf("Thread %d done with thread_create\n", thread);
}

void thread_exit(int status) 
{
    printf("Thread %d exiting\n", thread);
}
