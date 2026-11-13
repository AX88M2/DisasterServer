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
#include <fstream>

namespace DisasterServer {
    static std::string getCurrentTime(const std::string& format = "%d.%m.%Y %T") {
        std::stringstream ss;
        const time_t now = std::time(nullptr);

        std::tm local{};
#ifdef _WIN32
        localtime_s(&local, &now);
#else
        localtime_r(&now, &local);
#endif
        ss << std::put_time(&local, format.c_str());
        return ss.str();
    }
}

using clientId = uint16_t;
using mapId = uint16_t;
using entityId = uint16_t;
using PlayerFlags = uint8_t;
using commandHash = unsigned long;

using Clock = std::chrono::steady_clock;
using Duration = std::chrono::duration<double, std::milli>;
using TimeStamp = std::chrono::time_point<Clock>;

constexpr int TICKS_PER_SEC = 60;
constexpr int MAX_PLAYERS = 8;
constexpr int BUILD_VERSION = 1101;
static constexpr int PACKET_MAXSIZE = 256;

constexpr std::string_view CLRCODE_RED = "\\";
constexpr std::string_view CLRCODE_GRN = "@";
constexpr std::string_view CLRCODE_PUR = "&";
constexpr std::string_view CLRCODE_BLU = "/";
constexpr std::string_view CLRCODE_GRA = "|";
constexpr std::string_view CLRCODE_YLW = "`";
constexpr std::string_view CLRCODE_ORG = "\xE2\x84\x96";
constexpr std::string_view CLRCODE_RST = "~";

static std::array clr_list = { CLRCODE_RED, CLRCODE_GRN, CLRCODE_PUR, CLRCODE_BLU, CLRCODE_GRA, CLRCODE_YLW, CLRCODE_ORG, CLRCODE_RST };
constexpr int CLRLIST_LEN = 8;