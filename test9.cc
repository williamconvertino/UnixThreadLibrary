#include "interrupt.h"
#include "thread.h"
#include <cstdlib>
#include <cstdio> 

using namespace std;

void deadlock_a (void* args) {
    printf("%d",thread_lock(1));
    thread_lock(3);
    thread_wait(3,1);
    printf("%d",thread_lock(2));
    printf("Executed a");
}

void deadlock_b(void* args) {
    printf("%d", thread_lock(2));
    thread_broadcast(3,1);
    printf("%d",thread_lock(1));
    printf("Executed b");
}


void start(void* args) {
    thread_create(deadlock_a, NULL);
    thread_create(deadlock_b,NULL);
}


int main () {
    thread_libinit(start, NULL);
}