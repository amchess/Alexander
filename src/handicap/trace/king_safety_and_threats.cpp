#include "../../evaluate.h"
#include "trace.h"
#include "../evaluate_handicap.h"
#include "../../uci.h"
#include "../../movegen.h"
#include <sstream>
#include "king_safety_and_threats.h"
namespace Alexander {

using namespace Trace;

namespace Eval {

std::string analyze_king_safety_and_threats(const Position& pos, int phase) {
    std::stringstream ss;
    Trace::Helpers::append_term_row(ss, pos, phase, "\n\nKing safety", KING);
    ss << "\n";
    Trace::Helpers::append_term_row(ss, pos, phase, "Threats", THREAT);
    ss << "\n";
    return ss.str();
}

}

}  // namespace Alexander