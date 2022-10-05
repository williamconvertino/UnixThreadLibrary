#include "thread.h"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <stdlib.h>
using namespace std;

// Enumerate function signatures
int main(int argc, char *argv[]);
void start(void *args);

typedef struct cashier_list_data {

    char* cashiers[255];
    int size;

} cashier_list;

typedef struct cashier_data {

    int id;
    char* filename;

} cashier;

typedef struct order order;

struct order {

    int sandwitchNumber;
    int cashierId;
    order* next;
    order* prev;

};

typedef struct board_data {

    order* firstOrder;
    int numOrders;
    int boardSize;
    int numActiveCashiers;

}board;

board* myBoard;


void addOrder(order* orderToAdd, order* firstOrder) {
    if (firstOrder == NULL) {
        myBoard->firstOrder = orderToAdd;
        myBoard->numOrders++;
    } else if (firstOrder->next == NULL) {
        firstOrder->next = orderToAdd;
        orderToAdd->prev = firstOrder;
        myBoard->numOrders++;
    } else {
        addOrder(orderToAdd, firstOrder->next);
    }
}

order* popOrder(int sandwitch_number, order* current_order) {

    if (current_order == NULL) {
        return(NULL);
    }
    
    if (current_order->sandwitchNumber == sandwitch_number) {
        if (current_order->next != NULL) {
            current_order->next->prev = current_order->prev;
        }
        if (current_order==myBoard->firstOrder) {
            myBoard->firstOrder = current_order->next;
        } else {
            current_order->prev->next = current_order->next;
        }
        myBoard->numOrders--;
        return(current_order);
    }

    return(popOrder(sandwitch_number, current_order->next));

}

void printOrders(order* current) {
    if (current == NULL) {
        return;
    }
    printf("\nPO:%d\n", current->sandwitchNumber);
    printOrders(current->next);
}

int main(int argc, char *argv[]) {

    if (argc < 2) {
        cout << "Not enough inputs" << endl;
        return (0);
    }
    
    myBoard = (board*)malloc(sizeof(board));    
    myBoard->boardSize = strtol(argv[1], NULL, 10);
    myBoard->numActiveCashiers = 0;
    myBoard->numOrders = 0;
    myBoard->firstOrder = NULL;

    cashier_list* myCashierList = (cashier_list*)malloc(sizeof(cashier_list));
    myCashierList->size = argc-2;

    for (int i = 0; i < myCashierList->size; i++) {

        myCashierList->cashiers[i] = argv[i+2];        
        myBoard->numActiveCashiers++;
    }
    
    thread_libinit(start, (void*)myCashierList);
}

int findClosestOrderNumber (order* currentOrder, int currentMax, int previousSandwitch) {

    if (currentOrder == NULL) {
        return(currentMax);
    }
    if (abs(currentOrder->sandwitchNumber - previousSandwitch) < abs(currentMax - previousSandwitch)) {
        return(findClosestOrderNumber(currentOrder->next, currentOrder->sandwitchNumber, previousSandwitch));
    }
    return(findClosestOrderNumber(currentOrder->next, currentMax, previousSandwitch));

}
/*
void make_orders (void* args) {


    int previousSandwitch = -1;

    
    while (myBoard->numActiveCashiers > 0) {
        
        thread_lock(1);
        while (myBoard->numOrders < min(myBoard->boardSize, myBoard->numActiveCashiers)) {
            thread_wait(1,4);
        }
       
        int nextSandwitch = findClosestOrderNumber(myBoard->firstOrder, myBoard->firstOrder->sandwitchNumber, previousSandwitch);

        order* myOrder = popOrder(nextSandwitch, myBoard->firstOrder);

        thread_lock(16);
        myBoard->numOrders--;
        thread_unlock(16);

        cout << "READY: cashier " << myOrder->cashierId << " sandwich " << myOrder->sandwitchNumber << endl;
        previousSandwitch=nextSandwitch;

        thread_broadcast(99, myOrder->cashierId);
        thread_broadcast(1,2);
        thread_unlock(1);
    }
    
}
*/

/*
void read_orders (void* args) {

    cashier* myCashier = (cashier*)args;

    std::ifstream istrm(myCashier->filename);
    int sandwitch_number;

    while (istrm >> sandwitch_number) {
        
        thread_lock(1);
        while (myBoard->boardSize <= myBoard->numOrders) {
            thread_wait(1, 2);  //CV 2 -> cashier waiting for empty space
        }
        
        order* currentOrder = (order*)malloc(sizeof(order));
        currentOrder->cashierId = myCashier->id;
        currentOrder->sandwitchNumber = sandwitch_number;
        currentOrder->next = NULL;
        currentOrder->prev = NULL;
        addOrder(currentOrder, myBoard->firstOrder);
        cout << "POSTED: cashier " << currentOrder->cashierId << " sandwich " << currentOrder->sandwitchNumber << endl;
        thread_lock(16); //lock 16 = numOrders
        myBoard->numOrders++;
        thread_unlock(16);

        thread_broadcast(1, 4); //CV 4  -> added new order
        thread_unlock(1);
        
        thread_lock(16);
        thread_wait(16,myCashier->id);
        myBoard->numOrders--;
        thread_unlock(16);
    }

}*/


void make_orders (void* args) {

    int previousSandwitch = -1;
    thread_lock(1);
    while (myBoard->numActiveCashiers > 0) {

        while (myBoard->numOrders < min(myBoard->boardSize, myBoard->numActiveCashiers)) {
            thread_wait(1, 50); //CV 50 -> order has been added to queue
        }

        int nextSandwitch = findClosestOrderNumber(myBoard->firstOrder, myBoard->firstOrder->sandwitchNumber, previousSandwitch);
        order* myOrder = popOrder(nextSandwitch, myBoard->firstOrder);
        cout << "READY: cashier " << myOrder->cashierId << " sandwich " << myOrder->sandwitchNumber << endl;
        previousSandwitch=nextSandwitch;

        thread_broadcast(1,20);
        thread_broadcast(1, 9999 + myOrder->cashierId);
        thread_wait(1,99);
    }
    thread_unlock(1);

}

void read_orders (void* args) {

    cashier* myCashier = (cashier*)args;

    std::ifstream istrm(myCashier->filename);
    int sandwitch_number;

    thread_lock(1);
    while (istrm >> sandwitch_number) {

        while (myBoard->boardSize== myBoard->numOrders) {
            thread_wait(1, 20);  //CV 20 -> order has been made, spot open
        }

        order* currentOrder = (order*)malloc(sizeof(order));
        currentOrder->cashierId = myCashier->id;
        currentOrder->sandwitchNumber = sandwitch_number;
        currentOrder->next = NULL;
        currentOrder->prev = NULL;
        addOrder(currentOrder, myBoard->firstOrder);
        cout << "POSTED: cashier " << currentOrder->cashierId << " sandwich " << currentOrder->sandwitchNumber << endl;

        thread_broadcast(1,50); //CV 50 -> order has been added to queue
        thread_wait(1, 9999 + myCashier->id);
        thread_broadcast(1,99);

    }
    myBoard->numActiveCashiers--;
    thread_unlock(1);

}


void start_thread(void *args) {

   cashier_list* myCashierList = (cashier_list*)args;

    for (int i = 0; i < myCashierList->size; i++) {
        //printf("making boy %d\n",i );
        cashier* c = (cashier*)malloc(sizeof(cashier));
        c->id = i;
        c->filename =  myCashierList->cashiers[i];
        thread_create(read_orders,c);
    }

    thread_create(make_orders, NULL);

}

void start(void* deli) {
    thread_create(start_thread,deli);
}
 