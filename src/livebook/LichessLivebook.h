#ifndef JSON_LIVEBOOK_H
#define JSON_LIVEBOOK_H
#ifdef USE_LIVEBOOK

    #include "json/json.hpp"
    #include "BaseLivebook.h"

namespace Alexander::Livebook {

// Since March 2026 the Lichess services are on lichess.org (no more lichess.ovh) and the
// opening explorer (masters, lichess, player) requires an OAuth2 / personal API token.
// The tablebase is still open to anonymous requests.
inline constexpr const char* LICHESS_EXPLORER_URL  = "https://explorer.lichess.org";
inline constexpr const char* LICHESS_TABLEBASE_URL = "https://tablebase.lichess.org";

class LichessLivebook: public BaseLivebook {
   public:
    ~LichessLivebook() override = default;

    std::vector<std::pair<std::string, Analysis>> lookup(const Position& position) override;

   protected:
    virtual std::string format_url(const Position& position)  = 0;
    virtual std::string parse_uci(const nlohmann::json& move) = 0;
    // The returned analysis must be from the point of view of the side to move
    virtual Analysis* parse_analysis(const nlohmann::json& move, const Position& position) = 0;

    // Extra HTTP headers (e.g. Authorization)
    [[nodiscard]] virtual std::vector<std::string> request_headers() const { return {}; }
    // Appended to the message printed when the server refuses the credentials
    [[nodiscard]] virtual std::string auth_hint() const { return ""; }
    // True if the server streams newline delimited JSON (one snapshot per line)
    [[nodiscard]] virtual bool is_nd_json() const { return false; }
};
}

#endif  //JSON_LIVEBOOK_H
#endif
