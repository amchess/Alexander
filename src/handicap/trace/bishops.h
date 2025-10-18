#ifndef BISHOPS_H_INCLUDED
#define BISHOPS_H_INCLUDED

#include "../../position.h"
#include "../../types.h"
#include "../../bitboard.h"
#include "../../uci.h"
#include <string>
#include <sstream>
#include <iomanip>

namespace Alexander {

namespace Eval {
std::string
analyze_bishops(const Position& pos, int phase, const std::string& gamePhase, Value v_white);
}  // namespace Eval

}  // namespace Alexander

#endif