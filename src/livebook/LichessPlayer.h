#ifndef LICHESS_PLAYER_H
#define LICHESS_PLAYER_H

#ifdef USE_LIVEBOOK

    #include "LichessOpening.h"


namespace Alexander::Livebook {
class LichessPlayer final: public LichessOpening {
   public:
    LichessPlayer(const std::string& player_, std::string color_, std::string token_);
    ~LichessPlayer() override = default;

   protected:
    std::string player;
    std::string color;

    std::string format_url(const Position& position_) override;

    // The player endpoint streams newline delimited JSON
    [[nodiscard]] bool is_nd_json() const override { return true; }
};
}

#endif
#endif  //LICHESS_PLAYER_H
