#include "interrupt.h"
#include "thread.h"
#include <cstdlib>
#include <cstdio> 

using namespace std;

//Tests if the thread doesn't throw an error, but functions incorrectly. - DOES NOT WORK


int yield_thread_num = 0;

void yield_thread_1(void* args) {
     thread_lock(1);
     thread_wait(1,1);
     printf("\nThread executed");
     thread_unlock(1);
}

void yield_thread_2(void* args) {
     thread_yield();
     thread_yield();
     printf("METOO");
     thread_broadcast(1,1);
}

void start(void* args) {

    
    thread_create(yield_thread_1, NULL);
    thread_create(yield_thread_2, NULL);
    thread_create(yield_thread_1, NULL);
    thread_create(yield_thread_2, NULL);
    thread_create(yield_thread_1, NULL);
}

int main() {
    thread_libinit(start, NULL);
}