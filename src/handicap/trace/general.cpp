#include "general.h"
#include <iomanip>
#include <sstream>
#include "trace.h"

namespace Alexander {
namespace Eval {

std::string generate_general_info(Position& pos, Value v_white, uint8_t winProb, 
                                  const std::string& shashinZone, const std::string& gamePhase) {
    std::stringstream ss;

    // Retrieve the total scores extracted from the evaluation to compare MG vs EG
    ScoreForClassical w_tot = Trace::scores[Trace::TOTAL][WHITE];
    ScoreForClassical b_tot = Trace::scores[Trace::TOTAL][BLACK];
    ScoreForClassical pov_tot = (pos.side_to_move() == WHITE) ? (w_tot - b_tot) : (b_tot - w_tot);

    int mg_adv = mg_value(pov_tot);
    int eg_adv = eg_value(pov_tot);

    // Basic Header
    ss << "=== GENERAL INFORMATION ===\n"
       << "Final evaluation: " << std::fixed << std::setprecision(2) << (float)v_white / 100.0 
       << " (Win Probability: " << (int)winProb << "%)\n"
       << "Game Phase: " << gamePhase << "\n"
       << "Shashin Zone: " << shashinZone << "\n";

    // 1. Practical explanation of the Shashin Zone
    ss << "Strategic Outlook: ";
    if (shashinZone.find("Capablanca") != std::string::npos) {
        ss << "The position is balanced. Avoid unnecessary risks, focus on incremental piece improvement (maneuvering), and wait for opponent inaccuracies before forcing the game.\n";
    } else if (shashinZone.find("Tal") != std::string::npos) {
        ss << "Highly dynamic and sharp position. Initiative and attack are worth more than material. Calculate forcing variations and maintain high tactical pressure.\n";
    } else if (shashinZone.find("Steinitz") != std::string::npos) {
        ss << "Clear positional advantage. Accumulate small advantages, restrict opponent counterplay, and prepare a decisive breakthrough without rushing.\n";
    } else if (shashinZone.find("Petrosian") != std::string::npos) {
        ss << "Inferior position or under attack. Absolute priority on prophylaxis and defense: consolidate weaknesses, exchange the opponent's most active pieces, and try to build a fortress.\n";
    } else {
        ss << "Carefully evaluate direct threats and maintain a solid structure.\n";
    }

    // 2. MG vs EG analysis in natural language (with a 30 centipawn tolerance)
    ss << "Phase Advantage: ";
    if (mg_adv > eg_adv + 30) {
        ss << "You have significantly better prospects in the Middlegame (" << mg_adv << " vs " << eg_adv << "). "
           << "Maintain the tension, avoid piece exchanges (especially Queens), and leverage the position's complexity to create threats.\n";
    } else if (eg_adv > mg_adv + 30) {
        ss << "Your structure and material shine in the Endgame (" << eg_adv << " vs " << mg_adv << "). "
           << "Force simplifications, seek piece exchanges to defuse the middlegame, and transition into a technically winning endgame.\n";
    } else {
        ss << "Your advantage is stable in both the Middlegame and the Endgame. "
           << "You can flexibly choose whether to attack now or transition into a favorable endgame depending on your opponent's style.\n";
    }
    ss << "\n";

    return ss.str();
}

// ... [Keep your generate_main_table function down here untouched] ...
std::string generate_main_table() {
    std::stringstream ss;
    
    // Helper lambda to format MG and EG scores neatly
    auto format_score = [](ScoreForClassical s) {
        std::stringstream tmp;
        tmp << std::setw(5) << mg_value(s) << " " << std::setw(5) << eg_value(s);
        return tmp.str();
    };

    // Helper lambda to print a formatted row
    auto print_row = [&](const std::string& name, int term) {
        ScoreForClassical w = Trace::scores[term][WHITE];
        ScoreForClassical b = Trace::scores[term][BLACK];
        ScoreForClassical t = w - b; // Net Advantage
        
        std::stringstream row;
        row << "   | " << std::left << std::setw(16) << name 
            << " | " << format_score(w) 
            << " | " << format_score(b) 
            << " | " << format_score(t) << " |\n";
        return row.str();
    };

    ss << "=== DETAILED EVALUATION MATRIX ===\n";
    ss << "   +------------------+-------------+-------------+-------------+\n"
       << "   | Feature          |    White    |    Black    |  Advantage  |\n"
       << "   |                  |   MG    EG  |   MG    EG  |   MG    EG  |\n"
       << "   +------------------+-------------+-------------+-------------+\n";

    // Static & Material Features
    ss << print_row("Material", Trace::MATERIAL);
    ss << print_row("Imbalances", Trace::IMBALANCE);
    
    // The following terms belong directly to the Alexander namespace, not Trace::
    ss << print_row("Pawn Structure", PAWN);
    ss << print_row("Passed Pawns", Trace::PASSED);
    
    ss << "   +------------------+-------------+-------------+-------------+\n";
    
    // Pieces (using global piece types)
    ss << print_row("Knights", KNIGHT);
    ss << print_row("Bishops", BISHOP);
    ss << print_row("Rooks", ROOK);
    ss << print_row("Queens", QUEEN);
    
    ss << "   +------------------+-------------+-------------+-------------+\n";
    
    // Dynamic Features
    ss << print_row("Mobility", Trace::MOBILITY);
    ss << print_row("King Safety", KING); // King safety uses the KING piece type macro in trace
    ss << print_row("Threats", Trace::THREAT);
    ss << print_row("Space", Trace::SPACE);
    
    ss << "   +------------------+-------------+-------------+-------------+\n";
    
    // Totals
    ScoreForClassical w_tot = Trace::scores[Trace::TOTAL][WHITE];
    ScoreForClassical b_tot = Trace::scores[Trace::TOTAL][BLACK];
    ScoreForClassical t_tot = w_tot - b_tot;
    
    ss << "   | TOTAL (Raw Eval) | " << format_score(w_tot) 
       << " | " << format_score(b_tot) 
       << " | " << format_score(t_tot) << " |\n"
       << "   +------------------+-------------+-------------+-------------+\n\n";

    return ss.str();
}
}

}  // namespace Alexander