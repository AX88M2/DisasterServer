#ifndef DISASTERSERVER_EXCEPTIONS_HPP
#define DISASTERSERVER_EXCEPTIONS_HPP

namespace DisasterServer
{
    class PacketError : public std::exception {
    public:
        explicit PacketError(std::string& message) : std::exception(message.data()) {}
        explicit PacketError(const std::string& message) : std::exception(message.data()) {}

        template <typename ...Args>
        static PacketError Format(std::string_view fmt, Args&&... args) {
            return PacketError(std::vformat(fmt, std::make_format_args(args...)));
        }
    };
}

#endif //DISASTERSERVER_EXCEPTIONS_HPP
