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
    std::stringstream ss;
    Trace::Helpers::append_term_row(ss, pos, phase, "Knights", KNIGHT);
    if (Alexander::Eval::handicapConfig.pawnsToEvaluate)
    {
        ss << "=== KNIGHTS SUBELEMENTS ===\n";
        // Calcola la densità d'impacchettamento
        auto calculate_packing_density = [](const Position& pos, Color c) {
            Bitboard short_range_pieces =
              pos.pieces(c, KNIGHT) | pos.pieces(c, PAWN) | pos.pieces(c, KING);

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

        float white_density = calculate_packing_density(pos, WHITE);
        float black_density = calculate_packing_density(pos, BLACK);
        float deltak        = white_density - black_density;
        ss
          << "Packing Density Analysis (Shashin theory. Short-range pieces: Knights, Pawns, King):\n";
        ss << "=====================================================================\n";
        ss << "White packing density: " << std::fixed << std::setprecision(3) << white_density
           << "\n";
        ss << "Black packing density: " << std::fixed << std::setprecision(3) << black_density
           << "\n";
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
            recommendation =
              "Aggressive play tolerated (deltak negative accepted in attacking position)";
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

    return ss.str();
}


}

}  // namespace Alexander