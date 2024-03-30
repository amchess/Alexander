/*
  Alexander, a UCI chess playing engine derived from Stockfish
  Copyright (C) 2004-2024 Andrea Manzo, K.Kiniama and Alexander developers (see AUTHORS file)

  Alexander is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Alexander is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <iostream>
#include <unordered_map>

#include "bitboard.h"
#include "evaluate.h"
#include "endgame.h"
#include "misc.h"
#include "position.h"
#include "psqt.h"
#include "tune.h"
#include "types.h"
#include "uci.h"
#include "learn/learn.h"  //learning
using namespace Alexander;

int main(int argc, char* argv[]) {

    std::cout << engine_info() << std::endl;
    UCI uci(argc, argv);   //Khalid
    LD.init(uci.options);  //Kelly
    Bitboards::init();
    Position::init();
    Tune::init(uci.options);

    Eval::initHandicapMode(uci.options);
    Bitbases::init();
    Eval::loadAvatar(uci.options["Avatar File"]);  //handicap mode

    PSQT::init();
    Bitbases::init();
    Endgames::init();

    uci.loop();

    return 0;
}
