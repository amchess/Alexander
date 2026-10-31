#include "../../evaluate.h"
#include "trace.h"
#include "pawns_element.h" 
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

std::string analyze_pawns(const Position& pos, int phase) {
    (void)phase; // Silence unused parameter warning
    std::stringstream ss;
    ss << "=== PAWN STRUCTURE SUBELEMENTS ===\n";

    // 1. ENGINE NUMERICAL SCORES
    ScoreForClassical w_pawn = Trace::scores[PAWN][WHITE];
    ScoreForClassical b_pawn = Trace::scores[PAWN][BLACK];
    ScoreForClassical net_pawn = w_pawn - b_pawn;

    int mg_net = mg_value(net_pawn);
    int eg_net = eg_value(net_pawn);

    ss << "Engine Pawn Structure Evaluation (Penalties for Weaknesses, Space, etc.):\n"
       << "  White : " << std::setw(4) << mg_value(w_pawn) << " (MG) / " << std::setw(4) << eg_value(w_pawn) << " (EG)\n"
       << "  Black : " << std::setw(4) << mg_value(b_pawn) << " (MG) / " << std::setw(4) << eg_value(b_pawn) << " (EG)\n"
       << "  Net   : " << std::setw(4) << mg_net << " (MG) / " << std::setw(4) << eg_net << " (EG)\n\n";

    // 2. STRUCTURAL DETECTION ALGORITHMS
    auto find_pawn_islands = [&](Color color) -> std::vector<Bitboard> {
        Bitboard pawns = pos.pieces(color, PAWN);
        std::vector<bool> files_with_pawns(8, false);
        for (File f = FILE_A; f <= FILE_H; ++f) {
            if (pawns & file_bb(f)) files_with_pawns[f] = true;
        }
        std::vector<Bitboard> islands;
        for (int i = 0; i < 8; ++i) {
            if (files_with_pawns[i]) {
                Bitboard island = 0;
                while (i < 8 && files_with_pawns[i]) {
                    island |= pawns & file_bb(File(i));
                    i++;
                }
                islands.push_back(island);
            }
        }
        return islands;
    };
    std::vector<Bitboard> white_islands = find_pawn_islands(WHITE);
    std::vector<Bitboard> black_islands = find_pawn_islands(BLACK);

    auto find_doubled_pawns = [&](Color color) -> Bitboard {
        Bitboard pawns = pos.pieces(color, PAWN);
        Bitboard doubled = 0;
        for (File f = FILE_A; f <= FILE_H; ++f) {
            Bitboard file_pawns = pawns & file_bb(f);
            if (popcount(file_pawns) > 1) doubled |= file_pawns;
        }
        return doubled;
    };
    Bitboard white_doubled = find_doubled_pawns(WHITE);
    Bitboard black_doubled = find_doubled_pawns(BLACK);

    auto find_isolated_pawns = [&](Color color) -> Bitboard {
        Bitboard pawns = pos.pieces(color, PAWN);
        Bitboard isolated = 0;
        for (File f = FILE_A; f <= FILE_H; ++f) {
            Bitboard file_pawns = pawns & file_bb(f);
            if (file_pawns == 0) continue;
            bool has_adjacent = false;
            if (f > FILE_A) has_adjacent |= (pawns & file_bb(File(f - 1)));
            if (f < FILE_H) has_adjacent |= (pawns & file_bb(File(f + 1)));
            if (!has_adjacent) isolated |= file_pawns;
        }
        return isolated;
    };
    Bitboard white_isolated = find_isolated_pawns(WHITE);
    Bitboard black_isolated = find_isolated_pawns(BLACK);

    auto find_backward_pawns = [&](Color color) -> Bitboard {
        Color Them = ~color;
        Direction Up = pawn_push(color);
        Bitboard ourPawns = pos.pieces(color, PAWN);
        Bitboard theirPawns = pos.pieces(Them, PAWN);
        Bitboard backwardPawns = 0;
        Bitboard pawnsCopy = ourPawns;

        while (pawnsCopy) {
            Square s = pop_lsb(pawnsCopy);
            File f = file_of(s);
            Bitboard neighbours = ourPawns & adjacent_files_bb(s);
            bool lacksProtection = !(neighbours & forward_ranks_bb(Them, s + Up));
            bool isSemiOpen = true;
            Bitboard sameFilePawns = ourPawns & file_bb(f);
            
            if (color == WHITE) {
                if (sameFilePawns & forward_ranks_bb(WHITE, s)) isSemiOpen = false;
            } else {
                if (sameFilePawns & forward_ranks_bb(BLACK, s)) isSemiOpen = false;
            }

            bool blocked = (pos.piece_on(s + Up) == make_piece(Them, PAWN));
            Bitboard leverPush = (color == WHITE) ? (theirPawns & pawn_attacks_bb<WHITE>(s + Up)) : (theirPawns & pawn_attacks_bb<BLACK>(s + Up));
            bool vulnerable = blocked || leverPush;

            if (lacksProtection && isSemiOpen && vulnerable) {
                Bitboard stoppers = theirPawns & passed_pawn_span(color, s);
                Bitboard lever = (color == WHITE) ? (theirPawns & pawn_attacks_bb<WHITE>(s)) : (theirPawns & pawn_attacks_bb<BLACK>(s));
                if (!(!(stoppers ^ lever))) backwardPawns |= square_bb(s); 
            }
        }
        return backwardPawns;
    };
    Bitboard white_backward = find_backward_pawns(WHITE);
    Bitboard black_backward = find_backward_pawns(BLACK);

    auto find_hanging_pawns = [&](Color color) -> Bitboard {
        Bitboard pawns = pos.pieces(color, PAWN);
        Bitboard hanging = 0;
        Bitboard p = pawns;
        while (p) {
            Square s = pop_lsb(p);
            File f = file_of(s);
            Rank r = rank_of(s);
            if (f >= FILE_C && f <= FILE_E) {
                bool is_advanced = (color == WHITE) ? (r >= RANK_3) : (r <= RANK_6);
                if (is_advanced && (pawns & square_bb(Square(s + 1)))) {
                    bool has_left_outer = pawns & file_bb(File(f - 1));
                    bool has_right_outer = pawns & file_bb(File(f + 2));
                    if (!has_left_outer && !has_right_outer) {
                        hanging |= square_bb(s) | square_bb(Square(s + 1));
                    }
                }
            }
        }
        return hanging;
    };
    Bitboard white_hanging = find_hanging_pawns(WHITE);
    Bitboard black_hanging = find_hanging_pawns(BLACK);

    // 3. CENTER TYPE ANALYSIS
    auto analyze_center_type = [&]() -> std::string {
        Square d4 = SQ_D4, e4 = SQ_E4, d5 = SQ_D5, e5 = SQ_E5;
        Square d3 = SQ_D3, e3 = SQ_E3, c3 = SQ_C3, f3 = SQ_F3;
        Square d6 = SQ_D6, e6 = SQ_E6, c6 = SQ_C6, f6 = SQ_F6;

        Piece wp = W_PAWN, bp = B_PAWN;
        bool w_d4 = pos.piece_on(d4) == wp; bool w_e4 = pos.piece_on(e4) == wp;
        bool w_d5 = pos.piece_on(d5) == wp; bool w_e5 = pos.piece_on(e5) == wp;
        bool w_d3 = pos.piece_on(d3) == wp; bool w_e3 = pos.piece_on(e3) == wp;
        bool w_c3 = pos.piece_on(c3) == wp; bool w_f3 = pos.piece_on(f3) == wp;

        bool b_d4 = pos.piece_on(d4) == bp; bool b_e4 = pos.piece_on(e4) == bp;
        bool b_d5 = pos.piece_on(d5) == bp; bool b_e5 = pos.piece_on(e5) == bp;
        bool b_d6 = pos.piece_on(d6) == bp; bool b_e6 = pos.piece_on(e6) == bp;
        bool b_c6 = pos.piece_on(c6) == bp; bool b_f6 = pos.piece_on(f6) == bp;

        int w_center = (w_d4?1:0) + (w_e4?1:0) + (w_d5?1:0) + (w_e5?1:0);
        int b_center = (b_d4?1:0) + (b_e4?1:0) + (b_d5?1:0) + (b_e5?1:0);

        if (w_center == 0 && b_center == 0) return "Open Center";

        Bitboard w_center_pawns = pos.pieces(WHITE, PAWN) & (FileDBB | FileEBB);
        Bitboard b_center_pawns = pos.pieces(BLACK, PAWN) & (FileDBB | FileEBB);
        bool central_tension = (pawn_attacks_bb<WHITE>(w_center_pawns) & b_center_pawns) || 
                               (pawn_attacks_bb<BLACK>(b_center_pawns) & w_center_pawns);

        if (!central_tension && ((w_d4 && b_d5) || (w_e4 && b_e5) || (w_d5 && b_d6) || (w_e5 && b_e6))) {
            if ((w_center_pawns & file_bb(FILE_D)) && (w_center_pawns & file_bb(FILE_E)) &&
                (b_center_pawns & file_bb(FILE_D)) && (b_center_pawns & file_bb(FILE_E))) {
                if ((w_d4 && w_e5 && b_d5 && b_e6) || (b_d4 && b_e5 && w_d5 && w_e3)) return "Closed Center (French Chain)";
                if ((w_e4 && w_d5 && b_e5 && b_d6) || (b_e4 && b_d5 && w_e5 && w_d3)) return "Closed Center (King's Indian Chain)";
                return "Closed Center";
            }
        }

        auto no_immediate_tension = [&](Bitboard pairNeighbourFiles) -> bool {
            return !(pos.pieces(WHITE, PAWN) & pairNeighbourFiles & (Rank3BB | Rank4BB)) &&
                   !(pos.pieces(BLACK, PAWN) & pairNeighbourFiles & (Rank6BB | Rank5BB));
        };

        if ((w_d4 && b_d5) && !(w_e4 || b_e4 || w_e5 || b_e5) && no_immediate_tension(FileCBB | FileEBB)) return "Static Center (d4-d5)";
        if ((w_e4 && b_e5) && !(w_d4 || b_d4 || w_d5 || b_d5) && no_immediate_tension(FileDBB | FileFBB)) return "Static Center (e4-e5)";
        if ((w_d4 && (w_e3 || w_c3)) || (w_e4 && (w_d3 || w_f3))) return "Mobile Center";
        if ((b_d5 && (b_e6 || b_c6)) || (b_e5 && (b_d6 || b_f6))) return "Mobile Center";

        Bitboard w_pawns = pos.pieces(WHITE, PAWN);
        Bitboard b_pawns = pos.pieces(BLACK, PAWN);
        bool w_has_iqp = w_d4 && (popcount(w_pawns & FileDBB) == 1) && !(w_pawns & (FileCBB | FileEBB)) && !(b_pawns & FileDBB);
        bool b_has_iqp = b_d5 && (popcount(b_pawns & FileDBB) == 1) && !(b_pawns & (FileCBB | FileEBB)) && !(w_pawns & FileDBB);
        
        if (w_has_iqp || b_has_iqp) return "Dynamic Center (Isolated Queen's Pawn)";
        if (white_hanging || black_hanging) return "Dynamic Center (Hanging Pawns)";

        return "Dynamic Center";
    };
    std::string center_type = analyze_center_type();

    // 4. MASTER PAWN STRUCTURE SUMMARY
    int w_weaknesses = popcount(white_doubled) + popcount(white_isolated) + popcount(white_backward) + popcount(white_hanging);
    int b_weaknesses = popcount(black_doubled) + popcount(black_isolated) + popcount(black_backward) + popcount(black_hanging);
    int w_isls = white_islands.size();
    int b_isls = black_islands.size();

    ss << "========================================\n"
       << "   MASTER PAWN STRUCTURE SUMMARY\n"
       << "========================================\n";
       
    // Part A: Weaknesses Audit
    ss << "1. Weakness Audit:\n"
       << "  - White Weaknesses : " << w_weaknesses << " (" << w_isls << " Pawn Islands)\n"
       << "  - Black Weaknesses : " << b_weaknesses << " (" << b_isls << " Pawn Islands)\n";
       
    if (w_weaknesses > b_weaknesses) ss << "  * Verdict: Black possesses the structurally superior position.\n\n";
    else if (b_weaknesses > w_weaknesses) ss << "  * Verdict: White possesses the structurally superior position.\n\n";
    else ss << "  * Verdict: Structural integrity is balanced.\n\n";

    // Part B: Central Profiling
    ss << "2. Central Profiling:\n"
       << "  - Topography: " << center_type << "\n";
       
    if (center_type == "Open Center") ss << "  * Strategy: Tactics dominate. Maximize piece activity and centralization.\n\n";
    else if (center_type.find("Closed Center") != std::string::npos) ss << "  * Strategy: Flank attacks dictate play. Play on the side where your pawn chain points.\n\n";
    else if (center_type.find("Static Center") != std::string::npos) ss << "  * Strategy: Deep maneuvering phase. Improve piece placement behind the stable center.\n\n";
    else if (center_type == "Mobile Center") ss << "  * Strategy: Control the space. The side with the mobile center must prepare to push; the defender must blockade.\n\n";
    else ss << "  * Strategy: Asymmetrical central tension requires concrete calculation over general rules.\n\n";

    // Part C: Endgame Transition Directive (With Passed Pawn Override)
    ss << "3. Endgame Transition Directive (Ablation):\n";
    ScoreForClassical net_pass = Trace::scores[Trace::PASSED][WHITE] - Trace::scores[Trace::PASSED][BLACK];
    int pass_eg = eg_value(net_pass);

    if (pass_eg > 30) {
        ss << "  - OVERRIDE (PASSED PAWN): Transition heavily favors WHITE. White's passed pawn (+" << pass_eg << " EG) is the dominant factor. Black must avoid trades regardless of pawn islands.\n";
    } else if (pass_eg < -30) {
        ss << "  - OVERRIDE (PASSED PAWN): Transition heavily favors BLACK. Black's passed pawn (" << std::abs(pass_eg) << " EG) is the dominant factor. White must avoid trades regardless of pawn islands.\n";
    } 
    else if (w_isls > b_isls + 1 || w_weaknesses > b_weaknesses + 1) {
        ss << "  - Favors BLACK. White's pawn structure is fragmented. Black's primary strategic goal should be forcing piece trades to enter a winning endgame.\n";
    } else if (b_isls > w_isls + 1 || b_weaknesses > w_weaknesses + 1) {
        ss << "  - Favors WHITE. Black's pawn structure is fragmented. White's primary strategic goal should be forcing piece trades to enter a winning endgame.\n";
    } else {
        ss << "  - Neutral. The pawn structures are similarly robust. Forced piece trades will not yield a significant structural advantage based on pawns alone.\n";
    }
    ss << "========================================\n\n";

    // 5. DYNAMIC OTB STRATEGIC SYNTHESIS
    ss << "--- OTB STRATEGIC SYNTHESIS ---\n";
    
    // Theory Branch
    if (center_type.find("Isolated Queen's Pawn") != std::string::npos) {
        ss << "  * Theory : The IQP grants a spatial advantage and attacking chances in the middlegame, but becomes a severe liability in the endgame due to the inability to be defended by pawns.\n";
    } else if (white_hanging || black_hanging) {
        ss << "  * Theory : Hanging pawns control central squares but are inherently unstable. They must either advance to create a passed pawn or be blockaded and destroyed.\n";
    } else if (w_weaknesses > 0 || b_weaknesses > 0) {
        ss << "  * Theory : Every backward or isolated pawn creates a 'permanent outpost' in front of it for enemy pieces, severely restricting your own pieces which must passively defend it.\n";
    } else {
        ss << "  * Theory : A solid pawn structure dictates that play must proceed on the flanks or through piece maneuvering, as there are no permanent targets to attack.\n";
    }

    // Action Branch
    ss << "  * Action : ";
    if (white_isolated && center_type.find("Isolated Queen's Pawn") != std::string::npos) {
        ss << "White has the IQP. White must attack on the kingside and avoid trades. Black must blockade the d4 square (preferably with a Knight) and aggressively force piece trades.\n";
    } else if (black_isolated && center_type.find("Isolated Queen's Pawn") != std::string::npos) {
        ss << "Black has the IQP. Black must attack on the kingside and avoid trades. White must blockade the d5 square (preferably with a Knight) and aggressively force piece trades.\n";
    } else if (white_hanging) {
        ss << "White has Hanging Pawns. White must use their central space to generate a kingside attack. Black must relentlessly attack the pawns and force them to advance prematurely.\n";
    } else if (black_hanging) {
        ss << "Black has Hanging Pawns. Black must use their central space to generate a kingside attack. White must relentlessly attack the pawns and force them to advance prematurely.\n";
    } else if (white_backward && !black_backward) {
        ss << "White has a backward pawn. Black should pile heavy pieces onto the semi-open file in front of it and establish an iron grip on the square directly ahead of it.\n";
    } else if (black_backward && !white_backward) {
        ss << "Black has a backward pawn. White should pile heavy pieces onto the semi-open file in front of it and establish an iron grip on the square directly ahead of it.\n";
    } else if (w_isls > b_isls) {
        ss << "White's pawn structure is fragmented (more islands). White must avoid the endgame at all costs and create tactical chaos. Black should calmly trade off pieces.\n";
    } else if (b_isls > w_isls) {
        ss << "Black's pawn structure is fragmented (more islands). Black must avoid the endgame at all costs and create tactical chaos. White should calmly trade off pieces.\n";
    } else {
        ss << "Structure is balanced. Focus on controlling the center according to the " << center_type << " topographic profile.\n";
    }

    ss << "\n";
    return ss.str();
}

} // namespace Eval
} // namespace Alexander