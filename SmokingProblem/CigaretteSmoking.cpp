#include <iostream>
#include <mutex>
#include <thread>
#include <vector>
#include <random>
#include <chrono>

//mutexes
std::mutex tampers[2]; 	//ubijacze
std::mutex matches[2];  //zapalki
std::mutex cout_mutex;  //mutex do coutow

void tamper (int id) {
    bool done = false;
    do {
        for (int i = 0; i < std::size(tampers); i++) {
            if(tampers[i].try_lock()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1000 + rand() % 2000));
                cout_mutex.lock();
                std::cout << "Palacz " << id + 1 << " uzyl ubijacza nr " << i + 1 << std::endl;
                cout_mutex.unlock();
                tampers[i].unlock();
                done = true;
                break;
            }
        }
        if (!done) {
            std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 100)); 
        }
    } while (!done);
}

void lighting (int id) {
    bool done = false;
    do {
        for (int i = 0; i < std::size(matches); i++) {
            if(matches[i].try_lock()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1000 + rand() % 2000));
                cout_mutex.lock();
                std::cout << "Palacz " << id + 1 << " uzyl pudelka zapalek nr " << i + 1 << std::endl;
                cout_mutex.unlock();
                matches[i].unlock();
                done = true;
                break;
            }
        }        
        if (!done) {
            std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 100)); 
        }
    } while (!done);
}

void smoking (int id) {
    cout_mutex.lock();
    std::cout << "Palacz " << id + 1 << " pali" << std::endl;
    cout_mutex.unlock();
    std::this_thread::sleep_for(std::chrono::milliseconds(1000 + rand() % 2000));
}

void smoker (int id) {
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 100));
        tamper(id);
        lighting(id);
        smoking(id);
    }
}

int main() {
    std::vector<std::thread> smokers;

    for (int i = 0; i < 3; i++) {
        smokers.push_back(std::thread(smoker, i));
    }

    for (auto &c : smokers) c.join();

    return 0;
}