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

    // a. ISOLE PEDONALI
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

    // b. PEDONI IMPEDONATI (DOUBLED PAWNS)
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

    // c. PEDONI ISOLATI
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

    // d. PEDONI ARRETRATI (BACKWARD PAWNS)
    auto find_backward_pawns = [&](Color color) -> Bitboard {
        Bitboard  pawns    = pos.pieces(color, PAWN);
        Bitboard  backward = 0;
        Color     opponent = ~color;
        Direction push     = pawn_push(color);

        for (File f = FILE_A; f <= FILE_H; ++f)
        {
            Bitboard file_pawns = pawns & file_bb(f);
            if (file_pawns == 0)
                continue;

            // Trova il pedone più arretrato su questa fila
            Square back_pawn = (color == WHITE) ? msb(file_pawns) : lsb(file_pawns);

            // Controlla se ci sono pedoni amici sulle file adiacenti che possono difendere
            bool has_support = false;
            if (f > FILE_A)
            {
                Bitboard left_file = pawns & file_bb(File(f - 1));
                if (left_file)
                {
                    Square support_pawn = (color == WHITE) ? msb(left_file) : lsb(left_file);
                    if ((color == WHITE && rank_of(support_pawn) >= rank_of(back_pawn))
                        || (color == BLACK && rank_of(support_pawn) <= rank_of(back_pawn)))
                    {
                        has_support = true;
                    }
                }
            }
            if (f < FILE_H && !has_support)
            {
                Bitboard right_file = pawns & file_bb(File(f + 1));
                if (right_file)
                {
                    Square support_pawn = (color == WHITE) ? msb(right_file) : lsb(right_file);
                    if ((color == WHITE && rank_of(support_pawn) >= rank_of(back_pawn))
                        || (color == BLACK && rank_of(support_pawn) <= rank_of(back_pawn)))
                    {
                        has_support = true;
                    }
                }
            }

            // Controlla se il pedone è bloccato da un pedone avversario
            Square square_ahead = back_pawn + push;
            bool   is_blocked   = pos.piece_on(square_ahead) == make_piece(opponent, PAWN);

            if (!has_support && is_blocked)
            {
                backward |= back_pawn;
            }
        }
        return backward;
    };

    ss << "\nBackward Pawns:\n";
    Bitboard white_backward = find_backward_pawns(WHITE);
    Bitboard black_backward = find_backward_pawns(BLACK);
    ss << "White: " << bitboard_to_squares(white_backward) << "\n";
    ss << "Black: " << bitboard_to_squares(black_backward) << "\n";

    // e. PEDONI SOSPESI (HANGING PAWNS)
    auto find_hanging_pawns = [&](Color color) -> Bitboard {
        Bitboard pawns   = pos.pieces(color, PAWN);
        Bitboard hanging = 0;

        // Un pedone è sospeso se è su una colonna semi-aperta e non è supportato da pedoni amici
        for (File f = FILE_A; f <= FILE_H; ++f)
        {
            Bitboard file_pawns = pawns & file_bb(f);
            if (file_pawns == 0)
                continue;

            // Controlla se la colonna è semi-aperta (nessun pedone amico davanti)
            bool     semi_open  = true;
            Bitboard ahead_mask = (color == WHITE) ? forward_ranks_bb(WHITE, lsb(file_pawns))
                                                   : forward_ranks_bb(BLACK, lsb(file_pawns));
            if (pawns & ahead_mask)
                semi_open = false;

            // Controlla supporto dai pedoni adiacenti
            bool has_adjacent_support = false;
            if (f > FILE_A)
            {
                Bitboard left_adjacent = pawns & file_bb(File(f - 1));
                if (left_adjacent)
                {
                    // Controlla se il pedone adiacente è sulla stessa traversa o una traversa che può supportare
                    if (color == WHITE)
                    {
                        if (rank_of(lsb(left_adjacent)) >= rank_of(lsb(file_pawns)))
                            has_adjacent_support = true;
                    }
                    else
                    {
                        if (rank_of(lsb(left_adjacent)) <= rank_of(lsb(file_pawns)))
                            has_adjacent_support = true;
                    }
                }
            }
            if (f < FILE_H && !has_adjacent_support)
            {
                Bitboard right_adjacent = pawns & file_bb(File(f + 1));
                if (right_adjacent)
                {
                    if (color == WHITE)
                    {
                        if (rank_of(lsb(right_adjacent)) >= rank_of(lsb(file_pawns)))
                            has_adjacent_support = true;
                    }
                    else
                    {
                        if (rank_of(lsb(right_adjacent)) <= rank_of(lsb(file_pawns)))
                            has_adjacent_support = true;
                    }
                }
            }

            if (semi_open && !has_adjacent_support)
            {
                hanging |= file_pawns;
            }
        }
        return hanging;
    };

    ss << "\nHanging Pawns:\n";
    Bitboard white_hanging = find_hanging_pawns(WHITE);
    Bitboard black_hanging = find_hanging_pawns(BLACK);
    ss << "White: " << bitboard_to_squares(white_hanging) << "\n";
    ss << "Black: " << bitboard_to_squares(black_hanging) << "\n";

    // f. CASE DEBOLI (WEAK SQUARES)
    auto find_weak_squares = [&](Color color) -> Bitboard {
        Color    opponent    = ~color;
        Bitboard our_pawns   = pos.pieces(color, PAWN);
        Bitboard their_pawns = pos.pieces(opponent, PAWN);

        // Calcola gli attacchi dei pedoni
        Bitboard their_pawn_attacks = (opponent == WHITE) ? pawn_attacks_bb<WHITE>(their_pawns)
                                                          : pawn_attacks_bb<BLACK>(their_pawns);

        Bitboard our_pawn_attacks =
          (color == WHITE) ? pawn_attacks_bb<WHITE>(our_pawns) : pawn_attacks_bb<BLACK>(our_pawns);

        // Le case deboli sono quelle che non sono difese da pedoni amici
        Bitboard undefended_by_pawns = ~our_pawn_attacks;
        Bitboard potential_weak      = undefended_by_pawns & ~our_pawns;

        // Considera deboli le case nella metà campo avversario che sono attaccate
        Bitboard weak_candidates = potential_weak & their_pawn_attacks;

        // Aggiungi le case nelle prime 3 file del colore che sono indifese
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

    // RIASSUNTO STRUTTURALE
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
    // CENTER TYPE ANALYSIS (Intermediate and above)
    ss << "Center Type Analysis:\n";

    // Definizioni dei tipi di centro
    auto analyze_center_type = [&]() -> std::string {
        Bitboard center_squares =
          (FileCBB | FileDBB | FileEBB | FileFBB) & (Rank3BB | Rank4BB | Rank5BB | Rank6BB);
        Bitboard white_pawns = pos.pieces(WHITE, PAWN);
        Bitboard black_pawns = pos.pieces(BLACK, PAWN);

        // Conta i pedoni centrali per ogni colore
        Bitboard white_central       = white_pawns & center_squares;
        Bitboard black_central       = black_pawns & center_squares;
        int      white_central_count = popcount(white_central);
        int      black_central_count = popcount(black_central);

        // 1. Centro APERTO: nessun pedone al centro
        if (white_central_count == 0 && black_central_count == 0)
        {
            return "Open Center";
        }

        // 2. Centro CHIUSO: entrambi i giocatori hanno catene di pedoni al centro
        // Controlla se ci sono pedoni bloccati che si fronteggiano
        bool has_pawn_chains = false;
        for (File f = FILE_C; f <= FILE_F; ++f)
        {
            for (Rank r = RANK_3; r <= RANK_6; ++r)
            {
                Square sq = make_square(f, r);
                Piece  p  = pos.piece_on(sq);
                if (p == W_PAWN)
                {
                    Square ahead = sq + NORTH;
                    if (pos.piece_on(ahead) == B_PAWN)
                    {
                        has_pawn_chains = true;
                        break;
                    }
                }
                else if (p == B_PAWN)
                {
                    Square ahead = sq + SOUTH;
                    if (pos.piece_on(ahead) == W_PAWN)
                    {
                        has_pawn_chains = true;
                        break;
                    }
                }
            }
            if (has_pawn_chains)
                break;
        }

        // Se entrambi hanno pedoni centrali e ci sono catene, è chiuso
        if (white_central_count >= 2 && black_central_count >= 2 && has_pawn_chains)
        {
            return "Closed Center";
        }

        // 4. Centro di PEDONI: un giocatore ha pedoni al centro e l'altro no
        if ((white_central_count >= 2 && black_central_count == 0)
            || (black_central_count >= 2 && white_central_count == 0))
        {
            return "Pawn Center";
        }

        // 3. Centro STATICO: 1-2 pedoni per lato, situazione stabile
        if (white_central_count >= 1 && white_central_count <= 2 && black_central_count >= 1
            && black_central_count <= 2)
        {
            return "Static Center";
        }

        // 5. Centro DINAMICO: situazione non ancora definita
        return "Dynamic Center";
    };

    std::string center_type = analyze_center_type();
    ss << "Center Type: " << center_type << "\n";

    // Spiegazione del tipo di centro
    ss << "\nCenter Characteristics:\n";
    if (center_type == "Open Center")
    {
        ss << "- No pawns in the center\n";
        ss << "- Maximum piece mobility\n";
        ss << "- Tactical play dominates\n";
        ss << "- Bishops and queens are strong\n";
    }
    else if (center_type == "Closed Center")
    {
        ss << "- Both players have pawn chains in center\n";
        ss << "- Limited piece mobility through center\n";
        ss << "- Play typically develops on the wings\n";
        ss << "- Knights often better than bishops\n";
    }
    else if (center_type == "Static Center")
    {
        ss << "- Both players have 1-2 central pawns\n";
        ss << "- Stable pawn structure\n";
        ss << "- Strategic maneuvering game\n";
        ss << "- Piece placement is crucial\n";
    }
    else if (center_type == "Pawn Center")
    {
        ss << "- One player dominates center with pawns\n";
        ss << "- Space advantage for the controlling side\n";
        ss << "- Potential for pawn expansion\n";
        ss << "- Controlling side should maintain, other should challenge\n";
    }
    else
    {  // Dynamic Center
        ss << "- Center structure is not yet defined\n";
        ss << "- Multiple pawn breaks possible\n";
        ss << "- Tactical opportunities common\n";
        ss << "- Both sides have active possibilities\n";
    }

    // Dettaglio dei pedoni centrali
    ss << "\nCentral Pawns (c3-c6, d3-d6, e3-e6, f3-f6):\n";
    Bitboard central_area =
      (FileCBB | FileDBB | FileEBB | FileFBB) & (Rank3BB | Rank4BB | Rank5BB | Rank6BB);
    Bitboard central_white = pos.pieces(WHITE, PAWN) & central_area;
    Bitboard central_black = pos.pieces(BLACK, PAWN) & central_area;
    ss << "White: " << bitboard_to_squares(central_white) << "\n";
    ss << "Black: " << bitboard_to_squares(central_black) << "\n";

    // RACCOMANDAZIONI STRATEGICHE
    ss << "\nStrategic Recommendations:\n";
    ss << "==========================\n";

    if (center_type == "Open Center")
    {
        ss << "- Activate all pieces quickly\n";
        ss << "- Bishops and rooks are particularly strong\n";
        ss << "- Control open files with rooks\n";
        ss << "- Look for tactical opportunities\n";
    }
    else if (center_type == "Closed Center")
    {
        ss << "- Knights are usually superior to bishops\n";
        ss << "- Develop play on the queenside or kingside\n";
        ss << "- Prepare pawn breaks (f4/f5 or c4/c5)\n";
        ss << "- Avoid premature piece exchanges\n";
    }
    else if (center_type == "Static Center")
    {
        ss << "- Focus on piece maneuverability\n";
        ss << "- Improve position of all pieces\n";
        ss << "- Create weaknesses in opponent's camp\n";
        ss << "- Patient, strategic play required\n";
    }
    else if (center_type == "Pawn Center")
    {
        if (popcount(central_white) > popcount(central_black))
        {
            ss << "- White: maintain and protect pawn center\n";
            ss << "- White: look for opportunities to advance pawns\n";
            ss << "- Black: challenge with pieces and pawn breaks\n";
            ss << "- Black: try to undermine the pawn center\n";
        }
        else
        {
            ss << "- Black: maintain and protect pawn center\n";
            ss << "- Black: look for opportunities to advance pawns\n";
            ss << "- White: challenge with pieces and pawn breaks\n";
            ss << "- White: try to undermine the pawn center\n";
        }
    }
    else
    {  // Dynamic Center
        ss << "- Be prepared for tactical skirmishes\n";
        ss << "- Calculate pawn breaks carefully\n";
        ss << "- Maintain piece activity and coordination\n";
        ss << "- Watch for sudden changes in pawn structure\n";
    }
    ss << "\n";

    return ss.str();
}
}

}  // namespace Alexander