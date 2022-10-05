#include "interrupt.h"
#include "thread.h"
#include <cstdlib>
#include <cstdio> 

using namespace std;

void yield_test_1(void* args) {
    for(int i = 0; i < 6; i++) {
        printf("1");
    }
}
void yield_test_2(void* args) {
    printf("%d", thread_yield());
    for(int i = 0; i < 6; i++) {
        printf("2");
    }
}

void keep_lock(void* args) {
    printf("%d",thread_lock(1));
}

void start(void* args) {
    thread_create(yield_test_1, NULL);
    thread_create(yield_test_2, NULL);
    thread_create(keep_lock, NULL);

}

int main() {
    printf("%d",thread_libinit(start, NULL));
}