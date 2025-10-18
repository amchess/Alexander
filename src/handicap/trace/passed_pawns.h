#ifndef PASSED_PAWNS_H_INCLUDED
#define PASSED_PAWNS_H_INCLUDED

#include "../../position.h"
#include "../../types.h"
#include "../../bitboard.h"
#include "../../uci.h"
#include <string>
#include <sstream>
#include <iomanip>

namespace Alexander {


namespace Eval {
std::string analyze_passed_pawns(const Position& pos, Pawns::Entry* pawnEntry, int phase);
}  // namespace Eval

}  // namespace Alexander

#endif