#ifndef GENERAL_H_INCLUDED
#define GENERAL_H_INCLUDED

#include "../../position.h"
#include "../../types.h"
#include "../../bitboard.h"
#include "../../uci.h"
#include <string>
#include <sstream>
#include <iomanip>

namespace Alexander {

namespace Eval {
std::string generate_general_info(Position&          pos,
                                  Value              v_white,
                                  uint8_t            winProb,
                                  const std::string& shashinZone,
                                  const std::string& gamePhase);
std::string generate_main_table();

}  // namespace Eval

}  // namespace Alexander

#endif