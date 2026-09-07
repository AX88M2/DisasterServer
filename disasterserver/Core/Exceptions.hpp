#ifndef DISASTERSERVER_EXCEPTIONS_HPP
#define DISASTERSERVER_EXCEPTIONS_HPP

namespace DisasterServer
{
    class PacketError : public std::exception {
    public:
        explicit PacketError(std::string message) : message(std::move(message)) {}

        const char* what() const noexcept override {
            return message.c_str();
        }

        template <typename... Args>
        static PacketError format(std::format_string<Args...> fmt, Args&&... args) {
            return PacketError(std::format(fmt, std::forward<Args>(args)...));
        }

    private:
        std::string message;
    };
}

#endif //DISASTERSERVER_EXCEPTIONS_HPP
