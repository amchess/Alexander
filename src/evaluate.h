/*
  Alexander, a UCI chess playing engine derived from Stockfish
  Copyright (C) 2004-2025 The Alexander developers (see AUTHORS file)

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

#ifndef EVALUATE_H_INCLUDED
#define EVALUATE_H_INCLUDED

#include <string>
#include <unordered_map>  //for classical

#include "types.h"

namespace Alexander {

class Position;
class OptionsMap;  //for classical

namespace Eval {

std::string trace(Position& pos);  //for classical

//true handicap mode begin
extern bool limitStrength, pawnsToEvaluate, winnableToEvaluate, imbalancesToEvaluate,
  handicappedAvatarPlayer, handicappedDepth;
extern int uciElo, RandomEvalPerturb;
Value      evaluate(const Position& pos);
void       loadAvatar(const std::string& fname);  //avatar
void       initHandicapMode(const OptionsMap&);   //handicap mode
//true handicap mode end

}  // namespace Eval

}  // namespace Alexander

#endif  // #ifndef EVALUATE_H_INCLUDED
