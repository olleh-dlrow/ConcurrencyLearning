#ifndef INNPUTSYSTEM_H
#define INNPUTSYSTEM_H

#include "FineGrainedLockQueue.h"
#include <atomic>
#include <thread>
#include <Windows.h>
#include <conio.h>

class InputSystem {
public:
    InputSystem():disable(false), done(false) {
        receive_thread = std::thread(&InputSystem::receive, this);
    }

    ~InputSystem() {
        done.store(true);
        receive_thread.join();
    }

    void update() {
        memset(key_states, 0, sizeof(key_states));
        char ch;
        while(queue.try_pop(ch)) {
            key_states[ch] = true;
        }
    }

    void set_mask(bool mask) {
        disable.store(mask);
    }

    bool is_mask() {
        return disable.load();
    }

    bool get_key_down(char ch) {
        return key_states[ch];
    }

private:
    void receive() {
        while(!done.load()) {
            if(!disable.load()) {
                if(_kbhit()) {
                    queue.push(_getch_nolock());
                }
            }
        }
    }

    std::atomic<bool> disable;
    std::atomic<bool> done;
    std::thread receive_thread;
    FineGrainedLockQueue<char> queue;

    bool key_states[256];
};

#endif
