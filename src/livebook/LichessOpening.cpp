#ifdef USE_LIVEBOOK
    #include "LichessOpening.h"

    #include <algorithm>
    #include <limits>

    #include "json/json.hpp"

using namespace Alexander::Livebook;

namespace {

uint32_t games_count(const nlohmann::json& move_, const char* key_) {
    if (!move_.contains(key_) || !move_[key_].is_number_integer())
    {
        return 0;
    }

    const auto games = move_[key_].get<int64_t>();

    return static_cast<uint32_t>(
      std::clamp<int64_t>(games, 0, std::numeric_limits<uint32_t>::max()));
}

}

LichessOpening::LichessOpening(std::string endpoint_, std::string token_) :
    endpoint(std::move(endpoint_)),
    token(std::move(token_)) {}

std::string LichessOpening::parse_uci(const nlohmann::json& move_) {
    if (!move_.contains("uci") || !move_["uci"].is_string())
    {
        return "";
    }

    return move_["uci"].get<std::string>();
}

Analysis* LichessOpening::parse_analysis(const nlohmann::json& move_, const Position& position_) {
    const auto white = games_count(move_, "white");
    const auto draws = games_count(move_, "draws");
    const auto black = games_count(move_, "black");

    // The explorer counts the results from White's point of view
    const auto wdl = position_.side_to_move() == WHITE ? new Wdl(white, draws, black)
                                                       : new Wdl(black, draws, white);

    return new Analysis(wdl);
}

std::string LichessOpening::format_url(const Position& position_) {
    std::string fen_encoded = position_.fen();
    std::replace(fen_encoded.begin(), fen_encoded.end(), ' ', '_');
    const std::string full_uri = this->endpoint + "fen=" + fen_encoded;

    return full_uri;
}

std::vector<std::string> LichessOpening::request_headers() const {
    if (token.empty())
    {
        return {};
    }

    return {"Authorization: Bearer " + token};
}

std::string LichessOpening::auth_hint() const {
    if (token.empty())
    {
        return ". The opening explorer requires a Lichess API token: create one (no scope is "
               "needed) at https://lichess.org/account/oauth/token and set it in the option "
               "'LiveBook Lichess Token'";
    }

    return ". The token in 'LiveBook Lichess Token' is invalid, expired or revoked";
}
#endif
