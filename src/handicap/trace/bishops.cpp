#include "../../evaluate.h"
#include "trace.h"
#include "bishops.h"
#include "../evaluate_handicap.h"
#include "../../uci.h"
#include "../../movegen.h"
#include "../../bitboard.h"
#include <sstream>
#include <iomanip>

namespace Alexander {
namespace Eval {

using namespace Trace;

std::string
analyze_bishops(const Position& pos, int phase, const std::string& gamePhase, Value v_white) {
    (void)phase; // Silence the unused parameter warning
    std::stringstream ss;
    
    // =========================================================================
    // BLOCCO 1: OUTPUT ORIGINALE INTATTO AL 100%
    // =========================================================================
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
        
        // =========================================================================
        // BLOCCO 2: OUTPUT AGGIORNATO (MASTER REPORT / ADVANCED MODULES)
        // =========================================================================
        ss << "================================================================\n";
        ss << "=== ADVANCED BISHOP MODULES ===\n\n";

        // 1. ENGINE NUMERICAL SCORES
        ScoreForClassical w_bish = Trace::scores[BISHOP][WHITE];
        ScoreForClassical b_bish = Trace::scores[BISHOP][BLACK];
        ScoreForClassical net_bish = w_bish - b_bish;
        
        ss << "Bishop Positional Scores (Mobility, Outposts, X-Rays, Blockages):\n"
           << "  White : " << std::setw(4) << mg_value(w_bish) << " (MG) / " << std::setw(4) << eg_value(w_bish) << " (EG)\n"
           << "  Black : " << std::setw(4) << mg_value(b_bish) << " (MG) / " << std::setw(4) << eg_value(b_bish) << " (EG)\n"
           << "  Net   : " << std::setw(4) << mg_value(net_bish) << " (MG) / " << std::setw(4) << eg_value(net_bish) << " (EG)\n\n";

        bool is_ocb_endgame_adv = false;
        bool dvoretsky_violation_adv = false;
        Color weakerSideAdv = WHITE;
        int oppositeColorCountAdv = 0;

        ss << "Structural Analysis (Opposite-Colored Bishops):\n";
        
        if (isEndgame && popcount(whiteBishops) == 1 && popcount(blackBishops) == 1) {
            Square wBishopSq = lsb(whiteBishops);
            Square bBishopSq = lsb(blackBishops);

            if (square_color(wBishopSq) != square_color(bBishopSq)) {
                is_ocb_endgame_adv = true;
                ss << "  - Opposite-colored bishops endgame detected.\n";
                
                weakerSideAdv = (v_white < 0) ? WHITE : BLACK;
                ss << "  - The defending side (eval-based) is: " << (weakerSideAdv == WHITE ? "White" : "Black") << "\n";

                Square weakerBishopSq = (weakerSideAdv == WHITE) ? wBishopSq : bBishopSq;
                Color bishopColor = square_color(weakerBishopSq);
                ss << "  - Defender's bishop operates on " << color_name(bishopColor) << " squares.\n";

                Bitboard weakerPawns = pos.pieces(weakerSideAdv, PAWN);
                Bitboard pawnsOnSameColor = 0;
                Bitboard pawnsOnOppositeColor = 0;

                while (weakerPawns) {
                    Square pawnSq = pop_lsb(weakerPawns);
                    if (square_color(pawnSq) == bishopColor) pawnsOnSameColor |= pawnSq;
                    else pawnsOnOppositeColor |= pawnSq;
                }

                oppositeColorCountAdv = popcount(pawnsOnOppositeColor);
                int sameColorCount = popcount(pawnsOnSameColor);

                ss << "  - Defender's Pawn Structure:\n"
                   << "      On same color as bishop: " << sameColorCount << " pawns.\n"
                   << "      On opposite color      : " << oppositeColorCountAdv << " pawns.\n";

                // Dvoretsky Check
                if (oppositeColorCountAdv > 0) {
                    dvoretsky_violation_adv = true;
                    ss << "  - Dvoretsky Rule Status: VIOLATION. Defender has " << oppositeColorCountAdv 
                       << " pawn(s) vulnerable on the opponent's bishop color.\n";
                } else {
                    ss << "  - Dvoretsky Rule Status: OPTIMAL. All defending pawns form a blockade on the correct color.\n";
                }
            } else {
                ss << "  - Bishops are on the same color complex.\n";
            }
        } else if (isEndgame && (popcount(whiteBishops) == 1 || popcount(blackBishops) == 1)) {
            ss << "  - Single bishop endgame (not an OCB endgame).\n";
        } else {
            ss << "  - Not an opposite-colored bishops endgame.\n";
        }

        // DYNAMIC OTB STRATEGIC SYNTHESIS
        ss << "\n--- OTB STRATEGIC SYNTHESIS ---\n";
        
        // Theory Branch
        if (is_ocb_endgame_adv) {
            ss << "  * Theory : Opposite-Colored Bishop endgames are notoriously drawish because neither side can attack the squares the other bishop defends. The attacker MUST create passed pawns on opposite sides of the board to stretch the defender.\n";
        } else {
            ss << "  * Theory : Bishops act as long-range snipers. Their value scales with the openness of the board and their ability to operate on unopposed color complexes.\n";
        }

        // Action Branch
        ss << "  * Action : ";
        if (is_ocb_endgame_adv && dvoretsky_violation_adv) {
            ss << (weakerSideAdv == WHITE ? "White" : "Black") << " is violating Dvoretsky's Rule. The defender MUST immediately push their pawns onto the color of their OWN Bishop (" << oppositeColorCountAdv << " pawns are currently vulnerable on the wrong color).\n";
        } else if (is_ocb_endgame_adv && !dvoretsky_violation_adv) {
            ss << (weakerSideAdv == WHITE ? "White" : "Black") << " has correctly established a Dvoretsky Blockade. The attacker must sacrifice a pawn to create a second weakness on the other side of the board to break the fortress.\n";
        } else if (popcount(pos.pieces(WHITE, BISHOP)) >= 2) {
            ss << "White holds the Bishop Pair. White must aggressively open the center by trading pawns. Black must keep the pawn structure closed and restrict diagonals.\n";
        } else if (popcount(pos.pieces(BLACK, BISHOP)) >= 2) {
            ss << "Black holds the Bishop Pair. Black must aggressively open the center by trading pawns. White must keep the pawn structure closed and restrict diagonals.\n";
        } else {
            ss << "Place your pawns on the OPPOSITE color of your active Bishop to give it open diagonals. If the opponent has a strong Bishop, place your pawns on its color to blunt its scope.\n";
        }

        ss << "\n";
    }
    return ss.str();
}

} // namespace Eval
} // namespace Alexander