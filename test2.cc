#include "interrupt.h"
#include "thread.h"
#include <cstdlib>
#include <cstdio> 

using namespace std;

//Tests if the user runs multiple lock statements with the same thread

int ret = 0;
int now = 1;

void test_invalid_lock_and_release(void* arg) {
    

    printf("%d", thread_lock(20));
    printf("%d", thread_lock(20));
    printf("%d", thread_unlock(20));
    printf("%d", thread_unlock(20));

}

void start (void* arg) {
    thread_create(test_invalid_lock_and_release, NULL);
}

int main() {
    thread_libinit(start, NULL);
}