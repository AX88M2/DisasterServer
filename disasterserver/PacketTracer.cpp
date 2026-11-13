#include "PacketTracer.hpp"

using namespace DisasterServer;

PacketTracer::PacketTracer() = default;

PacketTracer::~PacketTracer() {
    if (traceFile.is_open()) {
        traceFile.close();
    }
}

void PacketTracer::initialize() {
    traceFile.open(std::format("Traces/{}.trace", getCurrentTime("%d.%m.%Y %H-%M-%S")), std::ios::binary | std::ios::out);
}

void PacketTracer::addRecord(const bool received, const std::array<uint8_t, PACKET_MAXSIZE> &buffer) {
    EntryTrace entry = {};
    entry.received = received;
    entry.timestamp = static_cast<uint64_t>(std::time(nullptr));
    memcpy(entry.buffer, buffer.data(), buffer.size());

    entries.push_back(entry);
}

void PacketTracer::save() {
    TraceMetadata metadata {};
    metadata.counts = entries.size();
    traceFile.write(reinterpret_cast<char *>(&metadata), sizeof(TraceMetadata));

    for (EntryTrace entry : entries) {
        traceFile.write(reinterpret_cast<char *>(&entry), sizeof(EntryTrace));
    }
}
