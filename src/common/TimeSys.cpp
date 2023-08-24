//
// Created by DELL on 2021/11/22.
//

#include "TimeSys.h"

#include <memory>

using namespace std::chrono;

TimeSys TimeSys::timeSys;

TimeSys::TimeSys()
{
    this->startTime = high_resolution_clock::now();
}

void TimeSys::update()
{
    auto current = high_resolution_clock::now();
    timeSys.delta = duration<double, std::chrono::seconds::period>(current - timeSys.now).count();
    timeSys.time =  duration<double, std::chrono::seconds::period>(current - timeSys.startTime).count();
    timeSys.now = current;
}
