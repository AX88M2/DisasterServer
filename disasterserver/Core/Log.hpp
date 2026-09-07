#ifndef LOG_HPP
#define LOG_HPP

#include <iostream>
#include <source_location>
#include <thread>

#ifdef true //SYS_USE_SDL2
    #define DEBUG_TYPE "DBG "
    #define INFO_TYPE "INF "
    #define WARN_TYPE "WRN "
    #define ERROR_TYPE "ERR "

    #define LOG_GRN CLRCODE_GRN
    #define LOG_GRA CLRCODE_GRA
    #define LOG_RED CLRCODE_RED
    #define LOG_BLU CLRCODE_BLU
    #define LOG_YLW CLRCODE_YLW
    #define LOG_PUR CLRCODE_PUR
    #define LOG_RST CLRCODE_RST
#else
    #define DEBUG_TYPE "\x1B[36mDBG\x1B[0m"
    #define INFO_TYPE "\x1B[32mINF\x1B[0m"
    #define WARN_TYPE "\x1B[33mWRN\x1B[0m"
    #define ERROR_TYPE "\x1B[31mERR\x1B[0m"

    #define LOG_GRN
    #define LOG_GRA
    #define LOG_RED
    #define LOG_BLU
    #define LOG_YLW
    #define LOG_PUR
    #define LOG_RST
#endif

namespace DisasterServer
{
    namespace TerminalColors {
        constexpr std::string_view reset = "\x1b[0m";
        constexpr std::string_view bright = "\x1b[1m";
        constexpr std::string_view dim = "\x1b[2m";
        constexpr std::string_view underscore = "\x1b[4m";
        constexpr std::string_view blink = "\x1b[5m";
        constexpr std::string_view reverse = "\x1b[7m";
        constexpr std::string_view hidden = "\x1b[8m";

        constexpr std::string_view black = "\x1b[30m";
        constexpr std::string_view red = "\x1b[31m";
        constexpr std::string_view green = "\x1b[32m";
        constexpr std::string_view yellow = "\x1b[33m";
        constexpr std::string_view blue = "\x1b[34m";
        constexpr std::string_view magenta = "\x1b[35m";
        constexpr std::string_view cyan = "\x1b[36m";
        constexpr std::string_view white = "\x1b[37m";
        constexpr std::string_view standard = "\x1b[39m";
        constexpr std::string_view light_gray = "\x1b[90m";
        constexpr std::string_view light_red = "\x1b[91m";
        constexpr std::string_view light_green = "\x1b[92m";
        constexpr std::string_view light_yellow = "\x1b[93m";
        constexpr std::string_view light_blue = "\x1b[94m";
        constexpr std::string_view light_magenta = "\x1b[95m";
        constexpr std::string_view light_cyan = "\x1b[96m";
        constexpr std::string_view light_white = "\x1b[97m";


        constexpr std::string_view bg_black = "\x1b[40m";
        constexpr std::string_view bg_red = "\x1b[41m";
        constexpr std::string_view bg_green = "\x1b[42m";
        constexpr std::string_view bg_yellow = "\x1b[43m";
        constexpr std::string_view bg_blue = "\x1b[44m";
        constexpr std::string_view bg_magenta = "\x1b[45m";
        constexpr std::string_view bg_cyan = "\x1b[46m";
        constexpr std::string_view bg_white = "\x1b[47m";
    }

    enum class LogLevel {
        Debug, Info, Warning, Error
    };

    class Logger {
        static void write(LogLevel level, std::string_view message, std::source_location &location);

        template <typename... Args>
        static void log(LogLevel level, std::source_location location, std::format_string<Args...> fmt, Args&&... args) {
            write(level, std::format(fmt, std::forward<Args>(args)...), location);
        }

    public:

        template <typename... Args>
        static void debug(std::source_location location, std::format_string<Args...> fmt, Args&&... args) {
#ifdef _DEBUG
            log(LogLevel::Debug, location, fmt, std::forward<Args>(args)...);
#endif
        }

        template <typename... Args>
        static void info(std::source_location location, std::format_string<Args...> fmt, Args&&... args) {
            log(LogLevel::Info, location, fmt, std::forward<Args>(args)...);
        }

        template <typename... Args>
        static void warning(std::source_location location, std::format_string<Args...> fmt, Args&&... args) {
            log(LogLevel::Warning, location, fmt, std::forward<Args>(args)...);
        }

        template <typename... Args>
        static void error(std::source_location location, std::format_string<Args...> fmt, Args&&... args) {
            log(LogLevel::Error, location, fmt, std::forward<Args>(args)...);
        }
    };

    constexpr std::string_view BoolStringify(bool value) {
        return value ? "true" : "false";
    }
}

#define RAssert(x) if (!(x)) { Err("RAssert({}) failed!", #x); return false; }
#define RAssertEx(x) if (!(x)) { Err("RAssert({}) failed!", #x); }

#define Info(fmt, ...) DisasterServer::Logger::info(std::source_location::current(), fmt, ##__VA_ARGS__)
#define Warn(fmt, ...) DisasterServer::Logger::warning(std::source_location::current(), fmt, ##__VA_ARGS__)
#define Err(fmt, ...) DisasterServer::Logger::error(std::source_location::current(), fmt, ##__VA_ARGS__)
#define Debug(fmt, ...) DisasterServer::Logger::debug(std::source_location::current(), fmt, ##__VA_ARGS__)

#endif //LOG_HPP
