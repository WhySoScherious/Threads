//
//  threads.c
//
//
//  Created by Scott Brandt on 5/6/13.
//
//

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
//#include <ucontext.c>
#include <ucontext.h>

#define _XOPEN_SOURCE
#include <ucontext.h>

#define INTERVAL 1
#define NUM_THREADS 16
#define NUM_SLOTS 160
static ucontext_t ctx[NUM_THREADS];
static int scheduler[NUM_SLOTS];

static void test_thread(void);
static int thread = 0;
void thread_exit(int);

void times_up (int i) {
    printf ("Switching thread\n");

    thread_yield ();
}

// This is the main thread
// In a real program, it should probably start all of the threads and then wait for them to finish
// without doing any "real" work
int main(void) {
    printf("Main starting\n");

    // Seed the random number generator
    srand (time (NULL));

    printf("Main calling thread_create\n");

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
    }

    printf ("Setting up itimer\n");

    struct itimerval itimer;

    itimer.it_interval.tv_sec = INTERVAL;
    itimer.it_interval.tv_usec = 0;
    itimer.it_value = itimer.it_interval;

    signal (SIGVTALRM, times_up);

    setitimer (ITIMER_VIRTUAL, &itimer, NULL);

    // Loop forever
    while(1) ;

    // We should never get here
    exit(0);

}

// This is the thread that gets started by thread_create
static void test_thread(void) {
    printf("In test_thread\n");

    // Loop, doing a little work then yielding to the other thread
    while(1) ;

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
