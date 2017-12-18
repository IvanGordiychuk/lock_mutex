#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <stdexcept>

#define RING_BUF_SIZE 50

using namespace std;

class Ring {
public:
    int buffer[RING_BUF_SIZE]; // как то так буфер интов задается по-моему
    unsigned int readIndex;
    unsigned int length;

    mutex lock;

    Ring() {
        length = 0;
        readIndex = 0;
    }
};

class Producer {
    Ring * ring;
public:
    Producer(Ring * ringBuffer):ring(ringBuffer) { }

    bool canWrite();
    void write(int);
};

class Consumer {
    Ring * ring;
public:
    Consumer(Ring * ringBuffer):ring(ringBuffer) { }

    bool canRead();
    int read();
};


void Producer::write(int elem){
    ring->lock.lock();
    ring->buffer[(ring->readIndex + ring->length) % RING_BUF_SIZE] = elem;
    if (ring->length < RING_BUF_SIZE) {
        ring->length++;
    } else {
        ring->readIndex = (ring->readIndex + 1) % RING_BUF_SIZE;
    }
    ring->lock.unlock();
}

bool Producer::canWrite() {
    return true;
}

bool Consumer::canRead() {
    return ring->length > 0;
}

int Consumer::read() {
    ring->lock.lock();
    if (ring->length == 0) {
        ring->lock.unlock();
        throw std::runtime_error("No data to read from ring buffer");
    }
    ring->length--;
    int result = ring->buffer[ring->readIndex];
    ring->readIndex = (ring->readIndex + 1) % RING_BUF_SIZE;
    ring->lock.unlock();
    return result;
}



void runProducer(Ring * ringBuffer) {
    Producer prod(ringBuffer);

    cout << prod.canWrite() << endl;
    for (int i = 0; i <= 100; ++i) {
            // задержка для реалистичности, типа какое то время тратится на создание объекта
    	//this_thread::sleep_for(chrono::microseconds(2));

    	prod.write(i);
        }



}

void runConsumer(Ring * ringBuffer) {
    int value;
    Consumer cons(ringBuffer);

    while (true) {
        if (cons.canRead()) {
        	//считываем с буффера с помощью метода cons класса Consumer
            value = cons.read();
            // задержка для реалистичности, типа какое то время тратится на обработку полученного объекта
            this_thread::sleep_for(chrono::microseconds(2));
            cout << value << endl;
            if (value == 100) {
                cout << "got last, done" << endl;
                break;
            }
        } else {
            cout << "nothing to read, will sleep a bit" << endl;
            this_thread::sleep_for(chrono::microseconds(5));
        }
    }
}

int main(){
    Ring ringBuffer;

    //cout << boolalpha; // печатать булевские значения как "true" и "false"

    std::cout << "start" << endl;
    thread producer(runProducer, &ringBuffer);

    thread consumer(runConsumer, &ringBuffer);


    consumer.join();
    producer.join();
    std::cout << "done" << endl;

    return 0;
}

