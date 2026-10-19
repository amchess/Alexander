#ifdef USE_LIVEBOOK
    #include "LichessUsers.h"

using namespace Alexander::Livebook;

LichessUsers::LichessUsers(std::string token_) :
    LichessOpening(std::string(LICHESS_EXPLORER_URL) + "/lichess?topGames=0&recentGames=0&",
                   std::move(token_)) {}
#endif
