#include "interrupt.h"
#include "thread.h"
#include <cstdlib>
#include <cstdio> 

using namespace std;

void immediate (void* args) {
    for (int i = 0; i < 6; i++) {
        printf(" imm ");
    }
    
}

void unlock_b(void* args) {

    for (int i = 0; i < 10; i++) {
        printf(" ub ");
    }

}

void unlock_a(void* args) {
    thread_lock(1);
    thread_unlock(1);
    for (int i = 0; i < 10; i++) {
        printf(" ua ");
    }
}

void ready_fifo_1(void* args) {
    thread_lock(1);
    printf(" rf1 ");
    thread_unlock(1);
}
void ready_fifo_2(void* args) {
    thread_lock(1);
    printf(" rf2 ");
    thread_unlock(1);
}
void ready_fifo_3(void* args) {
    thread_lock(1);
    printf(" rf3 ");
    thread_unlock(1);
}
void ready_fifo_4(void* args) {
    thread_lock(1);
    printf(" rf4 ");
    thread_unlock(1);
}
void signal_fifo_1(void* args) {
    thread_lock(5);
    thread_wait(5,1);
    printf(" sf1 ");
    thread_signal(5,1);
    thread_unlock(5);
}
void signal_fifo_2(void* args) {
    thread_lock(5);
    thread_wait(5,1);
    printf(" sf2 ");
    thread_signal(5,1);
    thread_unlock(5);
}
void signal_fifo_3(void* args) {
     thread_lock(5);
     thread_wait(5,1);
    printf(" sf3 ");
    thread_signal(5,1);
    thread_unlock(5);
}
void signal_fifo_4(void* args) {
    thread_lock(5);
    printf(" sf4 ");
    thread_signal(5,1);
    thread_unlock(5);
}
void start(void* args) {
    thread_create(immediate, NULL);
    for (int i = 0; i < 6; i++) {
        printf(" start ");
    }
    thread_yield();
    thread_create(unlock_a, NULL);
    thread_create(unlock_b, NULL);
    thread_yield();
    thread_create(ready_fifo_1, NULL);
    thread_create(ready_fifo_2, NULL);
    thread_create(ready_fifo_3, NULL);
    thread_create(ready_fifo_4, NULL);
    thread_yield();
    thread_create(signal_fifo_1, NULL);
    thread_create(signal_fifo_2, NULL);
    thread_create(signal_fifo_3, NULL);
    thread_create(signal_fifo_4, NULL);
}


int main () {
    thread_libinit(start, NULL);
}