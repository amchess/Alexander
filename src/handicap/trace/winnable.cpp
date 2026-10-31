#include "../../evaluate.h"
#include "trace.h"
#include "../evaluate_handicap.h"
#include "../../uci.h"
#include "../../movegen.h"
#include <sstream>
#include "winnable.h"

namespace Alexander {
namespace Eval {

using namespace Trace;

std::string analyze_winnable(const Position& pos, int phase) {
    (void)pos; (void)phase; // Silence compiler warnings for unused variables
    std::stringstream ss;
    // We intentionally return an empty string here to mute the redundant "Winnable" header.
    // Win Probability is already handled in the General Information block.
    return ss.str();
}

} // namespace Eval
} // namespace Alexander