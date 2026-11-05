#include "../../evaluate.h"
#include "trace.h"
#include "pawns_element.h"
#include "../evaluate_handicap.h"
#include "../../uci.h"
#include "../../movegen.h"
#include "../../bitboard.h"
#include <sstream>
#include <vector>

namespace Alexander {

using namespace Trace;

namespace Eval {

std::string analyze_pawns(const Position& pos, int phase) {
    std::stringstream ss;
    Trace::Helpers::append_term_row(ss, pos, phase, "Pawns", PAWN);
    ss << "=== PAWNS SUBELEMENTS ===\n";
    // PAWN STRUCTURE ANALYSIS
    ss << "Pawn Structure Analysis:\n";
    ss << "========================\n";

    auto bitboard_to_squares = [&](Bitboard b) -> std::string {
        std::string result;
        while (b)
        {
            Square s = pop_lsb(b);
            result += Trace::Helpers::square_to_string(s) + " ";
        }
        return result.empty() ? "none" : result;
    };

    // a. PAWN ISLANDS
    auto find_pawn_islands = [&](Color color) -> std::vector<Bitboard> {
        Bitboard          pawns = pos.pieces(color, PAWN);
        std::vector<bool> files_with_pawns(8, false);
        for (File f = FILE_A; f <= FILE_H; ++f)
        {
            if (pawns & file_bb(f))
            {
                files_with_pawns[f] = true;
            }
        }

        std::vector<Bitboard> islands;
        for (int i = 0; i < 8; ++i)
        {
            if (files_with_pawns[i])
            {
                Bitboard island = 0;
                while (i < 8 && files_with_pawns[i])
                {
                    island |= pawns & file_bb(File(i));
                    i++;
                }
                islands.push_back(island);
            }
        }
        return islands;
    };

    ss << "Pawn Islands:\n";
    std::vector<Bitboard> white_islands = find_pawn_islands(WHITE);
    std::vector<Bitboard> black_islands = find_pawn_islands(BLACK);
    ss << "White: " << white_islands.size() << " islands\n";
    for (size_t i = 0; i < white_islands.size(); ++i)
    {
        ss << "  Island " << (i + 1) << ": " << bitboard_to_squares(white_islands[i]) << "\n";
    }
    ss << "Black: " << black_islands.size() << " islands\n";
    for (size_t i = 0; i < black_islands.size(); ++i)
    {
        ss << "  Island " << (i + 1) << ": " << bitboard_to_squares(black_islands[i]) << "\n";
    }

    // b. DOUBLED PAWNS
    auto find_doubled_pawns = [&](Color color) -> Bitboard {
        Bitboard pawns   = pos.pieces(color, PAWN);
        Bitboard doubled = 0;

        for (File f = FILE_A; f <= FILE_H; ++f)
        {
            Bitboard file_pawns = pawns & file_bb(f);
            if (popcount(file_pawns) > 1)
            {
                doubled |= file_pawns;
            }
        }
        return doubled;
    };

    ss << "\nDoubled Pawns:\n";
    Bitboard white_doubled = find_doubled_pawns(WHITE);
    Bitboard black_doubled = find_doubled_pawns(BLACK);
    ss << "White: " << bitboard_to_squares(white_doubled) << "\n";
    ss << "Black: " << bitboard_to_squares(black_doubled) << "\n";

    // c. ISOLATED PAWNS
    auto find_isolated_pawns = [&](Color color) -> Bitboard {
        Bitboard pawns    = pos.pieces(color, PAWN);
        Bitboard isolated = 0;

        for (File f = FILE_A; f <= FILE_H; ++f)
        {
            Bitboard file_pawns = pawns & file_bb(f);
            if (file_pawns == 0)
                continue;

            bool has_adjacent_friends = false;
            if (f > FILE_A)
                has_adjacent_friends |= (pawns & file_bb(File(f - 1)));
            if (f < FILE_H)
                has_adjacent_friends |= (pawns & file_bb(File(f + 1)));

            if (!has_adjacent_friends)
            {
                isolated |= file_pawns;
            }
        }
        return isolated;
    };

    ss << "\nIsolated Pawns:\n";
    Bitboard white_isolated = find_isolated_pawns(WHITE);
    Bitboard black_isolated = find_isolated_pawns(BLACK);
    ss << "White: " << bitboard_to_squares(white_isolated) << "\n";
    ss << "Black: " << bitboard_to_squares(black_isolated) << "\n";

    // d. BACKWARD PAWNS - COMPLETE DEFINITION (without warnings)
    auto find_backward_pawns = [&](Color color) -> Bitboard {
        Color     Them = ~color;
        Direction Up   = pawn_push(color);

        Bitboard ourPawns      = pos.pieces(color, PAWN);
        Bitboard theirPawns    = pos.pieces(Them, PAWN);
        Bitboard backwardPawns = 0;

        // For every pawn
        Bitboard pawnsCopy = ourPawns;
        while (pawnsCopy)
        {
            Square s = pop_lsb(pawnsCopy);
            File   f = file_of(s);

            // CONDITION 1: no protection by friendly pawns
            Bitboard neighbours           = ourPawns & adjacent_files_bb(s);
            bool     hasForwardNeighbours = neighbours & forward_ranks_bb(Them, s + Up);

            // Simplified condition: no friendly pawn can protect it by advancing
            // (the pawns on the adjacent files are further advanced or missing)
            bool lacksProtection = !hasForwardNeighbours;

            // CONDITION 2: vulnerability on a (half-)open file
            bool isSemiOpen = true;  // half-open file (no friendly pawn in front)

            // Check the file of the pawn
            Bitboard sameFilePawns = ourPawns & file_bb(f);

            // Half-open file: no friendly pawn in front
            if (color == WHITE)
            {
                Bitboard aheadMask = forward_ranks_bb(WHITE, s);
                if (sameFilePawns & aheadMask)
                {
                    isSemiOpen = false;
                }
            }
            else
            {
                Bitboard aheadMask = forward_ranks_bb(BLACK, s);
                if (sameFilePawns & aheadMask)
                {
                    isSemiOpen = false;
                }
            }

            // It is on a (half-)open file
            bool onOpenOrSemiOpen = isSemiOpen;

            // ADDITIONAL CONDITION: vulnerable to blockade/attack
            bool blocked = (pos.piece_on(s + Up) == make_piece(Them, PAWN));

            // leverPush (enemy pawn attack on the square in front)
            Bitboard leverPush = 0;
            if (color == WHITE)
            {
                leverPush = theirPawns & pawn_attacks_bb<WHITE>(s + Up);
            }
            else
            {
                leverPush = theirPawns & pawn_attacks_bb<BLACK>(s + Up);
            }

            bool vulnerable = blocked || leverPush;

            // COMPLETE DEFINITION: a pawn is backward if:
            // 1. No friendly pawn can protect it (they are further advanced or missing)
            // 2. It is on a half-open or open file
            // 3. It is vulnerable (blocked or threatened)
            bool isBackward = lacksProtection && onOpenOrSemiOpen && vulnerable;

            // Exclude the passed pawns
            if (isBackward)
            {
                Bitboard stoppers = theirPawns & passed_pawn_span(color, s);
                Bitboard lever    = 0;
                if (color == WHITE)
                {
                    lever = theirPawns & pawn_attacks_bb<WHITE>(s);
                }
                else
                {
                    lever = theirPawns & pawn_attacks_bb<BLACK>(s);
                }

                bool passed = !(stoppers ^ lever);

                if (!passed)
                {
                    backwardPawns |= square_bb(s);
                }
            }
        }

        return backwardPawns;
    };

    ss << "\nBackward Pawns:\n";
    Bitboard white_backward = find_backward_pawns(WHITE);
    Bitboard black_backward = find_backward_pawns(BLACK);
    ss << "White: " << bitboard_to_squares(white_backward) << "\n";
    ss << "Black: " << bitboard_to_squares(black_backward) << "\n";

    // e. HANGING PAWNS - CORRECT, CENTRAL DEFINITION
    auto find_hanging_pawns = [&](Color color) -> Bitboard {
        Bitboard pawns   = pos.pieces(color, PAWN);
        Bitboard hanging = 0;
        Bitboard p       = pawns;

        while (p)
        {
            Square s = pop_lsb(p);
            File   f = file_of(s);
            Rank   r = rank_of(s);

            // 1. They must be central or semi-central pawns (files c, d, e).
            // If f is FILE_C the pair is c+d; if it is FILE_D, d+e.
            if (f >= FILE_C && f <= FILE_E)
            {
                // 2. They must be advanced (not on their starting ranks)
                bool is_advanced = (color == WHITE) ? (r >= RANK_3) : (r <= RANK_6);

                // 3. The partner must be exactly on the right
                if (is_advanced && (pawns & square_bb(Square(s + 1))))
                {
                    // A central phalanx: now check the outer files.
                    // Since f is between C and E, the outer limits are always valid (B and F/G)
                    File left_outer = File(f - 1);
                    File right_outer =
                      File(f + 2);  // f+1 is the other pawn, f+2 the outer file on the right

                    bool has_left_outer_pawn  = pawns & file_bb(left_outer);
                    bool has_right_outer_pawn = pawns & file_bb(right_outer);

                    // Without friendly pawns on the outer files they are true central hanging pawns
                    if (!has_left_outer_pawn && !has_right_outer_pawn)
                    {
                        hanging |= square_bb(s) | square_bb(Square(s + 1));
                    }
                }
            }
        }
        return hanging;
    };

    ss << "\nHanging Pawns:\n";
    Bitboard white_hanging = find_hanging_pawns(WHITE);
    Bitboard black_hanging = find_hanging_pawns(BLACK);
    ss << "White: " << bitboard_to_squares(white_hanging) << "\n";
    ss << "Black: " << bitboard_to_squares(black_hanging) << "\n";

    // Counts the pairs of hanging pawns
    auto count_hanging_pairs = [&](Color color) -> int {
        Bitboard hanging = find_hanging_pawns(color);
        int      pairs   = 0;

        // Check all the pairs of adjacent files
        for (int i = FILE_A; i <= FILE_G; i++)
        {
            File f      = File(i);
            File next_f = File(i + 1);

            Bitboard pawns_on_f    = hanging & file_bb(f);
            Bitboard pawns_on_next = hanging & file_bb(next_f);

            if (pawns_on_f && pawns_on_next)
            {
                pairs++;
                i++;  // Skip the next file
            }
        }
        return pairs;
    };

    int white_pairs = count_hanging_pairs(WHITE);
    int black_pairs = count_hanging_pairs(BLACK);
    ss << "Hanging pawn pairs: White " << white_pairs << ", Black " << black_pairs << "\n";

    // f. WEAK SQUARES
    auto find_weak_squares = [&](Color color) -> Bitboard {
        Color    opponent    = ~color;
        Bitboard our_pawns   = pos.pieces(color, PAWN);
        Bitboard their_pawns = pos.pieces(opponent, PAWN);

        // Pawn attacks
        Bitboard their_pawn_attacks = (opponent == WHITE) ? pawn_attacks_bb<WHITE>(their_pawns)
                                                          : pawn_attacks_bb<BLACK>(their_pawns);

        Bitboard our_pawn_attacks =
          (color == WHITE) ? pawn_attacks_bb<WHITE>(our_pawns) : pawn_attacks_bb<BLACK>(our_pawns);

        // Weak squares are those not defended by friendly pawns
        Bitboard undefended_by_pawns = ~our_pawn_attacks;
        Bitboard potential_weak      = undefended_by_pawns & ~our_pawns;

        // Squares in the opponent's half that are attacked count as weak
        Bitboard weak_candidates = potential_weak & their_pawn_attacks;

        // Add the undefended squares on the first 3 ranks of the colour
        Bitboard home_territory =
          (color == WHITE) ? (Rank1BB | Rank2BB | Rank3BB) : (Rank8BB | Rank7BB | Rank6BB);
        weak_candidates |= potential_weak & home_territory;

        return weak_candidates;
    };

    ss << "\nWeak Squares (not defended by pawns):\n";
    Bitboard white_weak = find_weak_squares(WHITE);
    Bitboard black_weak = find_weak_squares(BLACK);
    ss << "White: " << bitboard_to_squares(white_weak) << "\n";
    ss << "Black: " << bitboard_to_squares(black_weak) << "\n";

    // STRUCTURAL SUMMARY
    ss << "\nPawn Structure Summary:\n";
    ss << "=======================\n";
    ss << "White pawn weaknesses: "
       << popcount(white_doubled) + popcount(white_isolated) + popcount(white_backward)
            + popcount(white_hanging)
       << " (Doubled: " << popcount(white_doubled) << ", Isolated: " << popcount(white_isolated)
       << ", Backward: " << popcount(white_backward) << ", Hanging: " << popcount(white_hanging)
       << ")\n";

    ss << "Black pawn weaknesses: "
       << popcount(black_doubled) + popcount(black_isolated) + popcount(black_backward)
            + popcount(black_hanging)
       << " (Doubled: " << popcount(black_doubled) << ", Isolated: " << popcount(black_isolated)
       << ", Backward: " << popcount(black_backward) << ", Hanging: " << popcount(black_hanging)
       << ")\n";

    // CENTER TYPE ANALYSIS (based on the systematic classification of the PDF)
    ss << "Center Type Analysis:\n";
    ss << "=====================\n";

    // Precise definition of the center types, based on the PDF
    auto analyze_center_type = [&]() -> std::string {
        Square d4_center = SQ_D4, e4_center = SQ_E4, d5_center = SQ_D5, e5_center = SQ_E5;
        Square d3 = SQ_D3, e3 = SQ_E3, c3 = SQ_C3, f3 = SQ_F3;
        Square d6 = SQ_D6, e6 = SQ_E6, c6 = SQ_C6, f6 = SQ_F6;

        Piece w_pawn = W_PAWN, b_pawn = B_PAWN;

        bool w_d4 = pos.piece_on(d4_center) == w_pawn;
        bool w_e4 = pos.piece_on(e4_center) == w_pawn;
        bool w_d5 = pos.piece_on(d5_center) == w_pawn;
        bool w_e5 = pos.piece_on(e5_center) == w_pawn;
        bool w_d3 = pos.piece_on(d3) == w_pawn;
        bool w_e3 = pos.piece_on(e3) == w_pawn;
        bool w_c3 = pos.piece_on(c3) == w_pawn;
        bool w_f3 = pos.piece_on(f3) == w_pawn;

        bool b_d4 = pos.piece_on(d4_center) == b_pawn;
        bool b_e4 = pos.piece_on(e4_center) == b_pawn;
        bool b_d5 = pos.piece_on(d5_center) == b_pawn;
        bool b_e5 = pos.piece_on(e5_center) == b_pawn;
        bool b_d6 = pos.piece_on(d6) == b_pawn;
        bool b_e6 = pos.piece_on(e6) == b_pawn;
        bool b_c6 = pos.piece_on(c6) == b_pawn;
        bool b_f6 = pos.piece_on(f6) == b_pawn;

        // Count the central pawns of each colour
        int white_center_count = (w_d4 ? 1 : 0) + (w_e4 ? 1 : 0) + (w_d5 ? 1 : 0) + (w_e5 ? 1 : 0);
        int black_center_count = (b_d4 ? 1 : 0) + (b_e4 ? 1 : 0) + (b_d5 ? 1 : 0) + (b_e5 ? 1 : 0);

        // 1. OPEN center: no pawn on the 4 central squares
        if (white_center_count == 0 && black_center_count == 0)
        {
            return "Open Center";
        }

        // 2. CLOSED center: pawn chains blocked in the center WITHOUT tension
        Bitboard w_center_pawns = pos.pieces(WHITE, PAWN) & (FileDBB | FileEBB);
        Bitboard b_center_pawns = pos.pieces(BLACK, PAWN) & (FileDBB | FileEBB);

        Bitboard w_center_attacks = pawn_attacks_bb<WHITE>(w_center_pawns);
        Bitboard b_center_attacks = pawn_attacks_bb<BLACK>(b_center_pawns);

        // There is tension if the central pawns can capture each other
        bool central_tension =
          (w_center_attacks & b_center_pawns) || (b_center_attacks & w_center_pawns);

        bool closed_center = false;

        // With tension it is NOT a closed center.
        if (!central_tension)
        {
            // Check that there is at least one frontal blockade
            if ((w_d4 && b_d5) || (w_e4 && b_e5) || (w_d5 && b_d6) || (w_e5 && b_e6))
            {
                // A true closed center needs pawns of both colours on both central files
                if ((w_center_pawns & file_bb(FILE_D)) && (w_center_pawns & file_bb(FILE_E))
                    && (b_center_pawns & file_bb(FILE_D)) && (b_center_pawns & file_bb(FILE_E)))
                {
                    closed_center = true;
                }
            }
        }

        if (closed_center)
        {
            // French structure: White d4, e5 vs Black d5, e6
            if ((w_d4 && w_e5 && b_d5 && b_e6) || (b_d4 && b_e5 && w_d5 && w_e3))
            {
                return "Closed Center (French Chain)";
            }
            // King's Indian structure: White e4, d5 vs Black e5, d6
            if ((w_e4 && w_d5 && b_e5 && b_d6) || (b_e4 && b_d5 && w_e5 && w_d3))
            {
                return "Closed Center (King's Indian Chain)";
            }
            return "Closed Center";
        }

        // 3. STATIC center: a pair of blocked central pawns (d4-d5 or e4-e5) without
        //    immediate tension: no pawn, on the files next to the pair, that attacks it
        //    already (c4/e4, c5/e5 for d4-d5; d4/f4, d5/f5 for e4-e5) or can attack it in one step
        //    (c3/e3, c6/e6; d3/f3, d6/f6).
        //    The previous check looked at all the white pawns on ranks 3/4 and all the black ones
        //    on ranks 5/6, the pair included: d4 (e4) is on rank 4, so the static center
        //    was never detected.
        auto no_immediate_tension = [&](Bitboard pairNeighbourFiles) -> bool {
            return !(pos.pieces(WHITE, PAWN) & pairNeighbourFiles & (Rank3BB | Rank4BB))
                && !(pos.pieces(BLACK, PAWN) & pairNeighbourFiles & (Rank6BB | Rank5BB));
        };

        if ((w_d4 && b_d5) && !(w_e4 || b_e4 || w_e5 || b_e5)
            && no_immediate_tension(FileCBB | FileEBB))
        {
            return "Static Center (d4-d5)";
        }

        if ((w_e4 && b_e5) && !(w_d4 || b_d4 || w_d5 || b_d5)
            && no_immediate_tension(FileDBB | FileFBB))
        {
            return "Static Center (e4-e5)";
        }

        // 4. MOBILE center: a central pawn supported by another one
        if ((w_d4 && (w_e3 || w_c3)) || (w_e4 && (w_d3 || w_f3)))
        {
            return "Mobile Center";
        }
        if ((b_d5 && (b_e6 || b_c6)) || (b_e5 && (b_d6 || b_f6)))
        {
            return "Mobile Center";
        }

        // 5. DYNAMIC center: an IQP or hanging pawns

        Bitboard w_pawns = pos.pieces(WHITE, PAWN);
        Bitboard b_pawns = pos.pieces(BLACK, PAWN);

        // IQP check (isolated queen pawn)
        // Strict definition with absolute bitboard masks:
        // 1. The friendly pawn must be on d4 (White) or d5 (Black).
        // 2. It must be the ONLY friendly pawn on the d-file.
        // 3. There must be NO friendly pawns on the c- and e-files.
        // 4. There must be NO enemy pawns on the d-file (the file must be half-open).

        bool w_has_iqp = w_d4 && (popcount(w_pawns & FileDBB) == 1)
                      && !(w_pawns & (FileCBB | FileEBB)) && !(b_pawns & FileDBB);

        bool b_has_iqp = b_d5 && (popcount(b_pawns & FileDBB) == 1)
                      && !(b_pawns & (FileCBB | FileEBB)) && !(w_pawns & FileDBB);

        if (w_has_iqp || b_has_iqp)
        {
            return "Dynamic Center (Isolated Queen's Pawn)";
        }

        // Hanging pawns, using the function defined above
        if (find_hanging_pawns(WHITE) || find_hanging_pawns(BLACK))
        {
            return "Dynamic Center (Hanging Pawns)";
        }

        // Hanging pawns, using the function defined above
        if (find_hanging_pawns(WHITE) || find_hanging_pawns(BLACK))
        {
            return "Dynamic Center (Hanging Pawns)";
        }

        // If none of the cases above applies, return the generic "Dynamic Center"
        return "Dynamic Center";
    };

    std::string center_type = analyze_center_type();
    ss << "Center Type: " << center_type << "\n";

    // Explanation of the center type
    ss << "\nCenter Characteristics:\n";
    if (center_type == "Open Center")
    {
        ss << "- No pawns in the center (d4, e4, d5, e5)\n";
        ss << "- Maximum piece mobility\n";
        ss << "- Tactical play dominates\n";
        ss << "- Bishops and queens are strong\n";
    }
    else if (center_type.find("Closed Center") != std::string::npos)
    {
        ss << "- Both players have pawn chains in center\n";
        ss << "- Limited piece mobility through center\n";
        ss << "- Play typically develops on the wings\n";
        ss << "- Knights often better than bishops\n";
    }
    else if (center_type.find("Static Center") != std::string::npos)
    {
        ss << "- Stable pawn structure with opposed pawns in one file\n";
        ss << "- Strategic maneuvering game\n";
        ss << "- Piece placement is crucial\n";
        ss << "- Both players have some central control\n";
    }
    else if (center_type == "Mobile Center")
    {
        ss << "- One player has a mobile pawn center\n";
        ss << "- Space advantage for the controlling side\n";
        ss << "- Potential for pawn expansion\n";
        ss << "- Controlling side should advance, other should block\n";
    }
    else if (center_type.find("Dynamic Center") != std::string::npos)
    {
        ss << "- Dynamic imbalance between static and dynamic advantages\n";
        ss << "- Tactical opportunities common\n";
        ss << "- Initiative and piece activity are key\n";
        if (center_type.find("Isolated Queen's Pawn") != std::string::npos)
        {
            ss << "- Features an Isolated Queen's Pawn (IQP)\n";
        }
        if (center_type.find("Hanging Pawns") != std::string::npos)
        {
            ss << "- Features Hanging Pawns\n";
        }
    }

    // Detail of the central pawns
    ss << "\nCentral Pawns (d4, e4, d5, e5):\n";
    Bitboard central_squares = (FileDBB | FileEBB) & (Rank4BB | Rank5BB);
    Bitboard central_white   = pos.pieces(WHITE, PAWN) & central_squares;
    Bitboard central_black   = pos.pieces(BLACK, PAWN) & central_squares;
    ss << "White: " << bitboard_to_squares(central_white) << "\n";
    ss << "Black: " << bitboard_to_squares(central_black) << "\n";

    // RACCOMANDAZIONI STRATEGICHE PRECISE
    ss << "\nStrategic Recommendations:\n";
    ss << "==========================\n";

    if (center_type == "Open Center")
    {
        ss << "- Attack the center and try to occupy it with your pieces\n";
        ss << "- Use your powerful central position to attack on the wing\n";
        ss << "- Avoid pawn moves without careful consideration\n";
        ss << "- Tactical play dominates - focus on concrete variations\n";
        ss << "- Bishops and rooks are particularly valuable\n";
    }
    else if (center_type.find("Closed Center") != std::string::npos)
    {
        ss << "- Attack on the wing where you have more activity\n";
        ss << "- Use pawns to create attacks (pawn storms)\n";
        ss << "- Knights are usually superior to bishops\n";
        ss << "- Defender can advance pawns to block attacks\n";
        ss << "- Counterattack on the opposite wing if necessary\n";
    }
    else if (center_type.find("Static Center") != std::string::npos)
    {
        ss << "- Fight for control of central files and squares\n";
        ss << "- Place pieces on protected central squares\n";
        ss << "- Patient, strategic maneuvering game\n";
        ss << "- Improve position of all pieces gradually\n";
        ss << "- Consider pawn breaks on wings only when center is controlled\n";
    }
    else if (center_type == "Mobile Center")
    {
        ss << "- Advance your pawn center to gain space and limit opponent's pieces\n";
        ss << "- Support the pawn advance with your pieces\n";
        ss << "- Avoid blocking your own pawns\n";
        ss << "- For the defender: block the pawn advance and pressure the pawns\n";
    }
    else if (center_type.find("Dynamic Center") != std::string::npos)
    {
        if (center_type.find("Isolated Queen's Pawn") != std::string::npos)
        {
            ss << "- For the side with IQP: attack quickly, avoid exchanges, use the initiative\n";
            ss
              << "- For the side against IQP: block the pawn, force exchanges, target the weak pawn\n";
        }
        else if (center_type.find("Hanging Pawns") != std::string::npos)
        {
            ss << "- For the side with hanging pawns: maintain initiative, consider pawn breaks\n";
            ss
              << "- For the side against hanging pawns: pressure the pawns, force advances to create weaknesses\n";
        }
        else
        {
            ss
              << "- For the side with dynamic advantage: attack, avoid exchanges, convert initiative\n";
            ss
              << "- For the side with static advantage: defend, force exchanges, aim for endgame\n";
        }
    }

    ss << "\n";

    // =========================================================================
    // [SC:trace] BLOCK 2: ADVANCED PAWN STRUCTURE MODULES
    // =========================================================================
    ss << "================================================================\n";
    ss << "=== ADVANCED PAWN STRUCTURE MODULES ===\n\n";
    Trace::Helpers::append_score_block(ss, pos, "Pawn Structure Engine Evaluation", PAWN);

    auto weakness_count = [&](Color c) {
        return popcount(c == WHITE ? white_doubled : black_doubled)
             + popcount(c == WHITE ? white_isolated : black_isolated)
             + popcount(c == WHITE ? white_backward : black_backward)
             + (c == WHITE ? white_pairs : black_pairs);
    };
    const int wWeak = weakness_count(WHITE), bWeak = weakness_count(BLACK);
    const int wIsl = int(white_islands.size()), bIsl = int(black_islands.size());

    // Targets: weak pawns of a side that sit on a file not covered by its own pawns from behind,
    // i.e. pawns the opponent's heavy pieces can attack frontally
    auto frontal_targets = [&](Color c) {
        Bitboard weak    = (c == WHITE ? white_isolated | white_backward
                                       : black_isolated | black_backward);
        Bitboard targets = 0;
        while (weak)
        {
            Square s = pop_lsb(weak);
            if (!(pos.pieces(~c, PAWN) & file_bb(file_of(s))))
                targets |= s;
        }
        return targets;
    };
    const Bitboard wTargets = frontal_targets(WHITE), bTargets = frontal_targets(BLACK);

    ss << "Structural Balance Sheet:\n";
    ss << "  - Pawn islands      : White " << wIsl << ", Black " << bIsl << "\n";
    ss << "  - Weaknesses        : White " << wWeak << ", Black " << bWeak
       << " (doubled + isolated + backward + hanging pairs)\n";
    ss << "  - Frontal targets   : White pawns " << Trace::Helpers::bitboard_to_squares(wTargets)
       << "| Black pawns " << Trace::Helpers::bitboard_to_squares(bTargets) << "\n";
    ss << "  - Holes (weak squares in the own camp, ranks 1-4 for White / 5-8 for Black):\n"
       << "      White camp: "
       << Trace::Helpers::bitboard_to_squares(white_weak & (Rank1BB | Rank2BB | Rank3BB | Rank4BB))
       << "\n      Black camp: "
       << Trace::Helpers::bitboard_to_squares(black_weak & (Rank5BB | Rank6BB | Rank7BB | Rank8BB))
       << "\n\n";

    ss << "--- OTB STRATEGIC SYNTHESIS ---\n";
    const int netPawnMg = mg_value(scores[PAWN][WHITE] - scores[PAWN][BLACK]);
    if (center_type.find("Dynamic") != std::string::npos)
        ss << "  * Theory : Dynamic centers (IQP, hanging pawns) trade structural weaknesses for "
              "space and piece activity: their owner must act before the endgame arrives.\n";
    else if (center_type.find("Closed") != std::string::npos)
        ss << "  * Theory : With a closed center the play moves to the wings: attack on the side "
              "where your pawn chain points, with pawn breaks against the base of the enemy chain.\n";
    else if (wWeak + bWeak > 0)
        ss << "  * Theory : Static pawn weaknesses are permanent: they become decisive when the "
              "pieces that defend them are traded, so the side with the better structure welcomes "
              "simplifications.\n";
    else
        ss << "  * Theory : Healthy structures on both sides: the game will be decided by piece "
              "activity and by who first creates a weakness with a well prepared pawn break.\n";

    ss << "  * Action : ";
    if (wWeak > bWeak && wTargets)
        ss << "Black should blockade and pile up on the weak White pawns ("
           << Trace::Helpers::bitboard_to_squares(wTargets)
           << ") with rooks on the half-open files; White must keep pieces active to defend them "
              "dynamically and avoid passive trades.\n";
    else if (bWeak > wWeak && bTargets)
        ss << "White should blockade and pile up on the weak Black pawns ("
           << Trace::Helpers::bitboard_to_squares(bTargets)
           << ") with rooks on the half-open files; Black must keep pieces active to defend them "
              "dynamically and avoid passive trades.\n";
    else if (netPawnMg > 10)
        ss << "White has the healthier structure: trade pieces (not pawns) and steer towards an "
              "endgame where the structural edge counts.\n";
    else if (netPawnMg < -10)
        ss << "Black has the healthier structure: trade pieces (not pawns) and steer towards an "
              "endgame where the structural edge counts.\n";
    else
        ss << "The structures are equivalent: look for the pawn break that improves your pieces "
              "(see the candidate pawn breaks in the PLANS section).\n";
    ss << "\n";

    return ss.str();
}
}

}  // namespace Alexander