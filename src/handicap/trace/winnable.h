#ifndef WINNABLE_H_INCLUDED
#define WINNABLE_H_INCLUDED

#include "../../position.h"
#include "../../types.h"
#include "../../bitboard.h"
#include "../../uci.h"
#include <string>
#include <sstream>
#include <iomanip>

namespace Alexander {


namespace Eval {
std::string analyze_winnable(const Position& pos, int phase);

}  // namespace Eval

}  // namespace Alexander

#endif