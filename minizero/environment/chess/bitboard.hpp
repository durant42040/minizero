#pragma once

#include "square.hpp"

#include <cstdint>
#include <iostream>
#include <iterator>

namespace minizero::env::chess {

constexpr uint64_t kPawnStartPos = 0xFF00 | (0xFFULL << 48);
constexpr uint64_t kRookStartPos = 0x81 | (0x81ULL << 56);
constexpr uint64_t kKnightStartPos = 0x42 | (0x42ULL << 56);
constexpr uint64_t kBishopStartPos = 0x24 | (0x24ULL << 56);
constexpr uint64_t kKingStartPos = 0x10 | (0x10ULL << 56);
constexpr uint64_t kQueenStartPos = 0x08 | (0x08ULL << 56);

constexpr uint64_t kWhiteStartPos = 0xFFFF;
constexpr uint64_t kBlackStartPos = 0xFFFFULL << 48;

constexpr uint64_t kStartPos = kWhiteStartPos | kBlackStartPos;

template <typename T>
class BitIterator {
public:
    using iterator_category = std::input_iterator_tag;
    using difference_type = T;
    using value_type = T;
    using pointer = T*;
    using reference = T&;

    BitIterator(std::uint64_t value) : value_(value) {}
    bool operator!=(const BitIterator& other) { return value_ != other.value_; }

    void operator++() { value_ &= (value_ - 1); }
    T operator*() const { return __builtin_ctzll(value_); }

private:
    std::uint64_t value_;
};

class Bitboard {
public:
    Bitboard(uint64_t board) : bitboard_(board) {}

    // check if the ith bit is set
    inline bool get(Square i) const
    {
        return get(i.square_);
    }

    inline bool get(int i) const
    {
        return bitboard_ & (1ULL << i);
    }

    // set the ith bit
    inline void set(Square i)
    {
        set(i.square_);
    }

    inline void set(int i)
    {
        bitboard_ |= (1ULL << i);
    }

    inline void reset()
    {
        bitboard_ = 0;
    }

    // clear the ith bit
    inline void clear(Square i)
    {
        clear(i.square_);
    }

    inline void clear(int i)
    {
        bitboard_ &= ~(1ULL << i);
    }

    inline bool empty() const
    {
        return bitboard_ == 0;
    }

    // check if "from" bit is 1, if so, make move to "to" bit
    inline void update(Square from, Square to)
    {
        clear(to);
        if (get(from)) {
            clear(from);
            set(to);
        }
    }

    inline int getLSB() const
    {
        return __builtin_ctzll(bitboard_);
    }

    inline int count() const
    {
        return __builtin_popcountll(bitboard_);
    }

    friend Bitboard operator&(const Bitboard& lhs, const Bitboard& rhs)
    {
        return Bitboard(lhs.bitboard_ & rhs.bitboard_);
    }

    friend Bitboard operator|(const Bitboard& lhs, const Bitboard& rhs)
    {
        return Bitboard(lhs.bitboard_ | rhs.bitboard_);
    }

    friend Bitboard operator~(const Bitboard& board)
    {
        return Bitboard(~board.bitboard_);
    }

    friend Bitboard operator<<(const Bitboard& board, int shift)
    {
        return Bitboard(board.bitboard_ << shift);
    }

    friend Bitboard operator>>(const Bitboard& board, int shift)
    {
        return Bitboard(board.bitboard_ >> shift);
    }

    friend Bitboard operator|=(Bitboard& lhs, const Bitboard& rhs)
    {
        lhs.bitboard_ |= rhs.bitboard_;
        return lhs;
    }

    friend Bitboard operator&=(Bitboard& lhs, const Bitboard& rhs)
    {
        lhs.bitboard_ &= rhs.bitboard_;
        return lhs;
    }

    friend std::ostream& operator<<(std::ostream& os, const Bitboard& board)
    {
        for (int i = 7; i >= 0; i--) {
            for (int j = 0; j < 8; j++) {
                os << (board.get(i * 8 + j) ? "X" : "-") << " ";
            }
            os << std::endl;
        }
        return os;
    }

    BitIterator<Square> begin() const { return bitboard_; }
    BitIterator<Square> end() const { return 0; }

    uint64_t bitboard_;
};

} // namespace minizero::env::chess
