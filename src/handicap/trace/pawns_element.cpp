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

    // d. PEDONI ARRETRATI (BACKWARD PAWNS) - DEFINIZIONE COMPLETA (senza warning)
    auto find_backward_pawns = [&](Color color) -> Bitboard {
        Color     Them = ~color;
        Direction Up   = pawn_push(color);

        Bitboard ourPawns      = pos.pieces(color, PAWN);
        Bitboard theirPawns    = pos.pieces(Them, PAWN);
        Bitboard backwardPawns = 0;

        // Per ogni pedone
        Bitboard pawnsCopy = ourPawns;
        while (pawnsCopy)
        {
            Square s = pop_lsb(pawnsCopy);
            File   f = file_of(s);

            // CONDIZIONE 1: Mancanza di protezione da pedoni amici
            Bitboard neighbours           = ourPawns & adjacent_files_bb(s);
            bool     hasForwardNeighbours = neighbours & forward_ranks_bb(Them, s + Up);

            // Condizione semplificata: nessun pedone amico che possa proteggerlo avanzando
            // (i pedoni sulle colonne adiacenti sono avanzati oltre o assenti)
            bool lacksProtection = !hasForwardNeighbours;

            // CONDIZIONE 2: Vulnerabilità su colonna (semi-)aperta
            bool isSemiOpen = true;  // colonna semi-aperta (nessun pedone amico davanti)

            // Controlla la colonna del pedone
            Bitboard sameFilePawns = ourPawns & file_bb(f);

            // Per colonna semi-aperta: nessun pedone amico davanti
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

            // È su colonna (semi-)aperta
            bool onOpenOrSemiOpen = isSemiOpen;

            // CONDIZIONE AGGIUNTIVA: vulnerabilità al blocco/attacco
            bool blocked = (pos.piece_on(s + Up) == make_piece(Them, PAWN));

            // Calcola leverPush (attacco avversario alla casa davanti)
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

            // DEFINIZIONE COMPLETA: Pedone arretrato se:
            // 1. Mancano pedoni amici che possano proteggerlo (sono avanzati oltre o assenti)
            // 2. È su colonna semi-aperta o aperta
            // 3. È vulnerabile (bloccato o minacciato)
            bool isBackward = lacksProtection && onOpenOrSemiOpen && vulnerable;

            // Escludi i pedoni passati
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

    // e. PEDONI SOSPESI (HANGING PAWNS) - DEFINIZIONE CORRETTA E CENTRALE
    auto find_hanging_pawns = [&](Color color) -> Bitboard {
        Bitboard pawns   = pos.pieces(color, PAWN);
        Bitboard hanging = 0;
        Bitboard p       = pawns;

        while (p)
        {
            Square s = pop_lsb(p);
            File   f = file_of(s);
            Rank   r = rank_of(s);

            // 1. Devono essere pedoni CENTRALi o semi-centrali (colonne c, d, e).
            // Se f è FILE_C, la coppia sarà c+d. Se è FILE_D, sarà d+e.
            if (f >= FILE_C && f <= FILE_E)
            {
                // 2. Devono essere avanzati (non sulle traverse di partenza)
                bool is_advanced = (color == WHITE) ? (r >= RANK_3) : (r <= RANK_6);

                // 3. Controlliamo che ci sia il compagno esattamente a destra
                if (is_advanced && (pawns & square_bb(Square(s + 1))))
                {
                    // Abbiamo una Phalanx centrale. Ora controlliamo le colonne esterne.
                    // Dato che f è tra C ed E, i limiti esterni sono sempre validi (B e F/G)
                    File left_outer = File(f - 1);
                    File right_outer =
                      File(f + 2);  // f+1 è l'altro pedone, f+2 è la colonna esterna destra

                    bool has_left_outer_pawn  = pawns & file_bb(left_outer);
                    bool has_right_outer_pawn = pawns & file_bb(right_outer);

                    // Se non ci sono alleati sulle colonne esterne, sono veri "Hanging Pawns" centrali
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

    // Funzione per contare le coppie di hanging pawns
    auto count_hanging_pairs = [&](Color color) -> int {
        Bitboard hanging = find_hanging_pawns(color);
        int      pairs   = 0;

        // Controlla tutte le coppie di colonne adiacenti
        for (int i = FILE_A; i <= FILE_G; i++)
        {
            File f      = File(i);
            File next_f = File(i + 1);

            Bitboard pawns_on_f    = hanging & file_bb(f);
            Bitboard pawns_on_next = hanging & file_bb(next_f);

            if (pawns_on_f && pawns_on_next)
            {
                pairs++;
                i++;  // Salta la prossima colonna
            }
        }
        return pairs;
    };

    int white_pairs = count_hanging_pairs(WHITE);
    int black_pairs = count_hanging_pairs(BLACK);
    ss << "Hanging pawn pairs: White " << white_pairs << ", Black " << black_pairs << "\n";

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

        // Considera deboli le case nella metà campo avversario che sono attaccated
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

        // Conta i pedoni centrali per colore
        int white_center_count = (w_d4 ? 1 : 0) + (w_e4 ? 1 : 0) + (w_d5 ? 1 : 0) + (w_e5 ? 1 : 0);
        int black_center_count = (b_d4 ? 1 : 0) + (b_e4 ? 1 : 0) + (b_d5 ? 1 : 0) + (b_e5 ? 1 : 0);

        // 1. Centro APERTO: nessun pedone sulle 4 case centrali
        if (white_center_count == 0 && black_center_count == 0)
        {
            return "Open Center";
        }

        // 2. Centro CHIUSO: catene di pedoni bloccate al centro SENZA tensione
        Bitboard w_center_pawns = pos.pieces(WHITE, PAWN) & (FileDBB | FileEBB);
        Bitboard b_center_pawns = pos.pieces(BLACK, PAWN) & (FileDBB | FileEBB);

        Bitboard w_center_attacks = pawn_attacks_bb<WHITE>(w_center_pawns);
        Bitboard b_center_attacks = pawn_attacks_bb<BLACK>(b_center_pawns);

        // C'è tensione se i pedoni centrali possono catturarsi a vicenda
        bool central_tension =
          (w_center_attacks & b_center_pawns) || (b_center_attacks & w_center_pawns);

        bool closed_center = false;

        // Se c'è tensione, NON è un centro chiuso.
        if (!central_tension)
        {
            // Verifichiamo la presenza di almeno un blocco frontale
            if ((w_d4 && b_d5) || (w_e4 && b_e5) || (w_d5 && b_d6) || (w_e5 && b_e6))
            {
                // Un vero centro chiuso richiede pedoni su entrambe le colonne centrali per entrambi i colori
                if ((w_center_pawns & file_bb(FILE_D)) && (w_center_pawns & file_bb(FILE_E))
                    && (b_center_pawns & file_bb(FILE_D)) && (b_center_pawns & file_bb(FILE_E)))
                {
                    closed_center = true;
                }
            }
        }

        if (closed_center)
        {
            // Struttura Francese: Bianco: d4, e5 vs Nero: d5, e6
            if ((w_d4 && w_e5 && b_d5 && b_e6) || (b_d4 && b_e5 && w_d5 && w_e3))
            {
                return "Closed Center (French Chain)";
            }
            // Struttura Est-Indiana: Bianco: e4, d5 vs Nero: e5, d6
            if ((w_e4 && w_d5 && b_e5 && b_d6) || (b_e4 && b_d5 && w_e5 && w_d3))
            {
                return "Closed Center (King's Indian Chain)";
            }
            return "Closed Center";
        }

        // 3. Centro STATICO: pedoni centrali fissi su colonne opposte
        if ((w_d4 && b_d5) && !(w_e4 || b_e4 || w_e5 || b_e5))
        {
            bool no_immediate_tension = true;
            if ((pos.pieces(WHITE, PAWN) & (Rank3BB | Rank4BB))
                || (pos.pieces(BLACK, PAWN) & (Rank6BB | Rank5BB)))
            {
                no_immediate_tension = false;
            }
            if (no_immediate_tension)
                return "Static Center (d4-d5)";
        }

        if ((w_e4 && b_e5) && !(w_d4 || b_d4 || w_d5 || b_d5))
        {
            bool no_immediate_tension = true;
            if ((pos.pieces(WHITE, PAWN) & (Rank3BB | Rank4BB))
                || (pos.pieces(BLACK, PAWN) & (Rank6BB | Rank5BB)))
            {
                no_immediate_tension = false;
            }
            if (no_immediate_tension)
                return "Static Center (e4-e5)";
        }

        // 4. Centro MOBILE: un pedone centrale sostenuto da un altro
        if ((w_d4 && (w_e3 || w_c3)) || (w_e4 && (w_d3 || w_f3)))
        {
            return "Mobile Center";
        }
        if ((b_d5 && (b_e6 || b_c6)) || (b_e5 && (b_d6 || b_f6)))
        {
            return "Mobile Center";
        }

        // 5. Centro DINAMICO: presenza di IQP o Pedoni Sospesi

        Bitboard w_pawns = pos.pieces(WHITE, PAWN);
        Bitboard b_pawns = pos.pieces(BLACK, PAWN);

        // Verifica IQP (Pedone di Donna Isolato)
        // Definizione blindata con maschere assolute Bitboard:
        // 1. Il pedone amico deve essere in d4 (Bianco) o d5 (Nero).
        // 2. Deve essere l'UNICO pedone amico sulla colonna D.
        // 3. NON ci devono essere pedoni amici sulle colonne C ed E.
        // 4. NON ci devono essere pedoni avversari sulla colonna D (la colonna deve essere semi-aperta).

        bool w_has_iqp = w_d4 && (popcount(w_pawns & FileDBB) == 1)
                      && !(w_pawns & (FileCBB | FileEBB)) && !(b_pawns & FileDBB);

        bool b_has_iqp = b_d5 && (popcount(b_pawns & FileDBB) == 1)
                      && !(b_pawns & (FileCBB | FileEBB)) && !(w_pawns & FileDBB);

        if (w_has_iqp || b_has_iqp)
        {
            return "Dynamic Center (Isolated Queen's Pawn)";
        }

        // Verifica pedoni sospesi richiamando la funzione corretta definita sopra
        if (find_hanging_pawns(WHITE) || find_hanging_pawns(BLACK))
        {
            return "Dynamic Center (Hanging Pawns)";
        }

        // Verifica pedoni sospesi richiamando la funzione corretta definita sopra
        if (find_hanging_pawns(WHITE) || find_hanging_pawns(BLACK))
        {
            return "Dynamic Center (Hanging Pawns)";
        }

        // Se nessuno dei casi sopra, restituisce "Dynamic Center" generico
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