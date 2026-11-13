#ifndef LEARN_H_INCLUDED
#define LEARN_H_INCLUDED

#include <cstdint>
#include <set>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include "../types.h"
#include "../ucioption.h"
#include "../position.h"

enum class LearningMode {
    Off      = 1,
    Standard = 2,
    Self     = 3,
};

struct LearningMove {
    Alexander::Depth depth       = 0;
    Alexander::Value score       = Alexander::VALUE_NONE;
    Alexander::Move  move        = Alexander::Move::none();
    int              performance = 100;
};

struct PersistedLearningMove {
    Alexander::Key key{};
    LearningMove   learningMove;
};

// [SC:learning] Size and time of the experience file when it was last read or written by
// this engine: if they are different, another engine has written the file in the meantime
struct ExperienceFileStamp {
    bool           exists = false;
    std::uintmax_t size   = 0;
    long long      time   = 0;

    bool operator==(const ExperienceFileStamp& o) const {
        return exists == o.exists && size == o.size && time == o.time;
    }
};

struct QLearningMove {
    PersistedLearningMove persistedLearningMove;
    int                   materialClamp;
};

class LearningData {
    bool         isPaused;
    bool         isReadOnly;
    bool         needPersisting;
    bool         isSuspended;
    LearningMode learningMode;
    // [SC:learning] The root position of the search that found the game decided (see pause())
    Alexander::Key pausedKey;

    std::unordered_multimap<Alexander::Key, LearningMove*> HT;
    std::vector<void*>                                     mainDataBuffers;
    std::vector<void*>                                     newMovesDataBuffers;

    // [SC:learning] begin
    // More engines may share the experience file: the two sides of a cutechess match played
    // by the same engine, and all the games played at the same time with concurrency > 1.
    // Each one has its own table and saves all of it: before, the last one to save deleted
    // what the others had learned (and two saving at the same time, as they do when
    // cutechess sends 'ucinewgame' or 'quit' to both, could leave no file at all).
    // Now the file is merged into the table before it is replaced, and what this engine
    // has learned since its last save (the moves below) prevails on the file.
    std::set<std::pair<Alexander::Key, std::uint16_t>> dirtyMoves;
    bool                                               dirtyAll;
    // The experience file as this engine last read or wrote it (see persist())
    ExperienceFileStamp masterStamp;
    bool                masterSynced;

    // allowPartial: a file that was being written when its engine was terminated is read
    // up to its last complete move
    bool load(const std::string& filename, bool allowPartial = false);
    // Reads a file written by another engine into the table, keeping no buffer
    bool merge(const std::string& filename);
    // fromDisk: the move comes from a file and not from this engine's own games
    bool insert_or_update(PersistedLearningMove* plm, bool qLearning, bool fromDisk);
    [[nodiscard]] bool is_dirty(const PersistedLearningMove& plm) const;
    // Writes the table; consolidate: always to the experience file, also with Concurrent
    // Experience. The caller holds the lock on the experience files
    bool save(const Alexander::OptionsMap& o, bool consolidate);
    // [SC:learning] end

   public:
    LearningData();
    ~LearningData();

    // [SC:learning] Learning stops when the game is decided, but only for that game: the
    // experience is saved and nothing more is learned while the engine is given positions
    // that continue it. A 'position' command that does not continue it resumes learning
    // (Engine::set_position), as 'ucinewgame' does. Before, only 'ucinewgame' resumed it, and
    // the console and the GUIs in analysis mode never send it: after a single decided position
    // (a mate, more than 5 pawns, fewer than 5 pieces, move > 75) nothing more was learned.
    void               pause(Alexander::Key decidedKey);
    void               resume();
    [[nodiscard]] bool is_paused() const { return isPaused; };
    // True if 'key' is the position where the paused game was found decided: a 'position'
    // command whose history contains it continues that game
    [[nodiscard]] bool is_decided_position(Alexander::Key key) const {
        return isPaused && key == pausedKey;
    }

    // [SC:learning] bench and speedtest: learning works as if it were Off (nothing learned,
    // paused, saved or probed), and then everything is as before
    void set_suspended(bool s) { isSuspended = s; }

    void quick_reset_exp();
    void set_learning_mode(Alexander::OptionsMap& options, const std::string& lm);
    [[nodiscard]] LearningMode learning_mode() const;
    [[nodiscard]] bool         is_enabled() const {
        return learningMode != LearningMode::Off && !isSuspended;
    }

    void               set_readonly(bool ro) { isReadOnly = ro; }
    [[nodiscard]] bool is_readonly() const { return isReadOnly; }

    void clear();
    void init(Alexander::OptionsMap& o);
    // [SC:learning] True if the experience file is complete on disk
    bool persist(const Alexander::OptionsMap& o);

    void add_new_learning(Alexander::Key key, const LearningMove& lm);

    int probeByMaxDepthAndScore(Alexander::Key key, const LearningMove*& learningMove);
    const LearningMove*        probe_move(Alexander::Key key, Alexander::Move move);
    std::vector<LearningMove*> probe(Alexander::Key key);
    static void                sortLearningMoves(std::vector<LearningMove*>& learningMoves);
    static void                show_exp(const Alexander::Position& pos);
    // [SC:learning] GUI-readable variant ("info string" lines) for the "Show Experience" button
    static void show_exp_info(const Alexander::Position& pos, bool experienceBook);
};

extern LearningData LD;

#endif  // #ifndef LEARN_H_INCLUDED
