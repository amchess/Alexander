#include "../../evaluate.h"
#include "trace.h"
#include "../evaluate_handicap.h"
#include "../../uci.h"
#include "../../movegen.h"
#include "../../bitboard.h"
#include <sstream>
#include <vector>
#include <iomanip>
#include <algorithm> // for std::find

namespace Alexander {

using namespace Trace;

namespace Eval {

std::string analyze_rooks_and_queens(const Position& pos, int phase) {
    std::stringstream ss;
    
    // =========================================================================
    // BLOCCO 1: OUTPUT ORIGINALE INTATTO AL 100%
    // =========================================================================
    Trace::Helpers::append_term_row(ss, pos, phase, "Rooks", ROOK);
    ss << "\n";
    Trace::Helpers::append_term_row(ss, pos, phase, "Queens", QUEEN);
    ss << "Diagonals and columns control:\n";
    ss << "=== LONG RANGE PIECES SUBELEMENTS ===\n";
    
    // SEZIONE COLONNE APERTE E SEMIAPERTE
    ss << "Files:\n";
    ss << "=======\n";

    Bitboard allPawns   = pos.pieces(PAWN);
    Bitboard whitePawns = pos.pieces(WHITE, PAWN);
    Bitboard blackPawns = pos.pieces(BLACK, PAWN);

    // Open files (no pawns)
    ss << "Open: ";
    bool hasOpen = false;
    for (File f = FILE_A; f <= FILE_H; ++f)
    {
        if (!(allPawns & file_bb(f)))
        {
            ss << static_cast<char>('a' + f) << " ";
            hasOpen = true;
        }
    }
    if (!hasOpen)
        ss << "none";
    ss << "\n";

    // Semi-open files for white (no white pawns)
    ss << "Semi-open White: ";
    bool hasSemiOpenWhite = false;
    for (File f = FILE_A; f <= FILE_H; ++f)
    {
        if (!(whitePawns & file_bb(f)) && (blackPawns & file_bb(f)))
        {
            ss << static_cast<char>('a' + f) << " ";
            hasSemiOpenWhite = true;
        }
    }
    if (!hasSemiOpenWhite)
        ss << "none";
    ss << "\n";

    // Semi-open files for black (no black pawns)
    ss << "Semi-open Black: ";
    bool hasSemiOpenBlack = false;
    for (File f = FILE_A; f <= FILE_H; ++f)
    {
        if (!(blackPawns & file_bb(f)) && (whitePawns & file_bb(f)))
        {
            ss << static_cast<char>('a' + f) << " ";
            hasSemiOpenBlack = true;
        }
    }
    if (!hasSemiOpenBlack)
        ss << "none";
    ss << "\n";

    // SEZIONE DIAGONALI APERTE E SEMIAPERTE
    ss << "Diagonals:\n";
    ss << "==========\n";

    // Helper function to get diagonal endpoints
    auto get_diagonal_endpoints = [](Bitboard diag) -> std::pair<Square, Square> {
        if (diag == 0)
            return {SQ_NONE, SQ_NONE};

        Square minSq = pop_lsb(diag);
        Square maxSq = msb(diag);

        // Find actual endpoints by checking file and rank
        Square lowest  = minSq;
        Square highest = maxSq;

        while (diag)
        {
            Square s = pop_lsb(diag);
            if (file_of(s) < file_of(lowest)
                || (file_of(s) == file_of(lowest) && rank_of(s) < rank_of(lowest)))
                lowest = s;
            if (file_of(s) > file_of(highest)
                || (file_of(s) == file_of(highest) && rank_of(s) > rank_of(highest)))
                highest = s;
        }

        return {lowest, highest};
    };

    // Generate all possible diagonals dynamically
    auto generate_diagonals = []() -> std::vector<Bitboard> {
        std::vector<Bitboard> diagonals;

        // Generate A1-H8 diagonals
        for (int start_rank = RANK_1; start_rank <= RANK_8; ++start_rank)
        {
            for (int start_file = FILE_A; start_file <= FILE_H; ++start_file)
            {
                Bitboard diag     = 0;
                Square   start_sq = make_square(File(start_file), Rank(start_rank));

                // Generate diagonal in A1-H8 direction
                Square sq = start_sq;
                while (is_ok(sq))
                {
                    diag |= sq;
                    if (file_of(sq) == FILE_H || rank_of(sq) == RANK_8)
                        break;
                    sq += NORTH_EAST;
                }

                // Only add if it has at least 3 squares
                if (popcount(diag) >= 3)
                {
                    bool duplicate = false;
                    for (const auto& existing : diagonals)
                    {
                        if (existing == diag)
                        {
                            duplicate = true;
                            break;
                        }
                    }
                    if (!duplicate)
                    {
                        diagonals.push_back(diag);
                    }
                }
            }
        }

        // Generate H1-A8 anti-diagonals
        for (int start_rank = RANK_1; start_rank <= RANK_8; ++start_rank)
        {
            for (int start_file = FILE_A; start_file <= FILE_H; ++start_file)
            {
                Bitboard anti_diag = 0;
                Square   start_sq  = make_square(File(start_file), Rank(start_rank));

                // Generate diagonal in H1-A8 direction
                Square sq = start_sq;
                while (is_ok(sq))
                {
                    anti_diag |= sq;
                    if (file_of(sq) == FILE_A || rank_of(sq) == RANK_8)
                        break;
                    sq += NORTH_WEST;
                }

                // Only add if it has at least 3 squares
                if (popcount(anti_diag) >= 3)
                {
                    bool duplicate = false;
                    for (const auto& existing : diagonals)
                    {
                        if (existing == anti_diag)
                        {
                            duplicate = true;
                            break;
                        }
                    }
                    if (!duplicate)
                    {
                        diagonals.push_back(anti_diag);
                    }
                }
            }
        }

        return diagonals;
    };

    // Generate all diagonals
    std::vector<Bitboard> all_diagonals = generate_diagonals();

    // Separate A1-H8 and H1-A8 diagonals
    std::vector<Bitboard> a1h8_diagonals, h1a8_diagonals;

    for (const auto& diag : all_diagonals)
    {
        // Check if it's A1-H8 (NE/SW) or H1-A8 (NW/SE) by checking endpoints
        auto [start, end] = get_diagonal_endpoints(diag);
        if (file_of(start) <= file_of(end) && rank_of(start) <= rank_of(end))
        {
            a1h8_diagonals.push_back(diag);
        }
        else
        {
            h1a8_diagonals.push_back(diag);
        }
    }

    // Check A1-H8 diagonals
    ss << "A1-H8 direction:\n";

    // Open diagonals (no pawns)
    ss << "  Open: ";
    bool hasOpenDiag1 = false;
    for (const auto& diag : a1h8_diagonals)
    {
        if (!(allPawns & diag))
        {
            auto [start, end] = get_diagonal_endpoints(diag);
            ss << Trace::Helpers::square_to_string(start) << Trace::Helpers::square_to_string(end)
               << " ";
            hasOpenDiag1 = true;
        }
    }
    if (!hasOpenDiag1)
        ss << "none";
    ss << "\n";

    // Semi-open diagonals for white (no white pawns)
    ss << "  Semi-open White: ";
    bool hasSemiOpenWhiteDiag1 = false;
    for (const auto& diag : a1h8_diagonals)
    {
        if (!(whitePawns & diag) && (blackPawns & diag))
        {
            auto [start, end] = get_diagonal_endpoints(diag);
            ss << Trace::Helpers::square_to_string(start) << Trace::Helpers::square_to_string(end)
               << " ";
            hasSemiOpenWhiteDiag1 = true;
        }
    }
    if (!hasSemiOpenWhiteDiag1)
        ss << "none";
    ss << "\n";

    // Semi-open diagonals for black (no black pawns)
    ss << "  Semi-open Black: ";
    bool hasSemiOpenBlackDiag1 = false;
    for (const auto& diag : a1h8_diagonals)
    {
        if (!(blackPawns & diag) && (whitePawns & diag))
        {
            auto [start, end] = get_diagonal_endpoints(diag);
            ss << Trace::Helpers::square_to_string(start) << Trace::Helpers::square_to_string(end)
               << " ";
            hasSemiOpenBlackDiag1 = true;
        }
    }
    if (!hasSemiOpenBlackDiag1)
        ss << "none";
    ss << "\n";

    // Check H1-A8 diagonals
    ss << "H1-A8 direction:\n";

    // Open diagonals (no pawns)
    ss << "  Open: ";
    bool hasOpenDiag2 = false;
    for (const auto& diag : h1a8_diagonals)
    {
        if (!(allPawns & diag))
        {
            auto [start, end] = get_diagonal_endpoints(diag);
            ss << Trace::Helpers::square_to_string(start) << Trace::Helpers::square_to_string(end)
               << " ";
            hasOpenDiag2 = true;
        }
    }
    if (!hasOpenDiag2)
        ss << "none";
    ss << "\n";

    // Semi-open diagonals for white (no white pawns)
    ss << "  Semi-open White: ";
    bool hasSemiOpenWhiteDiag2 = false;
    for (const auto& diag : h1a8_diagonals)
    {
        if (!(whitePawns & diag) && (blackPawns & diag))
        {
            auto [start, end] = get_diagonal_endpoints(diag);
            ss << Trace::Helpers::square_to_string(start) << Trace::Helpers::square_to_string(end)
               << " ";
            hasSemiOpenWhiteDiag2 = true;
        }
    }
    if (!hasSemiOpenWhiteDiag2)
        ss << "none";
    ss << "\n";

    // Semi-open diagonals for black (no black pawns)
    ss << "  Semi-open Black: ";
    bool hasSemiOpenBlackDiag2 = false;
    for (const auto& diag : h1a8_diagonals)
    {
        if (!(blackPawns & diag) && (whitePawns & diag))
        {
            auto [start, end] = get_diagonal_endpoints(diag);
            ss << Trace::Helpers::square_to_string(start) << Trace::Helpers::square_to_string(end)
               << " ";
            hasSemiOpenBlackDiag2 = true;
        }
    }
    if (!hasSemiOpenBlackDiag2)
        ss << "none";
    ss << "\n\n";

    // =========================================================================
    // BLOCCO 2: OUTPUT AGGIORNATO (ADVANCED MAJOR PIECES MODULES)
    // =========================================================================
    ss << "================================================================\n";
    ss << "=== ADVANCED MAJOR PIECES MODULES ===\n\n";

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