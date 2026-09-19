#ifndef DISASTERSERVER_PLAYER_HPP
#define DISASTERSERVER_PLAYER_HPP

#include <array>

#include "Core/Time.hpp"
#include "Core/Types.hpp"
#include "Core/Vector2.hpp"

namespace DisasterServer
{
    class GameState;

    class PlayerStats {
        double survive_time = 0;
        double danger_time = 0;
        double camp_time = 0.0;

        double braindead_time = 0.0;
        bool brain_damage = false;

        uint16_t stun_time = 0;
        uint16_t stuns = 0;
        uint16_t hp_restored = 0;
        uint16_t rings = 0;
        uint16_t damage = 0;
        uint16_t damage_taken = 0;
        uint16_t kills = 0;
    public:
        PlayerStats() = default;
        ~PlayerStats() = default;

        void addSurviveTime() { survive_time++; }
        double getSurviveTime() const { return this->survive_time; }
        void setSurviveTime(double value) { this->survive_time = value; }

        void addDangerTime() { danger_time++; }
        double getDangerTime() const { return this->danger_time; }
        void setDangerTime(double value) { this->danger_time = value; }

        void addCampTime() { camp_time++; }
        double getCampTime() const { return this->camp_time; }
        void setCampTime(double value) { this->camp_time = value; }

        void addBraindeadTime() { braindead_time++; }
        double getBraindeadTime() const { return this->braindead_time; }
        void setBraindeadTime(double value) { this->braindead_time = value; }

        void setBrainDamage(bool flag) { this->brain_damage = flag; }
        bool getBrainDamage() const { return this->brain_damage; }

        void addStunTime() { stun_time++; }
        uint16_t getStunTime() const { return this->stun_time; }
        void setStunTime(double time) { this->stun_time = time; }

        void addStun() { stuns++; }
        uint16_t getStuns() const { return this->stuns; }
        void setStuns(uint16_t value) { this->stuns = value; }

        void addHpRestored() { hp_restored++; }
        uint16_t getHpRestored() const { return this->hp_restored; }
        void setHpRestored(uint16_t value) { this->hp_restored = value; }

        void addRing() { rings++; }
        uint16_t getRings() const { return rings; }
        void clearRings() { rings = 0; }

        void addDamage() { damage++; }
        uint16_t getDamage() const { return this->damage; }
        void setDamage(uint16_t value) { this->damage = value; }

        void addDamageTaken() { damage_taken++; }
        uint16_t getDamageTaken() const { return this->damage_taken; }
        void setDamageTaken(uint16_t value) { this->damage_taken = value; }

        void addKill() { kills++; }
        uint16_t getKills() const { return this->kills; }
        void setKills(uint16_t value) { this->kills = value; }

        void reset() {
            this->survive_time = 0;
            this->danger_time = 0;
            this->camp_time = 0.0;
            this->braindead_time = 0;
            this->brain_damage = false;
            this->stun_time = 0;
            this->stuns = 0;
            this->hp_restored = 0;
            this->rings = 0;
            this->damage = 0;
            this->damage_taken = 0;
            this->kills = 0;
        }
    };

    class Player {
        uint8_t	ready = 0;
        uint16_t seq = 0;
        uint16_t errors = 0; /* Used as tracker for errors like lag/etc */
        uint8_t	exTeleport = 0;
        double timeout = 0.0;

        uint32_t modifiedClientTimer = 0;
        uint32_t chunk = 0;
        TimeStamp lastPacket;

        /* Attack */
        bool isAttack = false;
        double attackTimer = 0.0;
        TimeStamp lastAttack;

        uint16_t pingLast = 0;
        double pingTotal = 0.0;
        double pingTimer = 0.0;
        uint16_t rings = 0;
        TimeStamp lastRings;
        uint16_t healRings = 0;

        uint8_t	state = 0;
        PlayerFlags flags = 0;
        uint8_t	deathTimerSec = 0;
        double deathTimer = 0;
        double revival = 0.0;
        std::array<int32_t, 5> revivalInit = { -1, -1, -1, -1, -1 };

        struct Userdata {
            uint8_t	shards = 0;
        } userdata = {};

        Vector2 startPos = {};
        Vector2 position = {};

        PlayerStats stats = {};

    public:
        enum class Flags : uint8_t {
            PLAYER_NONE = 0,
            PLAYER_ESCAPED = 0x1 << 0,
            PLAYER_DEAD = 0x1 << 1,
            PLAYER_DEMONIZED = 0x1 << 2,
            PLAYER_REVIVED = 0x1 << 3,
            PLAYER_CANTREVIVE = 0x1 << 4,
            PLAYER_LEFT = 0x1 << 5,
            PLAYER_KILLER = 0x1 << 6,
            PLAYER_ATTACKING = 1 << 4,
        };

        Player();
        ~Player();

        void reset();

        bool isFlag(Flags flag) const { return this->flags & static_cast<PlayerFlags>(flag); }
        void setFlag(Flags flag) { this->flags |= static_cast<PlayerFlags>(flag); }
        void delFlag(Flags flag) { this->flags = this->flags & ~static_cast<PlayerFlags>(flag); }

        uint8_t isReady() const { return ready; }
        void setReady(const bool flag) { ready = flag; }

        uint16_t getSeq() const { return this->seq; }
        void setSeq(const uint16_t value) { this->seq = value; }

        uint16_t getErrors() const { return this->errors; }
        void setErrors(const uint16_t value) { this->errors = value; }

        uint8_t getExTeleport() const { return this->exTeleport; }
        void setExTeleport(const uint8_t value) { this->exTeleport = value; }

        double getTimeout() const { return this->timeout; }
        void setTimeout(const double value) { this->timeout = value; }

        uint32_t getChunk() const { return this->chunk; }
        void setChunk(const uint32_t value) { this->chunk = value; }

        TimeStamp getLastPacket() const { return this->lastPacket; }
        void setLastPacket(const TimeStamp value) { this->lastPacket = value; }

        TimeStamp getLastRings() const { return this->lastRings; }
        void setLastRings(const TimeStamp value) { this->lastRings = value; }

        bool isAttacking() const { return this->isAttack; }
        void setAttacking(const bool value) { this->isAttack = value; }

        double getAttackTimer() const { return this->attackTimer; }
        void setAttackTimer(double value) { this->attackTimer = value; }

        TimeStamp getLastAttack() const { return this->lastAttack; }
        void setLastAttack(const TimeStamp value) { this->lastAttack = value; }

        uint16_t getLastPing() const { return this->pingLast; }
        void setLastPing(const uint16_t value) { this->pingLast = value; }

        uint16_t getRings() const { return rings; }
        void setRings(uint16_t ring) { this->rings = ring; }

        uint8_t getState() { return this->state; }
        void setState(const uint8_t value) { this->state = value; }

        uint8_t getDeathTimerSec() const { return this->deathTimerSec; }
        void setDeathTimerSec(const uint8_t value) { this->deathTimerSec = value; }

        Vector2 getStartPosition() const { return this->startPos; }
        void setStartPosition(Vector2 vec2) { this->startPos = vec2;  }

        Vector2 getPosition() const { return this->position; }
        void setPosition(Vector2 vec2) { position = vec2; }

        PlayerStats getStats() const { return this->stats; }
    };
}

#endif //DISASTERSERVER_PLAYER_HPP
