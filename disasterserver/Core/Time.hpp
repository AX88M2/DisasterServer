#pragma once
#include <chrono>

using Clock = std::chrono::steady_clock;
using Duration = std::chrono::duration<double, std::milli>;
using TimeStamp = std::chrono::time_point<Clock>;
