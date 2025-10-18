#include "../../evaluate.h"
#include "general.h"
#include "../evaluate_handicap.h"
#include "../../uci.h"
#include "../../movegen.h"
#include <sstream>

namespace Alexander {

namespace Eval {
std::string generate_general_info(Position&          pos,
                                  Value              v_white,
                                  uint8_t            winProb,
                                  const std::string& shashinZone,
                                  const std::string& gamePhase) {
    std::stringstream ss;
    ss << "=== GENERAL INFORMATION ===\n";
    ss << "Final evaluation: " << UCIEngine::to_cp(v_white, pos)
       << " (Win Probability: " << static_cast<int>(winProb) << "%)\n";
    ss << "Shashin Zone: " << shashinZone << "\n";
    ss << "Game Phase: " << gamePhase << "\n\n";
    return ss.str();
}
std::string generate_main_table() {
    std::stringstream ss;
    ss << "     ELEMENT     |   MG     EG   Total\n";
    ss << " -------------+--------------------\n";
    return ss.str();
}
}

}  // namespace Alexander