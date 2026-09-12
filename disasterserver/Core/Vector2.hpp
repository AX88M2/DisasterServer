#pragma once

#include <cmath>
#include <compare>

namespace DisasterServer
{
    class Vector2
    {
    public:
        float x = 0;
        float y = 0;

        constexpr Vector2() noexcept = default;

        constexpr Vector2(float x, float y) noexcept : x(x), y(y){}

        [[nodiscard]]
        float length() const noexcept {
            return std::sqrt(x * x + y * y);
        }

        [[nodiscard]]
        float distance(const Vector2& other) const noexcept {
            const float dx = other.x - x;
            const float dy = other.y - y;

            return std::sqrt(dx * dx + dy * dy);
        }

        [[nodiscard]]
        constexpr Vector2 direction(const Vector2& other) const noexcept {
            return {sign(x - other.x), sign(y - other.y)};
        }

        [[nodiscard]]
        constexpr Vector2 lerp(const Vector2& other, float factor) const noexcept {
            return {
                x * (1.0f - factor) + other.x * factor,
                y * (1.0f - factor) + other.y * factor
            };
        }

        [[nodiscard]]
        Vector2 normalized() const noexcept {
            const float len = length();

            if (len == 0.0f)
                return {};

            return {x / len, y / len};
        }

        constexpr Vector2 operator+(const Vector2& other) const noexcept {
            return {x + other.x, y + other.y};
        }

        constexpr Vector2 operator-(const Vector2& other) const noexcept {
            return {x - other.x, y - other.y};
        }

        constexpr Vector2 operator*(float scalar) const noexcept {
            return {x * scalar, y * scalar};
        }

        constexpr Vector2 operator/(float scalar) const noexcept {
            return {x / scalar, y / scalar};
        }

        constexpr Vector2& operator+=(const Vector2& other) noexcept {
            x += other.x;
            y += other.y;
            return *this;
        }

        constexpr Vector2& operator-=(const Vector2& other) noexcept {
            x -= other.x;
            y -= other.y;
            return *this;
        }

        constexpr Vector2& operator*=(float scalar) noexcept {
            x *= scalar;
            y *= scalar;
            return *this;
        }

        constexpr Vector2& operator/=(float scalar) noexcept {
            x /= scalar;
            y /= scalar;
            return *this;
        }

        constexpr bool operator==(const Vector2&) const noexcept = default;

    private:
        static constexpr float sign(float value) noexcept {
            return value > 0.0f ? 1.0f : value < 0.0f ? -1.0f : 0.0f;
        }
    };

    constexpr Vector2 operator*(float scalar, const Vector2& vector) noexcept {
        return vector * scalar;
    }
}