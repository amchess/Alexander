#include "../../evaluate.h"
#include "trace.h"
#include "../evaluate_handicap.h"
#include "../../uci.h"
#include "../../movegen.h"
#include <sstream>
#include <iomanip>
#include <algorithm> // for std::max
#include "mobility.h"

namespace Alexander {

using namespace Trace;

namespace Eval {

namespace {

const char* AreaNames[AREA_NB] = {"queen side", "center", "king side"};

// [SC:trace] Area where 'c' has the largest mobility surplus over the opponent
Area best_area(Color c) {
    int  best  = -1000;
    Area bestA = CENTER;
    for (int a = Trace::QUEEN_SIDE; a < AREA_NB; ++a)
    {
        int d = mobility_area_counts[c][a] - mobility_area_counts[~c][a];
        if (d > best)
        {
            best  = d;
            bestA = Area(a);
        }
    }
    return bestA;
}

}  // namespace

std::string analyze_mobility(const Position& pos, int phase) {
    std::stringstream ss;

    // =========================================================================
    // BLOCK 1: ORIGINAL OUTPUT
    // =========================================================================
    Trace::Helpers::append_term_row(ss, pos, phase, "Mobility", MOBILITY);

    // [SC:trace] Who has the initiative: the side ahead in the evaluation; when the evaluation
    // is about level (|eval| < 10 cp) the side with the larger mobility. The Kasparov Principle
    // is then applied for real: each side attacks where ITS mobility surplus is largest (the
    // old code sent White to the king side and Black to the queen side unconditionally).
    ScoreForClassical total_net = scores[TOTAL][WHITE] - scores[TOTAL][BLACK];
    Value             total_eval =
      (mg_value(total_net) * phase + eg_value(total_net) * (PHASE_MIDGAME - phase))
      / PHASE_MIDGAME;
    const int  mobNet    = Helpers::interpolate(scores[MOBILITY][WHITE] - scores[MOBILITY][BLACK], phase);
    const bool level     = std::abs(UCIEngine::to_cp(total_eval, pos)) < 10;
    const Color stronger = level ? (mobNet >= 0 ? WHITE : BLACK) : (total_eval > 0 ? WHITE : BLACK);
    const Color weaker   = ~stronger;
    const Area  attackA  = best_area(stronger);
    const Area  counterA = best_area(weaker);
    const std::string strongerName = stronger == WHITE ? "White" : "Black";
    const std::string weakerName   = weaker == WHITE ? "White" : "Black";

    auto print_areas = [&](const std::string& indent) {
        ss << "Mobility by area based on the Kasparov Principle:\n";
        for (Color c : {WHITE, BLACK})
            ss << indent << (c == WHITE ? "White: " : "Black: ") << "Queen Side("
               << mobility_area_counts[c][Trace::QUEEN_SIDE] << ") Center("
               << mobility_area_counts[c][Trace::CENTER] << ") King Side("
               << mobility_area_counts[c][Trace::KING_SIDE] << ")\n";
        ss << indent << "Difference (White-Black): Queen Side("
           << mobility_area_counts[WHITE][Trace::QUEEN_SIDE] - mobility_area_counts[BLACK][Trace::QUEEN_SIDE]
           << ") Center(" << mobility_area_counts[WHITE][Trace::CENTER] - mobility_area_counts[BLACK][Trace::CENTER]
           << ") King Side("
           << mobility_area_counts[WHITE][Trace::KING_SIDE] - mobility_area_counts[BLACK][Trace::KING_SIDE]
           << ")\n";
    };

    ss << "=== MOBILITY SUBELEMENTS ===\n";
    print_areas("    ");
    ss << "  " << strongerName << " has the initiative"
       << (level ? " (level evaluation, more mobile side)" : "")
       << ". Kasparov Principle: try to attack on the " << AreaNames[attackA] << ";\n";
    ss << "  " << weakerName << " should attack on the " << AreaNames[counterA]
       << ", if possible or defend on the " << AreaNames[attackA] << " otherwise.\n";
    ss << "\n";

    // =========================================================================
    // BLOCK 2: UPDATED OUTPUT (ADVANCED MOBILITY MODULES)
    // =========================================================================
    ss << "================================================================\n";
    ss << "=== ADVANCED MOBILITY MODULES ===\n\n";

    Trace::Helpers::append_score_block(ss, pos, "Piece Mobility Evaluation (Safe available squares)", Trace::MOBILITY);
    const int mobNetCp = UCIEngine::to_cp(mobNet, pos);

    ss << "--- OTB STRATEGIC SYNTHESIS ---\n";
    if (std::abs(mobNetCp) > 15)
        ss << "  * Theory : Mobility is the lifeblood of the initiative: the more mobile side can "
              "switch its pieces from one wing to the other faster than the opponent can "
              "regroup its defense.\n";
    else
        ss << "  * Theory : The Kasparov Principle: play on the flank where your pieces have the "
              "highest relative mobility; attacking on your weaker flank is a positional error.\n";

    ss << "  * Action : " << strongerName << " should concentrate the pieces on the "
       << AreaNames[attackA] << " (largest mobility surplus); " << weakerName
       << " should seek counterplay on the " << AreaNames[counterA]
       << (counterA == attackA ? " by contesting the same sector" : "")
       << " rather than waiting passively.\n\n";

    return ss.str();
}

} // namespace Eval
} // namespace Alexander
