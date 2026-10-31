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

std::string analyze_bishops(const Position& pos, int phase, const std::string& gamePhase, Value v_white) {
    (void)phase; // Silence the unused parameter warning

    std::stringstream ss;
    ss << "=== BISHOP SUBELEMENTS ===\n";

    // 1. ENGINE NUMERICAL SCORES
    ScoreForClassical w_bish = Trace::scores[BISHOP][WHITE];
    ScoreForClassical b_bish = Trace::scores[BISHOP][BLACK];
    ScoreForClassical net_bish = w_bish - b_bish;
    
    ss << "Bishop Positional Scores (Mobility, Outposts, X-Rays, Blockages):\n"
       << "  White : " << std::setw(4) << mg_value(w_bish) << " (MG) / " << std::setw(4) << eg_value(w_bish) << " (EG)\n"
       << "  Black : " << std::setw(4) << mg_value(b_bish) << " (MG) / " << std::setw(4) << eg_value(b_bish) << " (EG)\n"
       << "  Net   : " << std::setw(4) << mg_value(net_bish) << " (MG) / " << std::setw(4) << eg_value(net_bish) << " (EG)\n\n";

    bool is_ocb_endgame = false;
    bool dvoretsky_violation = false;
    Color weakerSide = WHITE;
    int oppositeColorCount = 0;

    if (Alexander::Eval::handicapConfig.pawnsToEvaluate) {
        ss << "Structural Analysis (Opposite-Colored Bishops):\n";
        
        auto square_color = [](Square s) -> Color {
            return (file_of(s) + rank_of(s)) % 2 == 0 ? BLACK : WHITE;
        };
        auto color_name = [](Color c) -> std::string { return c == WHITE ? "light" : "dark"; };

        bool isEndgame = (gamePhase == "Endgame" || gamePhase == "Transition to endgame");
        Bitboard whiteBishops = pos.pieces(WHITE, BISHOP);
        Bitboard blackBishops = pos.pieces(BLACK, BISHOP);

        if (isEndgame && popcount(whiteBishops) == 1 && popcount(blackBishops) == 1) {
            Square wBishopSq = lsb(whiteBishops);
            Square bBishopSq = lsb(blackBishops);

            if (square_color(wBishopSq) != square_color(bBishopSq)) {
                is_ocb_endgame = true;
                ss << "  - Opposite-colored bishops endgame detected.\n";
                
                weakerSide = (v_white < 0) ? WHITE : BLACK;
                ss << "  - The defending side (eval-based) is: " << (weakerSide == WHITE ? "White" : "Black") << "\n";

                Square weakerBishopSq = (weakerSide == WHITE) ? wBishopSq : bBishopSq;
                Color bishopColor = square_color(weakerBishopSq);
                ss << "  - Defender's bishop operates on " << color_name(bishopColor) << " squares.\n";

                Bitboard weakerPawns = pos.pieces(weakerSide, PAWN);
                Bitboard pawnsOnSameColor = 0;
                Bitboard pawnsOnOppositeColor = 0;

                while (weakerPawns) {
                    Square pawnSq = pop_lsb(weakerPawns);
                    if (square_color(pawnSq) == bishopColor) pawnsOnSameColor |= pawnSq;
                    else pawnsOnOppositeColor |= pawnSq;
                }

                oppositeColorCount = popcount(pawnsOnOppositeColor);
                int sameColorCount = popcount(pawnsOnSameColor);

                ss << "  - Defender's Pawn Structure:\n"
                   << "      On same color as bishop: " << sameColorCount << " pawns.\n"
                   << "      On opposite color      : " << oppositeColorCount << " pawns.\n";

                // Dvoretsky Check
                if (oppositeColorCount > 0) {
                    dvoretsky_violation = true;
                    ss << "  - Dvoretsky Rule Status: VIOLATION. Defender has " << oppositeColorCount 
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
    }

    // DYNAMIC OTB STRATEGIC SYNTHESIS
    ss << "\n--- OTB STRATEGIC SYNTHESIS ---\n";
    
    // Theory Branch
    if (is_ocb_endgame) {
        ss << "  * Theory : Opposite-Colored Bishop endgames are notoriously drawish because neither side can attack the squares the other bishop defends. The attacker MUST create passed pawns on opposite sides of the board to stretch the defender.\n";
    } else {
        ss << "  * Theory : Bishops act as long-range snipers. Their value scales with the openness of the board and their ability to operate on unopposed color complexes.\n";
    }

    // Action Branch
    ss << "  * Action : ";
    if (is_ocb_endgame && dvoretsky_violation) {
        ss << (weakerSide == WHITE ? "White" : "Black") << " is violating Dvoretsky's Rule. The defender MUST immediately push their pawns onto the color of their OWN Bishop (" << oppositeColorCount << " pawns are currently vulnerable on the wrong color).\n";
    } else if (is_ocb_endgame && !dvoretsky_violation) {
        ss << (weakerSide == WHITE ? "White" : "Black") << " has correctly established a Dvoretsky Blockade. The attacker must sacrifice a pawn to create a second weakness on the other side of the board to break the fortress.\n";
    } else if (popcount(pos.pieces(WHITE, BISHOP)) >= 2) {
        ss << "White holds the Bishop Pair. White must aggressively open the center by trading pawns. Black must keep the pawn structure closed and restrict diagonals.\n";
    } else if (popcount(pos.pieces(BLACK, BISHOP)) >= 2) {
        ss << "Black holds the Bishop Pair. Black must aggressively open the center by trading pawns. White must keep the pawn structure closed and restrict diagonals.\n";
    } else {
        ss << "Place your pawns on the OPPOSITE color of your active Bishop to give it open diagonals. If the opponent has a strong Bishop, place your pawns on its color to blunt its scope.\n";
    }

    ss << "\n";
    return ss.str();
}

} // namespace Eval
} // namespace Alexander