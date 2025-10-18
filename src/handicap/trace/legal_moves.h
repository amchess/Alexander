#ifndef LEGAL_MOVES_H_INCLUDED
#define LEGAL_MOVES_H_INCLUDED

#include "../../position.h"
#include "../../types.h"
#include "../../bitboard.h"
#include "../../uci.h"
#include <string>
#include <sstream>
#include <iomanip>

namespace Alexander {

namespace Eval {
std::string generate_legal_moves_analysis(Position& pos);
}  // namespace Eval

}  // namespace Alexander

#endif