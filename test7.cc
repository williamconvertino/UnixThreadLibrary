#include "interrupt.h"
#include "thread.h"
#include <cstdlib>
#include <cstdio> 

using namespace std;

//Tests if correctly used functions will cause an error.


void a(void* args) {
    printf("%d",thread_lock(1));
    printf("%d",thread_wait(1,1));
    printf("\nThread a executed.\n");
    printf("%d",thread_unlock(1));
}



void b(void* args) {
    printf("%d",thread_lock(1));
    printf("%d",thread_signal(1,1));
    printf("%d",thread_wait(1,2));
    printf("\nThread b executed.\n");
    printf("%d",thread_unlock(1));
}


void c(void* args) {
    printf("%d",thread_yield());
    printf("%d",thread_lock(1));
    printf("%d",thread_broadcast(1,2));
    printf("\nThread c executed.\n");
    printf("%d",thread_unlock(1));
}


void start(void* args) {
    thread_create(a, NULL);
    thread_create(b, NULL);
    thread_create(c, NULL);
    printf("%d", thread_broadcast(1,3));
    printf("%d", thread_signal(1,3));
}

int main() {
    printf("%d", thread_libinit(start, NULL));
}
