#include "BoundedBuffer.h"
#include <assert.h>
#include <string.h>

using namespace std;


BoundedBuffer::BoundedBuffer (int _cap) : cap(_cap) {
    // modify as needed
}

BoundedBuffer::~BoundedBuffer () {
    // modify as needed
}

void BoundedBuffer::push (char* msg, int size) {
    // 1. Convert the incoming byte sequence given by msg and size into a vector<char>
    //    use one of the vector constructors
    vector<char> data (msg, msg+size);
    // 2. Wait until there is room in the queue (i.e., queue lengh is less than cap)
    //     waiting on slot available
    unique_lock<mutex> lock(m);

    slot_available.wait(lock, [this]{return static_cast<int>(q.size())<cap;});
    // 3. Then push the vector at the end of the queue
    q.push(data);
    lock.unlock();
    // 4. Wake up threads that were waiting for push
    //     notifying data available
    data_available.notify_one();
}

int BoundedBuffer::pop (char* msg, int size) {
    // 1. Wait until the queue has at least 1 item
    //    waiitng on data available
    unique_lock<mutex> lock(m);
    data_available.wait(lock, [this]{return q.size()>0;});
    // 2. Pop the front item of the queue. The popped item is a vector<char>
    vector<char> data= q.front();
    q.pop();
    lock.unlock();
    // 3. Convert the popped vector<char> into a char*, copy that into msg; assert that the vector<char>'s length is <= size
    //      use vector data
    assert(static_cast<int>(data.size()) <= size);
    memcpy(msg, data.data(), data.size());
    
    // 4. Wake up threads that were waiting for pop
    //    notifying slot available
    slot_available.notify_one();
    // 5. Return the vector's length to the caller so that they know how many bytes were popped
    return data.size();
}

size_t BoundedBuffer::size () {
    return q.size();
}