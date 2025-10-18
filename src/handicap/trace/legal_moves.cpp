#include "../../evaluate.h"
#include "trace.h"
#include "../evaluate_handicap.h"
#include "../../uci.h"
#include "../../movegen.h"
#include <sstream>
#include <algorithm>
#include <vector>
#include "legal_moves.h"

namespace Alexander {


using namespace Trace;

namespace Eval {


std::string generate_legal_moves_analysis(Position& pos) {
    std::stringstream ss;
    // =============================================
    // SEZIONE MOSSE LEGALI ORDINATE PER WIN PROBABILITY E SCORE
    ss
      << "\nLegal moves sorted by static activity (Win Probability / CentiPawns: Best to Worst):\n";

    struct MoveActivity {
        Move        move;
        int         winProbability;
        int         activityScore;
        std::string moveStr;
    };

    std::vector<MoveActivity> legalMoves;

    // NON serve più salvare la valutazione corrente separatamente
    // usiamo direttamente v_white che ci è stato passato come parametro

    // Genera tutte le mosse legali
    for (const auto& move : MoveList<LEGAL>(pos))
    {
        // Esegui la mossa
        StateInfo st;
        pos.do_move(move, st);

        // ⬇️ USA LA NUOVA FUNZIONE invece di Evaluation<NO_TRACE>
        // Valuta la posizione risultante
        Value v_after = Eval::evaluate_position(pos);

        // Calcola la win probability dal punto di vista del giocatore che muove
        uint8_t currentWinProbability;
        currentWinProbability = WDLModel::get_win_probability(-v_after, pos);

        // Annulla la mossa
        pos.undo_move(move);

        // Calcola il miglioramento per il giocatore che muove
        int currentActivityScore = UCIEngine::to_cp(-v_after, pos);

        legalMoves.push_back({move, (int) currentWinProbability, currentActivityScore,
                              UCIEngine::move(move, pos.is_chess960())});
    }

    // ordina per win probability decrescente e, a parità, per activity decrescente
    std::sort(legalMoves.begin(), legalMoves.end(),
              [](const MoveActivity& a, const MoveActivity& b) {
                  if (a.winProbability != b.winProbability)
                      return a.winProbability > b.winProbability;
                  return a.activityScore > b.activityScore;
              });

    // Stampa le mosse in orizzontale
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

        // Evidenzia la mossa migliore
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