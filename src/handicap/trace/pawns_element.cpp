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

    // CENTER TYPE ANALYSIS (Basata sulla classificazione sistematica del PDF)
    ss << "Center Type Analysis:\n";
    ss << "=====================\n";

    // Definizione precisa dei tipi di centro basata sul PDF
    auto analyze_center_type = [&]() -> std::string {
        Square d4 = SQ_D4, e4 = SQ_E4, d5 = SQ_D5, e5 = SQ_E5;
        Piece  w_pawn = W_PAWN, b_pawn = B_PAWN;

        bool w_d4 = pos.piece_on(d4) == w_pawn;
        bool w_e4 = pos.piece_on(e4) == w_pawn;
        bool w_d5 = pos.piece_on(d5) == w_pawn;
        bool w_e5 = pos.piece_on(e5) == w_pawn;
        bool b_d4 = pos.piece_on(d4) == b_pawn;
        bool b_e4 = pos.piece_on(e4) == b_pawn;
        bool b_d5 = pos.piece_on(d5) == b_pawn;
        bool b_e5 = pos.piece_on(e5) == b_pawn;

        // Conta i pedoni centrali per colore
        int white_center_count = (w_d4 ? 1 : 0) + (w_e4 ? 1 : 0) + (w_d5 ? 1 : 0) + (w_e5 ? 1 : 0);
        int black_center_count = (b_d4 ? 1 : 0) + (b_e4 ? 1 : 0) + (b_d5 ? 1 : 0) + (b_e5 ? 1 : 0);

        // 1. Centro APERTO: nessun pedone sulle 4 case centrali
        if (white_center_count == 0 && black_center_count == 0)
        {
            return "Open Center";
        }

        // 2. Centro CHIUSO: verificare le catene di pedoni
        // Configurazione Francese: Bianco: d4, e5 vs Nero: d5, e6
        if ((w_d4 && w_e5 && b_d5 && pos.piece_on(SQ_E6) == b_pawn)
            || (b_d4 && b_e5 && w_d5 && pos.piece_on(SQ_E6) == w_pawn))
        {
            return "Closed Center (French Chain)";
        }
        // Configurazione e4-d5 vs e5-d6
        if ((w_e4 && w_d5 && b_e5 && pos.piece_on(SQ_D6) == b_pawn)
            || (b_e4 && b_d5 && w_e5 && pos.piece_on(SQ_D6) == w_pawn))
        {
            return "Closed Center (e4-d5 Chain)";
        }

        // 3. Centro STATICO: pedoni fissi su una colonna centrale
        // Caso 1: d4 vs d5 senza pedoni in e4 e e5
        if ((w_d4 && b_d5) && !w_e4 && !b_e4 && !w_e5 && !b_e5)
        {
            return "Static Center (d4-d5)";
        }
        // Caso 2: e4 vs e5 senza pedoni in d4 e d5
        if ((w_e4 && b_e5) && !w_d4 && !b_d4 && !w_d5 && !b_d5)
        {
            return "Static Center (e4-e5)";
        }

        // 4. Centro MOBILE: un giocatore ha due pedoni centrali adiacenti e possono avanzare
        if (w_d4 && w_e4 && !b_d5 && !b_e5)
        {
            return "Mobile Center";
        }
        if (b_d5 && b_e5 && !w_d4 && !w_e4)
        {
            return "Mobile Center";
        }

        // 5. Centro DINAMICO: presenza di IQP o Pedoni Sospesi
        // Funzione per verificare se un pedone è isolato
        auto is_isolated = [&](Color color, Square sq) -> bool {
            File     f       = file_of(sq);
            File     left_f  = File(f - 1);
            File     right_f = File(f + 1);
            Bitboard pawns   = pos.pieces(color, PAWN);
            if (left_f >= FILE_A && (pawns & file_bb(left_f)))
                return false;
            if (right_f <= FILE_H && (pawns & file_bb(right_f)))
                return false;
            return true;
        };

        // Verifica IQP per Bianco e Nero
        if (w_d4 && is_isolated(WHITE, d4))
        {
            return "Dynamic Center (Isolated Queen's Pawn)";
        }
        if (b_d5 && is_isolated(BLACK, d5))
        {
            return "Dynamic Center (Isolated Queen's Pawn)";
        }

        // Funzione per verificare i pedoni sospesi
        auto has_hanging_pawns = [&](Color color) -> bool {
            Bitboard pawns = pos.pieces(color, PAWN);
            Square   c4    = (color == WHITE) ? SQ_C4 : SQ_C5;
            Square   d4    = (color == WHITE) ? SQ_D4 : SQ_D5;
            if (pos.piece_on(c4) == make_piece(color, PAWN)
                && pos.piece_on(d4) == make_piece(color, PAWN))
            {
                File b_file = FILE_B, e_file = FILE_E;
                if (!(pawns & file_bb(b_file)) && !(pawns & file_bb(e_file)))
                {
                    return true;
                }
            }
            return false;
        };

        if (has_hanging_pawns(WHITE) || has_hanging_pawns(BLACK))
        {
            return "Dynamic Center (Hanging Pawns)";
        }

        // Se nessuno dei above, restituisce "Dynamic Center" generico
        return "Dynamic Center";
    };

    std::string center_type = analyze_center_type();
    ss << "Center Type: " << center_type << "\n";

    // Spiegazione del tipo di centro
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

    // Dettaglio dei pedoni centrali
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

    return ss.str();
}
}

}  // namespace Alexander