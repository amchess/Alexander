#ifndef VERDICT_H_INCLUDED
#define VERDICT_H_INCLUDED

#include <string>

#include "../../position.h"
#include "../../types.h"
#include "plans.h"

namespace Alexander {

namespace Eval {

// [SC:trace] The Master's Verdict: high-level synthesis of the whole trace in natural language,
// printed first. Same sections and same rules as the verdict of ShashChess, so that the two
// traces can be compared, plus the information that only the classical evaluation provides
// (the weight of every evaluation term, middlegame/endgame prospects, endgame scaling).
std::string compose_verdict(Position&          pos,
                            const PlanFacts&   f,
                            Value              v_white,
                            uint8_t            winProb,
                            const std::string& shashinZone,
                            const std::string& gamePhase,
                            int                phase);

}  // namespace Eval

}  // namespace Alexander

#endif  // #ifndef VERDICT_H_INCLUDED
