#ifndef TRACE_H_INCLUDED
#define TRACE_H_INCLUDED

#include "../../position.h"
#include "../../types.h"
#include "../../bitboard.h"
#include "../../uci.h"
#include <string>
#include <sstream>
#include <iomanip>

namespace Alexander {

namespace Trace {
// Enumerazioni
enum Tracing {
    NO_TRACE,
    TRACE
};

enum Term {
    MATERIAL = 8,
    IMBALANCE,
    MOBILITY,
    THREAT,
    PASSED,
    SPACE,
    WINNABLE,
    TOTAL,
    TERM_NB
};

enum Area {
    QUEEN_SIDE,
    CENTER,
    KING_SIDE,
    AREA_NB
};

// Dichiarazioni delle variabili globali
extern ScoreForClassical scores[TERM_NB][COLOR_NB];
extern int               mobility_area_counts[COLOR_NB][AREA_NB];

// Dichiarazioni delle funzioni
void add(int idx, Color c, ScoreForClassical s);
void add(int idx, ScoreForClassical w, ScoreForClassical b = SCORE_ZERO);
Area area_of(Square s);

// Add function declarations that are used in evaluate.cpp
std::string get_shashin_zone(uint8_t winProb, Color sideToMove);
std::string get_game_phase(const Position& pos);
std::string piece_type_name(PieceType pt);

// =============================================
// HELPER FUNCTIONS - Implementate inline nell'header
// =============================================
namespace Helpers {

// Conversione square to string
inline std::string square_to_string(Square s) {
    return std::string{static_cast<char>('a' + file_of(s)), static_cast<char>('1' + rank_of(s))};
}

// Pretty print per square
inline std::string square_str(Square sq) {
    char file = 'a' + file_of(sq);
    char rank = '1' + rank_of(sq);
    return std::string{file, rank};
}
inline std::string bitboard_to_squares(Bitboard b) {
    std::string result;
    while (b)
    {
        Square s = pop_lsb(b);
        result += square_to_string(s) + " ";
    }
    return result.empty() ? "none" : result;
}
// Helper per appendere una riga con il valore interpolato
inline void append_term_row(
  std::stringstream& ss, const Position& pos, int phase, const char* term_name, int term) {
    ScoreForClassical net   = scores[term][WHITE] - scores[term][BLACK];
    Value             mg    = mg_value(net);
    Value             eg    = eg_value(net);
    Value             total = (mg * phase + eg * (PHASE_MIDGAME - phase)) / PHASE_MIDGAME;

    ss << " " << std::setw(12) << term_name << " | " << std::setw(5) << UCIEngine::to_cp(mg, pos)
       << " " << std::setw(5) << UCIEngine::to_cp(eg, pos) << " " << std::setw(5)
       << UCIEngine::to_cp(total, pos) << " |\n";
}

}  // namespace Helpers

}  // namespace Trace

namespace Eval {
std::string analyze_passed_pawns(const Position& pos, Pawns::Entry* pawnEntry, int phase);
std::string analyze_knights(const Position& pos, int phase, const std::string& shashinZone);
std::string
analyze_bishops(const Position& pos, int phase, const std::string& gamePhase, Value v_white);
std::string analyze_rooks_and_queens(const Position& pos, int phase);
std::string analyze_king_safety_and_threats(const Position& pos, int phase);
std::string analyze_mobility(const Position& pos, int phase);
std::string analyze_space(const Position& pos, int phase, uint8_t winProb);
std::string analyze_winnable(const Position& pos, int phase);
std::string generate_makogonov_ranking(const Position& pos, Pawns::Entry* pawnEntry);
std::string generate_legal_moves_analysis(Position& pos);

std::string trace_analysis(Position&          pos,
                           Value              v_white,
                           uint8_t            winProb,
                           const std::string& shashinZone,
                           const std::string& gamePhase,
                           int                phase);
}  // namespace Eval

}  // namespace Alexander

#endif