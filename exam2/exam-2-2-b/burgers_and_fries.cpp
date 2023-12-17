#include <iostream>
#include <thread>
#include <semaphore.h>
#include <mutex>
#include <unistd.h>
using namespace std;
#define MAX_THREADS 100

#define BURGER 0
#define FRIES 1
const char* type_names[] = {"BURGER", "FRIES"};
#define pii pair<int, int>

int k;
mutex m;
sem_t semaphores[2]; 
int active_orders = 0;
int waiting_orders[2] = {0, 0};
int last_order_type = -1; 

// Do not change
void process_order() {
    sleep(2);
}

void place_order(int type) {
    /**
     *  Add logic for synchronization before order processing
     *  Check if already k orders in process; 
     *     if true -> wait (print waiting)
     *     otherwise place this order (print order)
     *  Use type_names[type] to print the order type
     */
    unique_lock<mutex> lock(m);
    if (active_orders == k || (last_order_type != -1 && last_order_type != type && waiting_orders[last_order_type] > 0)) {
        waiting_orders[type]++;
        cout << "Waiting: " << type_names[type] << endl;
        lock.unlock(); 
        sem_wait(&semaphores[type]);
        lock.lock(); 
        waiting_orders[type]--;
    }

    active_orders++;
    cout << "Order: " << type_names[type] << endl;
    lock.unlock();
    process_order();        // Do not remove, simulates preparation

    /**
     *  Add logic for synchronization after order processed
     *  Allow next order of the same type to proceed if there is any waiting; if not, allow the other type to proceed.
     */
    lock.lock();
    active_orders--;
    last_order_type = type;

    if (waiting_orders[type] > 0) {
        sem_post(&semaphores[type]);
    } else if (waiting_orders[1 - type] > 0) {
        sem_post(&semaphores[1 - type]);
    }

    lock.unlock();
}

int main() {
    // Initialize necessary variables, semaphores etc.
    sem_init(&semaphores[BURGER], 0, 0);
    sem_init(&semaphores[FRIES], 0, 0);
    // Read data: done for you, do not change
    pii incoming[MAX_THREADS];
    int _type, _arrival;
    int t;
    cin >> k;
    cin >> t;
    for (int i = 0; i < t; ++i) {
        cin >> _type >> _arrival;
        incoming[i].first = _type;
        incoming[i].second = _arrival;
    }

    // Create threads: done for you, do not change
    thread* threads[MAX_THREADS];
    for (int i = 0; i < t; ++i) {
        _type = incoming[i].first;
        threads[i] = new thread(place_order, _type);
        if (i < t - 1) {
            int _sleep = incoming[i + 1].second - incoming[i].second;
            sleep(_sleep);
        }
    }

    // Join threads: done for you, do not change
    for (int i = 0; i < t; ++i) {
        threads[i]->join();
        delete threads[i];
    }

    sem_destroy(&semaphores[BURGER]);
    sem_destroy(&semaphores[FRIES]);
    
    return 0;
}