#ifndef SPINNER_H
#define SPINNER_H

#include <iostream>
#include <vector>
#include <thread>

#include <functional>

class Spinner
{
public:
    static void show(std::function<void()> f)
    {
        bool stop{false};
        std::thread spinner_thread([&]() {
                while (!stop) {
                    Spinner::show();
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
            });
        f();
        stop = true;
        spinner_thread.join();
        std::cout << "\r";
        std::cout << std::flush;
    }
    static void show()
    {
        static size_t i{0};
        std::vector<char> chars{'/', '-', '\\', '|'};
        std::cout << "\r" << chars.at(i);
        std::cout << std::flush;
        i = (i + 1) % chars.size();
    }

};

#endif // SPINNER_H
