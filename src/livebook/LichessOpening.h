#ifndef LICHESS_OPENING_H
#define LICHESS_OPENING_H
#ifdef USE_LIVEBOOK

    #include "LichessLivebook.h"

namespace Alexander::Livebook {
class LichessOpening: public LichessLivebook {
   public:
    // endpoint_ must end with '?' or '&': the position is appended as "fen=..."
    // token_ is the Lichess API token (required by the explorer since March 2026)
    LichessOpening(std::string endpoint_, std::string token_);
    ~LichessOpening() override = default;

   protected:
    std::string endpoint;
    std::string token;

    std::string parse_uci(const nlohmann::json& move_) override;
    Analysis*   parse_analysis(const nlohmann::json& move_, const Position& position_) override;
    std::string format_url(const Position& position_) override;

    [[nodiscard]] std::vector<std::string> request_headers() const override;
    [[nodiscard]] std::string              auth_hint() const override;
};
}

#endif  //LICHESS_OPENING_H
#endif
