#include "interrupt.h"
#include "thread.h"
#include <queue>
#include <map>
#include <ucontext.h>
#include <cstddef>
#include <cstdlib>
#include <stdio.h>

using namespace std;    

int thread_count = 0;

struct Thread {
    Thread(ucontext_t* context) {
        this->context = context;
        this->complete = false;
        this->id = thread_count+1;
        thread_count++;
    }
    ~Thread() {
        delete(context);
    }
    ucontext_t* context;
    bool complete;   
    int id; 
};

struct Lock {
    Lock() {
        owner = NULL;
    }
    ~Lock() {
        delete(&wating_queue);

        for(std::map<int, queue<Thread*>*>::iterator itr = cv_map.begin(); itr != cv_map.end(); itr++) {
        delete (itr->second);
        }
        delete(&cv_map);
    }
    Thread* owner;
    queue<Thread*> wating_queue;
    map<int,queue<Thread*>*> cv_map;
};

bool libinit_active = false;

//Global Variables
queue<Thread*> readyThreads;

map<int,Lock*> myLocks;

Thread* test;

ucontext_t* libinit_context;

Thread* active_thread;
Thread* previous_thread;

//Returns the next thread to run.
void scheduler() {
    
    //If the previous thread is complete, delete it.
    if (previous_thread != NULL && previous_thread->complete) {
        delete(previous_thread);
        previous_thread = NULL;
    }

    //If there are more threads to run, run them.
    if (!readyThreads.empty()) {
        previous_thread = active_thread;
        active_thread = readyThreads.front();
        readyThreads.pop();
        swapcontext(previous_thread->context, active_thread->context);
    }
    
    //If there are no more threads to run, and the active thread is complete, return to libinit.
    if (active_thread->complete) {
        swapcontext(active_thread->context, libinit_context);
    }
}

//Initializes the library with the first thread.
int thread_libinit(thread_startfunc_t func, void *arg) {

    //If libinit was already called, return -1
    if (libinit_active) {
        return(-1);
    }

    libinit_active = true;
    
    //Create the start thread.
    thread_create(func, arg);
        
    //Get the context of libinit.
    libinit_context = (ucontext_t*)malloc(sizeof(ucontext_t));
    getcontext(libinit_context);

    //Run the start thread.
    active_thread = readyThreads.front();
    readyThreads.pop();
    swapcontext(libinit_context, active_thread->context);
    
    // //Remove the final threads and exit.
    // if (active_thread != NULL) {
    //     //delete(active_thread);
    //     active_thread = NULL;
    // }
    printf("Thread library exiting.\n");
    return(0);
}

//The buffer function to run for each thread.
void* buffer_function(thread_startfunc_t func, void* arg) {

    //Run the user function
    //interrupt_enable();
    func(arg);
    //interrupt_disable();

    //When done, mark thread as complete and schedule new thread.
    active_thread->complete = true;
    scheduler();
}

int thread_create(thread_startfunc_t func, void *arg) {

    if (!libinit_active) {
        return(-1);
    }

    //Initialize the thread's context.
    ucontext_t* context_ptr = (ucontext_t*)malloc(sizeof(ucontext_t));
    getcontext(context_ptr);
    
    context_ptr->uc_stack.ss_sp = malloc(STACK_SIZE);
    context_ptr->uc_stack.ss_size = STACK_SIZE;
    context_ptr->uc_stack.ss_flags = 0;
    context_ptr->uc_link = NULL;

    makecontext(context_ptr, (void(*) ())buffer_function, 2, func, arg);

    //Create the thread struct.
    Thread* new_thread = new Thread(context_ptr);

    //Add it to the read_thread queue.
    readyThreads.push(new_thread);

    return(0);

}

//Yields the current thread, sending it to the back of the readyThread queue and starting the next ready thread.
int thread_yield() {
    
    if (!libinit_active) {
        return(-1);
    }
    
    //Add the current thread to the readyThreads queue.
    readyThreads.push(active_thread);
    scheduler(); //HERE
    return(0);
}



int thread_lock(unsigned int lock_ID) {

    if (!libinit_active) {
        return(-1);
    }

    //interrupt_disable();

    //Find the lock, or make it if it doesn't exist.
    map<int,Lock*>::iterator lk = myLocks.find(lock_ID);
    if (lk == myLocks.end()) {
        myLocks.insert(std::pair<int,Lock*>(lock_ID,new Lock()));
        lk = myLocks.find(lock_ID);
    }

    Lock* myLock = lk->second;


    //printf("%d", myLock->locked);
    //If not locked, grab the lock.
    
    if (myLock->owner == NULL) {
        myLock->owner = active_thread;
        //printf("%d", myLock->owner->id);
    } else {
        //Otherwise, add the lock to the waiting queue and schedule the next thread.
        myLock->wating_queue.push(active_thread);
        scheduler();
    }
    //interrupt_enable();

    return(0);
}

int thread_unlock(unsigned int lock_ID) {

    if (!libinit_active) {
        return(-1);
    }

    //interrupt_disable();

    map<int,Lock*>::iterator lk = myLocks.find(lock_ID);
    if (lk == myLocks.end()) {
        return(-1);
    }

    Lock* myLock = lk->second;

    //If owner thread didn't call, return -1.
    if (myLock->owner != active_thread) {
        //interrupt_enable();
        return(-1);
    }

    if (!myLock->wating_queue.empty()) {
        Thread* next_thread = myLock->wating_queue.front();
        myLock->wating_queue.pop();
        myLock->owner = next_thread;
        readyThreads.push(next_thread);
    } else {
        myLock->owner = NULL;
    }
    //interrupt_enable();
    return(0);
}

int thread_wait(unsigned int lock_ID, unsigned int cond) {

    if (!libinit_active) {
        return(-1);
    }

    //interrupt_disable();

    map<int,Lock*>::iterator lk = myLocks.find(lock_ID);
    if (lk == myLocks.end()) {
        return(-1);
    }

    Lock* myLock = lk->second;

    //If owner thread didn't call, return -1.
    if (myLock->owner != active_thread) {
        //interrupt_enable();
        return(-1);
    }

    //Find the lock, or make it if it doesn't exist.
    map<int,queue<Thread*>*>::iterator qu = myLock->cv_map.find(cond);
    if (qu == myLock->cv_map.end()) {
        myLock->cv_map.insert(std::pair<int,queue<Thread*>*>(cond,new queue<Thread*>()));
        qu = myLock->cv_map.find(cond);
    }
    queue<Thread*>* myQueue = qu->second;
    myQueue->push(active_thread);
    
    if (!myLock->wating_queue.empty()) {
        Thread* next_thread = myLock->wating_queue.front();
        myLock->wating_queue.pop();
        myLock->owner = next_thread;
        readyThreads.push(next_thread);
    } else {
        myLock->owner = NULL;
    }

    //scheduler();

    return(0);
}

int thread_signal(unsigned int lock_ID, unsigned int cond) {

    
    if (!libinit_active) {
        return(-1);
    }

    //interrupt_disable();

    map<int,Lock*>::iterator lk = myLocks.find(lock_ID);
    if (lk == myLocks.end()) {
        return(-1);
    }

    Lock* myLock = lk->second;

    //If owner thread didn't call, return -1.
    if (myLock->owner != active_thread) {
        //interrupt_enable();
        return(-1);
    }

    //Find the lock, or make it if it doesn't exist.
    map<int,queue<Thread*>*>::iterator qu = myLock->cv_map.find(cond);
    if (qu == myLock->cv_map.end()) {
        myLock->cv_map.insert(std::pair<int,queue<Thread*>*>(cond,new queue<Thread*>()));
        qu = myLock->cv_map.find(cond);
    }
    queue<Thread*>* myQueue = qu->second;


    return(0);
}

int thread_broadcast(unsigned int lock, unsigned int cond) {
    return(0);
}