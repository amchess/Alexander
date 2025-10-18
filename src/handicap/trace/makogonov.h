#ifndef MAKOGONOV_H_INCLUDED
#define MAKOGONOV_H_INCLUDED

#include "../../position.h"
#include "../../types.h"
#include "../../bitboard.h"
#include "../../uci.h"
#include <string>
#include <sstream>
#include <iomanip>

namespace Alexander {

namespace Eval {
std::string generate_makogonov_ranking(const Position& pos, Pawns::Entry* pawnEntry);
}  // namespace Eval

}  // namespace Alexander

#endif