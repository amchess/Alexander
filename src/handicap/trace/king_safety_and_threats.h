#ifndef KING_SAFETY_AND_THREATS_H_INCLUDED
#define KING_SAFETY_AND_THREATS_H_INCLUDED

#include "../../position.h"
#include "../../types.h"
#include "../../bitboard.h"
#include "../../uci.h"
#include <string>
#include <sstream>
#include <iomanip>

namespace Alexander {

namespace Eval {
std::string analyze_king_safety_and_threats(const Position& pos, int phase);

}  // namespace Eval

}  // namespace Alexander

#endif