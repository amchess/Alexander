#include "../../evaluate.h"
#include "trace.h"
#include "../evaluate_handicap.h"
#include "../../uci.h"
#include "../../movegen.h"
#include <sstream>
#include <algorithm>
#include <vector>
#include "legal_moves.h"
#include "plans.h"

namespace Alexander {


using namespace Trace;

namespace Eval {


std::string generate_legal_moves_analysis(Position& pos) {
    std::stringstream ss;
    // =============================================
    // LEGAL MOVES SORTED BY WIN PROBABILITY AND SCORE
    ss
      << "\nLegal moves sorted by static activity (Win Probability / CentiPawns: Best to Worst):\n";

    struct MoveActivity {
        Move        move;
        int         winProbability;
        int         activityScore;
        std::string moveStr;
    };

    std::vector<MoveActivity> legalMoves;

    // There is no need to save the current evaluation separately:
    // v_white, passed as a parameter, is used directly

    // Generate all the legal moves
    for (const auto& move : MoveList<LEGAL>(pos))
    {
        // Make the move
        StateInfo st;
        pos.do_move(move, st);

        // [SC:trace] Quiet evaluation (static evaluation + capture-only quiescence search):
        // with the bare static evaluation every capture looked winning, because the
        // recapture was never considered (e.g. Bxf6 "+3 pawns" with ...Nxf6 on the board)
        Value v_after = Eval::quiet_evaluation(pos);

        // Win probability from the point of view of the side that moves
        uint8_t currentWinProbability;
        currentWinProbability = WDLModel::get_win_probability(-v_after, pos);

        // Undo the move
        pos.undo_move(move);

        // Improvement for the side that moves
        int currentActivityScore = UCIEngine::to_cp(-v_after, pos);

        legalMoves.push_back({move, (int) currentWinProbability, currentActivityScore,
                              UCIEngine::move(move, pos.is_chess960())});
    }

    // sort by decreasing win probability and, on ties, by decreasing activity
    std::sort(legalMoves.begin(), legalMoves.end(),
              [](const MoveActivity& a, const MoveActivity& b) {
                  if (a.winProbability != b.winProbability)
                      return a.winProbability > b.winProbability;
                  return a.activityScore > b.activityScore;
              });

    // Print the moves horizontally
    if (legalMoves.empty())
    {
        ss << "No legal moves found.\n";
    }
    else
    {
        ss << "Moves: ";
        for (size_t i = 0; i < legalMoves.size(); ++i)
        {
            const auto& moveAct = legalMoves[i];
            ss << moveAct.moveStr << "(" << moveAct.winProbability << "%/" << moveAct.activityScore
               << ")";
            if (i < legalMoves.size() - 1)
                ss << ", ";
        }
        ss << "\n";

        // Highlight the best move
        if (!legalMoves.empty())
        {
            const auto& bestMove = legalMoves[0];
            ss << "\nBest move: " << bestMove.moveStr
               << " (Win Probability: " << bestMove.winProbability
               << "%, Activity: " << bestMove.activityScore << ")\n";
        }
    }
    ss << "\n";
    return ss.str();
}

}

}  // namespace Alexander