#ifndef LICHESS_GAMES_H
#define LICHESS_GAMES_H
#ifdef USE_LIVEBOOK
    #include "LichessOpening.h"


namespace Alexander::Livebook {
class LichessGames final: public LichessOpening {
   public:
    explicit LichessGames(std::string token_);

   protected:
    // Only the games of the Lichess players in the rating band of the engine
    std::string format_url(const Position& position_) override;

   private:
    std::string lastBand;  // To report the band only when it changes
};
}

#endif  //LICHESS_GAMES_H
#endif
