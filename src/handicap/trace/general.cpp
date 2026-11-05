#include "../../evaluate.h"
#include "general.h"
#include "../evaluate_handicap.h"
#include "../../uci.h"
#include "../../movegen.h"
#include <iomanip>
#include <sstream>
#include "trace.h"

namespace Alexander {
namespace Eval {

namespace {

// [SC:trace] Terms that make up the classical evaluation (the TOTAL entry of the trace table is
// already phase-weighted, so its MG and EG halves cannot be compared with each other).
constexpr int EvalTerms[] = {Trace::MATERIAL, Trace::IMBALANCE, PAWN,           KNIGHT,
                             BISHOP,          ROOK,             QUEEN,          Trace::MOBILITY,
                             KING,            Trace::THREAT,    Trace::PASSED, Trace::SPACE};

// Unweighted sum of the MG and EG halves of all the terms, White point of view
ScoreForClassical raw_total() {
    ScoreForClassical t = SCORE_ZERO;
    for (int term : EvalTerms)
        t += Trace::scores[term][WHITE] - Trace::scores[term][BLACK];
    return t;
}

}  // namespace

std::string generate_general_info(Position&          pos,
                                  Value              v_white,
                                  uint8_t            winProb,
                                  const std::string& shashinZone,
                                  const std::string& gamePhase) {
    std::stringstream ss;

    // =========================================================================
    // BLOCK 1: ORIGINAL OUTPUT, UNCHANGED
    // =========================================================================
    ss << "=== GENERAL INFORMATION ===\n";
    ss << "Final evaluation: " << UCIEngine::to_cp(v_white, pos)
       << " (Win Probability: " << static_cast<int>(winProb) << "%)\n";
    ss << "Shashin Zone: " << shashinZone << "\n";
    ss << "Game Phase: " << gamePhase << "\n\n";

    // =========================================================================
    // BLOCK 2: UPDATED OUTPUT (ADVANCED MODULES)
    // =========================================================================
    const ScoreForClassical tot    = raw_total();
    const Color             us     = pos.side_to_move();
    const int               sign   = us == WHITE ? 1 : -1;
    const int               mg_adv = UCIEngine::to_cp(sign * mg_value(tot), pos);
    const int               eg_adv = UCIEngine::to_cp(sign * eg_value(tot), pos);
    const std::string       usName = us == WHITE ? "White" : "Black";
    const int               wpUs   = us == WHITE ? winProb : 100 - winProb;

    ss << "=== ADVANCED GENERAL MODULES ===\n";
    ss << "Side to move: " << usName << " (Win Probability for " << usName << ": " << wpUs
       << "%)\n";

    // 1. Practical explanation of the Shashin Zone (zone is computed from White's view)
    ss << "Strategic Outlook: ";
    if (winProb >= 76)
        ss << "White is clearly better (Tal zone). The advantage must be converted with forcing "
              "play: calculate concrete variations, keep the initiative, do not allow counterplay.\n";
    else if (winProb >= 51)
        ss << "White holds the initiative (Capablanca-Tal chaos). Increase the pressure with "
              "piece improvement and timely pawn breaks; Black must seek activity, not passivity.\n";
    else if (winProb == 50)
        ss << "The position is balanced (Capablanca). Avoid unnecessary risks, improve the worst "
              "piece and wait for the opponent's inaccuracies before forcing the game.\n";
    else if (winProb >= 25)
        ss << "Black holds the initiative (Capablanca-Petrosian chaos). White must neutralize "
              "the pressure with prophylaxis and trades of Black's most active pieces.\n";
    else
        ss << "White is clearly worse (Petrosian zone). Absolute priority to defense: consolidate "
              "weaknesses, trade attackers, look for a fortress; Black must convert.\n";

    // 2. MG vs EG analysis (same units as the main table; tolerance of 15 cp)
    ss << "Phase Advantage (" << usName << " point of view, sum of all terms): MG " << std::showpos
       << mg_adv << " cp, EG " << eg_adv << " cp" << std::noshowpos << "\n  -> ";
    if (mg_adv > eg_adv + 15)
        ss << usName << " has better prospects in the middlegame than in the endgame. "
           << "Keep the pieces (especially the queens) and use the complexity to create threats.\n";
    else if (eg_adv > mg_adv + 15)
        ss << usName << "'s structure and material shine in the endgame. "
           << "Seek simplifications and transition into a favorable endgame.\n";
    else
        ss << "The balance is stable between middlegame and endgame: " << usName
           << " can choose between attacking now and simplifying, depending on the opponent.\n";
    ss << "\n";

    return ss.str();
}

std::string generate_main_table(const Position& pos) {
    std::stringstream ss;

    // =========================================================================
    // BLOCK 1: DETAILED EVALUATION MATRIX (centipawns)
    // =========================================================================
    auto cp           = [&](Value v) { return UCIEngine::to_cp(v, pos); };
    auto format_score = [&](ScoreForClassical s) {
        std::stringstream tmp;
        tmp << std::setw(5) << cp(mg_value(s)) << " " << std::setw(5) << cp(eg_value(s));
        return tmp.str();
    };

    auto print_row = [&](const std::string& name, int term) {
        ScoreForClassical w = Trace::scores[term][WHITE];
        ScoreForClassical b = Trace::scores[term][BLACK];
        ScoreForClassical t = w - b;  // Net Advantage

        std::stringstream row;
        row << "   | " << std::left << std::setw(16) << name << std::right << " | "
            << format_score(w) << " | " << format_score(b) << " | " << format_score(t) << " |\n";
        return row.str();
    };

    ss << "\n=== DETAILED EVALUATION MATRIX (cp) ===\n";
    ss << "   +------------------+-------------+-------------+-------------+\n"
       << "   | Feature          |    White    |    Black    |  Advantage  |\n"
       << "   |                  |  MG    EG   |  MG    EG   |  MG    EG   |\n"
       << "   +------------------+-------------+-------------+-------------+\n";

    ss << print_row("Material", Trace::MATERIAL);
    ss << print_row("Imbalances", Trace::IMBALANCE);
    ss << print_row("Pawn Structure", PAWN);
    ss << print_row("Passed Pawns", Trace::PASSED);
    ss << "   +------------------+-------------+-------------+-------------+\n";
    ss << print_row("Knights", KNIGHT);
    ss << print_row("Bishops", BISHOP);
    ss << print_row("Rooks", ROOK);
    ss << print_row("Queens", QUEEN);
    ss << "   +------------------+-------------+-------------+-------------+\n";
    ss << print_row("Mobility", Trace::MOBILITY);
    ss << print_row("King Safety", KING);
    ss << print_row("Threats", Trace::THREAT);
    ss << print_row("Space", Trace::SPACE);
    ss << "   +------------------+-------------+-------------+-------------+\n";

    const ScoreForClassical t_tot = raw_total();
    ss << "   | TOTAL (Raw Eval) |             |             | " << format_score(t_tot) << " |\n"
       << "   +------------------+-------------+-------------+-------------+\n"
       << "   (Material and Imbalances are stored as net values in the White column.)\n\n";

    // =========================================================================
    // BLOCK 2: ORIGINAL PER-ELEMENT TABLE (its rows are printed by every module below)
    // =========================================================================
    ss << "\n     ELEMENT     |   MG     EG   Total\n";
    ss << " -------------+--------------------\n";
    return ss.str();
}

}  // namespace Eval
}  // namespace Alexander
