#include "../../evaluate.h"
#include "trace.h"
#include "imbalances.h"
#include "../evaluate_handicap.h"
#include "../../uci.h"
#include "../../movegen.h"
#include "../../bitboard.h"
#include <sstream>
#include <vector>
#include <iomanip>

namespace Alexander {
namespace Eval {

using namespace Trace;

std::string analyze_imbalances(const Position& pos, int phase) {
    (void)phase; // Silence the unused parameter warning
    std::stringstream ss;
    ss << "=== MATERIAL IMBALANCES SUBELEMENTS ===\n";

    // 1. ENGINE NUMERICAL SCORES
    ScoreForClassical w_imb = Trace::scores[Trace::IMBALANCE][WHITE];
    ScoreForClassical b_imb = Trace::scores[Trace::IMBALANCE][BLACK];
    ScoreForClassical net_imb = w_imb - b_imb;
    
    int mg_net = mg_value(net_imb);
    int eg_net = eg_value(net_imb);

    ss << "Piece Synergy Evaluation:\n"
       << "  White : " << std::setw(4) << mg_value(w_imb) << " (MG) / " << std::setw(4) << eg_value(w_imb) << " (EG)\n"
       << "  Black : " << std::setw(4) << mg_value(b_imb) << " (MG) / " << std::setw(4) << eg_value(b_imb) << " (EG)\n"
       << "  Net   : " << std::setw(4) << mg_net << " (MG) / " << std::setw(4) << eg_net << " (EG)\n\n";

    // Helper to determine the skew magnitude
    auto get_skew = [](int val) {
        int abs_v = std::abs(val);
        if (abs_v > 50) return "Heavily skewed towards";
        if (abs_v > 15) return "Moderately skewed towards";
        return "Slightly favors";
    };

    ss << "Synergy Verdict (Skew Analysis):\n";
    if (mg_net > 0) ss << "  - Middlegame : " << get_skew(mg_net) << " White (+" << mg_net << ").\n";
    else if (mg_net < 0) ss << "  - Middlegame : " << get_skew(mg_net) << " Black (" << mg_net << ").\n";
    else ss << "  - Middlegame : Piece synergy is perfectly balanced.\n";

    if (eg_net > 0) ss << "  - Endgame    : " << get_skew(eg_net) << " White (+" << eg_net << ").\n";
    else if (eg_net < 0) ss << "  - Endgame    : " << get_skew(eg_net) << " Black (" << eg_net << ").\n";
    else ss << "  - Endgame    : Piece synergy remains balanced.\n\n";

    // 2. DYNAMIC STRUCTURAL DETECTION (Algorithmic Cancellation)
    int dQueens  = popcount(pos.pieces(WHITE, QUEEN))  - popcount(pos.pieces(BLACK, QUEEN));
    int dRooks   = popcount(pos.pieces(WHITE, ROOK))   - popcount(pos.pieces(BLACK, ROOK));
    int dBishops = popcount(pos.pieces(WHITE, BISHOP)) - popcount(pos.pieces(BLACK, BISHOP));
    int dKnights = popcount(pos.pieces(WHITE, KNIGHT)) - popcount(pos.pieces(BLACK, KNIGHT));
    int dPawns   = popcount(pos.pieces(WHITE, PAWN))   - popcount(pos.pieces(BLACK, PAWN));

    // Calculate strict surplus for each side
    int w_extra_q = std::max(0, dQueens);
    int b_extra_q = std::max(0, -dQueens);
    int w_extra_r = std::max(0, dRooks);
    int b_extra_r = std::max(0, -dRooks);
    int w_extra_b = std::max(0, dBishops);
    int b_extra_b = std::max(0, -dBishops);
    int w_extra_n = std::max(0, dKnights);
    int b_extra_n = std::max(0, -dKnights);
    int w_extra_p = std::max(0, dPawns);
    int b_extra_p = std::max(0, -dPawns);

    // Helper lambda to construct a natural English string from piece counts
    auto build_piece_str = [](int q, int r, int b, int n, int p) {
        std::vector<std::string> parts;
        if (q == 1) parts.push_back("a Queen"); else if (q > 1) parts.push_back(std::to_string(q) + " Queens");
        if (r == 1) parts.push_back("a Rook");  else if (r > 1) parts.push_back(std::to_string(r) + " Rooks");
        if (b == 1) parts.push_back("a Bishop");else if (b > 1) parts.push_back(std::to_string(b) + " Bishops");
        if (n == 1) parts.push_back("a Knight");else if (n > 1) parts.push_back(std::to_string(n) + " Knights");
        if (p == 1) parts.push_back("a Pawn");  else if (p > 1) parts.push_back(std::to_string(p) + " Pawns");

        if (parts.empty()) return std::string("");
        if (parts.size() == 1) return parts[0];
        
        std::string res = parts[0];
        for (size_t i = 1; i < parts.size() - 1; ++i) res += ", " + parts[i];
        res += " and " + parts.back();
        return res;
    };

    std::string w_str = build_piece_str(w_extra_q, w_extra_r, w_extra_b, w_extra_n, w_extra_p);
    std::string b_str = build_piece_str(b_extra_q, b_extra_r, b_extra_b, b_extra_n, b_extra_p);

    ss << "Detected Structural Imbalances:\n";
    if (!w_str.empty() && !b_str.empty()) {
        ss << "  - White has " << w_str << " for Black's " << b_str << ".\n";
    } else if (!w_str.empty()) {
        ss << "  - White is strictly up material: " << w_str << ".\n";
    } else if (!b_str.empty()) {
        ss << "  - Black is strictly up material: " << b_str << ".\n";
    } else {
        ss << "  - No asymmetric material imbalances detected (Perfectly equal piece types).\n";
    }

    // Bishop Pair specific tracking
    bool w_bp = pos.count<BISHOP>(WHITE) >= 2;
    bool b_bp = pos.count<BISHOP>(BLACK) >= 2;
    if (w_bp && b_bp) ss << "  - Both sides possess the Bishop Pair.\n";
    else if (w_bp) ss << "  - White has the Bishop Pair.\n";
    else if (b_bp) ss << "  - Black has the Bishop Pair.\n";

    // 3. COLOR COMPLEX ANALYSIS (Unopposed Bishops)
    auto square_color = [](Square s) -> Color {
        return (file_of(s) + rank_of(s)) % 2 == 0 ? BLACK : WHITE;
    };
    
    bool w_has_light = false, w_has_dark = false;
    bool b_has_light = false, b_has_dark = false;

    Bitboard w_b_temp = pos.pieces(WHITE, BISHOP);
    while (w_b_temp) {
        Square s = pop_lsb(w_b_temp);
        if (square_color(s) == WHITE) w_has_light = true;
        else w_has_dark = true;
    }

    Bitboard b_b_temp = pos.pieces(BLACK, BISHOP);
    while (b_b_temp) {
        Square s = pop_lsb(b_b_temp);
        if (square_color(s) == WHITE) b_has_light = true;
        else b_has_dark = true;
    }

    ss << "\nColor Complex Analysis (Unopposed Bishops):\n";
    bool unopposed_bishop_detected = false;

    if (w_has_light && !b_has_light) {
        ss << "  - White has an unopposed light-squared bishop.\n"
           << "    Advice for Black: Put your pawns on light squares in the middlegame to obstruct it, and play actively on the dark squares.\n";
        unopposed_bishop_detected = true;
    }
    if (w_has_dark && !b_has_dark) {
        ss << "  - White has an unopposed dark-squared bishop.\n"
           << "    Advice for Black: Put your pawns on dark squares in the middlegame to obstruct it, and play actively on the light squares.\n";
        unopposed_bishop_detected = true;
    }
    if (b_has_light && !w_has_light) {
        ss << "  - Black has an unopposed light-squared bishop.\n"
           << "    Advice for White: Put your pawns on light squares in the middlegame to obstruct it, and play actively on the dark squares.\n";
        unopposed_bishop_detected = true;
    }
    if (b_has_dark && !w_has_dark) {
        ss << "  - Black has an unopposed dark-squared bishop.\n"
           << "    Advice for White: Put your pawns on dark squares in the middlegame to obstruct it, and play actively on the light squares.\n";
        unopposed_bishop_detected = true;
    }

    if (!unopposed_bishop_detected) {
        ss << "  - No unopposed color complexes detected (Bishops are matched on colors, or neither side has Bishops).\n";
    }

    // 4. STRATEGIC ABLATIONS (What-If Scenarios based on MG/EG scaling)
    ss << "\nStrategic Ablations (Simplification Scenarios):\n";
    
    // Queen Trade Ablation
    bool has_queens = pos.count<QUEEN>(WHITE) > 0 && pos.count<QUEEN>(BLACK) > 0;
    if (has_queens) {
        int q_delta = eg_net - mg_net;
        if (q_delta > 10) {
            ss << "  - Queen Trade (Transition): Favors WHITE. Trading Queens shifts the synergy balance towards White by +" << q_delta << " points.\n";
        } else if (q_delta < -10) {
            ss << "  - Queen Trade (Transition): Favors BLACK. Trading Queens shifts the synergy balance towards Black by " << std::abs(q_delta) << " points.\n";
        } else {
            ss << "  - Queen Trade (Transition): Neutral. A Queen trade does not significantly alter the piece synergy balance.\n";
        }
    } else {
        ss << "  - Queen Trade: N/A (Queens are already off the board).\n";
    }

    // Imbalance Removal Ablation
    if (mg_net > 20) {
        ss << "  - Imbalance Removal: Favors BLACK. White holds a synergy advantage (+" << mg_net << "). Black should seek to trade off the mismatched pieces to restore symmetry and neutralize this advantage.\n";
    } else if (mg_net < -20) {
        ss << "  - Imbalance Removal: Favors WHITE. Black holds a synergy advantage (+" << std::abs(mg_net) << " for Black). White should seek to trade off the mismatched pieces to restore symmetry and neutralize this advantage.\n";
    } else {
        ss << "  - Imbalance Removal: Neutral. The structural tension is manageable; neither side is forced to seek asymmetric trades.\n";
    }

    // 5. MATERIAL DETAIL
    const int pawnValue   = 1;
    const int knightValue = 3;
    const int bishopValue = 3;
    const int rookValue   = 5;
    const int queenValue  = 9;

    int whiteMaterial = pawnValue * popcount(pos.pieces(WHITE, PAWN))
                      + knightValue * popcount(pos.pieces(WHITE, KNIGHT))
                      + bishopValue * popcount(pos.pieces(WHITE, BISHOP))
                      + rookValue * popcount(pos.pieces(WHITE, ROOK))
                      + queenValue * popcount(pos.pieces(WHITE, QUEEN));

    int blackMaterial = pawnValue * popcount(pos.pieces(BLACK, PAWN))
                      + knightValue * popcount(pos.pieces(BLACK, KNIGHT))
                      + bishopValue * popcount(pos.pieces(BLACK, BISHOP))
                      + rookValue * popcount(pos.pieces(BLACK, ROOK))
                      + queenValue * popcount(pos.pieces(BLACK, QUEEN));

    ss << "\nMaterial Detail:\n";
    ss << "  White: " << whiteMaterial << " pts (P:" << popcount(pos.pieces(WHITE, PAWN))
       << " N:" << popcount(pos.pieces(WHITE, KNIGHT))
       << " B:" << popcount(pos.pieces(WHITE, BISHOP)) << " R:" << popcount(pos.pieces(WHITE, ROOK))
       << " Q:" << popcount(pos.pieces(WHITE, QUEEN)) << ")\n";
    ss << "  Black: " << blackMaterial << " pts (P:" << popcount(pos.pieces(BLACK, PAWN))
       << " N:" << popcount(pos.pieces(BLACK, KNIGHT))
       << " B:" << popcount(pos.pieces(BLACK, BISHOP)) << " R:" << popcount(pos.pieces(BLACK, ROOK))
       << " Q:" << popcount(pos.pieces(BLACK, QUEEN)) << ")\n\n";

    // 6. DYNAMIC OTB STRATEGIC SYNTHESIS
    ss << "--- OTB STRATEGIC SYNTHESIS ---\n";
    
    // Theory Branch
    if (std::abs(whiteMaterial - blackMaterial) >= 3) {
        ss << "  * Theory : When a significant raw material advantage exists, complex positional nuances take a backseat to technical conversion.\n";
    } else if (w_bp || b_bp) {
        ss << "  * Theory : The Bishop Pair is a long-term asset that multiplies in strength as the board empties and pawns are traded.\n";
    } else if (unopposed_bishop_detected) {
        ss << "  * Theory : An unopposed Bishop acts as an absolute dictator over its color complex, essentially giving you an extra piece on those squares.\n";
    } else {
        ss << "  * Theory : Material is balanced. The game will be decided by piece coordination (synergy) rather than raw point values.\n";
    }

    // Action Branch
    ss << "  * Action : ";
    if (whiteMaterial >= blackMaterial + 3) {
        ss << "White is up material. Trade pieces to reduce Black's counterplay, but keep pawns on the board to ensure a winning endgame.\n";
    } else if (blackMaterial >= whiteMaterial + 3) {
        ss << "Black is up material. Trade pieces to reduce White's counterplay, but keep pawns on the board to ensure a winning endgame.\n";
    } else if (w_bp && !b_bp) {
        ss << "White holds the Bishop Pair. Force the center open to maximize their scope. Black must seek to lock the pawn structure or trade off one of the bishops.\n";
    } else if (b_bp && !w_bp) {
        ss << "Black holds the Bishop Pair. Force the center open to maximize their scope. White must seek to lock the pawn structure or trade off one of the bishops.\n";
    } else if (w_extra_n > b_extra_n && b_extra_b > w_extra_b) {
        ss << "Classic minor piece imbalance (W: Knight, B: Bishop). White must keep the center closed and establish outposts. Black must blow the center open for the Bishop.\n";
    } else if (b_extra_n > w_extra_n && w_extra_b > b_extra_b) {
        ss << "Classic minor piece imbalance (W: Bishop, B: Knight). White must blow the center open for the Bishop. Black must keep the center closed and establish outposts.\n";
    } else if (mg_net > 15) {
        ss << "White has superior piece coordination (+" << mg_net << "). Maintain the tension and avoid piece trades unless they lead to a concrete tactical win.\n";
    } else if (mg_net < -15) {
        ss << "Black has superior piece coordination (" << std::abs(mg_net) << "). Maintain the tension and avoid piece trades unless they lead to a concrete tactical win.\n";
    } else {
        ss << "Piece activity is relatively symmetrical. Focus on improving your worst-placed piece (see Makogonov ranking) and creating pawn structure targets.\n";
    }

    ss << "\n";
    return ss.str();
}

} // namespace Eval
} // namespace Alexander