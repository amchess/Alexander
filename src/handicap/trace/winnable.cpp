#include "../../evaluate.h"
#include "trace.h"
#include "../evaluate_handicap.h"
#include "../../uci.h"
#include "../../movegen.h"
#include <sstream>
#include "winnable.h"

namespace Alexander {

using namespace Trace;

namespace Eval {

std::string analyze_winnable(const Position& pos, int phase) {
    std::stringstream ss;
    Trace::Helpers::append_term_row(ss, pos, phase, "Winnable", WINNABLE);
    ss << "\n";
    return ss.str();
}

}

}  // namespace Alexander