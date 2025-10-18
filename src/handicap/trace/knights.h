#ifndef KNIGHTS_H_INCLUDED
#define KNIGHTS_H_INCLUDED

#include "../../position.h"
#include "../../types.h"
#include "../../bitboard.h"
#include "../../uci.h"
#include <string>
#include <sstream>
#include <iomanip>

namespace Alexander {

namespace Eval {
std::string analyze_knights(const Position& pos, int phase, const std::string& shashinZone);
}  // namespace Eval

}  // namespace Alexander

#endif