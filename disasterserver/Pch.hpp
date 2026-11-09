#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <variant>
#include <chrono>
#include <array>
#include <cstring>
#include <functional>
#include <optional>
#include <memory>
#include <thread>
#include <mutex>
#include <random>
#include <algorithm>
#include <unordered_set>
#include <regex>
#include <typeindex>
#include <unordered_set>
#include <unordered_map>
#include <concepts>
#include <ranges>
#include <utility>
#include <string_view>

using clientId = uint16_t;
using mapId = uint16_t;
using entityId = uint16_t;
using PlayerFlags = uint8_t;
using commandHash = unsigned long;

using Clock = std::chrono::steady_clock;
using Duration = std::chrono::duration<double, std::milli>;
using TimeStamp = std::chrono::time_point<Clock>;
