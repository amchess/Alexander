#include "../../evaluate.h"
#include "trace.h"
#include "imbalances.h"
#include "../evaluate_handicap.h"
#include "../../uci.h"
#include "../../movegen.h"
#include "../../bitboard.h"
#include <sstream>
#include <vector>

namespace Alexander {
using namespace Trace;
namespace Eval {
std::string analyze_imbalances(const Position& pos, int phase) {
    std::stringstream ss;
    Trace::Helpers::append_term_row(ss, pos, phase, "Imbalances", IMBALANCE);
    // SQUILIBRI DI MATERIALE (IMBALANCES)
    ss << "=== MATERIAL IMBALANCES SUBELEMENTS ===\n";
    // Dettagli squilibri material - sottoelemento
    // Valori approssimativi dei pezzi (per il calcolo degli imbalances)
    const int pawnValue   = 1;
    const int knightValue = 3;
    const int bishopValue = 3;
    const int rookValue   = 5;
    const int queenValue  = 9;

    // Calcolo del materiale per ogni colore
    int whiteMaterial = pawnValue * popcount(pos.pieces(WHITE, PAWN))
                      + knightValue * popcount(pos.pieces(WHITE, KNIGHT))
                      + bishopValue * popcount(pos.pieces(WHITE, BISHOP))
                      + rookValue * popcount(pos.pieces(WHITE, ROOK))
                      + queenValue * popcount(pos.pieces(WHITE, QUEEN));

    int blackMaterial = pawnValue * popcount(pos.pieces(BLACK, PAWN))
                      + knightValue * popcount(pos.pieces(BLACK, KNIGHT))
                      + bishopValue * popcount(pos.pieces(BLACK, BISHOP))
                      + rookValue * popcount(pos.pieces(BLACK, ROOK))
                      + queenValue * popcount(pos.pieces(BLACK, QUEEN));

    // Differenze di materiale
    int dPawns   = popcount(pos.pieces(WHITE, PAWN)) - popcount(pos.pieces(BLACK, PAWN));
    int dKnights = popcount(pos.pieces(WHITE, KNIGHT)) - popcount(pos.pieces(BLACK, KNIGHT));
    int dBishops = popcount(pos.pieces(WHITE, BISHOP)) - popcount(pos.pieces(BLACK, BISHOP));
    int dRooks   = popcount(pos.pieces(WHITE, ROOK)) - popcount(pos.pieces(BLACK, ROOK));
    int dQueens  = popcount(pos.pieces(WHITE, QUEEN)) - popcount(pos.pieces(BLACK, QUEEN));

    std::vector<std::string> imbalances;

    // Controllo degli squilibri solo se il materiale totale è quasi pari (tolleranza di 2 punti)
    if (std::abs(whiteMaterial - blackMaterial) <= 2)
    {
        // 1. Qualità: Torre vs Cavallo/Alfiere + Pedone (scenario classico)
        if (dRooks == 1 && dKnights == -1 && dPawns == -1)
            imbalances.push_back("White has the advantage (Rook for Knight+Pawn)");
        else if (dRooks == -1 && dKnights == 1 && dPawns == 1)
            imbalances.push_back("Black has the advantage (Rook for Knight+Pawn)");

        else if (dRooks == 1 && dBishops == -1 && dPawns == -1)
            imbalances.push_back("White has the advantage (Rook for Bishop+Pawn)");
        else if (dRooks == -1 && dBishops == 1 && dPawns == 1)
            imbalances.push_back("Black has the advantage (Rook for Bishop+Pawn)");

        // 2. Qualità in presenza di torri multiple
        else if (dRooks == 1 && dKnights == -1 && dPawns == -1
                 && popcount(pos.pieces(WHITE, ROOK)) >= 2)
            imbalances.push_back("White has the advantage (two Rooks against Rook+Knight+Pawn)");
        else if (dRooks == -1 && dKnights == 1 && dPawns == 1
                 && popcount(pos.pieces(BLACK, ROOK)) >= 2)
            imbalances.push_back("Black has the advantage (two Rooks against Rook+Knight+Pawn)");

        else if (dRooks == 1 && dBishops == -1 && dPawns == -1
                 && popcount(pos.pieces(WHITE, ROOK)) >= 2)
            imbalances.push_back("White has the advantage (two Rooks against Rook+Bishop+Pawn)");
        else if (dRooks == -1 && dBishops == 1 && dPawns == 1
                 && popcount(pos.pieces(BLACK, ROOK)) >= 2)
            imbalances.push_back("Black has the advantage (two Rooks against Rook+Bishop+Pawn)");

        // 3. Due pezzi leggeri vs Torre e Pedone
        if (dRooks == 1 && dPawns == 1 && (dKnights + dBishops == -2))
            imbalances.push_back("White exchanged two minor pieces for a rook and a pawn.");
        else if (dRooks == -1 && dPawns == -1 && (dKnights + dBishops == 2))
            imbalances.push_back("Black exchanged two minor pieces for a rook and a pawn.");

        // 4. Donna vs Tre pezzi leggeri
        if (dQueens == 1 && (dKnights + dBishops) == -3)
            imbalances.push_back("White has the Queen for three minor pieces");
        else if (dQueens == -1 && (dKnights + dBishops) == 3)
            imbalances.push_back("Black has the Queen for three minor pieces");

        // 5. Donna vs Torre + Cavallo/Alfiere + Pedone
        if (dQueens == 1 && dRooks == -1 && dKnights == -1 && dPawns == -1)
            imbalances.push_back("White has the Queen for Rook, Knight and Pawn");
        else if (dQueens == -1 && dRooks == 1 && dKnights == 1 && dPawns == 1)
            imbalances.push_back("Black has the Queen for Rook, Knight and Pawn");

        if (dQueens == 1 && dRooks == -1 && dBishops == -1 && dPawns == -1)
            imbalances.push_back("White has the Queen for Rook, Bishop, and Pawn");
        else if (dQueens == -1 && dRooks == 1 && dBishops == 1 && dPawns == 1)
            imbalances.push_back("Black has the Queen for Rook, Bishop, and Pawn");

        // 6. Donna vs Due Torri
        if (dQueens == 1 && dRooks == -2)
            imbalances.push_back("White has the Quen for Two Rooks");
        else if (dQueens == -1 && dRooks == 2)
            imbalances.push_back("Black has the Queen for Two Rooks");

        // 7. Torre e due Pedoni vs Due pezzi leggeri
        if (dRooks == 1 && dPawns == 2 && (dKnights + dBishops == -2))
            imbalances.push_back("White has a rook and two pawns for two minor pieces.");
        else if (dRooks == -1 && dPawns == -2 && (dKnights + dBishops == 2))
            imbalances.push_back("Black has a rook and two pawns for two minor pieces.");
    }

    // Mostra gli squilibri rilevati
    if (!imbalances.empty())
    {
        for (const auto& imbalance : imbalances)
        {
            ss << imbalance << "\n";
        }
    }
    else
    {
        ss << "No relevant material imbalance\n";
    }

    // Dettaglio del materiale per riferimento
    ss << "Material detail (by reference):\n";
    ss << "White: " << whiteMaterial << " points (P:" << popcount(pos.pieces(WHITE, PAWN))
       << " N:" << popcount(pos.pieces(WHITE, KNIGHT))
       << " B:" << popcount(pos.pieces(WHITE, BISHOP)) << " R:" << popcount(pos.pieces(WHITE, ROOK))
       << " Q:" << popcount(pos.pieces(WHITE, QUEEN)) << ")\n";
    ss << "Black: " << blackMaterial << " points (P:" << popcount(pos.pieces(BLACK, PAWN))
       << " N:" << popcount(pos.pieces(BLACK, KNIGHT))
       << " B:" << popcount(pos.pieces(BLACK, BISHOP)) << " R:" << popcount(pos.pieces(BLACK, ROOK))
       << " Q:" << popcount(pos.pieces(BLACK, QUEEN)) << ")\n";
    ss << "\n";
    return ss.str();
}
}

}  // namespace Alexander