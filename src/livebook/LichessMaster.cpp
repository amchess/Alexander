#ifdef USE_LIVEBOOK
    #include "LichessMaster.h"

using namespace Alexander::Livebook;

// topGames=0: only the statistics are needed, not the game references
LichessMaster::LichessMaster(std::string token_) :
    LichessOpening(std::string(LICHESS_EXPLORER_URL) + "/masters?topGames=0&", std::move(token_)) {}
#endif
