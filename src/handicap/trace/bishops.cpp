#include "../../evaluate.h"
#include "trace.h"
#include "../evaluate_handicap.h"
#include "../../uci.h"
#include "../../movegen.h"
#include "../../bitboard.h"
#include <sstream>
#include "bishops.h"

namespace Alexander {


using namespace Trace;

namespace Eval {

std::string
analyze_bishops(const Position& pos, int phase, const std::string& gamePhase, Value v_white) {
    std::stringstream ss;
    Trace::Helpers::append_term_row(ss, pos, phase, "Bishops", BISHOP);
    if (Alexander::Eval::handicapConfig.pawnsToEvaluate)
    {
        ss << "=== BISHOP SUBELEMENTS ===\n";
        // REGOLA DI DOVORETSKY PER FINALI DI ALFIERI DI COLORE CONTRARIO
        ss << "Dvoretsky's Rule for Opposite-Colored Bishops:\n";
        auto square_color = [](Square s) -> Color {
            return (file_of(s) + rank_of(s)) % 2 == 0 ? BLACK : WHITE;
        };

        // Helper function to get color name
        auto color_name = [](Color c) -> std::string { return c == WHITE ? "light" : "dark"; };

        // Check if we're in an endgame with opposite-colored bishops
        bool     isEndgame    = (gamePhase == "Endgame" || gamePhase == "Transition to endgame");
        Bitboard whiteBishops = pos.pieces(WHITE, BISHOP);
        Bitboard blackBishops = pos.pieces(BLACK, BISHOP);

        // Check for opposite-colored bishops (exactly one bishop each on different colors)
        if (isEndgame && popcount(whiteBishops) == 1 && popcount(blackBishops) == 1)
        {
            Square wBishopSq = lsb(whiteBishops);
            Square bBishopSq = lsb(blackBishops);

            // Check if bishops are on opposite colors
            if (square_color(wBishopSq) != square_color(bBishopSq))
            {
                ss << "Opposite-colored bishops endgame detected.\n";

                // Determine which side is weaker based on evaluation
                Color weakerSide = (v_white < 0) ? WHITE : BLACK;

                ss << "Weaker side: " << (weakerSide == WHITE ? "White" : "Black") << "\n";

                // Get the weaker side's bishop and its color
                Square weakerBishopSq = (weakerSide == WHITE) ? wBishopSq : bBishopSq;
                Color  bishopColor    = square_color(weakerBishopSq);

                ss << "Weaker bishop on " << color_name(bishopColor) << " squares\n";

                // Analyze pawns of the weaker side
                Bitboard weakerPawns          = pos.pieces(weakerSide, PAWN);
                Bitboard pawnsOnSameColor     = 0;
                Bitboard pawnsOnOppositeColor = 0;

                // Count pawns on same vs opposite color as bishop
                while (weakerPawns)
                {
                    Square pawnSq = pop_lsb(weakerPawns);
                    if (square_color(pawnSq) == bishopColor)
                    {
                        pawnsOnSameColor |= pawnSq;
                    }
                    else
                    {
                        pawnsOnOppositeColor |= pawnSq;
                    }
                }

                int sameColorCount     = popcount(pawnsOnSameColor);
                int oppositeColorCount = popcount(pawnsOnOppositeColor);

                ss << "Weaker side pawns:\n";
                ss << "  On same color as bishop: " << sameColorCount << " pawns";
                if (sameColorCount > 0)
                {
                    ss << " (";
                    Bitboard temp = pawnsOnSameColor;
                    while (temp)
                    {
                        Square s = pop_lsb(temp);
                        ss << Trace::Helpers::square_to_string(s);
                        if (temp)
                            ss << ", ";
                    }
                    ss << ")";
                }
                ss << "\n";

                ss << "  On opposite color: " << oppositeColorCount << " pawns";
                if (oppositeColorCount > 0)
                {
                    ss << " (";
                    Bitboard temp = pawnsOnOppositeColor;
                    while (temp)
                    {
                        Square s = pop_lsb(temp);
                        ss << Trace::Helpers::square_to_string(s);
                        if (temp)
                            ss << ", ";
                    }
                    ss << ")";
                }
                ss << "\n";

                // Dvoretsky's rule application
                if (oppositeColorCount > 0)
                {
                    ss << "WARNING: DVORETSKY'S RULE VIOLATION: Weaker side should keep pawns on "
                       << color_name(bishopColor) << " squares (same color as their bishop)\n";
                    ss << "   Recommendation: Try to trade pawns on opposite-colored squares\n";
                    ss << "   or reposition pawns to " << color_name(bishopColor)
                       << " squares when possible\n";
                }
                else
                {
                    ss << "OK DVORETSKY'S RULE SATISFIED: All pawns on correct color squares\n";
                }

                // Additional strategic advice
                ss << "Strategic principles:\n";
                ss << "  - Weaker side: Blockade with pawns on same color as bishop\n";
                ss << "  - Stronger side: Create passed pawns on opposite color of enemy bishop\n";
                ss << "  - Weaker side's bishop should defend pawns on its color\n";
            }
            else
            {
                ss << "Bishops on same color - standard bishop endgame rules apply\n";
            }
        }
        else if (isEndgame && (popcount(whiteBishops) == 1 || popcount(blackBishops) == 1))
        {
            // Only one bishop on the board
            ss << "Single bishop endgame (no opposite-colored bishops)\n";
        }
        else
        {
            ss << "Not an opposite-colored bishops endgame\n";
            if (!isEndgame)
            {
                ss << "  Reason: Not in endgame phase\n";
            }
            else if (popcount(whiteBishops) != 1 || popcount(blackBishops) != 1)
            {
                ss << "  Reason: Not exactly one bishop per side\n";
            }
        }
        ss << "\n";
    }
    ss << "\n";
    return ss.str();
}

}

}  // namespace Alexander