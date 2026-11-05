#include "../../evaluate.h"
#include "trace.h"
#include "general.h"
#include "imbalances.h"
#include "material_element.h"
#include "pawns_element.h"
#include "passed_pawns.h"
#include "knights.h"
#include "bishops.h"
#include "major_pieces.h"
#include "king_safety_and_threats.h"
#include "mobility.h"
#include "space.h"
#include "winnable.h"
#include "makogonov.h"
#include "legal_moves.h"
#include "plans.h"
#include "verdict.h"
#include "../evaluate_handicap.h"
#include "../../pawns.h"
#include "../../uci.h"
#include "../../movegen.h"
#include "../../bitboard.h"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <vector>

namespace Alexander {

namespace Trace {

// Shashin zone from the win probability
std::string get_shashin_zone(uint8_t winProb, Color sideToMove) {
    if (sideToMove == WHITE)
    {
        if (winProb <= 5)
            return "High Petrosian (Losing position)";
        else if (winProb <= 10)
            return "High-Middle Petrosian (Decisive disadvantage)";
        else if (winProb <= 15)
            return "Middle Petrosian (Clear disadvantage)";
        else if (winProb <= 20)
            return "Middle-Low Petrosian (Significant disadvantage)";
        else if (winProb <= 24)
            return "Low Petrosian (Slight disadvantage)";
        else if (winProb <= 49)
            return "Chaos: Capablanca-Petrosian (Balanced with opponent pressure)";
        else if (winProb == 50)
            return "Capablanca (Equal position)";
        else if (winProb <= 75)
            return "Chaos: Capablanca-Tal (Initiative)";
        else if (winProb <= 79)
            return "Low Tal (Slight advantage)";
        else if (winProb <= 84)
            return "Middle-Low Tal (Growing advantage)";
        else if (winProb <= 89)
            return "Middle Tal (Clear advantage)";
        else if (winProb <= 94)
            return "High-Middle Tal (Dominant position)";
        else
            return "High Tal (Winning position)";
    }
    else
    {
        uint8_t blackWinProb = 100 - winProb;
        if (blackWinProb <= 5)
            return "High Petrosian (Losing position)";
        else if (blackWinProb <= 10)
            return "High-Middle Petrosian (Decisive disadvantage)";
        else if (blackWinProb <= 15)
            return "Middle Petrosian (Clear disadvantage)";
        else if (blackWinProb <= 20)
            return "Middle-Low Petrosian (Significant disadvantage)";
        else if (blackWinProb <= 24)
            return "Low Petrosian (Slight disadvantage)";
        else if (blackWinProb <= 49)
            return "Chaos: Capablanca-Petrosian (Balanced with opponent pressure)";
        else if (blackWinProb == 50)
            return "Capablanca (Equal position)";
        else if (blackWinProb <= 75)
            return "Chaos: Capablanca-Tal (Initiative)";
        else if (blackWinProb <= 79)
            return "Low Tal (Slight advantage)";
        else if (blackWinProb <= 84)
            return "Middle-Low Tal (Growing advantage)";
        else if (blackWinProb <= 89)
            return "Middle Tal (Clear advantage)";
        else if (blackWinProb <= 94)
            return "High-Middle Tal (Dominant position)";
        else
            return "High Tal (Winning position)";
    }
}

// Game phase
std::string get_game_phase(const Position& pos) {
    constexpr float kingMatFase   = 3.0f;
    constexpr float queenMatFase  = 9.9f;
    constexpr float rookMatFase   = 5.5f;
    constexpr float bishopMatFase = 3.5f;
    constexpr float knightMatFase = 3.1f;
    constexpr float pawnMatFase   = 1.0f;

    int numTotUnit = pos.count<ALL_PIECES>();

    float totMatFase = kingMatFase * (pos.count<KING>(WHITE) + pos.count<KING>(BLACK))
                     + queenMatFase * (pos.count<QUEEN>(WHITE) + pos.count<QUEEN>(BLACK))
                     + rookMatFase * (pos.count<ROOK>(WHITE) + pos.count<ROOK>(BLACK))
                     + bishopMatFase * (pos.count<BISHOP>(WHITE) + pos.count<BISHOP>(BLACK))
                     + knightMatFase * (pos.count<KNIGHT>(WHITE) + pos.count<KNIGHT>(BLACK))
                     + pawnMatFase * (pos.count<PAWN>(WHITE) + pos.count<PAWN>(BLACK));

    int numSemiMoveInt = pos.game_ply();

    float faseNum = -1.0f;
    if (numSemiMoveInt != 0)
    {
        faseNum = (numTotUnit * totMatFase) / (3.0f * (numSemiMoveInt + 0.001f));
    }

    std::string faseDesc;
    if ((faseNum >= 0) && (faseNum < 5))
        faseDesc = "Endgame";
    else if ((faseNum >= 5) && (faseNum < 10))
        faseDesc = "Transition to endgame";
    else if ((faseNum >= 10) && (faseNum < 40))
        faseDesc = "Middlegame";
    else if ((faseNum >= 40) && (faseNum < 50))
        faseDesc = "Transition to middlegame";
    else if ((faseNum >= 50) || faseNum == -1)
        faseDesc = "Opening";

    return faseDesc;
}

// Name of a piece type
std::string piece_type_name(PieceType pt) {
    switch (pt)
    {
    case PAWN :
        return "Pawn";
    case KNIGHT :
        return "Knight";
    case BISHOP :
        return "Bishop";
    case ROOK :
        return "Rook";
    case QUEEN :
        return "Queen";
    default :
        return "Unknown";
    }
}

namespace {
// Constants of the pawn evaluation
constexpr ScoreForClassical Backward           = make_score(6, 19);
constexpr ScoreForClassical Doubled            = make_score(11, 51);
constexpr ScoreForClassical DoubledEarly       = make_score(17, 7);
constexpr ScoreForClassical Isolated           = make_score(1, 20);
constexpr ScoreForClassical WeakLever          = make_score(2, 57);
constexpr ScoreForClassical WeakUnopposed      = make_score(15, 18);
constexpr ScoreForClassical BlockedPawn[2]     = {make_score(-19, -8), make_score(-7, 3)};
constexpr int               Connected[RANK_NB] = {0, 3, 7, 7, 15, 54, 86};
}

// Area (queen side / center / king side) of a square
Area area_of(Square s) {
    File f = file_of(s);
    if (f <= FILE_C)
        return QUEEN_SIDE;
    if (f <= FILE_E)
        return CENTER;
    return KING_SIDE;
}

// Detailed mobility counts [COLOR][AREA]
int mobility_area_counts[COLOR_NB][AREA_NB] = {0};

ScoreForClassical scores[TERM_NB][COLOR_NB];

void add(int idx, Color c, ScoreForClassical s) { scores[idx][c] = s; }

void add(int idx, ScoreForClassical w, ScoreForClassical b) {
    scores[idx][WHITE] = w;
    scores[idx][BLACK] = b;
}

} // namespace Trace

using namespace Trace;

namespace Eval {

std::string trace_analysis(Position&          pos,
                           Value              v_white,
                           uint8_t            winProb,
                           const std::string& shashinZone,
                           const std::string& gamePhase,
                           int                phase) {
    // [SC:trace] The technical modules are collected first; the Master's Verdict, composed from
    // the structured facts of the counterfactual analysis, is printed before them.
    std::stringstream tech;
    Pawns::Entry*     pawnEntry = Pawns::probe(pos);

    if (pos.checkers())
        tech << "*** " << (pos.side_to_move() == WHITE ? "White" : "Black")
             << " is in check: static values below are indicative only. ***\n\n";
    tech << generate_general_info(pos, v_white, winProb, shashinZone, gamePhase);

    tech << generate_main_table(pos);

    tech << analyze_material(pos, phase);

    if (Alexander::Eval::handicapConfig.imbalancesToEvaluate)
    {
        tech << analyze_imbalances(pos, phase);
    }

    if (Alexander::Eval::handicapConfig.pawnsToEvaluate)
    {
        tech << analyze_pawns(pos, phase);
        tech << analyze_passed_pawns(pos, pawnEntry, phase);
    }

    tech << analyze_knights(pos, phase, shashinZone);
    tech << analyze_bishops(pos, phase, gamePhase, v_white);
    tech << analyze_rooks_and_queens(pos, phase);
    tech << analyze_king_safety_and_threats(pos, phase);
    tech << analyze_mobility(pos, phase);
    tech << analyze_space(pos, phase, winProb);

    if (Alexander::Eval::handicapConfig.winnableToEvaluate)
    {
        tech << analyze_winnable(pos, phase);
    }

    // Final analyses (Makogonov ranking uses the game phase)
    tech << generate_makogonov_ranking(pos, pawnEntry, gamePhase);
    tech << generate_legal_moves_analysis(pos);

    // [SC:trace] Counterfactual analysis (piece values, trades, plans). Run last: it evaluates
    // many positions, and the trace table is saved/restored around every snapshot.
    PlanFacts facts;
    tech << generate_plans_analysis(pos, v_white, winProb, shashinZone, gamePhase, phase, &facts);

    std::stringstream ss;
    ss << "Reading guide: the MASTER'S VERDICT (plain language) comes first; all the technical\n"
          "parameters it is based on follow after the separator.\n\n";
    ss << compose_verdict(pos, facts, v_white, winProb, shashinZone, gamePhase, phase);
    ss << "\n"
       << std::string(64, '#') << "\n"
       << "####            TECHNICAL PARAMETERS AND DETAILS            ####\n"
       << "####   (data behind the verdict above, for deeper study)    ####\n"
       << std::string(64, '#') << "\n\n";
    ss << tech.str();

    return ss.str();
}

} // namespace Eval

} // namespace Alexander