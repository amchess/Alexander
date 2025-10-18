#include "../../evaluate.h"
#include "trace.h"
#include "../evaluate_handicap.h"
#include "../../pawns.h"
#include "../../uci.h"
#include "../../movegen.h"
#include "../../bitboard.h"
#include <sstream>
#include <algorithm>
#include <vector>
#include "makogonov.h"

namespace Alexander {

using namespace Trace;

namespace Eval {

std::string generate_makogonov_ranking(const Position& pos, Pawns::Entry* pawnEntry) {
    std::stringstream ss;
    // SEZIONE MAKOGONOV - UNIT RANKING BY STATIC ACTIVITY
    ss << "FINAL SUMMARY\n";
    ss << "Unit Ranking by static activity (Makogonov Principle) - Worst to Best:\n";

    struct UnitActivity {
        Color       color;
        PieceType   pieceType;
        Square      square;
        int         activityScore;
        std::string pieceName;
    };

    std::vector<UnitActivity> whiteUnits;
    std::vector<UnitActivity> blackUnits;

    // Calcola l'attività statica per ogni pezzo
    for (Square s = SQ_A1; s <= SQ_H8; ++s)
    {
        Piece p = pos.piece_on(s);
        if (p != NO_PIECE)
        {
            Color     c  = color_of(p);
            PieceType pt = type_of(p);

            // ESCLUDI I RE DAL RANKING MAKOGONOV
            if (pt == KING)
            {
                continue;
            }

            // CALCOLA L'ATTIVITÀ STATICA PER OGNI TIPO DI PEZZO
            int activity = 0;

            if (pt == PAWN)
            {
                // [Mantieni il codice esistente per i pedoni...]
                Rank r = relative_rank(c, s);
                activity += r * 8;

                if (pawnEntry->passed_pawns(c) & s)
                {
                    activity += 25;
                }

                if (!(pos.pieces(c, PAWN) & adjacent_files_bb(s)))
                {
                    activity -= 15;
                }

                if (pos.pieces(c, PAWN) & adjacent_files_bb(s))
                {
                    activity += 5;
                }

                Square ahead = s + pawn_push(c);
                if (pos.empty(ahead))
                {
                    activity += 10;
                    if ((r == RANK_2 && c == WHITE) || (r == RANK_7 && c == BLACK))
                    {
                        Square two_ahead = ahead + pawn_push(c);
                        if (pos.empty(two_ahead))
                        {
                            activity += 5;
                        }
                    }
                }
            }
            else
            {
                // [Mantieni il codice esistente per gli altri pezzi...]
                Bitboard attacks = pt == BISHOP ? attacks_bb<BISHOP>(s, pos.pieces())
                                 : pt == ROOK   ? attacks_bb<ROOK>(s, pos.pieces())
                                 : pt == QUEEN  ? attacks_bb<QUEEN>(s, pos.pieces())
                                 : pt == KNIGHT ? attacks_bb<KNIGHT>(s)
                                                : 0;

                int mobility_count = popcount(attacks);
                activity += mobility_count * 4;

                activity += popcount(attacks & Center) * 12;

                Square opponent_king = pos.square<KING>(~c);
                activity += popcount(attacks & attacks_bb<KING>(opponent_king)) * 8;

                const Bitboard OutpostRanks =
                  (c == WHITE ? Rank4BB | Rank5BB | Rank6BB : Rank5BB | Rank4BB | Rank3BB);
                if ((pt == KNIGHT || pt == BISHOP) && (OutpostRanks & s))
                {
                    if (pos.pieces(c, PAWN) & attacks_bb<PAWN>(s, c))
                    {
                        activity += 20;
                    }
                    else
                    {
                        activity += 10;
                    }
                }

                if (mobility_count < 4)
                {
                    activity -= 10;
                }
            }

            // Aggiungi al ranking - ESCLUDI I RE
            if (c == WHITE)
                whiteUnits.push_back({c, pt, s, activity, piece_type_name(pt)});
            else
                blackUnits.push_back({c, pt, s, activity, piece_type_name(pt)});
        }
    }

    // Ordina le unità per attività (dal peggiore al migliore)
    std::sort(whiteUnits.begin(), whiteUnits.end(),
              [](const UnitActivity& a, const UnitActivity& b) {
                  return a.activityScore < b.activityScore;
              });
    std::sort(blackUnits.begin(), blackUnits.end(),
              [](const UnitActivity& a, const UnitActivity& b) {
                  return a.activityScore < b.activityScore;
              });

    // Funzione per stampare le unità in orizzontale
    auto print_units_horizontal = [&](const std::vector<UnitActivity>& units,
                                      const std::string&               colorName) {
        if (units.empty())
        {
            ss << "No " << colorName << " units found for ranking.\n";
            return;
        }

        ss << colorName << ": ";
        for (size_t i = 0; i < units.size(); ++i)
        {
            const auto& unit = units[i];
            // USA SIMBOLI STANDARD PER I PEZZI
            std::string pieceSymbol = (unit.pieceName == "Knight") ? "N"
                                    : (unit.pieceName == "Bishop") ? "B"
                                    : (unit.pieceName == "Rook")   ? "R"
                                    : (unit.pieceName == "Queen")  ? "Q"
                                    : (unit.pieceName == "Pawn")   ? "P"
                                                                   : "?";
            ss << pieceSymbol << Trace::Helpers::square_str(unit.square) << "("
               << unit.activityScore << ")";
            if (i < units.size() - 1)
                ss << ", ";
        }
        ss << "\n";

        if (!units.empty())
        {
            const auto& worstUnit = units[0];
            ss << "Makogonov " << colorName << ": Improve " << worstUnit.pieceName << " on "
               << Trace::Helpers::square_str(worstUnit.square)
               << " (activity: " << worstUnit.activityScore << ")";
        }
    };

    print_units_horizontal(whiteUnits, "White");
    ss << "\n";
    print_units_horizontal(blackUnits, "Black");

    ss << "\n";

    return ss.str();
}

}

}  // namespace Alexander