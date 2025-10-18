#include "../../evaluate.h"
#include "trace.h"
#include "../evaluate_handicap.h"
#include "../../uci.h"
#include "../../movegen.h"
#include <sstream>
#include "mobility.h"

namespace Alexander {

using namespace Trace;

namespace Eval {
std::string analyze_mobility(const Position& pos, int phase) {
    std::stringstream ss;
    Trace::Helpers::append_term_row(ss, pos, phase, "Mobility", MOBILITY);
    // Mobility by area con principio di Kasparov
    auto print_mobility_area_summary = [&]() {
        // Calcola l'attività totale dalla valutazione
        ScoreForClassical total_net = scores[TOTAL][WHITE] - scores[TOTAL][BLACK];
        Value             total_eval =
          (mg_value(total_net) * phase + eg_value(total_net) * (PHASE_MIDGAME - phase))
          / PHASE_MIDGAME;

        // Mostra solo se l'attività totale è diversa da zero
        if (total_eval == 0)
        {
            return;
        }

        ss << "Mobility by area based on the Kasparov Principle:\n";

        // Calcola la mobilità per tutte e tre le aree
        int white_queen_side = Trace::mobility_area_counts[WHITE][Trace::QUEEN_SIDE];
        int white_center = Trace::mobility_area_counts[WHITE][Trace::CENTER];
        int white_king_side = Trace::mobility_area_counts[WHITE][Trace::KING_SIDE];
        int black_queen_side = Trace::mobility_area_counts[BLACK][Trace::QUEEN_SIDE];
        int black_center = Trace::mobility_area_counts[BLACK][Trace::CENTER];
        int black_king_side = Trace::mobility_area_counts[BLACK][Trace::KING_SIDE];

        // Differenze per area (bianco - nero)
        int queen_side_diff = white_queen_side - black_queen_side;
        int center_diff = white_center - black_center;
        int king_side_diff = white_king_side - black_king_side;

        for (int c = WHITE; c <= BLACK; ++c)
        {
            ss << "    " << (c == WHITE ? "White: " : "Black: ");
            ss << "Queen Side(" << (c == WHITE ? white_queen_side : black_queen_side) << ") ";
            ss << "Center(" << (c == WHITE ? white_center : black_center) << ") ";
            ss << "King Side(" << (c == WHITE ? white_king_side : black_king_side) << ")\n";
        }

        // Mostra le differenze
        ss << "    Difference (White-Black): Queen Side(" << queen_side_diff 
           << ") Center(" << center_diff 
           << ") King Side(" << king_side_diff << ")\n";

        // Applica il principio di Kasparov
        Color       stronger_side  = total_eval > 0 ? WHITE : BLACK;
        std::string stronger_color = stronger_side == WHITE ? "White" : "Black";
        std::string weaker_color   = stronger_side == WHITE ? "Black" : "White";

        // Determina l'area di attacco per il lato forte
        std::string attack_side;
        if (stronger_side == WHITE) {
            attack_side = "king side";
        } else {
            attack_side = "queen side";
        }

        std::string kasparov_advice = "Kasparov Principle: try to attack on the " + attack_side + ";";
        ss << "  " << stronger_color << " has the initiative. " << kasparov_advice << "\n";

        // Per il lato debole: determina il settore di contrattacco in base al vantaggio relativo
        int weak_advantage_queen, weak_advantage_center, weak_advantage_king;
        if (stronger_side == WHITE) {
            // Il lato debole è il Nero: il vantaggio del Nero è -differenza
            weak_advantage_queen = -queen_side_diff;
            weak_advantage_center = -center_diff;
            weak_advantage_king = -king_side_diff;
        } else {
            // Il lato debole è il Bianco: il vantaggio è la differenza
            weak_advantage_queen = queen_side_diff;
            weak_advantage_center = center_diff;
            weak_advantage_king = king_side_diff;
        }

        // Trova l'area con il massimo vantaggio per il lato debole
        int max_advantage = std::max({weak_advantage_queen, weak_advantage_center, weak_advantage_king});
        std::string counterattack_side;
        if (max_advantage == weak_advantage_queen) {
            counterattack_side = "queen side";
        } else if (max_advantage == weak_advantage_center) {
            counterattack_side = "center";
        } else {
            counterattack_side = "king side";
        }

        // Raccomandazione per il lato più debole
        ss << "  " << weaker_color << " should attack on the " << counterattack_side 
           << ", if possible or defend on the " << attack_side << " otherwise.\n";
    };
    ss << "=== MOBILITY SUBELEMENTS ===\n";
    print_mobility_area_summary();
    ss << "\n";
    return ss.str();
}
}

}  // namespace Alexander