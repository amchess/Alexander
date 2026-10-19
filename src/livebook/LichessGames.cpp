#ifdef USE_LIVEBOOK
    #include "LichessGames.h"

    #include <algorithm>
    #include <iterator>

    #include "../handicap/evaluate_handicap.h"

using namespace Alexander::Livebook;

namespace {

struct RatingBand {
    int         maxElo;  // Upper bound of the band, inclusive
    const char* name;
    const char* ratings;  // Lichess rating groups (each one goes up to the next one)
    const char* players;  // The same groups, readable
};

// The bands of the handicap mode (handicap/evaluate_handicap.h): Beginner 1320-1999,
// Intermediate 2000-2199, Advanced 2200-2399, Expert 2400-3190 (maximum of the engine).
// The rating groups of the Lichess explorer are 0, 1000, 1200, 1400, 1600, 1800, 2000,
// 2200 (2200-2499) and 2500 (2500 and more).
constexpr RatingBand RATING_BANDS[] = {
  {Alexander::Eval::BEGINNER_MAX_ELO, "Beginner", "1200,1400,1600,1800", "1200-1999"},
  {Alexander::Eval::INTERMEDIATE_MAX_ELO, "Intermediate", "2000", "2000-2199"},
  {Alexander::Eval::ADVANCED_MAX_ELO, "Advanced", "2200", "2200-2499"},
  {Alexander::Eval::MAX_ELO, "Expert", "2500", "2500+"}};

const RatingBand& rating_band(const int elo) {
    for (const auto& band : RATING_BANDS)
    {
        if (elo <= band.maxElo)
        {
            return band;
        }
    }

    return RATING_BANDS[std::size(RATING_BANDS) - 1];
}

}

// topGames=0, recentGames=0: only the statistics are needed, not the game references
LichessGames::LichessGames(std::string token_) :
    LichessOpening(std::string(LICHESS_EXPLORER_URL) + "/lichess?topGames=0&recentGames=0&",
                   std::move(token_)) {}

std::string LichessGames::format_url(const Position& position_) {
    // Strength of the engine: UCI_Elo / ELO_CB when the strength is limited, 3190 otherwise
    const int   elo  = Alexander::Eval::handicapConfig.uciElo;
    const auto& band = rating_band(elo);

    std::string fen = position_.fen();
    std::replace(fen.begin(), fen.end(), ' ', '_');

    const std::string full_uri = endpoint + "ratings=" + band.ratings + "&fen=" + fen;

    if (lastBand != band.name)
    {
        lastBand = band.name;
        report(full_uri, "engine Elo " + std::to_string(elo) + " (" + band.name
                           + "): games of the players rated " + band.players);
    }

    return full_uri;
}

#endif
