#include "interrupt.h"
#include "thread.h"
#include <cstdlib>
#include <cstdio> 

using namespace std;

// the point of this simple test is to ensure that mutex lock actually
// does something.  If mutex lock is unimplemented, A will not be
// blocked from locking mutex 1.

/*

A correct output for the test looks like this:

B locks mutex
B unlocks mutex
A locks the mutex
A unlocks the mutex
Thread library exiting.

An incorrect output for the test looks like this:

B locks mutex
A locks the mutex
A unlocks the mutex
B unlocks mutex
Thread library exiting.


 */

int var = 0;

void A(void* arg) {
     while(var == 0) {
         thread_yield();
     }
     thread_lock(1);
    printf("A locks the mutex\n");
    thread_unlock(1);
    printf("A unlocks the mutex\n");
} 

void B(void* arg) {
    thread_lock(1);
    printf("B locks mutex\n");
     var = 1;
    // // we yield to try and trick A to get into the critical section
    thread_yield();
    thread_unlock(1);
    printf("B unlocks mutex\n");
}

void start(void* arg) {
    thread_create(A, NULL);
    thread_create(B, NULL);

}

int main() {

	thread_libinit(start, NULL);
    
}
