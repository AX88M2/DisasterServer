#ifndef DISASTERSERVER_COLORS_HPP
#define DISASTERSERVER_COLORS_HPP

#include <string_view>

constexpr std::string_view CLRCODE_RED = "\\";
constexpr std::string_view CLRCODE_GRN = "@";
constexpr std::string_view CLRCODE_PUR = "&";
constexpr std::string_view CLRCODE_BLU = "/";
constexpr std::string_view CLRCODE_GRA = "|";
constexpr std::string_view CLRCODE_YLW = "`";
constexpr std::string_view CLRCODE_ORG = "\xE2\x84\x96";
constexpr std::string_view CLRCODE_RST = "~";

#define CLRLIST  { CLRCODE_RED, CLRCODE_GRN, CLRCODE_PUR, CLRCODE_BLU, CLRCODE_GRA, CLRCODE_YLW, CLRCODE_ORG, CLRCODE_RST }
#define CLRLIST_LEN 8

#endif //DISASTERSERVER_COLORS_HPP
