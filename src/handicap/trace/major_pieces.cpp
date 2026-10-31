#include "../../evaluate.h"
#include "trace.h"
#include "../evaluate_handicap.h"
#include "../../uci.h"
#include "../../movegen.h"
#include "../../bitboard.h"
#include <sstream>
#include <vector>
#include <iomanip>

namespace Alexander {
namespace Eval {

using namespace Trace;

std::string analyze_rooks_and_queens(const Position& pos, int phase) {
    (void)phase; // Silence warning
    std::stringstream ss;
    ss << "=== MAJOR PIECES SUBELEMENTS (Rooks & Queens) ===\n";

    // 1. ENGINE NUMERICAL SCORES
    ScoreForClassical w_r = Trace::scores[ROOK][WHITE];
    ScoreForClassical b_r = Trace::scores[ROOK][BLACK];
    ScoreForClassical net_r = w_r - b_r;

    ScoreForClassical w_q = Trace::scores[QUEEN][WHITE];
    ScoreForClassical b_q = Trace::scores[QUEEN][BLACK];
    ScoreForClassical net_q = w_q - b_q;

    ss << "Rook Positional Scores (Open Files, 7th Rank, Connectivity):\n"
       << "  White : " << std::setw(4) << mg_value(w_r) << " (MG) / " << std::setw(4) << eg_value(w_r) << " (EG)\n"
       << "  Black : " << std::setw(4) << mg_value(b_r) << " (MG) / " << std::setw(4) << eg_value(b_r) << " (EG)\n"
       << "  Net   : " << std::setw(4) << mg_value(net_r) << " (MG) / " << std::setw(4) << eg_value(net_r) << " (EG)\n\n";

    ss << "Queen Positional Scores (Centralization, Mobility):\n"
       << "  White : " << std::setw(4) << mg_value(w_q) << " (MG) / " << std::setw(4) << eg_value(w_q) << " (EG)\n"
       << "  Black : " << std::setw(4) << mg_value(b_q) << " (MG) / " << std::setw(4) << eg_value(b_q) << " (EG)\n"
       << "  Net   : " << std::setw(4) << mg_value(net_q) << " (MG) / " << std::setw(4) << eg_value(net_q) << " (EG)\n\n";

    // 2. DYNAMIC DETECTION: Files Topography
    std::vector<File> open_files;
    std::vector<File> w_semi_open;
    std::vector<File> b_semi_open;

    for (File f = FILE_A; f <= FILE_H; ++f) {
        bool w_pawn_exists = (pos.pieces(WHITE, PAWN) & file_bb(f)) != 0;
        bool b_pawn_exists = (pos.pieces(BLACK, PAWN) & file_bb(f)) != 0;

        if (!w_pawn_exists && !b_pawn_exists) open_files.push_back(f);
        else if (!w_pawn_exists && b_pawn_exists) w_semi_open.push_back(f);
        else if (w_pawn_exists && !b_pawn_exists) b_semi_open.push_back(f);
    }

    auto file_list_to_str = [](const std::vector<File>& files) {
        if (files.empty()) return std::string("None");
        std::string s = "";
        for (size_t i = 0; i < files.size(); ++i) {
            s += char('a' + files[i]);
            if (i < files.size() - 1) s += ", ";
        }
        return s;
    };

    ss << "Structural Targets (File Topography):\n";
    ss << "  - Fully Open Files      : " << file_list_to_str(open_files) << "\n";
    ss << "  - White Semi-Open Files : " << file_list_to_str(w_semi_open) << " (Targets for White Rooks)\n";
    ss << "  - Black Semi-Open Files : " << file_list_to_str(b_semi_open) << " (Targets for Black Rooks)\n\n";

    // 3. MAJOR PIECE DEPLOYMENT CHECK
    bool w_has_7th = false;
    bool b_has_7th = false;
    bool w_controls_open = false;
    bool b_controls_open = false;

    auto analyze_deployment = [&](Color c) {
        Bitboard rooks = pos.pieces(c, ROOK);
        Bitboard queens = pos.pieces(c, QUEEN);
        
        if (!rooks && !queens) {
            ss << "  - No major pieces on the board.\n";
            return;
        }

        bool active_major_found = false;

        // Check Rooks
        while (rooks) {
            Square sq = pop_lsb(rooks);
            File f = file_of(sq);
            Rank r = rank_of(sq);
            
            std::string sq_str = "";
            sq_str += char('a' + f);
            sq_str += char('1' + r);

            bool on_open = std::find(open_files.begin(), open_files.end(), f) != open_files.end();
            bool on_semi = false;
            if (c == WHITE) on_semi = std::find(w_semi_open.begin(), w_semi_open.end(), f) != w_semi_open.end();
            else on_semi = std::find(b_semi_open.begin(), b_semi_open.end(), f) != b_semi_open.end();

            bool on_7th = (c == WHITE && r == RANK_7) || (c == BLACK && r == RANK_2);

            if (on_7th) {
                ss << "  - Rook on " << sq_str << " has invaded the 7th rank! (Lethal attacking position)\n";
                active_major_found = true;
                if (c == WHITE) w_has_7th = true;
                if (c == BLACK) b_has_7th = true;
            } else if (on_open) {
                ss << "  - Rook on " << sq_str << " actively controls the fully open " << char('a' + f) << "-file.\n";
                active_major_found = true;
                if (c == WHITE) w_controls_open = true;
                if (c == BLACK) b_controls_open = true;
            } else if (on_semi) {
                ss << "  - Rook on " << sq_str << " applies pressure down the semi-open " << char('a' + f) << "-file.\n";
                active_major_found = true;
            }
        }

        // Check Queens
        while (queens) {
            Square sq = pop_lsb(queens);
            File f = file_of(sq);
            Rank r = rank_of(sq);
            
            std::string sq_str = "";
            sq_str += char('a' + f);
            sq_str += char('1' + r);

            // A Queen is aggressively centralized if it sits in the D4/E4/D5/E5 box
            if ((f == FILE_D || f == FILE_E) && (r == RANK_4 || r == RANK_5)) {
                ss << "  - Queen on " << sq_str << " is aggressively centralized, radiating immense power across the board.\n";
                active_major_found = true;
            }
            
            // Queens supporting Rooks on open files (Batteries)
            bool on_open = std::find(open_files.begin(), open_files.end(), f) != open_files.end();
            if (on_open) {
                ss << "  - Queen on " << sq_str << " is aligned on the fully open " << char('a' + f) << "-file.\n";
                active_major_found = true;
                if (c == WHITE) w_controls_open = true;
                if (c == BLACK) b_controls_open = true;
            }
        }

        if (!active_major_found) {
            ss << "  - Major pieces are currently passively deployed (not occupying open files, central squares, or the 7th rank).\n";
        }
    };

    ss << "White Major Piece Deployment:\n";
    analyze_deployment(WHITE);

    ss << "\nBlack Major Piece Deployment:\n";
    analyze_deployment(BLACK);

    // 4. DYNAMIC OTB STRATEGIC SYNTHESIS
    ss << "\n--- OTB STRATEGIC SYNTHESIS ---\n";
    
    // Theory Branch
    if (w_has_7th || b_has_7th) {
        ss << "  * Theory : A Rook on the 7th rank (the 'Pig') is worth at least a pawn in pure positional compensation. It sweeps lateral pawns and severely restricts the enemy King.\n";
    } else if (!open_files.empty()) {
        ss << "  * Theory : Fully open files are highways into the enemy camp. The player who successfully doubles Rooks on an open file usually dictates the middlegame.\n";
    } else if (!w_semi_open.empty() || !b_semi_open.empty()) {
        ss << "  * Theory : Without fully open files, Rooks must be placed on semi-open files to exert pressure on backward or isolated enemy pawns (which act as physical stoppers).\n";
    } else {
        ss << "  * Theory : When all files are closed, Rooks are strictly defensive pieces. The position requires a pawn break to create a file before major pieces can become relevant.\n";
    }

    // Action Branch
    ss << "  * Action : ";
    if (w_has_7th && !b_has_7th) {
        ss << "White has successfully invaded the 7th rank. White must reinforce this Rook (double them) and use the lateral pressure to create mating nets or win material. Black must offer a Rook trade to evict it immediately.\n";
    } else if (b_has_7th && !w_has_7th) {
        ss << "Black has successfully invaded the 7th rank (2nd rank). Black must reinforce this Rook (double them) and use the lateral pressure to create mating nets or win material. White must offer a Rook trade to evict it immediately.\n";
    } else if (!open_files.empty()) {
        if (w_controls_open && !b_controls_open) {
            ss << "White controls the open file. White should prepare to invade the 7th rank. Black must immediately challenge the file by offering a Rook trade.\n";
        } else if (b_controls_open && !w_controls_open) {
            ss << "Black controls the open file. Black should prepare to invade the 2nd rank. White must immediately challenge the file by offering a Rook trade.\n";
        } else {
            ss << "Open files exist (" << file_list_to_str(open_files) << "). Move your Rooks to these files immediately. If your opponent is already there, challenge them. Do not yield the file.\n";
        }
    } else if (!w_semi_open.empty() || !b_semi_open.empty()) {
        ss << "No open files exist. Maneuver your Rooks to your designated semi-open files. Use the Rooks to pressure the enemy pawn at the base of the file, forcing their pieces into passive defense.\n";
    } else {
        ss << "The center and flanks are locked. Centralize your Rooks behind your pawn breaks and prepare to push pawns to forcibly rip open a file.\n";
    }

    ss << "\n";
    return ss.str();
}

} // namespace Eval
} // namespace Alexander