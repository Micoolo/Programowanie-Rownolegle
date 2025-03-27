#include <iostream>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <vector>
#include <random>
#include <chrono>

//wymagania:    wiele czytelnikow moze odczytywac dane; pisarz wyklucza dostep do danych innych procesow; 
//              dane moga byc odczytane dopiero po pierwszym zapisaniu oraz pomiedzy kolejnymi zapisami musza wystapic minimum 3 odczyty 

//requirements: many readers may read the data at the same time; a writer excludes access to the data for other processes;
//              data can be read only after first write and between (consecutive) writes there must be minimum 3 readings 

int data = -1;          // data (number stored)
int readersCounter = 0;  // counter of current readers
int numberOfReadings = 0;   // number of readings required before next write

//random number generator
std::random_device r;
std::mt19937 gen(r());
std::uniform_int_distribution<int> dist(1, 100);

//mutexes
std::mutex reader_mutex;
std::mutex data_mutex;
std::mutex cout_mutex;
std::condition_variable writer_cv;

void reader(int id) {
    do {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000 + rand() % 2000));
        
        //locking access to counter
        reader_mutex.lock();
            readersCounter++;
            if (readersCounter == 1) {
                //locking access to data 
                data_mutex.lock();
            }
        reader_mutex.unlock();

        cout_mutex.lock();
            std::cout << "Reader " << id << " read: " << data << std::endl;
            numberOfReadings++;
        cout_mutex.unlock();

        reader_mutex.lock();
            readersCounter--;
            if (readersCounter == 0) {
                data_mutex.unlock();
                writer_cv.notify_all();
            }
        reader_mutex.unlock();

    } while(true);
}

void writer(int id) {
    do {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000 + rand() % 2000));
        

        std::unique_lock<std::mutex> uniqueLock(data_mutex);
        writer_cv.wait(uniqueLock, [] {return (numberOfReadings >= 3) ? true : false; });

        data = dist(gen);
        std::cout << "WRITER " << id << " WROTE: " << data << std::endl;
        numberOfReadings = 0;

        writer_cv.notify_all();
    } while(true);
}


int main() {
    std::vector<std::thread> readers;
    std::vector<std::thread> writers;

    // 5 readers
    for (int i = 0; i < 5; i++) {
        readers.push_back(std::thread(reader, i));
    }

    // 2 writers
    for (int i = 0; i < 2; i++) {
        writers.push_back(std::thread(writer, i));
    }

    for (auto &c : readers) c.join();
    for (auto &p : writers) p.join();

    return 0;
}

//falszywe wybudzenie