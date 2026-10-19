#ifndef LICHESS_MASTER_H
#define LICHESS_MASTER_H
#ifdef USE_LIVEBOOK
    #include "LichessOpening.h"


namespace Alexander::Livebook {
class LichessMaster final: public LichessOpening {
   public:
    explicit LichessMaster(std::string token_);
};
}


#endif  //LICHESS_MASTER_H
#endif
