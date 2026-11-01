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

// ATTENZIONE: Firma aggiornata con gamePhase per supportare l'inclusione del Re nel Finale
std::string generate_makogonov_ranking(const Position& pos, Pawns::Entry* pawnEntry, const std::string& gamePhase) {
    std::stringstream ss;
    
    // =========================================================================
    // BLOCCO 1: OUTPUT ORIGINALE INTATTO AL 100%
    // =========================================================================
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

    // Calcola l'attività statica per ogni pezzo (VECCHIO METODO)
    for (Square s = SQ_A1; s <= SQ_H8; ++s)
    {
        Piece p = pos.piece_on(s);
        if (p != NO_PIECE)
        {
            Color     c  = color_of(p);
            PieceType pt = type_of(p);

            // ESCLUDI I RE DAL RANKING MAKOGONOV ORIGINALE
            if (pt == KING) continue;

            int activity = 0;

            if (pt == PAWN)
            {
                Rank r = relative_rank(c, s);
                activity += r * 8;
                if (pawnEntry->passed_pawns(c) & s) activity += 25;
                if (!(pos.pieces(c, PAWN) & adjacent_files_bb(s))) activity -= 15;
                if (pos.pieces(c, PAWN) & adjacent_files_bb(s)) activity += 5;

                Square ahead = s + pawn_push(c);
                if (pos.empty(ahead))
                {
                    activity += 10;
                    if ((r == RANK_2 && c == WHITE) || (r == RANK_7 && c == BLACK))
                    {
                        Square two_ahead = ahead + pawn_push(c);
                        if (pos.empty(two_ahead)) activity += 5;
                    }
                }
            }
            else
            {
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

                const Bitboard OutpostRanks = (c == WHITE ? Rank4BB | Rank5BB | Rank6BB : Rank5BB | Rank4BB | Rank3BB);
                if ((pt == KNIGHT || pt == BISHOP) && (OutpostRanks & s))
                {
                    if (pos.pieces(c, PAWN) & attacks_bb<PAWN>(s, c)) activity += 20;
                    else activity += 10;
                }

                if (mobility_count < 4) activity -= 10;
            }

            if (c == WHITE) whiteUnits.push_back({c, pt, s, activity, piece_type_name(pt)});
            else blackUnits.push_back({c, pt, s, activity, piece_type_name(pt)});
        }
    }

    std::sort(whiteUnits.begin(), whiteUnits.end(), [](const UnitActivity& a, const UnitActivity& b) { return a.activityScore < b.activityScore; });
    std::sort(blackUnits.begin(), blackUnits.end(), [](const UnitActivity& a, const UnitActivity& b) { return a.activityScore < b.activityScore; });

    auto print_units_horizontal = [&](const std::vector<UnitActivity>& units, const std::string& colorName) {
        if (units.empty())
        {
            ss << "No " << colorName << " units found for ranking.\n";
            return;
        }

        ss << colorName << ": ";
        for (size_t i = 0; i < units.size(); ++i)
        {
            const auto& unit = units[i];
            std::string pieceSymbol = (unit.pieceName == "Knight") ? "N"
                                    : (unit.pieceName == "Bishop") ? "B"
                                    : (unit.pieceName == "Rook")   ? "R"
                                    : (unit.pieceName == "Queen")  ? "Q"
                                    : (unit.pieceName == "Pawn")   ? "P"
                                                                   : "?";
            ss << pieceSymbol << Trace::Helpers::square_str(unit.square) << "(" << unit.activityScore << ")";
            if (i < units.size() - 1) ss << ", ";
        }
        ss << "\n";

        if (!units.empty())
        {
            const auto& worstUnit = units[0];
            ss << "Makogonov " << colorName << ": Improve " << worstUnit.pieceName << " on "
               << Trace::Helpers::square_str(worstUnit.square) << " (activity: " << worstUnit.activityScore << ")";
        }
    };

    print_units_horizontal(whiteUnits, "White");
    ss << "\n";
    print_units_horizontal(blackUnits, "Black");
    ss << "\n\n";

    // =========================================================================
    // BLOCCO 2: OUTPUT AGGIORNATO (ADVANCED MAKOGONOV MODULES)
    // =========================================================================
    ss << "================================================================\n";
    ss << "=== ADVANCED MAKOGONOV MODULES ===\n\n";

    bool isEndgame = (gamePhase == "Endgame" || gamePhase == "Transition to endgame");
    std::vector<UnitActivity> whiteUnitsAdv;
    std::vector<UnitActivity> blackUnitsAdv;

    // Ricalcolo includendo il Re nel finale
    for (Square s = SQ_A1; s <= SQ_H8; ++s) {
        Piece p = pos.piece_on(s);
        if (p != NO_PIECE) {
            Color c = color_of(p);
            PieceType pt = type_of(p);
            int activity = 0;

            if (pt == KING) {
                if (isEndgame) {
                    // Nel finale, il Re DEVE centralizzarsi
                    int center_dist = std::abs(file_of(s) - FILE_D) + std::abs(rank_of(s) - RANK_4);
                    activity = (14 - center_dist) * 5; // Più vicino al centro = più alto
                    activity += 20; // Base offset
                } else {
                    continue; // Salta il Re nel mediogioco (la sua attività non è prioritaria)
                }
            } else {
                // Trova il pezzo corrispondente dal calcolo originale
                const auto& oldUnits = (c == WHITE) ? whiteUnits : blackUnits;
                for (const auto& u : oldUnits) {
                    if (u.square == s) {
                        activity = u.activityScore;
                        break;
                    }
                }
            }
            if (c == WHITE) whiteUnitsAdv.push_back({c, pt, s, activity, piece_type_name(pt)});
            else blackUnitsAdv.push_back({c, pt, s, activity, piece_type_name(pt)});
        }
    }

    std::sort(whiteUnitsAdv.begin(), whiteUnitsAdv.end(), [](const UnitActivity& a, const UnitActivity& b) { return a.activityScore < b.activityScore; });
    std::sort(blackUnitsAdv.begin(), blackUnitsAdv.end(), [](const UnitActivity& a, const UnitActivity& b) { return a.activityScore < b.activityScore; });

    // Target Finder Heuristic
    auto find_best_target = [&](Color c, PieceType pt, Square sq) -> Square {
        Bitboard attacks = 0;
        if (pt == KNIGHT) attacks = attacks_bb<KNIGHT>(sq);
        else if (pt == BISHOP) attacks = attacks_bb<BISHOP>(sq, pos.pieces());
        else if (pt == ROOK) attacks = attacks_bb<ROOK>(sq, pos.pieces());
        else if (pt == QUEEN) attacks = attacks_bb<QUEEN>(sq, pos.pieces());
        else if (pt == KING) attacks = attacks_bb<KING>(sq);

        attacks &= ~pos.pieces(c); // Rimuovi case occupate da pezzi amici
        
        Square best_sq = SQ_NONE;
        int best_score = -9999;
        
        while(attacks) {
            Square target = pop_lsb(attacks);
            int score = 0;
            // Centralizzazione
            int center_dist = std::abs(file_of(target) - FILE_D) + std::abs(rank_of(target) - RANK_4);
            score -= center_dist * 5;
            
            // Avanzamento
            if (c == WHITE) score += rank_of(target) * 2;
            else score -= (7 - rank_of(target)) * 2; 
            
            if (score > best_score) {
                best_score = score;
                best_sq = target;
            }
        }
        return best_sq;
    };

    auto print_advanced_makogonov = [&](const std::vector<UnitActivity>& units, Color c) {
        if (units.empty()) return;
        std::string colorName = (c == WHITE) ? "White" : "Black";
        
        // 1. Worst Piece & Target Finder
        const auto& worstUnit = units.front();
        ss << colorName << " Advanced Analysis:\n";
        ss << "  - Makogonov Target: " << worstUnit.pieceName << " on " 
           << Trace::Helpers::square_str(worstUnit.square) << " is your worst piece. ";
           
        if (worstUnit.pieceType != PAWN) {
            Square target = find_best_target(c, worstUnit.pieceType, worstUnit.square);
            if (target != SQ_NONE) {
                ss << "Best immediate reroute direction is towards " << Trace::Helpers::square_str(target) << ".\n";
            } else {
                ss << "It has no immediate squares to improve. Consider pawn breaks to free it.\n";
            }
        } else {
            ss << "Consider advancing it to gain space or trading it.\n";
        }

        // 2. MVP
        const auto& bestUnit = units.back();
        ss << "  - MVP (Most Valuable Piece): " << bestUnit.pieceName << " on " 
           << Trace::Helpers::square_str(bestUnit.square) << " (Activity: " << bestUnit.activityScore << "). "
           << "This is the absolute anchor of your position. Avoid trading it.\n";

        // 3. Diagnostics (Bad Bishop / Lazy King)
        if (worstUnit.pieceType == BISHOP) {
            Color b_color = (file_of(worstUnit.square) + rank_of(worstUnit.square)) % 2 == 0 ? BLACK : WHITE;
            Bitboard pawns = pos.pieces(c, PAWN);
            int same_color_pawns = 0;
            while(pawns) {
                Square p_sq = pop_lsb(pawns);
                Color p_color = (file_of(p_sq) + rank_of(p_sq)) % 2 == 0 ? BLACK : WHITE;
                if (p_color == b_color) same_color_pawns++;
            }
            if (same_color_pawns >= 4) { 
                ss << "  * Diagnostic: This Bishop is a 'Bad Bishop', heavily restricted by your own pawns. Try to trade it or break the pawn structure.\n";
            }
        } else if (worstUnit.pieceType == KING && isEndgame) {
            ss << "  * Diagnostic: Your King is dangerously passive for an endgame. Centralize it immediately to support your remaining pieces.\n";
        }
        ss << "\n";
    };

    print_advanced_makogonov(whiteUnitsAdv, WHITE);
    print_advanced_makogonov(blackUnitsAdv, BLACK);

    return ss.str();
}

} // namespace Eval
} // namespace Alexander