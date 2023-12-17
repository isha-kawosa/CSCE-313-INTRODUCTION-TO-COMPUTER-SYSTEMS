#include <iostream>
#include <mutex>
#include <thread>
#include <condition_variable>
// include additional necessary headers


using namespace std;

std::mutex m;
condition_variable c;
bool sent= false;

void query(int count) {
    // Should print: the print number (starting from 0), "SYN", and the three dots "..."
    for(int i=0; i<count ; i++){
        std::unique_lock<std::mutex> lock(m);
        cout << "[" << i <<"] SYN ...";
        sent = true;
        c.notify_one();
        c.wait(lock, []{return !sent;});
    }
}

void response(int count) {
    // Should print "ACK"
    for(int i=0; i<count ; i++){
        std::unique_lock<std::mutex> lock(m);
        c.wait(lock, []{return sent;});
        cout << "ACK" << endl;
        sent = false;
        c.notify_one();
    }
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: ./handshake <count>\n");
        exit(1);
    }

    /**
     * Steps to follow:
     * 1. Get the "count" from cmd args
     * 2. Create necessary synchronization primitive(s)
     * 3. Create two threads, one for "SYN" and the other for "ACK"
     * 4. Provide the threads with necessary args
     * 5. Update the "query" and "response" functions to synchronize the output
    */
   
    int count = atoi(argv[1]);

    std::thread t1(query, count);
    std::thread t2(response, count);

    t1.join();
    t2.join();

    return 0;
}