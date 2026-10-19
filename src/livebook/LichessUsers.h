#ifndef LICHESS_USERS_H
#define LICHESS_USERS_H
#ifdef USE_LIVEBOOK
    #include "LichessOpening.h"

namespace Alexander::Livebook {
class LichessUsers final: public LichessOpening {
   public:
    explicit LichessUsers(std::string token_);
};
};


#endif  //LICHESS_USERS_H
#endif
