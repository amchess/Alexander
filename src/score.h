/*
  Alexander, a UCI chess playing engine derived from Stockfish
  Copyright (C) 2004-2024 The Alexander developers (see AUTHORS file)

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

#ifndef SCORE_H_INCLUDED
#define SCORE_H_INCLUDED

#include <variant>
#include <utility>

#include "types.h"

namespace Alexander {

class Position;

class Score {
   public:
    struct Mate {
        int plies;
    };

    struct Tablebase {
        int  plies;
        bool win;
    };

    struct InternalUnits {
        int value;
    };

    Score() = default;
    Score(Value v, const Position& pos);

    template<typename T>
    bool is() const {
        return std::holds_alternative<T>(score);
    }

    template<typename T>
    T get() const {
        return std::get<T>(score);
    }

    template<typename F>
    decltype(auto) visit(F&& f) const {
        return std::visit(std::forward<F>(f), score);
    }

   private:
    std::variant<Mate, Tablebase, InternalUnits> score;
};

}

#endif  // #ifndef SCORE_H_INCLUDED
