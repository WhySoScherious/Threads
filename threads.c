//
// threads.c
//
//
// Created by Scott Brandt on 5/6/13.
//
//

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <ucontext.h>
#include <signal.h>

#define _XOPEN_SOURCE
#include <ucontext.h>

#define NUM_THREADS 16
#define NUM_SLOTS 160

typedef void (*sighandler_t)(int);

static ucontext_t ctx[NUM_THREADS];
static int scheduler[NUM_SLOTS];

static void test_thread(void);
static int thread = 0;
void thread_exit(int);

void thread_handler(int signum){
   thread_yield();
   printf("Timer expired!\n");
}

// This is the main thread
// In a real program, it should probably start all of the threads and then wait for them to finish
// without doing any "real" work
int main(void) {
    srand (time (NULL));
    printf("Main starting\n");

    printf("Main calling thread_create\n");

    
    struct itimerval itval;
    itval.it_interval.tv_sec = 1;
    itval.it_interval.tv_usec = 0;
    itval.it_value = itval.it_interval;
    setitimer(ITIMER_VIRTUAL,&itval,NULL); 
    signal(SIGVTALRM,thread_handler);
    
    // Create 15 more threads
    for (int i = 1; i < NUM_THREADS; i++) {
        thread_create(&test_thread);
    }

    printf("Main returned from thread_create\n");

    // Reset the starting thread to thread 0
    thread = 0;

    printf("Handing out lottery tickets to threads\n");
    for (int i = 0; i < NUM_SLOTS; i++) {
        scheduler[i] = rand() % NUM_THREADS;
        printf ("%d\n", scheduler[i]);
    }

    // Loop, doing a little work then yielding to the other thread
    while(1);

    // We should never get here
    exit(0);

}

// This is the thread that gets started by thread_create
static void test_thread(void) {
    printf("In test_thread\n");

    // Loop, doing a little work then yielding to the other thread
    while(1); 
    thread_exit(0);
}

// Yield to another thread
int thread_yield() {
    int old_thread = thread;

    // This is the lottery scheduler.
    thread = scheduler[rand() % NUM_SLOTS];

    printf("Thread %d yielding to thread %d\n", old_thread, thread);
    printf("Thread %d calling swapcontext\n", old_thread);

    // This will stop us from running and restart the other thread
    swapcontext(&ctx[old_thread], &ctx[thread]);

    // The other thread yielded back to us
    printf("Thread %d back in thread_yield\n", thread);
}

// Create a thread
int thread_create(int (*thread_function)(void)) {
    int newthread = ++thread;

    printf("Thread %d in thread_create\n", thread);

    printf("Thread %d calling getcontext and makecontext\n", thread);

    // First, create a valid execution context the same as the current one
    getcontext(&ctx[newthread]);

    // Now set up its stack
    ctx[newthread].uc_stack.ss_sp = malloc(8192);
    ctx[newthread].uc_stack.ss_size = 8192;

    // This is the context that will run when this thread exits
    ctx[newthread].uc_link = &ctx[thread];

    // Now create the new context and specify what function it should run
    makecontext(&ctx[newthread], test_thread, 0);

    printf("Thread %d done with thread_create\n", thread);
}

// This doesn't do anything at present
void thread_exit(int status) {
    printf("Thread %d exiting\n", thread);
}
