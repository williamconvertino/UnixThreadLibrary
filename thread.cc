#include "interrupt.h"
#include "thread.h"
#include <queue>
#include <map>
#include <vector>
#include <algorithm>
#include <ucontext.h>
#include <cstddef>
#include <cstdlib>
#include <stdio.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdexcept>

using namespace std;    

/*
 -------------- Helpful Macros ------------ 
*/

#define EXIT_WITH_ERROR()\
            ENABLE_INTERRUPTS()\
            return(-1);\

#define EXIT_WITHOUT_ERROR()\
        ENABLE_INTERRUPTS()\
        return(0);\

#define START_THREAD_FUNC() \
        DISABLE_INTERRUPTS()\
        try {\
            if (!libinit_active) {\
            EXIT_WITH_ERROR()\
            }\

#define END_THREAD_FUNC()\
        } catch (exception & bad_alloc) { \
            EXIT_WITH_ERROR()\
        }\
        EXIT_WITHOUT_ERROR();\

#define DISABLE_INTERRUPTS()\
        interrupt_disable();\


#define ENABLE_INTERRUPTS()\
        interrupt_enable();\


/*
 -------------- Struct Declarations ------------ 
*/

// Thread Struct

struct Thread {

    Thread() {

        //Initialize a new context.
        this->context = new ucontext_t;
        getcontext(this->context);

        //Add the relevant context variables.
        this->context->uc_stack.ss_sp = malloc(STACK_SIZE);

        if (this->context->uc_stack.ss_sp == NULL) {
            throw(exception());
        }

        this->context->uc_stack.ss_size = STACK_SIZE;
        this->context->uc_stack.ss_flags = 0;
        this->context->uc_link = NULL;

        //Mark the thread as incomplete.
        this->complete = false;
    }
    ~Thread() {
        delete( (char*)(context->uc_stack.ss_sp));
        delete(context);
    }

    ucontext_t* context;
    bool complete;   
};


// Lock Struct

struct Lock {
    Lock() {
        owner = NULL;
        waiting_queue = new queue<Thread*>;
        cv_map = new map<int,queue<Thread*>*>;
    }
    ~Lock() {
        
        //delete(waiting_queue);
        
        //  for(std::map<int, queue<Thread*>*>::iterator itr = cv_map->begin(); itr != cv_map->end(); itr++) {
        //      delete (itr->second);
        //  }

        //delete(cv_map);
    }

    Thread* owner;
    queue<Thread*>* waiting_queue;
    map<int,queue<Thread*>*>* cv_map;
};


/*
 -------------- Global Variables ------------ 
*/

// Signals if the thread library has been initialized.

bool libinit_active = false;

// A reference to the libinit context.

ucontext_t* libinit_context;

// A queue to store the threads that are ready to be run.

queue<Thread*>* ready_threads = new queue<Thread*>;

// A map of locks and their IDs

map<int,Lock*>* lock_id_map = new map<int,Lock*>;

// The thread that is currently running.

Thread* current_thread;

// All the threads that are no longer waiting to run.
vector<Thread*>* active_threads = new vector<Thread*>;

/*
 -------------- Helper Functions ------------ 
*/

void at_remove_current_thread() {
    vector<Thread*>::iterator it = active_threads->begin();

    while (it != active_threads->end()) {
        if (*it == current_thread) {
            it = active_threads->erase(it);
        } else {
            it++;
        }
    }

}

void clear_completed_threads() {
    
    vector<Thread*>::iterator it = active_threads->begin();

    while(it != active_threads->end()) {
        
        Thread* thread = *it;

        if(thread->complete) {
            it = active_threads->erase(it);
            delete(thread);
        } else {
            it++;
        }

    }

}

void clear_all_threads() {
    vector<Thread*>::iterator it = active_threads->begin();

    while(it != active_threads->end()) {
        
        Thread* thread = *it;
        
        it = active_threads->erase(it);
        delete(thread);
      

    }
}

// Determines the next availible thread and runs it.

void schedule_next_thread() {

    at_remove_current_thread();
    clear_completed_threads();

    ucontext_t* current_context;

    if (current_thread == NULL) {
        current_context = libinit_context;
    } else {
        current_context = current_thread->context;
        active_threads->push_back(current_thread);  `
    }

    if (ready_threads->empty()) {
        swapcontext(current_context, libinit_context);
    } else {
        current_thread = ready_threads->front();
        ready_threads->pop();
        swapcontext(current_context, current_thread->context);
    }

}

// The buffer function to run for each thread.

void* buffer_function(thread_startfunc_t func, void* arg) {

    //Run the user function
    ENABLE_INTERRUPTS()
    func(arg);
    DISABLE_INTERRUPTS()

    //When done, mark thread as complete and schedule new thread.
    current_thread->complete = true;
    schedule_next_thread();
}

// Finds the lock with the specified ID, or returns NULL if it doesn't exist.

Lock* find_lock(int lock_id) {

    //Find the lock in the id map.
    map<int,Lock*>::iterator lock_iterator = lock_id_map->find(lock_id);

    //If the lock cannot be found, return NULL.
    if (lock_iterator == lock_id_map->end()) {
        return(NULL);
    }

    //Else, return the lock.
    return(lock_iterator->second);
}

// Adds a lock to the lock_id_map with the specified id, then returns it.

Lock* make_lock(int lock_id) {
    Lock* new_lock = new Lock();
    lock_id_map->insert(std::pair<int,Lock*>(lock_id, new_lock));
    return(new_lock);
}

// Finds and returns the queue of threads wiating for the specified condition variable.
// If the queue does not exist, it returns NULL.

queue<Thread*>* find_cv_queue(Lock* lock, int cv_id) {
    
    //Find the queue in the cv map.
    map<int,queue<Thread*>*>::iterator queue_iterator = lock->cv_map->find(cv_id);

    //If the queue cannot be found, return NULL.
    if (queue_iterator == lock->cv_map->end()) {
        return(NULL);
    }

    //Else, return the queue.
    return(queue_iterator->second);
}

// Makes and returns a new cv queue, and adds it to the lock's cv queue map.

queue<Thread*>* make_cv_queue(Lock* lock, int cv_id) {

    //Make the new queue.
    queue<Thread*>* new_queue = new queue<Thread*>;

    //Add it to the cv map.
    lock->cv_map->insert(std::pair<int,queue<Thread*>*>(cv_id, new_queue));

    //Return the queue.
    return(new_queue);
}

void free_mem() {

    delete( (char*)(libinit_context->uc_stack.ss_sp));
    delete(libinit_context);

    clear_all_threads();

}

/*
 -------------- Thread Functions ------------ 
*/

// Initializes the library with the first thread.

int thread_libinit(thread_startfunc_t func, void *arg) {

    //If libinit was already called, return -1
    if (libinit_active) {
        return(-1);
    }

    libinit_active = true;
    
    // [KILL] inifite loops. 
    // in your libinit, NEVER in any kind of loop
    int pid = getpid();
    if(fork() == 0) {
        sleep(20);
        int result = kill(pid, SIGTERM);
        if(result != -1) {
            printf("PROCESS KILLED FOR RUNNING TO LONG\n");
        }
        exit(0);
    }


    //Create the start thread.
    thread_create(func, arg);
        
    //Get the context of libinit.
    libinit_context = new ucontext_t;
    getcontext(libinit_context);

    DISABLE_INTERRUPTS();
    //Run the first thread.
    schedule_next_thread();
    ENABLE_INTERRUPTS()

    //If successfully returned, print the message and return 0.
    printf("Thread library exiting.\n");
    free_mem();
    exit(0);
}


// Creates a new thread and adds it to the ready queue.

int thread_create(thread_startfunc_t func, void *arg) {

    START_THREAD_FUNC()

    //Create the thread struct.
    Thread* new_thread = new Thread();

    //Make the thread's context using our buffer function.
    makecontext(new_thread->context, (void(*) ())buffer_function, 2, func, arg);

    //Add it to the read_thread queue.
    ready_threads->push(new_thread);

    END_THREAD_FUNC()

}

// Yields a thread by adding it to the ready queue and scheduling a new thread.

int thread_yield() {
    
    START_THREAD_FUNC()
    
    //Add the current thread to the ready_threads queue.
    ready_threads->push(current_thread);
    schedule_next_thread();

    END_THREAD_FUNC()
}

// Makes the current thread attempt to grab the lock with the specified id.

int thread_lock(unsigned int lock_id) {

    START_THREAD_FUNC()

    //Find the lock with the specified id.
    Lock* lock = find_lock(lock_id);

    //If the lock isn't found, make a new lock with that id.
    if (lock == NULL) {
        lock = make_lock(lock_id);
    }

    //If the thread already has the lock, return with an error.
    if (lock->owner == current_thread) {
        EXIT_WITH_ERROR()
    }

    //If the lock has no owner, lock it with the current thread.
    if (lock->owner == NULL) {
        lock->owner = current_thread;
    }
    //Otherwise, add the current thread to the waiting queue and schedule the next thread.
    else {
        lock->waiting_queue->push(current_thread);
        schedule_next_thread();
    }

    END_THREAD_FUNC()
}

// Unlocks the thread with the specified id.

int thread_unlock(unsigned int lock_id) {

    START_THREAD_FUNC()

    //Find the lock with the specified id.
    Lock* lock = find_lock(lock_id);

    //If the lock does not exist or the thread calling the unlock function is not the owner,
    //return with an error. 
    if (lock == NULL || lock->owner != current_thread) {
        EXIT_WITH_ERROR();
    }

    //If the waiting queue is empty, set the owner to NULL.
    if (lock->waiting_queue->empty()) {
        lock->owner = NULL;
    }
    //Otherwise, set the owner to the next thread in the waiting queue and mark that thread
    //as ready to be run.
    else {
        Thread* next_thread = lock->waiting_queue->front();
        lock->waiting_queue->pop();

        lock->owner = next_thread;
        ready_threads->push(next_thread);
    }
    
    END_THREAD_FUNC()
}

// Unlocks the current thread, adds it to the lock's cv_map, and schedules the next thread.

int thread_wait(unsigned int lock_id, unsigned int cv_id) {

    START_THREAD_FUNC()

    //Find the lock.
    Lock* lock = find_lock(lock_id);

    //If the lock does not exist or is not owned by the current thread, exit with an error.
    if (lock == NULL || lock->owner != current_thread) {
        EXIT_WITH_ERROR()
    }

    //Find the specified cv queue in the lock.
    queue<Thread*>* cv_queue = find_cv_queue(lock, cv_id);

    //If the queue does not exist, create it.
    if (cv_queue == NULL) {
        cv_queue = make_cv_queue(lock, cv_id);
    }

    // //Add the current thread to the cv_queue.
    cv_queue->push(current_thread);

    ENABLE_INTERRUPTS()
    //Unlock and schedule the next thread.
    thread_unlock(lock_id);
   
    DISABLE_INTERRUPTS()
    schedule_next_thread();
   
    ENABLE_INTERRUPTS()
    //When returning from wait, regrab the lock.
    thread_lock(lock_id);

    DISABLE_INTERRUPTS()
    END_THREAD_FUNC();
}

// Gets the next thread in the cv queue and lets it run.

int thread_signal(unsigned int lock_id, unsigned int cv_id) {

    START_THREAD_FUNC()

    //Find the lock.
    Lock* lock = find_lock(lock_id);

    //If the lock does not exist, exit without an error.
    if (lock == NULL) {
        EXIT_WITHOUT_ERROR()
    }

    //Find the specified cv queue in the lock.
    queue<Thread*>* cv_queue = find_cv_queue(lock, cv_id);

    //If the queue does not exist, exit without an error.
    if (cv_queue == NULL || cv_queue->empty()) {
        EXIT_WITHOUT_ERROR()
    }

    //Find the next thread in the cv queue.
    Thread* next_thread = cv_queue->front();
    cv_queue->pop();

    ready_threads->push(next_thread);

    END_THREAD_FUNC()
 }

// Signals to all threads in the cv queue.

int thread_broadcast(unsigned int lock_id, unsigned int cv_id) {

    START_THREAD_FUNC()

    //Find the lock.
    Lock* lock = find_lock(lock_id);

    //If the lock does not exist, exit without an error.
    if (lock == NULL) {
        EXIT_WITHOUT_ERROR()
    }

    //Find the specified cv queue in the lock.
    queue<Thread*>* cv_queue = find_cv_queue(lock, cv_id);

    //If the queue does not exist, exit without an error.
    if (cv_queue == NULL || cv_queue->empty()) {
        EXIT_WITHOUT_ERROR()
    }

    while (!cv_queue->empty()) {

        //Find the next thread in the cv queue.
        Thread* next_thread = cv_queue->front();
        cv_queue->pop();

        ready_threads->push(next_thread);

    }

    END_THREAD_FUNC()

}