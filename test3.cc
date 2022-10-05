#include "interrupt.h"
#include "thread.h"
#include <cstdlib>
#include <cstdio> 

using namespace std;

//Tets if libinit throws an error if it is called more than once. - DID NOT WORK??
int works = 1;
void test_thread_2(void* args) {
    printf("Something went wrong...");
}

void test_thread(void* args) {
   
    printf("%d", thread_libinit(test_thread_2, NULL));

}

int main() {
    
    thread_libinit(test_thread, NULL);

}