#include "../../evaluate.h"
#include "trace.h"
#include "../evaluate_handicap.h"
#include "../../uci.h"
#include "../../movegen.h"
#include "../../bitboard.h"
#include <sstream>
#include <iomanip>
#include "knights.h"

namespace Alexander {

using namespace Trace;

namespace Eval {

std::string analyze_knights(const Position& pos, int phase, const std::string& shashinZone) {
    (void)phase; // Silence warning
    std::stringstream ss;
    
    // =========================================================================
    // BLOCCO 1: OUTPUT ORIGINALE INTATTO AL 100%
    // =========================================================================
    Trace::Helpers::append_term_row(ss, pos, phase, "Knights", KNIGHT);
    
    // Pre-calculate variables needed for both blocks
    float white_density = 0.0f;
    float black_density = 0.0f;
    float deltak = 0.0f;
    
    if (Alexander::Eval::handicapConfig.pawnsToEvaluate)
    {
        ss << "=== KNIGHTS SUBELEMENTS ===\n";
        // Calcola la densità d'impacchettamento
        auto calculate_packing_density = [](const Position& position, Color c) {
            Bitboard short_range_pieces =
              position.pieces(c, KNIGHT) | position.pieces(c, PAWN) | position.pieces(c, KING);

            int piece_count = popcount(short_range_pieces);
            if (piece_count <= 1)
                return 0.0f;

            int      min_file = 7, max_file = 0, min_rank = 7, max_rank = 0;
            Bitboard pieces = short_range_pieces;

            while (pieces)
            {
                Square s = pop_lsb(pieces);
                File   f = file_of(s);
                Rank   r = rank_of(s);

                if (f < min_file)
                    min_file = f;
                if (f > max_file)
                    max_file = f;
                if (r < min_rank)
                    min_rank = r;
                if (r > max_rank)
                    max_rank = r;
            }

            int width  = max_file - min_file + 1;
            int height = max_rank - min_rank + 1;
            int area   = width * height;

            return static_cast<float>(piece_count) / area;
        };

        white_density = calculate_packing_density(pos, WHITE);
        black_density = calculate_packing_density(pos, BLACK);
        deltak        = white_density - black_density;
        ss << "Packing Density Analysis (Shashin theory. Short-range pieces: Knights, Pawns, King):\n";
        ss << "=====================================================================\n";
        ss << "White packing density: " << std::fixed << std::setprecision(3) << white_density << "\n";
        ss << "Black packing density: " << std::fixed << std::setprecision(3) << black_density << "\n";
        ss << "deltak (White - Black): " << std::fixed << std::setprecision(3) << deltak << "\n";

        // Interpretazione basata sulla zona di Shashin
        std::string zone = shashinZone;  // Usa la variabile già calcolata all'inizio della trace
        std::string recommendation;

        if (zone.find("Capablanca") != std::string::npos && zone.find("Chaos") == std::string::npos)
        {
            if (deltak < -0.05f)
                recommendation = "Avoid exchanges (deltak < 0 in equal position)";
            else
                recommendation = "Normal play (deltak >= 0)";
        }
        else if (zone.find("Petrosian") != std::string::npos)
        {
            if (deltak > 0.05f)
                recommendation = "Good defensive structure";
            else
                recommendation = "Improve packing density (deltak <= 0 in defensive position)";
        }
        else if (zone.find("Tal") != std::string::npos)
        {
            recommendation = "Aggressive play tolerated (deltak negative accepted in attacking position)";
        }
        else
        {
            recommendation = "Normal play";
        }

        ss << "Recommendation: " << recommendation << "\n";

        // Aggiungi una valutazione qualitativa della densità
        auto evaluate_density = [](float density) -> std::string {
            if (density > 0.7f)
                return "Very dense (secure structure)";
            if (density > 0.5f)
                return "Dense (good coordination)";
            if (density > 0.3f)
                return "Normal density";
            if (density > 0.1f)
                return "Sparse (watch for weaknesses)";
            return "Very sparse (potential weaknesses)";
        };

        ss << "White structure: " << evaluate_density(white_density) << "\n";
        ss << "Black structure: " << evaluate_density(black_density) << "\n";
        ss << "=====================================================================\n";
        ss << "\n";
    }

    // =========================================================================
    // BLOCCO 2: OUTPUT AGGIORNATO (ADVANCED MODULES)
    // =========================================================================
    ss << "================================================================\n";
    ss << "=== ADVANCED KNIGHT MODULES ===\n\n";

    // 1. ENGINE NUMERICAL SCORES
    ScoreForClassical w_k = Trace::scores[KNIGHT][WHITE];
    ScoreForClassical b_k = Trace::scores[KNIGHT][BLACK];
    ScoreForClassical net_k = w_k - b_k;

    ss << "Knight Positional Scores (Mobility, Outposts, Centralization):\n"
       << "  White : " << std::setw(4) << mg_value(w_k) << " (MG) / " << std::setw(4) << eg_value(w_k) << " (EG)\n"
       << "  Black : " << std::setw(4) << mg_value(b_k) << " (MG) / " << std::setw(4) << eg_value(b_k) << " (EG)\n"
       << "  Net   : " << std::setw(4) << mg_value(net_k) << " (MG) / " << std::setw(4) << eg_value(net_k) << " (EG)\n\n";

    // 2. STRUCTURAL DETECTION: Potential Outposts (Permanent Holes)
    ss << "Structural Outpost Analysis (Permanent Holes):\n";
    
    auto is_hole = [&](Square sq, Color knight_color) {
        File f = file_of(sq);
        Rank r = rank_of(sq);
        Color enemy = (knight_color == WHITE) ? BLACK : WHITE;
        Bitboard enemy_pawns = pos.pieces(enemy, PAWN);
        
        File adj_files[2] = { File(f - 1), File(f + 1) };
        for (File af : adj_files) {
            if (af >= FILE_A && af <= FILE_H) { 
                if (knight_color == WHITE) {
                    for (Rank ar = Rank(r + 1); ar <= RANK_7; ++ar) {
                        if (enemy_pawns & make_square(af, ar)) return false; 
                    }
                } else {
                    for (Rank ar = Rank(r - 1); ar >= RANK_2; --ar) {
                        if (enemy_pawns & make_square(af, ar)) return false; 
                    }
                }
            }
        }
        return true; 
    };

    std::vector<Square> w_holes;
    std::vector<Square> b_holes;
    
    for (File f = FILE_B; f <= FILE_G; ++f) {
        for (Rank r = RANK_4; r <= RANK_6; ++r) {
            Square sq = make_square(f, r);
            if (is_hole(sq, WHITE)) w_holes.push_back(sq);
        }
        for (Rank r = RANK_5; r >= RANK_3; --r) {
            Square sq = make_square(f, r);
            if (is_hole(sq, BLACK)) b_holes.push_back(sq);
        }
    }

    auto format_sq = [](Square sq) {
        std::string s;
        s += char('a' + file_of(sq));
        s += char('1' + rank_of(sq));
        return s;
    };

    ss << "  - White's Potential Outposts (Holes in Black's structure): ";
    if (w_holes.empty()) ss << "None detected in central files.\n";
    else {
        for (size_t i = 0; i < w_holes.size(); ++i) {
            ss << format_sq(w_holes[i]);
            if (i < w_holes.size() - 1) ss << ", ";
        }
        ss << "\n";
    }

    ss << "  - Black's Potential Outposts (Holes in White's structure): ";
    if (b_holes.empty()) ss << "None detected in central files.\n";
    else {
        for (size_t i = 0; i < b_holes.size(); ++i) {
            ss << format_sq(b_holes[i]);
            if (i < b_holes.size() - 1) ss << ", ";
        }
        ss << "\n";
    }
    
    // 3. CURRENT OUTPOST OCCUPATION
    ss << "\nActive Outpost Occupation:\n";
    Bitboard w_knights = pos.pieces(WHITE, KNIGHT);
    bool w_occ = false;
    while (w_knights) {
        Square sq = pop_lsb(w_knights);
        if (rank_of(sq) >= RANK_4 && is_hole(sq, WHITE)) {
            ss << "  - EXCELLENT: White Knight is actively occupying the unassailable outpost on " << format_sq(sq) << ".\n";
            w_occ = true;
        }
    }
    if (!w_occ) ss << "  - White has no Knights actively occupying advanced permanent outposts.\n";

    Bitboard b_knights = pos.pieces(BLACK, KNIGHT);
    bool b_occ = false;
    while (b_knights) {
        Square sq = pop_lsb(b_knights);
        if (rank_of(sq) <= RANK_5 && is_hole(sq, BLACK)) {
            ss << "  - EXCELLENT: Black Knight is actively occupying the unassailable outpost on " << format_sq(sq) << ".\n";
            b_occ = true;
        }
    }
    if (!b_occ) ss << "  - Black has no Knights actively occupying advanced permanent outposts.\n\n";

    if (Alexander::Eval::handicapConfig.pawnsToEvaluate) {
        ss << "Shashin Packing Density Analysis (Knights, Pawns, King):\n";
        ss << "  - White density: " << std::fixed << std::setprecision(3) << white_density << "\n";
        ss << "  - Black density: " << std::fixed << std::setprecision(3) << black_density << "\n";
        ss << "  - Delta (W-B):   " << std::fixed << std::setprecision(3) << deltak << "\n\n";
    }
    
    // 5. DYNAMIC OTB STRATEGIC SYNTHESIS
    ss << "--- OTB STRATEGIC SYNTHESIS ---\n";
    
    // Theory Branch
    if (w_occ || b_occ) {
        ss << "  * Theory : A Knight securely anchored on a 5th or 6th rank outpost is often mathematically superior to a Rook, as it exerts unbearable pressure without being easily dislodged.\n";
    } else if (!w_holes.empty() || !b_holes.empty()) {
        ss << "  * Theory : Permanent structural holes dictate the maneuver plans for Knights. Knights must be slowly rerouted to occupy these exact squares.\n";
    } else if (deltak > 0.1f || deltak < -0.1f) {
        ss << "  * Theory : Packing density dictates defensive solidity. A highly dense cluster of short-range pieces (Pawns/Knights) acts as an impenetrable shield.\n";
    } else {
        ss << "  * Theory : Knights are short-range operators. Without advanced outposts, their primary function is to blockade enemy pawns and support central breaks.\n";
    }

    // Action Branch
    ss << "  * Action : ";
    if (w_occ && !b_occ) {
        ss << "White has established a permanent Outpost. White must maintain this Knight at all costs; avoid trading it unless yielding a clear material gain. Black must sacrifice the exchange (Rook for Knight) if the Knight becomes intolerable.\n";
    } else if (b_occ && !w_occ) {
        ss << "Black has established a permanent Outpost. Black must maintain this Knight at all costs; avoid trading it unless yielding a clear material gain. White must sacrifice the exchange (Rook for Knight) if the Knight becomes intolerable.\n";
    } else if (!w_holes.empty() && w_holes.size() > b_holes.size()) {
        ss << "White has identified target holes in Black's camp. White's immediate positional goal is to maneuver a Knight to one of these squares. Black must prepare to challenge those squares with minor pieces.\n";
    } else if (!b_holes.empty() && b_holes.size() > w_holes.size()) {
        ss << "Black has identified target holes in White's camp. Black's immediate positional goal is to maneuver a Knight to one of these squares. White must prepare to challenge those squares with minor pieces.\n";
    } else if (deltak > 0.1f) {
        ss << "White's packing density is significantly higher. White possesses a secure, coiled structure. Black should avoid close-quarters combat and instead open long diagonals and files for heavy pieces.\n";
    } else if (deltak < -0.1f) {
        ss << "Black's packing density is significantly higher. Black possesses a secure, coiled structure. White should avoid close-quarters combat and instead open long diagonals and files for heavy pieces.\n";
    } else {
        ss << "No immediate Knight outposts exist. Keep Knights centralized to maximize their 8-square jump radius, and use them to support pawn advances.\n";
    }

    ss << "\n";
    return ss.str();
}

} // namespace Eval
} // namespace Alexander