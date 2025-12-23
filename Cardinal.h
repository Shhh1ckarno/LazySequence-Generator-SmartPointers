#pragma once

#include <cstddef>
#include <stdexcept>
#include <limits>

enum class CardinalType { FINITE = 0, OMEGA = 1 };

class Cardinal {
public:
    Cardinal() noexcept : type(CardinalType::FINITE), value(0) {}
    Cardinal(std::size_t v) noexcept : type(CardinalType::FINITE), value(v) {}

    static Cardinal Omega() noexcept { return Cardinal(CardinalType::OMEGA); }

    bool IsOmega() const noexcept { return type == CardinalType::OMEGA; }
    bool IsFinite() const noexcept { return type == CardinalType::FINITE; }

    std::size_t GetValue() const {
        if (IsOmega()) throw std::logic_error("Cardinal::GetValue() called on Omega");
        return value;
    }

    Cardinal operator+(const Cardinal& other) const noexcept {
        if (IsOmega() || other.IsOmega()) return Omega();
        const std::size_t maxv = std::numeric_limits<std::size_t>::max();
        if (value > maxv - other.value) return Omega();
        return Cardinal(value + other.value);
    }

    bool operator==(const Cardinal& other) const noexcept {
        if (IsOmega() && other.IsOmega()) return true;
        if (IsFinite() && other.IsFinite()) return value == other.value;
        return false;
    }
    bool operator!=(const Cardinal& other) const noexcept { return !(*this == other); }

    bool operator<(const Cardinal& other) const noexcept {
        if (IsFinite() && other.IsOmega()) return true;
        if (IsOmega() && other.IsFinite()) return false;
        if (IsOmega() && other.IsOmega()) return false;
        return value < other.value;
    }
    bool operator<=(const Cardinal& other) const noexcept { return (*this < other) || (*this == other); }
    bool operator>(const Cardinal& other) const noexcept { return other < *this; }
    bool operator>=(const Cardinal& other) const noexcept { return other <= *this; }

private:
    Cardinal(CardinalType t) noexcept : type(t), value(0) {}

    CardinalType type;
    std::size_t value;
};