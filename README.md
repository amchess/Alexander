# Introduction

![Alexander Logo](logo/Alexander.bmp)

Alexander is a free UCI chess engine derived from Stockfish family chess engines.
For the evaluation function, we utilize the collaboration between Leela Chess Zero and Stockfish, for which we express our sincere gratitude.
The goal is to apply Alexander Shashin theory exposed on the following book :
https://www.amazon.com/Best-Play-Method-Discovering-Strongest/dp/1936277468
to improve

- base engine strength
- engine's behaviour on the different positions types (requiring the corresponding algorithm) :
    - Tal
    - Capablanca
    - Petrosian
    - the mixed ones
       * Tal-Capablanca
       * Capablanca-Petrosian
       * Tal-Capablanca-Petrosian

Also during the search, to enhance it, we use both standard and Q/Self reinforcement learning.

## Terms of use

Shashchess is free, and distributed under the **GNU General Public License** (GPL). Essentially, this

means that you are free to do almost exactly what you want with the program, including distributing

it among your friends, making it available for download from your web site, selling it (either by

itself or as part of some bigger software package), or using it as the starting point for a software

project of your own.

The only real limitation is that whenever you distribute Alexander in some way, you must always

include the full source code, or a pointer to where the source code can be found. If you make any

changes to the source code, these changes must also be made available under the GPL.

For full details, read the copy of the GPL found in the file named _Copying.txt_.

## Files

This distribution of AlexanderPro consists of the following files:

- Readme.md, the file you are currently reading.
- Copying.txt, a text file containing the GNU General Public License.
- src, a subdirectory containing the full source code, including a Makefile and the compilation
    scripts makeAll.bat (Windows) and makeAll.sh (Linux).

## Uci options

### Hash Memory

#### Hash

_Integer, Default: 16, Min: 1, Max: 131072 MB (64-bit) : 2048 MB (32-bit)_


The amount of memory to use for the hash during search, specified in MB (megabytes). This
number should be smaller than the amount of physical memory for your system.
A modern formula to determine it is the following:

_(T x S / 100) MB_
where
_T = the average move time (in seconds)
S = the average node speed of your hardware_
A traditional formula is the following:
_(N x F x T) / 512_
where
_N = logical threads number
F = clock single processor frequency (MB)
T = the average move time (in seconds)_

#### Clear Hash

Button to clear the Hash Memory.
If the Never Clear Hash option is enabled, this button doesn't do anything.

### Threads

_Integer, Default: 1, Min: 1, Max: 512_
The number of threads to use during the search. This number should be set to the number of cores
(physical+logical) in your CPU.

### Ponder (checkbox)

_Boolean, Default: True_
Also called "Permanent Brain" : whether or not the engine should analyze when it is the opponent's
turn.


Usually not on the configuration window.

### MultiPV

_Integer, Default: 1, Min: 1, Max: 500_
The number of alternate lines of analysis to display. Specify 1 to just get the best line. Asking for
more lines slows down the search.
Usually not on the configuration window.

### UCI_Chess960 (checkbox)

Whether or not Alexander should play using Chess 960 mode. Usually not on the configuration
window.

### Move overhead 

_Default 30, min 0, max 5000_
In ms, the default value seems to be the best on Linux systems, but must be increased for slow GUI like Fritz. In general, on Windows system it seems a good value to be 100.

### Slow mover

_Default 84, min 10, max 1000_
 "Time usage percent": how much the engine thinks on a move. Many engines seem to move faster and the engine is behind in time clock. With lower values it plays faster, with higher values slower - of course always within the time control.

### Handicap mode

#### UCI_LimitStrength

Activate the handicap mode and the related following options: in this case, the evaluation function is always the classical one.

#### UCI_Elo (CB only for chessbase products users)

_Default 2850, min 1350, max 2850_
UCI-protocol compliant version of Strength parameter.
A very refined handicap mode based on the four famous sovietic chess school levels:
Internally the UCI_Elo value will be converted to a Strength value according to the following table:

- _beginner: elo < 2000_
- _intermediate: 2000 <= elo < 2200_
- _advanced: 2200 <= elo < 2400_
- _expert: elo >= 2400_

Every school corresponds to a different evaluation function, more and more refined.
The UCI_Elo feature is controlled by the chess GUI, and usually doesn't appear in the configuration
window.

#### Handicapped Depth
The engine stop calculating when it joins the handicapped depth, based on the following table:

Elo range     | Handicapped Depth |
| ----------- | ----------------- |
| [0,1999]    | [1,6]             |
| [2000,2199] | [7,9]             |
| [2200,2399] | [10,12]           |
| [2400,3190] | [13,20]           |

#### Simulate human blunders
If enabled, the engine not only simulates the thought process of a player of a certain level but also the mistakes he can make. These mistakes become more frequent the lower the player's Elo rating. This is the handicap mode implemented by Stockfish, but when combined with the previous options that simulate a player's thinking system, it truly approximates a "human avatar".

#### Avatar File
A file in .avt format with a player profile. You can edit this file containing the weights of Stockfish's classic evaluation terms, from 0 to 100, but with a private tool, we can generate these values to simulate a real player.
The private tool first analyzes the player's games and then generates his player card and avatar.
In this way, Alexander turns into the player's alter ego, simulating not only his playing strength but also his style:
no frustration for the OTB player who will not always lose. Once in a while, he will win, and most importantly, he will have an ideal sparring partner to improve.
The data sheet and our private NLG sw (Virtual trainer) will also allow him to understand his own mistakes verbally, simulating a live instructor!
Examples:

[Player card](examples/PlayerCard.xlsx)

[Avatar](examples/Avatar.avt)


### Sygyzy End Game table bases

Download at [http://olympuschess.com/egtb/sbases](http://olympuschess.com/egtb/sbases) (by Ronald De Man)

#### SyzygyPath

The path to the Syzygy endgame tablebases.this defines an absolute path on your computer to the
tablebase files, also on multiple paths separated with a semicolon (;) character (Windows), the colon
(:) character (OS X and Windows) character.
The folder(s) containing the Syzygy EGTB files. If multiple folders are used, separate them by the ;
(semicolon) character.

#### SygyzyProbeDepth

_Integer, Default: 1, Min: 1, Max: 100_
The probing tablebases depth (always the root position).
If you don't have a SSD HD,you have to set it to maximize the depth and kn/s in infinite analysis
and during a time equals to the double of that corresponding to half RAM size.
Choice a test position with a few pieces on the board (from 7 to 12). For example:

- Fen: _8/5r2/R7/8/1p5k/p3P3/4K3/8 w -- 0 1_ Solution : Ra4 (=)
- Fen: _1R6/7k/1P5p/5p2/3K2p1/1r3P1P/8 b - - 1 1_ Solution: 1...h5 !! (=)

#### Syzygy50MoveRule
    
Disable to let fifty-move rule draws detected by Syzygy tablebase probes count
as wins or losses. This is useful for ICCF correspondence games.
	
#### SygyzyProbeLimit

_Integer, Default: 6, Min: 0, Max: 6_
How many pieces need to be on the board before Alexander begins probing (even at the root).
Current default, obviously, is for 6-man.

### Advanced Chess Analyzer

Advanced analysis options, highly recommended for CC play

#### Full depth threads

_Integer, Default: 0, Min: 0, Max: 512_
The number of settled threads to use for a full depth brute force search. 
If the number is greater than threads number, all threads are for full depth brute force search.

### MonteCarlo Tree Search section (experimental: thanks to original Stephan Nicolet work)

#### MCTS by Shashin

_Boolean, Default: False_ If activated, thanks to Shashin theory, the engine will use the MonteCarlo Tree Search for Petrosian high, high middle and middle positions, in the manner specified by the following parameters. The idea is to exploit Lc0 best results in those positions types, because Lc0 uses mcts in the search.

#### MCTSThreads

_Integer, Default: 0, Min: 0, Max: 512_
The number of settled threads to use for MCTS search except the first (main) one always for alpha-beta search. 
In particular, if the number is greater than threads number, they will all do a montecarlo tree search, always except the first (main) for alpha-beta search.

#### MCTS Multi Strategy 

_Integer, Default: 20, Min: 0, Max: 100_ 
Only in multi mcts mode, for tree policy.

#### MCTS Multi MinVisits

_Integer, Default: 5, Min: 0, Max: 1000_
Only in multi mcts mode, for Upper Confidence Bound.

#### MCTS Explore

_Boolean, Default: False_ If activated, mcts is also performed for highly Tal type positions and Capablanca and Petrosian type positions.

### Live Book section (thanks to Eman's author Khalid Omar for windows builds)

#### LiveBook Proxy Url
_String, Default: "" (empty string)_  
The proxy URL to use for the live book. If empty, no proxy is used. The proxy should use the ChessDB REST API format.

#### LiveBook Proxy Diversity
_Boolean, Default: False_  
If enabled, the engine will play a random (best) move by the proxy (query and not querybest action).

#### LiveBook Lichess Games
_Boolean, Default: False_  
If enabled, the engine will use the Lichess live book by querying the Lichess API to access the game database available on the site. This option allows the engine to access a wide range of games played on Lichess to enhance its move choices.

#### LiveBook Lichess Masters
_Boolean, Default: False_  
If enabled, the engine will use the Lichess live book specifically for masters' games. This allows the engine to analyze games played at a high level and utilize the best moves made by master-level players.

#### LiveBook Lichess Player
_String, Default: "" (empty string)_  
The Lichess player name to use for the live book. If left empty, the engine will not query for the specific player's game data. This option is useful for studying or adapting the engine to a particular player's style.

#### LiveBook Lichess Player Color
_String, Default: "White"_  
Specifies the color the engine will play as in the Lichess live book for the specified player.
- **"White"**: The engine considers the games played by the specified player as White. When it's Black's turn, the move that performed best against the player will be chosen.
- **"Black"**: The engine considers the games played by the specified player as Black. When it's White's turn, the move that performed best against the player will be chosen.
- **"Both"**: The engine will always pretend to be the player, regardless of color, and choose the best-performing moves for the specified player.

#### LiveBook ChessDB
_Boolean, Default: False_  
If enabled, the engine will use the ChessDB live book by querying the ChessDB API.

#### LiveBook Depth
_Integer, Default: 255, Min: 1, Max: 255_  
Specifies the depth to reach using the live book in plies. The depth determines how many half-moves the engine will consider from the current position.

#### ChessDB Tablebase
_Boolean, Default: False_  
If enabled, allows the engine to query the ChessDB API for Tablebase data, up to 7 pieces. This provides perfect endgame knowledge for positions with up to 7 pieces.

#### Lichess Tablebase
_Boolean, Default: False_  
If enabled, allows the engine to query the Lichess API for Tablebase data, up to 7 pieces. This option also provides perfect endgame knowledge for positions with up to 7 pieces.

#### ChessDB Contribute
_Boolean, Default: False_  
If enabled, allows the engine to store a move in the queue of ChessDb to be analyzed.

### Full depth threads

_Default 0, min 0, max 512_ The number of threads doing a full depth analysis (brute force). Useful in analysis of particular hard positions to limit the strong pruning's drawbacks. 

### Variety  (checkbox)

Default is Off: no variety. The other values are "Standard" (no elo loss: randomicity in Capablanca zone) and Psychological (randomicity in Caos zones max).

### Concurrent Experience

_Boolean, Default: False_ 
Set this option to true when running under CuteChess and you experiences problems with concurrency > 1
When this option is true, the saved experience file name will be modified to something like experience-64a4c665c57504a4.bin
(64a4c665c57504a4 is random). Each concurrent instance of BrainLearn will have its own experience file name, however, all the concurrent instances will read "experience.bin" at start up.

### Persisted learning (checkbox)

Default is Off: no learning algorithm. The other values are "Standard" and "Self", this last to activate the [Q-learning](https://youtu.be/qhRNvCVVJaA?list=PLZbbT5o_s2xoWNVdDudn51XM8lOuZ_Njv), optimized for self play. Some GUIs don't write the experience file in some game's modes because the uci protocol is differently implemented

The persisted learning is based on a collection of one or more positions stored with the following format (similar to in memory Stockfish Transposition Table):

- _best move_
- _board signature (hash key)_
- _best move depth_
- _best move score_
- _best move performance_ , a new parameter you can calculate with any learning application supporting this specification. An example is the private one, kernel of SaaS part of [Alpha-Chess](http://www.alpha-chess.com) AI portal. The idea is to update it based on pattern recognition concept. In the portal, you can also exploit the reports of another NLG (virtual trainer) application and buy the products in the digishop based on all this. This open-source part has the performance default, based on score and depth. You can align the performance by uci token quickresetexp. Clearly, even if already strong, this private learning algorithm is a lot stronger as demostrate here: [Graphical result](https://github.com/amchess/BrainLearn/tree/master/tests/6-5.jpg) The perfomance, in this case, is updated based on the latest Stockfish wdl model (score and material).

This file is loaded in an hashtable at the engine load and updated each time the engine receive quit or stop uci command.
When BrainLearn starts a new game or when we have max 8 pieces on the chessboard, the learning is activated and the hash table updated each time the engine has a best score
at a depth >= 4 PLIES, according to Stockfish aspiration window.

At the engine loading, there is an automatic merge to experience.exp files, if we put the other ones, based on the following convention:

&lt;fileType&gt;&lt;qualityIndex&gt;.exp

where

- _fileType=experience_
- _qualityIndex_ , an integer, incrementally from 0 on based on the file&#39;s quality assigned by the user (0 best quality and so on)

N.B.

Because of disk access, to be effective, the learning must be made at no bullet time controls (less than 5 minutes/game).

### Read only learning

_Boolean, Default: False_ 
If activated, the learning file is only read.

### Experience Book

_Boolean, Default: False_ 
If activated, the engine will use the experience file as the book. In choosing the move to play, the engine will be based first on maximum win probability, then, on the engine's internal score, and finally, on depth. The UCI token “showexp” allows the book to display moves on a given position.

### Experience Book Max Moves

_Integer, Default: 100, Min: 1, Max: 100_
The maximum number of moves the engine chooses from the experience book

### Experience Book Min Depth

_Integer, Default: 4, Min: 1, Max: 255_
The min depth for the experience book

### Shashin section

_Default: no option settled_
The engine will determine dynamically the position's type starting from a "Capablanca/default
positions".
If one or more (mixed algorithms/positions types at the boundaries) of the seven following options
are settled, it will force the initial position/algorithm understanding
If, in the wdl model, we define wdl_w=Win percentage, wdl_d=Drawn percentage and Win probability=(2*wdl_w+wdl_d)/10, 
we have the following mapping:

| **WDL Range (W, D, L)**       | **Shashin Position’s Type**          | **Win Probability Range** | **Informator Symbols**| **Description**                                             |
|-------------------------------|--------------------------------------|---------------------------|-----------------------|-------------------------------------------------------------|
| [0, 3], [0, 4], [96, 100]    | High Petrosian                       | [0, 5]                  | -+                     | Winning: a decisive disadvantage, with the position clearly leading to victory.      |
| [4, 6], [5, 8], [89, 95]     | High-Middle Petrosian                | [6, 10]                 | -+ \ -/+               | Decisive disadvantage: dominant position and likely winning.                      |
| [7, 9], [9, 12], [80, 87]    | Middle Petrosian                     | [11, 15]                 | -/+                    | Clear disadvantage: a substantial positional advantage, but a win is not yet inevitable.                          |
| [10, 12], [13, 16], [73, 79]  | Middle-Low Petrosian                 | [16, 20]                 | -/+ \ =/+              | Significant disadvantage: strong edge                   |
| [13, 15], [17, 39], [66, 71]  | Low Petrosian                        | [21, 24]                 | =/+                    | Slight disadvantage with a positional edge, but no immediate threats.              |
| [0, 30], [40, 99], [31, 64]  | Chaos: Capablanca-Petrosian          | [25, 49]                 | ↓                      | Opponent pressure and initiative: defensive position.        |
| [0, 0], [100, 100], [0, 0]    | Capablanca                           | [50, 50]                 | =                      | Equal position. Both sides are evenly matched, with no evident advantage.           |
| [30, 64], [40, 99], [0, 30]  | Chaos: Capablanca-Tal                | [51, 75]                 | ↑                      | Initiative: playing dictation with active moves and forcing ideas.                     |
| [65, 71], [17, 39], [13, 15]   | Low Tal                              | [76, 79]                 | +/=                    | Slight advantage: a minor positional edge, but it’s not significant.                    |
| [72, 78], [13, 16], [10, 12]  | Middle-Low Tal                       | [80, 84]                 | +/= \ +/-              |  Slightly better, tending toward a clear advantage. The advantage is growing, but the position is still not decisive.            |
| [79, 87], [9, 12], [7, 9]    | Middle Tal                           | [85, 89]                 | +/-                    | Clear advantage: a significant edge, but still with defensive chances.                          |
| [88, 95], [5, 8], [4, 6]      | High-Middle Tal                      | [90, 94]                 | +/- \ +-               | Dominant position, almost decisive, not quite winning yet, but trending toward victory.                           |
| [96, 100], [0, 4], [0, 3]    | High Tal                             | [95, 100]                | +-                     | Winning: a decisive advantage, with victory nearly assured.   |
| In particular, [33, 33], [33, 33], [33, 33]                   | Chaos: Capablanca-Petrosian-Tal | (Unclassified)                   | ∞                      | Total chaos: unclear position, dynamically balanced, with no clear advantage for either side and no clear positional trends.                       |

N.B.
The wdl model also takes into account the history of the position at which a move has been calculated.
So, it's more effective than the cp. 

#### Tal

Attack position/algorithm

#### Capablanca

Strategical algorithm (for quiescent positions)

#### Petrosian

Defense position/algorithm (the "reversed colors" Tal)

## Acknowledgments

- Kozlov Sergey Aleksandrovitsch for his very interesting patch and code on Sugar engine
- Omar Khalid for his great experience in microsoft c/cpp programming environment
- Alexei Chernakoff for his pretious suggestions about the android version and its contribution to it
- Dariusz Domagala for the Mac version
- The BrainFish, McBrain, CorChess, CiChess and Crystal authors for their very interesting derivative
- Obviously, the chess theorician Alexander Shashin, whithout whom I wouldn't had the idea of this engine

Stockfish community

## Alexander team
- engine owner and main developer: ICCF IM Andrea Manzo (https://www.iccf.com/player?id=241224)
- IM Yohan Benitah for his professional chess understanding and help in testing against neural networks 
- official tester: ICCF CCE and CCM Maurizio Platino (https://www.iccf.com/player?id=241094)
- official tester: Maurizio Colbacchini, FSI 1N
- official tester and concept analyst: ICCF GM Fabio Finocchiaro (https://www.iccf.com/player?id=240090), 2012 ICCF world champion 
- official tester Dennis Marvin (NDL) (overall the online learning)
- tester and concept analyst: ICCF GM Matjas Pirs (https://www.iccf.com/player?id=480232), for his great experience and tests on positions analysis in different game's phases



Sorry If I forgot someone.

<h1 align="center">Stockfish</h1>

## Overview

[![Build Status](https://travis-ci.org/official-stockfish/Stockfish.svg?branch=master)](https://travis-ci.org/official-stockfish/Stockfish)
[![Build Status](https://ci.appveyor.com/api/projects/status/github/official-stockfish/Stockfish?branch=master&svg=true)](https://ci.appveyor.com/project/mcostalba/stockfish/branch/master)

[Stockfish](https://stockfishchess.org) is a free, powerful UCI chess engine
derived from Glaurung 2.1. Stockfish is not a complete chess program and requires a
UCI-compatible graphical user interface (GUI) (e.g. XBoard with PolyGlot, Scid,
Cute Chess, eboard, Arena, Sigma Chess, Shredder, Chess Partner or Fritz) in order
to be used comfortably. Read the documentation for your GUI of choice for information
about how to use Stockfish with it.

The Stockfish engine features the classical evaluation based on handcrafted terms. 
The classical evaluation runs efficiently on almost all CPU architectures.


## Files

This distribution of Stockfish consists of the following files:

  * Readme.md, the file you are currently reading.

  * Copying.txt, a text file containing the GNU General Public License version 3.

  * src, a subdirectory containing the full source code, including a Makefile
    that can be used to compile Stockfish on Unix-like systems.

## UCI options

Currently, Stockfish has the following UCI options:

  * #### Threads
    The number of CPU threads used for searching a position. For best performance, set
    this equal to the number of CPU cores available.

  * #### Hash
    The size of the hash table in MB. It is recommended to set Hash after setting Threads.

  * #### Ponder
    Let Stockfish ponder its next move while the opponent is thinking.

  * #### MultiPV
    Output the N best lines (principal variations, PVs) when searching.
    Leave at 1 for best performance.

  * #### UCI_AnalyseMode
    An option handled by your GUI.

  * #### UCI_Chess960
    An option handled by your GUI. If true, Stockfish will play Chess960.

  * #### UCI_ShowWDL
    If enabled, show approximate WDL statistics as part of the engine output.
    These WDL numbers model expected game outcomes for a given evaluation and
    game ply for engine self-play at fishtest LTC conditions (60+0.6s per game).

  * #### UCI_LimitStrength
    Enable weaker play aiming for an Elo rating as set by UCI_Elo. This option overrides Skill Level.

  * #### UCI_Elo
    If enabled by UCI_LimitStrength, aim for an engine strength of the given Elo.
    This Elo rating has been calibrated at a time control of 60s+0.6s and anchored to CCRL 40/4.

  * #### Skill Level
    Lower the Skill Level in order to make Stockfish play weaker (see also UCI_LimitStrength).
    Internally, MultiPV is enabled, and with a certain probability depending on the Skill Level a
    weaker move will be played.

  * #### SyzygyPath
    Path to the folders/directories storing the Syzygy tablebase files. Multiple
    directories are to be separated by ";" on Windows and by ":" on Unix-based
    operating systems. Do not use spaces around the ";" or ":".

    Example: `C:\tablebases\wdl345;C:\tablebases\wdl6;D:\tablebases\dtz345;D:\tablebases\dtz6`

    It is recommended to store .rtbw files on an SSD. There is no loss in storing
    the .rtbz files on a regular HD. It is recommended to verify all md5 checksums
    of the downloaded tablebase files (`md5sum -c checksum.md5`) as corruption will
    lead to engine crashes.

  * #### SyzygyProbeDepth
    Minimum remaining search depth for which a position is probed. Set this option
    to a higher value to probe less agressively if you experience too much slowdown
    (in terms of nps) due to TB probing.

  * #### Syzygy50MoveRule
    Disable to let fifty-move rule draws detected by Syzygy tablebase probes count
    as wins or losses. This is useful for ICCF correspondence games.

  * #### SyzygyProbeLimit
    Limit Syzygy tablebase probing to positions with at most this many pieces left
    (including kings and pawns).

  * #### Move Overhead
    Assume a time delay of x ms due to network and GUI overheads. This is useful to
    avoid losses on time in those cases.

  * #### Slow Mover
    Lower values will make Stockfish take less time in games, higher values will
    make it think longer.

  * ####  Minimum Thinking Time (old Stockfish option restored)
	Search for at least x ms per move.
	
  * #### nodestime
    Tells the engine to use nodes searched instead of wall time to account for
    elapsed time. Useful for engine testing.

  * #### Clear Hash
    Clear the hash table.

  * #### Debug Log File
    Write all communication to and from the engine into a text file.

## A note on classical evaluation

It assigns a value to a position that is used in alpha-beta (PVS) search
to find the best move. The classical evaluation computes this value as a function
of various chess concepts, handcrafted by experts, tested and tuned using fishtest.

# Trace (eval command) infos

## Metrics Table

| Section | Element / Sub-Element | Utility for Beginner (The "What") | Utility for Intermediate (The "Why") | Utility for Advanced (The "How") | Utility for Expert (The "How and Beyond") |
|---------|----------------------|-----------------------------------|--------------------------------------|----------------------------------|------------------------------------------|
| **1. General Information** | | | | | |
| | Final evaluation | Look at the score: + is advantage for White, - for Black. The percentage tells you who is more likely to win. | Understand the magnitude of the advantage. A score of +1.00 is like having an extra pawn. Use the percentage to understand how difficult it is to convert the advantage. | Analyze the stability of the evaluation. If it changes a lot, the position is tactical and uncertain. If stable, the advantage is strategic and solid. | Relate the evaluation to the engine's WDL (Win-Draw-Loss) model to understand conversion probabilities with perfect play. |
| | Shashin Zone | Tells you what "type" of position it is: Tal (attack), Petrosian (defense), Capablanca (strategy/endgame). | Use the classification to orient your general plan. If it's "Tal", look for tactical opportunities. If "Petrosian", improve safety. | Adapt move choices to the style required by the zone. In a "Capablanca" position, prioritize long-term plans and structures. | Integrate Shashin classification with other metrics (e.g., Density, Center) for holistic understanding and to anticipate phase transitions. |
| | Game Phase | Indicates if you're in Opening, Middlegame, or Endgame. | Understand how piece values and plans change. King safety is vital in middlegame, king activity is crucial in endgame. | Use game phase to interpret the terms table scores (MG vs EG) and understand which advantages are long-term. | Analyze the numerical phase value to predict the exact transition point and prepare the position for the next phase. |
| **2. Summary Table** | | | | | |
| | Terms Table (MG/EG/Total) | Look at the "Total" column to see who has advantage and in which category (e.g., "White wins due to Mobility"). | Compare MG (middlegame) and EG (endgame) values. A "Pawns" advantage might be more important in EG, while "Threats" is crucial in MG. | Analyze individual values to identify the precise source of advantage/disadvantage and build a plan to accentuate or neutralize it. | Look for significant discrepancies between terms. High "Mobility" but low "King Safety" indicates dynamic but risky advantage. |
| **3. Material Analysis** | | | | | |
| | Bishop Pair | Tells you if you have the "bishop pair", which is usually an advantage. | Understand that the bishop pair is strong in open positions. Try to open the game if you have it, or keep it closed if your opponent has it. | Quantify the bishop pair advantage from the summary table and plan how to use it to control key diagonals. | Evaluate the strength of the pair relative to pawn structure. If pawns are blocked on the wrong color, the advantage may be null or negative. |
| | Material Imbalances | Shows non-standard exchanges (e.g., Rook vs. Bishop+Knight) and tells you who benefits. | Learn the characteristics of common imbalances. A Queen is often stronger than two Rooks in complex positions, but weaker in simple endgames. | Base your strategy on the imbalance. If you have minor pieces against a Rook, try to create outposts and exploit their flexibility. | Analyze the imbalance based on piece coordination. The numerical value is only a guide; the pieces' ability to work together is decisive. |
| **4. Pawn Structure** | | | | | |
| | Islands, doubled, isolated, etc. | Identify weaknesses in your pawns and your opponent's. Fewer weaknesses are better. | Understand that every weakness is a potential target. An isolated pawn can be attacked, a backward pawn can be blocked. | Formulate a long-term plan based on weaknesses. If opponent has an isolated pawn, concentrate your pieces to attack it. | Evaluate weaknesses dynamically. A doubled pawn can be a weakness, but can also control important squares or open a file. |
| | Weak squares | Indicates squares in your position not defended by pawns that the opponent could use. | Understand that a weak square is a "hole" in your position, an invitation for enemy pieces to settle there (outpost). | Develop a plan to control opponent's weak squares with your pieces, or to cover yours with defensive maneuvers. | Use the concept of "weak square complex" (e.g., all light squares) to strategically dominate the opponent. |
| **5. Passed Pawns** | | | | | |
| | Position and classification | Tells you if you have a "passed pawn", a pawn that cannot be stopped by opponent's pawns. It's a very strong threat. | Distinguish between a strong passed pawn (supported) and a weak one (blocked, easy to attack). Understand the importance of the promotion "race". | Evaluate the real threat of the pawn based on its distance from promotion and opponent king's control of the promotion square. | Use the passed pawn not only as a promotion threat, but also to divert opponent's pieces and create weaknesses elsewhere on the board. |
| **6. Line Control** | | | | | |
| | Open files and diagonals | Suggests where to place your heavy pieces (Rooks, Queen) and Bishops to be more effective. | Apply the principle: occupy open files with Rooks to penetrate enemy territory. Use open diagonals for Bishops. | Coordinate your pieces to simultaneously control multiple lines, creating unsustainable pressure and multiple threats. | Evaluate line control relative to entry points. An open file is useless if all entry squares are controlled by the opponent. |
| **7. Dvoretsky's Rule** | | | | | |
| | Opposite-colored bishop endgames | Warns you that you're in an opposite-colored bishop endgame, which often ends in a draw even with an extra pawn. | Understand the drawing tendencies, but also attacking possibilities if other pieces remain. The side with initiative can create threats the other bishop cannot parry. | Apply Dvoretsky's rules: if you're winning, create a passed pawn; if you're losing, build a fortress and block opponent's pawns. | Analyze pawn positions relative to bishop colors. Pawns blocked on the same color as your bishop are weak and limit the piece. |
| **8. Packing Density** | | | | | |
| | Density calculation and Shashin analysis | Indicates if the position is "dense" (crowded) or "sparse" (open). | In dense positions, Knights are often superior to Bishops. In sparse positions, Bishops and Rooks dominate. | Use density to decide which pieces to exchange. In a dense position, try to trade your opponent's "bad" bishop. | Link density to Shashin Zone. A "dense" and "Tal" position suggests an attack with sacrifices to open lines. |
| **9. Center Analysis** | | | | | |
| | Center type | Tells you if the center is Open, Closed, Static, or Dynamic. This determines the general plan. | Learn typical plans: in closed centers, attack on the wings. In open centers, piece play and king safety are fundamental. | Choose moves that fit the center type. In a dynamic center, priority is initiative and precise calculation of variations. | Plan center transformation. Try to open a closed center to your advantage or block a dynamic center if under pressure. |
| **10. Makogonov Ranking** | | | | | |
| | Unit ranking and Piece to improve | Shows your "worst" or least active piece. Try to find it a better square. | Apply the principle of "improving the worst-placed piece". Often, activating a passive piece improves the entire position. | Use this ranking as a concrete guide for your next 2-3 moves, focusing on activating the identified piece. | Consider the ranking prophylactically: identify opponent's worst piece and plan how to prevent its improvement. |
| **11. Mobility by Areas** | | | | | |
| | Distribution and Kasparov's Principle | Shows which part of the board your pieces have more freedom of movement (queenside, center, kingside). | Apply Kasparov's principle: attack where you have more mobility. If your pieces are more active on the kingside, that's where you should create threats. | Use mobility analysis to decide where to make pawn breaks, opening lines for your most active pieces. | Analyze the "delta" of mobility. A slight advantage is not enough; look for areas with strong mobility imbalance to launch a decisive attack. |
| **12. Space Analysis** | | | | | |
| | Space per area and Area control | Tells you if you have more "space" than your opponent. Having more space is an advantage because your pieces move more easily. | Understand that a space advantage limits opponent's options. Try to "suffocate" their pieces and prevent coordination. | Transform space advantage into an attack. Use extra space to reorganize your pieces and create pressure points on the enemy's weak side. | Manage the risk of overextension. A large space advantage can also create weaknesses that an experienced opponent can exploit with counterplay. |
| **13. Expansion Factor** | | | | | |
| | Center of gravity and Expansion delta | Measures how far your pieces have advanced into enemy territory. The more advanced, the more aggressive you are. | Link expansion to initiative. Whoever has the more advanced "center of gravity" usually dictates the game's pace. | Use this metric in "Capablanca" (strategic) positions to evaluate who is making territorial progress. If behind, consolidate; if ahead, keep pushing. | Evaluate the stability of your center of gravity. Overly advanced pieces without adequate support can become targets and turn into weaknesses. |
| **14. Ordered Legal Moves** | | | | | |
| | Complete list and Activity score | The first move in the list is what the engine considers best. | Look at the first 2-3 moves. If their scores are very similar, you have more than one good option. If one move is clearly superior, it's almost certainly best. | Examine the entire list to find interesting "candidate" moves the engine doesn't rank first but might better fit your style or a long-term plan. | Analyze move ordering to understand engine heuristics. A tactical move (capture, check) will always be on top, but a quiet move with a good score is strategically very significant. |

## What to expect from Syzygybases?

If the engine is searching a position that is not in the tablebases (e.g.
a position with 8 pieces), it will access the tablebases during the search.
If the engine reports a very large score (typically 153.xx), this means
that it has found a winning line into a tablebase position.

If the engine is given a position to search that is in the tablebases, it
will use the tablebases at the beginning of the search to preselect all
good moves, i.e. all moves that preserve the win or preserve the draw while
taking into account the 50-move rule.
It will then perform a search only on those moves. **The engine will not move
immediately**, unless there is only a single good move. **The engine likely
will not report a mate score even if the position is known to be won.**

It is therefore clear that this behaviour is not identical to what one might
be used to with Nalimov tablebases. There are technical reasons for this
difference, the main technical reason being that Nalimov tablebases use the
DTM metric (distance-to-mate), while Syzygybases use a variation of the
DTZ metric (distance-to-zero, zero meaning any move that resets the 50-move
counter). This special metric is one of the reasons that Syzygybases are
more compact than Nalimov tablebases, while still storing all information
needed for optimal play and in addition being able to take into account
the 50-move rule.

## Large Pages

Stockfish supports large pages on Linux and Windows. Large pages make
the hash access more efficient, improving the engine speed, especially
on large hash sizes. Typical increases are 5..10% in terms of nodes per
second, but speed increases up to 30% have been measured. The support is
automatic. Stockfish attempts to use large pages when available and
will fall back to regular memory allocation when this is not the case.

### Support on Linux

Large page support on Linux is obtained by the Linux kernel
transparent huge pages functionality. Typically, transparent huge pages
are already enabled and no configuration is needed.

### Support on Windows

The use of large pages requires "Lock Pages in Memory" privilege. See
[Enable the Lock Pages in Memory Option (Windows)](https://docs.microsoft.com/en-us/sql/database-engine/configure-windows/enable-the-lock-pages-in-memory-option-windows)
on how to enable this privilege, then run [RAMMap](https://docs.microsoft.com/en-us/sysinternals/downloads/rammap)
to double-check that large pages are used. We suggest that you reboot
your computer after you have enabled large pages, because long Windows
sessions suffer from memory fragmentation which may prevent Stockfish
from getting large pages: a fresh session is better in this regard.

## Compiling Stockfish yourself from the sources

Stockfish has support for 32 or 64-bit CPUs, certain hardware
instructions, big-endian machines such as Power PC, and other platforms.

On Unix-like systems, it should be easy to compile Stockfish
directly from the source code with the included Makefile in the folder
`src`. In general it is recommended to run `make help` to see a list of make
targets with corresponding descriptions.

```
    cd src
    make help
    make build ARCH=x86-64-modern
```

When not using the Makefile to compile (for instance with Microsoft MSVC) you
need to manually set/unset some switches in the compiler command line; see
file *types.h* for a quick reference.

When reporting an issue or a bug, please tell us which version and
compiler you used to create your executable. These informations can
be found by typing the following commands in a console:

```
    ./stockfish compiler
```

## Understanding the code base and participating in the project

Stockfish's improvement over the last couple of years has been a great
community effort. There are a few ways to help contribute to its growth.

### Donating hardware

Improving Stockfish requires a massive amount of testing. You can donate
your hardware resources by installing the [Fishtest Worker](https://github.com/glinscott/fishtest/wiki/Running-the-worker:-overview)
and view the current tests on [Fishtest](https://tests.stockfishchess.org/tests).

### Improving the code

If you want to help improve the code, there are several valuable resources:

* [In this wiki,](https://www.chessprogramming.org) many techniques used in
Stockfish are explained with a lot of background information.

* [The section on Stockfish](https://www.chessprogramming.org/Stockfish)
describes many features and techniques used by Stockfish. However, it is
generic rather than being focused on Stockfish's precise implementation.
Nevertheless, a helpful resource.

* The latest source can always be found on [GitHub](https://github.com/official-stockfish/Stockfish).
Discussions about Stockfish take place in the [FishCooking](https://groups.google.com/forum/#!forum/fishcooking)
group and engine testing is done on [Fishtest](https://tests.stockfishchess.org/tests).
If you want to help improve Stockfish, please read this [guideline](https://github.com/glinscott/fishtest/wiki/Creating-my-first-test)
first, where the basics of Stockfish development are explained.


## Terms of use

Alexander is free and distributed under the
[**GNU General Public License version 3**][license-link] (GPL v3). Essentially,
this means you are free to do almost exactly what you want with the program,
including distributing it among your friends, making it available for download
from your website, selling it (either by itself or as part of some bigger
software package), or using it as the starting point for a software project of
your own.

The only real limitation is that whenever you distribute Brainlearn in some way,
you MUST always include the license and the full source code (or a pointer to
where the source code can be found) to generate the exact binary you are
distributing. If you make any changes to the source code, these changes must
also be made available under GPL v3.

## Acknowledgements

Stockfish uses neural networks trained on [data provided by the Leela Chess Zero
project][lc0-data-link], which is made available under the [Open Database License][odbl-link] (ODbL).


[authors-link]:       https://github.com/official-stockfish/Stockfish/blob/master/AUTHORS
[build-link]:         https://github.com/official-stockfish/Stockfish/actions/workflows/stockfish.yml
[commits-link]:       https://github.com/official-stockfish/Stockfish/commits/master
[discord-link]:       https://discord.gg/GWDRS3kU6R
[issue-link]:         https://github.com/official-stockfish/Stockfish/issues/new?assignees=&labels=&template=BUG-REPORT.yml
[discussions-link]:   https://github.com/official-stockfish/Stockfish/discussions/new
[fishtest-link]:      https://tests.stockfishchess.org/tests
[guideline-link]:     https://github.com/official-stockfish/fishtest/wiki/Creating-my-first-test
[license-link]:       https://github.com/official-stockfish/Stockfish/blob/master/Copying.txt
[programming-link]:   https://www.chessprogramming.org/Main_Page
[programmingsf-link]: https://www.chessprogramming.org/Stockfish
[readme-link]:        https://github.com/official-stockfish/Stockfish/blob/master/README.md
[release-link]:       https://github.com/official-stockfish/Stockfish/releases/latest
[src-link]:           https://github.com/official-stockfish/Stockfish/tree/master/src
[stockfish128-logo]:  https://stockfishchess.org/images/logo/icon_128x128.png
[uci-link]:           https://backscattering.de/chess/uci/
[website-link]:       https://stockfishchess.org
[website-blog-link]:  https://stockfishchess.org/blog/
[wiki-link]:          https://github.com/official-stockfish/Stockfish/wiki
[wiki-compile-link]:  https://github.com/official-stockfish/Stockfish/wiki/Compiling-from-source
[wiki-uci-link]:      https://github.com/official-stockfish/Stockfish/wiki/UCI-&-Commands
[wiki-usage-link]:    https://github.com/official-stockfish/Stockfish/wiki/Download-and-usage
[worker-link]:        https://github.com/official-stockfish/fishtest/wiki/Running-the-worker
[lc0-data-link]:      https://storage.lczero.org/files/training_data
[odbl-link]:          https://opendatacommons.org/licenses/odbl/odbl-10.txt

[build-badge]:        https://img.shields.io/github/actions/workflow/status/official-stockfish/Stockfish/stockfish.yml?branch=master&style=for-the-badge&label=stockfish&logo=github
[commits-badge]:      https://img.shields.io/github/commits-since/official-stockfish/Stockfish/latest?style=for-the-badge
[discord-badge]:      https://img.shields.io/discord/435943710472011776?style=for-the-badge&label=discord&logo=Discord
[fishtest-badge]:     https://img.shields.io/website?style=for-the-badge&down_color=red&down_message=Offline&label=Fishtest&up_color=success&up_message=Online&url=https%3A%2F%2Ftests.stockfishchess.org%2Ftests%2Ffinished
[license-badge]:      https://img.shields.io/github/license/official-stockfish/Stockfish?style=for-the-badge&label=license&color=success
[release-badge]:      https://img.shields.io/github/v/release/official-stockfish/Stockfish?style=for-the-badge&label=official%20release
[website-badge]:      https://img.shields.io/website?style=for-the-badge&down_color=red&down_message=Offline&label=website&up_color=success&up_message=Online&url=https%3A%2F%2Fstockfishchess.org
