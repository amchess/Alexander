#ifndef PLANS_H_INCLUDED
#define PLANS_H_INCLUDED

#include "../../position.h"
#include "../../types.h"
#include <string>
#include <utility>
#include <vector>

namespace Alexander {

namespace Eval {

// [SC:trace] Structured results of the counterfactual analysis, used by the Master's Verdict
// (verdict.cpp). Values are cp on the UCI scale; "Us" = side to move.
struct PlanPiece {
    Square sq;
    Piece  pc;
    int    cp;          // value for its owner
    int    efficiency;  // value / nominal value of its type, %
};

struct PlanCandidate {
    Move  m;
    Value q;   // quiet value after the move, mover's point of view
    int   cp;  // q in cp
    int   gain;
    int   wp;
    bool  capture, check, kingMove, sacrifice, pawnBreak;
    std::vector<std::pair<int, int>> termDeltas;  // (cp for the mover, term), largest first
    // Two-ply check (top candidates only): quiet value after the opponent's best reply, mover's
    // point of view. VALUE_NONE when not computed.
    Value q2 = VALUE_NONE;
    Move  reply = Move::none();
};

struct PlanTrade {
    Piece  a, b;  // White piece, Black piece
    Square sa, sb;
    int    cpWhite;
};

struct PlanReroute {
    Piece  pc;
    Square from, via, to;  // via == SQ_NONE for a one-move maneuver
    int    gain;
};

struct PlanFacts {
    Value staticUs = VALUE_ZERO, quietUs = VALUE_ZERO, nullQuietUs = VALUE_ZERO;
    bool  tactical = false, inCheck = false;
    Move  rootTactic = Move::none();
    bool  threat = false, threatMate = false;
    Move  threatMove = Move::none();
    int   threatCp   = 0;
    std::vector<PlanPiece> pieces;
    bool  hasQueenTrade = false, hasRookTrade = false;
    int   queenTrade = 0, rookTrade = 0;  // White point of view
    std::vector<PlanTrade>     trades;    // sorted by cpWhite, best for White first
    std::vector<PlanCandidate> cands;     // all legal moves, best first
    bool  tension = false, releaseTension = false;
    Move  tensionMove = Move::none();
    int   tensionCp   = 0;
    std::vector<Move>        goodBreaks, badBreaks, shelterBad;
    std::vector<PlanReroute> reroutes[COLOR_NB];  // best first
    Move  oppIdea   = Move::none();
    int   oppIdeaCp = 0;
};

// Names of the evaluation terms used by PlanCandidate::termDeltas
const char* plan_term_name(int term);

// [SC:trace] Counterfactual analysis of the position, built only on the classical evaluation:
// piece values by ablation, trade advisor, threats, candidate plans explained term by term,
// pawn breaks, maneuvers (Makogonov rerouting), opponent's ideas and the Master's Verdict.
std::string generate_plans_analysis(Position&          pos,
                                    Value              v_white,
                                    uint8_t            winProb,
                                    const std::string& shashinZone,
                                    const std::string& gamePhase,
                                    int                phase,
                                    PlanFacts*         facts = nullptr);

// [SC:trace] Static evaluation resolved by a capture-only quiescence search (side to move
// point of view): used by the trace instead of the bare static evaluation after a move.
Value quiet_evaluation(Position& pos);

}  // namespace Eval

}  // namespace Alexander

#endif  // #ifndef PLANS_H_INCLUDED
