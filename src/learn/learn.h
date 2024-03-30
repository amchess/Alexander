#ifndef LEARN_H_INCLUDED
#define LEARN_H_INCLUDED

#include <unordered_map>
#include "../types.h"
#include "../ucioption.h"

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
    Alexander::Key key;
    LearningMove   learningMove;
};

class LearningData {
   private:
    bool         isPaused;
    bool         isReadOnly;
    bool         needPersisting;
    LearningMode learningMode;

    std::unordered_multimap<Alexander::Key, LearningMove*> HT;
    std::vector<void*>                                     mainDataBuffers;
    std::vector<void*>                                     newMovesDataBuffers;

   private:
    bool load(const std::string& filename);
    void insert_or_update(PersistedLearningMove* plm, bool qLearning);

   public:
    LearningData();
    ~LearningData();

    void        pause();
    void        resume();
    inline bool is_paused() const { return isPaused; };

    void         set_learning_mode(Alexander::OptionsMap options, const std::string& lm);
    LearningMode learning_mode() const;
    inline bool  is_enabled() const { return learningMode != LearningMode::Off; }

    void        set_readonly(bool ro) { isReadOnly = ro; }
    inline bool is_readonly() const { return isReadOnly; }

    void clear();
    void init(Alexander::OptionsMap& o);
    void persist(const Alexander::OptionsMap& o);

    void add_new_learning(Alexander::Key key, const LearningMove& lm);

    int probeByMaxDepthAndScore(Alexander::Key key, const LearningMove*& learningMove);
    const LearningMove* probe_move(Alexander::Key key, Alexander::Move move);
};

extern LearningData LD;

#endif  // #ifndef LEARN_H_INCLUDED
