//
// Created by DELL on 2021/11/22.
//

#pragma once

#include <chrono>

using system_time_point = std::chrono::time_point<std::chrono::high_resolution_clock>;

class TimeSys
{
public:
    TimeSys(const TimeSys&) = delete;
    TimeSys& operator=(const TimeSys& ) = delete;

    static void update();

    [[nodiscard]] static const system_time_point& getStartTime()
    {
        return timeSys.startTime;
    }

    [[nodiscard]] static const system_time_point& getNow()
    {
        return timeSys.now;
    }

    // 返回程序启动后的运行时间，以微为单位
    [[nodiscard]]static int64_t getNowCountUs()
    {
        return std::chrono::time_point_cast<std::chrono::milliseconds>(timeSys.now).time_since_epoch().count();
    }

    [[nodiscard]] static double getDelta()
    {
        return timeSys.delta;
    }

    [[nodiscard]] static double getTime()
    {
        return timeSys.time;
    }

    static system_time_point startTiming()
    {
        timeSys.startTimingPoint = std::chrono::high_resolution_clock::now();
        return timeSys.startTimingPoint;
    }

    static double timing()
    {
        using namespace std::chrono;
        return duration<double, seconds::period>(high_resolution_clock::now() - timeSys.startTimingPoint).count();
    }

private:
    TimeSys();
    ~TimeSys() = default;

    static TimeSys timeSys;

    system_time_point startTime;
    system_time_point now;
    double delta{};
    double time{};

    system_time_point startTimingPoint;
};
