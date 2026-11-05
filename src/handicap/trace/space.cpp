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

using namespace Trace;

namespace Eval {

std::string analyze_space(const Position& pos, int phase, uint8_t winProb) {
    std::stringstream ss;
    Trace::Helpers::append_term_row(ss, pos, phase, "Space", SPACE);
    ss << "=== SPACE SUBELEMENTS ===\n";

    // SPACE DETAIL BY AREA
    ss << "Space Detail by Area:\n";

    // Masks of the areas
    const Bitboard QueenSideFiles = FileABB | FileBBB | FileCBB;  // a-c
    const Bitboard CenterFiles    = FileDBB | FileEBB;            // d-e
    const Bitboard KingSideFiles  = FileFBB | FileGBB | FileHBB;  // f-h

    // Pawn attacks of a colour
    auto pawn_attacks_bb = [](Color c, Bitboard pawns) {
        return c == WHITE ? (shift<NORTH_WEST>(pawns) | shift<NORTH_EAST>(pawns))
                          : (shift<SOUTH_WEST>(pawns) | shift<SOUTH_EAST>(pawns));
    };

    // Space of a colour within a mask of files
    auto calculate_space_for_area = [&](Color us, Bitboard area_mask) {
        Color them = ~us;

        // Ranks depend on the colour
        Bitboard rank_mask =
          (us == WHITE) ? (Rank2BB | Rank3BB | Rank4BB) : (Rank5BB | Rank6BB | Rank7BB);
        Bitboard space_mask = area_mask & rank_mask;

        // Enemy pawn attacks
        Bitboard their_pawns        = pos.pieces(them, PAWN);
        Bitboard their_pawn_attacks = pawn_attacks_bb(them, their_pawns);

        // Safe squares: in the mask, not occupied by our pawns and not attacked by enemy pawns
        Bitboard safe = space_mask & ~pos.pieces(us, PAWN) & ~their_pawn_attacks;

        // Squares behind the pawns: up to three squares behind the friendly pawns
        Bitboard behind = pos.pieces(us, PAWN);
        if (us == WHITE)
        {
            behind |= shift<SOUTH>(behind);
            behind |= shift<SOUTH>(shift<SOUTH>(behind));
        }
        else
        {
            behind |= shift<NORTH>(behind);
            behind |= shift<NORTH>(shift<NORTH>(behind));
        }

        // Bonus: safe squares + safe squares behind the pawns not attacked by enemy pawns
        int bonus = popcount(safe) + popcount(behind & safe & ~their_pawn_attacks);

        return bonus;
    };

    // Compute it for every area and every colour
    int white_queen_side = calculate_space_for_area(WHITE, QueenSideFiles);
    int white_center     = calculate_space_for_area(WHITE, CenterFiles);
    int white_king_side  = calculate_space_for_area(WHITE, KingSideFiles);
    int black_queen_side = calculate_space_for_area(BLACK, QueenSideFiles);
    int black_center     = calculate_space_for_area(BLACK, CenterFiles);
    int black_king_side  = calculate_space_for_area(BLACK, KingSideFiles);

    // Differences
    int queen_side_diff = white_queen_side - black_queen_side;
    int center_diff     = white_center - black_center;
    int king_side_diff  = white_king_side - black_king_side;

    // Table format
    ss << "                        Queen side   Center   King side\n";
    ss << "White                      " << std::setw(3) << white_queen_side << "         "
       << std::setw(3) << white_center << "        " << std::setw(3) << white_king_side << "\n";
    ss << "Black                      " << std::setw(3) << black_queen_side << "         "
       << std::setw(3) << black_center << "        " << std::setw(3) << black_king_side << "\n";
    ss << "Difference White-Black     " << std::setw(3) << queen_side_diff << "         "
       << std::setw(3) << center_diff << "        " << std::setw(3) << king_side_diff << "\n";

    // Total of each colour as well
    int white_total_space = white_queen_side + white_center + white_king_side;
    int black_total_space = black_queen_side + black_center + black_king_side;
    ss << "Total Space: White " << white_total_space << " - Black " << black_total_space << "\n";

    // Strategic recommendations based on the control of space
    ss << "Space Recommendations:\n";

    // Side with the advantage and its strongest area
    int total_diff = white_total_space - black_total_space;
    if (total_diff > 0)
    {
        // White has the advantage
        ss << "White has a space advantage and must increase it on the ";
        // Area of largest advantage for White
        if (queen_side_diff >= center_diff && queen_side_diff >= king_side_diff)
        {
            ss << "queen side";
        }
        else if (center_diff >= queen_side_diff && center_diff >= king_side_diff)
        {
            ss << "center";
        }
        else
        {
            ss << "king side";
        }
    }
    else if (total_diff < 0)
    {
        // Black has the advantage
        ss << "Black has a space advantage and must increase it on the ";
        // Area of largest advantage for Black (smallest negative values)
        if (queen_side_diff <= center_diff && queen_side_diff <= king_side_diff)
        {
            ss << "queen side";
        }
        else if (center_diff <= queen_side_diff && center_diff <= king_side_diff)
        {
            ss << "center";
        }
        else
        {
            ss << "king side";
        }
    }
    else
    {
        ss << "Space is balanced - both players should fight for control of the center";
    }
    ss << "\n";

    // EXPANSION FACTOR (Shashin Theory) - kept unchanged
    ss << "Expansion Factor (Shashin Theory):\n";
    ss << "===================================\n";

    // Center of gravity of a colour
    auto calculate_center_of_gravity = [&](Color color) -> double {
        Bitboard pieces      = pos.pieces(color);
        double   sum         = 0.0;
        int      piece_count = 0;

        while (pieces)
        {
            Square s    = pop_lsb(pieces);
            Rank   rank = rank_of(s);

            // For White: rank1 = 1, rank2 = 2, ... rank8 = 8
            // For Black: rank8 = 1, rank7 = 2, ... rank1 = 8
            double rank_value = (color == WHITE) ? (rank + 1) : (8 - rank);

            sum += rank_value;
            piece_count++;
        }

        return piece_count > 0 ? sum / piece_count : 0.0;
    };

    // Centers of gravity
    double white_gravity   = calculate_center_of_gravity(WHITE);
    double black_gravity   = calculate_center_of_gravity(BLACK);
    double delta_expansion = white_gravity - black_gravity;

    // Show the results
    ss << "White Center of Gravity: " << std::fixed << std::setprecision(2) << white_gravity
       << "\n";
    ss << "Black Center of Gravity: " << std::fixed << std::setprecision(2) << black_gravity
       << "\n";
    ss << "Delta Expansion (White-Black): " << std::fixed << std::setprecision(2) << delta_expansion
       << "\n";

    // Only for Capablanca-type positions (win probability ~50%)
    if (winProb >= 45 && winProb <= 55)
    {
        ss << "Capablanca Expansion Recommendations:\n";
        ss << "====================================\n";

        ss << "Both players should:\n";
        ss << "1. Increase their own expansion factor\n";
        ss << "2. Decrease opponent's expansion factor\n\n";

        if (delta_expansion > 0.5)
        {
            ss << "White has expansion advantage - convert to lasting positional pressure\n";
            ss << "Black should challenge White's advanced pieces and seek exchanges\n";
        }
        else if (delta_expansion < -0.5)
        {
            ss << "Black has expansion advantage - convert to lasting positional pressure\n";
            ss << "White should challenge Black's advanced pieces and seek exchanges\n";
        }
        else
        {
            ss << "Expansion is balanced - focus on gradual improvement and piece activity\n";
        }

        ss << "Specific moves to consider:\n";
        ss << "- Advance pawns to gain space\n";
        ss << "- Place pieces on active squares\n";
        ss << "- Restrict opponent's piece mobility\n";
        ss << "- Create weaknesses in opponent's camp\n";
    }
    ss << "\n";

    // =========================================================================
    // [SC:trace] BLOCK 2: ADVANCED SPACE MODULES
    // =========================================================================
    ss << "================================================================\n";
    ss << "=== ADVANCED SPACE MODULES ===\n\n";
    Trace::Helpers::append_score_block(
      ss, pos, "Territorial Control Evaluation (Pawn advancement & Central footprint)", SPACE);

    const int netSpace = UCIEngine::to_cp(mg_value(scores[SPACE][WHITE] - scores[SPACE][BLACK]), pos);
    const bool crampedW = pos.count<ALL_PIECES>(WHITE) > 10 && netSpace < -8;
    const bool crampedB = pos.count<ALL_PIECES>(BLACK) > 10 && netSpace > 8;

    ss << "Territorial Diagnostic:\n";
    if (netSpace > 8)
        ss << "  - White holds a clear space advantage (+" << netSpace
           << " cp). Black's pieces have fewer safe squares behind the pawn chain.\n\n";
    else if (netSpace < -8)
        ss << "  - Black holds a clear space advantage (+" << -netSpace
           << " cp). White's pieces have fewer safe squares behind the pawn chain.\n\n";
    else
        ss << "  - Space and territorial control are roughly equal. Neither side is critically "
              "cramped.\n\n";

    ss << "--- OTB STRATEGIC SYNTHESIS ---\n";
    ss << "  * Theory : Space matters in proportion to the pieces on the board: the side with more "
          "space wants to keep pieces, the cramped side needs exchanges and a liberating pawn "
          "break.\n";
    ss << "  * Action : ";
    if (crampedB)
        ss << "Black is cramped: trade minor pieces and prepare a freeing break (...c5/...e5/...f5 "
              "type) before White builds up. White should avoid trades and prevent the break.\n";
    else if (crampedW)
        ss << "White is cramped: trade minor pieces and prepare a freeing break (c4/e4/f4 "
              "type) before Black builds up. Black should avoid trades and prevent the break.\n";
    else if (netSpace > 8)
        ss << "White should use the extra space to transfer pieces to the wing of attack faster "
              "than Black can defend it.\n";
    else if (netSpace < -8)
        ss << "Black should use the extra space to transfer pieces to the wing of attack faster "
              "than White can defend it.\n";
    else
        ss << "Fight for the central squares with pieces before committing the pawns.\n";
    ss << "\n";

    return ss.str();
}

}

}  // namespace Alexander