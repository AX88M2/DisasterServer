#pragma once

namespace DisasterServer
{
    class ServerException : public std::exception {
    public:
        explicit ServerException(std::string message) : message(std::move(message)) {}

        const char* what() const noexcept override {
            return message.c_str();
        }

        template <typename... Args>
        static ServerException format(std::format_string<Args...> fmt, Args&&... args) {
            return ServerException(std::format(fmt, std::forward<Args>(args)...));
        }

    private:
        std::string message;
    };

    class PacketError : public ServerException {
    public:
        explicit PacketError(std::string message) : ServerException(message) {
        }

        template <typename... Args>
        static PacketError format(std::format_string<Args...> fmt, Args&&... args) {
            return PacketError(std::format(fmt, std::forward<Args>(args)...));
        }
    };
}