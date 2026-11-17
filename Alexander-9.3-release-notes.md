# Alexander 9.3

## New Monte-Carlo Tree Search

Alexander 9.3 gets the new MCTS of ShashChess 42.3: same algorithm and options, adapted to Alexander's classical evaluation. It is used only with "MCTS by Shashin" on (off by default), in Petrosian positions (MCTS Explore extends it to other kinds of positions). Everything else is as in 9.2.

- **Hybrid and pure modes.** With MCTSThreads < Threads the main thread searches with alpha-beta and the first MCTSThreads helpers search the tree, cooperating through the shared transposition table and histories. With MCTSThreads >= Threads every thread, the main one included, searches the tree and the move played comes from the tree. In 9.2 the main thread always searched with alpha-beta, so with a single thread MCTS never ran.
- **A new tree.** Moves are selected with the PUCT formula, with priors from shallow alpha-beta searches. A visited leaf is first searched deeper by alpha-beta, one ply per visit, and then expanded. Mates, tablebase results and stalemates are proven and propagated up the tree (MCTS-Solver). The tree is kept from one move to the next and may use as much memory as the Hash option (64 MB at least), besides the hash table.
- **MCTS Multi Strategy and MCTS Multi MinVisits** keep their names and defaults but have a new meaning: the percentage of expansions among the visits of a leaf, and the minimum number of visits of every root move. Both now apply with any number of MCTS threads (see the README).
- **Info line.** At the start of every search an "info string" line tells whether MCTS was started, with the mode and the number of MCTS threads, or not, with the reason.
- **MultiPV.** The hybrid mode now works with MultiPV greater than 1 too (before, MCTS was off with MultiPV > 1). Pure MCTS stays off with MultiPV > 1, because the tree would only report the lines without searching them: the search is then the usual alpha-beta one.
- **Unchanged without MCTS.** With "MCTS by Shashin" off the search is the same as in 9.2: same bench, 3905509 nodes.
- **README.** The MCTS section is rewritten: it still described the previous MCTS.

## Tests

The measurements were made with ShashChess 42.3, which shares the MCTS code; the details are in its release notes. On 55 positions where MCTS starts, the hybrid mode with Threads 3 and MCTSThreads 1 solved more positions than alpha-beta alone (37.0 against 34.6 at 5 s) and than the previous MCTS (35.9 against 33.0). The lead held with MultiPV 3 (42.3 against 41.0) and at 30 s with Threads 2 (42.1 against 39.1). Pure MCTS on a single thread was not stronger than alpha-beta.

Alexander itself was checked functionally (info lines, pure and hybrid modes, the MultiPV rule, an unchanged bench with MCTS off); its results with MCTS on test suites have not been measured yet.

## Suggested settings

Use the hybrid mode, with more threads than MCTS threads: for instance Threads = 3 and MCTSThreads = 1. With the default values (Threads 1, MCTSThreads 1), turning on "MCTS by Shashin" gives pure MCTS on a single thread. MCTS Explore was tested only with pure MCTS on a single thread, where it solved fewer positions.

## Thanks

Many thanks to Peter M. for testing the new MCTS with ShashChess, and to Stephan Nicolet for the original MCTS work.
