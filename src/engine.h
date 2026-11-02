/*
  Alexander, a UCI chess playing engine derived from Stockfish
  Copyright (C) 2004-2026 The Alexander developers (see AUTHORS file)

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

#ifndef ENGINE_H_INCLUDED
#define ENGINE_H_INCLUDED

#include <functional>
#include <filesystem>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

#include "misc.h"
#include "history.h"
#include "numa.h"  //for classical: no NNUE includes
#include "position.h"
#include "search.h"
#include "syzygy/tbprobe.h"  // for Alexander::Depth
#include "thread.h"
#include "tt.h"
#include "types.h"
#include "ucioption.h"
#include "book/book_manager.h"  // book management

namespace Alexander {

constexpr int MaxHashMB = Is64Bit ? 33554432 : 2048;
extern int    MaxThreads;

class Engine {
   public:
    using InfoShort = Search::InfoShort;
    using InfoFull  = Search::InfoFull;
    using InfoIter  = Search::InfoIteration;

    Engine(std::optional<std::filesystem::path> path = std::nullopt);

    // Cannot be movable due to components holding backreferences to fields
    Engine(const Engine&)            = delete;
    Engine(Engine&&)                 = delete;
    Engine& operator=(const Engine&) = delete;
    Engine& operator=(Engine&&)      = delete;

    ~Engine() { wait_for_search_finished(); }

    std::variant<u64, PositionSetError> perft(const std::string& fen, Depth depth, bool isChess960);

    // non blocking call to start searching
    void go(Search::LimitsType&);
    // non blocking call to stop searching
    void stop();

    // blocking call to wait for search to finish
    void wait_for_search_finished();
    // set a new position, moves are in UCI format
    std::optional<PositionSetError> set_position(const std::string&              fen,
                                                 const std::vector<std::string>& moves);

    // modifiers

    bool set_numa_config_from_option(const std::string& o);
    void resize_threads();
    void init_bookMan(int bookIndex);   // book management
    void resize_full(usize requested);  // full threads patch
    void set_tt_size(usize mb);
    void set_ponderhit(bool);
    void search_clear();
#ifdef USE_LIVEBOOK
    void setLiveBookURL(const std::string& newURL);
    void setLiveBookTimeout(usize newTimeoutMS);
#endif

    void set_on_update_no_moves(std::function<void(const InfoShort&)>&&);
    void set_on_update_full(std::function<void(const InfoFull&)>&&);
    void set_on_iter(std::function<void(const InfoIter&)>&&);
    void set_on_bestmove(std::function<void(std::string_view, std::string_view)>&&);
    void set_on_start(std::function<void()>&&);
    //for classical: no network to load, verify or save

    // utility functions

    void trace_eval() const;

    const OptionsMap& get_options() const;
    OptionsMap&       get_options();
    BookManager&      get_bookMan() { return bookMan; }  // book management
    int               get_hashfull(int maxAge = 0) const;

    std::string                     fen() const;
    std::optional<PositionSetError> flip();
    std::string                     visualize() const;
    void show_moves_bookMan(const Position& position);  // book management
    std::vector<std::pair<usize, usize>> get_bound_thread_count_by_numa_node() const;
    std::string                          get_numa_config_as_string() const;
    std::string                          numa_config_information_as_string() const;
    std::string                          thread_allocation_information_as_string() const;
    std::string                          thread_binding_information_as_string() const;

    Position pos;  // from learning

   private:
    const std::filesystem::path binaryDirectory;

    NumaReplicationContext numaContext;

    StateListPtr states;

    OptionsMap                           options;
    ThreadPool                           threads;
    TranspositionTable                   tt;
    BookManager                          bookMan;  // book management
    Search::SearchManager::UpdateContext updateContext;
    std::map<NumaIndex, SharedHistories> sharedHists;
};

}  // namespace Alexander

#endif  // #ifndef ENGINE_H_INCLUDED