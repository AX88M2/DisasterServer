#pragma once

#include <cstdint>
#include <string_view>
#include <vector>

#include "Types.hpp"

constexpr int TICKS_PER_SEC = 60;
constexpr int MAX_PLAYERS = 7;
constexpr int BUILD_VERSION = 1101;

constexpr std::string_view CLRCODE_RED = "\\";
constexpr std::string_view CLRCODE_GRN = "@";
constexpr std::string_view CLRCODE_PUR = "&";
constexpr std::string_view CLRCODE_BLU = "/";
constexpr std::string_view CLRCODE_GRA = "|";
constexpr std::string_view CLRCODE_YLW = "`";
constexpr std::string_view CLRCODE_ORG = "\xE2\x84\x96";
constexpr std::string_view CLRCODE_RST = "~";

enum CooldownId : uint8_t {
    TAILS_RECHARGE = 0,
    ETAILS_RECHARGE,
    EGGTRACK_RECHARGE,
    CREAM_RING_SPAWN,
    EXETIOR_BRING_SPAWN,
    PLAYER_COOLCOUNT
};

// Классный конечно кастыль (спасибо UwU)
static std::vector<mapId> convertMapIds = {
    0,   // 1. Hide And Seek Act 2
    2,   // 2. ... (DotDotDot)
    3,   // 3. Desert Town
    4,   // 4. You Can't Run (5)
    5,   // 5. Limp City (6)
    7,   // 8. Kind And Fair (8)
    13,  // 5. Majin Forest
    14,  // 6. Hide And Seek
    18,  // 7. Mystic Wood (19)
};

#define CLRLIST  { CLRCODE_RED, CLRCODE_GRN, CLRCODE_PUR, CLRCODE_BLU, CLRCODE_GRA, CLRCODE_YLW, CLRCODE_ORG, CLRCODE_RST }
constexpr int CLRLIST_LEN = 8;

