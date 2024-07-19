#include "bitboard.hpp"
#include "move_generator.hpp"
namespace minizero::env::chess {

const Bitboard kWhitePawnCaptures[64] = {
        0x200ULL, 0x500ULL, 0xa00ULL, 0x1400ULL,
        0x2800ULL, 0x5000ULL, 0xa000ULL, 0x4000ULL,
        0x20000ULL, 0x50000ULL, 0xa0000ULL, 0x140000ULL,
        0x280000ULL, 0x500000ULL, 0xa00000ULL, 0x400000ULL,
        0x2000000ULL, 0x5000000ULL, 0xa000000ULL, 0x14000000ULL,
        0x28000000ULL, 0x50000000ULL, 0xa0000000ULL, 0x40000000ULL,
        0x200000000ULL, 0x500000000ULL, 0xa00000000ULL, 0x1400000000ULL,
        0x2800000000ULL, 0x5000000000ULL, 0xa000000000ULL, 0x4000000000ULL,
        0x20000000000ULL, 0x50000000000ULL, 0xa0000000000ULL, 0x140000000000ULL,
        0x280000000000ULL, 0x500000000000ULL, 0xa00000000000ULL, 0x400000000000ULL,
        0x2000000000000ULL, 0x5000000000000ULL, 0xa000000000000ULL, 0x14000000000000ULL,
        0x28000000000000ULL, 0x50000000000000ULL, 0xa0000000000000ULL, 0x40000000000000ULL,
        0x200000000000000ULL, 0x500000000000000ULL, 0xa00000000000000ULL, 0x1400000000000000ULL,
        0x2800000000000000ULL, 0x5000000000000000ULL, 0xa000000000000000ULL, 0x4000000000000000ULL,
        0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL,
        0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL,
};

const Bitboard kBlackPawnCaptures[64] = {
        0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL,
        0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL,
        0x2ULL, 0x5ULL, 0xaULL, 0x14ULL,
        0x28ULL, 0x50ULL, 0xa0ULL, 0x40ULL,
        0x200ULL, 0x500ULL, 0xa00ULL, 0x1400ULL,
        0x2800ULL, 0x5000ULL, 0xa000ULL, 0x4000ULL,
        0x20000ULL, 0x50000ULL, 0xa0000ULL, 0x140000ULL,
        0x280000ULL, 0x500000ULL, 0xa00000ULL, 0x400000ULL,
        0x2000000ULL, 0x5000000ULL, 0xa000000ULL, 0x14000000ULL,
        0x28000000ULL, 0x50000000ULL, 0xa0000000ULL, 0x40000000ULL,
        0x200000000ULL, 0x500000000ULL, 0xa00000000ULL, 0x1400000000ULL,
        0x2800000000ULL, 0x5000000000ULL, 0xa000000000ULL, 0x4000000000ULL,
        0x20000000000ULL, 0x50000000000ULL, 0xa0000000000ULL, 0x140000000000ULL,
        0x280000000000ULL, 0x500000000000ULL, 0xa00000000000ULL, 0x400000000000ULL,
        0x2000000000000ULL, 0x5000000000000ULL, 0xa000000000000ULL, 0x14000000000000ULL,
        0x28000000000000ULL, 0x50000000000000ULL, 0xa0000000000000ULL, 0x40000000000000ULL,
};

const Bitboard kKingAttacks[64] = {
        0x302ULL, 0x705ULL, 0xe0aULL, 0x1c14ULL,
        0x3828ULL, 0x7050ULL, 0xe0a0ULL, 0xc040ULL,
        0x30203ULL, 0x70507ULL, 0xe0a0eULL, 0x1c141cULL,
        0x382838ULL, 0x705070ULL, 0xe0a0e0ULL, 0xc040c0ULL,
        0x3020300ULL, 0x7050700ULL, 0xe0a0e00ULL, 0x1c141c00ULL,
        0x38283800ULL, 0x70507000ULL, 0xe0a0e000ULL, 0xc040c000ULL,
        0x302030000ULL, 0x705070000ULL, 0xe0a0e0000ULL, 0x1c141c0000ULL,
        0x3828380000ULL, 0x7050700000ULL, 0xe0a0e00000ULL, 0xc040c00000ULL,
        0x30203000000ULL, 0x70507000000ULL, 0xe0a0e000000ULL, 0x1c141c000000ULL,
        0x382838000000ULL, 0x705070000000ULL, 0xe0a0e0000000ULL, 0xc040c0000000ULL,
        0x3020300000000ULL, 0x7050700000000ULL, 0xe0a0e00000000ULL, 0x1c141c00000000ULL,
        0x38283800000000ULL, 0x70507000000000ULL, 0xe0a0e000000000ULL, 0xc040c000000000ULL,
        0x302030000000000ULL, 0x705070000000000ULL, 0xe0a0e0000000000ULL, 0x1c141c0000000000ULL,
        0x3828380000000000ULL, 0x7050700000000000ULL, 0xe0a0e00000000000ULL, 0xc040c00000000000ULL,
        0x203000000000000ULL, 0x507000000000000ULL, 0xa0e000000000000ULL, 0x141c000000000000ULL,
        0x2838000000000000ULL, 0x5070000000000000ULL, 0xa0e0000000000000ULL, 0x40c0000000000000ULL,
};

const Bitboard kKnightAttacks[64] = {
        0x0000000000020400ULL, 0x0000000000050800ULL, 0x00000000000A1100ULL,
        0x0000000000142200ULL, 0x0000000000284400ULL, 0x0000000000508800ULL,
        0x0000000000A01000ULL, 0x0000000000402000ULL, 0x0000000002040004ULL,
        0x0000000005080008ULL, 0x000000000A110011ULL, 0x0000000014220022ULL,
        0x0000000028440044ULL, 0x0000000050880088ULL, 0x00000000A0100010ULL,
        0x0000000040200020ULL, 0x0000000204000402ULL, 0x0000000508000805ULL,
        0x0000000A1100110AULL, 0x0000001422002214ULL, 0x0000002844004428ULL,
        0x0000005088008850ULL, 0x000000A0100010A0ULL, 0x0000004020002040ULL,
        0x0000020400040200ULL, 0x0000050800080500ULL, 0x00000A1100110A00ULL,
        0x0000142200221400ULL, 0x0000284400442800ULL, 0x0000508800885000ULL,
        0x0000A0100010A000ULL, 0x0000402000204000ULL, 0x0002040004020000ULL,
        0x0005080008050000ULL, 0x000A1100110A0000ULL, 0x0014220022140000ULL,
        0x0028440044280000ULL, 0x0050880088500000ULL, 0x00A0100010A00000ULL,
        0x0040200020400000ULL, 0x0204000402000000ULL, 0x0508000805000000ULL,
        0x0A1100110A000000ULL, 0x1422002214000000ULL, 0x2844004428000000ULL,
        0x5088008850000000ULL, 0xA0100010A0000000ULL, 0x4020002040000000ULL,
        0x0400040200000000ULL, 0x0800080500000000ULL, 0x1100110A00000000ULL,
        0x2200221400000000ULL, 0x4400442800000000ULL, 0x8800885000000000ULL,
        0x100010A000000000ULL, 0x2000204000000000ULL, 0x0004020000000000ULL,
        0x0008050000000000ULL, 0x00110A0000000000ULL, 0x0022140000000000ULL,
        0x0044280000000000ULL, 0x0088500000000000ULL, 0x0010A00000000000ULL,
        0x0020400000000000ULL
};


Bitboard generateWhitePawnMoves(Square from, Bitboard all_pieces, Bitboard all_black_pieces) {
    Bitboard from_mask = Bitboard(1ULL << from.square_);

    Bitboard one_step_moves = (from_mask << 8) & ~all_pieces;
    Bitboard two_step_moves = ((one_step_moves & (0xFFULL << 16)) << 8) & ~all_pieces;
    Bitboard capture_moves = kWhitePawnCaptures[from.square_] & all_black_pieces;

    return (one_step_moves | two_step_moves | capture_moves);
}

Bitboard generateBlackPawnMoves(Square from, Bitboard all_pieces, Bitboard all_white_pieces) {
    Bitboard from_mask = Bitboard(1ULL << from.square_);

    Bitboard one_step_moves = (from_mask >> 8) & ~all_pieces;
    Bitboard two_step_moves = ((one_step_moves & (0xFFULL << 40)) >> 8) & ~all_pieces;
    Bitboard capture_moves = kBlackPawnCaptures[from.square_] & all_white_pieces;

    return (one_step_moves | two_step_moves | capture_moves);
}

Bitboard generateKnightMoves(Square from) {
    return kKnightAttacks[from.square_];
}

Bitboard generateKingMoves(Square from) {
    return kKingAttacks[from.square_];
}

Bitboard generateRookMoves(Square from, Bitboard all_pieces) {
    int file = from.file_;
    int rank = from.rank_;

    Bitboard validMoves(0);

    for (int i = rank + 1; i < 8; i++) {
        validMoves |= (1ULL << (8 * i + file));
        if (all_pieces.bitboard_ & 1ULL << (8 * i + file)) break;
    }
    for (int i = rank - 1; i >= 0; i--) {
        validMoves |= (1ULL << (8 * i + file));
        if (all_pieces.bitboard_ & 1ULL << (8 * i + file)) break;
    }
    for (int i = file + 1; i < 8; i++) {
        validMoves |= (1ULL << (8 * rank + i));
        if (all_pieces.bitboard_ & 1ULL << (8 * rank + i)) break;
    }
    for (int i = file - 1; i >= 0; i--) {
        validMoves |= (1ULL << (8 * rank + i));
        if (all_pieces.bitboard_ & 1ULL << (8 * rank + i)) break;
    }

    return validMoves;
}

Bitboard generateBishopMoves(Square from, Bitboard all_pieces) {
    int file = from.file_;
    int rank = from.rank_;

    Bitboard validMoves(0);

    for (int i = rank + 1, j = file + 1; i <= 7 && j <= 7; i++, j++) {
        validMoves |= (1ULL << (8 * i + j));
        if (all_pieces.bitboard_ & 1ULL << (8 * i + j)) break;
    }
    for (int i = rank - 1, j = file + 1; i >= 0 && j <= 7; i--, j++) {
        validMoves |= (1ULL << (8 * i + j));
        if (all_pieces.bitboard_ & 1ULL << (8 * i + j)) break;
    }
    for (int i = rank + 1, j = file - 1; i <= 7 && j >= 0; i++, j--) {
        validMoves |= (1ULL << (8 * i + j));
        if (all_pieces.bitboard_ & 1ULL << (8 * i + j)) break;
    }
    for (int i = rank - 1, j = file - 1; i >= 0 && j >= 0; i--, j--) {
        validMoves |= (1ULL << (8 * i + j));
        if (all_pieces.bitboard_ & 1ULL << (8 * i + j)) break;
    }
    
    return validMoves;
}

Bitboard generateQueenMoves(Square from, Bitboard all_pieces) {
    return generateRookMoves(from, all_pieces) | generateBishopMoves(from, all_pieces);
}

}
