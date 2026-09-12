#ifndef DISASTERSERVER_COUNTDOWN_HPP
#define DISASTERSERVER_COUNTDOWN_HPP

#include "Core/Types.hpp"

namespace DisasterServer
{
    class Countdown final
    {
    public:
        enum class TickResult : std::uint8_t
        {
            None,
            Second,
            Finished
        };

        explicit Countdown(double ticksPerSecond) noexcept
            : ticksPerSecond_{ticksPerSecond}
        {}

        void start(int seconds) noexcept {
            seconds_ = seconds;
            ticks_ = ticksPerSecond_;
        }

        void stop() noexcept {
            ticks_ = 0;
            seconds_ = 0;
        }

        void setRemaining(int seconds) noexcept {
            seconds_ = seconds;
            ticks_ = 0;
        }

        [[nodiscard]]
        TickResult tick(double delta) noexcept {
            if (!active())
                return TickResult::None;

            ticks_ -= delta;

            if (ticks_ > 0)
                return TickResult::None;

            ticks_ += ticksPerSecond_;
            --seconds_;

            if (seconds_ <= 0)
            {
                stop();
                return TickResult::Finished;
            }

            return TickResult::Second;
        }

        [[nodiscard]]
        bool active() const noexcept {
            return seconds_ > 0;
        }

        [[nodiscard]]
        int remaining() const noexcept {
            return seconds_;
        }

    private:
        double ticksPerSecond_;
        double ticks_ = 0;
        int seconds_ = 0;
    };
}

#endif //DISASTERSERVER_COUNTDOWN_HPP
