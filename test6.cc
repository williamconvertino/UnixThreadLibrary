#include "interrupt.h"
#include "thread.h"
#include <cstdlib>
#include <cstdio> 

using namespace std;

//Tests if signal works.


int broadcast_sum  = 0;

void broadcast_thread_1(void* args) {
    thread_lock(1);
    thread_wait(1,1);
    broadcast_sum++;
    thread_unlock(1);
}

void broadcast_thread_3(void* args) {
    thread_lock(1);
    thread_signal(1,1);
    thread_unlock(1);
    thread_yield();
    if (broadcast_sum == 1) {
        printf("SUCCESS: Only 1 thread awoken\n");
    } else {
        printf("ERROR: %d threads open\n", broadcast_sum);
    }
}


void start(void* args) {
    thread_create(broadcast_thread_1, NULL);
    thread_create(broadcast_thread_1, NULL);
    thread_create(broadcast_thread_1, NULL);
    thread_create(broadcast_thread_1, NULL);
    thread_create(broadcast_thread_1, NULL);
    thread_create(broadcast_thread_3, NULL);
}


int main () {
    thread_libinit(start, NULL);
}