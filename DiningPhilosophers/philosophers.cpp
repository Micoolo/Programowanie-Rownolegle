#include <iostream>
#include <mutex>
#include <thread>
#include <ctime>
#include <chrono>
#include <cstdlib>
#include <ncurses.h>
#include <random>

int nrOfPhilosophers;
bool quit = false;
std::thread philosophers[10];
std::mutex widelec[10]; 	//implementacja sztućców jako mutexy
std::mutex mutex; //mutex generalnego przeznaczenia


// generator liczb losowych (dla watków)
int intRand(const int & min, const int & max) {
    std::random_device r;
    static thread_local std::mt19937 generator(r());
    std::uniform_int_distribution<int> distribution(min, max);
    return distribution(generator);
}

void menuInit()
{
    for (int i = 0; i < nrOfPhilosophers; i++)
    {
        mutex.lock();
        move(i, 0);
        printw("Filozof[%d] czeka", i+1);
        refresh();
        move(i, 40);
        clrtoeol();
        printw("Widelec[%d] jest:\twolny", i+1);
        refresh();
        mutex.unlock();
    }
}

void think(int id)
{
    int randThink = intRand(20, 30);
    float progress = 0.0;

    for (int i = 1; i <= randThink; i++)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        progress = (100 * i) / randThink;
        mutex.lock();
        move(id, 0);
        printw("Filozof[%d] mysli:\t- %.0f\t%%", id+1, progress);
        refresh();
        mutex.unlock();

    }

}

//Filozof[ID] próbuje jeść
bool eat(int id, int left, int right)
{
    //Filozof[ID] próbuje podnieść lewy widelec, aż do skutku
    while (true) if (widelec[left].try_lock())
        {
            mutex.lock();
            move(left, 40);
            clrtoeol();
            printw("Widelec[%d] jest:\tzajety przez filozofa[%d]", left+1, id+1);
            refresh();
            mutex.unlock();
            //Filozof[ID] próbuje podnieść prawy widelec
            if (widelec[right].try_lock())
            {
                mutex.lock();
                move(right, 40);
                clrtoeol();
                printw("Widelec[%d] jest:\tzajety przez filozofa[%d]", right+1, id+1);
                refresh();
                mutex.unlock();

                int randEat = intRand(20, 30);
                float progress = 0.0;

                for (int i = 1; i <= randEat; i++)
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    progress = (100 * i) / randEat;
                    mutex.lock();
                    move(id, 0);
                    printw("Filozof[%d] obiaduje:\t- %.0f\t%%", id+1, progress);
                    refresh();
                    mutex.unlock();

                }

                return true;
            }
            else //Aby zapobiec zagłodzeniu i zakleszczeniu, odkładamy lewy wielec jeśli prawy jest zajęty
            {
                widelec[left].unlock();
                mutex.lock();
                move(left, 40);
                clrtoeol();
                printw("Widelec[%d] jest:\twolny", left+1);
                refresh();
                mutex.unlock();
                std::this_thread::sleep_for(std::chrono::milliseconds(300));
            }
        }
}

void putDownForks(int left, int right)
{

    widelec[right].unlock();
    mutex.lock();
    move(right, 40);
    clrtoeol();
    printw("Widelec[%d] jest:\twolny", right+1);
    refresh();
    mutex.unlock();

    widelec[left].unlock();
    mutex.lock();
    move(left, 40);
    clrtoeol();
    printw("Widelec[%d] jest:\twolny", left+1);
    refresh();
    mutex.unlock();

}

void startDinner(int philosopherID)
{
    // Hierarchia zasobów - filozof najpierw bierze widelec z nizszym numerem (lewy) (dla filozofa ostatniego nizszy numer to prawy widelec )
    int leftFork = std::min(philosopherID, (philosopherID + 1) % (nrOfPhilosophers));
    int rightFork = std::max(philosopherID, (philosopherID + 1) % (nrOfPhilosophers));

    while (!quit)
    {
        //Filozof[ID] próbuje jeść
        if (eat(philosopherID, leftFork, rightFork))
        {
            putDownForks(leftFork, rightFork);
            if (quit == false)
                think(philosopherID);
        }
    }
}

int main(int argc, char* argv[]) {

    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <number_of_philosophers> \n";
        return 1;
    }

    nrOfPhilosophers = std::stoi(argv[1]);

    if (nrOfPhilosophers < 5 || nrOfPhilosophers > 10) {
        std::cerr << "Number of philosophers must be between 5 and 10\n";
        return 1;
    }

    initscr();
    cbreak();
    start_color();
    init_pair(1, COLOR_YELLOW, COLOR_BLACK);
    attron(COLOR_PAIR(1));
    menuInit();

    //tworzenie wątków
    for (int i = 0; i < nrOfPhilosophers; ++i)
       philosophers[i] = std::thread(startDinner, i);

    char tmp = 'p';

    //czekanie na sygnal zakonczenia pracy
    while(tmp != 'e')
    {
        tmp = static_cast<char>(std::cin.get());
        if (tmp == 'e')
        {
            quit = true;
        }
    }

    //łączenie wątków
    for (auto &thread : philosophers) {
        if (thread.joinable()) {
            thread.join();
        }
    }



    endwin();
    exit(0);
}
