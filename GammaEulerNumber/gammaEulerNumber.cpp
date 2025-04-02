#include <iostream>
#include <mutex>
#include <thread>
#include <vector>
#include <random>
#include <chrono>
#include <cmath>

std::mutex numberMutex;
double globalNumber = 0.0;

void calculateSum(int start, int end) {
    double localNumber = 0.0;
    for (int i = start; i <= end; i++){
        localNumber += (1.0 / i);
    }

    numberMutex.lock();
    globalNumber += localNumber;
    numberMutex.unlock();
}

int main () {
    int n, p;
    std::vector<std::thread> processes;
    std::cout << "Podaj liczbe elementow sumy (n)" << std::endl;
    std::cin >> n;
    std::cout << "Podaj liczbe procesow (p)" << std::endl;
    std::cin >> p;
    int start = 1; 
    int end;
    int chunk_size = n / p;
    int mod = n % p;

    for (int i = 0; i < p; i++) {
        end = start + chunk_size - 1 + (mod > i ? 1 : 0);
        processes.push_back(std::thread(calculateSum, start, end));
        start = end + 1;
    }

    for (auto &c : processes) c.join();

    double gamma = globalNumber - log(n);
    std::cout << "Stala gamma Eulera (dla n=" << n << "): " << gamma << std::endl;

    return 0;
}