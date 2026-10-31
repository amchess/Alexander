#include "../../evaluate.h"
#include "trace.h"
#include "../evaluate_handicap.h"
#include "../../uci.h"
#include "../../movegen.h"
#include <sstream>
#include <iomanip>
#include "mobility.h"

namespace Alexander {
namespace Eval {

using namespace Trace;

std::string analyze_mobility(const Position& pos, int phase) {
    (void)phase; (void)pos; // Silence unused variable warnings
    std::stringstream ss;
    ss << "=== MOBILITY SUBELEMENTS ===\n";

    // 1. ENGINE NUMERICAL SCORES
    ScoreForClassical w_mob = Trace::scores[MOBILITY][WHITE];
    ScoreForClassical b_mob = Trace::scores[MOBILITY][BLACK];
    ScoreForClassical net_mob = w_mob - b_mob;
    
    int mg_net = mg_value(net_mob);

    ss << "Piece Mobility Evaluation (Safe available squares):\n"
       << "  White : " << std::setw(4) << mg_value(w_mob) << " (MG) / " << std::setw(4) << eg_value(w_mob) << " (EG)\n"
       << "  Black : " << std::setw(4) << mg_value(b_mob) << " (MG) / " << std::setw(4) << eg_value(b_mob) << " (EG)\n"
       << "  Net   : " << std::setw(4) << mg_net << " (MG) / " << std::setw(4) << eg_value(net_mob) << " (EG)\n\n";

    // 2. KASPAROV PRINCIPLE (Preserved and extracted for synthesis)
    ScoreForClassical total_net = scores[TOTAL][WHITE] - scores[TOTAL][BLACK];
    Value total_eval = (mg_value(total_net) * phase + eg_value(total_net) * (PHASE_MIDGAME - phase)) / PHASE_MIDGAME;

    bool has_eval = (total_eval != 0);
    Color stronger_side = WHITE;
    std::string attack_side = "";
    std::string counterattack_side = "";

    if (has_eval) {
        ss << "Mobility by area based on the Kasparov Principle:\n";

        int white_queen_side = Trace::mobility_area_counts[WHITE][Trace::QUEEN_SIDE];
        int white_center     = Trace::mobility_area_counts[WHITE][Trace::CENTER];
        int white_king_side  = Trace::mobility_area_counts[WHITE][Trace::KING_SIDE];
        int black_queen_side = Trace::mobility_area_counts[BLACK][Trace::QUEEN_SIDE];
        int black_center     = Trace::mobility_area_counts[BLACK][Trace::CENTER];
        int black_king_side  = Trace::mobility_area_counts[BLACK][Trace::KING_SIDE];

        int queen_side_diff = white_queen_side - black_queen_side;
        int center_diff     = white_center - black_center;
        int king_side_diff  = white_king_side - black_king_side;

        for (int c = WHITE; c <= BLACK; ++c) {
            ss << "    " << (c == WHITE ? "White: " : "Black: ");
            ss << "Queen Side(" << (c == WHITE ? white_queen_side : black_queen_side) << ") ";
            ss << "Center(" << (c == WHITE ? white_center : black_center) << ") ";
            ss << "King Side(" << (c == WHITE ? white_king_side : black_king_side) << ")\n";
        }

        ss << "    Difference (White-Black): Queen Side(" << queen_side_diff << ") Center("
           << center_diff << ") King Side(" << king_side_diff << ")\n";

        stronger_side = total_eval > 0 ? WHITE : BLACK;
        std::string stronger_color = stronger_side == WHITE ? "White" : "Black";
        std::string weaker_color   = stronger_side == WHITE ? "Black" : "White";

        attack_side = (stronger_side == WHITE) ? "king side" : "queen side";

        ss << "  - " << stronger_color << " has the initiative. Kasparov Principle: try to attack on the " << attack_side << ".\n";

        int weak_advantage_queen, weak_advantage_center, weak_advantage_king;
        if (stronger_side == WHITE) {
            weak_advantage_queen  = -queen_side_diff;
            weak_advantage_center = -center_diff;
            weak_advantage_king   = -king_side_diff;
        } else {
            weak_advantage_queen  = queen_side_diff;
            weak_advantage_center = center_diff;
            weak_advantage_king   = king_side_diff;
        }

        int max_advantage = std::max({weak_advantage_queen, weak_advantage_center, weak_advantage_king});
        
        if (max_advantage == weak_advantage_queen) counterattack_side = "queen side";
        else if (max_advantage == weak_advantage_center) counterattack_side = "center";
        else counterattack_side = "king side";

        ss << "  - " << weaker_color << " should counter-attack on the " << counterattack_side
           << " if possible, or defend the " << attack_side << " otherwise.\n";
    }

    // 3. DYNAMIC OTB STRATEGIC SYNTHESIS
    ss << "\n--- OTB STRATEGIC SYNTHESIS ---\n";
    
    // Theory Branch
    if (mg_net > 30 || mg_net < -30) {
        ss << "  * Theory : Mobility is the lifeblood of the initiative. A massive mobility advantage means your pieces can pivot across the board to create localized overloads faster than the opponent can rearrange their defenses.\n";
    } else if (has_eval) {
        ss << "  * Theory : The Kasparov Principle dictates that you must play on the flank where your pieces possess the highest relative mobility. Attacking on your weaker flank is a positional blunder.\n";
    } else {
        ss << "  * Theory : When piece mobility is equal, the game slows down. Prophylaxis and structural maneuvering take precedence over direct, sweeping attacks.\n";
    }

    // Action Branch
    ss << "  * Action : ";
    if (has_eval) {
        if (stronger_side == WHITE) {
            ss << "White dictates the game. Launch your primary offensive on the " << attack_side << " where your pieces naturally flow. Black must seek active counterplay on the " << counterattack_side << " rather than passively waiting to be crushed.\n";
        } else {
            ss << "Black dictates the game. Launch your primary offensive on the " << attack_side << " where your pieces naturally flow. White must seek active counterplay on the " << counterattack_side << " rather than passively waiting to be crushed.\n";
        }
    } else {
        ss << "Maintain flexibility. Centralize your pieces to maximize their safe available squares. Do not aggressively commit to a flank attack until a structural imbalance is created.\n";
    }

    ss << "\n";
    return ss.str();
}

} // namespace Eval
} // namespace Alexander