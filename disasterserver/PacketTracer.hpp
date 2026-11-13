#pragma once

#include "Core/Singleton.hpp"

enum class PacketType : uint8_t;

namespace DisasterServer
{
    class PacketTracer : public Singleton<PacketTracer> {
        friend class PacketTracer;
#pragma pack(push, 1)
        struct TraceMetadata {
            uint64_t counts;
        };

        struct EntryTrace {
            bool received;
            uint64_t timestamp;
            uint8_t buffer[PACKET_MAXSIZE];
        };
#pragma pack(pop)
        std::ofstream traceFile;
        std::vector<EntryTrace> entries;
    public:
        PacketTracer();
        ~PacketTracer();

        void initialize();
        void addRecord(bool received, const std::array<uint8_t, PACKET_MAXSIZE> &buffer);
        void save();
    };
}
