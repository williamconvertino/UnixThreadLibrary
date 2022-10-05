#include "interrupt.h"
#include "thread.h"
#include <cstdlib>
#include <cstdio> 

using namespace std;

//Tests the errors that occur by calling thread functions before libinit. 



void test_thread(void* args) {
    printf("%d", thread_lock(1));

    printf("%d", thread_unlock(1));
}

int main() {
    thread_libinit(test_thread, NULL);
}