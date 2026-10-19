#ifdef USE_LIVEBOOK
    #include "LichessPlayer.h"

using namespace Alexander::Livebook;

namespace {

std::string url_encode(const std::string& value_) {
    std::string encoded;

    if (char* escaped = curl_easy_escape(nullptr, value_.c_str(), static_cast<int>(value_.size())))
    {
        encoded = escaped;
        curl_free(escaped);
    }

    return encoded;
}

}

// recentGames=0: only the statistics are needed, not the game references
LichessPlayer::LichessPlayer(const std::string& player_, std::string color_, std::string token_) :
    LichessOpening(std::string(LICHESS_EXPLORER_URL) + "/player?player=" + url_encode(player_)
                     + "&recentGames=0&",
                   std::move(token_)),
    player(player_),
    color(std::move(color_)) {}

std::string LichessPlayer::format_url(const Position& position_) {
    auto fen = position_.fen();
    std::replace(fen.begin(), fen.end(), ' ', '_');

    return endpoint + "color="
         + (color == "both" ? position_.side_to_move() == WHITE ? "white" : "black" : color)
         + "&fen=" + fen;
}
#endif
