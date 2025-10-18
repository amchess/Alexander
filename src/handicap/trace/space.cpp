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

    // DETTAGLIO SPAZIO PER AREE
    ss << "Space Detail by Area:\n";

    // Definiamo le maschere per le aree
    const Bitboard QueenSideFiles = FileABB | FileBBB | FileCBB;  // a-c
    const Bitboard CenterFiles    = FileDBB | FileEBB;            // d-e
    const Bitboard KingSideFiles  = FileFBB | FileGBB | FileHBB;  // f-h

    // Funzione per calcolare gli attacchi dei pedoni per un colore
    auto pawn_attacks_bb = [](Color c, Bitboard pawns) {
        return c == WHITE ? (shift<NORTH_WEST>(pawns) | shift<NORTH_EAST>(pawns))
                          : (shift<SOUTH_WEST>(pawns) | shift<SOUTH_EAST>(pawns));
    };

    // Funzione per calcolare lo spazio per un colore in una data maschera di file
    auto calculate_space_for_area = [&](Color us, Bitboard area_mask) {
        Color them = ~us;

        // Determina le ranghe in base al colore
        Bitboard rank_mask =
          (us == WHITE) ? (Rank2BB | Rank3BB | Rank4BB) : (Rank5BB | Rank6BB | Rank7BB);
        Bitboard space_mask = area_mask & rank_mask;

        // Calcola gli attacchi dei pedoni avversari
        Bitboard their_pawns        = pos.pieces(them, PAWN);
        Bitboard their_pawn_attacks = pawn_attacks_bb(them, their_pawns);

        // Case sicure: nella maschera, non occupate da nostri pedoni e non attaccate da pedoni avversari
        Bitboard safe = space_mask & ~pos.pieces(us, PAWN) & ~their_pawn_attacks;

        // Case dietro i pedoni: fino a tre case dietro i pedoni amici
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

        // Bonus: case sicure + case sicure dietro i pedoni non attaccate da pedoni avversari
        int bonus = popcount(safe) + popcount(behind & safe & ~their_pawn_attacks);

        return bonus;
    };

    // Calcoliamo per ogni area e per ogni colore
    int white_queen_side = calculate_space_for_area(WHITE, QueenSideFiles);
    int white_center     = calculate_space_for_area(WHITE, CenterFiles);
    int white_king_side  = calculate_space_for_area(WHITE, KingSideFiles);
    int black_queen_side = calculate_space_for_area(BLACK, QueenSideFiles);
    int black_center     = calculate_space_for_area(BLACK, CenterFiles);
    int black_king_side  = calculate_space_for_area(BLACK, KingSideFiles);

    // Calcola le differenze
    int queen_side_diff = white_queen_side - black_queen_side;
    int center_diff     = white_center - black_center;
    int king_side_diff  = white_king_side - black_king_side;

    // Mostra in formato tabellare
    ss << "                        Queen side   Center   King side\n";
    ss << "White                      " << std::setw(3) << white_queen_side << "         "
       << std::setw(3) << white_center << "        " << std::setw(3) << white_king_side << "\n";
    ss << "Black                      " << std::setw(3) << black_queen_side << "         "
       << std::setw(3) << black_center << "        " << std::setw(3) << black_king_side << "\n";
    ss << "Difference White-Black     " << std::setw(3) << queen_side_diff << "         "
       << std::setw(3) << center_diff << "        " << std::setw(3) << king_side_diff << "\n";

    // Calcoliamo anche il totale per ogni colore
    int white_total_space = white_queen_side + white_center + white_king_side;
    int black_total_space = black_queen_side + black_center + black_king_side;
    ss << "Total Space: White " << white_total_space << " - Black " << black_total_space << "\n";

    // Raccomandazioni strategiche basate sul controllo dello spazio
    ss << "Space Recommendations:\n";

    // Determina il colore con vantaggio e l'area di forza
    int total_diff = white_total_space - black_total_space;
    if (total_diff > 0)
    {
        // Bianco ha vantaggio
        ss << "White has a space advantage and must increase it on the ";
        // Trova l'area di massimo vantaggio per il Bianco
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
        // Nero ha vantaggio
        ss << "Black has a space advantage and must increase it on the ";
        // Trova l'area di massimo vantaggio per il Nero (valori negativi più piccoli)
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

    // EXPANSION FACTOR (Shashin Theory) - mantenuto invariato
    ss << "Expansion Factor (Shashin Theory):\n";
    ss << "===================================\n";

    // Funzione per calcolare il centro di gravità per un colore
    auto calculate_center_of_gravity = [&](Color color) -> double {
        Bitboard pieces      = pos.pieces(color);
        double   sum         = 0.0;
        int      piece_count = 0;

        while (pieces)
        {
            Square s    = pop_lsb(pieces);
            Rank   rank = rank_of(s);

            // Per il bianco: rank1 = 1, rank2 = 2, ... rank8 = 8
            // Per il nero: rank8 = 1, rank7 = 2, ... rank1 = 8
            double rank_value = (color == WHITE) ? (rank + 1) : (8 - rank);

            sum += rank_value;
            piece_count++;
        }

        return piece_count > 0 ? sum / piece_count : 0.0;
    };

    // Calcola i centri di gravità
    double white_gravity   = calculate_center_of_gravity(WHITE);
    double black_gravity   = calculate_center_of_gravity(BLACK);
    double delta_expansion = white_gravity - black_gravity;

    // Mostra i risultati
    ss << "White Center of Gravity: " << std::fixed << std::setprecision(2) << white_gravity
       << "\n";
    ss << "Black Center of Gravity: " << std::fixed << std::setprecision(2) << black_gravity
       << "\n";
    ss << "Delta Expansion (White-Black): " << std::fixed << std::setprecision(2) << delta_expansion
       << "\n";

    // Solo per posizioni di tipo Capablanca (win probability ~50%)
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

    return ss.str();
}

}

}  // namespace Alexander