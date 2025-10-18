#ifndef SPACE_H_INCLUDED
#define SPACE_H_INCLUDED

#include "../../position.h"
#include "../../types.h"
#include "../../bitboard.h"
#include "../../uci.h"
#include <string>
#include <sstream>
#include <iomanip>

namespace Alexander {

namespace Eval {
std::string analyze_space(const Position& pos, int phase, uint8_t winProb);

}  // namespace Eval

}  // namespace Alexander

#endif