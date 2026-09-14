#ifndef DISASTERSERVER_COUNTDOWN_HPP
#define DISASTERSERVER_COUNTDOWN_HPP

#include <algorithm>
#include <cstdint>

namespace DisasterServer
{
    class Countdown final
    {
    public:
        enum class TickResult : uint8_t
        {
            None,
            Second,
            Finished
        };

        explicit Countdown(double ticksPerSecond) noexcept
            : ticksPerSecond_{ticksPerSecond}
        {}

        /**
         * Starts countdown from the specified amount of seconds.
         **/
        void start(int seconds) noexcept {
            seconds_ = std::max(0, seconds);
            ticks_ = ticksPerSecond_;
            active_ = seconds_ > 0;
        }

        /**
         * Stops the countdown and resets it.
         **/
        void stop() noexcept {
            ticks_ = 0;
            seconds_ = 0;
            active_ = false;
        }

        /**
         * Changes remaining seconds without starting the countdown.
         **/
        void setRemaining(int seconds) noexcept
        {
            seconds_ = std::max(0, seconds);

            if (seconds_ <= 0)
            {
                stop();
                return;
            }

            active_ = true;
        }

        /**
         * Adds seconds to the current countdown.
         **/
        void add(int seconds) noexcept
        {
            if (seconds <= 0)
                return;

            seconds_ += seconds;

            if (!active_)
            {
                active_ = true;
                ticks_ = ticksPerSecond_;
            }
        }

        /**
         * Removes seconds from the countdown.
         * If the countdown reaches zero, it becomes inactive.
         **/
        void subtract(int seconds) noexcept
        {
            if (seconds <= 0 || !active_)
                return;

            seconds_ -= seconds;

            if (seconds_ <= 0)
                stop();
        }


        /**
         * Advances the countdown by delta ticks.
         *  speed = 1.0  -> normal countdown
         *  speed = 0.5  -> countdown takes twice as long
         *  speed = 2.0  -> countdown takes half as long
         **/
        [[nodiscard]]
        TickResult tick(double delta, double speed = 1.0) noexcept {
            if (!active_ || delta <= 0.0 || ticksPerSecond_ <= 0.0)
                return TickResult::None;

            if (speed <= 0.0)
                return TickResult::None;

            ticks_ -= delta * speed;

            if (ticks_ > 0.0)
                return TickResult::None;

            int elapsedSeconds = 0;

            while (ticks_ <= 0.0 && seconds_ > 0) {
                ticks_ += ticksPerSecond_;
                --seconds_;
                ++elapsedSeconds;
            }

            if (seconds_ <= 0) {
                stop();
                return TickResult::Finished;
            }

            return elapsedSeconds > 0 ? TickResult::Second : TickResult::None;
        }

        // Returns true while the countdown is running.
        [[nodiscard]]
        bool active() const noexcept {
            return active_;
        }

        // Remaining whole seconds.
        [[nodiscard]]
        int remaining() const noexcept {
            return seconds_;
        }

        // Remaining fractional time in ticks.
        [[nodiscard]]
        double remainingTicks() const noexcept {
            if (!active_)
                return 0.0;

            return ticks_;
        }

        // Remaining time in seconds, including the fractional part.
        [[nodiscard]]
        double remainingTime() const noexcept {
            if (!active_ || ticksPerSecond_ <= 0.0)
                return 0.0;

            if (seconds_ <= 0)
                return 0.0;

            return static_cast<double>(seconds_ - 1) + ticks_ / ticksPerSecond_;
        }

        [[nodiscard]]
        double ticksPerSecond() const noexcept {
            return ticksPerSecond_;
        }

        // Returns the amount of time already elapsed in the current
        // second. Useful when synchronizing a client.
        [[nodiscard]]
        double elapsedTicks() const noexcept {
            if (!active_)
                return 0.0;

            return ticksPerSecond_ - ticks_;
        }

        void reset() noexcept {
            ticks_ = 0;
            seconds_ = 0;
            active_ = false;
        }

    private:
        double ticksPerSecond_ = 0;
        double ticks_ = 0.0;
        int seconds_ = 0;
        bool active_ = false;
    };
}



#endif //DISASTERSERVER_COUNTDOWN_HPP
