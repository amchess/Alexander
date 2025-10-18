#include "../../evaluate.h"
#include "trace.h"
#include "passed_pawns.h"
#include "../evaluate_handicap.h"
#include "../../pawns.h"
#include "../../uci.h"
#include "../../movegen.h"
#include "../../bitboard.h"
#include <sstream>


namespace Alexander {


using namespace Trace;

namespace Eval {
std::string analyze_passed_pawns(const Position& pos, Pawns::Entry* pawnEntry, int phase) {
    std::stringstream ss;
    Trace::Helpers::append_term_row(ss, pos, phase, "Passed", PASSED);
    ss << "=== Passed Pawns SUBELEMENTS ===\n";
    Bitboard white_passed = pawnEntry->passed_pawns(WHITE);
    Bitboard black_passed = pawnEntry->passed_pawns(BLACK);
    ss << "White: " << Trace::Helpers::bitboard_to_squares(white_passed) << "\n";
    ss << "Black: " << Trace::Helpers::bitboard_to_squares(black_passed) << "\n";

    // Analisi dettagliata dei pedoni passati
    auto analyze_passed_pawns = [&](Color color) {
        Bitboard  passed = (color == WHITE) ? white_passed : black_passed;
        Direction push   = pawn_push(color);

        while (passed)
        {
            Square pawn_sq = pop_lsb(passed);
            Rank   rank    = rank_of(pawn_sq);
            File   file    = file_of(pawn_sq);

            ss << "  " << Trace::Helpers::square_to_string(pawn_sq) << ": ";

            // Distanza dalla promozione
            int steps_to_promotion = (color == WHITE) ? (7 - rank) : rank;
            ss << steps_to_promotion << " steps to promotion";

            // Controlla se il pedone è supportato
            bool supported = false;
            if (file > FILE_A)
            {
                Square left_support = pawn_sq + push + WEST;
                if (pos.piece_on(left_support) == make_piece(color, PAWN))
                {
                    supported = true;
                }
            }
            if (file < FILE_H && !supported)
            {
                Square right_support = pawn_sq + push + EAST;
                if (pos.piece_on(right_support) == make_piece(color, PAWN))
                {
                    supported = true;
                }
            }
            ss << (supported ? " (supported)" : " (unsupported)");

            // Controlla se il pedone è bloccato
            Square square_ahead = pawn_sq + push;
            bool   blocked      = pos.piece_on(square_ahead) != NO_PIECE;
            ss << (blocked ? " (blocked)" : " (free)");

            // Valuta la forza del pedone passato in base alla traversa
            if ((color == WHITE && rank >= RANK_5) || (color == BLACK && rank <= RANK_4))
            {
                ss << " - strong";
            }
            else if ((color == WHITE && rank <= RANK_3) || (color == BLACK && rank >= RANK_6))
            {
                ss << " - weak";
            }
            else
            {
                ss << " - medium";
            }

            ss << "\n";
        }
    };

    if (white_passed)
    {
        ss << "White passed pawns detail:\n";
        analyze_passed_pawns(WHITE);
    }

    if (black_passed)
    {
        ss << "Black passed pawns detail:\n";
        analyze_passed_pawns(BLACK);
    }
    ss << "\n";
    return ss.str();
}
}

}  // namespace Alexander