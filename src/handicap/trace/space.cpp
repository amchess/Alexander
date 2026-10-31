#include "../../evaluate.h"
#include "trace.h"
#include "../evaluate_handicap.h"
#include "../../uci.h"
#include "../../movegen.h"
#include "../../bitboard.h"
#include <sstream>
#include <iomanip>
#include "space.h"

namespace Alexander {
namespace Eval {

using namespace Trace;

std::string analyze_space(const Position& pos, int phase, uint8_t winProb) {
    (void)phase; // Silence warning
    std::stringstream ss;
    ss << "=== SPACE SUBELEMENTS ===\n";

    // 1. ENGINE NUMERICAL SCORES
    ScoreForClassical w_sp = Trace::scores[SPACE][WHITE];
    ScoreForClassical b_sp = Trace::scores[SPACE][BLACK];
    ScoreForClassical net_sp = w_sp - b_sp;
    int mg_net = mg_value(net_sp);

    ss << "Territorial Control Evaluation (Pawn advancement & Central footprint):\n"
       << "  White : " << std::setw(4) << mg_value(w_sp) << " (MG) / " << std::setw(4) << eg_value(w_sp) << " (EG)\n"
       << "  Black : " << std::setw(4) << mg_value(b_sp) << " (MG) / " << std::setw(4) << eg_value(b_sp) << " (EG)\n"
       << "  Net   : " << std::setw(4) << mg_net << " (MG) / " << std::setw(4) << eg_value(net_sp) << " (EG)\n\n";

    // 2. OTB DIAGNOSTIC VERDICT (The Cramp)
    ss << "Territorial Diagnostic:\n";
    if (mg_net > 20) {
        ss << "  - White holds a clear space advantage (+" << mg_net << "). Black is cramped. White should AVOID piece trades to keep Black suffocated. Black must force trades to free up their position.\n\n";
    } else if (mg_net < -20) {
        ss << "  - Black holds a clear space advantage (" << std::abs(mg_net) << "). White is cramped. Black should AVOID piece trades to keep White suffocated. White must force trades to free up their position.\n\n";
    } else {
        ss << "  - Space and territorial control are roughly equal. Neither side is critically cramped.\n\n";
    }

    // 3. DETTAGLIO SPAZIO PER AREE
    ss << "Space Detail by Area:\n";
    const Bitboard QueenSideFiles = FileABB | FileBBB | FileCBB;
    const Bitboard CenterFiles    = FileDBB | FileEBB;
    const Bitboard KingSideFiles  = FileFBB | FileGBB | FileHBB;

    auto pawn_attacks_bb = [](Color c, Bitboard pawns) {
        return c == WHITE ? (shift<NORTH_WEST>(pawns) | shift<NORTH_EAST>(pawns))
                          : (shift<SOUTH_WEST>(pawns) | shift<SOUTH_EAST>(pawns));
    };

    auto calculate_space_for_area = [&](Color us, Bitboard area_mask) {
        Color them = ~us;
        Bitboard rank_mask = (us == WHITE) ? (Rank2BB | Rank3BB | Rank4BB) : (Rank5BB | Rank6BB | Rank7BB);
        Bitboard space_mask = area_mask & rank_mask;

        Bitboard their_pawns = pos.pieces(them, PAWN);
        Bitboard their_pawn_attacks = pawn_attacks_bb(them, their_pawns);

        Bitboard safe = space_mask & ~pos.pieces(us, PAWN) & ~their_pawn_attacks;

        Bitboard behind = pos.pieces(us, PAWN);
        if (us == WHITE) {
            behind |= shift<SOUTH>(behind);
            behind |= shift<SOUTH>(shift<SOUTH>(behind));
        } else {
            behind |= shift<NORTH>(behind);
            behind |= shift<NORTH>(shift<NORTH>(behind));
        }
        int bonus = popcount(safe) + popcount(behind & safe & ~their_pawn_attacks);
        return bonus;
    };

    int white_queen_side = calculate_space_for_area(WHITE, QueenSideFiles);
    int white_center     = calculate_space_for_area(WHITE, CenterFiles);
    int white_king_side  = calculate_space_for_area(WHITE, KingSideFiles);
    int black_queen_side = calculate_space_for_area(BLACK, QueenSideFiles);
    int black_center     = calculate_space_for_area(BLACK, CenterFiles);
    int black_king_side  = calculate_space_for_area(BLACK, KingSideFiles);

    int queen_side_diff = white_queen_side - black_queen_side;
    int center_diff     = white_center - black_center;
    int king_side_diff  = white_king_side - black_king_side;

    ss << "                           Queen side   Center   King side\n";
    ss << "White                      " << std::setw(3) << white_queen_side << "         " << std::setw(3) << white_center << "        " << std::setw(3) << white_king_side << "\n";
    ss << "Black                      " << std::setw(3) << black_queen_side << "         " << std::setw(3) << black_center << "        " << std::setw(3) << black_king_side << "\n";
    ss << "Difference White-Black     " << std::setw(3) << queen_side_diff << "         " << std::setw(3) << center_diff << "        " << std::setw(3) << king_side_diff << "\n";

    int white_total_space = white_queen_side + white_center + white_king_side;
    int black_total_space = black_queen_side + black_center + black_king_side;
    ss << "Total Space: White " << white_total_space << " - Black " << black_total_space << "\n\n";

    // 4. EXPANSION FACTOR (Shashin Theory)
    ss << "Expansion Factor (Shashin Theory):\n";
    auto calculate_center_of_gravity = [&](Color color) -> double {
        Bitboard pieces = pos.pieces(color);
        double sum = 0.0;
        int piece_count = 0;
        while (pieces) {
            Square s = pop_lsb(pieces);
            Rank rank = rank_of(s);
            double rank_value = (color == WHITE) ? (rank + 1) : (8 - rank);
            sum += rank_value;
            piece_count++;
        }
        return piece_count > 0 ? sum / piece_count : 0.0;
    };

    double white_gravity = calculate_center_of_gravity(WHITE);
    double black_gravity = calculate_center_of_gravity(BLACK);
    double delta_expansion = white_gravity - black_gravity;

    ss << "  - White Center of Gravity: " << std::fixed << std::setprecision(2) << white_gravity << "\n";
    ss << "  - Black Center of Gravity: " << std::fixed << std::setprecision(2) << black_gravity << "\n";
    ss << "  - Delta (White-Black):     " << std::fixed << std::setprecision(2) << delta_expansion << "\n\n";

    if (winProb >= 45 && winProb <= 55) {
        ss << "Capablanca Expansion Recommendations:\n";
        ss << "  Both players should:\n";
        ss << "  1. Increase their own expansion factor\n";
        ss << "  2. Decrease opponent's expansion factor\n\n";

        if (delta_expansion > 0.5) {
            ss << "  - White has expansion advantage - convert to lasting positional pressure.\n";
            ss << "  - Black should challenge White's advanced pieces and seek exchanges.\n";
        } else if (delta_expansion < -0.5) {
            ss << "  - Black has expansion advantage - convert to lasting positional pressure.\n";
            ss << "  - White should challenge Black's advanced pieces and seek exchanges.\n";
        } else {
            ss << "  - Expansion is balanced - focus on gradual improvement and piece activity.\n";
        }
    }
    
    // 5. DYNAMIC OTB STRATEGIC SYNTHESIS
    ss << "\n--- OTB STRATEGIC SYNTHESIS ---\n";
    
    // Theory Branch
    if (mg_net > 20 || mg_net < -20) {
        ss << "  * Theory : A spatial advantage acts as a positional boa constrictor. It grants your pieces immense maneuvering flexibility while forcing the opponent's pieces to step on each other's toes.\n";
    } else if (delta_expansion > 0.5 || delta_expansion < -0.5) {
        ss << "  * Theory : Shashin's Center of Gravity dictates the 'weight' of an attack. A high expansion factor means pieces are physically closer to the enemy King, reducing the time required to create mating nets.\n";
    } else {
        ss << "  * Theory : When space is equal, the game is decided by pawn structure integrity, piece synergy, and the creation of favorable imbalances.\n";
    }

    // Action Branch
    ss << "  * Action : ";
    if (mg_net > 20) {
        ss << "White possesses a significant space advantage. DO NOT TRADE PIECES. Maintain the tension and slowly suffocate Black. Black must aggressively seek piece exchanges to relieve the cramp.\n";
    } else if (mg_net < -20) {
        ss << "Black possesses a significant space advantage. DO NOT TRADE PIECES. Maintain the tension and slowly suffocate White. White must aggressively seek piece exchanges to relieve the cramp.\n";
    } else if (white_total_space == black_total_space && white_center > black_center) {
        ss << "Total space is balanced, but White controls the center. White should prepare a central break to split the board. Black must blockade the center and seek counterplay on the flanks.\n";
    } else if (white_total_space == black_total_space && black_center > white_center) {
        ss << "Total space is balanced, but Black controls the center. Black should prepare a central break to split the board. White must blockade the center and seek counterplay on the flanks.\n";
    } else {
        ss << "Territory is balanced. Use prophylactic moves to prevent your opponent from expanding on the flanks, and maneuver your pieces to their optimal squares (see Makogonov ranking) before forcing the issue.\n";
    }

    ss << "\n";
    return ss.str();
}

} // namespace Eval
} // namespace Alexander