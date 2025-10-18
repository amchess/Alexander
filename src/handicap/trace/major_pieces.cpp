#include "../../evaluate.h"
#include "trace.h"
#include "../evaluate_handicap.h"
#include "../../uci.h"
#include "../../movegen.h"
#include "../../bitboard.h"
#include <sstream>

namespace Alexander {

using namespace Trace;

namespace Eval {

std::string analyze_rooks_and_queens(const Position& pos, int phase) {
    std::stringstream ss;
    Trace::Helpers::append_term_row(ss, pos, phase, "Rooks", ROOK);
    ss << "\n";
    Trace::Helpers::append_term_row(ss, pos, phase, "Queens", QUEEN);
    ss << "Diagonals and columns control:\n";
    ss << "=== LONG RANGE PIECES SUBELEMENTS ===\n";
    // SEZIONE COLONNE APERTE E SEMIAPERTE
    ss << "Files:\n";
    ss << "=======\n";

    Bitboard allPawns   = pos.pieces(PAWN);
    Bitboard whitePawns = pos.pieces(WHITE, PAWN);
    Bitboard blackPawns = pos.pieces(BLACK, PAWN);

    // Open files (no pawns)
    ss << "Open: ";
    bool hasOpen = false;
    for (File f = FILE_A; f <= FILE_H; ++f)
    {
        if (!(allPawns & file_bb(f)))
        {
            ss << static_cast<char>('a' + f) << " ";
            hasOpen = true;
        }
    }
    if (!hasOpen)
        ss << "none";
    ss << "\n";

    // Semi-open files for white (no white pawns)
    ss << "Semi-open White: ";
    bool hasSemiOpenWhite = false;
    for (File f = FILE_A; f <= FILE_H; ++f)
    {
        if (!(whitePawns & file_bb(f)) && (blackPawns & file_bb(f)))
        {
            ss << static_cast<char>('a' + f) << " ";
            hasSemiOpenWhite = true;
        }
    }
    if (!hasSemiOpenWhite)
        ss << "none";
    ss << "\n";

    // Semi-open files for black (no black pawns)
    ss << "Semi-open Black: ";
    bool hasSemiOpenBlack = false;
    for (File f = FILE_A; f <= FILE_H; ++f)
    {
        if (!(blackPawns & file_bb(f)) && (whitePawns & file_bb(f)))
        {
            ss << static_cast<char>('a' + f) << " ";
            hasSemiOpenBlack = true;
        }
    }
    if (!hasSemiOpenBlack)
        ss << "none";
    ss << "\n";

    // SEZIONE DIAGONALI APERTE E SEMIAPERTE
    ss << "Diagonals:\n";
    ss << "==========\n";

    // Helper function to get diagonal endpoints
    auto get_diagonal_endpoints = [](Bitboard diag) -> std::pair<Square, Square> {
        if (diag == 0)
            return {SQ_NONE, SQ_NONE};

        Square minSq = pop_lsb(diag);
        Square maxSq = msb(diag);

        // Find actual endpoints by checking file and rank
        Square lowest  = minSq;
        Square highest = maxSq;

        while (diag)
        {
            Square s = pop_lsb(diag);
            if (file_of(s) < file_of(lowest)
                || (file_of(s) == file_of(lowest) && rank_of(s) < rank_of(lowest)))
                lowest = s;
            if (file_of(s) > file_of(highest)
                || (file_of(s) == file_of(highest) && rank_of(s) > rank_of(highest)))
                highest = s;
        }

        return {lowest, highest};
    };

    // Generate all possible diagonals dynamically
    auto generate_diagonals = []() -> std::vector<Bitboard> {
        std::vector<Bitboard> diagonals;

        // Generate A1-H8 diagonals
        for (int start_rank = RANK_1; start_rank <= RANK_8; ++start_rank)
        {
            for (int start_file = FILE_A; start_file <= FILE_H; ++start_file)
            {
                Bitboard diag     = 0;
                Square   start_sq = make_square(File(start_file), Rank(start_rank));

                // Generate diagonal in A1-H8 direction
                Square sq = start_sq;
                while (is_ok(sq))
                {
                    diag |= sq;
                    if (file_of(sq) == FILE_H || rank_of(sq) == RANK_8)
                        break;
                    sq += NORTH_EAST;
                }

                // Only add if it has at least 3 squares
                if (popcount(diag) >= 3)
                {
                    bool duplicate = false;
                    for (const auto& existing : diagonals)
                    {
                        if (existing == diag)
                        {
                            duplicate = true;
                            break;
                        }
                    }
                    if (!duplicate)
                    {
                        diagonals.push_back(diag);
                    }
                }
            }
        }

        // Generate H1-A8 anti-diagonals
        for (int start_rank = RANK_1; start_rank <= RANK_8; ++start_rank)
        {
            for (int start_file = FILE_A; start_file <= FILE_H; ++start_file)
            {
                Bitboard anti_diag = 0;
                Square   start_sq  = make_square(File(start_file), Rank(start_rank));

                // Generate diagonal in H1-A8 direction
                Square sq = start_sq;
                while (is_ok(sq))
                {
                    anti_diag |= sq;
                    if (file_of(sq) == FILE_A || rank_of(sq) == RANK_8)
                        break;
                    sq += NORTH_WEST;
                }

                // Only add if it has at least 3 squares
                if (popcount(anti_diag) >= 3)
                {
                    bool duplicate = false;
                    for (const auto& existing : diagonals)
                    {
                        if (existing == anti_diag)
                        {
                            duplicate = true;
                            break;
                        }
                    }
                    if (!duplicate)
                    {
                        diagonals.push_back(anti_diag);
                    }
                }
            }
        }

        return diagonals;
    };

    // Generate all diagonals
    std::vector<Bitboard> all_diagonals = generate_diagonals();

    // Separate A1-H8 and H1-A8 diagonals
    std::vector<Bitboard> a1h8_diagonals, h1a8_diagonals;

    for (const auto& diag : all_diagonals)
    {
        // Check if it's A1-H8 (NE/SW) or H1-A8 (NW/SE) by checking endpoints
        auto [start, end] = get_diagonal_endpoints(diag);
        if (file_of(start) <= file_of(end) && rank_of(start) <= rank_of(end))
        {
            a1h8_diagonals.push_back(diag);
        }
        else
        {
            h1a8_diagonals.push_back(diag);
        }
    }

    // Check A1-H8 diagonals
    ss << "A1-H8 direction:\n";

    // Open diagonals (no pawns)
    ss << "  Open: ";
    bool hasOpenDiag1 = false;
    for (const auto& diag : a1h8_diagonals)
    {
        if (!(allPawns & diag))
        {
            auto [start, end] = get_diagonal_endpoints(diag);
            ss << Trace::Helpers::square_to_string(start) << Trace::Helpers::square_to_string(end)
               << " ";
            hasOpenDiag1 = true;
        }
    }
    if (!hasOpenDiag1)
        ss << "none";
    ss << "\n";

    // Semi-open diagonals for white (no white pawns)
    ss << "  Semi-open White: ";
    bool hasSemiOpenWhiteDiag1 = false;
    for (const auto& diag : a1h8_diagonals)
    {
        if (!(whitePawns & diag) && (blackPawns & diag))
        {
            auto [start, end] = get_diagonal_endpoints(diag);
            ss << Trace::Helpers::square_to_string(start) << Trace::Helpers::square_to_string(end)
               << " ";
            hasSemiOpenWhiteDiag1 = true;
        }
    }
    if (!hasSemiOpenWhiteDiag1)
        ss << "none";
    ss << "\n";

    // Semi-open diagonals for black (no black pawns)
    ss << "  Semi-open Black: ";
    bool hasSemiOpenBlackDiag1 = false;
    for (const auto& diag : a1h8_diagonals)
    {
        if (!(blackPawns & diag) && (whitePawns & diag))
        {
            auto [start, end] = get_diagonal_endpoints(diag);
            ss << Trace::Helpers::square_to_string(start) << Trace::Helpers::square_to_string(end)
               << " ";
            hasSemiOpenBlackDiag1 = true;
        }
    }
    if (!hasSemiOpenBlackDiag1)
        ss << "none";
    ss << "\n";

    // Check H1-A8 diagonals
    ss << "H1-A8 direction:\n";

    // Open diagonals (no pawns)
    ss << "  Open: ";
    bool hasOpenDiag2 = false;
    for (const auto& diag : h1a8_diagonals)
    {
        if (!(allPawns & diag))
        {
            auto [start, end] = get_diagonal_endpoints(diag);
            ss << Trace::Helpers::square_to_string(start) << Trace::Helpers::square_to_string(end)
               << " ";
            hasOpenDiag2 = true;
        }
    }
    if (!hasOpenDiag2)
        ss << "none";
    ss << "\n";

    // Semi-open diagonals for white (no white pawns)
    ss << "  Semi-open White: ";
    bool hasSemiOpenWhiteDiag2 = false;
    for (const auto& diag : h1a8_diagonals)
    {
        if (!(whitePawns & diag) && (blackPawns & diag))
        {
            auto [start, end] = get_diagonal_endpoints(diag);
            ss << Trace::Helpers::square_to_string(start) << Trace::Helpers::square_to_string(end)
               << " ";
            hasSemiOpenWhiteDiag2 = true;
        }
    }
    if (!hasSemiOpenWhiteDiag2)
        ss << "none";
    ss << "\n";

    // Semi-open diagonals for black (no black pawns)
    ss << "  Semi-open Black: ";
    bool hasSemiOpenBlackDiag2 = false;
    for (const auto& diag : h1a8_diagonals)
    {
        if (!(blackPawns & diag) && (whitePawns & diag))
        {
            auto [start, end] = get_diagonal_endpoints(diag);
            ss << Trace::Helpers::square_to_string(start) << Trace::Helpers::square_to_string(end)
               << " ";
            hasSemiOpenBlackDiag2 = true;
        }
    }
    if (!hasSemiOpenBlackDiag2)
        ss << "none";
    ss << "\n";

    return ss.str();
}

}

}  // namespace Alexander