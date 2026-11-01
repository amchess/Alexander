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
    
    // =========================================================================
    // BLOCCO 1: OUTPUT ORIGINALE INTATTO AL 100% (Legacy Parser Support)
    // =========================================================================
    Trace::Helpers::append_term_row(ss, pos, phase, "Winnable", WINNABLE);
    ss << "\n";

    // =========================================================================
    // BLOCCO 2: OUTPUT AGGIORNATO (ADVANCED WINNABLE MODULE / MUTED LOGIC)
    // =========================================================================
    // Note: The redundant "Winnable" header can be effectively muted or handled 
    // here since Win Probability is already fully processed in General Information.
    // (Uncomment the line below if you wish to strictly enforce the silent behavior).
    // (void)pos; (void)phase;
    // ss.str(""); // Clear legacy row if strict muting is desired

    return ss.str();
}

} // namespace Eval
} // namespace Alexander