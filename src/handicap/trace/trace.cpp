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
// for improved trace begin
// Funzione per determinare la zona di Shashin basata sulla win probability
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

// Funzione per calcolare la fase del gioco
// Funzione per calcolare la fase del gioco
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
// Funzione per ottenere il nome del pezzo
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

// Costanti dalla valutazione dei pedoni
constexpr ScoreForClassical Backward           = make_score(6, 19);
constexpr ScoreForClassical Doubled            = make_score(11, 51);
constexpr ScoreForClassical DoubledEarly       = make_score(17, 7);
constexpr ScoreForClassical Isolated           = make_score(1, 20);
constexpr ScoreForClassical WeakLever          = make_score(2, 57);
constexpr ScoreForClassical WeakUnopposed      = make_score(15, 18);
constexpr ScoreForClassical BlockedPawn[2]     = {make_score(-19, -8), make_score(-7, 3)};
constexpr int               Connected[RANK_NB] = {0, 3, 7, 7, 15, 54, 86};
}
// for improved trace begin
// Funzione helper per determinare l'area di un square
Area area_of(Square s) {
    File f = file_of(s);
    if (f <= FILE_C)
        return QUEEN_SIDE;
    if (f <= FILE_E)
        return CENTER;
    return KING_SIDE;
}

// Array per i conteggi di mobilità dettagliati [COLOR][AREA]
int mobility_area_counts[COLOR_NB][AREA_NB] = {0};

ScoreForClassical scores[TERM_NB][COLOR_NB];

void add(int idx, Color c, ScoreForClassical s) { scores[idx][c] = s; }

void add(int idx, ScoreForClassical w, ScoreForClassical b) {
    scores[idx][WHITE] = w;
    scores[idx][BLACK] = b;
}
}
using namespace Trace;

namespace Eval {

std::string trace_analysis(Position&          pos,
                           Value              v_white,
                           uint8_t            winProb,
                           const std::string& shashinZone,
                           const std::string& gamePhase,
                           int                phase) {
    std::stringstream ss;
    Pawns::Entry*     pawnEntry = Pawns::probe(pos);

    ss << generate_general_info(pos, v_white, winProb, shashinZone, gamePhase);

    ss << generate_main_table();

    ss << analyze_material(pos, phase);

    if (Alexander::Eval::handicapConfig.imbalancesToEvaluate)
    {
        ss << analyze_imbalances(pos, phase);
    }

    if (Alexander::Eval::handicapConfig.pawnsToEvaluate)
    {
        ss << analyze_pawns(pos, phase);
        ss << analyze_passed_pawns(pos, pawnEntry, phase);
    }

    ss << analyze_knights(pos, phase, shashinZone);
    ss << analyze_bishops(pos, phase, gamePhase, v_white);
    ss << analyze_rooks_and_queens(pos, phase);
    ss << analyze_king_safety_and_threats(pos, phase);
    ss << analyze_mobility(pos, phase);
    ss << analyze_space(pos, phase, winProb);

    if (Alexander::Eval::handicapConfig.winnableToEvaluate)
    {
        ss << analyze_winnable(pos, phase);
    }

    // 4. ANALISI FINALI
    ss << generate_makogonov_ranking(pos, pawnEntry);
    ss << generate_legal_moves_analysis(pos);

    return ss.str();
}
}

}  // namespace Alexander