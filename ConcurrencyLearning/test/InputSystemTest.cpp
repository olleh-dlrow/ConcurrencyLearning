#include "InputSystem.h"
#include <iostream>
#include <Windows.h>
#include <thread>
using namespace std::chrono_literals;

namespace sc = std::chrono;

void input_test() {
    InputSystem input;

    bool run = true;
    sc::milliseconds frame_interval = 500ms;//20fps
    int frame = 0;
    sc::time_point last_time = sc::system_clock::now();
    while(run) {
        input.update();
        for(char i = 'a'; i <= 'z'; i++) {
            if(input.get_key_down(i)) {
                std::cout << "key down: " << i << std::endl;
                if(i == 'q') {
                    run = false;
                }
            }
        }
        sc::time_point current_time = sc::system_clock::now();
        sc::milliseconds delta_time = sc::duration_cast<std::chrono::milliseconds>
            (current_time - last_time);
        sc::milliseconds wait_time = frame_interval - delta_time;
        std::this_thread::sleep_for(wait_time);

        current_time = sc::system_clock::now();
        delta_time = sc::duration_cast<std::chrono::milliseconds>
            (current_time - last_time);
        std::cout << "frame: " << frame << " " << delta_time.count() << " ms" << std::endl;
        frame++;
        last_time = current_time;
    }
}

int main() {
    input_test();
    return 0;
}
