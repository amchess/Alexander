#include "../../evaluate.h"
#include "trace.h"
#include "passed_pawns.h" 
#include "../evaluate_handicap.h"
#include "../../pawns.h"
#include "../../uci.h"
#include "../../movegen.h"
#include "../../bitboard.h"
#include <sstream>
#include <iomanip>

namespace Alexander {
namespace Eval {

using namespace Trace;

std::string analyze_passed_pawns(const Position& pos, Pawns::Entry* pawnEntry, int phase) {
    (void)phase; // Silence warning
    std::stringstream ss;
    ss << "=== PASSED PAWNS SUBELEMENTS ===\n";

    // 1. ENGINE NUMERICAL SCORES
    ScoreForClassical w_pass = Trace::scores[Trace::PASSED][WHITE];
    ScoreForClassical b_pass = Trace::scores[Trace::PASSED][BLACK];
    ScoreForClassical net_pass = w_pass - b_pass;

    int mg_net = mg_value(net_pass);
    int eg_net = eg_value(net_pass);

    ss << "Passed Pawn Engine Evaluation (Advancement, Danger, Blockades):\n"
       << "  White : " << std::setw(4) << mg_value(w_pass) << " (MG) / " << std::setw(4) << eg_value(w_pass) << " (EG)\n"
       << "  Black : " << std::setw(4) << mg_value(b_pass) << " (MG) / " << std::setw(4) << eg_value(b_pass) << " (EG)\n"
       << "  Net   : " << std::setw(4) << mg_net << " (MG) / " << std::setw(4) << eg_net << " (EG)\n\n";

    // 2. DYNAMIC DETECTION
    Bitboard white_passed = pawnEntry->passed_pawns(WHITE);
    Bitboard black_passed = pawnEntry->passed_pawns(BLACK);

    bool lethal_passer_exists = false;
    bool w_passer_blocked = false, b_passer_blocked = false;
    bool w_passer_free = false, b_passer_free = false;
    bool w_passer_supported = false, b_passer_supported = false;

    auto analyze_passers = [&](Color color, Bitboard passed) {
        if (!passed) {
            ss << "  - None detected.\n";
            return;
        }
        Direction push = pawn_push(color);
        while (passed) {
            Square sq = pop_lsb(passed);
            Rank rank = rank_of(sq);
            File file = file_of(sq);

            std::string sq_str = "";
            sq_str += char('a' + file);
            sq_str += char('1' + rank);

            int steps = (color == WHITE) ? (RANK_8 - rank) : (rank - RANK_1);
            
            // CORRECTED SUPPORT LOGIC (Template fixed)
            Bitboard sq_bb = square_bb(sq);
            Bitboard protectors = (color == WHITE) 
                ? (pos.pieces(WHITE, PAWN) & pawn_attacks_bb<BLACK>(sq_bb))
                : (pos.pieces(BLACK, PAWN) & pawn_attacks_bb<WHITE>(sq_bb));
                
            bool supported = protectors != 0;
            if (supported && color == WHITE) w_passer_supported = true;
            if (supported && color == BLACK) b_passer_supported = true;

            // Blockade check
            Square square_ahead = sq + push;
            bool blocked = pos.piece_on(square_ahead) != NO_PIECE;
            if (blocked && color == WHITE) w_passer_blocked = true;
            else if (!blocked && color == WHITE) w_passer_free = true;
            if (blocked && color == BLACK) b_passer_blocked = true;
            else if (!blocked && color == BLACK) b_passer_free = true;

            // Danger Level classification
            std::string strength;
            if ((color == WHITE && rank >= RANK_6) || (color == BLACK && rank <= RANK_3)) {
                strength = "LETHAL (Highly advanced)";
                lethal_passer_exists = true;
            } else if ((color == WHITE && rank == RANK_5) || (color == BLACK && rank == RANK_4)) {
                strength = "STRONG (Crossed midline)";
            } else {
                strength = "WEAK (Early stage)";
            }

            ss << "  - Pawn on " << sq_str << " (" << steps << " steps to promotion). "
               << "Status: " << (blocked ? "BLOCKADED" : "FREE to advance") 
               << ", " << (supported ? "SUPPORTED by friendly pawn" : "UNSUPPORTED")
               << " [" << strength << "].\n";
        }
    };

    ss << "White's Passed Pawns:\n";
    analyze_passers(WHITE, white_passed);
    
    ss << "\nBlack's Passed Pawns:\n";
    analyze_passers(BLACK, black_passed);

    // 3. ENDGAME SCALING ABLATION
    ss << "\nStrategic Ablation (Endgame Scaling):\n";
    if (eg_net > mg_net + 15 || eg_net > 30) {
        ss << "  - Transition heavily favors WHITE. White's passed pawn(s) represent a massive endgame asset (+" << eg_net << "). Black must avoid piece trades and maintain middlegame tactical complications to survive.\n\n";
    } else if (eg_net < mg_net - 15 || eg_net < -30) {
        ss << "  - Transition heavily favors BLACK. Black's passed pawn(s) represent a massive endgame asset (" << std::abs(eg_net) << "). White must avoid piece trades and maintain middlegame tactical complications to survive.\n\n";
    } else if (white_passed || black_passed) {
        ss << "  - Passed pawns exist but are well-controlled or offset each other. The endgame scaling advantage is currently stable.\n\n";
    } else {
        ss << "  - No passed pawns on the board.\n\n";
    }

    // 4. DYNAMIC OTB STRATEGIC SYNTHESIS
    ss << "--- OTB STRATEGIC SYNTHESIS ---\n";
    
    // Theory Branch
    if (lethal_passer_exists) {
        ss << "  * Theory : A highly advanced passed pawn splits the board in half. The defender is forced to passively anchor heavy pieces to stop it, losing all attacking potential on the other flank.\n";
    } else if ((w_passer_supported && w_passer_free) || (b_passer_supported && b_passer_free)) {
        ss << "  * Theory : A protected, unblockaded passed pawn is structurally the strongest asset in chess. It guarantees long-term positional initiative.\n";
    } else if (!white_passed && !black_passed) {
        ss << "  * Theory : Without passed pawns, the game relies entirely on creating central breaks, king attacks, or forcing pawn weaknesses.\n";
    } else {
        ss << "  * Theory : A passed pawn's threat level is directly proportional to how easily the opponent can place a piece securely on the square in front of it (The Blockade).\n";
    }

    // Action Branch
    ss << "  * Action : ";
    if (white_passed && !black_passed) {
        if (w_passer_blocked) ss << "White has a passed pawn, but it is currently blockaded. White must use pawns or minor pieces to evict the blockader. Black must never use a Rook to blockade, only a Knight or Bishop.\n";
        else ss << "White's passed pawn is free to roll. White must push it immediately to tie down Black's pieces. Black must establish a physical blockade on the square directly in front of the pawn immediately.\n";
    } else if (black_passed && !white_passed) {
        if (b_passer_blocked) ss << "Black has a passed pawn, but it is currently blockaded. Black must use pawns or minor pieces to evict the blockader. White must never use a Rook to blockade, only a Knight or Bishop.\n";
        else ss << "Black's passed pawn is free to roll. Black must push it immediately to tie down White's pieces. White must establish a physical blockade on the square directly in front of the pawn immediately.\n";
    } else if (white_passed && black_passed) {
        ss << "Both sides have passed pawns (A Footrace). Do not passively defend. Calculate concrete promotion tempos. The player whose pawn is further advanced (or better supported) should push aggressively.\n";
    } else {
        ss << "No passed pawns exist. Focus on creating one by identifying your pawn majority (usually on the flank) and preparing to advance it.\n";
    }

    ss << "\n";
    return ss.str();
}

} // namespace Eval
} // namespace Alexander