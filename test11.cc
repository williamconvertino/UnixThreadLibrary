#include "interrupt.h"
#include "thread.h"
#include <cstdlib>
#include <cstdio> 

using namespace std;


void waiting(void* arg) {
    thread_lock(1);
    printf(" waiting ");
    thread_unlock(1);
}
void wants1(void* arg) {
    thread_lock(1);
    printf(" w1 ");
    thread_unlock(1);
}
void wants2(void* arg) {
    thread_lock(1);
    printf(" w2 ");
    thread_unlock(1);
}

void loaded(void* arg) {
    thread_lock(1);
    thread_yield();
    thread_create(wants1, NULL);
    thread_unlock(1);
    thread_create(wants2, NULL);
}



void start(void* arg) {
    thread_create(loaded, NULL);
    thread_create(waiting, NULL);
}


int main () {
    thread_libinit(start, NULL);
}