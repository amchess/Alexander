#include "../../evaluate.h"
#include "trace.h"
#include "../evaluate_handicap.h"
#include "../../uci.h"
#include "../../movegen.h"
#include "../../bitboard.h"
#include <sstream>
#include <iomanip>
#include "king_safety_and_threats.h"

namespace Alexander {

using namespace Trace;

namespace Eval {

std::string analyze_king_safety_and_threats(const Position& pos, int phase) {
    std::stringstream ss;
    
    // =========================================================================
    // BLOCCO 1: OUTPUT ORIGINALE INTATTO AL 100%
    // =========================================================================
    Trace::Helpers::append_term_row(ss, pos, phase, "\n\nKing safety", KING);
    ss << "\n";
    Trace::Helpers::append_term_row(ss, pos, phase, "Threats", THREAT);
    ss << "\n";

    // =========================================================================
    // BLOCCO 2: OUTPUT AGGIORNATO (ADVANCED MODULES)
    // =========================================================================
    ss << "================================================================\n";
    ss << "=== KING SAFETY & THREATS SUBELEMENTS ===\n\n";

    // 1. ENGINE NUMERICAL SCORES
    // Note: KING and THREAT belong to the global Alexander namespace, not Trace::
    ScoreForClassical w_ks = Trace::scores[KING][WHITE];
    ScoreForClassical b_ks = Trace::scores[KING][BLACK];
    ScoreForClassical net_ks = w_ks - b_ks;

    ScoreForClassical w_th = Trace::scores[THREAT][WHITE];
    ScoreForClassical b_th = Trace::scores[THREAT][BLACK];
    ScoreForClassical net_th = w_th - b_th;

    ss << "King Safety Engine Evaluation (Pawn Shelter & Attacker Proximity):\n"
       << "  White : " << std::setw(4) << mg_value(w_ks) << " (MG) / " << std::setw(4) << eg_value(w_ks) << " (EG)\n"
       << "  Black : " << std::setw(4) << mg_value(b_ks) << " (MG) / " << std::setw(4) << eg_value(b_ks) << " (EG)\n"
       << "  Net   : " << std::setw(4) << mg_value(net_ks) << " (MG) / " << std::setw(4) << eg_value(net_ks) << " (EG)\n\n";

    ss << "Tactical Threats Evaluation (Hanging Pieces, Forks, Pins):\n"
       << "  White : " << std::setw(4) << mg_value(w_th) << " (MG) / " << std::setw(4) << eg_value(w_th) << " (EG)\n"
       << "  Black : " << std::setw(4) << mg_value(b_th) << " (MG) / " << std::setw(4) << eg_value(b_th) << " (EG)\n"
       << "  Net   : " << std::setw(4) << mg_value(net_th) << " (MG) / " << std::setw(4) << eg_value(net_th) << " (EG)\n\n";

    // 2. KING SAFETY DIAGNOSTIC VERDICT
    int mg_ks = mg_value(net_ks);
    
    ss << "King Safety Audit:\n";
    if (mg_ks > 40) {
        ss << "  - White's King is significantly safer (+" << mg_ks << "). Black's King is exposed or under active attack.\n";
    } else if (mg_ks < -40) {
        ss << "  - Black's King is significantly safer (" << std::abs(mg_ks) << "). White's King is exposed or under active attack.\n";
    } else {
        ss << "  - Both Kings possess adequate shelter. No immediate mating nets detected.\n";
    }

    // 3. THREATS DIAGNOSTIC VERDICT
    int mg_th = mg_value(net_th);

    ss << "\nTactical Initiative Audit:\n";
    if (mg_th > 20) {
        ss << "  - White holds the tactical initiative (+" << mg_th << "). White is creating more immediate threats, forks, or pressuring hanging pieces.\n";
    } else if (mg_th < -20) {
        ss << "  - Black holds the tactical initiative (" << std::abs(mg_th) << "). Black is creating more immediate threats, forks, or pressuring hanging pieces.\n";
    } else {
        ss << "  - Tactical tension is balanced. Neither side holds a distinct forcing advantage.\n";
    }

    // 4. STRATEGIC ABLATION (Queen Trade Impact)
    ss << "\nStrategic Ablation (Queen Trade Impact):\n";
    if (mg_ks > 40) {
        ss << "  - ASSASSINATION PROTOCOL: Black's King is highly exposed. WHITE must AVOID trading Queens at all costs to sustain the attack. BLACK must FORCE a Queen trade to neutralize the danger.\n";
    } else if (mg_ks < -40) {
        ss << "  - ASSASSINATION PROTOCOL: White's King is highly exposed. BLACK must AVOID trading Queens at all costs to sustain the attack. WHITE must FORCE a Queen trade to neutralize the danger.\n";
    } else if (mg_th > 40 || mg_th < -40) {
        ss << "  - TACTICAL MINEFIELD: Severe tactical threats exist. The player under pressure should seek simplifications to defuse the tension.\n";
    } else {
        ss << "  - King safety and threats are stable. Piece trades will not significantly alter the King safety dynamic.\n";
    }

    // 5. DYNAMIC OTB STRATEGIC SYNTHESIS
    ss << "\n--- OTB STRATEGIC SYNTHESIS ---\n";
    
    // Theory Branch
    if (mg_ks > 40 || mg_ks < -40) {
        ss << "  * Theory : King safety supersedes all other positional rules. A naked King completely nullifies a material or structural advantage because mating attacks ignore point values.\n";
    } else if (mg_th > 30 || mg_th < -30) {
        ss << "  * Theory : The initiative is a perishable asset. When you have severe tactical threats, you must calculate forcing moves (checks, captures, threats) before the opponent can consolidate.\n";
    } else {
        ss << "  * Theory : When the Kings are safe and tactics are absent, the game enters a quiet maneuvering phase where structural advantages (pawns, space, outposts) slowly accumulate.\n";
    }

    // Action Branch
    ss << "  * Action : ";
    if (mg_ks > 40) {
        ss << "White must launch an all-out attack on the Black King. Do not hesitate to sacrifice a pawn to open lines for Rooks and Bishops. Avoid trading Queens under any circumstances.\n";
    } else if (mg_ks < -40) {
        ss << "Black must launch an all-out attack on the White King. Do not hesitate to sacrifice a pawn to open lines for Rooks and Bishops. Avoid trading Queens under any circumstances.\n";
    } else if (mg_th > 20) {
        ss << "White holds the initiative. Look for undefended Black pieces (hanging pieces) and calculate double-attacks (forks). Maintain the tension.\n";
    } else if (mg_th < -20) {
        ss << "Black holds the initiative. Look for undefended White pieces (hanging pieces) and calculate double-attacks (forks). Maintain the tension.\n";
    } else {
        ss << "Play prophylactically. Anticipate your opponent's structural breaks and slowly improve the placement of your worst piece (Makogonov principle).\n";
    }

    ss << "\n";
    return ss.str();
}

} // namespace Eval
} // namespace Alexander