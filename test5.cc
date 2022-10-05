#include "interrupt.h"
#include "thread.h"
#include <cstdlib>
#include <cstdio> 

using namespace std;

//Tests if broadcast works.

int broadcast_sum  = 0;

void broadcast_thread_1(void* args) {
    thread_lock(1);
    thread_wait(1,1);
    broadcast_sum++;
    printf("A thread has awoken.\n");
    thread_unlock(1);
}

void broadcast_thread_2(void* args) {
    thread_lock(1);
    thread_wait(1,1);
    broadcast_sum++;
    printf("SUCCESS: All threads awoken.\n");
    printf("%d", broadcast_sum);
    thread_unlock(1);
}
void broadcast_thread_3(void* args) {

    thread_broadcast(1,1);

}


void start(void* args) {
    thread_create(broadcast_thread_1, NULL);
    thread_create(broadcast_thread_1, NULL);
    thread_create(broadcast_thread_1, NULL);
    thread_create(broadcast_thread_2, NULL);
    
    thread_create(broadcast_thread_3, NULL);
}


int main() {
    thread_libinit(start, NULL);
}