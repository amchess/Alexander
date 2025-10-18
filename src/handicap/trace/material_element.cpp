#include "../../evaluate.h"
#include "trace.h"
#include "material_element.h"
#include "../evaluate_handicap.h"
#include "../../uci.h"
#include "../../movegen.h"
#include "../../bitboard.h"
#include <sstream>


namespace Alexander {

namespace Eval {

std::string analyze_material(const Position& pos, int phase) {
    std::stringstream ss;
    Trace::Helpers::append_term_row(ss, pos, phase, "Material", Trace::MATERIAL);
    ss << "=== MATERIAL SUBELEMENTS ===\n";

    // Bishop Pair - sottoelemento di Material
    ss << "Bishop Pair:\n";
    bool whiteHasPair = popcount(pos.pieces(WHITE, BISHOP)) >= 2;
    bool blackHasPair = popcount(pos.pieces(BLACK, BISHOP)) >= 2;

    if (whiteHasPair && blackHasPair)
        ss << "  Both sides have bishop pair\n";
    else if (whiteHasPair)
        ss << "  White has bishop pair\n";
    else if (blackHasPair)
        ss << "  Black has bishop pair\n";
    else
        ss << "  No bishop pair\n";

    ss << "  White bishops: " << popcount(pos.pieces(WHITE, BISHOP)) << "\n";
    ss << "  Black bishops: " << popcount(pos.pieces(BLACK, BISHOP)) << "\n";

    if (whiteHasPair || blackHasPair)
    {
        ss << "  Bishop pair advantages:\n";
        ss << "    - Controls both color complexes\n";
        ss << "    - Enhanced coordination and mobility\n";
        ss << "    - Stronger in open positions\n";
    }
    ss << "\n";
    return ss.str();
}
}

}  // namespace Alexander