#ifndef JOINERTHREADS_H
#define JOINERTHREADS_H

#include <vector>
#include <thread>

struct JoinThreads
{
    JoinThreads(std::vector<std::thread>& tids_):tids(tids_)
    {}

    ~JoinThreads() {
        for(auto&& tid : tids) {
            tid.join();
        }
    }

    std::vector<std::thread>& tids;
};

#endif
