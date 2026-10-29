// ============================================================================
// FILE ORIGINALE: benchmark.cpp
// ============================================================================

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

#include "benchmark.h"
#include "numa.h"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <vector>

namespace {

// clang-format off
const std::vector<std::string> Defaults = {
  "setoption name UCI_Chess960 value false",
  "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
  "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 10",
  "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 11",
  "4rrk1/pp1n3p/3q2pQ/2p1pb2/2PP4/2P3N1/P2B2PP/4RRK1 b - - 7 19",
  "rq3rk1/ppp2ppp/1bnpb3/3N2B1/3NP3/7P/PPPQ1PP1/2KR3R w - - 7 14 moves d4e6",
  "r1bq1r1k/1pp1n1pp/1p1p4/4p2Q/4Pp2/1BNP4/PPP2PPP/3R1RK1 w - - 2 14 moves g2g4",
  "r3r1k1/2p2ppp/p1p1bn2/8/1q2P3/2NPQN2/PPP3PP/R4RK1 b - - 2 15",
  "r1bbk1nr/pp3p1p/2n5/1N4p1/2Np1B2/8/PPP2PPP/2KR1B1R w kq - 0 13",
  "r1bq1rk1/ppp1nppp/4n3/3p3Q/3P4/1BP1B3/PP1N2PP/R4RK1 w - - 1 16",
  "4r1k1/r1q2ppp/ppp2n2/4P3/5Rb1/1N1BQ3/PPP3PP/R5K1 w - - 1 17",
  "2rqkb1r/ppp2p2/2npb1p1/1N1Nn2p/2P1PP2/8/PP2B1PP/R1BQK2R b KQ - 0 11",
  "r1bq1r1k/b1p1npp1/p2p3p/1p6/3PP3/1B2NN2/PP3PPP/R2Q1RK1 w - - 1 16",
  "3r1rk1/p5pp/bpp1pp2/8/q1PP1P2/b3P3/P2NQRPP/1R2B1K1 b - - 6 22",
  "r1q2rk1/2p1bppp/2Pp4/p6b/Q1PNp3/4B3/PP1R1PPP/2K4R w - - 2 18",
  "4k2r/1pb2ppp/1p2p3/1R1p4/3P4/2r1PN2/P4PPP/1R4K1 b - - 3 22",
  "3q2k1/pb3p1p/4pbp1/2r5/PpN2N2/1P2P2P/5PP1/Q2R2K1 b - - 4 26",
  "6k1/6p1/6Pp/ppp5/3pn2P/1P3K2/1PP2P2/3N4 b - - 0 1",
  "3b4/5kp1/1p1p1p1p/pP1PpP1P/P1P1P3/3KN3/8/8 w - - 0 1",
  "2K5/p7/7P/5pR1/8/5k2/r7/8 w - - 0 1 moves g5g6 f3e3 g6g5 e3f3",
  "8/6pk/1p6/8/PP3p1p/5P2/4KP1q/3Q4 w - - 0 1",
  "7k/3p2pp/4q3/8/4Q3/5Kp1/P6b/8 w - - 0 1",
  "8/2p5/8/2kPKp1p/2p4P/2P5/3P4/8 w - - 0 1",
  "8/1p3pp1/7p/5P1P/2k3P1/8/2K2P2/8 w - - 0 1",
  "8/pp2r1k1/2p1p3/3pP2p/1P1P1P1P/P5KR/8/8 w - - 0 1",
  "8/3p4/p1bk3p/Pp6/1Kp1PpPp/2P2P1P/2P5/5B2 b - - 0 1",
  "5k2/7R/4P2p/5K2/p1r2P1p/8/8/8 b - - 0 1",
  "6k1/6p1/P6p/r1N5/5p2/7P/1b3PP1/4R1K1 w - - 0 1",
  "1r3k2/4q3/2Pp3b/3Bp3/2Q2p2/1p1P2P1/1P2KP2/3N4 w - - 0 1",
  "6k1/4pp1p/3p2p1/P1pPb3/R7/1r2P1PP/3B1P2/6K1 w - - 0 1",
  "8/3p3B/5p2/5P2/p7/PP5b/k7/6K1 w - - 0 1",
  "5rk1/q6p/2p3bR/1pPp1rP1/1P1Pp3/P3B1Q1/1K3P2/R7 w - - 93 90",
  "4rrk1/1p1nq3/p7/2p1P1pp/3P2bp/3Q1Bn1/PPPB4/1K2R1NR w - - 40 21",
  "r3k2r/3nnpbp/q2pp1p1/p7/Pp1PPPP1/4BNN1/1P5P/R2Q1RK1 w kq - 0 16",
  "3Qb1k1/1r2ppb1/pN1n2q1/Pp1Pp1Pr/4P2p/4BP2/4B1R1/1R5K b - - 11 40",
  "4k3/3q1r2/1N2r1b1/3ppN2/2nPP3/1B1R2n1/2R1Q3/3K4 w - - 5 1",

  // Positions with high numbers of changed threats
  "k7/2n1n3/1nbNbn2/2NbRBn1/1nbRQR2/2NBRBN1/3N1N2/7K w - - 0 1",
  "K7/8/8/BNQNQNB1/N5N1/R1Q1q2r/n5n1/bnqnqnbk w - - 0 1",

  // 5-man positions
  "8/8/8/8/5kp1/P7/8/1K1N4 w - - 0 1",     // Kc2 - mate
  "8/8/8/5N2/8/p7/8/2NK3k w - - 0 1",      // Na2 - mate
  "8/3k4/8/8/8/4B3/4KB2/2B5 w - - 0 1",    // draw

  // 6-man positions
  "8/8/1P6/5pr1/8/4R3/7k/2K5 w - - 0 1",   // Re5 - mate
  "8/2p4P/8/kr6/6R1/8/8/1K6 w - - 0 1",    // Ka2 - mate
  "8/8/3P3k/8/1p6/8/1P6/1K3n2 b - - 0 1",  // Nd2 - draw

  // 7-man positions
  "8/R7/2q5/8/6k1/8/1P5p/K6R w - - 0 124", // Draw

  // Mate and stalemate positions
  "6k1/3b3r/1p1p4/p1n2p2/1PPNpP1q/P3Q1p1/1R1RB1P1/5K2 b - - 0 1",
  "r2r1n2/pp2bk2/2p1p2p/3q4/3PN1QP/2P3R1/P4PP1/5RK1 w - - 0 1",
  "8/8/8/8/8/6k1/6p1/6K1 w - -",
  "7k/7P/6K1/8/3B4/8/8/8 b - -",

  // Chess 960
  "setoption name UCI_Chess960 value true",
  "bbqnnrkr/pppppppp/8/8/8/8/PPPPPPPP/BBQNNRKR w HFhf - 0 1 moves g2g3 d7d5 d2d4 c8h3 c1g5 e8d6 g5e7 f7f6",
  "nqbnrkrb/pppppppp/8/8/8/8/PPPPPPPP/NQBNRKRB w KQkq - 0 1",
  "setoption name UCI_Chess960 value false"
};
// clang-format on

// clang-format off
// human-randomly picked 5 games with <60 moves from
// https://tests.stockfishchess.org/tests/view/665c71f9fd45fb0f907c21e0
// only moves for one side
const std::vector<std::vector<std::string>> BenchmarkPositions = {
    {
        "rnbq1k1r/ppp1bppp/4pn2/8/2B5/2NP1N2/PPP2PPP/R1BQR1K1 b - - 2 8",
        "rnbq1k1r/pp2bppp/4pn2/2p5/2B2B2/2NP1N2/PPP2PPP/R2QR1K1 b - - 1 9",
        "r1bq1k1r/pp2bppp/2n1pn2/2p5/2B1NB2/3P1N2/PPP2PPP/R2QR1K1 b - - 3 10",
        "r1bq1k1r/pp2bppp/2n1p3/2p5/2B1PB2/5N2/PPP2PPP/R2QR1K1 b - - 0 11",
        "r1b2k1r/pp2bppp/2n1p3/2p5/2B1PB2/5N2/PPP2PPP/3RR1K1 b - - 0 12",
        "r1b1k2r/pp2bppp/2n1p3/2p5/2B1PB2/2P2N2/PP3PPP/3RR1K1 b - - 0 13",
        "r1b1k2r/1p2bppp/p1n1p3/2p5/4PB2/2P2N2/PP2BPPP/3RR1K1 b - - 1 14",
        "r1b1k2r/4bppp/p1n1p3/1pp5/P3PB2/2P2N2/1P2BPPP/3RR1K1 b - - 0 15",
        "r1b1k2r/4bppp/p1n1p3/1P6/2p1PB2/2P2N2/1P2BPPP/3RR1K1 b - - 0 16",
        "r1b1k2r/4bppp/2n1p3/1p6/2p1PB2/1PP2N2/4BPPP/3RR1K1 b - - 0 17",
        "r3k2r/3bbppp/2n1p3/1p6/2P1PB2/2P2N2/4BPPP/3RR1K1 b - - 0 18",
        "r3k2r/3bbppp/2n1p3/8/1pP1P3/2P2N2/3BBPPP/3RR1K1 b - - 1 19",
        "1r2k2r/3bbppp/2n1p3/8/1pPNP3/2P5/3BBPPP/3RR1K1 b - - 3 20",
        "1r2k2r/3bbppp/2n1p3/8/2PNP3/2B5/4BPPP/3RR1K1 b - - 0 21",
        "1r2k2r/3bb1pp/2n1pp2/1N6/2P1P3/2B5/4BPPP/3RR1K1 b - - 1 22",
        "1r2k2r/3b2pp/2n1pp2/1N6/1BP1P3/8/4BPPP/3RR1K1 b - - 0 23",
        "1r2k2r/3b2pp/4pp2/1N6/1nP1P3/8/3RBPPP/4R1K1 b - - 1 24",
        "1r5r/3bk1pp/4pp2/1N6/1nP1PP2/8/3RB1PP/4R1K1 b - - 0 25",
        "1r5r/3bk1pp/2n1pp2/1N6/2P1PP2/8/3RBKPP/4R3 b - - 2 26",
        "1r5r/3bk1pp/2n2p2/1N2p3/2P1PP2/6P1/3RBK1P/4R3 b - - 0 27",
        "1r1r4/3bk1pp/2n2p2/1N2p3/2P1PP2/6P1/3RBK1P/R7 b - - 2 28",
        "1r1r4/N3k1pp/2n1bp2/4p3/2P1PP2/6P1/3RBK1P/R7 b - - 4 29",
        "1r1r4/3bk1pp/2N2p2/4p3/2P1PP2/6P1/3RBK1P/R7 b - - 0 30",
        "1r1R4/4k1pp/2b2p2/4p3/2P1PP2/6P1/4BK1P/R7 b - - 0 31",
        "3r4/4k1pp/2b2p2/4P3/2P1P3/6P1/4BK1P/R7 b - - 0 32",
        "3r4/R3k1pp/2b5/4p3/2P1P3/6P1/4BK1P/8 b - - 1 33",
        "8/3rk1pp/2b5/R3p3/2P1P3/6P1/4BK1P/8 b - - 3 34",
        "8/3r2pp/2bk4/R1P1p3/4P3/6P1/4BK1P/8 b - - 0 35",
        "8/2kr2pp/2b5/R1P1p3/4P3/4K1P1/4B2P/8 b - - 2 36",
        "1k6/3r2pp/2b5/RBP1p3/4P3/4K1P1/7P/8 b - - 4 37",
        "8/1k1r2pp/2b5/R1P1p3/4P3/3BK1P1/7P/8 b - - 6 38",
        "1k6/3r2pp/2b5/2P1p3/4P3/3BK1P1/7P/R7 b - - 8 39",
        "1k6/r5pp/2b5/2P1p3/4P3/3BK1P1/7P/5R2 b - - 10 40",
        "1k3R2/6pp/2b5/2P1p3/4P3/r2BK1P1/7P/8 b - - 12 41",
        "5R2/2k3pp/2b5/2P1p3/4P3/r2B2P1/3K3P/8 b - - 14 42",
        "5R2/2k3pp/2b5/2P1p3/4P3/3BK1P1/r6P/8 b - - 16 43",
        "5R2/2k3pp/2b5/2P1p3/4P3/r2B2P1/4K2P/8 b - - 18 44",
        "5R2/2k3pp/2b5/2P1p3/4P3/3B1KP1/r6P/8 b - - 20 45",
        "8/2k2Rpp/2b5/2P1p3/4P3/r2B1KP1/7P/8 b - - 22 46",
        "3k4/5Rpp/2b5/2P1p3/4P3/r2B2P1/4K2P/8 b - - 24 47",
        "3k4/5Rpp/2b5/2P1p3/4P3/3B1KP1/r6P/8 b - - 26 48",
        "3k4/5Rpp/2b5/2P1p3/4P3/r2B2P1/4K2P/8 b - - 28 49",
        "3k4/5Rpp/2b5/2P1p3/4P3/3BK1P1/r6P/8 b - - 30 50",
        "3k4/5Rpp/2b5/2P1p3/4P3/r2B2P1/3K3P/8 b - - 32 51",
        "3k4/5Rpp/2b5/2P1p3/4P3/2KB2P1/r6P/8 b - - 34 52",
        "3k4/5Rpp/2b5/2P1p3/4P3/r2B2P1/2K4P/8 b - - 36 53",
        "3k4/5Rpp/2b5/2P1p3/4P3/1K1B2P1/r6P/8 b - - 38 54",
        "3k4/6Rp/2b5/2P1p3/4P3/1K1B2P1/7r/8 b - - 0 55",
        "3k4/8/2b3Rp/2P1p3/4P3/1K1B2P1/7r/8 b - - 1 56",
        "8/2k3R1/2b4p/2P1p3/4P3/1K1B2P1/7r/8 b - - 3 57",
        "3k4/8/2b3Rp/2P1p3/4P3/1K1B2P1/7r/8 b - - 5 58",
        "8/2k5/2b3Rp/2P1p3/1K2P3/3B2P1/7r/8 b - - 7 59",
        "8/2k5/2b3Rp/2P1p3/4P3/2KB2P1/3r4/8 b - - 9 60",
        "8/2k5/2b3Rp/2P1p3/1K2P3/3B2P1/6r1/8 b - - 11 61",
        "8/2k5/2b3Rp/2P1p3/4P3/2KB2P1/3r4/8 b - - 13 62",
        "8/2k5/2b3Rp/2P1p3/2K1P3/3B2P1/6r1/8 b - - 15 63",
        "4b3/2k3R1/7p/2P1p3/2K1P3/3B2P1/6r1/8 b - - 17 64",
    },
    {
        "r1bqkbnr/npp1pppp/p7/3P4/4pB2/2N5/PPP2PPP/R2QKBNR w KQkq - 1 6",
        "r1bqkb1r/npp1pppp/p4n2/3P4/4pB2/2N5/PPP1QPPP/R3KBNR w KQkq - 3 7",
        "r2qkb1r/npp1pppp/p4n2/3P1b2/4pB2/2N5/PPP1QPPP/2KR1BNR w kq - 5 8",
        "r2qkb1r/1pp1pppp/p4n2/1n1P1b2/4pB2/2N4P/PPP1QPP1/2KR1BNR w kq - 1 9",
        "r2qkb1r/1pp1pppp/5n2/1p1P1b2/4pB2/7P/PPP1QPP1/2KR1BNR w kq - 0 10",
        "r2qkb1r/1ppbpppp/5n2/1Q1P4/4pB2/7P/PPP2PP1/2KR1BNR w kq - 1 11",
        "3qkb1r/1Qpbpppp/5n2/3P4/4pB2/7P/rPP2PP1/2KR1BNR w k - 0 12",
        "q3kb1r/1Qpbpppp/5n2/3P4/4pB2/7P/rPP2PP1/1K1R1BNR w k - 2 13",
        "r3kb1r/2pbpppp/5n2/3P4/4pB2/7P/1PP2PP1/1K1R1BNR w k - 0 14",
        "r3kb1r/2Bb1ppp/4pn2/3P4/4p3/7P/1PP2PP1/1K1R1BNR w k - 0 15",
        "r3kb1r/2Bb2pp/4pn2/8/4p3/7P/1PP2PP1/1K1R1BNR w k - 0 16",
        "r3k2r/2Bb2pp/4pn2/2b5/4p3/7P/1PP1NPP1/1K1R1B1R w k - 2 17",
        "r6r/2Bbk1pp/4pn2/2b5/3Np3/7P/1PP2PP1/1K1R1B1R w - - 4 18",
        "r6r/b2bk1pp/4pn2/4B3/3Np3/7P/1PP2PP1/1K1R1B1R w - - 6 19",
        "r1r5/b2bk1pp/4pn2/4B3/2BNp3/7P/1PP2PP1/1K1R3R w - - 8 20",
        "r7/b2bk1pp/4pn2/2r1B3/2BNp3/1P5P/2P2PP1/1K1R3R w - - 1 21",
        "rb6/3bk1pp/4pn2/2r1B3/2BNpP2/1P5P/2P3P1/1K1R3R w - - 1 22",
        "1r6/3bk1pp/4pn2/2r5/2BNpP2/1P5P/2P3P1/1K1R3R w - - 0 23",
        "1r6/3bk1p1/4pn1p/2r5/2BNpP2/1P5P/2P3P1/2KR3R w - - 0 24",
        "8/3bk1p1/1r2pn1p/2r5/2BNpP1P/1P6/2P3P1/2KR3R w - - 1 25",
        "8/3bk3/1r2pnpp/2r5/2BNpP1P/1P6/2P3P1/2K1R2R w - - 0 26",
        "2b5/4k3/1r2pnpp/2r5/2BNpP1P/1P4P1/2P5/2K1R2R w - - 1 27",
        "8/1b2k3/1r2pnpp/2r5/2BNpP1P/1P4P1/2P5/2K1R1R1 w - - 3 28",
        "8/1b1nk3/1r2p1pp/2r5/2BNpPPP/1P6/2P5/2K1R1R1 w - - 1 29",
        "8/1b2k3/1r2p1pp/2r1nP2/2BNp1PP/1P6/2P5/2K1R1R1 w - - 1 30",
        "8/1b2k3/1r2p1p1/2r1nPp1/2BNp2P/1P6/2P5/2K1R1R1 w - - 0 31",
        "8/1b2k3/1r2p1n1/2r3p1/2BNp2P/1P6/2P5/2K1R1R1 w - - 0 32",
        "8/1b2k3/1r2p1n1/6r1/2BNp2P/1P6/2P5/2K1R3 w - - 0 33",
        "8/1b2k3/1r2p3/4n1P1/2BNp3/1P6/2P5/2K1R3 w - - 1 34",
        "8/1b2k3/1r2p3/4n1P1/2BN4/1P2p3/2P5/2K4R w - - 0 35",
        "8/1b2k3/1r2p2R/6P1/2nN4/1P2p3/2P5/2K5 w - - 0 36",
        "8/1b2k3/3rp2R/6P1/2PN4/4p3/2P5/2K5 w - - 1 37",
        "8/4k3/3rp2R/6P1/2PN4/2P1p3/6b1/2K5 w - - 1 38",
        "8/4k3/r3p2R/2P3P1/3N4/2P1p3/6b1/2K5 w - - 1 39",
        "8/3k4/r3p2R/2P2NP1/8/2P1p3/6b1/2K5 w - - 3 40",
        "8/3k4/4p2R/2P3P1/8/2P1N3/6b1/r1K5 w - - 1 41",
        "8/3k4/4p2R/2P3P1/8/2P1N3/3K2b1/6r1 w - - 3 42",
        "8/3k4/4p2R/2P3P1/8/2PKNb2/8/6r1 w - - 5 43",
        "8/4k3/4p1R1/2P3P1/8/2PKNb2/8/6r1 w - - 7 44",
        "8/4k3/4p1R1/2P3P1/3K4/2P1N3/8/6rb w - - 9 45",
        "8/3k4/4p1R1/2P1K1P1/8/2P1N3/8/6rb w - - 11 46",
        "8/3k4/4p1R1/2P3P1/5K2/2P1N3/8/4r2b w - - 13 47",
        "8/3k4/2b1p2R/2P3P1/5K2/2P1N3/8/4r3 w - - 15 48",
        "8/3k4/2b1p3/2P3P1/5K2/2P1N2R/8/6r1 w - - 17 49",
        "2k5/7R/2b1p3/2P3P1/5K2/2P1N3/8/6r1 w - - 19 50",
        "2k5/7R/4p3/2P3P1/b1P2K2/4N3/8/6r1 w - - 1 51",
        "2k5/3bR3/4p3/2P3P1/2P2K2/4N3/8/6r1 w - - 3 52",
        "3k4/3b2R1/4p3/2P3P1/2P2K2/4N3/8/6r1 w - - 5 53",
        "3kb3/6R1/4p1P1/2P5/2P2K2/4N3/8/6r1 w - - 1 54",
        "3kb3/6R1/4p1P1/2P5/2P2KN1/8/8/2r5 w - - 3 55",
        "3kb3/6R1/4p1P1/2P1N3/2P2K2/8/8/5r2 w - - 5 56",
        "3kb3/6R1/4p1P1/2P1N3/2P5/4K3/8/4r3 w - - 7 57",
    },
    {
        "rnbq1rk1/ppp1npb1/4p1p1/3P3p/3PP3/2N2N2/PP2BPPP/R1BQ1RK1 b - - 0 8",
        "rnbq1rk1/ppp1npb1/6p1/3pP2p/3P4/2N2N2/PP2BPPP/R1BQ1RK1 b - - 0 9",
        "rn1q1rk1/ppp1npb1/6p1/3pP2p/3P2b1/2N2N2/PP2BPPP/R1BQR1K1 b - - 2 10",
        "r2q1rk1/ppp1npb1/2n3p1/3pP2p/3P2bN/2N5/PP2BPPP/R1BQR1K1 b - - 4 11",
        "r4rk1/pppqnpb1/2n3p1/3pP2p/3P2bN/2N4P/PP2BPP1/R1BQR1K1 b - - 0 12",
        "r4rk1/pppqnpb1/2n3p1/3pP2p/3P3N/7P/PP2NPP1/R1BQR1K1 b - - 0 13",
        "r4rk1/pppq1pb1/2n3p1/3pPN1p/3P4/7P/PP2NPP1/R1BQR1K1 b - - 0 14",
        "r4rk1/ppp2pb1/2n3p1/3pPq1p/3P1N2/7P/PP3PP1/R1BQR1K1 b - - 1 15",
        "r4rk1/pppq1pb1/2n3p1/3pP2p/P2P1N2/7P/1P3PP1/R1BQR1K1 b - - 0 16",
        "r2n1rk1/pppq1pb1/6p1/3pP2p/P2P1N2/R6P/1P3PP1/2BQR1K1 b - - 2 17",
        "r4rk1/pppq1pb1/4N1p1/3pP2p/P2P4/R6P/1P3PP1/2BQR1K1 b - - 0 18",
        "r4rk1/ppp2pb1/4q1p1/3pP1Bp/P2P4/R6P/1P3PP1/3QR1K1 b - - 1 19",
        "r3r1k1/ppp2pb1/4q1p1/3pP1Bp/P2P1P2/R6P/1P4P1/3QR1K1 b - - 0 20",
        "r3r1k1/ppp3b1/4qpp1/3pP2p/P2P1P1B/R6P/1P4P1/3QR1K1 b - - 1 21",
        "r3r1k1/ppp3b1/4q1p1/3pP2p/P4P1B/R6P/1P4P1/3QR1K1 b - - 0 22",
        "r4rk1/ppp3b1/4q1p1/3pP1Bp/P4P2/R6P/1P4P1/3QR1K1 b - - 2 23",
        "r4rk1/pp4b1/4q1p1/2ppP1Bp/P4P2/3R3P/1P4P1/3QR1K1 b - - 1 24",
        "r4rk1/pp4b1/4q1p1/2p1P1Bp/P2p1PP1/3R3P/1P6/3QR1K1 b - - 0 25",
        "r4rk1/pp4b1/4q1p1/2p1P1B1/P2p1PP1/3R4/1P6/3QR1K1 b - - 0 26",
        "r5k1/pp3rb1/4q1p1/2p1P1B1/P2p1PP1/6R1/1P6/3QR1K1 b - - 2 27",
        "5rk1/pp3rb1/4q1p1/2p1P1B1/P2pRPP1/6R1/1P6/3Q2K1 b - - 4 28",
        "5rk1/1p3rb1/p3q1p1/P1p1P1B1/3pRPP1/6R1/1P6/3Q2K1 b - - 0 29",
        "4r1k1/1p3rb1/p3q1p1/P1p1P1B1/3pRPP1/1P4R1/8/3Q2K1 b - - 0 30",
        "4r1k1/5rb1/pP2q1p1/2p1P1B1/3pRPP1/1P4R1/8/3Q2K1 b - - 0 31",
        "4r1k1/5rb1/pq4p1/2p1P1B1/3pRPP1/1P4R1/4Q3/6K1 b - - 1 32",
        "4r1k1/1r4b1/pq4p1/2p1P1B1/3pRPP1/1P4R1/2Q5/6K1 b - - 3 33",
        "4r1k1/1r4b1/1q4p1/p1p1P1B1/3p1PP1/1P4R1/2Q5/4R1K1 b - - 1 34",
        "4r1k1/3r2b1/1q4p1/p1p1P1B1/2Qp1PP1/1P4R1/8/4R1K1 b - - 3 35",
        "4r1k1/3r2b1/4q1p1/p1p1P1B1/2Qp1PP1/1P4R1/5K2/4R3 b - - 5 36",
        "4r1k1/3r2b1/6p1/p1p1P1B1/2Pp1PP1/6R1/5K2/4R3 b - - 0 37",
        "4r1k1/3r2b1/6p1/p1p1P1B1/2P2PP1/3p2R1/5K2/3R4 b - - 1 38",
        "5rk1/3r2b1/6p1/p1p1P1B1/2P2PP1/3p2R1/8/3RK3 b - - 3 39",
        "5rk1/6b1/6p1/p1p1P1B1/2Pr1PP1/3R4/8/3RK3 b - - 0 40",
        "5rk1/3R2b1/6p1/p1p1P1B1/2r2PP1/8/8/3RK3 b - - 1 41",
        "5rk1/3R2b1/6p1/p1p1P1B1/4rPP1/8/3K4/3R4 b - - 3 42",
        "1r4k1/3R2b1/6p1/p1p1P1B1/4rPP1/2K5/8/3R4 b - - 5 43",
        "1r4k1/3R2b1/6p1/p1p1P1B1/2K2PP1/4r3/8/3R4 b - - 7 44",
        "1r3bk1/8/3R2p1/p1p1P1B1/2K2PP1/4r3/8/3R4 b - - 9 45",
        "1r3bk1/8/6R1/2p1P1B1/p1K2PP1/4r3/8/3R4 b - - 0 46",
        "1r3b2/5k2/R7/2p1P1B1/p1K2PP1/4r3/8/3R4 b - - 2 47",
        "5b2/1r3k2/R7/2p1P1B1/p1K2PP1/4r3/8/7R b - - 4 48",
        "5b2/5k2/R7/2pKP1B1/pr3PP1/4r3/8/7R b - - 6 49",
        "5b2/5k2/R1K5/2p1P1B1/p2r1PP1/4r3/8/7R b - - 8 50",
        "8/R4kb1/2K5/2p1P1B1/p2r1PP1/4r3/8/7R b - - 10 51",
        "8/R5b1/2K3k1/2p1PPB1/p2r2P1/4r3/8/7R b - - 0 52",
        "8/6R1/2K5/2p1PPk1/p2r2P1/4r3/8/7R b - - 0 53",
        "8/6R1/2K5/2p1PP2/p2r1kP1/4r3/8/5R2 b - - 2 54",
        "8/6R1/2K2P2/2p1P3/p2r2P1/4r1k1/8/5R2 b - - 0 55",
        "8/5PR1/2K5/2p1P3/p2r2P1/4r3/6k1/5R2 b - - 0 56",
    },
    {
        "rn1qkb1r/p1pbpppp/5n2/8/2pP4/2N5/1PQ1PPPP/R1B1KBNR w KQkq - 0 7",
        "r2qkb1r/p1pbpppp/2n2n2/8/2pP4/2N2N2/1PQ1PPPP/R1B1KB1R w KQkq - 2 8",
        "r2qkb1r/p1pbpppp/5n2/8/1npPP3/2N2N2/1PQ2PPP/R1B1KB1R w KQkq - 1 9",
        "r2qkb1r/p1pb1ppp/4pn2/8/1npPP3/2N2N2/1P3PPP/R1BQKB1R w KQkq - 0 10",
        "r2qk2r/p1pbbppp/4pn2/8/1nBPP3/2N2N2/1P3PPP/R1BQK2R w KQkq - 1 11",
        "r2q1rk1/p1pbbppp/4pn2/8/1nBPP3/2N2N2/1P3PPP/R1BQ1RK1 w - - 3 12",
        "r2q1rk1/2pbbppp/p3pn2/8/1nBPPB2/2N2N2/1P3PPP/R2Q1RK1 w - - 0 13",
        "r2q1rk1/2p1bppp/p3pn2/1b6/1nBPPB2/2N2N2/1P3PPP/R2QR1K1 w - - 2 14",
        "r2q1rk1/4bppp/p1p1pn2/1b6/1nBPPB2/1PN2N2/5PPP/R2QR1K1 w - - 0 15",
        "r4rk1/3qbppp/p1p1pn2/1b6/1nBPPB2/1PN2N2/3Q1PPP/R3R1K1 w - - 2 16",
        "r4rk1/1q2bppp/p1p1pn2/1b6/1nBPPB2/1PN2N1P/3Q1PP1/R3R1K1 w - - 1 17",
        "r3r1k1/1q2bppp/p1p1pn2/1b6/1nBPPB2/1PN2N1P/4QPP1/R3R1K1 w - - 3 18",
        "r3r1k1/1q1nbppp/p1p1p3/1b6/1nBPPB2/1PN2N1P/4QPP1/3RR1K1 w - - 5 19",
        "r3rbk1/1q1n1ppp/p1p1p3/1b6/1nBPPB2/1PN2N1P/3RQPP1/4R1K1 w - - 7 20",
        "r3rbk1/1q3ppp/pnp1p3/1b6/1nBPPB2/1PN2N1P/3RQPP1/4R2K w - - 9 21",
        "2r1rbk1/1q3ppp/pnp1p3/1b6/1nBPPB2/1PN2N1P/3RQPP1/1R5K w - - 11 22",
        "2r1rbk1/1q4pp/pnp1pp2/1b6/1nBPPB2/1PN2N1P/4QPP1/1R1R3K w - - 0 23",
        "2r1rbk1/5qpp/pnp1pp2/1b6/1nBPP3/1PN1BN1P/4QPP1/1R1R3K w - - 2 24",
        "2r1rbk1/5qp1/pnp1pp1p/1b6/1nBPP3/1PN1BN1P/4QPP1/1R1R2K1 w - - 0 25",
        "2r1rbk1/5qp1/pnp1pp1p/1b6/2BPP3/1P2BN1P/n3QPP1/1R1R2K1 w - - 0 26",
        "r3rbk1/5qp1/pnp1pp1p/1b6/2BPP3/1P2BN1P/Q4PP1/1R1R2K1 w - - 1 27",
        "rr3bk1/5qp1/pnp1pp1p/1b6/2BPP3/1P2BN1P/Q4PP1/R2R2K1 w - - 3 28",
        "rr2qbk1/6p1/pnp1pp1p/1b6/2BPP3/1P2BN1P/4QPP1/R2R2K1 w - - 5 29",
        "rr2qbk1/6p1/1np1pp1p/pb6/2BPP3/1P1QBN1P/5PP1/R2R2K1 w - - 0 30",
        "rr2qbk1/6p1/1n2pp1p/pp6/3PP3/1P1QBN1P/5PP1/R2R2K1 w - - 0 31",
        "rr2qbk1/6p1/1n2pp1p/1p1P4/p3P3/1P1QBN1P/5PP1/R2R2K1 w - - 0 32",
        "rr2qbk1/3n2p1/3Ppp1p/1p6/p3P3/1P1QBN1P/5PP1/R2R2K1 w - - 1 33",
        "rr3bk1/3n2p1/3Ppp1p/1p5q/pP2P3/3QBN1P/5PP1/R2R2K1 w - - 1 34",
        "rr3bk1/3n2p1/3Ppp1p/1p5q/1P2P3/p2QBN1P/5PP1/2RR2K1 w - - 0 35",
        "1r3bk1/3n2p1/r2Ppp1p/1p5q/1P2P3/pQ2BN1P/5PP1/2RR2K1 w - - 2 36",
        "1r2qbk1/2Rn2p1/r2Ppp1p/1p6/1P2P3/pQ2BN1P/5PP1/3R2K1 w - - 4 37",
        "1r2qbk1/2Rn2p1/r2Ppp1p/1pB5/1P2P3/1Q3N1P/p4PP1/3R2K1 w - - 0 38",
        "1r2q1k1/2Rn2p1/r2bpp1p/1pB5/1P2P3/1Q3N1P/p4PP1/R5K1 w - - 0 39",
        "1r2q1k1/2Rn2p1/3rpp1p/1p6/1P2P3/1Q3N1P/p4PP1/R5K1 w - - 0 40",
        "2r1q1k1/2Rn2p1/3rpp1p/1p6/1P2P3/5N1P/Q4PP1/R5K1 w - - 1 41",
        "1r2q1k1/1R1n2p1/3rpp1p/1p6/1P2P3/5N1P/Q4PP1/R5K1 w - - 3 42",
        "2r1q1k1/2Rn2p1/3rpp1p/1p6/1P2P3/5N1P/Q4PP1/R5K1 w - - 5 43",
        "1r2q1k1/1R1n2p1/3rpp1p/1p6/1P2P3/5N1P/Q4PP1/R5K1 w - - 7 44",
        "1rq3k1/R2n2p1/3rpp1p/1p6/1P2P3/5N1P/Q4PP1/R5K1 w - - 9 45",
        "2q3k1/Rr1n2p1/3rpp1p/1p6/1P2P3/5N1P/4QPP1/R5K1 w - - 11 46",
        "Rrq3k1/3n2p1/3rpp1p/1p6/1P2P3/5N1P/4QPP1/R5K1 w - - 13 47",
    },
    {
        "rn1qkb1r/1pp2ppp/p4p2/3p1b2/5P2/1P2PN2/P1PP2PP/RN1QKB1R b KQkq - 1 6",
        "r2qkb1r/1pp2ppp/p1n2p2/3p1b2/3P1P2/1P2PN2/P1P3PP/RN1QKB1R b KQkq - 0 7",
        "r2qkb1r/1pp2ppp/p4p2/3p1b2/1n1P1P2/1P1BPN2/P1P3PP/RN1QK2R b KQkq - 2 8",
        "r2qkb1r/1pp2ppp/p4p2/3p1b2/3P1P2/1P1PPN2/P5PP/RN1QK2R b KQkq - 0 9",
        "r2qk2r/1pp2ppp/p2b1p2/3p1b2/3P1P2/1PNPPN2/P5PP/R2QK2R b KQkq - 2 10",
        "r2qk2r/1p3ppp/p1pb1p2/3p1b2/3P1P2/1PNPPN2/P5PP/R2Q1RK1 b kq - 1 11",
        "r2q1rk1/1p3ppp/p1pb1p2/3p1b2/3P1P2/1PNPPN2/P2Q2PP/R4RK1 b - - 3 12",
        "r2qr1k1/1p3ppp/p1pb1p2/3p1b2/3P1P2/1P1PPN2/P2QN1PP/R4RK1 b - - 5 13",
        "r3r1k1/1p3ppp/pqpb1p2/3p1b2/3P1P2/1P1PPNN1/P2Q2PP/R4RK1 b - - 7 14",
        "r3r1k1/1p3ppp/pqp2p2/3p1b2/1b1P1P2/1P1PPNN1/P1Q3PP/R4RK1 b - - 9 15",
        "r3r1k1/1p1b1ppp/pqp2p2/3p4/1b1P1P2/1P1PPNN1/P4QPP/R4RK1 b - - 11 16",
        "2r1r1k1/1p1b1ppp/pqp2p2/3p4/1b1PPP2/1P1P1NN1/P4QPP/R4RK1 b - - 0 17",
        "2r1r1k1/1p1b1ppp/pq3p2/2pp4/1b1PPP2/PP1P1NN1/5QPP/R4RK1 b - - 0 18",
        "2r1r1k1/1p1b1ppp/pq3p2/2Pp4/4PP2/PPbP1NN1/5QPP/R4RK1 b - - 0 19",
        "2r1r1k1/1p1b1ppp/p4p2/2Pp4/4PP2/PqbP1NN1/5QPP/RR4K1 b - - 1 20",
        "2r1r1k1/1p1b1ppp/p4p2/2Pp4/q3PP2/P1bP1NN1/R4QPP/1R4K1 b - - 3 21",
        "2r1r1k1/1p3ppp/p4p2/1bPP4/q4P2/P1bP1NN1/R4QPP/1R4K1 b - - 0 22",
        "2r1r1k1/1p3ppp/p4p2/2PP4/q4P2/P1bb1NN1/R4QPP/2R3K1 b - - 1 23",
        "2r1r1k1/1p3ppp/p2P1p2/2P5/2q2P2/P1bb1NN1/R4QPP/2R3K1 b - - 0 24",
        "2rr2k1/1p3ppp/p2P1p2/2P5/2q2P2/P1bb1NN1/R4QPP/2R4K b - - 2 25",
        "2rr2k1/1p3ppp/p2P1p2/2Q5/5P2/P1bb1NN1/R5PP/2R4K b - - 0 26",
        "3r2k1/1p3ppp/p2P1p2/2r5/5P2/P1bb1N2/R3N1PP/2R4K b - - 1 27",
        "3r2k1/1p3ppp/p2P1p2/2r5/5P2/P1b2N2/4R1PP/2R4K b - - 0 28",
        "3r2k1/1p3ppp/p2P1p2/2r5/1b3P2/P4N2/4R1PP/3R3K b - - 2 29",
        "3r2k1/1p2Rppp/p2P1p2/b1r5/5P2/P4N2/6PP/3R3K b - - 4 30",
        "3r2k1/1R3ppp/p1rP1p2/b7/5P2/P4N2/6PP/3R3K b - - 0 31",
        "3r2k1/1R3ppp/p2R1p2/b7/5P2/P4N2/6PP/7K b - - 0 32",
        "6k1/1R3ppp/p2r1p2/b7/5P2/P4NP1/7P/7K b - - 0 33",
        "6k1/1R3p1p/p2r1pp1/b7/5P1P/P4NP1/8/7K b - - 0 34",
        "6k1/3R1p1p/pr3pp1/b7/5P1P/P4NP1/8/7K b - - 2 35",
        "6k1/5p2/pr3pp1/b2R3p/5P1P/P4NP1/8/7K b - - 1 36",
        "6k1/5p2/pr3pp1/7p/5P1P/P1bR1NP1/8/7K b - - 3 37",
        "6k1/5p2/p1r2pp1/7p/5P1P/P1bR1NP1/6K1/8 b - - 5 38",
        "6k1/5p2/p1r2pp1/b2R3p/5P1P/P4NP1/6K1/8 b - - 7 39",
        "6k1/5p2/p4pp1/b2R3p/5P1P/P4NPK/2r5/8 b - - 9 40",
        "6k1/2b2p2/p4pp1/7p/5P1P/P2R1NPK/2r5/8 b - - 11 41",
        "6k1/2b2p2/5pp1/p6p/3N1P1P/P2R2PK/2r5/8 b - - 1 42",
        "6k1/2b2p2/5pp1/p6p/3N1P1P/P1R3PK/r7/8 b - - 3 43",
        "6k1/5p2/1b3pp1/p6p/5P1P/P1R3PK/r1N5/8 b - - 5 44",
        "8/5pk1/1bR2pp1/p6p/5P1P/P5PK/r1N5/8 b - - 7 45",
        "3b4/5pk1/2R2pp1/p4P1p/7P/P5PK/r1N5/8 b - - 0 46",
        "8/4bpk1/2R2pp1/p4P1p/6PP/P6K/r1N5/8 b - - 0 47",
        "8/5pk1/2R2pP1/p6p/6PP/b6K/r1N5/8 b - - 0 48",
        "8/6k1/2R2pp1/p6P/7P/b6K/r1N5/8 b - - 0 49",
        "8/6k1/2R2p2/p6p/7P/b5K1/r1N5/8 b - - 1 50",
        "8/8/2R2pk1/p6p/7P/b4K2/r1N5/8 b - - 3 51",
        "8/8/2R2pk1/p6p/7P/4NK2/rb6/8 b - - 5 52",
        "2R5/8/5pk1/7p/p6P/4NK2/rb6/8 b - - 1 53",
        "6R1/8/5pk1/7p/p6P/4NK2/1b6/r7 b - - 3 54",
        "R7/5k2/5p2/7p/p6P/4NK2/1b6/r7 b - - 5 55",
        "R7/5k2/5p2/7p/7P/p3N3/1b2K3/r7 b - - 1 56",
        "8/R4k2/5p2/7p/7P/p3N3/1b2K3/7r b - - 3 57",
        "8/8/5pk1/7p/R6P/p3N3/1b2K3/7r b - - 5 58",
        "8/8/5pk1/7p/R6P/p7/4K3/2bN3r b - - 7 59",
        "8/8/5pk1/7p/R6P/p7/4KN1r/2b5 b - - 9 60",
        "8/8/5pk1/7p/R6P/p3K3/1b3N1r/8 b - - 11 61",
        "8/8/R4pk1/7p/7P/p1b1K3/5N1r/8 b - - 13 62",
        "8/8/5pk1/7p/7P/2b1K3/R4N1r/8 b - - 0 63",
        "8/8/5pk1/7p/3K3P/8/R4N1r/4b3 b - - 2 64",
    }
};
// clang-format on

}  // namespace

namespace Alexander::Benchmark {

// Builds a list of UCI commands to be run by bench. There
// are five parameters: TT size in MB, number of search threads that
// should be used, the limit value spent for each position, a file name
// where to look for positions in FEN format, and the type of the limit:
// depth, perft, nodes and movetime (in milliseconds). Examples:
//
// bench                            : search default positions up to depth 13
// bench 64 1 15                    : search default positions up to depth 15 (TT = 64MB)
// bench 64 1 100000 default nodes  : search default positions for 100K nodes each
// bench 64 4 5000 current movetime : search current position with 4 threads for 5 sec
// bench 16 1 5 blah perft          : run a perft 5 on positions in file "blah"
std::vector<std::string> setup_bench(const std::string& currentFen, std::istream& is) {

    std::vector<std::string> fens, list;
    std::string              go, token;

    // Assign default values to missing arguments
    std::string ttSize    = (is >> token) ? token : "16";
    std::string threads   = (is >> token) ? token : "1";
    std::string limit     = (is >> token) ? token : "13";
    std::string fenFile   = (is >> token) ? token : "default";
    std::string limitType = (is >> token) ? token : "depth";

    go = limitType == "eval" ? "eval" : "go " + limitType + " " + limit;

    if (fenFile == "default")
        fens = Defaults;

    else if (fenFile == "current")
        fens.push_back(currentFen);

    else
    {
        std::string   fen;
        std::ifstream file(fenFile);

        if (!file.is_open())
        {
            std::cerr << "Unable to open file " << fenFile << std::endl;
            exit(EXIT_FAILURE);
        }

        while (getline(file, fen))
            if (!fen.empty())
                fens.push_back(fen);

        file.close();
    }

    list.emplace_back("setoption name Threads value " + threads);
    list.emplace_back("setoption name Hash value " + ttSize);
    list.emplace_back("ucinewgame");

    for (const std::string& fen : fens)
        if (fen.find("setoption") != std::string::npos)
            list.emplace_back(fen);
        else
        {
            list.emplace_back("position fen " + fen);
            list.emplace_back(go);
        }

    return list;
}

BenchmarkSetup setup_benchmark(std::istream& is) {
    // TT_SIZE_PER_THREAD is chosen such that roughly half of the hash is used all positions
    // for the current sequence have been searched.
    static constexpr int TT_SIZE_PER_THREAD = 128;

    static constexpr int DEFAULT_DURATION_S = 150;

    BenchmarkSetup setup{};

    // Assign default values to missing arguments
    int desiredTimeS;

    if (!(is >> setup.threads))
        setup.threads = int(get_hardware_concurrency());
    else
        setup.originalInvocation += std::to_string(setup.threads);

    if (!(is >> setup.ttSize))
        setup.ttSize = TT_SIZE_PER_THREAD * setup.threads;
    else
        setup.originalInvocation += " " + std::to_string(setup.ttSize);

    if (!(is >> desiredTimeS))
        desiredTimeS = DEFAULT_DURATION_S;
    else
        setup.originalInvocation += " " + std::to_string(desiredTimeS);

    setup.filledInvocation += std::to_string(setup.threads) + " " + std::to_string(setup.ttSize)
                            + " " + std::to_string(desiredTimeS);

    auto getCorrectedTime = [&](int ply) {
        // time per move is fit roughly based on LTC games
        // seconds = 50/{ply+15}
        // ms = 50000/{ply+15}
        // with this fit 10th move gets 2000ms
        // adjust for desired 10th move time
        return 50000.0 / (static_cast<double>(ply) + 15.0);
    };

    float totalTime = 0;
    for (const auto& game : BenchmarkPositions)
    {
        int ply = 1;
        for (int i = 0; i < static_cast<int>(game.size()); ++i)
        {
            const float correctedTime = float(getCorrectedTime(ply));
            totalTime += correctedTime;
            ply += 1;
        }
    }

    float timeScaleFactor = static_cast<float>(desiredTimeS * 1000) / totalTime;

    for (const auto& game : BenchmarkPositions)
    {
        setup.commands.emplace_back("ucinewgame");
        int ply = 1;
        for (const std::string& fen : game)
        {
            setup.commands.emplace_back("position fen " + fen);

            const int correctedTime = static_cast<int>(getCorrectedTime(ply) * timeScaleFactor);
            setup.commands.emplace_back("go movetime " + std::to_string(correctedTime));

            ply += 1;
        }
    }

    return setup;
}

}  // namespace Alexander

// ============================================================================
// FILE ORIGINALE: benchmark.h
// ============================================================================

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

#ifndef BENCHMARK_H_INCLUDED
#define BENCHMARK_H_INCLUDED

#include <iosfwd>
#include <string>
#include <vector>

namespace Alexander::Benchmark {

std::vector<std::string> setup_bench(const std::string&, std::istream&);

struct BenchmarkSetup {
    int                      ttSize;
    int                      threads;
    std::vector<std::string> commands;
    std::string              originalInvocation;
    std::string              filledInvocation;
};

BenchmarkSetup setup_benchmark(std::istream&);

}  // namespace Alexander

#endif  // #ifndef BENCHMARK_H_INCLUDED


// ============================================================================
// FILE ORIGINALE: bitbase.cpp
// ============================================================================

/*
  Alexander, a UCI chess playing engine derived from Stockfish
  Copyright (C) 2004-2023 The Stockfish developers (see AUTHORS file)

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

#include <cassert>
#include <vector>
#include <bitset>

#include "bitboard.h"
#include "types.h"

namespace Alexander {

namespace {

// There are 24 possible pawn squares: files A to D and ranks from 2 to 7.
// Positions with the pawn on files E to H will be mirrored before probing.
constexpr unsigned MAX_INDEX = 2 * 24 * 64 * 64;  // stm * psq * wksq * bksq = 196608

std::bitset<MAX_INDEX> KPKBitbase;

// A KPK bitbase index is an integer in [0, IndexMax] range
//
// Information is mapped in a way that minimizes the number of iterations:
//
// bit  0- 5: white king square (from SQ_A1 to SQ_H8)
// bit  6-11: black king square (from SQ_A1 to SQ_H8)
// bit    12: side to move (WHITE or BLACK)
// bit 13-14: white pawn file (from FILE_A to FILE_D)
// bit 15-17: white pawn RANK_7 - rank (from RANK_7 - RANK_7 to RANK_7 - RANK_2)
unsigned index(Color stm, Square bksq, Square wksq, Square psq) {
    return int(wksq) | (bksq << 6) | (stm << 12) | (file_of(psq) << 13)
         | ((RANK_7 - rank_of(psq)) << 15);
}

enum Result {
    INVALID = 0,
    UNKNOWN = 1,
    DRAW    = 2,
    WIN     = 4
};

Result& operator|=(Result& r, Result v) { return r = Result(r | v); }

struct KPKPosition {
    KPKPosition() = default;
    explicit KPKPosition(unsigned idx);
           operator Result() const { return result; }
    Result classify(const std::vector<KPKPosition>& db);

    Color  stm;
    Square ksq[COLOR_NB], psq;
    Result result;
};

}  // namespace

bool Bitbases::probe(Square wksq, Square wpsq, Square bksq, Color stm) {

    assert(file_of(wpsq) <= FILE_D);

    return KPKBitbase[index(stm, bksq, wksq, wpsq)];
}


void Bitbases::init() {

    std::vector<KPKPosition> db(MAX_INDEX);
    unsigned                 idx, repeat = 1;

    // Initialize db with known win / draw positions
    for (idx = 0; idx < MAX_INDEX; ++idx)
        db[idx] = KPKPosition(idx);

    // Iterate through the positions until none of the unknown positions can be
    // changed to either wins or draws (15 cycles needed).
    while (repeat)
        for (repeat = idx = 0; idx < MAX_INDEX; ++idx)
            repeat |= (db[idx] == UNKNOWN && db[idx].classify(db) != UNKNOWN);

    // Fill the bitbase with the decisive results
    for (idx = 0; idx < MAX_INDEX; ++idx)
        if (db[idx] == WIN)
            KPKBitbase.set(idx);
}

namespace {

KPKPosition::KPKPosition(unsigned idx) {

    ksq[WHITE] = Square((idx >> 0) & 0x3F);
    ksq[BLACK] = Square((idx >> 6) & 0x3F);
    stm        = Color((idx >> 12) & 0x01);
    psq        = make_square(File((idx >> 13) & 0x3), Rank(RANK_7 - ((idx >> 15) & 0x7)));

    // Invalid if two pieces are on the same square or if a king can be captured
    if (distance(ksq[WHITE], ksq[BLACK]) <= 1 || ksq[WHITE] == psq || ksq[BLACK] == psq
        || (stm == WHITE && (attacks_bb<PAWN>(psq, WHITE) & ksq[BLACK])))
        result = INVALID;

    // Win if the pawn can be promoted without getting captured
    else if (stm == WHITE && rank_of(psq) == RANK_7 && ksq[WHITE] != psq + NORTH
             && (distance(ksq[BLACK], psq + NORTH) > 1 || (distance(ksq[WHITE], psq + NORTH) == 1)))
        result = WIN;

    // Draw if it is stalemate or the black king can capture the pawn
    else if (stm == BLACK
             && (!(attacks_bb<KING>(ksq[BLACK])
                   & ~(attacks_bb<KING>(ksq[WHITE]) | attacks_bb<PAWN>(psq, WHITE)))
                 || (attacks_bb<KING>(ksq[BLACK]) & ~attacks_bb<KING>(ksq[WHITE]) & psq)))
        result = DRAW;

    // Position will be classified later
    else
        result = UNKNOWN;
}

Result KPKPosition::classify(const std::vector<KPKPosition>& db) {

    // White to move: If one move leads to a position classified as WIN, the result
    // of the current position is WIN. If all moves lead to positions classified
    // as DRAW, the current position is classified as DRAW, otherwise the current
    // position is classified as UNKNOWN.
    //
    // Black to move: If one move leads to a position classified as DRAW, the result
    // of the current position is DRAW. If all moves lead to positions classified
    // as WIN, the position is classified as WIN, otherwise the current position is
    // classified as UNKNOWN.
    const Result Good = (stm == WHITE ? WIN : DRAW);
    const Result Bad  = (stm == WHITE ? DRAW : WIN);

    Result   r = INVALID;
    Bitboard b = attacks_bb<KING>(ksq[stm]);

    while (b)
        r |= stm == WHITE ? db[index(BLACK, ksq[BLACK], pop_lsb(b), psq)]
                          : db[index(WHITE, pop_lsb(b), ksq[WHITE], psq)];

    if (stm == WHITE)
    {
        if (rank_of(psq) < RANK_7)  // Single push
            r |= db[index(BLACK, ksq[BLACK], ksq[WHITE], psq + NORTH)];

        if (rank_of(psq) == RANK_2  // Double push
            && psq + NORTH != ksq[WHITE] && psq + NORTH != ksq[BLACK])
            r |= db[index(BLACK, ksq[BLACK], ksq[WHITE], psq + NORTH + NORTH)];
    }

    return result = r & Good ? Good : r & UNKNOWN ? UNKNOWN : Bad;
}

}  // namespace

}  // namespace Alexander


// ============================================================================
// FILE ORIGINALE: bitboard.cpp
// ============================================================================

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

#include "bitboard.h"

#include <algorithm>
#include <bitset>
#include <initializer_list>

#include "misc.h"

namespace Alexander {

uint8_t PopCnt16[1 << 16];
uint8_t SquareDistance[SQUARE_NB][SQUARE_NB];

Bitboard LineBB[SQUARE_NB][SQUARE_NB];
Bitboard BetweenBB[SQUARE_NB][SQUARE_NB];
Bitboard RayPassBB[SQUARE_NB][SQUARE_NB];

alignas(64) Magic Magics[SQUARE_NB][2];

namespace {

Bitboard RookTable[0x19000];   // To store rook attacks
Bitboard BishopTable[0x1480];  // To store bishop attacks

void init_magics(PieceType pt, Bitboard table[], Magic magics[][2]);
}

// Returns an ASCII representation of a bitboard suitable
// to be printed to standard output. Useful for debugging.
std::string Bitboards::pretty(Bitboard b) {

    std::string s = "+---+---+---+---+---+---+---+---+\n";

    for (Rank r = RANK_8;; --r)
    {
        for (File f = FILE_A; f <= FILE_H; ++f)
            s += b & make_square(f, r) ? "| X " : "|   ";

        s += "| " + std::to_string(1 + r) + "\n+---+---+---+---+---+---+---+---+\n";

        if (r == RANK_1)
            break;
    }
    s += "  a   b   c   d   e   f   g   h\n";

    return s;
}


// Initializes various bitboard tables. It is called at
// startup and relies on global objects to be already zero-initialized.
void Bitboards::init() {

    for (unsigned i = 0; i < (1 << 16); ++i)
        PopCnt16[i] = uint8_t(std::bitset<16>(i).count());

    for (Square s1 = SQ_A1; s1 <= SQ_H8; ++s1)
        for (Square s2 = SQ_A1; s2 <= SQ_H8; ++s2)
            SquareDistance[s1][s2] = std::max(distance<File>(s1, s2), distance<Rank>(s1, s2));

    init_magics(ROOK, RookTable, Magics);
    init_magics(BISHOP, BishopTable, Magics);

    for (Square s1 = SQ_A1; s1 <= SQ_H8; ++s1)
    {
        for (PieceType pt : {BISHOP, ROOK})
            for (Square s2 = SQ_A1; s2 <= SQ_H8; ++s2)
            {
                if (PseudoAttacks[pt][s1] & s2)
                {
                    LineBB[s1][s2] = (attacks_bb(pt, s1, 0) & attacks_bb(pt, s2, 0)) | s1 | s2;
                    BetweenBB[s1][s2] =
                      (attacks_bb(pt, s1, square_bb(s2)) & attacks_bb(pt, s2, square_bb(s1)));
                    RayPassBB[s1][s2] =
                      attacks_bb(pt, s1, 0) & (attacks_bb(pt, s2, square_bb(s1)) | s2);
                }
                BetweenBB[s1][s2] |= s2;
            }
    }
}

namespace {
// Computes all rook and bishop attacks at startup. Magic
// bitboards are used to look up attacks of sliding pieces. As a reference see
// https://www.chessprogramming.org/Magic_Bitboards. In particular, here we use
// the so called "fancy" approach.
void init_magics(PieceType pt, Bitboard table[], Magic magics[][2]) {

#ifndef USE_PEXT
    // Optimal PRNG seeds to pick the correct magics in the shortest time
    int seeds[][RANK_NB] = {{8977, 44560, 54343, 38998, 5731, 95205, 104912, 17020},
                            {728, 10316, 55013, 32803, 12281, 15100, 16645, 255}};

    Bitboard occupancy[4096];
    int      epoch[4096] = {}, cnt = 0;
#endif
    Bitboard reference[4096];
    int      size = 0;

    for (Square s = SQ_A1; s <= SQ_H8; ++s)
    {
        // Board edges are not considered in the relevant occupancies
        Bitboard edges = ((Rank1BB | Rank8BB) & ~rank_bb(s)) | ((FileABB | FileHBB) & ~file_bb(s));

        // Given a square 's', the mask is the bitboard of sliding attacks from
        // 's' computed on an empty board. The index must be big enough to contain
        // all the attacks for each possible subset of the mask and so is 2 power
        // the number of 1s of the mask. Hence we deduce the size of the shift to
        // apply to the 64 or 32 bits word to get the index.
        Magic& m = magics[s][pt - BISHOP];
        m.mask   = Bitboards::sliding_attack(pt, s, 0) & ~edges;
#ifndef USE_PEXT
        m.shift = (Is64Bit ? 64 : 32) - popcount(m.mask);
#endif
        // Set the offset for the attacks table of the square. We have individual
        // table sizes for each square with "Fancy Magic Bitboards".
        m.attacks = s == SQ_A1 ? table : magics[s - 1][pt - BISHOP].attacks + size;
        size      = 0;

        // Use Carry-Rippler trick to enumerate all subsets of masks[s] and
        // store the corresponding sliding attack bitboard in reference[].
        Bitboard b = 0;
        do
        {
#ifndef USE_PEXT
            occupancy[size] = b;
#endif
            reference[size] = Bitboards::sliding_attack(pt, s, b);

            if (HasPext)
                m.attacks[pext(b, m.mask)] = reference[size];

            size++;
            b = (b - m.mask) & m.mask;
        } while (b);

#ifndef USE_PEXT
        PRNG rng(seeds[Is64Bit][rank_of(s)]);

        // Find a magic for square 's' picking up an (almost) random number
        // until we find the one that passes the verification test.
        for (int i = 0; i < size;)
        {
            for (m.magic = 0; popcount((m.magic * m.mask) >> 56) < 6;)
                m.magic = rng.sparse_rand<Bitboard>();

            // A good magic must map every possible occupancy to an index that
            // looks up the correct sliding attack in the attacks[s] database.
            // Note that we build up the database for square 's' as a side
            // effect of verifying the magic. Keep track of the attempt count
            // and save it in epoch[], little speed-up trick to avoid resetting
            // m.attacks[] after every failed attempt.
            for (++cnt, i = 0; i < size; ++i)
            {
                unsigned idx = m.index(occupancy[i]);

                if (epoch[idx] < cnt)
                {
                    epoch[idx]     = cnt;
                    m.attacks[idx] = reference[i];
                }
                else if (m.attacks[idx] != reference[i])
                    break;
            }
        }
#endif
    }
}
}

}  // namespace Alexander


// ============================================================================
// FILE ORIGINALE: bitboard.h
// ============================================================================

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

#ifndef BITBOARD_H_INCLUDED
#define BITBOARD_H_INCLUDED

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstring>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <initializer_list>
#include <array>

#include "types.h"

namespace Alexander {
//for classical begin
namespace Bitbases {

void init();
bool probe(Square wksq, Square wpsq, Square bksq, Color us);

}  // namespace Alexander::Bitbases
//for classical end
namespace Bitboards {

void        init();
std::string pretty(Bitboard b);

}  // namespace Alexander::Bitboards
//for classical begin
constexpr Bitboard AllSquares  = ~Bitboard(0);
constexpr Bitboard DarkSquares = 0xAA55AA55AA55AA55ULL;
//for classical end
constexpr Bitboard FileABB = 0x0101010101010101ULL;
constexpr Bitboard FileBBB = FileABB << 1;
constexpr Bitboard FileCBB = FileABB << 2;
constexpr Bitboard FileDBB = FileABB << 3;
constexpr Bitboard FileEBB = FileABB << 4;
constexpr Bitboard FileFBB = FileABB << 5;
constexpr Bitboard FileGBB = FileABB << 6;
constexpr Bitboard FileHBB = FileABB << 7;

constexpr Bitboard Rank1BB = 0xFF;
constexpr Bitboard Rank2BB = Rank1BB << (8 * 1);
constexpr Bitboard Rank3BB = Rank1BB << (8 * 2);
constexpr Bitboard Rank4BB = Rank1BB << (8 * 3);
constexpr Bitboard Rank5BB = Rank1BB << (8 * 4);
constexpr Bitboard Rank6BB = Rank1BB << (8 * 5);
constexpr Bitboard Rank7BB = Rank1BB << (8 * 6);
constexpr Bitboard Rank8BB = Rank1BB << (8 * 7);
//for classical begin
constexpr Bitboard QueenSide   = FileABB | FileBBB | FileCBB | FileDBB;
constexpr Bitboard CenterFiles = FileCBB | FileDBB | FileEBB | FileFBB;
constexpr Bitboard KingSide    = FileEBB | FileFBB | FileGBB | FileHBB;
constexpr Bitboard Center      = (FileDBB | FileEBB) & (Rank4BB | Rank5BB);

constexpr Bitboard KingFlank[FILE_NB] = {
  QueenSide ^ FileDBB, QueenSide, QueenSide, CenterFiles,
  CenterFiles,         KingSide,  KingSide,  KingSide ^ FileEBB};
//for classical end
extern uint8_t PopCnt16[1 << 16];
extern uint8_t SquareDistance[SQUARE_NB][SQUARE_NB];

extern Bitboard BetweenBB[SQUARE_NB][SQUARE_NB];
extern Bitboard LineBB[SQUARE_NB][SQUARE_NB];
extern Bitboard RayPassBB[SQUARE_NB][SQUARE_NB];


// Magic holds all magic bitboards relevant data for a single square
struct Magic {
    Bitboard  mask;
    Bitboard* attacks;
#ifndef USE_PEXT
    Bitboard magic;
    unsigned shift;
#endif

    // Compute the attack's index using the 'magic bitboards' approach
    unsigned index(Bitboard occupied) const {

#ifdef USE_PEXT
        return unsigned(pext(occupied, mask));
#else
        if (Is64Bit)
            return unsigned(((occupied & mask) * magic) >> shift);

        unsigned lo = unsigned(occupied) & unsigned(mask);
        unsigned hi = unsigned(occupied >> 32) & unsigned(mask >> 32);
        return (lo * unsigned(magic) ^ hi * unsigned(magic >> 32)) >> shift;
#endif
    }

    Bitboard attacks_bb(Bitboard occupied) const { return attacks[index(occupied)]; }
};

extern Magic Magics[SQUARE_NB][2];

constexpr Bitboard square_bb(Square s) {
    assert(is_ok(s));
    return (1ULL << s);
}


// Overloads of bitwise operators between a Bitboard and a Square for testing
// whether a given bit is set in a bitboard, and for setting and clearing bits.

constexpr Bitboard  operator&(Bitboard b, Square s) { return b & square_bb(s); }
constexpr Bitboard  operator|(Bitboard b, Square s) { return b | square_bb(s); }
constexpr Bitboard  operator^(Bitboard b, Square s) { return b ^ square_bb(s); }
constexpr Bitboard& operator|=(Bitboard& b, Square s) { return b |= square_bb(s); }
constexpr Bitboard& operator^=(Bitboard& b, Square s) { return b ^= square_bb(s); }

constexpr Bitboard operator&(Square s, Bitboard b) { return b & s; }
constexpr Bitboard operator|(Square s, Bitboard b) { return b | s; }
constexpr Bitboard operator^(Square s, Bitboard b) { return b ^ s; }

constexpr Bitboard operator|(Square s1, Square s2) { return square_bb(s1) | s2; }

constexpr bool more_than_one(Bitboard b) { return b & (b - 1); }
//for classical begin
constexpr bool opposite_colors(Square s1, Square s2) {
    return (s1 + rank_of(s1) + s2 + rank_of(s2)) & 1;
}
//for classical end
// rank_bb() and file_bb() return a bitboard representing all the squares on
// the given file or rank.

constexpr Bitboard rank_bb(Rank r) { return Rank1BB << (8 * r); }

constexpr Bitboard rank_bb(Square s) { return rank_bb(rank_of(s)); }

constexpr Bitboard file_bb(File f) { return FileABB << f; }

constexpr Bitboard file_bb(Square s) { return file_bb(file_of(s)); }


// Moves a bitboard one or two steps as specified by the direction D
template<Direction D>
constexpr Bitboard shift(Bitboard b) {
    return D == NORTH         ? b << 8
         : D == SOUTH         ? b >> 8
         : D == NORTH + NORTH ? b << 16
         : D == SOUTH + SOUTH ? b >> 16
         : D == EAST          ? (b & ~FileHBB) << 1
         : D == WEST          ? (b & ~FileABB) >> 1
         : D == NORTH_EAST    ? (b & ~FileHBB) << 9
         : D == NORTH_WEST    ? (b & ~FileABB) << 7
         : D == SOUTH_EAST    ? (b & ~FileHBB) >> 7
         : D == SOUTH_WEST    ? (b & ~FileABB) >> 9
                              : 0;
}


// Returns the squares attacked by pawns of the given color
// from the squares in the given bitboard.
template<Color C>
constexpr Bitboard pawn_attacks_bb(Bitboard b) {
    return C == WHITE ? shift<NORTH_WEST>(b) | shift<NORTH_EAST>(b)
                      : shift<SOUTH_WEST>(b) | shift<SOUTH_EAST>(b);
}

//for classical begin
/// pawn_double_attacks_bb() returns the squares doubly attacked by pawns of the
/// given color from the squares in the given bitboard.

template<Color C>
constexpr Bitboard pawn_double_attacks_bb(Bitboard b) {
    return C == WHITE ? shift<NORTH_WEST>(b) & shift<NORTH_EAST>(b)
                      : shift<SOUTH_WEST>(b) & shift<SOUTH_EAST>(b);
}


/// adjacent_files_bb() returns a bitboard representing all the squares on the
/// adjacent files of a given square.

constexpr Bitboard adjacent_files_bb(Square s) {
    return shift<EAST>(file_bb(s)) | shift<WEST>(file_bb(s));
}
//for classical end
// Returns a bitboard representing an entire line (from board edge
// to board edge) that intersects the two given squares. If the given squares
// are not on a same file/rank/diagonal, the function returns 0. For instance,
// line_bb(SQ_C4, SQ_F7) will return a bitboard with the A2-G8 diagonal.
inline Bitboard line_bb(Square s1, Square s2) {

    assert(is_ok(s1) && is_ok(s2));
    return LineBB[s1][s2];
}


// Returns a bitboard representing the squares in the semi-open
// segment between the squares s1 and s2 (excluding s1 but including s2). If the
// given squares are not on a same file/rank/diagonal, it returns s2. For instance,
// between_bb(SQ_C4, SQ_F7) will return a bitboard with squares D5, E6 and F7, but
// between_bb(SQ_E6, SQ_F8) will return a bitboard with the square F8. This trick
// allows to generate non-king evasion moves faster: the defending piece must either
// interpose itself to cover the check or capture the checking piece.
inline Bitboard between_bb(Square s1, Square s2) {

    assert(is_ok(s1) && is_ok(s2));
    return BetweenBB[s1][s2];
}
//for classical begin
/// forward_ranks_bb() returns a bitboard representing the squares on the ranks in
/// front of the given one, from the point of view of the given color. For instance,
/// forward_ranks_bb(BLACK, SQ_D3) will return the 16 squares on ranks 1 and 2.

constexpr Bitboard forward_ranks_bb(Color c, Square s) {
    return c == WHITE ? ~Rank1BB << 8 * relative_rank(WHITE, s)
                      : ~Rank8BB >> 8 * relative_rank(BLACK, s);
}
/// forward_file_bb() returns a bitboard representing all the squares along the
/// line in front of the given one, from the point of view of the given color.

constexpr Bitboard forward_file_bb(Color c, Square s) {
    return forward_ranks_bb(c, s) & file_bb(s);
}


/// pawn_attack_span() returns a bitboard representing all the squares that can
/// be attacked by a pawn of the given color when it moves along its file, starting
/// from the given square.

constexpr Bitboard pawn_attack_span(Color c, Square s) {
    return forward_ranks_bb(c, s) & adjacent_files_bb(s);
}


/// passed_pawn_span() returns a bitboard which can be used to test if a pawn of
/// the given color and on the given square is a passed pawn.

constexpr Bitboard passed_pawn_span(Color c, Square s) {
    return pawn_attack_span(c, s) | forward_file_bb(c, s);
}
//for classical end


// distance() functions return the distance between x and y, defined as the
// number of steps for a king in x to reach y.

template<typename T1 = Square>
inline int distance(Square x, Square y);

template<>
inline int distance<File>(Square x, Square y) {
    return std::abs(file_of(x) - file_of(y));
}

template<>
inline int distance<Rank>(Square x, Square y) {
    return std::abs(rank_of(x) - rank_of(y));
}

template<>
inline int distance<Square>(Square x, Square y) {
    return SquareDistance[x][y];
}

inline int edge_distance(File f) { return std::min(f, File(FILE_H - f)); }
inline int edge_distance(Rank r) { return std::min(r, Rank(RANK_8 - r)); }  //for classical


constexpr int constexpr_popcount(Bitboard b) {
    b = b - ((b >> 1) & 0x5555555555555555ULL);
    b = (b & 0x3333333333333333ULL) + ((b >> 2) & 0x3333333333333333ULL);
    b = (b + (b >> 4)) & 0x0F0F0F0F0F0F0F0FULL;
    return static_cast<int>((b * 0x0101010101010101ULL) >> 56);
}


// Counts the number of non-zero bits in a bitboard.
inline int popcount(Bitboard b) {

#ifndef USE_POPCNT

    std::uint16_t indices[4];
    std::memcpy(indices, &b, sizeof(b));
    return PopCnt16[indices[0]] + PopCnt16[indices[1]] + PopCnt16[indices[2]]
         + PopCnt16[indices[3]];

#elif defined(_MSC_VER)

    return int(_mm_popcnt_u64(b));

#else  // Assumed gcc or compatible compiler

    return __builtin_popcountll(b);

#endif
}

// Returns the least significant bit in a non-zero bitboard.
inline Square lsb(Bitboard b) {
    assert(b);

#if defined(__GNUC__)  // GCC, Clang, ICX

    return Square(__builtin_ctzll(b));

#elif defined(_MSC_VER)
    #ifdef _WIN64  // MSVC, WIN64

    unsigned long idx;
    _BitScanForward64(&idx, b);
    return Square(idx);

    #else  // MSVC, WIN32
    unsigned long idx;

    if (b & 0xffffffff)
    {
        _BitScanForward(&idx, int32_t(b));
        return Square(idx);
    }
    else
    {
        _BitScanForward(&idx, int32_t(b >> 32));
        return Square(idx + 32);
    }
    #endif
#else  // Compiler is neither GCC nor MSVC compatible
    #error "Compiler not supported."
#endif
}

// Returns the most significant bit in a non-zero bitboard.
inline Square msb(Bitboard b) {
    assert(b);

#if defined(__GNUC__)  // GCC, Clang, ICX

    return Square(63 ^ __builtin_clzll(b));

#elif defined(_MSC_VER)
    #ifdef _WIN64  // MSVC, WIN64

    unsigned long idx;
    _BitScanReverse64(&idx, b);
    return Square(idx);

    #else  // MSVC, WIN32

    unsigned long idx;

    if (b >> 32)
    {
        _BitScanReverse(&idx, int32_t(b >> 32));
        return Square(idx + 32);
    }
    else
    {
        _BitScanReverse(&idx, int32_t(b));
        return Square(idx);
    }
    #endif
#else  // Compiler is neither GCC nor MSVC compatible
    #error "Compiler not supported."
#endif
}

// Returns the bitboard of the least significant
// square of a non-zero bitboard. It is equivalent to square_bb(lsb(bb)).
inline Bitboard least_significant_square_bb(Bitboard b) {
    assert(b);
    return b & -b;
}

// Finds and clears the least significant bit in a non-zero bitboard.
inline Square pop_lsb(Bitboard& b) {
    assert(b);
    const Square s = lsb(b);
    b &= b - 1;
    return s;
}

namespace Bitboards {
// Returns the bitboard of target square for the given step
// from the given square. If the step is off the board, returns empty bitboard.
constexpr Bitboard safe_destination(Square s, int step) {
    constexpr auto abs = [](int v) { return v < 0 ? -v : v; };
    Square         to  = Square(s + step);
    return is_ok(to) && abs(file_of(s) - file_of(to)) <= 2 ? square_bb(to) : Bitboard(0);
}

constexpr Bitboard sliding_attack(PieceType pt, Square sq, Bitboard occupied) {
    Bitboard  attacks             = 0;
    Direction RookDirections[4]   = {NORTH, SOUTH, EAST, WEST};
    Direction BishopDirections[4] = {NORTH_EAST, SOUTH_EAST, SOUTH_WEST, NORTH_WEST};

    for (Direction d : (pt == ROOK ? RookDirections : BishopDirections))
    {
        Square s = sq;
        while (safe_destination(s, d))
        {
            attacks |= (s += d);
            if (occupied & s)
            {
                break;
            }
        }
    }

    return attacks;
}

constexpr Bitboard knight_attack(Square sq) {
    Bitboard b = {};
    for (int step : {-17, -15, -10, -6, 6, 10, 15, 17})
        b |= safe_destination(sq, step);
    return b;
}

constexpr Bitboard king_attack(Square sq) {
    Bitboard b = {};
    for (int step : {-9, -8, -7, -1, 1, 7, 8, 9})
        b |= safe_destination(sq, step);
    return b;
}

constexpr Bitboard pseudo_attacks(PieceType pt, Square sq) {
    switch (pt)
    {
    case PieceType::ROOK :
    case PieceType::BISHOP :
        return sliding_attack(pt, sq, 0);
    case PieceType::QUEEN :
        return sliding_attack(PieceType::ROOK, sq, 0) | sliding_attack(PieceType::BISHOP, sq, 0);
    case PieceType::KNIGHT :
        return knight_attack(sq);
    case PieceType::KING :
        return king_attack(sq);
    default :
        assert(false);
        return 0;
    }
}

}

inline constexpr auto PseudoAttacks = []() constexpr {
    std::array<std::array<Bitboard, SQUARE_NB>, PIECE_TYPE_NB> attacks{};

    for (Square s1 = SQ_A1; s1 <= SQ_H8; ++s1)
    {
        attacks[WHITE][s1] = pawn_attacks_bb<WHITE>(square_bb(s1));
        attacks[BLACK][s1] = pawn_attacks_bb<BLACK>(square_bb(s1));

        attacks[KING][s1]   = Bitboards::pseudo_attacks(KING, s1);
        attacks[KNIGHT][s1] = Bitboards::pseudo_attacks(KNIGHT, s1);
        attacks[QUEEN][s1] = attacks[BISHOP][s1] = Bitboards::pseudo_attacks(BISHOP, s1);
        attacks[QUEEN][s1] |= attacks[ROOK][s1]  = Bitboards::pseudo_attacks(ROOK, s1);
    }

    return attacks;
}();


// Returns the pseudo attacks of the given piece type
// assuming an empty board.
template<PieceType Pt>
inline Bitboard attacks_bb(Square s, Color c = COLOR_NB) {

    assert((Pt != PAWN || c < COLOR_NB) && (is_ok(s)));
    return Pt == PAWN ? PseudoAttacks[c][s] : PseudoAttacks[Pt][s];
}


// Returns the attacks by the given piece
// assuming the board is occupied according to the passed Bitboard.
// Sliding piece attacks do not continue passed an occupied square.
template<PieceType Pt>
inline Bitboard attacks_bb(Square s, Bitboard occupied) {

    assert((Pt != PAWN) && (is_ok(s)));

    switch (Pt)
    {
    case BISHOP :
    case ROOK :
        return Magics[s][Pt - BISHOP].attacks_bb(occupied);
    case QUEEN :
        return attacks_bb<BISHOP>(s, occupied) | attacks_bb<ROOK>(s, occupied);
    default :
        return PseudoAttacks[Pt][s];
    }
}

// Returns the attacks by the given piece
// assuming the board is occupied according to the passed Bitboard.
// Sliding piece attacks do not continue passed an occupied square.
inline Bitboard attacks_bb(PieceType pt, Square s, Bitboard occupied) {

    assert((pt != PAWN) && (is_ok(s)));

    switch (pt)
    {
    case BISHOP :
        return attacks_bb<BISHOP>(s, occupied);
    case ROOK :
        return attacks_bb<ROOK>(s, occupied);
    case QUEEN :
        return attacks_bb<BISHOP>(s, occupied) | attacks_bb<ROOK>(s, occupied);
    default :
        return PseudoAttacks[pt][s];
    }
}

inline Bitboard attacks_bb(Piece pc, Square s) {
    if (type_of(pc) == PAWN)
        return PseudoAttacks[color_of(pc)][s];

    return PseudoAttacks[type_of(pc)][s];
}


inline Bitboard attacks_bb(Piece pc, Square s, Bitboard occupied) {
    if (type_of(pc) == PAWN)
        return PseudoAttacks[color_of(pc)][s];

    return attacks_bb(type_of(pc), s, occupied);
}

//for classical begin
/// frontmost_sq() returns the most advanced square for the given color,
/// requires a non-zero bitboard.
inline Square frontmost_sq(Color c, Bitboard b) {
    assert(b);
    return c == WHITE ? msb(b) : lsb(b);
}
//for classical end
}  // namespace Alexander

#endif  // #ifndef BITBOARD_H_INCLUDED


// ============================================================================
// FILE ORIGINALE: endgame.cpp
// ============================================================================

/*
  Alexander, a UCI chess playing engine derived from Stockfish
  Copyright (C) 2004-2023 The Stockfish developers (see AUTHORS file)

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

#include <cassert>

#include "bitboard.h"
#include "endgame.h"
#include "movegen.h"

namespace Alexander {

namespace {

// Used to drive the king towards the edge of the board
// in KX vs K and KQ vs KR endgames.
// Values range from 27 (center squares) to 90 (in the corners)
inline int push_to_edge(Square s) {
    int rd = edge_distance(rank_of(s)), fd = edge_distance(file_of(s));
    return 90 - (7 * fd * fd / 2 + 7 * rd * rd / 2);
}

// Used to drive the king towards A1H8 corners in KBN vs K endgames.
// Values range from 0 on A8H1 diagonal to 7 in A1H8 corners
inline int push_to_corner(Square s) { return abs(7 - rank_of(s) - file_of(s)); }

// Drive a piece close to or away from another piece
inline int push_close(Square s1, Square s2) { return 140 - 20 * distance(s1, s2); }
inline int push_away(Square s1, Square s2) { return 120 - push_close(s1, s2); }

#ifndef NDEBUG
bool verify_material(const Position& pos, Color c, Value npm, int pawnsCnt) {
    return pos.non_pawn_material(c) == npm && pos.count<PAWN>(c) == pawnsCnt;
}
#endif

// Map the square as if strongSide is white and strongSide's only pawn
// is on the left half of the board.
Square normalize(const Position& pos, Color strongSide, Square sq) {

    assert(pos.count<PAWN>(strongSide) == 1);

    if (file_of(pos.square<PAWN>(strongSide)) >= FILE_E)
        sq = flip_file(sq);

    return strongSide == WHITE ? sq : flip_rank(sq);
}

}  // namespace


namespace Endgames {

std::pair<Map<Value>, Map<ScaleFactor>> maps;

void init() {

    add<KPK>("KPK");
    add<KNNK>("KNNK");
    add<KBNK>("KBNK");
    add<KRKP>("KRKP");
    add<KRKB>("KRKB");
    add<KRKN>("KRKN");
    add<KQKP>("KQKP");
    add<KQKR>("KQKR");
    add<KNNKP>("KNNKP");

    add<KRPKR>("KRPKR");
    add<KRPKB>("KRPKB");
    add<KBPKB>("KBPKB");
    add<KBPKN>("KBPKN");
    add<KBPPKB>("KBPPKB");
    add<KRPPKRP>("KRPPKRP");
}
}


/// Mate with KX vs K. This function is used to evaluate positions with
/// king and plenty of material vs a lone king. It simply gives the
/// attacking side a bonus for driving the defending king towards the edge
/// of the board, and for keeping the distance between the two kings small.
template<>
Value Endgame<KXK>::operator()(const Position& pos) const {

    assert(verify_material(pos, weakSide, VALUE_ZERO, 0));
    assert(!pos.checkers());  // Eval is never called when in check

    // Stalemate detection with lone king
    if (pos.side_to_move() == weakSide && !MoveList<LEGAL>(pos).size())
        return VALUE_DRAW;

    Square strongKing = pos.square<KING>(strongSide);
    Square weakKing   = pos.square<KING>(weakSide);

    Value result = pos.non_pawn_material(strongSide) + pos.count<PAWN>(strongSide) * PawnValueEg
                 + push_to_edge(weakKing) + push_close(strongKing, weakKing);

    if (pos.count<QUEEN>(strongSide) || pos.count<ROOK>(strongSide)
        || (pos.count<BISHOP>(strongSide) && pos.count<KNIGHT>(strongSide))
        || ((pos.pieces(strongSide, BISHOP) & ~DarkSquares)
            && (pos.pieces(strongSide, BISHOP) & DarkSquares)))
        result = std::min(result + VALUE_KNOWN_WIN, VALUE_TB_WIN_IN_MAX_PLY - 1);

    return strongSide == pos.side_to_move() ? result : -result;
}


/// Mate with KBN vs K. This is similar to KX vs K, but we have to drive the
/// defending king towards a corner square that our bishop attacks.
template<>
Value Endgame<KBNK>::operator()(const Position& pos) const {

    assert(verify_material(pos, strongSide, KnightValueMg + BishopValueMg, 0));
    assert(verify_material(pos, weakSide, VALUE_ZERO, 0));

    Square strongKing   = pos.square<KING>(strongSide);
    Square strongBishop = pos.square<BISHOP>(strongSide);
    Square weakKing     = pos.square<KING>(weakSide);

    // If our bishop does not attack A1/H8, we flip the enemy king square
    // to drive to opposite corners (A8/H1).

    Value result =
      (VALUE_KNOWN_WIN + 3520) + push_close(strongKing, weakKing)
      + 420 * push_to_corner(opposite_colors(strongBishop, SQ_A1) ? flip_file(weakKing) : weakKing);

    assert(abs(result) < VALUE_TB_WIN_IN_MAX_PLY);
    return strongSide == pos.side_to_move() ? result : -result;
}


/// KP vs K. This endgame is evaluated with the help of a bitbase
template<>
Value Endgame<KPK>::operator()(const Position& pos) const {

    assert(verify_material(pos, strongSide, VALUE_ZERO, 1));
    assert(verify_material(pos, weakSide, VALUE_ZERO, 0));

    // Assume strongSide is white and the pawn is on files A-D
    Square strongKing = normalize(pos, strongSide, pos.square<KING>(strongSide));
    Square strongPawn = normalize(pos, strongSide, pos.square<PAWN>(strongSide));
    Square weakKing   = normalize(pos, strongSide, pos.square<KING>(weakSide));

    Color us = strongSide == pos.side_to_move() ? WHITE : BLACK;

    if (!Bitbases::probe(strongKing, strongPawn, weakKing, us))
        return VALUE_DRAW;

    Value result = VALUE_KNOWN_WIN + PawnValueEg + Value(rank_of(strongPawn));

    return strongSide == pos.side_to_move() ? result : -result;
}


/// KR vs KP. This is a somewhat tricky endgame to evaluate precisely without
/// a bitbase. The function below returns drawish scores when the pawn is
/// far advanced with support of the king, while the attacking king is far
/// away.
template<>
Value Endgame<KRKP>::operator()(const Position& pos) const {

    assert(verify_material(pos, strongSide, RookValueMg, 0));
    assert(verify_material(pos, weakSide, VALUE_ZERO, 1));

    Square strongKing     = pos.square<KING>(strongSide);
    Square weakKing       = pos.square<KING>(weakSide);
    Square strongRook     = pos.square<ROOK>(strongSide);
    Square weakPawn       = pos.square<PAWN>(weakSide);
    Square queeningSquare = make_square(file_of(weakPawn), relative_rank(weakSide, RANK_8));
    Value  result;

    // If the stronger side's king is in front of the pawn, it's a win
    if (forward_file_bb(strongSide, strongKing) & weakPawn)
        result = RookValueEg - distance(strongKing, weakPawn);

    // If the weaker side's king is too far from the pawn and the rook,
    // it's a win.
    else if (distance(weakKing, weakPawn) >= 3 + (pos.side_to_move() == weakSide)
             && distance(weakKing, strongRook) >= 3)
        result = RookValueEg - distance(strongKing, weakPawn);

    // If the pawn is far advanced and supported by the defending king,
    // the position is drawish
    else if (relative_rank(strongSide, weakKing) <= RANK_3 && distance(weakKing, weakPawn) == 1
             && relative_rank(strongSide, strongKing) >= RANK_4
             && distance(strongKing, weakPawn) > 2 + (pos.side_to_move() == strongSide))
        result = Value(80) - 8 * distance(strongKing, weakPawn);

    else
        result = Value(200)
               - 8
                   * (distance(strongKing, weakPawn + pawn_push(weakSide))
                      - distance(weakKing, weakPawn + pawn_push(weakSide))
                      - distance(weakPawn, queeningSquare));

    return strongSide == pos.side_to_move() ? result : -result;
}


/// KR vs KB. This is very simple, and always returns drawish scores. The
/// score is slightly bigger when the defending king is close to the edge.
template<>
Value Endgame<KRKB>::operator()(const Position& pos) const {

    assert(verify_material(pos, strongSide, RookValueMg, 0));
    assert(verify_material(pos, weakSide, BishopValueMg, 0));

    Value result = Value(push_to_edge(pos.square<KING>(weakSide)));
    return strongSide == pos.side_to_move() ? result : -result;
}


/// KR vs KN. The attacking side has slightly better winning chances than
/// in KR vs KB, particularly if the king and the knight are far apart.
template<>
Value Endgame<KRKN>::operator()(const Position& pos) const {

    assert(verify_material(pos, strongSide, RookValueMg, 0));
    assert(verify_material(pos, weakSide, KnightValueMg, 0));

    Square weakKing   = pos.square<KING>(weakSide);
    Square weakKnight = pos.square<KNIGHT>(weakSide);
    Value  result     = Value(push_to_edge(weakKing) + push_away(weakKing, weakKnight));
    return strongSide == pos.side_to_move() ? result : -result;
}


/// KQ vs KP. In general, this is a win for the stronger side, but there are a
/// few important exceptions. A pawn on 7th rank and on the A,C,F or H files
/// with a king positioned next to it can be a draw, so in that case, we only
/// use the distance between the kings.
template<>
Value Endgame<KQKP>::operator()(const Position& pos) const {

    assert(verify_material(pos, strongSide, QueenValueMg, 0));
    assert(verify_material(pos, weakSide, VALUE_ZERO, 1));

    Square strongKing = pos.square<KING>(strongSide);
    Square weakKing   = pos.square<KING>(weakSide);
    Square weakPawn   = pos.square<PAWN>(weakSide);

    Value result = Value(push_close(strongKing, weakKing));

    if (relative_rank(weakSide, weakPawn) != RANK_7 || distance(weakKing, weakPawn) != 1
        || ((FileBBB | FileDBB | FileEBB | FileGBB) & weakPawn))
        result += QueenValueEg - PawnValueEg;

    return strongSide == pos.side_to_move() ? result : -result;
}


/// KQ vs KR. This is almost identical to KX vs K: we give the attacking
/// king a bonus for having the kings close together, and for forcing the
/// defending king towards the edge. If we also take care to avoid null move for
/// the defending side in the search, this is usually sufficient to win KQ vs KR.
template<>
Value Endgame<KQKR>::operator()(const Position& pos) const {

    assert(verify_material(pos, strongSide, QueenValueMg, 0));
    assert(verify_material(pos, weakSide, RookValueMg, 0));

    Square strongKing = pos.square<KING>(strongSide);
    Square weakKing   = pos.square<KING>(weakSide);

    Value result =
      QueenValueEg - RookValueEg + push_to_edge(weakKing) + push_close(strongKing, weakKing);

    return strongSide == pos.side_to_move() ? result : -result;
}


/// KNN vs KP. Very drawish, but there are some mate opportunities if we can
/// press the weakSide King to a corner before the pawn advances too much.
template<>
Value Endgame<KNNKP>::operator()(const Position& pos) const {

    assert(verify_material(pos, strongSide, 2 * KnightValueMg, 0));
    assert(verify_material(pos, weakSide, VALUE_ZERO, 1));

    Square weakKing = pos.square<KING>(weakSide);
    Square weakPawn = pos.square<PAWN>(weakSide);

    Value result =
      PawnValueEg + 2 * push_to_edge(weakKing) - 10 * relative_rank(weakSide, weakPawn);

    return strongSide == pos.side_to_move() ? result : -result;
}


/// Some cases of trivial draws
template<>
Value Endgame<KNNK>::operator()(const Position&) const {
    return VALUE_DRAW;
}


/// KB and one or more pawns vs K. It checks for draws with rook pawns and
/// a bishop of the wrong color. If such a draw is detected, SCALE_FACTOR_DRAW
/// is returned. If not, the return value is SCALE_FACTOR_NONE, i.e. no scaling
/// will be used.
template<>
ScaleFactor Endgame<KBPsK>::operator()(const Position& pos) const {

    assert(pos.non_pawn_material(strongSide) == BishopValueMg);
    assert(pos.count<PAWN>(strongSide) >= 1);

    // No assertions about the material of weakSide, because we want draws to
    // be detected even when the weaker side has some pawns.

    Bitboard strongPawns = pos.pieces(strongSide, PAWN);
    Bitboard allPawns    = pos.pieces(PAWN);

    Square strongBishop = pos.square<BISHOP>(strongSide);
    Square weakKing     = pos.square<KING>(weakSide);
    Square strongKing   = pos.square<KING>(strongSide);

    // All strongSide pawns are on a single rook file?
    if (!(strongPawns & ~FileABB) || !(strongPawns & ~FileHBB))
    {
        Square queeningSquare =
          relative_square(strongSide, make_square(file_of(lsb(strongPawns)), RANK_8));

        if (opposite_colors(queeningSquare, strongBishop)
            && distance(queeningSquare, weakKing) <= 1)
            return SCALE_FACTOR_DRAW;
    }

    // If all the pawns are on the same B or G file, then it's potentially a draw
    if ((!(allPawns & ~FileBBB) || !(allPawns & ~FileGBB)) && pos.non_pawn_material(weakSide) == 0
        && pos.count<PAWN>(weakSide) >= 1)
    {
        // Get the least advanced weakSide pawn
        Square weakPawn = frontmost_sq(strongSide, pos.pieces(weakSide, PAWN));

        // There's potential for a draw if our pawn is blocked on the 7th rank,
        // the bishop cannot attack it or they only have one pawn left.
        if (relative_rank(strongSide, weakPawn) == RANK_7
            && (strongPawns & (weakPawn + pawn_push(weakSide)))
            && (opposite_colors(strongBishop, weakPawn) || !more_than_one(strongPawns)))
        {
            int strongKingDist = distance(weakPawn, strongKing);
            int weakKingDist   = distance(weakPawn, weakKing);

            // It's a draw if the weak king is on its back two ranks, within 2
            // squares of the blocking pawn and the strong king is not
            // closer. (I think this rule only fails in practically
            // unreachable positions such as 5k1K/6p1/6P1/8/8/3B4/8/8 w
            // and positions where qsearch will immediately correct the
            // problem such as 8/4k1p1/6P1/1K6/3B4/8/8/8 w).
            if (relative_rank(strongSide, weakKing) >= RANK_7 && weakKingDist <= 2
                && weakKingDist <= strongKingDist)
                return SCALE_FACTOR_DRAW;
        }
    }

    return SCALE_FACTOR_NONE;
}


/// KQ vs KR and one or more pawns. It tests for fortress draws with a rook on
/// the third rank defended by a pawn.
template<>
ScaleFactor Endgame<KQKRPs>::operator()(const Position& pos) const {

    assert(verify_material(pos, strongSide, QueenValueMg, 0));
    assert(pos.count<ROOK>(weakSide) == 1);
    assert(pos.count<PAWN>(weakSide) >= 1);

    Square strongKing = pos.square<KING>(strongSide);
    Square weakKing   = pos.square<KING>(weakSide);
    Square weakRook   = pos.square<ROOK>(weakSide);

    if (relative_rank(weakSide, weakKing) <= RANK_2 && relative_rank(weakSide, strongKing) >= RANK_4
        && relative_rank(weakSide, weakRook) == RANK_3
        && (pos.pieces(weakSide, PAWN) & attacks_bb<KING>(weakKing)
            & attacks_bb<PAWN>(weakRook, strongSide)))
        return SCALE_FACTOR_DRAW;

    return SCALE_FACTOR_NONE;
}


/// KRP vs KR. This function knows a handful of the most important classes of
/// drawn positions, but is far from perfect. It would probably be a good idea
/// to add more knowledge in the future.
///
/// It would also be nice to rewrite the actual code for this function,
/// which is mostly copied from Glaurung 1.x, and isn't very pretty.
template<>
ScaleFactor Endgame<KRPKR>::operator()(const Position& pos) const {

    assert(verify_material(pos, strongSide, RookValueMg, 1));
    assert(verify_material(pos, weakSide, RookValueMg, 0));

    // Assume strongSide is white and the pawn is on files A-D
    Square strongKing = normalize(pos, strongSide, pos.square<KING>(strongSide));
    Square strongRook = normalize(pos, strongSide, pos.square<ROOK>(strongSide));
    Square strongPawn = normalize(pos, strongSide, pos.square<PAWN>(strongSide));
    Square weakKing   = normalize(pos, strongSide, pos.square<KING>(weakSide));
    Square weakRook   = normalize(pos, strongSide, pos.square<ROOK>(weakSide));

    File   pawnFile       = file_of(strongPawn);
    Rank   pawnRank       = rank_of(strongPawn);
    Square queeningSquare = make_square(pawnFile, RANK_8);
    int    tempo          = (pos.side_to_move() == strongSide);

    // If the pawn is not too far advanced and the defending king defends the
    // queening square, use the third-rank defence.
    if (pawnRank <= RANK_5 && distance(weakKing, queeningSquare) <= 1 && strongKing <= SQ_H5
        && (rank_of(weakRook) == RANK_6 || (pawnRank <= RANK_3 && rank_of(strongRook) != RANK_6)))
        return SCALE_FACTOR_DRAW;

    // The defending side saves a draw by checking from behind in case the pawn
    // has advanced to the 6th rank with the king behind.
    if (pawnRank == RANK_6 && distance(weakKing, queeningSquare) <= 1
        && rank_of(strongKing) + tempo <= RANK_6
        && (rank_of(weakRook) == RANK_1 || (!tempo && distance<File>(weakRook, strongPawn) >= 3)))
        return SCALE_FACTOR_DRAW;

    if (pawnRank >= RANK_6 && weakKing == queeningSquare && rank_of(weakRook) == RANK_1
        && (!tempo || distance(strongKing, strongPawn) >= 2))
        return SCALE_FACTOR_DRAW;

    // White pawn on a7 and rook on a8 is a draw if black's king is on g7 or h7
    // and the black rook is behind the pawn.
    if (strongPawn == SQ_A7 && strongRook == SQ_A8 && (weakKing == SQ_H7 || weakKing == SQ_G7)
        && file_of(weakRook) == FILE_A
        && (rank_of(weakRook) <= RANK_3 || file_of(strongKing) >= FILE_D
            || rank_of(strongKing) <= RANK_5))
        return SCALE_FACTOR_DRAW;

    // If the defending king blocks the pawn and the attacking king is too far
    // away, it's a draw.
    if (pawnRank <= RANK_5 && weakKing == strongPawn + NORTH
        && distance(strongKing, strongPawn) - tempo >= 2
        && distance(strongKing, weakRook) - tempo >= 2)
        return SCALE_FACTOR_DRAW;

    // Pawn on the 7th rank supported by the rook from behind usually wins if the
    // attacking king is closer to the queening square than the defending king,
    // and the defending king cannot gain tempi by threatening the attacking rook.
    if (pawnRank == RANK_7 && pawnFile != FILE_A && file_of(strongRook) == pawnFile
        && strongRook != queeningSquare
        && (distance(strongKing, queeningSquare) < distance(weakKing, queeningSquare) - 2 + tempo)
        && (distance(strongKing, queeningSquare) < distance(weakKing, strongRook) + tempo))
        return ScaleFactor(SCALE_FACTOR_MAX - 2 * distance(strongKing, queeningSquare));

    // Similar to the above, but with the pawn further back
    if (pawnFile != FILE_A && file_of(strongRook) == pawnFile && strongRook < strongPawn
        && (distance(strongKing, queeningSquare) < distance(weakKing, queeningSquare) - 2 + tempo)
        && (distance(strongKing, strongPawn + NORTH)
            < distance(weakKing, strongPawn + NORTH) - 2 + tempo)
        && (distance(weakKing, strongRook) + tempo >= 3
            || (distance(strongKing, queeningSquare) < distance(weakKing, strongRook) + tempo
                && (distance(strongKing, strongPawn + NORTH)
                    < distance(weakKing, strongPawn) + tempo))))
        return ScaleFactor(SCALE_FACTOR_MAX - 8 * distance(strongPawn, queeningSquare)
                           - 2 * distance(strongKing, queeningSquare));

    // If the pawn is not far advanced and the defending king is somewhere in
    // the pawn's path, it's probably a draw.
    if (pawnRank <= RANK_4 && weakKing > strongPawn)
    {
        if (file_of(weakKing) == file_of(strongPawn))
            return ScaleFactor(10);
        if (distance<File>(weakKing, strongPawn) == 1 && distance(strongKing, weakKing) > 2)
            return ScaleFactor(24 - 2 * distance(strongKing, weakKing));
    }
    return SCALE_FACTOR_NONE;
}

template<>
ScaleFactor Endgame<KRPKB>::operator()(const Position& pos) const {

    assert(verify_material(pos, strongSide, RookValueMg, 1));
    assert(verify_material(pos, weakSide, BishopValueMg, 0));

    // Test for a rook pawn
    if (pos.pieces(PAWN) & (FileABB | FileHBB))
    {
        Square    weakKing   = pos.square<KING>(weakSide);
        Square    weakBishop = pos.square<BISHOP>(weakSide);
        Square    strongKing = pos.square<KING>(strongSide);
        Square    strongPawn = pos.square<PAWN>(strongSide);
        Rank      pawnRank   = relative_rank(strongSide, strongPawn);
        Direction push       = pawn_push(strongSide);

        // If the pawn is on the 5th rank and the pawn (currently) is on
        // the same color square as the bishop then there is a chance of
        // a fortress. Depending on the king position give a moderate
        // reduction or a stronger one if the defending king is near the
        // corner but not trapped there.
        if (pawnRank == RANK_5 && !opposite_colors(weakBishop, strongPawn))
        {
            int d = distance(strongPawn + 3 * push, weakKing);

            if (d <= 2 && !(d == 0 && weakKing == strongKing + 2 * push))
                return ScaleFactor(24);
            else
                return ScaleFactor(48);
        }

        // When the pawn has moved to the 6th rank we can be fairly sure
        // it's drawn if the bishop attacks the square in front of the
        // pawn from a reasonable distance and the defending king is near
        // the corner
        if (pawnRank == RANK_6 && distance(strongPawn + 2 * push, weakKing) <= 1
            && (attacks_bb<BISHOP>(weakBishop) & (strongPawn + push))
            && distance<File>(weakBishop, strongPawn) >= 2)
            return ScaleFactor(8);
    }

    return SCALE_FACTOR_NONE;
}

/// KRPP vs KRP. There is just a single rule: if the stronger side has no passed
/// pawns and the defending king is actively placed, the position is drawish.
template<>
ScaleFactor Endgame<KRPPKRP>::operator()(const Position& pos) const {

    assert(verify_material(pos, strongSide, RookValueMg, 2));
    assert(verify_material(pos, weakSide, RookValueMg, 1));

    Square strongPawn1 = lsb(pos.pieces(strongSide, PAWN));
    Square strongPawn2 = msb(pos.pieces(strongSide, PAWN));
    Square weakKing    = pos.square<KING>(weakSide);

    // Does the stronger side have a passed pawn?
    if (pos.pawn_passed(strongSide, strongPawn1) || pos.pawn_passed(strongSide, strongPawn2))
        return SCALE_FACTOR_NONE;

    Rank pawnRank =
      std::max(relative_rank(strongSide, strongPawn1), relative_rank(strongSide, strongPawn2));

    if (distance<File>(weakKing, strongPawn1) <= 1 && distance<File>(weakKing, strongPawn2) <= 1
        && relative_rank(strongSide, weakKing) > pawnRank)
    {
        assert(pawnRank > RANK_1 && pawnRank < RANK_7);
        return ScaleFactor(7 * pawnRank);
    }
    return SCALE_FACTOR_NONE;
}


/// K and two or more pawns vs K. There is just a single rule here: if all pawns
/// are on the same rook file and are blocked by the defending king, it's a draw.
template<>
ScaleFactor Endgame<KPsK>::operator()(const Position& pos) const {

    assert(pos.non_pawn_material(strongSide) == VALUE_ZERO);
    assert(pos.count<PAWN>(strongSide) >= 2);
    assert(verify_material(pos, weakSide, VALUE_ZERO, 0));

    Square   weakKing    = pos.square<KING>(weakSide);
    Bitboard strongPawns = pos.pieces(strongSide, PAWN);

    // If all pawns are ahead of the king on a single rook file, it's a draw.
    if (!(strongPawns & ~(FileABB | FileHBB))
        && !(strongPawns & ~passed_pawn_span(weakSide, weakKing)))
        return SCALE_FACTOR_DRAW;

    return SCALE_FACTOR_NONE;
}


/// KBP vs KB. There are two rules: if the defending king is somewhere along the
/// path of the pawn, and the square of the king is not of the same color as the
/// stronger side's bishop, it's a draw. If the two bishops have opposite color,
/// it's almost always a draw.
template<>
ScaleFactor Endgame<KBPKB>::operator()(const Position& pos) const {

    assert(verify_material(pos, strongSide, BishopValueMg, 1));
    assert(verify_material(pos, weakSide, BishopValueMg, 0));

    Square strongPawn   = pos.square<PAWN>(strongSide);
    Square strongBishop = pos.square<BISHOP>(strongSide);
    Square weakBishop   = pos.square<BISHOP>(weakSide);
    Square weakKing     = pos.square<KING>(weakSide);

    // Case 1: Defending king blocks the pawn, and cannot be driven away
    if ((forward_file_bb(strongSide, strongPawn) & weakKing)
        && (opposite_colors(weakKing, strongBishop)
            || relative_rank(strongSide, weakKing) <= RANK_6))
        return SCALE_FACTOR_DRAW;

    // Case 2: Opposite colored bishops
    if (opposite_colors(strongBishop, weakBishop))
        return SCALE_FACTOR_DRAW;

    return SCALE_FACTOR_NONE;
}


/// KBPP vs KB. It detects a few basic draws with opposite-colored bishops
template<>
ScaleFactor Endgame<KBPPKB>::operator()(const Position& pos) const {

    assert(verify_material(pos, strongSide, BishopValueMg, 2));
    assert(verify_material(pos, weakSide, BishopValueMg, 0));

    Square strongBishop = pos.square<BISHOP>(strongSide);
    Square weakBishop   = pos.square<BISHOP>(weakSide);

    if (!opposite_colors(strongBishop, weakBishop))
        return SCALE_FACTOR_NONE;

    Square weakKing    = pos.square<KING>(weakSide);
    Square strongPawn1 = lsb(pos.pieces(strongSide, PAWN));
    Square strongPawn2 = msb(pos.pieces(strongSide, PAWN));
    Square blockSq1, blockSq2;

    if (relative_rank(strongSide, strongPawn1) > relative_rank(strongSide, strongPawn2))
    {
        blockSq1 = strongPawn1 + pawn_push(strongSide);
        blockSq2 = make_square(file_of(strongPawn2), rank_of(strongPawn1));
    }
    else
    {
        blockSq1 = strongPawn2 + pawn_push(strongSide);
        blockSq2 = make_square(file_of(strongPawn1), rank_of(strongPawn2));
    }

    switch (distance<File>(strongPawn1, strongPawn2))
    {
    case 0 :
        // Both pawns are on the same file. It's an easy draw if the defender firmly
        // controls some square in the frontmost pawn's path.
        if (file_of(weakKing) == file_of(blockSq1)
            && relative_rank(strongSide, weakKing) >= relative_rank(strongSide, blockSq1)
            && opposite_colors(weakKing, strongBishop))
            return SCALE_FACTOR_DRAW;
        else
            return SCALE_FACTOR_NONE;

    case 1 :
        // Pawns on adjacent files. It's a draw if the defender firmly controls the
        // square in front of the frontmost pawn's path, and the square diagonally
        // behind this square on the file of the other pawn.
        if (weakKing == blockSq1 && opposite_colors(weakKing, strongBishop)
            && (weakBishop == blockSq2
                || (attacks_bb<BISHOP>(blockSq2, pos.pieces()) & pos.pieces(weakSide, BISHOP))
                || distance<Rank>(strongPawn1, strongPawn2) >= 2))
            return SCALE_FACTOR_DRAW;

        else if (weakKing == blockSq2 && opposite_colors(weakKing, strongBishop)
                 && (weakBishop == blockSq1
                     || (attacks_bb<BISHOP>(blockSq1, pos.pieces())
                         & pos.pieces(weakSide, BISHOP))))
            return SCALE_FACTOR_DRAW;
        else
            return SCALE_FACTOR_NONE;

    default :
        // The pawns are not on the same file or adjacent files. No scaling.
        return SCALE_FACTOR_NONE;
    }
}


/// KBP vs KN. There is a single rule: if the defending king is somewhere along
/// the path of the pawn, and the square of the king is not of the same color as
/// the stronger side's bishop, it's a draw.
template<>
ScaleFactor Endgame<KBPKN>::operator()(const Position& pos) const {

    assert(verify_material(pos, strongSide, BishopValueMg, 1));
    assert(verify_material(pos, weakSide, KnightValueMg, 0));

    Square strongPawn   = pos.square<PAWN>(strongSide);
    Square strongBishop = pos.square<BISHOP>(strongSide);
    Square weakKing     = pos.square<KING>(weakSide);

    if (file_of(weakKing) == file_of(strongPawn)
        && relative_rank(strongSide, strongPawn) < relative_rank(strongSide, weakKing)
        && (opposite_colors(weakKing, strongBishop)
            || relative_rank(strongSide, weakKing) <= RANK_6))
        return SCALE_FACTOR_DRAW;

    return SCALE_FACTOR_NONE;
}


/// KP vs KP. This is done by removing the weakest side's pawn and probing the
/// KP vs K bitbase: if the weakest side has a draw without the pawn, it probably
/// has at least a draw with the pawn as well. The exception is when the stronger
/// side's pawn is far advanced and not on a rook file; in this case it is often
/// possible to win (e.g. 8/4k3/3p4/3P4/6K1/8/8/8 w - - 0 1).
template<>
ScaleFactor Endgame<KPKP>::operator()(const Position& pos) const {

    assert(verify_material(pos, strongSide, VALUE_ZERO, 1));
    assert(verify_material(pos, weakSide, VALUE_ZERO, 1));

    // Assume strongSide is white and the pawn is on files A-D
    Square strongKing = normalize(pos, strongSide, pos.square<KING>(strongSide));
    Square weakKing   = normalize(pos, strongSide, pos.square<KING>(weakSide));
    Square strongPawn = normalize(pos, strongSide, pos.square<PAWN>(strongSide));

    Color us = strongSide == pos.side_to_move() ? WHITE : BLACK;

    // If the pawn has advanced to the fifth rank or further, and is not a
    // rook pawn, it's too dangerous to assume that it's at least a draw.
    if (rank_of(strongPawn) >= RANK_5 && file_of(strongPawn) != FILE_A)
        return SCALE_FACTOR_NONE;

    // Probe the KPK bitbase with the weakest side's pawn removed. If it's a draw,
    // it's probably at least a draw even with the pawn.
    return Bitbases::probe(strongKing, strongPawn, weakKing, us) ? SCALE_FACTOR_NONE
                                                                 : SCALE_FACTOR_DRAW;
}

}  // namespace Alexander


// ============================================================================
// FILE ORIGINALE: endgame.h
// ============================================================================

/*
  Alexander, a UCI chess playing engine derived from Stockfish
  Copyright (C) 2004-2023 The Stockfish developers (see AUTHORS file)

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

#ifndef ENDGAME_H_INCLUDED
#define ENDGAME_H_INCLUDED

#include <memory>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>

#include "position.h"
#include "types.h"

namespace Alexander {

/// EndgameCode lists all supported endgame functions by corresponding codes

enum EndgameCode {

    EVALUATION_FUNCTIONS,
    KNNK,   // KNN vs K
    KNNKP,  // KNN vs KP
    KXK,    // Generic "mate lone king" eval
    KBNK,   // KBN vs K
    KPK,    // KP vs K
    KRKP,   // KR vs KP
    KRKB,   // KR vs KB
    KRKN,   // KR vs KN
    KQKP,   // KQ vs KP
    KQKR,   // KQ vs KR

    SCALING_FUNCTIONS,
    KBPsK,    // KB and pawns vs K
    KQKRPs,   // KQ vs KR and pawns
    KRPKR,    // KRP vs KR
    KRPKB,    // KRP vs KB
    KRPPKRP,  // KRPP vs KRP
    KPsK,     // K and pawns vs K
    KBPKB,    // KBP vs KB
    KBPPKB,   // KBPP vs KB
    KBPKN,    // KBP vs KN
    KPKP      // KP vs KP
};


/// Endgame functions can be of two types depending on whether they return a
/// Value or a ScaleFactor.

template<EndgameCode E>
using eg_type = typename std::conditional<(E < SCALING_FUNCTIONS), Value, ScaleFactor>::type;


/// Base and derived functors for endgame evaluation and scaling functions

template<typename T>
struct EndgameBase {

    explicit EndgameBase(Color c) :
        strongSide(c),
        weakSide(~c) {}
    virtual ~EndgameBase()                      = default;
    virtual T operator()(const Position&) const = 0;

    const Color strongSide, weakSide;
};


template<EndgameCode E, typename T = eg_type<E>>
struct Endgame: public EndgameBase<T> {

    explicit Endgame(Color c) :
        EndgameBase<T>(c) {}
    T operator()(const Position&) const override;
};


/// The Endgames namespace handles the pointers to endgame evaluation and scaling
/// base objects in two std::map. We use polymorphism to invoke the actual
/// endgame function by calling its virtual operator().

namespace Endgames {

template<typename T>
using Ptr = std::unique_ptr<EndgameBase<T>>;
template<typename T>
using Map = std::unordered_map<Key, Ptr<T>>;

extern std::pair<Map<Value>, Map<ScaleFactor>> maps;

void init();

template<typename T>
Map<T>& map() {
    return std::get<std::is_same<T, ScaleFactor>::value>(maps);
}

template<EndgameCode E, typename T = eg_type<E>>
void add(const std::string& code) {

    StateInfo st;
    map<T>()[Position().set(code, WHITE, &st).material_key()] = Ptr<T>(new Endgame<E>(WHITE));
    map<T>()[Position().set(code, BLACK, &st).material_key()] = Ptr<T>(new Endgame<E>(BLACK));
}

template<typename T>
const EndgameBase<T>* probe(Key key) {
    auto it = map<T>().find(key);
    return it != map<T>().end() ? it->second.get() : nullptr;
}
}

}  // namespace Alexander

#endif  // #ifndef ENDGAME_H_INCLUDED


// ============================================================================
// FILE ORIGINALE: engine.cpp
// ============================================================================

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

#include "engine.h"

#include <algorithm>
#include <cassert>
#include <deque>
#include <iosfwd>
#include <memory>
#include <ostream>
#include <sstream>
#include <string_view>
#include <utility>
#include <vector>

#include "evaluate.h"
#include "misc.h"
//from classical
#include "numa.h"
#include "perft.h"
#include "position.h"
#include "search.h"
#include "shm.h"
#include "syzygy/tbprobe.h"
#include "types.h"
#include "uci.h"
#include "ucioption.h"
#include "wdl/win_probability.h"
#include "learn/learn.h"      //learning
#include "book/book.h"        //book management
#include "mcts/montecarlo.h"  //mcts
namespace Alexander {
//for classical

constexpr auto StartFEN   = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
constexpr int  MaxHashMB  = Is64Bit ? 33554432 : 2048;
int            MaxThreads = std::max(1024, 4 * int(get_hardware_concurrency()));

// The default configuration will attempt to group L3 domains up to 32 threads.
// This size was found to be a good balance between the Elo gain of increased
// history sharing and the speed loss from more cross-cache accesses (see
// PR#6526). The user can always explicitly override this behavior.
constexpr NumaAutoPolicy DefaultNumaPolicy = BundledL3Policy{32};

Engine::Engine(std::optional<std::string> path) :
    binaryDirectory(CommandLine::get_binary_directory(
      path.value_or(""), CommandLine::get_working_directory())),  //learning
    numaContext(NumaConfig::from_system(DefaultNumaPolicy)),
    states(new std::deque<StateInfo>(1)),
    threads() {
    pos.set(StartFEN, false, &states->back(),
            threads.empty() ? nullptr : threads.main_thread());  //for classical

    options.add(  //
      "Debug Log File", Option("", [](const Option& o) {
          start_logger(o);
          return std::nullopt;
      }));

    options.add(  //
      "NumaPolicy", Option("auto", [this](const Option& o) {
          set_numa_config_from_option(o);
          return numa_config_information_as_string() + "\n"
               + thread_allocation_information_as_string();
      }));

    options.add(  //
      "Threads", Option(1, 1, MaxThreads, [this](const Option&) {
          resize_threads();
          return thread_allocation_information_as_string();
      }));

    options.add(  //
      "Hash", Option(16, 1, MaxHashMB, [this](const Option& o) {
          set_tt_size(o);
          return std::nullopt;
      }));

    options.add(  //
      "Clear Hash", Option([this](const Option&) {
          search_clear();
          return std::nullopt;
      }));

    options.add("Ponder", Option(false));

    options.add("MultiPV", Option(1, 1, MAX_MOVES));

    options.add("Anti Dancing Analysis", Option(false));

    options.add("Move Overhead", Option(10, 0, 5000));

    options.add("Minimum Thinking Time", Option(100, 0, 5000));

    options.add("Slow Mover", Option(100, 10, 1000));

    options.add("nodestime", Option(0, 0, 10000));

    options.add("UCI_Chess960", Option(false));

    options.add("UCI_ShowWDL", Option(true));

    // Book management
    for (int i = 0; i < BookManager::NumberOfBooks; ++i)
    {
        options.add(Util::format_string("CTG/BIN Book %d File", i + 1),
                    Option(EMPTY, [this, i](const Option&) {
                        init_bookMan(i);
                        return std::nullopt;
                    }));

        options.add(Util::format_string("Book %d Width", i + 1), Option(1, 1, 20));

        options.add(Util::format_string("Book %d Depth", i + 1), Option(255, 1, 255));

        options.add(Util::format_string("(CTG) Book %d Only Green", i + 1), Option(true));
    }

    options.add(  //
      "SyzygyPath", Option("", [](const Option& o) {
          Tablebases::init(o);
          return std::nullopt;
      }));

    options.add("SyzygyProbeDepth", Option(1, 1, 100));

    options.add("Syzygy50MoveRule", Option(true));

    options.add("SyzygyProbeLimit", Option(7, 0, 7));

    // Handicap mode
    options.add("UCI_LimitStrength", Option(false, [this](const Option&) {
                    Eval::initHandicapMode(get_options());
                    return std::nullopt;
                }));

    options.add("Handicapped Depth",
                Option(false, [this](const Option&) -> std::optional<std::string> {
                    Eval::initHandicapMode(get_options());
                    return std::nullopt;
                }));

    options.add("UCI_Elo",
                Option(3190, 1320, 3190, [this](const Option&) -> std::optional<std::string> {
                    Eval::initHandicapMode(get_options());
                    return std::nullopt;
                }));

    options.add("Simulate human blunders", Option(false, [this](const Option&) {
                    Eval::initHandicapMode(get_options());
                    return std::nullopt;
                }));

    options.add("Avatar File", Option("", [](const Option& o) {
                    Eval::loadAvatar(o);
                    return std::nullopt;
                }));

    // ChessBase handicap mode
    options.add("LimitStrength_CB", Option(false, [this](const Option&) {
                    Eval::initHandicapMode(get_options());
                    return std::nullopt;
                }));

    options.add("ELO_CB",
                Option(3190, 1320, 3190, [this](const Option&) -> std::optional<std::string> {
                    Eval::initHandicapMode(get_options());
                    return std::nullopt;
                }));

    options.add(  //
      "Full depth threads", Option(0, 0, 1024, [this](const Option& o) {
          resize_full(o);
          return thread_binding_information_as_string();
      }));

    // From learning
    options.add("Persisted learning", Option("Off var Off var Standard var Self", "Off",
                                             [this](const Option& o) -> std::optional<std::string> {
                                                 if (!(o == "Off"))
                                                 {
                                                     LD.set_learning_mode(get_options(), o);
                                                 }
                                                 return std::optional<std::string>{};
                                             }));

    options.add("Read only learning", Option(false, [](const Option& o) {
                    LD.set_readonly(o);
                    return std::nullopt;
                }));

    options.add("Experience Book", Option(false, [this](const Option&) {
                    LD.init(get_options());
                    return std::nullopt;
                }));

    options.add("Experience Book Max Moves", Option(100, 1, 100));

    options.add("Experience Book Min Depth", Option(4, 1, 255));

    // From MCTS
    options.add("MCTS by Shashin", Option(false));

    options.add("MCTSThreads", Option(1, 1, 512));

    options.add("MCTS Multi Strategy", Option(20, 0, 100));

    options.add("MCTS Multi MinVisits", Option(5, 0, 1000));

    options.add("MCTS Explore", Option(false));

    // LiveBook options
#ifdef USE_LIVEBOOK
    options.add("LiveBook Proxy Url", Option("", [](const Option& o) -> std::optional<std::string> {
                    Search::set_proxy_url(o);
                    return std::optional<std::string>{};
                }));

    options.add("LiveBook Lichess Games", Option(false, [](const Option& o) {
                    Search::set_use_lichess_games(o);
                    return std::nullopt;
                }));

    options.add("LiveBook Lichess Masters", Option(false, [](const Option& o) {
                    Search::set_use_lichess_masters(o);
                    return std::nullopt;
                }));

    options.add("LiveBook Lichess Player",
                Option("", [](const Option& o) -> std::optional<std::string> {
                    Search::set_lichess_player(o);
                    return std::optional<std::string>{};
                }));

    options.add("LiveBook Lichess Player Color",
                Option("White var Both var White var Black", "White",
                       [](const Option& o) -> std::optional<std::string> {
                           std::string str = o;
                           std::transform(str.begin(), str.end(), str.begin(),
                                          [](const unsigned char c) { return std::tolower(c); });
                           Search::set_lichess_player_color(str);
                           return std::optional<std::string>{};
                       }));

    options.add("LiveBook ChessDB", Option(false, [](const Option& o) {
                    Search::set_use_chess_db(o);
                    return std::nullopt;
                }));

    options.add("LiveBook Depth", Option(255, 1, 255, [](const Option& o) {
                    Search::set_livebook_depth(o);
                    return std::nullopt;
                }));

    options.add("ChessDB Tablebase", Option(false, [](const Option& o) {
                    Search::set_use_chess_db_tablebase(o);
                    return std::nullopt;
                }));

    options.add("Lichess Tablebase", Option(false, [](const Option& o) {
                    Search::set_use_lichess_tablebase(o);
                    return std::nullopt;
                }));

    options.add("ChessDB Contribute", Option(false, [](const Option& o) {
                    Search::set_chess_db_contribute(o);
                    return std::nullopt;
                }));
#endif

    options.add("Variety", Option("Off var Off var Standard var Psychological", "Off",
                                  [](const Option& o) -> std::optional<std::string> {
                                      Search::set_variety(o);
                                      return std::optional<std::string>{};
                                  }));

    options.add("Concurrent Experience", Option(false));

    // Shashin personalities begin
    options.add("High Tal",
                Option(false, [this](const Option&) noexcept -> std::optional<std::string> {
                    resize_threads();
                    return std::nullopt;
                }));
    options.add("Middle Tal",
                Option(false, [this](const Option&) noexcept -> std::optional<std::string> {
                    resize_threads();
                    return std::nullopt;
                }));
    options.add("Low Tal",
                Option(false, [this](const Option&) noexcept -> std::optional<std::string> {
                    resize_threads();
                    return std::nullopt;
                }));
    options.add("Capablanca",
                Option(false, [this](const Option&) noexcept -> std::optional<std::string> {
                    resize_threads();
                    return std::nullopt;
                }));
    options.add("Low Petrosian",
                Option(false, [this](const Option&) noexcept -> std::optional<std::string> {
                    resize_threads();
                    return std::nullopt;
                }));
    options.add("Middle Petrosian",
                Option(false, [this](const Option&) noexcept -> std::optional<std::string> {
                    resize_threads();
                    return std::nullopt;
                }));
    options.add("High Petrosian",
                Option(false, [this](const Option&) noexcept -> std::optional<std::string> {
                    resize_threads();
                    return std::nullopt;
                }));
    // Shashin personalities end
    //from classical
    resize_threads();
}
std::uint64_t Engine::perft(const std::string& fen, Depth depth, bool isChess960, Thread* th) {
    //from classical
    return Benchmark::perft(fen, depth, isChess960, th);
}

void Engine::go(Search::LimitsType& limits) {
    assert(limits.perft == 0);
    //from classical

    threads.start_thinking(options, pos, states, limits);
}
void Engine::stop() { threads.stop = true; }

void Engine::search_clear() {
    wait_for_search_finished();

    MCTS.clear();  //mcts
    tt.clear(threads);
    threads.clear();

    // @TODO wont work with multiple instances
    Tablebases::init(options["SyzygyPath"]);  // Free mapped files
}

void Engine::set_on_update_no_moves(std::function<void(const Engine::InfoShort&)>&& f) {
    updateContext.onUpdateNoMoves = std::move(f);
}

void Engine::set_on_update_full(std::function<void(const Engine::InfoFull&)>&& f) {
    updateContext.onUpdateFull = std::move(f);
}

void Engine::set_on_iter(std::function<void(const Engine::InfoIter&)>&& f) {
    updateContext.onIter = std::move(f);
}

void Engine::set_on_bestmove(std::function<void(std::string_view, std::string_view)>&& f) {
    updateContext.onBestmove = std::move(f);
}

//from classical
void Engine::wait_for_search_finished() { threads.main_thread()->wait_for_search_finished(); }

void Engine::set_position(const std::string& fen, const std::vector<std::string>& moves) {
    // Drop the old state and create a new one
    states = StateListPtr(new std::deque<StateInfo>(1));
    pos.set(fen, options["UCI_Chess960"], &states->back(), threads.main_thread());  //for classical

    for (const auto& move : moves)
    {
        auto m = UCIEngine::to_move(pos, move);

        if (m == Move::none())
            break;
        //Kelly begin
        if (LD.is_enabled() && LD.learning_mode() != LearningMode::Self && !LD.is_paused())
        {
            PersistedLearningMove persistedLearningMove;

            persistedLearningMove.key                      = pos.key();
            persistedLearningMove.learningMove.depth       = 0;
            persistedLearningMove.learningMove.move        = m;
            persistedLearningMove.learningMove.score       = VALUE_NONE;
            persistedLearningMove.learningMove.performance = WDLModel::get_win_probability(0, pos);
            LD.add_new_learning(persistedLearningMove.key, persistedLearningMove.learningMove);
        }
        //Kelly end
        states->emplace_back();
        pos.do_move(m, states->back());
        //for classical
    }
}

// modifiers

void Engine::set_numa_config_from_option(const std::string& o) {
    if (o == "auto" || o == "system")
    {
        numaContext.set_numa_config(NumaConfig::from_system(DefaultNumaPolicy));
    }
    else if (o == "hardware")
    {
        // Don't respect affinity set in the system.
        numaContext.set_numa_config(NumaConfig::from_system(DefaultNumaPolicy, false));
    }
    else if (o == "none")
    {
        numaContext.set_numa_config(NumaConfig{});
    }
    else
    {
        numaContext.set_numa_config(NumaConfig::from_string(o));
    }

    // Force reallocation of threads in case affinities need to change.
    resize_threads();
}

void Engine::resize_threads() {
    threads.wait_for_search_finished();
    //from shashin begin
    ShashinConfig shCfg;
    shCfg.highTal         = options["High Tal"];
    shCfg.middleTal       = options["Middle Tal"];
    shCfg.lowTal          = options["Low Tal"];
    shCfg.capablanca      = options["Capablanca"];
    shCfg.highPetrosian   = options["High Petrosian"];
    shCfg.middlePetrosian = options["Middle Petrosian"];
    shCfg.lowPetrosian    = options["Low Petrosian"];
    threads.set(numaContext.get_numa_config(), {bookMan, options, threads, tt, sharedHists, shCfg},
                updateContext);  //book management from classical

    //from shashin end
    // Reallocate the hash with the new threadpool size
    set_tt_size(options["Hash"]);
    //from classical
}
void Engine::init_bookMan(int bookIndex) { bookMan.init(bookIndex, options); }  //book management
void Engine::resize_full(size_t requested) { threads.setFull(requested); }      //full threads patch

void Engine::set_tt_size(size_t mb) {
    wait_for_search_finished();
    tt.resize(mb, threads);
}

void Engine::set_ponderhit(bool b) { threads.main_manager()->ponder = b; }

//from classical
// utility functions

void Engine::trace_eval() const {
    StateListPtr trace_states(new std::deque<StateInfo>(1));
    Position     p;
    p.set(pos.fen(), options["UCI_Chess960"], &states->back(),
          threads.main_thread());  //for classical

    sync_cout << "\n" << Eval::trace(p) << sync_endl;
}

const OptionsMap& Engine::get_options() const { return options; }
OptionsMap&       Engine::get_options() { return options; }

std::string Engine::fen() const { return pos.fen(); }

void Engine::flip() { pos.flip(); }
void Engine::show_moves_bookMan(const Position& position) {
    bookMan.show_moves(position, options);
}  //book management
std::string Engine::visualize() const {
    std::stringstream ss;
    ss << pos;
    return ss.str();
}

int Engine::get_hashfull(int maxAge) const { return tt.hashfull(maxAge); }

std::vector<std::pair<size_t, size_t>> Engine::get_bound_thread_count_by_numa_node() const {
    auto                                   counts = threads.get_bound_thread_count_by_numa_node();
    const NumaConfig&                      cfg    = numaContext.get_numa_config();
    std::vector<std::pair<size_t, size_t>> ratios;
    NumaIndex                              n = 0;
    for (; n < counts.size(); ++n)
        ratios.emplace_back(counts[n], cfg.num_cpus_in_numa_node(n));
    if (!counts.empty())
        for (; n < cfg.num_numa_nodes(); ++n)
            ratios.emplace_back(0, cfg.num_cpus_in_numa_node(n));
    return ratios;
}

std::string Engine::get_numa_config_as_string() const {
    return numaContext.get_numa_config().to_string();
}

std::string Engine::numa_config_information_as_string() const {
    auto cfgStr = get_numa_config_as_string();
    return "Available processors: " + cfgStr;
}

std::string Engine::thread_binding_information_as_string() const {
    auto              boundThreadsByNode = get_bound_thread_count_by_numa_node();
    std::stringstream ss;
    if (boundThreadsByNode.empty())
        return ss.str();

    bool isFirst = true;

    for (auto&& [current, total] : boundThreadsByNode)
    {
        if (!isFirst)
            ss << ":";
        ss << current << "/" << total;
        isFirst = false;
    }

    return ss.str();
}

std::string Engine::thread_allocation_information_as_string() const {
    std::stringstream ss;

    size_t threadsSize = threads.size();
    ss << "Using " << threadsSize << (threadsSize > 1 ? " threads" : " thread");

    auto boundThreadsByNodeStr = thread_binding_information_as_string();
    if (boundThreadsByNodeStr.empty())
        return ss.str();

    ss << " with NUMA node thread binding: ";
    ss << boundThreadsByNodeStr;

    return ss.str();
}

}


// ============================================================================
// FILE ORIGINALE: engine.h
// ============================================================================

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

#include <cstddef>
#include <cstdint>
#include <functional>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "history.h"
//for classical
#include "position.h"
#include "search.h"
#include "syzygy/tbprobe.h"  // for Alexander::Depth
#include "thread.h"
#include "tt.h"
#include "ucioption.h"

namespace Alexander {

class Engine {
   public:
    using InfoShort = Search::InfoShort;
    using InfoFull  = Search::InfoFull;
    using InfoIter  = Search::InfoIteration;

    Engine(std::optional<std::string> path = std::nullopt);

    // Cannot be movable due to components holding backreferences to fields
    Engine(const Engine&)            = delete;
    Engine(Engine&&)                 = delete;
    Engine& operator=(const Engine&) = delete;
    Engine& operator=(Engine&&)      = delete;

    ~Engine() { wait_for_search_finished(); }

    std::uint64_t
    perft(const std::string& fen, Depth depth, bool isChess960, Thread* th);  //for classical

    // non blocking call to start searching
    void go(Search::LimitsType&);
    // non blocking call to stop searching
    void stop();

    // blocking call to wait for search to finish
    void wait_for_search_finished();
    // set a new position, moves are in UCI format
    void set_position(const std::string& fen, const std::vector<std::string>& moves);

    // modifiers

    void set_numa_config_from_option(const std::string& o);
    void resize_threads();
    void init_bookMan(int bookIndex);    //book management
    void resize_full(size_t requested);  //full threads patch
    void set_tt_size(size_t mb);
    void set_ponderhit(bool);
    void search_clear();
#ifdef USE_LIVEBOOK
    void setLiveBookURL(const std::string& newURL);
    void setLiveBookTimeout(size_t newTimeoutMS);
#endif

    void set_on_update_no_moves(std::function<void(const InfoShort&)>&&);
    void set_on_update_full(std::function<void(const InfoFull&)>&&);
    void set_on_iter(std::function<void(const InfoIter&)>&&);
    void set_on_bestmove(std::function<void(std::string_view, std::string_view)>&&);

    //for classical
    // utility functions

    void trace_eval() const;

    const OptionsMap& get_options() const;
    OptionsMap&       get_options();
    BookManager       get_bookMan();  //book management
    int               get_hashfull(int maxAge = 0) const;

    std::string fen() const;
    void        flip();
    std::string visualize() const;
    void        show_moves_bookMan(const Position& position);  //book management
    std::vector<std::pair<size_t, size_t>> get_bound_thread_count_by_numa_node() const;
    std::string                            get_numa_config_as_string() const;
    std::string                            numa_config_information_as_string() const;
    std::string                            thread_allocation_information_as_string() const;
    std::string                            thread_binding_information_as_string() const;
    Position                               pos;  //from learning
   private:
    const std::string binaryDirectory;

    NumaReplicationContext numaContext;
    //from learning
    StateListPtr states;

    OptionsMap         options;
    ThreadPool         threads;
    TranspositionTable tt;
    //for classical
    BookManager                          bookMan;  //book management
    Search::SearchManager::UpdateContext updateContext;
    std::map<NumaIndex, SharedHistories> sharedHists;
    //for classical
};

}  // namespace Alexander


#endif  // #ifndef ENGINE_H_INCLUDED

// ============================================================================
// FILE ORIGINALE: evaluate.cpp
// ============================================================================

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

#include "evaluate.h"

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <iomanip>
#include <sstream>
#include "handicap/evaluate_handicap.h"
#include "position.h"
#include "types.h"
#include "uci.h"
#include "pawns.h"
#include "bitboard.h"
#include "material.h"
#include <cstring>  // For std::memset
#include <string>
#include "handicap/trace/trace.h"
#include "wdl/win_probability.h"
namespace Alexander {
using namespace Trace;
namespace {

// Threshold for lazy and space evaluation
constexpr Value LazyThreshold1 = Value(3622);
constexpr Value LazyThreshold2 = Value(1962);
constexpr Value SpaceThreshold = Value(11551);

// KingAttackWeights[PieceType] contains king attack weights by piece type
constexpr int KingAttackWeights[PIECE_TYPE_NB] = {0, 0, 76, 46, 45, 14};

// SafeCheck[PieceType][single/multiple] contains safe check bonus by piece type,
// higher if multiple safe checks are possible for that piece type.
constexpr int SafeCheck[][2] = {{}, {}, {805, 1292}, {650, 984}, {1071, 1886}, {730, 1128}};
#define S(mg, eg) make_score(mg, eg)

// MobilityBonus[PieceType-2][attacked] contains bonuses for middle and end game,
// indexed by piece type and number of attacked squares in the mobility area.
constexpr ScoreForClassical MobilityBonus[][32] = {
  {S(-62, -79), S(-53, -57), S(-12, -31), S(-3, -17), S(3, 7), S(12, 13),  // Knight
   S(21, 16), S(28, 21), S(37, 26)},
  {S(-47, -59), S(-20, -25), S(14, -8), S(29, 12), S(39, 21), S(53, 40),  // Bishop
   S(53, 56), S(60, 58), S(62, 65), S(69, 72), S(78, 78), S(83, 87), S(91, 88), S(96, 98)},
  {S(-60, -82), S(-24, -15), S(0, 17), S(3, 43), S(4, 72), S(14, 100),  // Rook
   S(20, 102), S(30, 122), S(41, 133), S(41, 139), S(41, 153), S(45, 160), S(57, 165), S(58, 170),
   S(67, 175)},
  {S(-29, -49), S(-16, -29), S(-8, -8),   S(-8, 17),   S(18, 39),   S(25, 54),  // Queen
   S(23, 59),   S(37, 73),   S(41, 76),   S(54, 95),   S(65, 95),   S(68, 101),
   S(69, 124),  S(70, 128),  S(70, 132),  S(70, 133),  S(71, 136),  S(72, 140),
   S(74, 147),  S(76, 149),  S(90, 153),  S(104, 169), S(105, 171), S(106, 171),
   S(112, 178), S(114, 185), S(114, 187), S(119, 221)}};

// BishopPawns[distance from edge] contains a file-dependent penalty for pawns on
// squares of the same color as our bishop.
constexpr ScoreForClassical BishopPawns[int(FILE_NB) / 2] = {S(3, 8), S(3, 9), S(2, 7), S(3, 7)};

// KingProtector[knight/bishop] contains penalty for each distance unit to own king
constexpr ScoreForClassical KingProtector[] = {S(9, 9), S(7, 9)};

// Outpost[knight/bishop] contains bonuses for each knight or bishop occupying a
// pawn protected square on rank 4 to 6 which is also safe from a pawn attack.
constexpr ScoreForClassical Outpost[] = {S(54, 34), S(31, 25)};

// PassedRank[Rank] contains a bonus according to the rank of a passed pawn
constexpr ScoreForClassical PassedRank[RANK_NB] = {S(0, 0),   S(2, 38),    S(15, 36),  S(22, 50),
                                                   S(64, 81), S(166, 184), S(284, 269)};

constexpr ScoreForClassical RookOnClosedFile = S(10, 5);
constexpr ScoreForClassical RookOnOpenFile[] = {S(18, 8), S(49, 26)};

// ThreatByMinor/ByRook[attacked PieceType] contains bonuses according to
// which piece type attacks which one. Attacks on lesser pieces which are
// pawn-defended are not considered.
constexpr ScoreForClassical ThreatByMinor[PIECE_TYPE_NB] = {S(0, 0),   S(6, 37),    S(64, 50),
                                                            S(82, 57), S(103, 130), S(81, 163)};

constexpr ScoreForClassical ThreatByRook[PIECE_TYPE_NB] = {S(0, 0),   S(3, 44), S(36, 71),
                                                           S(44, 59), S(0, 39), S(60, 39)};

constexpr Value CorneredBishop = Value(50);

// Assorted bonuses and penalties
constexpr ScoreForClassical UncontestedOutpost  = S(0, 10);
constexpr ScoreForClassical BishopOnKingRing    = S(24, 0);
constexpr ScoreForClassical BishopXRayPawns     = S(4, 5);
constexpr ScoreForClassical FlankAttacks        = S(8, 0);
constexpr ScoreForClassical Hanging             = S(72, 40);
constexpr ScoreForClassical KnightOnQueen       = S(16, 11);
constexpr ScoreForClassical LongDiagonalBishop  = S(45, 0);
constexpr ScoreForClassical MinorBehindPawn     = S(18, 3);
constexpr ScoreForClassical PassedFile          = S(13, 8);
constexpr ScoreForClassical PawnlessFlank       = S(19, 97);
constexpr ScoreForClassical ReachableOutpost    = S(33, 19);
constexpr ScoreForClassical RestrictedPiece     = S(6, 7);
constexpr ScoreForClassical RookOnKingRing      = S(16, 0);
constexpr ScoreForClassical SliderOnQueen       = S(62, 21);
constexpr ScoreForClassical ThreatByKing        = S(24, 87);
constexpr ScoreForClassical ThreatByPawnPush    = S(48, 39);
constexpr ScoreForClassical ThreatBySafePawn    = S(167, 99);
constexpr ScoreForClassical TrappedRook         = S(55, 13);
constexpr ScoreForClassical WeakQueenProtection = S(14, 0);
constexpr ScoreForClassical WeakQueen           = S(57, 19);
#undef S

// Evaluation class computes and stores attacks tables and other working data
template<Trace::Tracing T>
class Evaluation {
   public:
    Evaluation() = delete;
    explicit Evaluation(const Position& p) :
        pos(p) {}
    Evaluation& operator=(const Evaluation&) = delete;
    Value       value();

   private:
    template<Color Us>
    void initialize();
    template<Color Us, PieceType Pt>
    ScoreForClassical pieces();
    template<Color Us>
    ScoreForClassical king() const;
    template<Color Us>
    ScoreForClassical threats() const;
    template<Color Us>
    ScoreForClassical passed() const;
    template<Color Us>
    ScoreForClassical space() const;
    Value             winnable(ScoreForClassical score) const;
    const Position&   pos;
    Material::Entry*  me;
    Pawns::Entry*     pe;
    Bitboard          mobilityArea[COLOR_NB];
    ScoreForClassical mobility[COLOR_NB] = {SCORE_ZERO, SCORE_ZERO};

    // attackedBy[color][piece type] is a bitboard representing all squares
    // attacked by a given color and piece type. Special "piece types" which
    // is also calculated is ALL_PIECES.
    Bitboard attackedBy[COLOR_NB][PIECE_TYPE_NB];

    // attackedBy2[color] are the squares attacked by at least 2 units of a given
    // color, including x-rays. But diagonal x-rays through pawns are not computed.
    Bitboard attackedBy2[COLOR_NB];

    // kingRing[color] are the squares adjacent to the king plus some other
    // very near squares, depending on king position.
    Bitboard kingRing[COLOR_NB];

    // kingAttackersCount[color] is the number of pieces of the given color
    // which attack a square in the kingRing of the enemy king.
    int kingAttackersCount[COLOR_NB];

    // kingAttackersWeight[color] is the sum of the "weights" of the pieces of
    // the given color which attack a square in the kingRing of the enemy king.
    // The weights of the individual piece types are given by the elements in
    // the KingAttackWeights array.
    int kingAttackersWeight[COLOR_NB];

    // kingAttacksCount[color] is the number of attacks by the given color to
    // squares directly adjacent to the enemy king. Pieces which attack more
    // than one square are counted multiple times. For instance, if there is
    // a white knight on g5 and black's king is on g8, this white knight adds 2
    // to kingAttacksCount[WHITE].
    int kingAttacksCount[COLOR_NB];
    // Funzione helper per calcolare la mobilità dettagliata per area
    template<Color Us, PieceType Pt>
    void update_mobility_area_counts(Bitboard attacks) {
        if constexpr (T)
        {
            while (attacks)
            {
                Square s    = pop_lsb(attacks);
                Area   area = Trace::area_of(s);
                Trace::mobility_area_counts[Us][area]++;
            }
        }
    }
};
// Evaluation::initialize() computes king and pawn attacks, and the king ring
// bitboard for a given color. This is done at the beginning of the evaluation.

template<Trace::Tracing T>
template<Color Us>
void Evaluation<T>::initialize() {

    constexpr Color     Them     = ~Us;
    constexpr Direction Up       = pawn_push(Us);
    constexpr Direction Down     = -Up;
    constexpr Bitboard  LowRanks = (Us == WHITE ? Rank2BB | Rank3BB : Rank7BB | Rank6BB);

    const Square ksq = pos.square<KING>(Us);

    Bitboard dblAttackByPawn = pawn_double_attacks_bb<Us>(pos.pieces(Us, PAWN));

    // Find our pawns that are blocked or on the first two ranks
    Bitboard b = pos.pieces(Us, PAWN) & (shift<Down>(pos.pieces()) | LowRanks);

    // Squares occupied by those pawns, by our king or queen, by blockers to attacks on our king
    // or controlled by enemy pawns are excluded from the mobility area.
    mobilityArea[Us] =
      ~(b | pos.pieces(Us, KING, QUEEN) | pos.blockers_for_king(Us) | pe->pawn_attacks(Them));

    // Initialize attackedBy[] for king and pawns
    attackedBy[Us][KING]       = attacks_bb<KING>(ksq);
    attackedBy[Us][PAWN]       = pe->pawn_attacks(Us);
    attackedBy[Us][ALL_PIECES] = attackedBy[Us][KING] | attackedBy[Us][PAWN];
    attackedBy2[Us]            = dblAttackByPawn | (attackedBy[Us][KING] & attackedBy[Us][PAWN]);

    // Init our king safety tables
    Square s     = make_square(std::clamp(file_of(ksq), FILE_B, FILE_G),
                               std::clamp(rank_of(ksq), RANK_2, RANK_7));
    kingRing[Us] = attacks_bb<KING>(s) | s;

    kingAttackersCount[Them] = popcount(kingRing[Us] & pe->pawn_attacks(Them));
    kingAttacksCount[Them] = kingAttackersWeight[Them] = 0;

    // Remove from kingRing[] the squares defended by two pawns
    kingRing[Us] &= ~dblAttackByPawn;
}


// Evaluation::pieces() scores pieces of a given color and type

template<Trace::Tracing T>
template<Color Us, PieceType Pt>
ScoreForClassical Evaluation<T>::pieces() {

    constexpr Color                      Them = ~Us;
    [[maybe_unused]] constexpr Direction Down = -pawn_push(Us);
    [[maybe_unused]] constexpr Bitboard  OutpostRanks =
      (Us == WHITE ? Rank4BB | Rank5BB | Rank6BB : Rank5BB | Rank4BB | Rank3BB);
    Bitboard          b1 = pos.pieces(Us, Pt);
    Bitboard          b, bb;
    ScoreForClassical score = SCORE_ZERO;

    attackedBy[Us][Pt] = 0;

    while (b1)
    {
        Square s = pop_lsb(b1);

        // Find attacked squares, including x-ray attacks for bishops and rooks
        b = Pt == BISHOP ? attacks_bb<BISHOP>(s, pos.pieces() ^ pos.pieces(QUEEN))
          : Pt == ROOK
            ? attacks_bb<ROOK>(s, pos.pieces() ^ pos.pieces(QUEEN) ^ pos.pieces(Us, ROOK))
            : attacks_bb<Pt>(s, pos.pieces());

        if (pos.blockers_for_king(Us) & s)
            b &= line_bb(pos.square<KING>(Us), s);

        attackedBy2[Us] |= attackedBy[Us][ALL_PIECES] & b;
        attackedBy[Us][Pt] |= b;
        attackedBy[Us][ALL_PIECES] |= b;

        if (b & kingRing[Them])
        {
            kingAttackersCount[Us]++;
            kingAttackersWeight[Us] += KingAttackWeights[Pt];
            kingAttacksCount[Us] += popcount(b & attackedBy[Them][KING]);
        }

        else if (Pt == ROOK && (file_bb(s) & kingRing[Them]))
            score += RookOnKingRing;

        else if (Pt == BISHOP && (attacks_bb<BISHOP>(s, pos.pieces(PAWN)) & kingRing[Them]))
            score += BishopOnKingRing;

        int mob = popcount(b & mobilityArea[Us]);
        mobility[Us] += MobilityBonus[Pt - 2][mob];

        // CALCOLA CONTEGGI MOBILITÀ DETTAGLIATI PER AREA (senza distinguere il pezzo)
        Bitboard mobility_attacks = b & mobilityArea[Us];
        update_mobility_area_counts<Us, Pt>(mobility_attacks);

        if constexpr (Pt == BISHOP || Pt == KNIGHT)
        {
            // Bonus if the piece is on an outpost square or can reach one
            // Bonus for knights (UncontestedOutpost) if few relevant targets
            bb = OutpostRanks & (attackedBy[Us][PAWN] | shift<Down>(pos.pieces(PAWN)))
               & ~pe->pawn_attacks_span(Them);
            Bitboard targets = pos.pieces(Them) & ~pos.pieces(PAWN);

            if (Pt == KNIGHT && bb & s & ~CenterFiles  // on a side outpost
                && !(b & targets)                      // no relevant attacks
                && (!more_than_one(targets & (s & QueenSide ? QueenSide : KingSide))))
                score += UncontestedOutpost
                       * popcount(pos.pieces(PAWN) & (s & QueenSide ? QueenSide : KingSide));
            else if (bb & s)
                score += Outpost[Pt == BISHOP];
            else if (Pt == KNIGHT && bb & b & ~pos.pieces(Us))
                score += ReachableOutpost;

            // Bonus for a knight or bishop shielded by pawn
            if (shift<Down>(pos.pieces(PAWN)) & s)
                score += MinorBehindPawn;

            // Penalty if the piece is far from the king
            score -= KingProtector[Pt == BISHOP] * distance(pos.square<KING>(Us), s);

            if constexpr (Pt == BISHOP)
            {
                // Penalty according to the number of our pawns on the same color square as the
                // bishop, bigger when the center files are blocked with pawns and smaller
                // when the bishop is outside the pawn chain.
                Bitboard blocked = pos.pieces(Us, PAWN) & shift<Down>(pos.pieces());

                score -= BishopPawns[edge_distance(file_of(s))]
                       * pos.pawns_on_same_color_squares(Us, s)
                       * (!(attackedBy[Us][PAWN] & s) + popcount(blocked & CenterFiles));

                // Penalty for all enemy pawns x-rayed
                score -= BishopXRayPawns * popcount(attacks_bb<BISHOP>(s) & pos.pieces(Them, PAWN));

                // Bonus for bishop on a long diagonal which can "see" both center squares
                if (more_than_one(attacks_bb<BISHOP>(s, pos.pieces(PAWN)) & Center))
                    score += LongDiagonalBishop;

                // An important Chess960 pattern: a cornered bishop blocked by a friendly
                // pawn diagonally in front of it is a very serious problem, especially
                // when that pawn is also blocked.
                if (pos.is_chess960()
                    && (s == relative_square(Us, SQ_A1) || s == relative_square(Us, SQ_H1)))
                {
                    Direction d = pawn_push(Us) + (file_of(s) == FILE_A ? EAST : WEST);
                    if (pos.piece_on(s + d) == make_piece(Us, PAWN))
                        score -= !pos.empty(s + d + pawn_push(Us))
                                 ? 4 * make_score(CorneredBishop, CorneredBishop)
                                 : 3 * make_score(CorneredBishop, CorneredBishop);
                }
            }
        }

        if constexpr (Pt == ROOK)
        {
            // Bonuses for rook on a (semi-)open or closed file
            if (pos.is_on_semiopen_file(Us, s))
            {
                score += RookOnOpenFile[pos.is_on_semiopen_file(Them, s)];
            }
            else
            {
                // If our pawn on this file is blocked, increase penalty
                if (pos.pieces(Us, PAWN) & shift<Down>(pos.pieces()) & file_bb(s))
                {
                    score -= RookOnClosedFile;
                }

                // Penalty when trapped by the king, even more if the king cannot castle
                if (mob <= 3)
                {
                    File kf = file_of(pos.square<KING>(Us));
                    if ((kf < FILE_E) == (file_of(s) < kf))
                        score -= TrappedRook * (1 + !pos.castling_rights(Us));
                }
            }
        }

        if constexpr (Pt == QUEEN)
        {
            // Penalty if any relative pin or discovered attack against the queen
            Bitboard queenPinners;

            if (pos.slider_blockers(pos.pieces(Them, ROOK, BISHOP), s, queenPinners))
                score -= WeakQueen;
        }
    }
    if constexpr (T)
        Trace::add(Pt, Us, score);

    return score;
}

// Evaluation::king() assigns bonuses and penalties to a king of a given color

template<Trace::Tracing T>
template<Color Us>
ScoreForClassical Evaluation<T>::king() const {

    constexpr Color    Them = ~Us;
    constexpr Bitboard Camp = (Us == WHITE ? AllSquares ^ Rank6BB ^ Rank7BB ^ Rank8BB
                                           : AllSquares ^ Rank1BB ^ Rank2BB ^ Rank3BB);

    Bitboard     weak, b1, b2, b3, safe, unsafeChecks = 0;
    Bitboard     rookChecks, queenChecks, bishopChecks, knightChecks;
    int          kingDanger = 0;
    const Square ksq        = pos.square<KING>(Us);

    // Init the score with king shelter and enemy pawns storm
    ScoreForClassical score = pe->king_safety<Us>(pos);

    // Attacked squares defended at most once by our queen or king
    weak = attackedBy[Them][ALL_PIECES] & ~attackedBy2[Us]
         & (~attackedBy[Us][ALL_PIECES] | attackedBy[Us][KING] | attackedBy[Us][QUEEN]);

    // Analyse the safe enemy's checks which are possible on next move
    safe = ~pos.pieces(Them);
    safe &= ~attackedBy[Us][ALL_PIECES] | (weak & attackedBy2[Them]);

    b1 = attacks_bb<ROOK>(ksq, pos.pieces() ^ pos.pieces(Us, QUEEN));
    b2 = attacks_bb<BISHOP>(ksq, pos.pieces() ^ pos.pieces(Us, QUEEN));

    // Enemy rooks checks
    rookChecks = b1 & attackedBy[Them][ROOK] & safe;
    if (rookChecks)
        kingDanger += SafeCheck[ROOK][more_than_one(rookChecks)];
    else
        unsafeChecks |= b1 & attackedBy[Them][ROOK];

    // Enemy queen safe checks: count them only if the checks are from squares from
    // which opponent cannot give a rook check, because rook checks are more valuable.
    queenChecks =
      (b1 | b2) & attackedBy[Them][QUEEN] & safe & ~(attackedBy[Us][QUEEN] | rookChecks);
    if (queenChecks)
        kingDanger += SafeCheck[QUEEN][more_than_one(queenChecks)];

    // Enemy bishops checks: count them only if they are from squares from which
    // opponent cannot give a queen check, because queen checks are more valuable.
    bishopChecks = b2 & attackedBy[Them][BISHOP] & safe & ~queenChecks;
    if (bishopChecks)
        kingDanger += SafeCheck[BISHOP][more_than_one(bishopChecks)];

    else
        unsafeChecks |= b2 & attackedBy[Them][BISHOP];

    // Enemy knights checks
    knightChecks = attacks_bb<KNIGHT>(ksq) & attackedBy[Them][KNIGHT];
    if (knightChecks & safe)
        kingDanger += SafeCheck[KNIGHT][more_than_one(knightChecks & safe)];
    else
        unsafeChecks |= knightChecks;

    // Find the squares that opponent attacks in our king flank, the squares
    // which they attack twice in that flank, and the squares that we defend.
    b1 = attackedBy[Them][ALL_PIECES] & KingFlank[file_of(ksq)] & Camp;
    b2 = b1 & attackedBy2[Them];
    b3 = attackedBy[Us][ALL_PIECES] & KingFlank[file_of(ksq)] & Camp;

    int kingFlankAttack  = popcount(b1) + popcount(b2);
    int kingFlankDefense = popcount(b3);

    kingDanger += kingAttackersCount[Them] * kingAttackersWeight[Them]       // (~10 Elo)
                + 183 * popcount(kingRing[Us] & weak)                        // (~15 Elo)
                + 148 * popcount(unsafeChecks)                               // (~4 Elo)
                + 98 * popcount(pos.blockers_for_king(Us))                   // (~2 Elo)
                + 69 * kingAttacksCount[Them]                                // (~0.5 Elo)
                + 3 * kingFlankAttack * kingFlankAttack / 8                  // (~0.5 Elo)
                + mg_value(mobility[Them] - mobility[Us])                    // (~0.5 Elo)
                - 873 * !pos.count<QUEEN>(Them)                              // (~24 Elo)
                - 100 * bool(attackedBy[Us][KNIGHT] & attackedBy[Us][KING])  // (~5 Elo)
                - 6 * mg_value(score) / 8                                    // (~8 Elo)
                - 4 * kingFlankDefense                                       // (~5 Elo)
                + 37;                                                        // (~0.5 Elo)

    // Transform the kingDanger units into a Score, and subtract it from the evaluation
    if (kingDanger > 100)
        score -= make_score(kingDanger * kingDanger / 4096, kingDanger / 16);

    // Penalty when our king is on a pawnless flank
    if (!(pos.pieces(PAWN) & KingFlank[file_of(ksq)]))
        score -= PawnlessFlank;

    // Penalty if king flank is under attack, potentially moving toward the king
    score -= FlankAttacks * kingFlankAttack;

    if constexpr (T)
        Trace::add(KING, Us, score);

    return score;
}


// Evaluation::threats() assigns bonuses according to the types of the
// attacking and the attacked pieces.

template<Trace::Tracing T>
template<Color Us>
ScoreForClassical Evaluation<T>::threats() const {

    constexpr Color     Them     = ~Us;
    constexpr Direction Up       = pawn_push(Us);
    constexpr Bitboard  TRank3BB = (Us == WHITE ? Rank3BB : Rank6BB);

    Bitboard          b, weak, defended, nonPawnEnemies, stronglyProtected, safe;
    ScoreForClassical score = SCORE_ZERO;

    // Non-pawn enemies
    nonPawnEnemies = pos.pieces(Them) & ~pos.pieces(PAWN);

    // Squares strongly protected by the enemy, either because they defend the
    // square with a pawn, or because they defend the square twice and we don't.
    stronglyProtected = attackedBy[Them][PAWN] | (attackedBy2[Them] & ~attackedBy2[Us]);

    // Non-pawn enemies, strongly protected
    defended = nonPawnEnemies & stronglyProtected;

    // Enemies not strongly protected and under our attack
    weak = pos.pieces(Them) & ~stronglyProtected & attackedBy[Us][ALL_PIECES];

    // Bonus according to the kind of attacking pieces
    if (defended | weak)
    {
        b = (defended | weak) & (attackedBy[Us][KNIGHT] | attackedBy[Us][BISHOP]);
        while (b)
            score += ThreatByMinor[type_of(pos.piece_on(pop_lsb(b)))];

        b = weak & attackedBy[Us][ROOK];
        while (b)
            score += ThreatByRook[type_of(pos.piece_on(pop_lsb(b)))];

        if (weak & attackedBy[Us][KING])
            score += ThreatByKing;

        b = ~attackedBy[Them][ALL_PIECES] | (nonPawnEnemies & attackedBy2[Us]);
        score += Hanging * popcount(weak & b);

        // Additional bonus if weak piece is only protected by a queen
        score += WeakQueenProtection * popcount(weak & attackedBy[Them][QUEEN]);
    }

    // Bonus for restricting their piece moves
    b = attackedBy[Them][ALL_PIECES] & ~stronglyProtected & attackedBy[Us][ALL_PIECES];
    score += RestrictedPiece * popcount(b);

    // Protected or unattacked squares
    safe = ~attackedBy[Them][ALL_PIECES] | attackedBy[Us][ALL_PIECES];

    // Bonus for attacking enemy pieces with our relatively safe pawns
    b = pos.pieces(Us, PAWN) & safe;
    b = pawn_attacks_bb<Us>(b) & nonPawnEnemies;
    score += ThreatBySafePawn * popcount(b);

    // Find squares where our pawns can push on the next move
    b = shift<Up>(pos.pieces(Us, PAWN)) & ~pos.pieces();
    b |= shift<Up>(b & TRank3BB) & ~pos.pieces();

    // Keep only the squares which are relatively safe
    b &= ~attackedBy[Them][PAWN] & safe;

    // Bonus for safe pawn threats on the next move
    b = pawn_attacks_bb<Us>(b) & nonPawnEnemies;
    score += ThreatByPawnPush * popcount(b);

    // Bonus for threats on the next moves against enemy queen
    if (pos.count<QUEEN>(Them) == 1)
    {
        bool queenImbalance = pos.count<QUEEN>() == 1;

        Square s = pos.square<QUEEN>(Them);
        safe     = mobilityArea[Us] & ~pos.pieces(Us, PAWN) & ~stronglyProtected;

        b = attackedBy[Us][KNIGHT] & attacks_bb<KNIGHT>(s);

        score += KnightOnQueen * popcount(b & safe) * (1 + queenImbalance);

        b = (attackedBy[Us][BISHOP] & attacks_bb<BISHOP>(s, pos.pieces()))
          | (attackedBy[Us][ROOK] & attacks_bb<ROOK>(s, pos.pieces()));

        score += SliderOnQueen * popcount(b & safe & attackedBy2[Us]) * (1 + queenImbalance);
    }

    if constexpr (T)
        Trace::add(THREAT, Us, score);

    return score;
}

// Evaluation::passed() evaluates the passed pawns and candidate passed
// pawns of the given color.

template<Trace::Tracing T>
template<Color Us>
ScoreForClassical Evaluation<T>::passed() const {

    constexpr Color     Them = ~Us;
    constexpr Direction Up   = pawn_push(Us);
    constexpr Direction Down = -Up;

    auto king_proximity = [&](Color c, Square s) {
        return std::min(distance(pos.square<KING>(c), s), 5);
    };

    Bitboard          b, bb, squaresToQueen, unsafeSquares, blockedPassers, helpers;
    ScoreForClassical score = SCORE_ZERO;

    b = pe->passed_pawns(Us);

    blockedPassers = b & shift<Down>(pos.pieces(Them, PAWN));
    if (blockedPassers)
    {
        helpers = shift<Up>(pos.pieces(Us, PAWN)) & ~pos.pieces(Them)
                & (~attackedBy2[Them] | attackedBy[Us][ALL_PIECES]);

        // Remove blocked candidate passers that don't have help to pass
        b &= ~blockedPassers | shift<WEST>(helpers) | shift<EAST>(helpers);
    }

    while (b)
    {
        Square s = pop_lsb(b);

        assert(!(pos.pieces(Them, PAWN) & forward_file_bb(Us, s + Up)));

        int r = relative_rank(Us, s);

        ScoreForClassical bonus = PassedRank[r];

        if (r > RANK_3)
        {
            int    w       = 5 * r - 13;
            Square blockSq = s + Up;

            // Adjust bonus based on the king's proximity
            bonus += make_score(
              0, (king_proximity(Them, blockSq) * 19 / 4 - king_proximity(Us, blockSq) * 2) * w);

            // If blockSq is not the queening square then consider also a second push
            if (r != RANK_7)
                bonus -= make_score(0, king_proximity(Us, blockSq + Up) * w);

            // If the pawn is free to advance, then increase the bonus
            if (pos.empty(blockSq))
            {
                squaresToQueen = forward_file_bb(Us, s);
                unsafeSquares  = passed_pawn_span(Us, s);

                bb = forward_file_bb(Them, s) & pos.pieces(ROOK, QUEEN);

                if (!(pos.pieces(Them) & bb))
                    unsafeSquares &= attackedBy[Them][ALL_PIECES] | pos.pieces(Them);

                // If there are no enemy pieces or attacks on passed pawn span, assign a big bonus.
                // Or if there is some, but they are all attacked by our pawns, assign a bit smaller bonus.
                // Otherwise assign a smaller bonus if the path to queen is not attacked
                // and even smaller bonus if it is attacked but block square is not.
                int k = !unsafeSquares                           ? 36
                      : !(unsafeSquares & ~attackedBy[Us][PAWN]) ? 30
                      : !(unsafeSquares & squaresToQueen)        ? 17
                      : !(unsafeSquares & blockSq)               ? 7
                                                                 : 0;

                // Assign a larger bonus if the block square is defended
                if ((pos.pieces(Us) & bb) || (attackedBy[Us][ALL_PIECES] & blockSq))
                    k += 5;

                bonus += make_score(k * w, k * w);
            }
        }  // r > RANK_3

        score += bonus - PassedFile * edge_distance(file_of(s));
    }

    if constexpr (T)
        Trace::add(PASSED, Us, score);

    return score;
}


// Evaluation::space() computes a space evaluation for a given side, aiming to improve game
// play in the opening. It is based on the number of safe squares on the four central files
// on ranks 2 to 4. Completely safe squares behind a friendly pawn are counted twice.
// Finally, the space bonus is multiplied by a weight which decreases according to occupancy.

template<Trace::Tracing T>
template<Color Us>
ScoreForClassical Evaluation<T>::space() const {

    // Early exit if, for example, both queens or 6 minor pieces have been exchanged
    if (pos.non_pawn_material() < SpaceThreshold)
        return SCORE_ZERO;

    constexpr Color     Them      = ~Us;
    constexpr Direction Down      = -pawn_push(Us);
    constexpr Bitboard  SpaceMask = Us == WHITE ? CenterFiles & (Rank2BB | Rank3BB | Rank4BB)
                                                : CenterFiles & (Rank7BB | Rank6BB | Rank5BB);

    // Find the available squares for our pieces inside the area defined by SpaceMask
    Bitboard safe = SpaceMask & ~pos.pieces(Us, PAWN) & ~attackedBy[Them][PAWN];

    // Find all squares which are at most three squares behind some friendly pawn
    Bitboard behind = pos.pieces(Us, PAWN);
    behind |= shift<Down>(behind);
    behind |= shift<Down + Down>(behind);

    // Compute space score based on the number of safe squares and number of our pieces
    // increased with number of total blocked pawns in position.
    int bonus  = popcount(safe) + popcount(behind & safe & ~attackedBy[Them][ALL_PIECES]);
    int weight = pos.count<ALL_PIECES>(Us) - 3 + std::min(pe->blocked_count(), 9);
    ScoreForClassical score = make_score(bonus * weight * weight / 16, 0);

    if constexpr (T)
        Trace::add(SPACE, Us, score);

    return score;
}


// Evaluation::winnable() adjusts the midgame and endgame score components, based on
// the known attacking/defending status of the players. The final value is derived
// by interpolation from the midgame and endgame values.

template<Trace::Tracing T>
Value Evaluation<T>::winnable(ScoreForClassical score) const {

    int outflanking = distance<File>(pos.square<KING>(WHITE), pos.square<KING>(BLACK))
                    + int(rank_of(pos.square<KING>(WHITE)) - rank_of(pos.square<KING>(BLACK)));

    bool pawnsOnBothFlanks = (pos.pieces(PAWN) & QueenSide) && (pos.pieces(PAWN) & KingSide);

    bool almostUnwinnable = outflanking < 0 && !pawnsOnBothFlanks;

    bool infiltration =
      rank_of(pos.square<KING>(WHITE)) > RANK_4 || rank_of(pos.square<KING>(BLACK)) < RANK_5;

    // Compute the initiative bonus for the attacking side
    int complexity = 9 * pe->passed_count() + 12 * pos.count<PAWN>() + 9 * outflanking
                   + 21 * pawnsOnBothFlanks + 24 * infiltration + 51 * !pos.non_pawn_material()
                   - 43 * almostUnwinnable - 110;

    Value mg = mg_value(score);
    Value eg = eg_value(score);

    // Now apply the bonus: note that we find the attacking side by extracting the
    // sign of the midgame or endgame values, and that we carefully cap the bonus
    // so that the midgame and endgame scores do not change sign after the bonus.
    int u = ((mg > 0) - (mg < 0)) * std::clamp(complexity + 50, -abs(mg), 0);
    int v = ((eg > 0) - (eg < 0)) * std::max(complexity, -abs(eg));

    //Handicap Mode begin
    if (Alexander::Eval::handicapConfig.winnableToEvaluate)
    {
        mg += u;
        eg += v;
    }

    //Handicap Mode end
    // Compute the scale factor for the winning side
    Color strongSide = eg > VALUE_DRAW ? WHITE : BLACK;
    int   sf         = me->scale_factor(pos, strongSide);

    // If scale factor is not already specific, scale up/down via general heuristics
    if (sf == SCALE_FACTOR_NORMAL)
    {
        if (pos.opposite_bishops())
        {
            // For pure opposite colored bishops endgames use scale factor
            // based on the number of passed pawns of the strong side.
            if (pos.non_pawn_material(WHITE) == BishopValueMg
                && pos.non_pawn_material(BLACK) == BishopValueMg)
                sf = 18 + 4 * popcount(pe->passed_pawns(strongSide));
            // For every other opposite colored bishops endgames use scale factor
            // based on the number of all pieces of the strong side.
            else
                sf = 22 + 3 * pos.count<ALL_PIECES>(strongSide);
        }
        // For rook endgames with strong side not having overwhelming pawn number advantage
        // and its pawns being on one flank and weak side protecting its pieces with a king
        // use lower scale factor.
        else if (pos.non_pawn_material(WHITE) == RookValueMg
                 && pos.non_pawn_material(BLACK) == RookValueMg
                 && pos.count<PAWN>(strongSide) - pos.count<PAWN>(~strongSide) <= 1
                 && bool(KingSide & pos.pieces(strongSide, PAWN))
                      != bool(QueenSide & pos.pieces(strongSide, PAWN))
                 && (attacks_bb<KING>(pos.square<KING>(~strongSide))
                     & pos.pieces(~strongSide, PAWN)))
            sf = 36;
        // For queen vs no queen endgames use scale factor
        // based on number of minors of side that doesn't have queen.
        else if (pos.count<QUEEN>() == 1)
            sf = 37
               + 3
                   * (pos.count<QUEEN>(WHITE) == 1
                        ? pos.count<BISHOP>(BLACK) + pos.count<KNIGHT>(BLACK)
                        : pos.count<BISHOP>(WHITE) + pos.count<KNIGHT>(WHITE));
        // In every other case use scale factor based on
        // the number of pawns of the strong side reduced if pawns are on a single flank.
        else
            sf = std::min(sf, 36 + 7 * pos.count<PAWN>(strongSide)) - 4 * !pawnsOnBothFlanks;

        // Reduce scale factor in case of pawns being on a single flank
        sf -= 4 * !pawnsOnBothFlanks;
    }

    ScoreForClassical w = make_score(mg, eg);
    w                   = apply_weights(w, WINNABLE_INDEX);  //handicap mode

    mg = mg_value(w);
    eg = eg_value(w);

    // Interpolate between the middlegame and (scaled by 'sf') endgame score
    v = mg * int(me->game_phase())
      + eg * int(PHASE_MIDGAME - me->game_phase()) * ScaleFactor(sf) / SCALE_FACTOR_NORMAL;
    v /= PHASE_MIDGAME;

    if constexpr (T)
    {
        Trace::add(WINNABLE, w);
        Trace::add(TOTAL,
                   make_score(mg * int(me->game_phase()), eg * int(PHASE_MIDGAME - me->game_phase())
                                                            * ScaleFactor(sf) / SCALE_FACTOR_NORMAL)
                     / PHASE_MIDGAME);
    }

    return Value(v);
}


// Evaluation::value() is the main function of the class. It computes the various
// parts of the evaluation and returns the value of the position from the point
// of view of the side to move.

template<Tracing T>
Value Evaluation<T>::value() {

    assert(!pos.checkers());

    // Probe the material hash table
    me = Material::probe(pos);

    // If we have a specialized evaluation function for the current material
    // configuration, call it and return.
    if (me->specialized_eval_exists())
        return me->evaluate(pos);

    // Initialize score by reading the incrementally updated scores included in
    // the position object (material + piece square tables) and the material
    // imbalance. ScoreForClassical is computed internally from the white point of view.
    //from handicap mode begin
    ScoreForClassical score = apply_weights(pos.psq_score(), MATERIAL_INDEX)
                            + (Alexander::Eval::handicapConfig.imbalancesToEvaluate
                                 ? apply_weights(me->imbalance(), IMBALANCE_INDEX)
                                 : 0);  //handicap mode
    // Probe the pawn hash table
    pe = Pawns::probe(pos);
    if (Alexander::Eval::handicapConfig.pawnsToEvaluate)
    {
        score += apply_weights(pe->pawn_score(WHITE) - pe->pawn_score(BLACK), PAWN_STRUCTURE_INDEX);
        if constexpr (T)
        {
            Trace::add(PAWN, pe->pawn_score(WHITE), pe->pawn_score(BLACK));
        }
    }

    // Early exit if score is high
    auto lazy_skip = [&](Value lazyThreshold) {
        return abs(mg_value(score) + eg_value(score))
             > lazyThreshold + std::abs(pos.this_thread()->bestValue) * 5 / 4
                 + pos.non_pawn_material() / 32;
    };

    if (lazy_skip(LazyThreshold1))
        goto make_v;

    // Main evaluation begins here
    initialize<WHITE>();
    initialize<BLACK>();

    //from handicap mode begin
    // Pieces evaluated first (also populates attackedBy, attackedBy2).
    // Note that the order of evaluation of the terms is left unspecified.
    {
        ScoreForClassical knights = pieces<WHITE, KNIGHT>() - pieces<BLACK, KNIGHT>();
        ScoreForClassical bishops = pieces<WHITE, BISHOP>() - pieces<BLACK, BISHOP>();
        ScoreForClassical rooks   = pieces<WHITE, ROOK>() - pieces<BLACK, ROOK>();
        ScoreForClassical queens  = pieces<WHITE, QUEEN>() - pieces<BLACK, QUEEN>();

        score += apply_weights(knights, KNIGHT_INDEX) + apply_weights(bishops, BISHOP_INDEX)
               + apply_weights(rooks, ROOK_INDEX) + apply_weights(queens, QUEEN_INDEX);

        // AGGIUNGI IL TRACING PER I PEZZI
        if constexpr (T)
        {
            Trace::add(KNIGHT, knights);
            Trace::add(BISHOP, bishops);
            Trace::add(ROOK, rooks);
            Trace::add(QUEEN, queens);
        }
    }
    {
        ScoreForClassical mobilityScore = mobility[WHITE] - mobility[BLACK];
        score += apply_weights(mobilityScore, MOBILITY_INDEX);

        if constexpr (T)
        {
            Trace::add(MOBILITY, mobilityScore);
        }
    }
    //from handicap mode end
    {
        ScoreForClassical kingSafetyScore = king<WHITE>() - king<BLACK>();
        score += apply_weights(kingSafetyScore, KING_SAFETY_INDEX);
        if constexpr (T)
        {
            Trace::add(KING, kingSafetyScore);
        }
    }
    //from handicap mode begin
    if (Alexander::Eval::handicapConfig.pawnsToEvaluate)
    {
        ScoreForClassical passedScore = passed<WHITE>() - passed<BLACK>();
        score += apply_weights(passedScore, PASSED_PAWN_INDEX);
        if constexpr (T)
        {
            Trace::add(PASSED, passedScore);
        }
    }

    //from handicap mode end
    if (lazy_skip(LazyThreshold2))
        goto make_v;

    {
        ScoreForClassical threatsScore = threats<WHITE>() - threats<BLACK>();
        score += apply_weights(threatsScore, THREATS_INDEX);
        if constexpr (T)
        {
            Trace::add(THREAT, threatsScore);
        }
    }
    {
        ScoreForClassical spaceScore = space<WHITE>() - space<BLACK>();
        score += apply_weights(spaceScore, SPACE_INDEX);
        if constexpr (T)
        {
            Trace::add(SPACE, spaceScore);
        }
    }
make_v:
    // Derive single value from mg and eg parts of score
    Value v = apply_weight<EG>(winnable(score), WINNABLE_INDEX);

    // In case of tracing add all remaining individual evaluation terms
    if constexpr (T)
    {
        Trace::add(MATERIAL, pos.psq_score());
        Trace::add(IMBALANCE, me->imbalance());
    }

    // Evaluation grain
    v = (v / 16) * 16;

    // Side to move point of view
    v = (pos.side_to_move() == WHITE ? v : -v);

    return v;
}
}
namespace Eval {
Value evaluate_position(const Position& pos) {
    assert(!pos.checkers());
    return Evaluation<NO_TRACE>(pos).value();
}
Value evaluate(const Position& pos) {

    assert(!pos.checkers());

    // Evaluate the position without trace
    Value v = Evaluation<NO_TRACE>(pos).value();

    // Damp down the evaluation linearly when shuffling (rule50 count)
    int shuffling = pos.rule50_count();
    v             = v * (200 - shuffling) / 214;

    // If the engine is in handicap mode
    if (handicapConfig.limitStrength && handicapConfig.simulateHumanBlunders)
    {
        v = get_perturbated_value(pos, v);
    }

    // Guarantee evaluation does not hit the tablebase range
    v = std::clamp(v, VALUE_TB_LOSS_IN_MAX_PLY + 1, VALUE_TB_WIN_IN_MAX_PLY - 1);

    return v;  // Returns final evaluation
}
}  // namespace Eval

// Like evaluate(), but instead of returning a value, it returns
// a string (suitable for outputting to stdout) that contains the detailed
// descriptions and values of each evaluation term. Useful for debugging.
// Trace scores are from white's point of view
std::string Eval::trace(Position& pos) {
    if (pos.checkers())
        return "Final evaluation: none (in check)";

    // Inizializza le strutture per la trace
    std::memset(Trace::scores, 0, sizeof(Trace::scores));
    std::memset(Trace::mobility_area_counts, 0, sizeof(Trace::mobility_area_counts));

    pos.this_thread()->bestValue = VALUE_ZERO;
    // Create evaluation object and compute value
    Evaluation<Trace::TRACE> eval(pos);
    Value                    v = eval.value();

    // Converti il valore al punto di vista del bianco
    Value       v_white     = (pos.side_to_move() == WHITE) ? v : -v;
    uint8_t     winProb     = WDLModel::get_win_probability(v_white, pos);
    std::string shashinZone = get_shashin_zone(winProb, WHITE);
    std::string gamePhase   = get_game_phase(pos);

    // Ottieni la fase di gioco per l'interpolazione
    Material::Entry* materialEntry = Material::probe(pos);
    int              phase         = materialEntry->game_phase();

    // Chiama la funzione di analisi handicap per generare l'output della trace
    return Eval::trace_analysis(pos, v_white, winProb, shashinZone, gamePhase, phase);
}
}  // namespace Alexander

// ============================================================================
// FILE ORIGINALE: evaluate.h
// ============================================================================

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

#ifndef EVALUATE_H_INCLUDED
#define EVALUATE_H_INCLUDED

#include "handicap/evaluate_handicap.h"
namespace Alexander {

class Position;
class OptionsMap;  //for classical

namespace Eval {
std::string trace(Position& pos);  //for classical
Value       evaluate(const Position& pos);
Value       evaluate_position(const Position& pos); //for trace
}  // namespace Eval

}  // namespace Alexander

#endif  // #ifndef EVALUATE_H_INCLUDED


// ============================================================================
// FILE ORIGINALE: history.h
// ============================================================================

/*
  Alexander, a UCI chess playing engine derived from Stockfish
  Copyright (C) 2004-2026 A.Manzo, F.Ferraguti, K.Kiniama and Alexander developers (see AUTHORS file)

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

#ifndef HISTORY_H_INCLUDED
#define HISTORY_H_INCLUDED

#include <algorithm>
#include <array>
#include <atomic>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <type_traits>  // IWYU pragma: keep

#include "memory.h"
#include "misc.h"
#include "position.h"

namespace Alexander {

constexpr int PAWN_HISTORY_BASE_SIZE   = 8192;  // has to be a power of 2
constexpr int UINT_16_HISTORY_SIZE     = std::numeric_limits<uint16_t>::max() + 1;
constexpr int CORRHIST_BASE_SIZE       = UINT_16_HISTORY_SIZE;
constexpr int CORRECTION_HISTORY_LIMIT = 1024;
constexpr int LOW_PLY_HISTORY_SIZE     = 5;

static_assert((PAWN_HISTORY_BASE_SIZE & (PAWN_HISTORY_BASE_SIZE - 1)) == 0,
              "PAWN_HISTORY_BASE_SIZE has to be a power of 2");

static_assert((CORRHIST_BASE_SIZE & (CORRHIST_BASE_SIZE - 1)) == 0,
              "CORRHIST_BASE_SIZE has to be a power of 2");

// StatsEntry is the container of various numerical statistics. We use a class
// instead of a naked value to directly call history update operator<<() on
// the entry. The first template parameter T is the base type of the array,
// and the second template parameter D limits the range of updates in [-D, D]
// when we update values with the << operator
template<typename T, int D, bool Atomic = false>
struct StatsEntry {

    static_assert(std::is_arithmetic_v<T>, "Not an arithmetic type");

   private:
    std::conditional_t<Atomic, std::atomic<T>, T> entry;

   public:
    void operator=(const T& v) {
        if constexpr (Atomic)
            entry.store(v, std::memory_order_relaxed);
        else
            entry = v;
    }

    operator T() const {
        if constexpr (Atomic)
            return entry.load(std::memory_order_relaxed);
        else
            return entry;
    }

    void operator<<(int bonus) {
        // Make sure that bonus is in range [-D, D]
        int clampedBonus = std::clamp(bonus, -D, D);
        T   val          = *this;
        *this            = val + clampedBonus - val * std::abs(clampedBonus) / D;

        assert(std::abs(T(*this)) <= D);
    }
};

enum StatsType {
    NoCaptures,
    Captures
};

template<typename T, int D, std::size_t... Sizes>
using Stats = MultiArray<StatsEntry<T, D>, Sizes...>;

template<typename T, int D, std::size_t... Sizes>
using AtomicStats = MultiArray<StatsEntry<T, D, true>, Sizes...>;

// DynStats is a dynamically sized array of Stats, used for thread-shared histories
// which should scale with the total number of threads. The SizeMultiplier gives
// the per-thread allocation count of T.
template<typename T, int SizeMultiplier>
struct DynStats {
    explicit DynStats(size_t s) {
        size = s * SizeMultiplier;
        data = make_unique_large_page<T[]>(size);
    }
    // Sets all values in the range to 0
    void clear_range(int value, size_t threadIdx, size_t numaTotal) {
        size_t start = uint64_t(threadIdx) * size / numaTotal;
        assert(start < size);
        size_t end = threadIdx + 1 == numaTotal ? size : uint64_t(threadIdx + 1) * size / numaTotal;

        while (start < end)
            data[start++].fill(value);
    }
    size_t get_size() const { return size; }
    T&     operator[](size_t index) {
        assert(index < size);
        return data.get()[index];
    }
    const T& operator[](size_t index) const {
        assert(index < size);
        return data.get()[index];
    }

   private:
    size_t            size;
    LargePagePtr<T[]> data;
};

// ButterflyHistory records how often quiet moves have been successful or unsuccessful
// during the current search, and is used for reduction and move ordering decisions.
// It uses 2 tables (one for each color) indexed by the move's from and to squares,
// see https://www.chessprogramming.org/Butterfly_Boards
using ButterflyHistory = Stats<std::int16_t, 7183, COLOR_NB, UINT_16_HISTORY_SIZE>;

// LowPlyHistory is addressed by ply and move's from and to squares, used
// to improve move ordering near the root
using LowPlyHistory = Stats<std::int16_t, 7183, LOW_PLY_HISTORY_SIZE, UINT_16_HISTORY_SIZE>;

// CapturePieceToHistory is addressed by a move's [piece][to][captured piece type]
using CapturePieceToHistory = Stats<std::int16_t, 10692, PIECE_NB, SQUARE_NB, PIECE_TYPE_NB>;

// PieceToHistory is like ButterflyHistory but is addressed by a move's [piece][to]
using PieceToHistory = Stats<std::int16_t, 30000, PIECE_NB, SQUARE_NB>;

// ContinuationHistory is the combined history of a given pair of moves, usually
// the current one given a previous one. The nested history table is based on
// PieceToHistory instead of ButterflyBoards.
using ContinuationHistory = MultiArray<PieceToHistory, PIECE_NB, SQUARE_NB>;

// PawnHistory is addressed by the pawn structure and a move's [piece][to]
using PawnHistory =
  DynStats<AtomicStats<std::int16_t, 8192, PIECE_NB, SQUARE_NB>, PAWN_HISTORY_BASE_SIZE>;

// Correction histories record differences between the static evaluation of
// positions and their search score. It is used to improve the static evaluation
// used by some search heuristics.
// see https://www.chessprogramming.org/Static_Evaluation_Correction_History
enum CorrHistType {
    Pawn,          // By color and pawn structure
    Minor,         // By color and positions of minor pieces (Knight, Bishop)
    NonPawn,       // By non-pawn material positions and color
    PieceTo,       // By [piece][to] move
    Continuation,  // Combined history of move pairs
};

template<typename T, int D>
struct CorrectionBundle {
    StatsEntry<T, D, true> pawn;
    StatsEntry<T, D, true> minor;
    StatsEntry<T, D, true> nonPawnWhite;
    StatsEntry<T, D, true> nonPawnBlack;

    void operator=(T val) {
        pawn         = val;
        minor        = val;
        nonPawnWhite = val;
        nonPawnBlack = val;
    }
};

namespace Detail {

template<CorrHistType>
struct CorrHistTypedef {
    using type =
      DynStats<Stats<std::int16_t, CORRECTION_HISTORY_LIMIT, COLOR_NB>, CORRHIST_BASE_SIZE>;
};

template<>
struct CorrHistTypedef<PieceTo> {
    using type = Stats<std::int16_t, CORRECTION_HISTORY_LIMIT, PIECE_NB, SQUARE_NB>;
};

template<>
struct CorrHistTypedef<Continuation> {
    using type = MultiArray<CorrHistTypedef<PieceTo>::type, PIECE_NB, SQUARE_NB>;
};

template<>
struct CorrHistTypedef<NonPawn> {
    using type = DynStats<Stats<std::int16_t, CORRECTION_HISTORY_LIMIT, COLOR_NB, COLOR_NB>,
                          CORRHIST_BASE_SIZE>;
};

}

using UnifiedCorrectionHistory =
  DynStats<MultiArray<CorrectionBundle<std::int16_t, CORRECTION_HISTORY_LIMIT>, COLOR_NB>,
           CORRHIST_BASE_SIZE>;

template<CorrHistType T>
using CorrectionHistory = typename Detail::CorrHistTypedef<T>::type;

using TTMoveHistory = StatsEntry<std::int16_t, 8192>;

// Set of histories shared between groups of threads. To avoid excessive
// cross-node data transfer, histories are shared only between threads
// on a given NUMA node. The passed size must be a power of two to make
// the indexing more efficient.
struct SharedHistories {
    SharedHistories(size_t threadCount) :
        correctionHistory(threadCount),
        pawnHistory(threadCount) {
        assert((threadCount & (threadCount - 1)) == 0 && threadCount != 0);
        sizeMinus1         = correctionHistory.get_size() - 1;
        pawnHistSizeMinus1 = pawnHistory.get_size() - 1;
    }

    size_t get_size() const { return sizeMinus1 + 1; }

    auto& pawn_entry(const Position& pos) {
        return pawnHistory[pos.pawn_key() & pawnHistSizeMinus1];
    }
    const auto& pawn_entry(const Position& pos) const {
        return pawnHistory[pos.pawn_key() & pawnHistSizeMinus1];
    }

    auto& pawn_correction_entry(const Position& pos) {
        return correctionHistory[pos.pawn_key() & sizeMinus1];
    }
    const auto& pawn_correction_entry(const Position& pos) const {
        return correctionHistory[pos.pawn_key() & sizeMinus1];
    }

    auto& minor_piece_correction_entry(const Position& pos) {
        return correctionHistory[pos.minor_piece_key() & sizeMinus1];
    }
    const auto& minor_piece_correction_entry(const Position& pos) const {
        return correctionHistory[pos.minor_piece_key() & sizeMinus1];
    }

    template<Color c>
    auto& nonpawn_correction_entry(const Position& pos) {
        return correctionHistory[pos.non_pawn_key(c) & sizeMinus1];
    }
    template<Color c>
    const auto& nonpawn_correction_entry(const Position& pos) const {
        return correctionHistory[pos.non_pawn_key(c) & sizeMinus1];
    }

    UnifiedCorrectionHistory correctionHistory;
    PawnHistory              pawnHistory;

   private:
    size_t sizeMinus1, pawnHistSizeMinus1;
};

}  // namespace Alexander

#endif  // #ifndef HISTORY_H_INCLUDED


// ============================================================================
// FILE ORIGINALE: main.cpp
// ============================================================================

/*
  Alexander, a UCI chess playing engine derived from Stockfish
  Copyright (C) 2004-2026 Alexander developers (see AUTHORS file)

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
//for classical begin
#include <unordered_map>
#include "evaluate.h"
#include "endgame.h"
#include "psqt.h"
//for classical end
#include "bitboard.h"
#include "misc.h"
#include "position.h"
#include "tune.h"
//wdl model
#include "uci.h"
#include "wdl/win_probability.h"
#include "learn/learn.h"  //learning
using namespace Alexander;

int main(int argc, char* argv[]) {

    std::cout << engine_info() << std::endl;

    WDLModel::init();  //wdl model

    Bitboards::init();
    Position::init();

    auto uci = std::make_unique<UCIEngine>(argc, argv);
    LD.init(uci->engine_options());  //learning
    Tune::init(uci->engine_options());
    //for classical begin
    Eval::initHandicapMode(uci->engine_options());
    Bitbases::init();
    Eval::loadAvatar(uci->engine_options()["Avatar File"]);  //handicap mode
    PSQT::init();
    Bitbases::init();
    Endgames::init();
    //for classical end
    uci->loop();

    return 0;
}


// ============================================================================
// FILE ORIGINALE: material.cpp
// ============================================================================

/*
  Alexander, a UCI chess playing engine derived from Stockfish
  Copyright (C) 2004-2023 The Stockfish developers (see AUTHORS file)

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

#include <cassert>
#include <cstring>  // For std::memset

#include "material.h"
#include "thread.h"

using namespace std;

namespace Alexander {

namespace {
#define S(mg, eg) make_score(mg, eg)

// Polynomial material imbalance parameters

// One ScoreForClassical parameter for each pair (our piece, another of our pieces)
constexpr ScoreForClassical QuadraticOurs[][PIECE_TYPE_NB] = {
  // OUR PIECE 2
  // bishop pair    pawn         knight       bishop       rook           queen
  {S(1419, 1455)},                                                  // Bishop pair
  {S(101, 28), S(37, 39)},                                          // Pawn
  {S(57, 64), S(249, 187), S(-49, -62)},                            // Knight      OUR PIECE 1
  {S(0, 0), S(118, 137), S(10, 27), S(0, 0)},                       // Bishop
  {S(-63, -68), S(-5, 3), S(100, 81), S(132, 118), S(-246, -244)},  // Rook
  {S(-210, -211), S(37, 14), S(147, 141), S(161, 105), S(-158, -174), S(-9, -31)}  // Queen
};

// One ScoreForClassical parameter for each pair (our piece, their piece)
constexpr ScoreForClassical QuadraticTheirs[][PIECE_TYPE_NB] = {
  // THEIR PIECE
  // bishop pair   pawn         knight       bishop       rook         queen
  {},                                                              // Bishop pair
  {S(33, 30)},                                                     // Pawn
  {S(46, 18), S(106, 84)},                                         // Knight      OUR PIECE
  {S(75, 35), S(59, 44), S(60, 15)},                               // Bishop
  {S(26, 35), S(6, 22), S(38, 39), S(-12, -2)},                    // Rook
  {S(97, 93), S(100, 163), S(-58, -91), S(112, 192), S(276, 225)}  // Queen
};

#undef S

// Endgame evaluation and scaling functions are accessed directly and not through
// the function maps because they correspond to more than one material hash key.
Endgame<KXK> EvaluateKXK[] = {Endgame<KXK>(WHITE), Endgame<KXK>(BLACK)};

Endgame<KBPsK>  ScaleKBPsK[]  = {Endgame<KBPsK>(WHITE), Endgame<KBPsK>(BLACK)};
Endgame<KQKRPs> ScaleKQKRPs[] = {Endgame<KQKRPs>(WHITE), Endgame<KQKRPs>(BLACK)};
Endgame<KPsK>   ScaleKPsK[]   = {Endgame<KPsK>(WHITE), Endgame<KPsK>(BLACK)};
Endgame<KPKP>   ScaleKPKP[]   = {Endgame<KPKP>(WHITE), Endgame<KPKP>(BLACK)};

// Helper used to detect a given material distribution
bool is_KXK(const Position& pos, Color us) {
    return !more_than_one(pos.pieces(~us)) && pos.non_pawn_material(us) >= RookValueMg;
}

bool is_KBPsK(const Position& pos, Color us) {
    return pos.non_pawn_material(us) == BishopValueMg && pos.count<PAWN>(us) >= 1;
}

bool is_KQKRPs(const Position& pos, Color us) {
    return !pos.count<PAWN>(us) && pos.non_pawn_material(us) == QueenValueMg
        && pos.count<ROOK>(~us) == 1 && pos.count<PAWN>(~us) >= 1;
}


/// imbalance() calculates the imbalance by comparing the piece count of each
/// piece type for both colors.

template<Color Us>
ScoreForClassical imbalance(const int pieceCount[][PIECE_TYPE_NB]) {

    constexpr Color Them = ~Us;

    ScoreForClassical bonus = SCORE_ZERO;

    // Second-degree polynomial material imbalance, by Tord Romstad
    for (int pt1 = NO_PIECE_TYPE; pt1 <= QUEEN; ++pt1)
    {
        if (!pieceCount[Us][pt1])
            continue;

        int v = QuadraticOurs[pt1][pt1] * pieceCount[Us][pt1];

        for (int pt2 = NO_PIECE_TYPE; pt2 < pt1; ++pt2)
            v += QuadraticOurs[pt1][pt2] * pieceCount[Us][pt2]
               + QuadraticTheirs[pt1][pt2] * pieceCount[Them][pt2];

        bonus += pieceCount[Us][pt1] * v;
    }

    return bonus;
}

}  // namespace

namespace Material {


/// Material::probe() looks up the current position's material configuration in
/// the material hash table. It returns a pointer to the Entry if the position
/// is found. Otherwise a new Entry is computed and stored there, so we don't
/// have to recompute all when the same material configuration occurs again.

Entry* probe(const Position& pos) {

    Key    key = pos.material_key();
    Entry* e   = pos.this_thread()->materialTable[key];

    if (e->key == key)
        return e;

    std::memset(e, 0, sizeof(Entry));
    e->key           = key;
    e->factor[WHITE] = e->factor[BLACK] = (uint8_t) SCALE_FACTOR_NORMAL;

    Value npm_w = pos.non_pawn_material(WHITE);
    Value npm_b = pos.non_pawn_material(BLACK);
    Value npm   = std::clamp(npm_w + npm_b, EndgameLimit, MidgameLimit);

    // Map total non-pawn material into [PHASE_ENDGAME, PHASE_MIDGAME]
    e->gamePhase = Phase(((npm - EndgameLimit) * PHASE_MIDGAME) / (MidgameLimit - EndgameLimit));

    // Let's look if we have a specialized evaluation function for this particular
    // material configuration. Firstly we look for a fixed configuration one, then
    // for a generic one if the previous search failed.
    if ((e->evaluationFunction = Endgames::probe<Value>(key)) != nullptr)
        return e;

    for (Color c : {WHITE, BLACK})
        if (is_KXK(pos, c))
        {
            e->evaluationFunction = &EvaluateKXK[c];
            return e;
        }

    // OK, we didn't find any special evaluation function for the current material
    // configuration. Is there a suitable specialized scaling function?
    const auto* sf = Endgames::probe<ScaleFactor>(key);

    if (sf)
    {
        e->scalingFunction[sf->strongSide] = sf;  // Only strong color assigned
        return e;
    }

    // We didn't find any specialized scaling function, so fall back on generic
    // ones that refer to more than one material distribution. Note that in this
    // case we don't return after setting the function.
    for (Color c : {WHITE, BLACK})
    {
        if (is_KBPsK(pos, c))
            e->scalingFunction[c] = &ScaleKBPsK[c];

        else if (is_KQKRPs(pos, c))
            e->scalingFunction[c] = &ScaleKQKRPs[c];
    }

    if (npm_w + npm_b == VALUE_ZERO && pos.pieces(PAWN))  // Only pawns on the board
    {
        if (!pos.count<PAWN>(BLACK))
        {
            assert(pos.count<PAWN>(WHITE) >= 2);

            e->scalingFunction[WHITE] = &ScaleKPsK[WHITE];
        }
        else if (!pos.count<PAWN>(WHITE))
        {
            assert(pos.count<PAWN>(BLACK) >= 2);

            e->scalingFunction[BLACK] = &ScaleKPsK[BLACK];
        }
        else if (pos.count<PAWN>(WHITE) == 1 && pos.count<PAWN>(BLACK) == 1)
        {
            // This is a special case because we set scaling functions
            // for both colors instead of only one.
            e->scalingFunction[WHITE] = &ScaleKPKP[WHITE];
            e->scalingFunction[BLACK] = &ScaleKPKP[BLACK];
        }
    }

    // Zero or just one pawn makes it difficult to win, even with a small material
    // advantage. This catches some trivial draws like KK, KBK and KNK and gives a
    // drawish scale factor for cases such as KRKBP and KmmKm (except for KBBKN).
    if (!pos.count<PAWN>(WHITE) && npm_w - npm_b <= BishopValueMg)
        e->factor[WHITE] = uint8_t(npm_w < RookValueMg      ? SCALE_FACTOR_DRAW
                                   : npm_b <= BishopValueMg ? 4
                                                            : 14);

    if (!pos.count<PAWN>(BLACK) && npm_b - npm_w <= BishopValueMg)
        e->factor[BLACK] = uint8_t(npm_b < RookValueMg      ? SCALE_FACTOR_DRAW
                                   : npm_w <= BishopValueMg ? 4
                                                            : 14);

    // Evaluate the material imbalance. We use PIECE_TYPE_NONE as a place holder
    // for the bishop pair "extended piece", which allows us to be more flexible
    // in defining bishop pair bonuses.
    const int pieceCount[COLOR_NB][PIECE_TYPE_NB] = {
      {pos.count<BISHOP>(WHITE) > 1, pos.count<PAWN>(WHITE), pos.count<KNIGHT>(WHITE),
       pos.count<BISHOP>(WHITE), pos.count<ROOK>(WHITE), pos.count<QUEEN>(WHITE)},
      {pos.count<BISHOP>(BLACK) > 1, pos.count<PAWN>(BLACK), pos.count<KNIGHT>(BLACK),
       pos.count<BISHOP>(BLACK), pos.count<ROOK>(BLACK), pos.count<QUEEN>(BLACK)}};

    e->score = (imbalance<WHITE>(pieceCount) - imbalance<BLACK>(pieceCount)) / 16;
    return e;
}

}  // namespace Material

}  // namespace Alexander


// ============================================================================
// FILE ORIGINALE: material.h
// ============================================================================

/*
  Alexander, a UCI chess playing engine derived from Stockfish
  Copyright (C) 2004-2023 The Stockfish developers (see AUTHORS file)

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

#ifndef MATERIAL_H_INCLUDED
#define MATERIAL_H_INCLUDED

#include "endgame.h"
#include "misc.h"
#include "position.h"
#include "types.h"

namespace Alexander::Material {

/// Material::Entry contains various information about a material configuration.
/// It contains a material imbalance evaluation, a function pointer to a special
/// endgame evaluation function (which in most cases is nullptr, meaning that the
/// standard evaluation function will be used), and scale factors.
///
/// The scale factors are used to scale the evaluation score up or down. For
/// instance, in KRB vs KR endgames, the score is scaled down by a factor of 4,
/// which will result in scores of absolute value less than one pawn.

struct Entry {

    ScoreForClassical imbalance() const { return score; }
    Phase             game_phase() const { return (Phase) gamePhase; }
    bool              specialized_eval_exists() const { return evaluationFunction != nullptr; }
    Value             evaluate(const Position& pos) const { return (*evaluationFunction)(pos); }

    // scale_factor() takes a position and a color as input and returns a scale factor
    // for the given color. We have to provide the position in addition to the color
    // because the scale factor may also be a function which should be applied to
    // the position. For instance, in KBP vs K endgames, the scaling function looks
    // for rook pawns and wrong-colored bishops.
    ScaleFactor scale_factor(const Position& pos, Color c) const {
        ScaleFactor sf = scalingFunction[c] ? (*scalingFunction[c])(pos) : SCALE_FACTOR_NONE;
        return sf != SCALE_FACTOR_NONE ? sf : ScaleFactor(factor[c]);
    }

    Key                             key;
    const EndgameBase<Value>*       evaluationFunction;
    const EndgameBase<ScaleFactor>* scalingFunction[COLOR_NB];  // Could be one for each
                                                                // side (e.g. KPKP, KBPsK)
    ScoreForClassical score;
    int16_t           gamePhase;
    uint8_t           factor[COLOR_NB];
};

using Table = HashTable<Entry, 8192>;

Entry* probe(const Position& pos);

}  // namespace Alexander::Material

#endif  // #ifndef MATERIAL_H_INCLUDED


// ============================================================================
// FILE ORIGINALE: memory.cpp
// ============================================================================

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

#include "memory.h"

#include <cstdlib>

#if __has_include("features.h")
    #include <features.h>
#endif

#if defined(__linux__) && !defined(__ANDROID__)
    #include <sys/mman.h>
#endif

#if defined(__APPLE__) || defined(__ANDROID__) || defined(__OpenBSD__) \
  || (defined(__GLIBCXX__) && !defined(_GLIBCXX_HAVE_ALIGNED_ALLOC) && !defined(_WIN32)) \
  || defined(__e2k__)
    #define POSIXALIGNEDALLOC
    #include <stdlib.h>
#endif

#ifdef _WIN32
    #if _WIN32_WINNT < 0x0601
        #undef _WIN32_WINNT
        #define _WIN32_WINNT 0x0601  // Force to include needed API prototypes
    #endif

    #ifndef NOMINMAX
        #define NOMINMAX
    #endif

    #include <ios>       // std::hex, std::dec
    #include <iostream>  // std::cerr
    #include <ostream>   // std::endl
    #include <windows.h>

// The needed Windows API for processor groups could be missed from old Windows
// versions, so instead of calling them directly (forcing the linker to resolve
// the calls at compile time), try to load them at runtime. To do this we need
// first to define the corresponding function pointers.

#endif


namespace Alexander {

// Wrappers for systems where the c++17 implementation does not guarantee the
// availability of aligned_alloc(). Memory allocated with std_aligned_alloc()
// must be freed with std_aligned_free().

void* std_aligned_alloc(size_t alignment, size_t size) {
#if defined(_ISOC11_SOURCE)
    return aligned_alloc(alignment, size);
#elif defined(POSIXALIGNEDALLOC)
    void* mem = nullptr;
    posix_memalign(&mem, alignment, size);
    return mem;
#elif defined(_WIN32) && !defined(_M_ARM) && !defined(_M_ARM64)
    return _mm_malloc(size, alignment);
#elif defined(_WIN32)
    return _aligned_malloc(size, alignment);
#else
    return std::aligned_alloc(alignment, size);
#endif
}

void std_aligned_free(void* ptr) {

#if defined(POSIXALIGNEDALLOC)
    free(ptr);
#elif defined(_WIN32) && !defined(_M_ARM) && !defined(_M_ARM64)
    _mm_free(ptr);
#elif defined(_WIN32)
    _aligned_free(ptr);
#else
    free(ptr);
#endif
}

// aligned_large_pages_alloc() will return suitably aligned memory,
// if possible using large pages.

#if defined(_WIN32)

static void* aligned_large_pages_alloc_windows([[maybe_unused]] size_t allocSize) {

    return windows_try_with_large_page_priviliges(
      [&](size_t largePageSize) {
          // Round up size to full pages and allocate
          allocSize = (allocSize + largePageSize - 1) & ~size_t(largePageSize - 1);
          return VirtualAlloc(nullptr, allocSize, MEM_RESERVE | MEM_COMMIT | MEM_LARGE_PAGES,
                              PAGE_READWRITE);
      },
      []() { return (void*) nullptr; });
}

void* aligned_large_pages_alloc(size_t allocSize) {

    // Try to allocate large pages
    void* mem = aligned_large_pages_alloc_windows(allocSize);

    // Fall back to regular, page-aligned, allocation if necessary
    if (!mem)
        mem = VirtualAlloc(nullptr, allocSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    return mem;
}

#else

void* aligned_large_pages_alloc(size_t allocSize) {

    #if defined(__linux__)
    constexpr size_t alignment = 2 * 1024 * 1024;  // 2MB page size assumed
    #else
    constexpr size_t alignment = 4096;  // small page size assumed
    #endif

    // Round up to multiples of alignment
    size_t size = ((allocSize + alignment - 1) / alignment) * alignment;
    void*  mem  = std_aligned_alloc(alignment, size);
    #if defined(MADV_HUGEPAGE)
    madvise(mem, size, MADV_HUGEPAGE);
    #endif
    return mem;
}

#endif

bool has_large_pages() {

#if defined(_WIN32)

    constexpr size_t page_size = 2 * 1024 * 1024;  // 2MB page size assumed
    void*            mem       = aligned_large_pages_alloc_windows(page_size);
    if (mem == nullptr)
    {
        return false;
    }
    else
    {
        aligned_large_pages_free(mem);
        return true;
    }

#elif defined(__linux__)

    #if defined(MADV_HUGEPAGE)
    return true;
    #else
    return false;
    #endif

#else

    return false;

#endif
}


// aligned_large_pages_free() will free the previously memory allocated
// by aligned_large_pages_alloc(). The effect is a nop if mem == nullptr.

#if defined(_WIN32)

void aligned_large_pages_free(void* mem) {

    if (mem && !VirtualFree(mem, 0, MEM_RELEASE))
    {
        DWORD err = GetLastError();
        std::cerr << "Failed to free large page memory. Error code: 0x" << std::hex << err
                  << std::dec << std::endl;
        exit(EXIT_FAILURE);
    }
}

#else

void aligned_large_pages_free(void* mem) { std_aligned_free(mem); }

#endif
}  // namespace Alexander


// ============================================================================
// FILE ORIGINALE: memory.h
// ============================================================================

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

#ifndef MEMORY_H_INCLUDED
#define MEMORY_H_INCLUDED

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <new>
#include <type_traits>
#include <utility>

#include "types.h"

#if defined(_WIN64)

    #if _WIN32_WINNT < 0x0601
        #undef _WIN32_WINNT
        #define _WIN32_WINNT 0x0601  // Force to include needed API prototypes
    #endif

    #if !defined(NOMINMAX)
        #define NOMINMAX
    #endif
    #include <windows.h>

    // Some Windows headers (RPC/old headers) define short macros such
    // as 'small' expanding to 'char', which breaks identifiers in the code.
    // Undefine those macros immediately after including <windows.h>.
    #ifdef small
        #undef small
    #endif

    #include <psapi.h>

extern "C" {
using OpenProcessToken_t      = bool (*)(HANDLE, DWORD, PHANDLE);
using LookupPrivilegeValueA_t = bool (*)(LPCSTR, LPCSTR, PLUID);
using AdjustTokenPrivileges_t =
  bool (*)(HANDLE, BOOL, PTOKEN_PRIVILEGES, DWORD, PTOKEN_PRIVILEGES, PDWORD);
}
#endif


namespace Alexander {

void* std_aligned_alloc(size_t alignment, size_t size);
void  std_aligned_free(void* ptr);

// Memory aligned by page size, min alignment: 4096 bytes
void* aligned_large_pages_alloc(size_t size);
void  aligned_large_pages_free(void* mem);

bool has_large_pages();

// Frees memory which was placed there with placement new.
// Works for both single objects and arrays of unknown bound.
template<typename T, typename FREE_FUNC>
void memory_deleter(T* ptr, FREE_FUNC free_func) {
    if (!ptr)
        return;

    // Explicitly needed to call the destructor
    if constexpr (!std::is_trivially_destructible_v<T>)
        ptr->~T();

    free_func(ptr);
}

// Frees memory which was placed there with placement new.
// Works for both single objects and arrays of unknown bound.
template<typename T, typename FREE_FUNC>
void memory_deleter_array(T* ptr, FREE_FUNC free_func) {
    if (!ptr)
        return;


    // Move back on the pointer to where the size is allocated
    const size_t array_offset = std::max(sizeof(size_t), alignof(T));
    char*        raw_memory   = reinterpret_cast<char*>(ptr) - array_offset;

    if constexpr (!std::is_trivially_destructible_v<T>)
    {
        const size_t size = *reinterpret_cast<size_t*>(raw_memory);

        // Explicitly call the destructor for each element in reverse order
        for (size_t i = size; i-- > 0;)
            ptr[i].~T();
    }

    free_func(raw_memory);
}

// Allocates memory for a single object and places it there with placement new
template<typename T, typename ALLOC_FUNC, typename... Args>
inline std::enable_if_t<!std::is_array_v<T>, T*> memory_allocator(ALLOC_FUNC alloc_func,
                                                                  Args&&... args) {
    void* raw_memory = alloc_func(sizeof(T));
    ASSERT_ALIGNED(raw_memory, alignof(T));
    return new (raw_memory) T(std::forward<Args>(args)...);
}

// Allocates memory for an array of unknown bound and places it there with placement new
template<typename T, typename ALLOC_FUNC>
inline std::enable_if_t<std::is_array_v<T>, std::remove_extent_t<T>*>
memory_allocator(ALLOC_FUNC alloc_func, size_t num) {
    using ElementType = std::remove_extent_t<T>;

    const size_t array_offset = std::max(sizeof(size_t), alignof(ElementType));

    // Save the array size in the memory location
    char* raw_memory =
      reinterpret_cast<char*>(alloc_func(array_offset + num * sizeof(ElementType)));
    ASSERT_ALIGNED(raw_memory, alignof(T));

    new (raw_memory) size_t(num);

    for (size_t i = 0; i < num; ++i)
        new (raw_memory + array_offset + i * sizeof(ElementType)) ElementType();

    // Need to return the pointer at the start of the array so that
    // the indexing in unique_ptr<T[]> works.
    return reinterpret_cast<ElementType*>(raw_memory + array_offset);
}

//
//
// aligned large page unique ptr
//
//

template<typename T>
struct LargePageDeleter {
    void operator()(T* ptr) const { return memory_deleter<T>(ptr, aligned_large_pages_free); }
};

template<typename T>
struct LargePageArrayDeleter {
    void operator()(T* ptr) const { return memory_deleter_array<T>(ptr, aligned_large_pages_free); }
};

template<typename T>
using LargePagePtr =
  std::conditional_t<std::is_array_v<T>,
                     std::unique_ptr<T, LargePageArrayDeleter<std::remove_extent_t<T>>>,
                     std::unique_ptr<T, LargePageDeleter<T>>>;

// make_unique_large_page for single objects
template<typename T, typename... Args>
std::enable_if_t<!std::is_array_v<T>, LargePagePtr<T>> make_unique_large_page(Args&&... args) {
    static_assert(alignof(T) <= 4096,
                  "aligned_large_pages_alloc() may fail for such a big alignment requirement of T");

    T* obj = memory_allocator<T>(aligned_large_pages_alloc, std::forward<Args>(args)...);

    return LargePagePtr<T>(obj);
}

// make_unique_large_page for arrays of unknown bound
template<typename T>
std::enable_if_t<std::is_array_v<T>, LargePagePtr<T>> make_unique_large_page(size_t num) {
    using ElementType = std::remove_extent_t<T>;

    static_assert(alignof(ElementType) <= 4096,
                  "aligned_large_pages_alloc() may fail for such a big alignment requirement of T");

    ElementType* memory = memory_allocator<T>(aligned_large_pages_alloc, num);

    return LargePagePtr<T>(memory);
}

//
//
// aligned unique ptr
//
//

template<typename T>
struct AlignedDeleter {
    void operator()(T* ptr) const { return memory_deleter<T>(ptr, std_aligned_free); }
};

template<typename T>
struct AlignedArrayDeleter {
    void operator()(T* ptr) const { return memory_deleter_array<T>(ptr, std_aligned_free); }
};

template<typename T>
using AlignedPtr =
  std::conditional_t<std::is_array_v<T>,
                     std::unique_ptr<T, AlignedArrayDeleter<std::remove_extent_t<T>>>,
                     std::unique_ptr<T, AlignedDeleter<T>>>;

// make_unique_aligned for single objects
template<typename T, typename... Args>
std::enable_if_t<!std::is_array_v<T>, AlignedPtr<T>> make_unique_aligned(Args&&... args) {
    const auto func = [](size_t size) { return std_aligned_alloc(alignof(T), size); };
    T*         obj  = memory_allocator<T>(func, std::forward<Args>(args)...);

    return AlignedPtr<T>(obj);
}

// make_unique_aligned for arrays of unknown bound
template<typename T>
std::enable_if_t<std::is_array_v<T>, AlignedPtr<T>> make_unique_aligned(size_t num) {
    using ElementType = std::remove_extent_t<T>;

    const auto   func   = [](size_t size) { return std_aligned_alloc(alignof(ElementType), size); };
    ElementType* memory = memory_allocator<T>(func, num);

    return AlignedPtr<T>(memory);
}


// Get the first aligned element of an array.
// ptr must point to an array of size at least `sizeof(T) * N + alignment` bytes,
// where N is the number of elements in the array.
template<uintptr_t Alignment, typename T>
T* align_ptr_up(T* ptr) {
    static_assert(alignof(T) < Alignment);

    const uintptr_t ptrint = reinterpret_cast<uintptr_t>(reinterpret_cast<char*>(ptr));
    return reinterpret_cast<T*>(
      reinterpret_cast<char*>((ptrint + (Alignment - 1)) / Alignment * Alignment));
}

#if defined(_WIN32)

template<typename FuncYesT, typename FuncNoT>
auto windows_try_with_large_page_priviliges([[maybe_unused]] FuncYesT&& fyes, FuncNoT&& fno) {

    #if !defined(_WIN64)
    return fno();
    #else

    HANDLE hProcessToken{};
    LUID   luid{};

    const size_t largePageSize = GetLargePageMinimum();
    if (!largePageSize)
        return fno();

    // Dynamically link OpenProcessToken, LookupPrivilegeValue and AdjustTokenPrivileges

    HMODULE hAdvapi32 = GetModuleHandle(TEXT("advapi32.dll"));

    if (!hAdvapi32)
        hAdvapi32 = LoadLibrary(TEXT("advapi32.dll"));

    auto OpenProcessToken_f =
      OpenProcessToken_t((void (*)()) GetProcAddress(hAdvapi32, "OpenProcessToken"));
    if (!OpenProcessToken_f)
        return fno();
    auto LookupPrivilegeValueA_f =
      LookupPrivilegeValueA_t((void (*)()) GetProcAddress(hAdvapi32, "LookupPrivilegeValueA"));
    if (!LookupPrivilegeValueA_f)
        return fno();
    auto AdjustTokenPrivileges_f =
      AdjustTokenPrivileges_t((void (*)()) GetProcAddress(hAdvapi32, "AdjustTokenPrivileges"));
    if (!AdjustTokenPrivileges_f)
        return fno();

    // We need SeLockMemoryPrivilege, so try to enable it for the process

    if (!OpenProcessToken_f(  // OpenProcessToken()
          GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hProcessToken))
        return fno();

    if (!LookupPrivilegeValueA_f(nullptr, "SeLockMemoryPrivilege", &luid))
        return fno();

    TOKEN_PRIVILEGES tp{};
    TOKEN_PRIVILEGES prevTp{};
    DWORD            prevTpLen = 0;

    tp.PrivilegeCount           = 1;
    tp.Privileges[0].Luid       = luid;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    // Try to enable SeLockMemoryPrivilege. Note that even if AdjustTokenPrivileges()
    // succeeds, we still need to query GetLastError() to ensure that the privileges
    // were actually obtained.

    if (!AdjustTokenPrivileges_f(hProcessToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), &prevTp,
                                 &prevTpLen)
        || GetLastError() != ERROR_SUCCESS)
        return fno();

    auto&& ret = fyes(largePageSize);

    // Privilege no longer needed, restore previous state
    AdjustTokenPrivileges_f(hProcessToken, FALSE, &prevTp, 0, nullptr, nullptr);

    CloseHandle(hProcessToken);

    return std::forward<decltype(ret)>(ret);

    #endif
}

#endif

}  // namespace Alexander

#endif  // #ifndef MEMORY_H_INCLUDED

// ============================================================================
// FILE ORIGINALE: misc.cpp
// ============================================================================

/*
  Alexander, a UCI chess playing engine derived from Stockfish
  Copyright (C) 2004-2026 Andrea Manzo, F. Ferraguti, K.Kiniama and Alexander developers (see AUTHORS file)

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

#include "misc.h"

#include <array>
#include <atomic>
#include <cassert>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <limits>
#include <mutex>
#include <sstream>
//not used string_view: improved from Stockfish
//from Alexander begin
#include <algorithm>
#include <stdarg.h>
//from Alexander end

#include "types.h"

namespace Alexander {

namespace {

// Version number or dev.
constexpr std::string_view version = "8.3";

// Our fancy logging facility. The trick here is to replace cin.rdbuf() and
// cout.rdbuf() with two Tie objects that tie cin and cout to a file stream. We
// can toggle the logging of std::cout and std:cin at runtime whilst preserving
// usual I/O functionality, all without changing a single line of code!
// Idea from http://groups.google.com/group/comp.lang.c++/msg/1d941c0f26ea0d81

struct Tie: public std::streambuf {  // MSVC requires split streambuf for cin and cout

    Tie(std::streambuf* b, std::streambuf* l) :
        buf(b),
        logBuf(l) {}

    int sync() override { return logBuf->pubsync(), buf->pubsync(); }
    int overflow(int c) override { return log(buf->sputc(char(c)), "<< "); }
    int underflow() override { return buf->sgetc(); }
    int uflow() override { return log(buf->sbumpc(), ">> "); }

    std::streambuf *buf, *logBuf;

    int log(int c, const char* prefix) {

        static int last = '\n';  // Single log file

        if (last == '\n')
            logBuf->sputn(prefix, 3);

        return last = logBuf->sputc(char(c));
    }
};

class Logger {

    Logger() :
        in(std::cin.rdbuf(), file.rdbuf()),
        out(std::cout.rdbuf(), file.rdbuf()) {}
    ~Logger() { start(""); }

    std::ofstream file;
    Tie           in, out;

   public:
    static void start(const std::string& fname) {

        static Logger l;

        if (l.file.is_open())
        {
            std::cout.rdbuf(l.out.buf);
            std::cin.rdbuf(l.in.buf);
            l.file.close();
        }

        if (!fname.empty())
        {
            l.file.open(fname, std::ifstream::out);

            if (!l.file.is_open())
            {
                std::cerr << "Unable to open debug log file " << fname << std::endl;
                exit(EXIT_FAILURE);
            }

            std::cin.rdbuf(&l.in);
            std::cout.rdbuf(&l.out);
        }
    }
};

}  // namespace


// Returns the full name of the current Alexander version.
//
// For local dev compiles we try to append the commit SHA and
// commit date from git. If that fails only the local compilation
// date is set and "nogit" is specified:
//      Alexander dev-YYYYMMDD-SHA
//      or
//      Alexander dev-YYYYMMDD-nogit
//
// For releases (non-dev builds) we only include the version number:
//      Alexander version
std::string engine_version_info() {
    std::stringstream ss;
    ss << "Alexander " << version << std::setfill('0');

    if constexpr (version == "dev")
    {
        ss << "-";
#ifdef GIT_DATE
        ss << stringify(GIT_DATE);
#else
        constexpr std::string_view months("Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec");
        std::string                month, day, year;
        std::stringstream          date(__DATE__);  // From compiler, format is "Sep 21 2008"

        date >> month >> day >> year;
        ss << year << std::setw(2) << std::setfill('0') << (1 + months.find(month) / 4)
           << std::setw(2) << std::setfill('0') << day;
#endif

        ss << "-";

#ifdef GIT_SHA
        ss << stringify(GIT_SHA);
#else
        ss << "nogit";
#endif
    }

// --- NOME PERSONALIZZATO PER GOLD DIGGER ---
#ifdef GOLD_DIGGER
    ss << " GoldDigger";
#endif
    // -------------------------------------------
    return ss.str();
}

std::string engine_info(bool to_uci) {
    return engine_version_info() + (to_uci ? "\nid author " : " by ")
         + "the Alexander developers (see AUTHORS file)";
}


// Returns a string trying to describe the compiler we use
std::string compiler_info() {

#define make_version_string(major, minor, patch) \
    stringify(major) "." stringify(minor) "." stringify(patch)

    // Predefined macros hell:
    //
    // __GNUC__                Compiler is GCC, Clang or ICX
    // __clang__               Compiler is Clang or ICX
    // __INTEL_LLVM_COMPILER   Compiler is ICX
    // _MSC_VER                Compiler is MSVC
    // _WIN32                  Building on Windows (any)
    // _WIN64                  Building on Windows 64 bit

    std::string compiler = "\nCompiled by                : ";

#if defined(__INTEL_LLVM_COMPILER)
    compiler += "ICX ";
    compiler += stringify(__INTEL_LLVM_COMPILER);
#elif defined(__clang__)
    compiler += "clang++ ";
    compiler += make_version_string(__clang_major__, __clang_minor__, __clang_patchlevel__);
#elif _MSC_VER
    compiler += "MSVC ";
    compiler += "(version ";
    compiler += stringify(_MSC_FULL_VER) "." stringify(_MSC_BUILD);
    compiler += ")";
#elif defined(__e2k__) && defined(__LCC__)
    #define dot_ver2(n) \
        compiler += char('.'); \
        compiler += char('0' + (n) / 10); \
        compiler += char('0' + (n) % 10);

    compiler += "MCST LCC ";
    compiler += "(version ";
    compiler += std::to_string(__LCC__ / 100);
    dot_ver2(__LCC__ % 100) dot_ver2(__LCC_MINOR__) compiler += ")";
#elif __GNUC__
    compiler += "g++ (GNUC) ";
    compiler += make_version_string(__GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
#else
    compiler += "Unknown compiler ";
    compiler += "(unknown version)";
#endif

#if defined(__APPLE__)
    compiler += " on Apple";
#elif defined(__CYGWIN__)
    compiler += " on Cygwin";
#elif defined(__MINGW64__)
    compiler += " on MinGW64";
#elif defined(__MINGW32__)
    compiler += " on MinGW32";
#elif defined(__ANDROID__)
    compiler += " on Android";
#elif defined(__linux__)
    compiler += " on Linux";
#elif defined(_WIN64)
    compiler += " on Microsoft Windows 64-bit";
#elif defined(_WIN32)
    compiler += " on Microsoft Windows 32-bit";
#else
    compiler += " on unknown system";
#endif

    compiler += "\nCompilation architecture   : ";
#if defined(ARCH)
    compiler += stringify(ARCH);
#else
    compiler += "(undefined architecture)";
#endif

    compiler += "\nCompilation settings       : ";
    compiler += (Is64Bit ? "64bit" : "32bit");
#if defined(USE_AVX512ICL)
    compiler += " AVX512ICL";
#endif
#if defined(USE_VNNI)
    compiler += " VNNI";
#endif
#if defined(USE_AVX512)
    compiler += " AVX512";
#endif
    compiler += (HasPext ? " BMI2" : "");
#if defined(USE_AVX2)
    compiler += " AVX2";
#endif
#if defined(USE_SSE41)
    compiler += " SSE41";
#endif
#if defined(USE_SSSE3)
    compiler += " SSSE3";
#endif
#if defined(USE_SSE2)
    compiler += " SSE2";
#endif
#if defined(USE_NEON_DOTPROD)
    compiler += " NEON_DOTPROD";
#elif defined(USE_NEON)
    compiler += " NEON";
#endif
    compiler += (HasPopCnt ? " POPCNT" : "");

#if !defined(NDEBUG)
    compiler += " DEBUG";
#endif

    compiler += "\nCompiler __VERSION__ macro : ";
#ifdef __VERSION__
    compiler += __VERSION__;
#else
    compiler += "(undefined macro)";
#endif

    compiler += "\n";

    return compiler;
}


// Debug functions used mainly to collect run-time statistics
constexpr int MaxDebugSlots = 32;

namespace {

template<size_t N>
struct DebugInfo {
    std::array<std::atomic<int64_t>, N> data = {0};

    [[nodiscard]] constexpr std::atomic<int64_t>& operator[](size_t index) {
        assert(index < N);
        return data[index];
    }

    constexpr DebugInfo& operator=(const DebugInfo& other) {
        for (size_t i = 0; i < N; i++)
            data[i].store(other.data[i].load());
        return *this;
    }
};

struct DebugExtremes: public DebugInfo<3> {
    DebugExtremes() {
        data[1] = std::numeric_limits<int64_t>::min();
        data[2] = std::numeric_limits<int64_t>::max();
    }
};

std::array<DebugInfo<2>, MaxDebugSlots>  hit;
std::array<DebugInfo<2>, MaxDebugSlots>  mean;
std::array<DebugInfo<3>, MaxDebugSlots>  stdev;
std::array<DebugInfo<6>, MaxDebugSlots>  correl;
std::array<DebugExtremes, MaxDebugSlots> extremes;

}  // namespace

void dbg_hit_on(bool cond, int slot) {

    ++hit.at(slot)[0];
    if (cond)
        ++hit.at(slot)[1];
}

void dbg_mean_of(int64_t value, int slot) {

    ++mean.at(slot)[0];
    mean.at(slot)[1] += value;
}

void dbg_stdev_of(int64_t value, int slot) {

    ++stdev.at(slot)[0];
    stdev.at(slot)[1] += value;
    stdev.at(slot)[2] += value * value;
}

void dbg_extremes_of(int64_t value, int slot) {
    ++extremes.at(slot)[0];

    int64_t current_max = extremes.at(slot)[1].load();
    while (current_max < value && !extremes.at(slot)[1].compare_exchange_weak(current_max, value))
    {}

    int64_t current_min = extremes.at(slot)[2].load();
    while (current_min > value && !extremes.at(slot)[2].compare_exchange_weak(current_min, value))
    {}
}

void dbg_correl_of(int64_t value1, int64_t value2, int slot) {

    ++correl.at(slot)[0];
    correl.at(slot)[1] += value1;
    correl.at(slot)[2] += value1 * value1;
    correl.at(slot)[3] += value2;
    correl.at(slot)[4] += value2 * value2;
    correl.at(slot)[5] += value1 * value2;
}

void dbg_print() {

    int64_t n;
    auto    E   = [&n](int64_t x) { return double(x) / n; };
    auto    sqr = [](double x) { return x * x; };

    for (int i = 0; i < MaxDebugSlots; ++i)
        if ((n = hit[i][0]))
            std::cerr << "Hit #" << i << ": Total " << n << " Hits " << hit[i][1]
                      << " Hit Rate (%) " << 100.0 * E(hit[i][1]) << std::endl;

    for (int i = 0; i < MaxDebugSlots; ++i)
        if ((n = mean[i][0]))
        {
            std::cerr << "Mean #" << i << ": Total " << n << " Mean " << E(mean[i][1]) << std::endl;
        }

    for (int i = 0; i < MaxDebugSlots; ++i)
        if ((n = stdev[i][0]))
        {
            double r = sqrt(E(stdev[i][2]) - sqr(E(stdev[i][1])));
            std::cerr << "Stdev #" << i << ": Total " << n << " Stdev " << r << std::endl;
        }

    for (int i = 0; i < MaxDebugSlots; ++i)
        if ((n = extremes[i][0]))
        {
            std::cerr << "Extremity #" << i << ": Total " << n << " Min " << extremes[i][2]
                      << " Max " << extremes[i][1] << std::endl;
        }

    for (int i = 0; i < MaxDebugSlots; ++i)
        if ((n = correl[i][0]))
        {
            double r = (E(correl[i][5]) - E(correl[i][1]) * E(correl[i][3]))
                     / (sqrt(E(correl[i][2]) - sqr(E(correl[i][1])))
                        * sqrt(E(correl[i][4]) - sqr(E(correl[i][3]))));
            std::cerr << "Correl. #" << i << ": Total " << n << " Coefficient " << r << std::endl;
        }
}

void dbg_clear() {
    hit.fill({});
    mean.fill({});
    stdev.fill({});
    correl.fill({});
    extremes.fill({});
}

// Used to serialize access to std::cout
// to avoid multiple threads writing at the same time.
std::ostream& operator<<(std::ostream& os, SyncCout sc) {

    static std::mutex m;

    if (sc == IO_LOCK)
        m.lock();

    if (sc == IO_UNLOCK)
        m.unlock();

    return os;
}

void sync_cout_start() { std::cout << IO_LOCK; }
void sync_cout_end() { std::cout << IO_UNLOCK; }

// Trampoline helper to avoid moving Logger to misc.h
void start_logger(const std::string& fname) { Logger::start(fname); }


#ifdef NO_PREFETCH

void prefetch(const void*) {}

#else

void prefetch(const void* addr) {

    #if defined(_MSC_VER)
    _mm_prefetch((char const*) addr, _MM_HINT_T0);
    #else
    __builtin_prefetch(addr);
    #endif
}

#endif

#ifdef _WIN32
    #include <direct.h>
    #define GETCWD _getcwd
#else
    #include <unistd.h>
    #define GETCWD getcwd
#endif

size_t str_to_size_t(const std::string& s) {
    unsigned long long value = std::stoull(s);
    if (value > std::numeric_limits<size_t>::max())
        std::exit(EXIT_FAILURE);
    return static_cast<size_t>(value);
}

std::optional<std::string> read_file_to_string(const std::string& path) {
    std::ifstream f(path, std::ios_base::binary);
    if (!f)
        return std::nullopt;
    return std::string(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>());
}

void remove_whitespace(std::string& s) {
    s.erase(std::remove_if(s.begin(), s.end(), [](char c) { return std::isspace(c); }), s.end());
}

bool is_whitespace(std::string_view s) {
    return std::all_of(s.begin(), s.end(), [](char c) { return std::isspace(c); });
}

//from learning begin
CommandLine::CommandLine(int _argc, char** _argv) :
    argc(_argc),
    argv(_argv) {
    workingDirectory = CommandLine::get_working_directory();
    binaryDirectory  = CommandLine::get_binary_directory(argv[0], workingDirectory);
    Util::init(this);
    //from learning end
}

std::string CommandLine::get_binary_directory(std::string argv0, std::string workingDirectory) {
    std::string pathSeparator;

#ifdef _WIN32
    pathSeparator = "\\";
    #ifdef _MSC_VER
    // Under windows argv[0] may not have the extension. Also _get_pgmptr() had
    // issues in some Windows 10 versions, so check returned values carefully.
    char* pgmptr = nullptr;
    if (!_get_pgmptr(&pgmptr) && pgmptr != nullptr && *pgmptr)
        argv0 = pgmptr;
    #endif
#else
    pathSeparator = "/";
#endif
    //from learning end
    // Extract the binary directory path from argv0
    auto   binaryDirectory = argv0;
    size_t pos             = binaryDirectory.find_last_of("\\/");
    if (pos == std::string::npos)
        binaryDirectory = "." + pathSeparator;
    else
        binaryDirectory.resize(pos + 1);

    // Pattern replacement: "./" at the start of path is replaced by the working directory
    //from learning begin
    if (binaryDirectory.find("." + pathSeparator) == 0)
    {
        binaryDirectory.replace(0, 1, workingDirectory);
    }
    binaryDirectory = Util::fix_path(binaryDirectory);
    //from learning end
    return binaryDirectory;
}

std::string CommandLine::get_working_directory() {
    std::string workingDirectory = "";
    char        buff[40000];
    char*       cwd = GETCWD(buff, 40000);
    //from learning begin
    if (cwd)
    {
        workingDirectory = cwd;
    }
    workingDirectory = Util::fix_path(workingDirectory);  //learning
    //from learning end
    return workingDirectory;
}

//Book management and learning begin
CommandLine* Util::cli = nullptr;
void         Util::init(CommandLine* _cli) { cli = _cli; }
std::string  Util::unquote(const std::string& s) {
    std::string s1 = s;

    if (s1.size() > 2)
    {
        if ((s1.front() == '\"' && s1.back() == '\"') || (s1.front() == '\'' && s1.back() == '\''))
        {
            s1 = s1.substr(1, s1.size() - 2);
        }
    }

    return s1;
}

bool Util::is_empty_filename(const std::string& fn) {
    if (fn.empty())
        return true;

    static std::string Empty = EMPTY;
    return std::equal(fn.begin(), fn.end(), Empty.begin(), Empty.end(),
                      [](char a, char b) { return tolower(a) == tolower(b); });
}

std::string Util::fix_path(const std::string& p) {
    if (is_empty_filename(p))
        return p;

    std::string p1 = unquote(p);
    std::replace(p1.begin(), p1.end(), ReverseDirectorySeparator, DirectorySeparator);

    return p1;
}

std::string Util::combine_path(const std::string& p1, const std::string& p2) {
    //We don't expect the first part of the path to be empty!
    assert(is_empty_filename(p1) == false);

    if (is_empty_filename(p2))
        return p2;

    std::string p;
    if (p1.back() == DirectorySeparator || p1.back() == ReverseDirectorySeparator)
        p = p1 + p2;
    else
        p = p1 + DirectorySeparator + p2;

    return fix_path(p);
}

std::string Util::map_path(const std::string& p) {
    if (is_empty_filename(p))
        return p;

    std::string p2 = fix_path(p);

    //Make sure we can map this path
    if (p2.find(DirectorySeparator) == std::string::npos)
        p2 = combine_path(cli->binaryDirectory, p);

    return p2;
}

size_t Util::get_file_size(const std::string& f) {
    if (is_empty_filename(f))
        return (size_t) -1;

    std::ifstream in(map_path(f), std::ifstream::ate | std::ifstream::binary);
    if (!in.is_open())
        return (size_t) -1;

    return (size_t) in.tellg();
}

bool Util::is_same_file(const std::string& f1, const std::string& f2) {
    return map_path(f1) == map_path(f2);
}

std::string Util::format_bytes(uint64_t bytes, int decimals) {
    static const uint64_t KB = 1024;
    static const uint64_t MB = KB * 1024;
    static const uint64_t GB = MB * 1024;
    static const uint64_t TB = GB * 1024;

    std::stringstream ss;

    if (bytes < KB)
        ss << bytes << " B";
    else if (bytes < MB)
        ss << std::fixed << std::setprecision(decimals) << (double(bytes) / KB) << "KB";
    else if (bytes < GB)
        ss << std::fixed << std::setprecision(decimals) << (double(bytes) / MB) << "MB";
    else if (bytes < TB)
        ss << std::fixed << std::setprecision(decimals) << (double(bytes) / GB) << "GB";
    else
        ss << std::fixed << std::setprecision(decimals) << (double(bytes) / TB) << "TB";

    return ss.str();
}

//Code is an `edited` version of: https://stackoverflow.com/a/49812018
std::string Util::format_string(const char* const fmt, ...) {
    //Initialize use of the variable arguments
    va_list vaArgs;
    va_start(vaArgs, fmt);

    //Acquire the required string size
    va_start(vaArgs, fmt);
    int len = vsnprintf(nullptr, 0, fmt, vaArgs);
    va_end(vaArgs);


    //Allocate enough buffer and format
    std::vector<char> v(len + 1);

    va_start(vaArgs, fmt);
    vsnprintf(v.data(), v.size(), fmt, vaArgs);
    va_end(vaArgs);

    return std::string(v.data(), len);
}
//Book management and learning end


}  // namespace Alexander


// ============================================================================
// FILE ORIGINALE: misc.h
// ============================================================================

/*
  Alexander, a UCI chess playing engine derived from Stockfish
  Copyright (C) 2004-2026 Andrea Manzo, F. Ferraguti, K.Kiniama and Alexander developers (see AUTHORS file)

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

#ifndef MISC_H_INCLUDED
#define MISC_H_INCLUDED

#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <exception>  // IWYU pragma: keep
// IWYU pragma: no_include <__exception/terminate.h>
#include <functional>
#include <iosfwd>
#include <optional>
#include <cstring>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>
//Alexander specific begin
#include "types.h"
#include <iostream>
#ifndef _WIN32
    #include <fcntl.h>
    #include <unistd.h>
    #include <sys/mman.h>
    #include <sys/stat.h>
#else
    #define WIN32_LEAN_AND_MEAN
    #ifndef NOMINMAX
        #define NOMINMAX  // Disable macros min() and max()
    #endif
    #include <windows.h>
#endif
//Alexander specific end

#define stringify2(x) #x
#define stringify(x) stringify2(x)

namespace Alexander {

std::string engine_version_info();
std::string engine_info(bool to_uci = false);
std::string compiler_info();

// Preloads the given address in L1/L2 cache. This is a non-blocking
// function that doesn't stall the CPU waiting for data to be loaded from memory,
// which can be quite slow.
void prefetch(const void* addr);

void start_logger(const std::string& fname);

size_t str_to_size_t(const std::string& s);

#if defined(__linux__)

struct PipeDeleter {
    void operator()(FILE* file) const {
        if (file != nullptr)
        {
            pclose(file);
        }
    }
};

#endif

// Reads the file as bytes.
// Returns std::nullopt if the file does not exist.
std::optional<std::string> read_file_to_string(const std::string& path);

void dbg_hit_on(bool cond, int slot = 0);
void dbg_mean_of(int64_t value, int slot = 0);
void dbg_stdev_of(int64_t value, int slot = 0);
void dbg_extremes_of(int64_t value, int slot = 0);
void dbg_correl_of(int64_t value1, int64_t value2, int slot = 0);
void dbg_print();
void dbg_clear();

using TimePoint = std::chrono::milliseconds::rep;  // A value in milliseconds
static_assert(sizeof(TimePoint) == sizeof(int64_t), "TimePoint should be 64 bits");
inline TimePoint now() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
             std::chrono::steady_clock::now().time_since_epoch())
      .count();
}
//for classical begin
template<class Entry, int Size>
struct HashTable {
    Entry* operator[](Key key) { return &table[(uint32_t) key & (Size - 1)]; }

   private:
    std::vector<Entry> table = std::vector<Entry>(Size);  // Allocate on the heap
};
//for classical end
inline std::vector<std::string_view> split(std::string_view s, std::string_view delimiter) {
    std::vector<std::string_view> res;

    if (s.empty())
        return res;

    size_t begin = 0;
    for (;;)
    {
        const size_t end = s.find(delimiter, begin);
        if (end == std::string::npos)
            break;

        res.emplace_back(s.substr(begin, end - begin));
        begin = end + delimiter.size();
    }

    res.emplace_back(s.substr(begin));

    return res;
}

void remove_whitespace(std::string& s);
bool is_whitespace(std::string_view s);

enum SyncCout {
    IO_LOCK,
    IO_UNLOCK
};
std::ostream& operator<<(std::ostream&, SyncCout);

#define sync_cout std::cout << IO_LOCK
#define sync_endl std::endl << IO_UNLOCK

void sync_cout_start();
void sync_cout_end();

// True if and only if the binary is compiled on a little-endian machine
static inline const std::uint16_t Le             = 1;
static inline const bool          IsLittleEndian = *reinterpret_cast<const char*>(&Le) == 1;


//omissis Alexander: in types.h


template<typename T, std::size_t Size, std::size_t... Sizes>
class MultiArray;

namespace Detail {

template<typename T, std::size_t Size, std::size_t... Sizes>
struct MultiArrayHelper {
    using ChildType = MultiArray<T, Sizes...>;
};

template<typename T, std::size_t Size>
struct MultiArrayHelper<T, Size> {
    using ChildType = T;
};

template<typename To, typename From>
constexpr bool is_strictly_assignable_v =
  std::is_assignable_v<To&, From> && (std::is_same_v<To, From> || !std::is_convertible_v<From, To>);

}

// MultiArray is a generic N-dimensional array.
// The template parameters (Size and Sizes) encode the dimensions of the array.
template<typename T, std::size_t Size, std::size_t... Sizes>
class MultiArray {
    using ChildType = typename Detail::MultiArrayHelper<T, Size, Sizes...>::ChildType;
    using ArrayType = std::array<ChildType, Size>;
    ArrayType data_;

   public:
    using value_type             = typename ArrayType::value_type;
    using size_type              = typename ArrayType::size_type;
    using difference_type        = typename ArrayType::difference_type;
    using reference              = typename ArrayType::reference;
    using const_reference        = typename ArrayType::const_reference;
    using pointer                = typename ArrayType::pointer;
    using const_pointer          = typename ArrayType::const_pointer;
    using iterator               = typename ArrayType::iterator;
    using const_iterator         = typename ArrayType::const_iterator;
    using reverse_iterator       = typename ArrayType::reverse_iterator;
    using const_reverse_iterator = typename ArrayType::const_reverse_iterator;

    constexpr auto&       at(size_type index) noexcept { return data_.at(index); }
    constexpr const auto& at(size_type index) const noexcept { return data_.at(index); }

    constexpr auto&       operator[](size_type index) noexcept { return data_[index]; }
    constexpr const auto& operator[](size_type index) const noexcept { return data_[index]; }

    constexpr auto&       front() noexcept { return data_.front(); }
    constexpr const auto& front() const noexcept { return data_.front(); }
    constexpr auto&       back() noexcept { return data_.back(); }
    constexpr const auto& back() const noexcept { return data_.back(); }

    auto*       data() { return data_.data(); }
    const auto* data() const { return data_.data(); }

    constexpr auto begin() noexcept { return data_.begin(); }
    constexpr auto end() noexcept { return data_.end(); }
    constexpr auto begin() const noexcept { return data_.begin(); }
    constexpr auto end() const noexcept { return data_.end(); }
    constexpr auto cbegin() const noexcept { return data_.cbegin(); }
    constexpr auto cend() const noexcept { return data_.cend(); }

    constexpr auto rbegin() noexcept { return data_.rbegin(); }
    constexpr auto rend() noexcept { return data_.rend(); }
    constexpr auto rbegin() const noexcept { return data_.rbegin(); }
    constexpr auto rend() const noexcept { return data_.rend(); }
    constexpr auto crbegin() const noexcept { return data_.crbegin(); }
    constexpr auto crend() const noexcept { return data_.crend(); }

    constexpr bool      empty() const noexcept { return data_.empty(); }
    constexpr size_type size() const noexcept { return data_.size(); }
    constexpr size_type max_size() const noexcept { return data_.max_size(); }

    template<typename U>
    void fill(const U& v) {
        static_assert(Detail::is_strictly_assignable_v<T, U>,
                      "Cannot assign fill value to entry type");
        for (auto& ele : data_)
        {
            if constexpr (sizeof...(Sizes) == 0)
                ele = v;
            else
                ele.fill(v);
        }
    }

    constexpr void swap(MultiArray<T, Size, Sizes...>& other) noexcept { data_.swap(other.data_); }
};


// xorshift64star Pseudo-Random Number Generator
// This class is based on original code written and dedicated
// to the public domain by Sebastiano Vigna (2014).
// It has the following characteristics:
//
//  -  Outputs 64-bit numbers
//  -  Passes Dieharder and SmallCrush test batteries
//  -  Does not require warm-up, no zeroland to escape
//  -  Internal state is a single 64-bit integer
//  -  Period is 2^64 - 1
//  -  Speed: 1.60 ns/call (Core i7 @3.40GHz)
//
// For further analysis see
//   <http://vigna.di.unimi.it/ftp/papers/xorshift.pdf>

class PRNG {

    uint64_t s;

    uint64_t rand64() {

        s ^= s >> 12, s ^= s << 25, s ^= s >> 27;
        return s * 2685821657736338717LL;
    }

   public:
    PRNG(uint64_t seed) :
        s(seed) {
        assert(seed);
    }

    template<typename T>
    T rand() {
        return T(rand64());
    }

    // Special generator used to fast init magic numbers.
    // Output values only have 1/8th of their bits set on average.
    template<typename T>
    T sparse_rand() {
        return T(rand64() & rand64() & rand64());
    }
};

inline uint64_t mul_hi64(uint64_t a, uint64_t b) {
#if defined(__GNUC__) && defined(IS_64BIT)
    __extension__ using uint128 = unsigned __int128;
    return (uint128(a) * uint128(b)) >> 64;
#else
    uint64_t aL = uint32_t(a), aH = a >> 32;
    uint64_t bL = uint32_t(b), bH = b >> 32;
    uint64_t c1 = (aL * bL) >> 32;
    uint64_t c2 = aH * bL + c1;
    uint64_t c3 = aL * bH + uint32_t(c2);
    return aH * bH + (c2 >> 32) + (c3 >> 32);
#endif
}

inline std::uint64_t hash_bytes(const char* data, std::size_t size) {
    // FNV-1a 64-bit
    const char*   p = data;
    std::uint64_t h = 14695981039346656037ull;
    for (std::size_t i = 0; i < size; ++i)
        h = (h ^ p[i]) * 1099511628211ull;
    return h;
}

template<typename T>
inline std::size_t get_raw_data_hash(const T& value) {
    // We must have no padding bytes because we're reinterpreting as char
    static_assert(std::has_unique_object_representations<T>());

    return static_cast<std::size_t>(
      hash_bytes(reinterpret_cast<const char*>(&value), sizeof(value)));
}

template<typename T>
inline void hash_combine(std::size_t& seed, const T& v) {
    std::size_t x;
    // For primitive types we avoid using the default hasher, which may be
    // nondeterministic across program invocations
    if constexpr (std::is_integral<T>())
        x = v;
    else
        x = std::hash<T>{}(v);
    seed ^= x + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

inline std::uint64_t hash_string(const std::string& sv) { return hash_bytes(sv.data(), sv.size()); }

template<std::size_t Capacity>
class FixedString {
   public:
    FixedString() :
        length_(0) {
        data_[0] = '\0';
    }

    FixedString(const char* str) {
        size_t len = std::strlen(str);
        if (len > Capacity)
            std::terminate();
        std::memcpy(data_, str, len);
        length_        = len;
        data_[length_] = '\0';
    }

    FixedString(const std::string& str) {
        if (str.size() > Capacity)
            std::terminate();
        std::memcpy(data_, str.data(), str.size());
        length_        = str.size();
        data_[length_] = '\0';
    }

    std::size_t size() const { return length_; }
    std::size_t capacity() const { return Capacity; }

    const char* c_str() const { return data_; }
    const char* data() const { return data_; }

    char& operator[](std::size_t i) { return data_[i]; }

    const char& operator[](std::size_t i) const { return data_[i]; }

    FixedString& operator+=(const char* str) {
        size_t len = std::strlen(str);
        if (length_ + len > Capacity)
            std::terminate();
        std::memcpy(data_ + length_, str, len);
        length_ += len;
        data_[length_] = '\0';
        return *this;
    }

    FixedString& operator+=(const FixedString& other) { return (*this += other.c_str()); }

    operator std::string() const { return std::string(data_, length_); }

    operator std::string_view() const { return std::string_view(data_, length_); }

    template<typename T>
    bool operator==(const T& other) const noexcept {
        return (std::string_view) (*this) == other;
    }

    template<typename T>
    bool operator!=(const T& other) const noexcept {
        return (std::string_view) (*this) != other;
    }

    void clear() {
        length_  = 0;
        data_[0] = '\0';
    }

   private:
    char        data_[Capacity + 1];  // +1 for null terminator
    std::size_t length_;
};

struct CommandLine {
   public:
    CommandLine(int, char**);  //from learning

    int    argc;
    char** argv;
    //from learning begin
    std::string        binaryDirectory;   // path of the executable directory
    std::string        workingDirectory;  // path of the working directory
    static std::string get_working_directory();
    static std::string get_binary_directory(std::string argv0, std::string workingDirectory);
    //from learning end
};

namespace Utility {

template<typename T, typename Predicate>
void move_to_front(std::vector<T>& vec, Predicate pred) {
    auto it = std::find_if(vec.begin(), vec.end(), pred);

    if (it != vec.end())
    {
        std::rotate(vec.begin(), it, it + 1);
    }
}
}


#if defined(__GNUC__)
    #define sf_always_inline __attribute__((always_inline))
#elif defined(_MSC_VER)
    #define sf_always_inline __forceinline
#else
    // do nothing for other compilers
    #define sf_always_inline
#endif

//Alexander omissis: in types.h
//begin from learning
#define EMPTY "<empty>"

class Util {
   private:
#if defined(_WIN32) || defined(_WIN64)
    static constexpr char DirectorySeparator        = '\\';
    static constexpr char ReverseDirectorySeparator = '/';
#else
    static constexpr char DirectorySeparator        = '/';
    static constexpr char ReverseDirectorySeparator = '\\';
#endif
    static CommandLine* cli;

   public:
    static void        init(CommandLine* _cli);
    static std::string unquote(const std::string& s);
    static bool        is_empty_filename(const std::string& f);
    static std::string fix_path(const std::string& p);
    static std::string combine_path(const std::string& p1, const std::string& p2);
    static std::string map_path(const std::string& p);

    static size_t get_file_size(const std::string& f);
    static bool   is_same_file(const std::string& f1, const std::string& f2);

    static std::string format_bytes(uint64_t bytes, int decimals);

    static std::string format_string(const char* const fmt, ...);
};
//end from learning

}  // namespace Alexander
template<std::size_t N>
struct std::hash<Alexander::FixedString<N>> {
    std::size_t operator()(const Alexander::FixedString<N>& fstr) const noexcept {
        return Alexander::hash_bytes(fstr.data(), fstr.size());
    }
};
#endif  // #ifndef MISC_H_INCLUDED


// ============================================================================
// FILE ORIGINALE: movegen.cpp
// ============================================================================

/*
  Alexander, a UCI chess playing engine derived from Stockfish
 Copyright (C) 2004-2026 Andrea Manzo, F. Ferraguti, K.Kiniama and Alexander developers (see AUTHORS file)

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

#include "movegen.h"

#include <cassert>
#include <initializer_list>

#include "bitboard.h"
#include "position.h"

#if defined(USE_AVX512ICL)
    #include <array>
    #include <algorithm>
    #include <immintrin.h>
#endif

namespace Alexander {
namespace {

#if defined(USE_AVX512ICL)

inline Move* write_moves(Move* moveList, uint32_t mask, __m512i vector) {
    // Avoid _mm512_mask_compressstoreu_epi16() as it's 256 uOps on Zen4
    _mm512_storeu_si512(reinterpret_cast<__m512i*>(moveList),
                        _mm512_maskz_compress_epi16(mask, vector));
    return moveList + popcount(mask);
}

template<Direction offset>
inline Move* splat_pawn_moves(Move* moveList, Bitboard to_bb) {
    alignas(64) static constexpr auto SPLAT_TABLE = [] {
        std::array<Move, 64> table{};
        for (int i = 0; i < 64; i++)
        {
            Square from{uint8_t(std::clamp(i - offset, 0, 63))};
            table[i] = {Move(from, Square{uint8_t(i)})};
        }
        return table;
    }();

    auto table = reinterpret_cast<const __m512i*>(SPLAT_TABLE.data());

    moveList =
      write_moves(moveList, static_cast<uint32_t>(to_bb >> 0), _mm512_load_si512(table + 0));
    moveList =
      write_moves(moveList, static_cast<uint32_t>(to_bb >> 32), _mm512_load_si512(table + 1));

    return moveList;
}

inline Move* splat_moves(Move* moveList, Square from, Bitboard to_bb) {
    alignas(64) static constexpr auto SPLAT_TABLE = [] {
        std::array<Move, 64> table{};
        for (uint8_t i = 0; i < 64; i++)
            table[i] = {Move(SQUARE_ZERO, Square{i})};
        return table;
    }();

    __m512i fromVec = _mm512_set1_epi16(Move(from, SQUARE_ZERO).raw());

    auto table = reinterpret_cast<const __m512i*>(SPLAT_TABLE.data());

    moveList = write_moves(moveList, static_cast<uint32_t>(to_bb >> 0),
                           _mm512_or_si512(_mm512_load_si512(table + 0), fromVec));
    moveList = write_moves(moveList, static_cast<uint32_t>(to_bb >> 32),
                           _mm512_or_si512(_mm512_load_si512(table + 1), fromVec));

    return moveList;
}

#else

template<Direction offset>
inline Move* splat_pawn_moves(Move* moveList, Bitboard to_bb) {
    while (to_bb)
    {
        Square to   = pop_lsb(to_bb);
        *moveList++ = Move(to - offset, to);
    }
    return moveList;
}

inline Move* splat_moves(Move* moveList, Square from, Bitboard to_bb) {
    while (to_bb)
        *moveList++ = Move(from, pop_lsb(to_bb));
    return moveList;
}

#endif

template<GenType Type, Direction D, bool Enemy>
Move* make_promotions(Move* moveList, [[maybe_unused]] Square to) {

    constexpr bool all = Type == EVASIONS || Type == NON_EVASIONS;

    if constexpr (Type == CAPTURES || all)
        *moveList++ = Move::make<PROMOTION>(to - D, to, QUEEN);

    if constexpr ((Type == CAPTURES && Enemy) || (Type == QUIETS && !Enemy) || all)
    {
        *moveList++ = Move::make<PROMOTION>(to - D, to, ROOK);
        *moveList++ = Move::make<PROMOTION>(to - D, to, BISHOP);
        *moveList++ = Move::make<PROMOTION>(to - D, to, KNIGHT);
    }

    return moveList;
}


template<Color Us, GenType Type>
Move* generate_pawn_moves(const Position& pos, Move* moveList, Bitboard target) {

    constexpr Color     Them     = ~Us;
    constexpr Bitboard  TRank7BB = (Us == WHITE ? Rank7BB : Rank2BB);
    constexpr Bitboard  TRank3BB = (Us == WHITE ? Rank3BB : Rank6BB);
    constexpr Direction Up       = pawn_push(Us);
    constexpr Direction UpRight  = (Us == WHITE ? NORTH_EAST : SOUTH_WEST);
    constexpr Direction UpLeft   = (Us == WHITE ? NORTH_WEST : SOUTH_EAST);

    const Bitboard emptySquares = ~pos.pieces();
    const Bitboard enemies      = Type == EVASIONS ? pos.checkers() : pos.pieces(Them);

    Bitboard pawnsOn7    = pos.pieces(Us, PAWN) & TRank7BB;
    Bitboard pawnsNotOn7 = pos.pieces(Us, PAWN) & ~TRank7BB;

    // Single and double pawn pushes, no promotions
    if constexpr (Type != CAPTURES)
    {
        Bitboard b1 = shift<Up>(pawnsNotOn7) & emptySquares;
        Bitboard b2 = shift<Up>(b1 & TRank3BB) & emptySquares;

        if constexpr (Type == EVASIONS)  // Consider only blocking squares
        {
            b1 &= target;
            b2 &= target;
        }

        moveList = splat_pawn_moves<Up>(moveList, b1);
        moveList = splat_pawn_moves<Up + Up>(moveList, b2);
    }

    // Promotions and underpromotions
    if (pawnsOn7)
    {
        Bitboard b1 = shift<UpRight>(pawnsOn7) & enemies;
        Bitboard b2 = shift<UpLeft>(pawnsOn7) & enemies;
        Bitboard b3 = shift<Up>(pawnsOn7) & emptySquares;

        if constexpr (Type == EVASIONS)
            b3 &= target;

        while (b1)
            moveList = make_promotions<Type, UpRight, true>(moveList, pop_lsb(b1));

        while (b2)
            moveList = make_promotions<Type, UpLeft, true>(moveList, pop_lsb(b2));

        while (b3)
            moveList = make_promotions<Type, Up, false>(moveList, pop_lsb(b3));
    }

    // Standard and en passant captures
    if constexpr (Type == CAPTURES || Type == EVASIONS || Type == NON_EVASIONS)
    {
        Bitboard b1 = shift<UpRight>(pawnsNotOn7) & enemies;
        Bitboard b2 = shift<UpLeft>(pawnsNotOn7) & enemies;

        moveList = splat_pawn_moves<UpRight>(moveList, b1);
        moveList = splat_pawn_moves<UpLeft>(moveList, b2);

        if (pos.ep_square() != SQ_NONE)
        {
            assert(rank_of(pos.ep_square()) == relative_rank(Us, RANK_6));

            // An en passant capture cannot resolve a discovered check
            if (Type == EVASIONS && (target & (pos.ep_square() + Up)))
                return moveList;

            b1 = pawnsNotOn7 & attacks_bb<PAWN>(pos.ep_square(), Them);

            assert(b1);

            while (b1)
                *moveList++ = Move::make<EN_PASSANT>(pop_lsb(b1), pos.ep_square());
        }
    }

    return moveList;
}


template<Color Us, PieceType Pt>
Move* generate_moves(const Position& pos, Move* moveList, Bitboard target) {

    static_assert(Pt != KING && Pt != PAWN, "Unsupported piece type in generate_moves()");

    Bitboard bb = pos.pieces(Us, Pt);

    while (bb)
    {
        Square   from = pop_lsb(bb);
        Bitboard b    = attacks_bb<Pt>(from, pos.pieces()) & target;

        moveList = splat_moves(moveList, from, b);
    }

    return moveList;
}


template<Color Us, GenType Type>
Move* generate_all(const Position& pos, Move* moveList) {

    static_assert(Type != LEGAL, "Unsupported type in generate_all()");

    const Square ksq = pos.square<KING>(Us);
    Bitboard     target;

    // Skip generating non-king moves when in double check
    if (Type != EVASIONS || !more_than_one(pos.checkers()))
    {
        target = Type == EVASIONS     ? between_bb(ksq, lsb(pos.checkers()))
               : Type == NON_EVASIONS ? ~pos.pieces(Us)
               : Type == CAPTURES     ? pos.pieces(~Us)
                                      : ~pos.pieces();  // QUIETS

        moveList = generate_pawn_moves<Us, Type>(pos, moveList, target);
        moveList = generate_moves<Us, KNIGHT>(pos, moveList, target);
        moveList = generate_moves<Us, BISHOP>(pos, moveList, target);
        moveList = generate_moves<Us, ROOK>(pos, moveList, target);
        moveList = generate_moves<Us, QUEEN>(pos, moveList, target);
    }

    Bitboard b = attacks_bb<KING>(ksq) & (Type == EVASIONS ? ~pos.pieces(Us) : target);


    moveList = splat_moves(moveList, ksq, b);

    if ((Type == QUIETS || Type == NON_EVASIONS) && pos.can_castle(Us & ANY_CASTLING))
        for (CastlingRights cr : {Us & KING_SIDE, Us & QUEEN_SIDE})
            if (!pos.castling_impeded(cr) && pos.can_castle(cr))
                *moveList++ = Move::make<CASTLING>(ksq, pos.castling_rook_square(cr));

    return moveList;
}

}  // namespace


// <CAPTURES>     Generates all pseudo-legal captures plus queen promotions
// <QUIETS>       Generates all pseudo-legal non-captures and underpromotions
// <EVASIONS>     Generates all pseudo-legal check evasions
// <NON_EVASIONS> Generates all pseudo-legal captures and non-captures
//
// Returns a pointer to the end of the move list.
template<GenType Type>
Move* generate(const Position& pos, Move* moveList) {

    static_assert(Type != LEGAL, "Unsupported type in generate()");
    assert((Type == EVASIONS) == bool(pos.checkers()));

    Color us = pos.side_to_move();

    return us == WHITE ? generate_all<WHITE, Type>(pos, moveList)
                       : generate_all<BLACK, Type>(pos, moveList);
}

// Explicit template instantiations
template Move* generate<CAPTURES>(const Position&, Move*);
template Move* generate<QUIETS>(const Position&, Move*);
template Move* generate<EVASIONS>(const Position&, Move*);
template Move* generate<NON_EVASIONS>(const Position&, Move*);

// generate<LEGAL> generates all the legal moves in the given position

template<>
Move* generate<LEGAL>(const Position& pos, Move* moveList) {

    Color    us     = pos.side_to_move();
    Bitboard pinned = pos.blockers_for_king(us) & pos.pieces(us);
    Square   ksq    = pos.square<KING>(us);
    Move*    cur    = moveList;

    moveList =
      pos.checkers() ? generate<EVASIONS>(pos, moveList) : generate<NON_EVASIONS>(pos, moveList);
    while (cur != moveList)
        if (((pinned & cur->from_sq()) || cur->from_sq() == ksq || cur->type_of() == EN_PASSANT)
            && !pos.legal(*cur))
            *cur = *(--moveList);
        else
            ++cur;

    return moveList;
}

}  // namespace Alexander


// ============================================================================
// FILE ORIGINALE: movegen.h
// ============================================================================

/*
  Alexander, a UCI chess playing engine derived from Stockfish
  Copyright (C) 2004-2026 Andrea Manzo, F. Ferraguti, K.Kiniama and Alexander developers (see AUTHORS file)

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

#ifndef MOVEGEN_H_INCLUDED
#define MOVEGEN_H_INCLUDED

#include <algorithm>  // IWYU pragma: keep
#include <cstddef>

#include "types.h"
#include "./shashin/moveconfig.h"  //from shashin
namespace Alexander {
class Position;

enum GenType {
    CAPTURES,
    QUIETS,
    EVASIONS,
    NON_EVASIONS,
    LEGAL
};

struct ExtMove: public Move {
    int value;

    void operator=(Move m) { data = m.raw(); }

    // Inhibit unwanted implicit conversions to Move
    // with an ambiguity that yields to a compile error.
    operator float() const = delete;
};

inline bool operator<(const ExtMove& f, const ExtMove& s) { return f.value < s.value; }

template<GenType>
Move* generate(const Position& pos, Move* moveList);

// The MoveList struct wraps the generate() function and returns a convenient
// list of moves. Using MoveList is sometimes preferable to directly calling
// the lower level generate() function.
template<GenType T>
struct MoveList {

    explicit MoveList(const Position& pos) :
        last(generate<T>(pos, moveList)) {}
    const Move* begin() const { return moveList; }
    const Move* end() const { return last; }
    size_t      size() const { return last - moveList; }
    bool        contains(Move move) const { return std::find(begin(), end(), move) != end(); }

   private:
    Move moveList[MAX_MOVES], *last;
};

}  // namespace Alexander

#endif  // #ifndef MOVEGEN_H_INCLUDED


// ============================================================================
// FILE ORIGINALE: movepick.cpp
// ============================================================================

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

#include "movepick.h"

#include <cassert>
#include <limits>
#include <utility>
#include <cmath>  //for Alexander
#include "misc.h"
//for Alexander

namespace Alexander {

using namespace Alexander::Shashin;  //shashin
namespace {

enum Stages {
    // generate main search moves
    MAIN_TT,
    CAPTURE_INIT,
    GOOD_CAPTURE,
    QUIET_INIT,
    GOOD_QUIET,
    BAD_CAPTURE,
    BAD_QUIET,

    // generate evasion moves
    EVASION_TT,
    EVASION_INIT,
    EVASION,

    // generate probcut moves
    PROBCUT_TT,
    PROBCUT_INIT,
    PROBCUT,

    // generate qsearch moves
    QSEARCH_TT,
    QCAPTURE_INIT,
    QCAPTURE
};


// Sort moves in descending order up to and including a given limit.
// The order of moves smaller than the limit is left unspecified.
void partial_insertion_sort(ExtMove* begin, ExtMove* end, int limit) {

    for (ExtMove *sortedEnd = begin, *p = begin + 1; p < end; ++p)
        if (p->value >= limit)
        {
            ExtMove tmp = *p, *q;
            *p          = *++sortedEnd;
            for (q = sortedEnd; q != begin && *(q - 1) < tmp; --q)
                *q = *(q - 1);
            *q = tmp;
        }
}

}  // namespace


// Constructors of the MovePicker class. As arguments, we pass information
// to decide which class of moves to emit, to help sorting the (presumably)
// good moves first, and how important move ordering is at the current node.

// MovePicker constructor for the main search and for the quiescence search
MovePicker::MovePicker(const Position&              p,
                       Move                         ttm,
                       Depth                        d,
                       const ButterflyHistory*      mh,
                       const LowPlyHistory*         lph,
                       const CapturePieceToHistory* cph,
                       const PieceToHistory**       ch,
                       const SharedHistories*       sh,
                       int                          pl) :
    pos(p),
    mainHistory(mh),
    lowPlyHistory(lph),
    captureHistory(cph),
    continuationHistory(ch),
    sharedHistory(sh),
    ttMove(ttm),
    depth(d),
    ply(pl) {

    if (pos.checkers())
        stage = EVASION_TT + !(ttm && pos.pseudo_legal(ttm));

    else
        stage = (depth > 0 ? MAIN_TT : QSEARCH_TT) + !(ttm && pos.pseudo_legal(ttm));
}

// MovePicker constructor for ProbCut: we generate captures with Static Exchange
// Evaluation (SEE) greater than or equal to the given threshold.
MovePicker::MovePicker(const Position& p, Move ttm, int th, const CapturePieceToHistory* cph) :
    pos(p),
    captureHistory(cph),
    ttMove(ttm),
    threshold(th) {
    assert(!pos.checkers());

    stage = PROBCUT_TT + !(ttm && pos.capture_stage(ttm) && pos.pseudo_legal(ttm));
}

// Assigns a numerical value to each move in a list, used for sorting.
// Captures are ordered by Most Valuable Victim (MVV), preferring captures
// with a good history. Quiets moves are ordered using the history tables.
template<GenType Type>
ExtMove* MovePicker::score(MoveList<Type>& ml) {

    static_assert(Type == CAPTURES || Type == QUIETS || Type == EVASIONS, "Wrong type");

    Color us = pos.side_to_move();

    [[maybe_unused]] Bitboard threatByLesser[KING + 1];
    if constexpr (Type == QUIETS)
    {
        threatByLesser[PAWN]   = 0;
        threatByLesser[KNIGHT] = threatByLesser[BISHOP] = pos.attacks_by<PAWN>(~us);
        threatByLesser[ROOK] =
          pos.attacks_by<KNIGHT>(~us) | pos.attacks_by<BISHOP>(~us) | threatByLesser[KNIGHT];
        threatByLesser[QUEEN] = pos.attacks_by<ROOK>(~us) | threatByLesser[ROOK];
        threatByLesser[KING]  = pos.attacks_by<QUEEN>(~us) | threatByLesser[QUEEN];
    }

    ExtMove* it = cur;
    for (auto move : ml)
    {
        ExtMove& m = *it++;
        m          = move;

        const Square    from          = m.from_sq();
        const Square    to            = m.to_sq();
        const Piece     pc            = pos.moved_piece(m);
        const PieceType pt            = type_of(pc);
        const Piece     capturedPiece = pos.piece_on(to);

        if constexpr (Type == CAPTURES)
        {
            m.value = (*captureHistory)[pc][to][type_of(capturedPiece)]
                    + 7 * int(PieceValue[capturedPiece]);

            // *** RIMOSSO BONUS AGGRESSIVO ***
            // Rimosso bonus scacco/SEE. Lasciamo che MVV e CaptureHistory facciano il loro lavoro.
        }
        else if constexpr (Type == QUIETS)
        {
            // histories
            m.value = 2 * (*mainHistory)[us][m.raw()];
            m.value += 2 * sharedHistory->pawn_entry(pos)[pc][to];
            m.value += (*continuationHistory[0])[pc][to];
            m.value += (*continuationHistory[1])[pc][to];
            m.value += (*continuationHistory[2])[pc][to];
            m.value += (*continuationHistory[3])[pc][to];
            m.value += (*continuationHistory[5])[pc][to];

            // bonus for checks
            m.value += (bool(pos.check_squares(pt) & to) && pos.see_ge(m, -75)) * 16384;

            // penalty for moving to a square threatened by a lesser piece
            // or bonus for escaping an attack by a lesser piece.
            int v = threatByLesser[pt] & to ? -19 : 20 * bool(threatByLesser[pt] & from);
            m.value += PieceValue[pt] * v;

            if (ply < LOW_PLY_HISTORY_SIZE)
                m.value += 8 * (*lowPlyHistory)[ply][m.raw()] / (1 + ply);

            // *** SHASHIN MOVEPICK TUNING ***
            if (depth > 6)
            {
                // 1. CAPABLANCA: Favorisce l'occupazione di avamposti sicuri.
                if (MoveConfig::isStrategical && type_of(pc) != PAWN && !pos.attackers_to(to, ~us))
                {
                    m.value += 30;
                }

                // 2. FASI TATTICHE (Tal, Petrosian, Caos): Bonus Sopravvivenza/Dinamismo.
                // Se NON siamo in fase puramente strategica (c'è tensione)
                // e stiamo spostando un pezzo importante VIA da una minaccia verso la salvezza.
                else if (!MoveConfig::isStrategical && type_of(pc) != PAWN)
                {
                    // Se la casa di partenza ERA minacciata, e la casa di arrivo è SICURA
                    if (pos.attackers_to(from, ~us) && !pos.attackers_to(to, ~us))
                    {
                        m.value += 25;
                    }
                }
            }
        }
        else  // Type == EVASIONS
        {
            if (pos.capture_stage(m))
                m.value = PieceValue[capturedPiece] + (1 << 28);
            else
                m.value = (*mainHistory)[us][m.raw()] + (*continuationHistory[0])[pc][to];
        }
    }
    return it;
}

// Returns the next move satisfying a predicate function.
// This never returns the TT move, as it was emitted before.
template<typename Pred>
Move MovePicker::select(Pred filter) {

    for (; cur < endCur; ++cur)
        if (*cur != ttMove && filter())
            return *cur++;

    return Move::none();
}

// This is the most important method of the MovePicker class. We emit one
// new pseudo-legal move on every call until there are no more moves left,
// picking the move with the highest score from a list of generated moves.
Move MovePicker::next_move() {

    constexpr int goodQuietThreshold = -14000;
top:
    switch (stage)
    {

    case MAIN_TT :
    case EVASION_TT :
    case QSEARCH_TT :
    case PROBCUT_TT :
        ++stage;
        return ttMove;

    case CAPTURE_INIT :
    case PROBCUT_INIT :
    case QCAPTURE_INIT : {
        MoveList<CAPTURES> ml(pos);

        cur = endBadCaptures = moves;
        endCur = endCaptures = score<CAPTURES>(ml);

        partial_insertion_sort(cur, endCur, std::numeric_limits<int>::min());
        ++stage;
        goto top;
    }

    case GOOD_CAPTURE :
        if (select([&]() {
                if (pos.see_ge(*cur, -cur->value / 18))
                    return true;
                std::swap(*endBadCaptures++, *cur);
                return false;
            }))
            return *(cur - 1);

        ++stage;
        [[fallthrough]];

    case QUIET_INIT :
        if (!skipQuiets)
        {
            MoveList<QUIETS> ml(pos);

            endCur = endGenerated = score<QUIETS>(ml);

            partial_insertion_sort(cur, endCur, -3560 * depth);
        }

        ++stage;
        [[fallthrough]];

    case GOOD_QUIET :
        if (!skipQuiets && select([&]() { return cur->value > goodQuietThreshold; }))
            return *(cur - 1);

        // Prepare the pointers to loop over the bad captures
        cur    = moves;
        endCur = endBadCaptures;

        ++stage;
        [[fallthrough]];

    case BAD_CAPTURE :
        if (select([]() { return true; }))
            return *(cur - 1);

        // Prepare the pointers to loop over quiets again
        cur    = endCaptures;
        endCur = endGenerated;

        ++stage;
        [[fallthrough]];

    case BAD_QUIET :
        if (!skipQuiets)
            return select([&]() { return cur->value <= goodQuietThreshold; });

        return Move::none();

    case EVASION_INIT : {
        MoveList<EVASIONS> ml(pos);

        cur    = moves;
        endCur = endGenerated = score<EVASIONS>(ml);

        partial_insertion_sort(cur, endCur, std::numeric_limits<int>::min());
        ++stage;
        [[fallthrough]];
    }

    case EVASION :
    case QCAPTURE :
        return select([]() { return true; });

    case PROBCUT :
        return select([&]() { return pos.see_ge(*cur, threshold); });
    }

    assert(false);
    return Move::none();  // Silence warning
}

void MovePicker::skip_quiet_moves() { skipQuiets = true; }

}  // namespace Alexander

// ============================================================================
// FILE ORIGINALE: movepick.h
// ============================================================================

/*
  Alexander, a UCI chess playing engine derived from Stockfish
  Copyright (C) 2004-2026 Alexander developers (see AUTHORS file)

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

#ifndef MOVEPICK_H_INCLUDED
#define MOVEPICK_H_INCLUDED

#include "history.h"
//from shashin begin
#include "shashin/shashin_position.h"
#include "shashin/moveconfig.h"
#include "bitboard.h"
//from shashin end
namespace Alexander {
static constexpr uint8_t AdjPawnCount[8] = {0, 1, 2, 2, 2, 2, 1, 0};
class Position;


// The MovePicker class is used to pick one pseudo-legal move at a time from the
// current position. The most important method is next_move(), which emits one
// new pseudo-legal move on every call, until there are no moves left, when
// Move::none() is returned. In order to improve the efficiency of the alpha-beta
// algorithm, MovePicker attempts to return the moves which are most likely to get
// a cut-off first.
class MovePicker {

   public:
    MovePicker(const MovePicker&)            = delete;
    MovePicker& operator=(const MovePicker&) = delete;
    MovePicker(const Position&,
               Move,
               Depth,
               const ButterflyHistory*,
               const LowPlyHistory*,
               const CapturePieceToHistory*,
               const PieceToHistory**,
               const SharedHistories*,
               int);
    MovePicker(const Position&, Move, int, const CapturePieceToHistory*);
    Move next_move();
    void skip_quiet_moves();

   private:
    template<typename Pred>
    Move select(Pred);
    template<GenType T>
    ExtMove* score(MoveList<T>&);
    ExtMove* begin() { return cur; }
    ExtMove* end() { return endCur; }

    const Position&              pos;
    const ButterflyHistory*      mainHistory;
    const LowPlyHistory*         lowPlyHistory;
    const CapturePieceToHistory* captureHistory;
    const PieceToHistory**       continuationHistory;
    const SharedHistories*       sharedHistory;
    Move                         ttMove;
    ExtMove *                    cur, *endCur, *endBadCaptures, *endCaptures, *endGenerated = moves;
    int                          stage;
    int                          threshold;
    Depth                        depth;
    int                          ply;
    bool                         skipQuiets = false;
    ExtMove                      moves[MAX_MOVES];
};

}  // namespace Alexander

#endif  // #ifndef MOVEPICK_H_INCLUDED


// ============================================================================
// FILE ORIGINALE: numa.h
// ============================================================================

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

#ifndef NUMA_H_INCLUDED
#define NUMA_H_INCLUDED

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <sstream>
#include <string>
#include <thread>
#include <utility>
#include <vector>
#include <cstring>

#include "shm.h"

// We support linux very well, but we explicitly do NOT support Android,
// because there is no affected systems, not worth maintaining.
#if defined(__linux__) && !defined(__ANDROID__)
    #if !defined(_GNU_SOURCE)
        #define _GNU_SOURCE
    #endif
    #include <sched.h>
#elif defined(_WIN64)

    #if _WIN32_WINNT < 0x0601
        #undef _WIN32_WINNT
        #define _WIN32_WINNT 0x0601  // Force to include needed API prototypes
    #endif

// On Windows each processor group can have up to 64 processors.
// https://learn.microsoft.com/en-us/windows/win32/procthread/processor-groups
static constexpr size_t WIN_PROCESSOR_GROUP_SIZE = 64;

    #if !defined(NOMINMAX)
        #define NOMINMAX
    #endif
    #include <windows.h>
    #if defined small
        #undef small
    #endif

// https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-setthreadselectedcpusetmasks
using SetThreadSelectedCpuSetMasks_t = BOOL (*)(HANDLE, PGROUP_AFFINITY, USHORT);

// https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-getthreadselectedcpusetmasks
using GetThreadSelectedCpuSetMasks_t = BOOL (*)(HANDLE, PGROUP_AFFINITY, USHORT, PUSHORT);

#endif

#include "misc.h"

namespace Alexander {

using CpuIndex  = size_t;
using NumaIndex = size_t;

inline CpuIndex get_hardware_concurrency() {
    CpuIndex concurrency = std::thread::hardware_concurrency();

    // Get all processors across all processor groups on windows, since
    // hardware_concurrency() only returns the number of processors in
    // the first group, because only these are available to std::thread.
#ifdef _WIN64
    concurrency = std::max<CpuIndex>(concurrency, GetActiveProcessorCount(ALL_PROCESSOR_GROUPS));
#endif

    return concurrency;
}

inline const CpuIndex SYSTEM_THREADS_NB = std::max<CpuIndex>(1, get_hardware_concurrency());

#if defined(_WIN64)

struct WindowsAffinity {
    std::optional<std::set<CpuIndex>> oldApi;
    std::optional<std::set<CpuIndex>> newApi;

    // We also provide diagnostic for when the affinity is set to nullopt
    // whether it was due to being indeterminate. If affinity is indeterminate
    // it is best to assume it is not set at all, so consistent with the meaning
    // of the nullopt affinity.
    bool isNewDeterminate = true;
    bool isOldDeterminate = true;

    std::optional<std::set<CpuIndex>> get_combined() const {
        if (!oldApi.has_value())
            return newApi;
        if (!newApi.has_value())
            return oldApi;

        std::set<CpuIndex> intersect;
        std::set_intersection(oldApi->begin(), oldApi->end(), newApi->begin(), newApi->end(),
                              std::inserter(intersect, intersect.begin()));
        return intersect;
    }

    // Since Windows 11 and Windows Server 2022 thread affinities can span
    // processor groups and can be set as such by a new WinAPI function. However,
    // we may need to force using the old API if we detect that the process has
    // affinity set by the old API already and we want to override that. Due to the
    // limitations of the old API we cannot detect its use reliably. There will be
    // cases where we detect not use but it has actually been used and vice versa.

    bool likely_used_old_api() const { return oldApi.has_value() || !isOldDeterminate; }
};

inline std::pair<BOOL, std::vector<USHORT>> get_process_group_affinity() {

    // GetProcessGroupAffinity requires the GroupArray argument to be
    // aligned to 4 bytes instead of just 2.
    static constexpr size_t GroupArrayMinimumAlignment = 4;
    static_assert(GroupArrayMinimumAlignment >= alignof(USHORT));

    // The function should succeed the second time, but it may fail if the group
    // affinity has changed between GetProcessGroupAffinity calls. In such case
    // we consider this a hard error, as we Cannot work with unstable affinities
    // anyway.
    static constexpr int MAX_TRIES  = 2;
    USHORT               GroupCount = 1;
    for (int i = 0; i < MAX_TRIES; ++i)
    {
        auto GroupArray = std::make_unique<USHORT[]>(
          GroupCount + (GroupArrayMinimumAlignment / alignof(USHORT) - 1));

        USHORT* GroupArrayAligned = align_ptr_up<GroupArrayMinimumAlignment>(GroupArray.get());

        const BOOL status =
          GetProcessGroupAffinity(GetCurrentProcess(), &GroupCount, GroupArrayAligned);

        if (status == 0 && GetLastError() != ERROR_INSUFFICIENT_BUFFER)
        {
            break;
        }

        if (status != 0)
        {
            return std::make_pair(status,
                                  std::vector(GroupArrayAligned, GroupArrayAligned + GroupCount));
        }
    }

    return std::make_pair(0, std::vector<USHORT>());
}

// On Windows there are two ways to set affinity, and therefore 2 ways to get it.
// These are not consistent, so we have to check both. In some cases it is actually
// not possible to determine affinity. For example when two different threads have
// affinity on different processor groups, set using SetThreadAffinityMask, we cannot
// retrieve the actual affinities.
// From documentation on GetProcessAffinityMask:
//     > If the calling process contains threads in multiple groups,
//     > the function returns zero for both affinity masks.
// In such cases we just give up and assume we have affinity for all processors.
// nullopt means no affinity is set, that is, all processors are allowed
inline WindowsAffinity get_process_affinity() {
    HMODULE k32                            = GetModuleHandle(TEXT("Kernel32.dll"));
    auto    GetThreadSelectedCpuSetMasks_f = GetThreadSelectedCpuSetMasks_t(
      (void (*)()) GetProcAddress(k32, "GetThreadSelectedCpuSetMasks"));

    BOOL status = 0;

    WindowsAffinity affinity;

    if (GetThreadSelectedCpuSetMasks_f != nullptr)
    {
        USHORT RequiredMaskCount;
        status = GetThreadSelectedCpuSetMasks_f(GetCurrentThread(), nullptr, 0, &RequiredMaskCount);

        // We expect ERROR_INSUFFICIENT_BUFFER from GetThreadSelectedCpuSetMasks,
        // but other failure is an actual error.
        if (status == 0 && GetLastError() != ERROR_INSUFFICIENT_BUFFER)
        {
            affinity.isNewDeterminate = false;
        }
        else if (RequiredMaskCount > 0)
        {
            // If RequiredMaskCount then these affinities were never set, but it's
            // not consistent so GetProcessAffinityMask may still return some affinity.
            auto groupAffinities = std::make_unique<GROUP_AFFINITY[]>(RequiredMaskCount);

            status = GetThreadSelectedCpuSetMasks_f(GetCurrentThread(), groupAffinities.get(),
                                                    RequiredMaskCount, &RequiredMaskCount);

            if (status == 0)
            {
                affinity.isNewDeterminate = false;
            }
            else
            {
                std::set<CpuIndex> cpus;

                for (USHORT i = 0; i < RequiredMaskCount; ++i)
                {
                    const size_t procGroupIndex = groupAffinities[i].Group;

                    for (size_t j = 0; j < WIN_PROCESSOR_GROUP_SIZE; ++j)
                    {
                        if (groupAffinities[i].Mask & (KAFFINITY(1) << j))
                            cpus.insert(procGroupIndex * WIN_PROCESSOR_GROUP_SIZE + j);
                    }
                }

                affinity.newApi = std::move(cpus);
            }
        }
    }

    // NOTE: There is no way to determine full affinity using the old API if
    //       individual threads set affinity on different processor groups.

    DWORD_PTR proc, sys;
    status = GetProcessAffinityMask(GetCurrentProcess(), &proc, &sys);

    // If proc == 0 then we cannot determine affinity because it spans processor groups.
    // On Windows 11 and Server 2022 it will instead
    //     > If, however, hHandle specifies a handle to the current process, the function
    //     > always uses the calling thread's primary group (which by default is the same
    //     > as the process' primary group) in order to set the
    //     > lpProcessAffinityMask and lpSystemAffinityMask.
    // So it will never be indeterminate here. We can only make assumptions later.
    if (status == 0 || proc == 0)
    {
        affinity.isOldDeterminate = false;
        return affinity;
    }

    // If SetProcessAffinityMask was never called the affinity must span
    // all processor groups, but if it was called it must only span one.

    std::vector<USHORT> groupAffinity;  // We need to capture this later and capturing
                                        // from structured bindings requires c++20.

    std::tie(status, groupAffinity) = get_process_group_affinity();
    if (status == 0)
    {
        affinity.isOldDeterminate = false;
        return affinity;
    }

    if (groupAffinity.size() == 1)
    {
        // We detect the case when affinity is set to all processors and correctly
        // leave affinity.oldApi as nullopt.
        if (GetActiveProcessorGroupCount() != 1 || proc != sys)
        {
            std::set<CpuIndex> cpus;

            const size_t procGroupIndex = groupAffinity[0];

            const uint64_t mask = static_cast<uint64_t>(proc);
            for (size_t j = 0; j < WIN_PROCESSOR_GROUP_SIZE; ++j)
            {
                if (mask & (KAFFINITY(1) << j))
                    cpus.insert(procGroupIndex * WIN_PROCESSOR_GROUP_SIZE + j);
            }

            affinity.oldApi = std::move(cpus);
        }
    }
    else
    {
        // If we got here it means that either SetProcessAffinityMask was never set
        // or we're on Windows 11/Server 2022.

        // Since Windows 11 and Windows Server 2022 the behaviour of
        // GetProcessAffinityMask changed:
        //     > If, however, hHandle specifies a handle to the current process,
        //     > the function always uses the calling thread's primary group
        //     > (which by default is the same as the process' primary group)
        //     > in order to set the lpProcessAffinityMask and lpSystemAffinityMask.
        // In which case we can actually retrieve the full affinity.

        if (GetThreadSelectedCpuSetMasks_f != nullptr)
        {
            std::thread th([&]() {
                std::set<CpuIndex> cpus;
                bool               isAffinityFull = true;

                for (auto procGroupIndex : groupAffinity)
                {
                    const int numActiveProcessors =
                      GetActiveProcessorCount(static_cast<WORD>(procGroupIndex));

                    // We have to schedule to two different processors
                    // and & the affinities we get. Otherwise our processor
                    // choice could influence the resulting affinity.
                    // We assume the processor IDs within the group are
                    // filled sequentially from 0.
                    uint64_t procCombined = std::numeric_limits<uint64_t>::max();
                    uint64_t sysCombined  = std::numeric_limits<uint64_t>::max();

                    for (int i = 0; i < std::min(numActiveProcessors, 2); ++i)
                    {
                        GROUP_AFFINITY GroupAffinity;
                        std::memset(&GroupAffinity, 0, sizeof(GROUP_AFFINITY));
                        GroupAffinity.Group = static_cast<WORD>(procGroupIndex);

                        GroupAffinity.Mask = static_cast<KAFFINITY>(1) << i;

                        status =
                          SetThreadGroupAffinity(GetCurrentThread(), &GroupAffinity, nullptr);
                        if (status == 0)
                        {
                            affinity.isOldDeterminate = false;
                            return;
                        }

                        SwitchToThread();

                        DWORD_PTR proc2, sys2;
                        status = GetProcessAffinityMask(GetCurrentProcess(), &proc2, &sys2);
                        if (status == 0)
                        {
                            affinity.isOldDeterminate = false;
                            return;
                        }

                        procCombined &= static_cast<uint64_t>(proc2);
                        sysCombined &= static_cast<uint64_t>(sys2);
                    }

                    if (procCombined != sysCombined)
                        isAffinityFull = false;

                    for (size_t j = 0; j < WIN_PROCESSOR_GROUP_SIZE; ++j)
                    {
                        if (procCombined & (KAFFINITY(1) << j))
                            cpus.insert(procGroupIndex * WIN_PROCESSOR_GROUP_SIZE + j);
                    }
                }

                // We have to detect the case where the affinity was not set,
                // or is set to all processors so that we correctly produce as
                // std::nullopt result.
                if (!isAffinityFull)
                {
                    affinity.oldApi = std::move(cpus);
                }
            });

            th.join();
        }
    }

    return affinity;
}

// Type machinery used to emulate Cache->GroupCount

template<typename T, typename = void>
struct HasGroupCount: std::false_type {};

template<typename T>
struct HasGroupCount<T, std::void_t<decltype(std::declval<T>().Cache.GroupCount)>>: std::true_type {
};

template<typename T, typename Pred, std::enable_if_t<HasGroupCount<T>::value, bool> = true>
std::set<CpuIndex> readCacheMembers(const T* info, Pred&& is_cpu_allowed) {
    std::set<CpuIndex> cpus;
    // On Windows 10 this will read a 0 because GroupCount doesn't exist
    int groupCount = std::max(info->Cache.GroupCount, WORD(1));
    for (WORD procGroup = 0; procGroup < groupCount; ++procGroup)
    {
        for (BYTE number = 0; number < WIN_PROCESSOR_GROUP_SIZE; ++number)
        {
            WORD           groupNumber = info->Cache.GroupMasks[procGroup].Group;
            const CpuIndex c = static_cast<CpuIndex>(groupNumber) * WIN_PROCESSOR_GROUP_SIZE
                             + static_cast<CpuIndex>(number);
            if (!(info->Cache.GroupMasks[procGroup].Mask & (1ULL << number)) || !is_cpu_allowed(c))
                continue;
            cpus.insert(c);
        }
    }
    return cpus;
}

template<typename T, typename Pred, std::enable_if_t<!HasGroupCount<T>::value, bool> = true>
std::set<CpuIndex> readCacheMembers(const T* info, Pred&& is_cpu_allowed) {
    std::set<CpuIndex> cpus;
    for (BYTE number = 0; number < WIN_PROCESSOR_GROUP_SIZE; ++number)
    {
        WORD           groupNumber = info->Cache.GroupMask.Group;
        const CpuIndex c           = static_cast<CpuIndex>(groupNumber) * WIN_PROCESSOR_GROUP_SIZE
                         + static_cast<CpuIndex>(number);
        if (!(info->Cache.GroupMask.Mask & (1ULL << number)) || !is_cpu_allowed(c))
            continue;
        cpus.insert(c);
    }
    return cpus;
}

#endif

#if defined(__linux__) && !defined(__ANDROID__)

inline std::set<CpuIndex> get_process_affinity() {

    std::set<CpuIndex> cpus;

    // For unsupported systems, or in case of a soft error, we may assume
    // all processors are available for use.
    [[maybe_unused]] auto set_to_all_cpus = [&]() {
        for (CpuIndex c = 0; c < SYSTEM_THREADS_NB; ++c)
            cpus.insert(c);
    };

    // cpu_set_t by default holds 1024 entries. This may not be enough soon,
    // but there is no easy way to determine how many threads there actually
    // is. In this case we just choose a reasonable upper bound.
    static constexpr CpuIndex MaxNumCpus = 1024 * 64;

    cpu_set_t* mask = CPU_ALLOC(MaxNumCpus);
    if (mask == nullptr)
        std::exit(EXIT_FAILURE);

    const size_t masksize = CPU_ALLOC_SIZE(MaxNumCpus);

    CPU_ZERO_S(masksize, mask);

    const int status = sched_getaffinity(0, masksize, mask);

    if (status != 0)
    {
        CPU_FREE(mask);
        std::exit(EXIT_FAILURE);
    }

    for (CpuIndex c = 0; c < MaxNumCpus; ++c)
        if (CPU_ISSET_S(c, masksize, mask))
            cpus.insert(c);

    CPU_FREE(mask);

    return cpus;
}

#endif

#if defined(__linux__) && !defined(__ANDROID__)

inline static const auto STARTUP_PROCESSOR_AFFINITY = get_process_affinity();

#elif defined(_WIN64)

inline static const auto STARTUP_PROCESSOR_AFFINITY = get_process_affinity();
inline static const auto STARTUP_USE_OLD_AFFINITY_API =
  STARTUP_PROCESSOR_AFFINITY.likely_used_old_api();

#endif

// We want to abstract the purpose of storing the numa node index somewhat.
// Whoever is using this does not need to know the specifics of the replication
// machinery to be able to access NUMA replicated memory.
class NumaReplicatedAccessToken {
   public:
    NumaReplicatedAccessToken() :
        n(0) {}

    explicit NumaReplicatedAccessToken(NumaIndex idx) :
        n(idx) {}

    NumaIndex get_numa_index() const { return n; }

   private:
    NumaIndex n;
};

struct L3Domain {
    NumaIndex          systemNumaIndex{};
    std::set<CpuIndex> cpus{};
};

// Use system NUMA nodes
struct SystemNumaPolicy {};
// Use system-reported L3 domains
struct L3DomainsPolicy {};
// Group system-reported L3 domains until they reach bundleSize
struct BundledL3Policy {
    size_t bundleSize;
};

using NumaAutoPolicy = std::variant<SystemNumaPolicy, L3DomainsPolicy, BundledL3Policy>;

// Designed as immutable, because there is no good reason to alter an already
// existing config in a way that doesn't require recreating it completely, and
// it would be complex and expensive to maintain class invariants.
// The CPU (processor) numbers always correspond to the actual numbering used
// by the system. The NUMA node numbers MAY NOT correspond to the system's
// numbering of the NUMA nodes. In particular, by default, if the processor has
// non-uniform cache access within a NUMA node (i.e., a non-unified L3 cache structure),
// then L3 domains within a system NUMA node will be used to subdivide it
// into multiple logical NUMA nodes in the config. Additionally, empty nodes may
// be removed, or the user may create custom nodes.
//
// As a special case, when performing system-wide replication of read-only data
// (i.e., LazyNumaReplicatedSystemWide), the system NUMA node is used, rather than
// custom or L3-aware nodes. See that class's get_discriminator() function.
//
// It is guaranteed that NUMA nodes are NOT empty: every node exposed by NumaConfig
// has at least one processor assigned.
//
// We use startup affinities so as not to modify its own behaviour in time.
//
// Since Stockfish doesn't support exceptions all places where an exception
// should be thrown are replaced by std::exit.
class NumaConfig {
   public:
    NumaConfig() :
        highestCpuIndex(0),
        customAffinity(false) {
        const auto numCpus = SYSTEM_THREADS_NB;
        add_cpu_range_to_node(NumaIndex{0}, CpuIndex{0}, numCpus - 1);
    }

    // This function gets a NumaConfig based on the system's provided information.
    // The available policies are documented above.
    static NumaConfig from_system([[maybe_unused]] const NumaAutoPolicy& policy,
                                  bool respectProcessAffinity = true) {
        NumaConfig cfg = empty();

#if !((defined(__linux__) && !defined(__ANDROID__)) || defined(_WIN64))
        // Fallback for unsupported systems.
        for (CpuIndex c = 0; c < SYSTEM_THREADS_NB; ++c)
            cfg.add_cpu_to_node(NumaIndex{0}, c);
#else

    #if defined(_WIN64)

        std::optional<std::set<CpuIndex>> allowedCpus;

        if (respectProcessAffinity)
            allowedCpus = STARTUP_PROCESSOR_AFFINITY.get_combined();

        // The affinity cannot be determined in all cases on Windows,
        // but we at least guarantee that the number of allowed processors
        // is >= number of processors in the affinity mask. In case the user
        // is not satisfied they must set the processor numbers explicitly.
        auto is_cpu_allowed = [&allowedCpus](CpuIndex c) {
            return !allowedCpus.has_value() || allowedCpus->count(c) == 1;
        };

    #elif defined(__linux__) && !defined(__ANDROID__)

        std::set<CpuIndex> allowedCpus;

        if (respectProcessAffinity)
            allowedCpus = STARTUP_PROCESSOR_AFFINITY;

        auto is_cpu_allowed = [respectProcessAffinity, &allowedCpus](CpuIndex c) {
            return !respectProcessAffinity || allowedCpus.count(c) == 1;
        };

    #endif

        bool l3Success = false;
        if (!std::holds_alternative<SystemNumaPolicy>(policy))
        {
            size_t l3BundleSize = 0;
            if (const auto* v = std::get_if<BundledL3Policy>(&policy))
            {
                l3BundleSize = v->bundleSize;
            }
            if (auto l3Cfg =
                  try_get_l3_aware_config(respectProcessAffinity, l3BundleSize, is_cpu_allowed))
            {
                cfg       = std::move(*l3Cfg);
                l3Success = true;
            }
        }
        if (!l3Success)
            cfg = from_system_numa(respectProcessAffinity, is_cpu_allowed);

    #if defined(_WIN64)
        // Split the NUMA nodes to be contained within a group if necessary.
        // This is needed between Windows 10 Build 20348 and Windows 11, because
        // the new NUMA allocation behaviour was introduced while there was
        // still no way to set thread affinity spanning multiple processor groups.
        // See https://learn.microsoft.com/en-us/windows/win32/procthread/numa-support
        // We also do this is if need to force old API for some reason.
        //
        // 2024-08-26: It appears that we need to actually always force this behaviour.
        // While Windows allows this to work now, such assignments have bad interaction
        // with the scheduler - in particular it still prefers scheduling on the thread's
        // "primary" node, even if it means scheduling SMT processors first.
        // See https://github.com/official-stockfish/Stockfish/issues/5551
        // See https://learn.microsoft.com/en-us/windows/win32/procthread/processor-groups
        //
        //     Each process is assigned a primary group at creation, and by default all
        //     of its threads' primary group is the same. Each thread's ideal processor
        //     is in the thread's primary group, so threads will preferentially be
        //     scheduled to processors on their primary group, but they are able to
        //     be scheduled to processors on any other group.
        //
        // used to be guarded by if (STARTUP_USE_OLD_AFFINITY_API)
        {
            NumaConfig splitCfg = empty();

            NumaIndex splitNodeIndex = 0;
            for (const auto& cpus : cfg.nodes)
            {
                if (cpus.empty())
                    continue;

                size_t lastProcGroupIndex = *(cpus.begin()) / WIN_PROCESSOR_GROUP_SIZE;
                for (CpuIndex c : cpus)
                {
                    const size_t procGroupIndex = c / WIN_PROCESSOR_GROUP_SIZE;
                    if (procGroupIndex != lastProcGroupIndex)
                    {
                        splitNodeIndex += 1;
                        lastProcGroupIndex = procGroupIndex;
                    }
                    splitCfg.add_cpu_to_node(splitNodeIndex, c);
                }
                splitNodeIndex += 1;
            }

            cfg = std::move(splitCfg);
        }
    #endif

#endif

        // We have to ensure no empty NUMA nodes persist.
        cfg.remove_empty_numa_nodes();

        // If the user explicitly opts out from respecting the current process affinity
        // then it may be inconsistent with the current affinity (obviously), so we
        // consider it custom.
        if (!respectProcessAffinity)
            cfg.customAffinity = true;

        return cfg;
    }

    // ':'-separated numa nodes
    // ','-separated cpu indices
    // supports "first-last" range syntax for cpu indices
    // For example "0-15,128-143:16-31,144-159:32-47,160-175:48-63,176-191"
    static NumaConfig from_string(const std::string& s) {
        NumaConfig cfg = empty();

        NumaIndex n = 0;
        for (auto&& nodeStr : split(s, ":"))
        {
            auto indices = indices_from_shortened_string(std::string(nodeStr));
            if (!indices.empty())
            {
                for (auto idx : indices)
                {
                    if (!cfg.add_cpu_to_node(n, CpuIndex(idx)))
                        std::exit(EXIT_FAILURE);
                }

                n += 1;
            }
        }

        cfg.customAffinity = true;

        return cfg;
    }

    NumaConfig(const NumaConfig&)            = delete;
    NumaConfig(NumaConfig&&)                 = default;
    NumaConfig& operator=(const NumaConfig&) = delete;
    NumaConfig& operator=(NumaConfig&&)      = default;

    bool is_cpu_assigned(CpuIndex n) const { return nodeByCpu.count(n) == 1; }

    NumaIndex num_numa_nodes() const { return nodes.size(); }

    CpuIndex num_cpus_in_numa_node(NumaIndex n) const {
        assert(n < nodes.size());
        return nodes[n].size();
    }

    CpuIndex num_cpus() const { return nodeByCpu.size(); }

    bool requires_memory_replication() const { return customAffinity || nodes.size() > 1; }

    std::string to_string() const {
        std::string str;

        bool isFirstNode = true;
        for (auto&& cpus : nodes)
        {
            if (!isFirstNode)
                str += ":";

            bool isFirstSet = true;
            auto rangeStart = cpus.begin();
            for (auto it = cpus.begin(); it != cpus.end(); ++it)
            {
                auto next = std::next(it);
                if (next == cpus.end() || *next != *it + 1)
                {
                    // cpus[i] is at the end of the range (may be of size 1)
                    if (!isFirstSet)
                        str += ",";

                    const CpuIndex last = *it;

                    if (it != rangeStart)
                    {
                        const CpuIndex first = *rangeStart;

                        str += std::to_string(first);
                        str += "-";
                        str += std::to_string(last);
                    }
                    else
                        str += std::to_string(last);

                    rangeStart = next;
                    isFirstSet = false;
                }
            }

            isFirstNode = false;
        }

        return str;
    }

    bool suggests_binding_threads(CpuIndex numThreads) const {
        // If we can reasonably determine that the threads cannot be contained
        // by the OS within the first NUMA node then we advise distributing
        // and binding threads. When the threads are not bound we can only use
        // NUMA memory replicated objects from the first node, so when the OS
        // has to schedule on other nodes we lose performance. We also suggest
        // binding if there's enough threads to distribute among nodes with minimal
        // disparity. We try to ignore small nodes, in particular the empty ones.

        // If the affinity set by the user does not match the affinity given by
        // the OS then binding is necessary to ensure the threads are running on
        // correct processors.
        if (customAffinity)
            return true;

        // We obviously cannot distribute a single thread, so a single thread
        // should never be bound.
        if (numThreads <= 1)
            return false;

        size_t largestNodeSize = 0;
        for (auto&& cpus : nodes)
            if (cpus.size() > largestNodeSize)
                largestNodeSize = cpus.size();

        auto is_node_small = [largestNodeSize](const std::set<CpuIndex>& node) {
            static constexpr double SmallNodeThreshold = 0.6;
            return static_cast<double>(node.size()) / static_cast<double>(largestNodeSize)
                <= SmallNodeThreshold;
        };

        size_t numNotSmallNodes = 0;
        for (auto&& cpus : nodes)
            if (!is_node_small(cpus))
                numNotSmallNodes += 1;

        return (numThreads > largestNodeSize / 2 || numThreads >= numNotSmallNodes * 4)
            && nodes.size() > 1;
    }

    std::vector<NumaIndex> distribute_threads_among_numa_nodes(CpuIndex numThreads) const {
        std::vector<NumaIndex> ns;

        if (nodes.size() == 1)
        {
            // Special case for when there's no NUMA nodes. This doesn't buy us
            // much, but let's keep the default path simple.
            ns.resize(numThreads, NumaIndex{0});
        }
        else
        {
            std::vector<size_t> occupation(nodes.size(), 0);
            for (CpuIndex c = 0; c < numThreads; ++c)
            {
                NumaIndex bestNode{0};
                float     bestNodeFill = std::numeric_limits<float>::max();
                for (NumaIndex n = 0; n < nodes.size(); ++n)
                {
                    float fill =
                      static_cast<float>(occupation[n] + 1) / static_cast<float>(nodes[n].size());
                    // NOTE: Do we want to perhaps fill the first available node
                    //       up to 50% first before considering other nodes?
                    //       Probably not, because it would interfere with running
                    //       multiple instances. We basically shouldn't favor any
                    //       particular node.
                    if (fill < bestNodeFill)
                    {
                        bestNode     = n;
                        bestNodeFill = fill;
                    }
                }
                ns.emplace_back(bestNode);
                occupation[bestNode] += 1;
            }
        }

        return ns;
    }

    NumaReplicatedAccessToken bind_current_thread_to_numa_node(NumaIndex n) const {
        if (n >= nodes.size() || nodes[n].size() == 0)
            std::exit(EXIT_FAILURE);

#if defined(__linux__) && !defined(__ANDROID__)

        cpu_set_t* mask = CPU_ALLOC(highestCpuIndex + 1);
        if (mask == nullptr)
            std::exit(EXIT_FAILURE);

        const size_t masksize = CPU_ALLOC_SIZE(highestCpuIndex + 1);

        CPU_ZERO_S(masksize, mask);

        for (CpuIndex c : nodes[n])
            CPU_SET_S(c, masksize, mask);

        const int status = sched_setaffinity(0, masksize, mask);

        CPU_FREE(mask);

        if (status != 0)
            std::exit(EXIT_FAILURE);

        // We yield this thread just to be sure it gets rescheduled.
        // This is defensive, allowed because this code is not performance critical.
        sched_yield();

#elif defined(_WIN64)

        // Requires Windows 11. No good way to set thread affinity spanning
        // processor groups before that.
        HMODULE k32                            = GetModuleHandle(TEXT("Kernel32.dll"));
        auto    SetThreadSelectedCpuSetMasks_f = SetThreadSelectedCpuSetMasks_t(
          (void (*)()) GetProcAddress(k32, "SetThreadSelectedCpuSetMasks"));

        // We ALWAYS set affinity with the new API if available, because
        // there's no downsides, and we forcibly keep it consistent with
        // the old API should we need to use it. I.e. we always keep this
        // as a superset of what we set with SetThreadGroupAffinity.
        if (SetThreadSelectedCpuSetMasks_f != nullptr)
        {
            // Only available on Windows 11 and Windows Server 2022 onwards
            const USHORT numProcGroups = USHORT(
              ((highestCpuIndex + 1) + WIN_PROCESSOR_GROUP_SIZE - 1) / WIN_PROCESSOR_GROUP_SIZE);
            auto groupAffinities = std::make_unique<GROUP_AFFINITY[]>(numProcGroups);
            std::memset(groupAffinities.get(), 0, sizeof(GROUP_AFFINITY) * numProcGroups);
            for (WORD i = 0; i < numProcGroups; ++i)
                groupAffinities[i].Group = i;

            for (CpuIndex c : nodes[n])
            {
                const size_t procGroupIndex     = c / WIN_PROCESSOR_GROUP_SIZE;
                const size_t idxWithinProcGroup = c % WIN_PROCESSOR_GROUP_SIZE;
                groupAffinities[procGroupIndex].Mask |= KAFFINITY(1) << idxWithinProcGroup;
            }

            HANDLE hThread = GetCurrentThread();

            const BOOL status =
              SetThreadSelectedCpuSetMasks_f(hThread, groupAffinities.get(), numProcGroups);
            if (status == 0)
                std::exit(EXIT_FAILURE);

            // We yield this thread just to be sure it gets rescheduled.
            // This is defensive, allowed because this code is not performance critical.
            SwitchToThread();
        }

        // Sometimes we need to force the old API, but do not use it unless necessary.
        if (SetThreadSelectedCpuSetMasks_f == nullptr || STARTUP_USE_OLD_AFFINITY_API)
        {
            // On earlier windows version (since windows 7) we cannot run a single thread
            // on multiple processor groups, so we need to restrict the group.
            // We assume the group of the first processor listed for this node.
            // Processors from outside this group will not be assigned for this thread.
            // Normally this won't be an issue because windows used to assign NUMA nodes
            // such that they cannot span processor groups. However, since Windows 10
            // Build 20348 the behaviour changed, so there's a small window of versions
            // between this and Windows 11 that might exhibit problems with not all
            // processors being utilized.
            //
            // We handle this in NumaConfig::from_system by manually splitting the
            // nodes when we detect that there is no function to set affinity spanning
            // processor nodes. This is required because otherwise our thread distribution
            // code may produce suboptimal results.
            //
            // See https://learn.microsoft.com/en-us/windows/win32/procthread/numa-support
            GROUP_AFFINITY affinity;
            std::memset(&affinity, 0, sizeof(GROUP_AFFINITY));
            // We use an ordered set to be sure to get the smallest cpu number here.
            const size_t forcedProcGroupIndex = *(nodes[n].begin()) / WIN_PROCESSOR_GROUP_SIZE;
            affinity.Group                    = static_cast<WORD>(forcedProcGroupIndex);
            for (CpuIndex c : nodes[n])
            {
                const size_t procGroupIndex     = c / WIN_PROCESSOR_GROUP_SIZE;
                const size_t idxWithinProcGroup = c % WIN_PROCESSOR_GROUP_SIZE;
                // We skip processors that are not in the same processor group.
                // If everything was set up correctly this will never be an issue,
                // but we have to account for bad NUMA node specification.
                if (procGroupIndex != forcedProcGroupIndex)
                    continue;

                affinity.Mask |= KAFFINITY(1) << idxWithinProcGroup;
            }

            HANDLE hThread = GetCurrentThread();

            const BOOL status = SetThreadGroupAffinity(hThread, &affinity, nullptr);
            if (status == 0)
                std::exit(EXIT_FAILURE);

            // We yield this thread just to be sure it gets rescheduled. This is
            // defensive, allowed because this code is not performance critical.
            SwitchToThread();
        }

#endif

        return NumaReplicatedAccessToken(n);
    }

    template<typename FuncT>
    void execute_on_numa_node(NumaIndex n, FuncT&& f) const {
        std::thread th([this, &f, n]() {
            bind_current_thread_to_numa_node(n);
            std::forward<FuncT>(f)();
        });

        th.join();
    }

    std::vector<std::set<CpuIndex>> nodes;
    std::map<CpuIndex, NumaIndex>   nodeByCpu;

   private:
    CpuIndex highestCpuIndex;

    bool customAffinity;

    static NumaConfig empty() { return NumaConfig(EmptyNodeTag{}); }

    struct EmptyNodeTag {};

    NumaConfig(EmptyNodeTag) :
        highestCpuIndex(0),
        customAffinity(false) {}

    void remove_empty_numa_nodes() {
        std::vector<std::set<CpuIndex>> newNodes;
        for (auto&& cpus : nodes)
            if (!cpus.empty())
                newNodes.emplace_back(std::move(cpus));
        nodes = std::move(newNodes);
    }

    // Returns true if successful
    // Returns false if failed, i.e. when the cpu is already present
    //                          strong guarantee, the structure remains unmodified
    bool add_cpu_to_node(NumaIndex n, CpuIndex c) {
        if (is_cpu_assigned(c))
            return false;

        while (nodes.size() <= n)
            nodes.emplace_back();

        nodes[n].insert(c);
        nodeByCpu[c] = n;

        if (c > highestCpuIndex)
            highestCpuIndex = c;

        return true;
    }

    // Returns true if successful
    // Returns false if failed, i.e. when any of the cpus is already present
    //                          strong guarantee, the structure remains unmodified
    bool add_cpu_range_to_node(NumaIndex n, CpuIndex cfirst, CpuIndex clast) {
        for (CpuIndex c = cfirst; c <= clast; ++c)
            if (is_cpu_assigned(c))
                return false;

        while (nodes.size() <= n)
            nodes.emplace_back();

        for (CpuIndex c = cfirst; c <= clast; ++c)
        {
            nodes[n].insert(c);
            nodeByCpu[c] = n;
        }

        if (clast > highestCpuIndex)
            highestCpuIndex = clast;

        return true;
    }

    static std::vector<size_t> indices_from_shortened_string(const std::string& s) {
        std::vector<size_t> indices;

        if (s.empty())
            return indices;

        for (const auto& ss : split(s, ","))
        {
            if (ss.empty())
                continue;

            auto parts = split(ss, "-");
            if (parts.size() == 1)
            {
                const CpuIndex c = CpuIndex{str_to_size_t(std::string(parts[0]))};
                indices.emplace_back(c);
            }
            else if (parts.size() == 2)
            {
                const CpuIndex cfirst = CpuIndex{str_to_size_t(std::string(parts[0]))};
                const CpuIndex clast  = CpuIndex{str_to_size_t(std::string(parts[1]))};
                for (size_t c = cfirst; c <= clast; ++c)
                {
                    indices.emplace_back(c);
                }
            }
        }

        return indices;
    }

    // This function queries the system for the mapping of processors to NUMA nodes.
    // On Linux we read from standardized kernel sysfs, with a fallback to single NUMA
    // node. On Windows we utilize GetNumaProcessorNodeEx, which has its quirks, see
    // comment for Windows implementation of get_process_affinity.
    template<typename Pred>
    static NumaConfig from_system_numa([[maybe_unused]] bool   respectProcessAffinity,
                                       [[maybe_unused]] Pred&& is_cpu_allowed) {
        NumaConfig cfg = empty();

#if defined(__linux__) && !defined(__ANDROID__)

        // On Linux things are straightforward, since there's no processor groups and
        // any thread can be scheduled on all processors.
        // We try to gather this information from the sysfs first
        // https://www.kernel.org/doc/Documentation/ABI/stable/sysfs-devices-node

        bool useFallback = false;
        auto fallback    = [&]() {
            useFallback = true;
            cfg         = empty();
        };

        // /sys/devices/system/node/online contains information about active NUMA nodes
        auto nodeIdsStr = read_file_to_string("/sys/devices/system/node/online");
        if (!nodeIdsStr.has_value() || nodeIdsStr->empty())
        {
            fallback();
        }
        else
        {
            remove_whitespace(*nodeIdsStr);
            for (size_t n : indices_from_shortened_string(*nodeIdsStr))
            {
                // /sys/devices/system/node/node.../cpulist
                std::string path =
                  std::string("/sys/devices/system/node/node") + std::to_string(n) + "/cpulist";
                auto cpuIdsStr = read_file_to_string(path);
                // Now, we only bail if the file does not exist. Some nodes may be
                // empty, that's fine. An empty node still has a file that appears
                // to have some whitespace, so we need to handle that.
                if (!cpuIdsStr.has_value())
                {
                    fallback();
                    break;
                }
                else
                {
                    remove_whitespace(*cpuIdsStr);
                    for (size_t c : indices_from_shortened_string(*cpuIdsStr))
                    {
                        if (is_cpu_allowed(c))
                            cfg.add_cpu_to_node(n, c);
                    }
                }
            }
        }

        if (useFallback)
        {
            for (CpuIndex c = 0; c < SYSTEM_THREADS_NB; ++c)
                if (is_cpu_allowed(c))
                    cfg.add_cpu_to_node(NumaIndex{0}, c);
        }

#elif defined(_WIN64)

        WORD numProcGroups = GetActiveProcessorGroupCount();
        for (WORD procGroup = 0; procGroup < numProcGroups; ++procGroup)
        {
            for (BYTE number = 0; number < WIN_PROCESSOR_GROUP_SIZE; ++number)
            {
                PROCESSOR_NUMBER procnum;
                procnum.Group    = procGroup;
                procnum.Number   = number;
                procnum.Reserved = 0;
                USHORT nodeNumber;

                const BOOL     status = GetNumaProcessorNodeEx(&procnum, &nodeNumber);
                const CpuIndex c      = static_cast<CpuIndex>(procGroup) * WIN_PROCESSOR_GROUP_SIZE
                                 + static_cast<CpuIndex>(number);
                if (status != 0 && nodeNumber != std::numeric_limits<USHORT>::max()
                    && is_cpu_allowed(c))
                {
                    cfg.add_cpu_to_node(nodeNumber, c);
                }
            }
        }

#else

        abort();  // should not reach here

#endif

        return cfg;
    }

    template<typename Pred>
    static std::optional<NumaConfig> try_get_l3_aware_config(
      bool respectProcessAffinity, size_t bundleSize, [[maybe_unused]] Pred&& is_cpu_allowed) {
        // Get the normal system configuration so we know to which NUMA node
        // each L3 domain belongs.
        NumaConfig systemConfig =
          NumaConfig::from_system(SystemNumaPolicy{}, respectProcessAffinity);
        std::vector<L3Domain> l3Domains;

#if defined(__linux__) && !defined(__ANDROID__)

        std::set<CpuIndex> seenCpus;
        auto               nextUnseenCpu = [&seenCpus]() {
            for (CpuIndex i = 0;; ++i)
                if (!seenCpus.count(i))
                    return i;
        };

        while (true)
        {
            CpuIndex next = nextUnseenCpu();
            auto     siblingsStr =
              read_file_to_string("/sys/devices/system/cpu/cpu" + std::to_string(next)
                                  + "/cache/index3/shared_cpu_list");

            if (!siblingsStr.has_value() || siblingsStr->empty())
            {
                break;  // we have read all available CPUs
            }

            L3Domain domain;
            for (size_t c : indices_from_shortened_string(*siblingsStr))
            {
                if (is_cpu_allowed(c))
                {
                    domain.systemNumaIndex = systemConfig.nodeByCpu.at(c);
                    domain.cpus.insert(c);
                }
                seenCpus.insert(c);
            }
            if (!domain.cpus.empty())
            {
                l3Domains.emplace_back(std::move(domain));
            }
        }

#elif defined(_WIN64)

        DWORD bufSize = 0;
        GetLogicalProcessorInformationEx(RelationCache, nullptr, &bufSize);
        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
            return std::nullopt;

        std::vector<char> buffer(bufSize);
        auto info = reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(buffer.data());
        if (!GetLogicalProcessorInformationEx(RelationCache, info, &bufSize))
            return std::nullopt;

        while (reinterpret_cast<char*>(info) < buffer.data() + bufSize)
        {
            info = std::launder(info);
            if (info->Relationship == RelationCache && info->Cache.Level == 3)
            {
                L3Domain domain{};
                domain.cpus = readCacheMembers(info, is_cpu_allowed);
                if (!domain.cpus.empty())
                {
                    domain.systemNumaIndex = systemConfig.nodeByCpu.at(*domain.cpus.begin());
                    l3Domains.push_back(std::move(domain));
                }
            }
            // Variable length data structure, advance to next
            info = reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(
              reinterpret_cast<char*>(info) + info->Size);
        }
#endif

        if (!l3Domains.empty())
            return {NumaConfig::from_l3_info(std::move(l3Domains), bundleSize)};

        return std::nullopt;
    }


    static NumaConfig from_l3_info(std::vector<L3Domain>&& domains, size_t bundleSize) {
        assert(!domains.empty());

        std::map<NumaIndex, std::vector<L3Domain>> list;
        for (auto& d : domains)
            list[d.systemNumaIndex].emplace_back(std::move(d));

        NumaConfig cfg = empty();
        NumaIndex  n   = 0;
        for (auto& [_, ds] : list)
        {
            bool changed;
            // Scan through pairs and merge them. With roughly equal L3 sizes, should give
            // a decent distribution.
            do
            {
                changed = false;
                for (size_t j = 0; j + 1 < ds.size(); ++j)
                {
                    if (ds[j].cpus.size() + ds[j + 1].cpus.size() <= bundleSize)
                    {
                        changed = true;
                        ds[j].cpus.merge(ds[j + 1].cpus);
                        ds.erase(ds.begin() + j + 1);
                    }
                }
                // ds.size() has decreased if changed is true, so this loop will terminate
            } while (changed);
            for (const L3Domain& d : ds)
            {
                const NumaIndex dn = n++;
                for (CpuIndex cpu : d.cpus)
                {
                    cfg.add_cpu_to_node(dn, cpu);
                }
            }
        }
        return cfg;
    }
};

class NumaReplicationContext;

// Instances of this class are tracked by the NumaReplicationContext instance.
// NumaReplicationContext informs all tracked instances when NUMA configuration changes.
class NumaReplicatedBase {
   public:
    NumaReplicatedBase(NumaReplicationContext& ctx);

    NumaReplicatedBase(const NumaReplicatedBase&) = delete;
    NumaReplicatedBase(NumaReplicatedBase&& other) noexcept;

    NumaReplicatedBase& operator=(const NumaReplicatedBase&) = delete;
    NumaReplicatedBase& operator=(NumaReplicatedBase&& other) noexcept;

    virtual void on_numa_config_changed() = 0;
    virtual ~NumaReplicatedBase();

    const NumaConfig& get_numa_config() const;

   private:
    NumaReplicationContext* context;
};

// We force boxing with a unique_ptr. If this becomes an issue due to added
// indirection we may need to add an option for a custom boxing type. When the
// NUMA config changes the value stored at the index 0 is replicated to other nodes.
template<typename T>
class NumaReplicated: public NumaReplicatedBase {
   public:
    using ReplicatorFuncType = std::function<T(const T&)>;

    NumaReplicated(NumaReplicationContext& ctx) :
        NumaReplicatedBase(ctx) {
        replicate_from(T{});
    }

    NumaReplicated(NumaReplicationContext& ctx, T&& source) :
        NumaReplicatedBase(ctx) {
        replicate_from(std::move(source));
    }

    NumaReplicated(const NumaReplicated&) = delete;
    NumaReplicated(NumaReplicated&& other) noexcept :
        NumaReplicatedBase(std::move(other)),
        instances(std::exchange(other.instances, {})) {}

    NumaReplicated& operator=(const NumaReplicated&) = delete;
    NumaReplicated& operator=(NumaReplicated&& other) noexcept {
        NumaReplicatedBase::operator=(*this, std::move(other));
        instances = std::exchange(other.instances, {});

        return *this;
    }

    NumaReplicated& operator=(T&& source) {
        replicate_from(std::move(source));

        return *this;
    }

    ~NumaReplicated() override = default;

    const T& operator[](NumaReplicatedAccessToken token) const {
        assert(token.get_numa_index() < instances.size());
        return *(instances[token.get_numa_index()]);
    }

    const T& operator*() const { return *(instances[0]); }

    const T* operator->() const { return instances[0].get(); }

    template<typename FuncT>
    void modify_and_replicate(FuncT&& f) {
        auto source = std::move(instances[0]);
        std::forward<FuncT>(f)(*source);
        replicate_from(std::move(*source));
    }

    void on_numa_config_changed() override {
        // Use the first one as the source. It doesn't matter which one we use,
        // because they all must be identical, but the first one is guaranteed to exist.
        auto source = std::move(instances[0]);
        replicate_from(std::move(*source));
    }

   private:
    std::vector<std::unique_ptr<T>> instances;

    void replicate_from(T&& source) {
        instances.clear();

        const NumaConfig& cfg = get_numa_config();
        if (cfg.requires_memory_replication())
        {
            for (NumaIndex n = 0; n < cfg.num_numa_nodes(); ++n)
            {
                cfg.execute_on_numa_node(
                  n, [this, &source]() { instances.emplace_back(std::make_unique<T>(source)); });
            }
        }
        else
        {
            assert(cfg.num_numa_nodes() == 1);
            // We take advantage of the fact that replication is not required
            // and reuse the source value, avoiding one copy operation.
            instances.emplace_back(std::make_unique<T>(std::move(source)));
        }
    }
};

// We force boxing with a unique_ptr. If this becomes an issue due to added
// indirection we may need to add an option for a custom boxing type.
template<typename T>
class LazyNumaReplicated: public NumaReplicatedBase {
   public:
    using ReplicatorFuncType = std::function<T(const T&)>;

    LazyNumaReplicated(NumaReplicationContext& ctx) :
        NumaReplicatedBase(ctx) {
        prepare_replicate_from(T{});
    }

    LazyNumaReplicated(NumaReplicationContext& ctx, T&& source) :
        NumaReplicatedBase(ctx) {
        prepare_replicate_from(std::move(source));
    }

    LazyNumaReplicated(const LazyNumaReplicated&) = delete;
    LazyNumaReplicated(LazyNumaReplicated&& other) noexcept :
        NumaReplicatedBase(std::move(other)),
        instances(std::exchange(other.instances, {})) {}

    LazyNumaReplicated& operator=(const LazyNumaReplicated&) = delete;
    LazyNumaReplicated& operator=(LazyNumaReplicated&& other) noexcept {
        NumaReplicatedBase::operator=(*this, std::move(other));
        instances = std::exchange(other.instances, {});

        return *this;
    }

    LazyNumaReplicated& operator=(T&& source) {
        prepare_replicate_from(std::move(source));

        return *this;
    }

    ~LazyNumaReplicated() override = default;

    const T& operator[](NumaReplicatedAccessToken token) const {
        assert(token.get_numa_index() < instances.size());
        ensure_present(token.get_numa_index());
        return *(instances[token.get_numa_index()]);
    }

    const T& operator*() const { return *(instances[0]); }

    const T* operator->() const { return instances[0].get(); }

    template<typename FuncT>
    void modify_and_replicate(FuncT&& f) {
        auto source = std::move(instances[0]);
        std::forward<FuncT>(f)(*source);
        prepare_replicate_from(std::move(*source));
    }

    void on_numa_config_changed() override {
        // Use the first one as the source. It doesn't matter which one we use,
        // because they all must be identical, but the first one is guaranteed to exist.
        auto source = std::move(instances[0]);
        prepare_replicate_from(std::move(*source));
    }

   private:
    mutable std::vector<std::unique_ptr<T>> instances;
    mutable std::mutex                      mutex;

    void ensure_present(NumaIndex idx) const {
        assert(idx < instances.size());

        if (instances[idx] != nullptr)
            return;

        assert(idx != 0);

        std::unique_lock<std::mutex> lock(mutex);
        // Check again for races.
        if (instances[idx] != nullptr)
            return;

        const NumaConfig& cfg = get_numa_config();
        cfg.execute_on_numa_node(
          idx, [this, idx]() { instances[idx] = std::make_unique<T>(*instances[0]); });
    }

    void prepare_replicate_from(T&& source) {
        instances.clear();

        const NumaConfig& cfg = get_numa_config();
        if (cfg.requires_memory_replication())
        {
            assert(cfg.num_numa_nodes() > 0);

            // We just need to make sure the first instance is there.
            // Note that we cannot move here as we need to reallocate the data
            // on the correct NUMA node.
            cfg.execute_on_numa_node(
              0, [this, &source]() { instances.emplace_back(std::make_unique<T>(source)); });

            // Prepare others for lazy init.
            instances.resize(cfg.num_numa_nodes());
        }
        else
        {
            assert(cfg.num_numa_nodes() == 1);
            // We take advantage of the fact that replication is not required
            // and reuse the source value, avoiding one copy operation.
            instances.emplace_back(std::make_unique<T>(std::move(source)));
        }
    }
};

// Utilizes shared memory.
template<typename T>
class LazyNumaReplicatedSystemWide: public NumaReplicatedBase {
   public:
    using ReplicatorFuncType = std::function<T(const T&)>;

    LazyNumaReplicatedSystemWide(NumaReplicationContext& ctx) :
        NumaReplicatedBase(ctx) {
        prepare_replicate_from(std::make_unique<T>());
    }

    LazyNumaReplicatedSystemWide(NumaReplicationContext& ctx, std::unique_ptr<T>&& source) :
        NumaReplicatedBase(ctx) {
        prepare_replicate_from(std::move(source));
    }

    LazyNumaReplicatedSystemWide(const LazyNumaReplicatedSystemWide&) = delete;
    LazyNumaReplicatedSystemWide(LazyNumaReplicatedSystemWide&& other) noexcept :
        NumaReplicatedBase(std::move(other)),
        instances(std::exchange(other.instances, {})) {}

    LazyNumaReplicatedSystemWide& operator=(const LazyNumaReplicatedSystemWide&) = delete;
    LazyNumaReplicatedSystemWide& operator=(LazyNumaReplicatedSystemWide&& other) noexcept {
        NumaReplicatedBase::operator=(*this, std::move(other));
        instances = std::exchange(other.instances, {});

        return *this;
    }

    LazyNumaReplicatedSystemWide& operator=(std::unique_ptr<T>&& source) {
        prepare_replicate_from(std::move(source));

        return *this;
    }

    ~LazyNumaReplicatedSystemWide() override = default;

    const T& operator[](NumaReplicatedAccessToken token) const {
        assert(token.get_numa_index() < instances.size());
        ensure_present(token.get_numa_index());
        return *(instances[token.get_numa_index()]);
    }

    const T& operator*() const { return *(instances[0]); }

    const T* operator->() const { return &*instances[0]; }

    std::vector<std::pair<SystemWideSharedConstantAllocationStatus, std::optional<std::string>>>
    get_status_and_errors() const {
        std::vector<std::pair<SystemWideSharedConstantAllocationStatus, std::optional<std::string>>>
          status;
        status.reserve(instances.size());

        for (const auto& instance : instances)
        {
            status.emplace_back(instance.get_status(), instance.get_error_message());
        }

        return status;
    }

    template<typename FuncT>
    void modify_and_replicate(FuncT&& f) {
        auto source = std::make_unique<T>(*instances[0]);
        std::forward<FuncT>(f)(*source);
        prepare_replicate_from(std::move(source));
    }

    void on_numa_config_changed() override {
        // Use the first one as the source. It doesn't matter which one we use,
        // because they all must be identical, but the first one is guaranteed to exist.
        auto source = std::make_unique<T>(*instances[0]);
        prepare_replicate_from(std::move(source));
    }

   private:
    mutable std::vector<SystemWideSharedConstant<T>> instances;
    mutable std::mutex                               mutex;

    std::size_t get_discriminator(NumaIndex idx) const {
        const NumaConfig& cfg     = get_numa_config();
        const NumaConfig& cfg_sys = NumaConfig::from_system(SystemNumaPolicy{}, false);
        // as a discriminator, locate the hardware/system numadomain this cpuindex belongs to
        CpuIndex    cpu     = *cfg.nodes[idx].begin();  // get a CpuIndex from NumaIndex
        NumaIndex   sys_idx = cfg_sys.is_cpu_assigned(cpu) ? cfg_sys.nodeByCpu.at(cpu) : 0;
        std::string s       = cfg_sys.to_string() + "$" + std::to_string(sys_idx);
        return static_cast<std::size_t>(hash_string(s));
    }

    void ensure_present(NumaIndex idx) const {
        assert(idx < instances.size());

        if (instances[idx] != nullptr)
            return;

        assert(idx != 0);

        std::unique_lock<std::mutex> lock(mutex);
        // Check again for races.
        if (instances[idx] != nullptr)
            return;

        const NumaConfig& cfg = get_numa_config();
        cfg.execute_on_numa_node(idx, [this, idx]() {
            instances[idx] = SystemWideSharedConstant<T>(*instances[0], get_discriminator(idx));
        });
    }

    void prepare_replicate_from(std::unique_ptr<T>&& source) {
        instances.clear();

        const NumaConfig& cfg = get_numa_config();
        // We just need to make sure the first instance is there.
        // Note that we cannot move here as we need to reallocate the data
        // on the correct NUMA node.
        // Even in the case of a single NUMA node we have to copy since it's shared memory.
        if (cfg.requires_memory_replication())
        {
            assert(cfg.num_numa_nodes() > 0);

            cfg.execute_on_numa_node(0, [this, &source]() {
                instances.emplace_back(SystemWideSharedConstant<T>(*source, get_discriminator(0)));
            });

            // Prepare others for lazy init.
            instances.resize(cfg.num_numa_nodes());
        }
        else
        {
            assert(cfg.num_numa_nodes() == 1);
            instances.emplace_back(SystemWideSharedConstant<T>(*source, get_discriminator(0)));
        }
    }
};

class NumaReplicationContext {
   public:
    NumaReplicationContext(NumaConfig&& cfg) :
        config(std::move(cfg)) {}

    NumaReplicationContext(const NumaReplicationContext&) = delete;
    NumaReplicationContext(NumaReplicationContext&&)      = delete;

    NumaReplicationContext& operator=(const NumaReplicationContext&) = delete;
    NumaReplicationContext& operator=(NumaReplicationContext&&)      = delete;

    ~NumaReplicationContext() {
        // The context must outlive replicated objects
        if (!trackedReplicatedObjects.empty())
            std::exit(EXIT_FAILURE);
    }

    void attach(NumaReplicatedBase* obj) {
        assert(trackedReplicatedObjects.count(obj) == 0);
        trackedReplicatedObjects.insert(obj);
    }

    void detach(NumaReplicatedBase* obj) {
        assert(trackedReplicatedObjects.count(obj) == 1);
        trackedReplicatedObjects.erase(obj);
    }

    // oldObj may be invalid at this point
    void move_attached([[maybe_unused]] NumaReplicatedBase* oldObj, NumaReplicatedBase* newObj) {
        assert(trackedReplicatedObjects.count(oldObj) == 1);
        assert(trackedReplicatedObjects.count(newObj) == 0);
        trackedReplicatedObjects.erase(oldObj);
        trackedReplicatedObjects.insert(newObj);
    }

    void set_numa_config(NumaConfig&& cfg) {
        config = std::move(cfg);
        for (auto&& obj : trackedReplicatedObjects)
            obj->on_numa_config_changed();
    }

    const NumaConfig& get_numa_config() const { return config; }

   private:
    NumaConfig config;

    // std::set uses std::less by default, which is required for pointer comparison
    std::set<NumaReplicatedBase*> trackedReplicatedObjects;
};

inline NumaReplicatedBase::NumaReplicatedBase(NumaReplicationContext& ctx) :
    context(&ctx) {
    context->attach(this);
}

inline NumaReplicatedBase::NumaReplicatedBase(NumaReplicatedBase&& other) noexcept :
    context(std::exchange(other.context, nullptr)) {
    context->move_attached(&other, this);
}

inline NumaReplicatedBase& NumaReplicatedBase::operator=(NumaReplicatedBase&& other) noexcept {
    context = std::exchange(other.context, nullptr);

    context->move_attached(&other, this);

    return *this;
}

inline NumaReplicatedBase::~NumaReplicatedBase() {
    if (context != nullptr)
        context->detach(this);
}

inline const NumaConfig& NumaReplicatedBase::get_numa_config() const {
    return context->get_numa_config();
}

}  // namespace Alexander


#endif  // #ifndef NUMA_H_INCLUDED


// ============================================================================
// FILE ORIGINALE: pawns.cpp
// ============================================================================

/*
  Alexander, a UCI chess playing engine derived from Stockfish
  Copyright (C) 2004-2023 The Stockfish developers (see AUTHORS file)

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

#include <algorithm>
#include <cassert>

#include "bitboard.h"
#include "pawns.h"
#include "position.h"
#include "thread.h"

namespace Alexander {

namespace {

#define V Value
#define S(mg, eg) make_score(mg, eg)

// Pawn penalties
constexpr ScoreForClassical Backward      = S(6, 19);
constexpr ScoreForClassical Doubled       = S(11, 51);
constexpr ScoreForClassical DoubledEarly  = S(17, 7);
constexpr ScoreForClassical Isolated      = S(1, 20);
constexpr ScoreForClassical WeakLever     = S(2, 57);
constexpr ScoreForClassical WeakUnopposed = S(15, 18);

// Bonus for blocked pawns at 5th or 6th rank
constexpr ScoreForClassical BlockedPawn[2] = {S(-19, -8), S(-7, 3)};

constexpr ScoreForClassical BlockedStorm[RANK_NB] = {S(0, 0),    S(0, 0),  S(64, 75), S(-3, 14),
                                                     S(-12, 19), S(-7, 4), S(-10, 5)};

// Connected pawn bonus
constexpr int Connected[RANK_NB] = {0, 3, 7, 7, 15, 54, 86};

// Strength of pawn shelter for our king by [distance from edge][rank].
// RANK_1 = 0 is used for files where we have no pawn, or pawn is behind our king.
constexpr Value ShelterStrength[int(FILE_NB) / 2][RANK_NB] = {
  {V(-2), V(85), V(95), V(53), V(39), V(23), V(25)},
  {V(-55), V(64), V(32), V(-55), V(-30), V(-11), V(-61)},
  {V(-11), V(75), V(19), V(-6), V(26), V(9), V(-47)},
  {V(-41), V(-11), V(-27), V(-58), V(-42), V(-66), V(-163)}};

// Danger of enemy pawns moving toward our king by [distance from edge][rank].
// RANK_1 = 0 is used for files where the enemy has no pawn, or their pawn
// is behind our king. Note that UnblockedStorm[0][1-2] accommodate opponent pawn
// on edge, likely blocked by our king.
constexpr Value UnblockedStorm[int(FILE_NB) / 2][RANK_NB] = {
  {V(94), V(-280), V(-170), V(90), V(59), V(47), V(53)},
  {V(43), V(-17), V(128), V(39), V(26), V(-17), V(15)},
  {V(-9), V(62), V(170), V(34), V(-5), V(-20), V(-11)},
  {V(-27), V(-19), V(106), V(10), V(2), V(-13), V(-24)}};


// KingOnFile[semi-open Us][semi-open Them] contains bonuses/penalties
// for king when the king is on a semi-open or open file.
constexpr ScoreForClassical KingOnFile[2][2] = {{S(-18, 11), S(-6, -3)}, {S(0, 0), S(5, -4)}};

#undef S
#undef V


/// evaluate() calculates a score for the static pawn structure of the given position.
/// We cannot use the location of pieces or king in this function, as the evaluation
/// of the pawn structure will be stored in a small cache for speed reasons, and will
/// be re-used even when the pieces have moved.

template<Color Us>
ScoreForClassical evaluate(const Position& pos, Pawns::Entry* e) {

    constexpr Color     Them = ~Us;
    constexpr Direction Up   = pawn_push(Us);
    constexpr Direction Down = -Up;

    Bitboard          neighbours, stoppers, support, phalanx, opposed;
    Bitboard          lever, leverPush, blocked;
    Square            s;
    bool              backward, passed, doubled;
    ScoreForClassical score = SCORE_ZERO;
    Bitboard          b     = pos.pieces(Us, PAWN);

    Bitboard ourPawns   = pos.pieces(Us, PAWN);
    Bitboard theirPawns = pos.pieces(Them, PAWN);

    Bitboard doubleAttackThem = pawn_double_attacks_bb<Them>(theirPawns);

    e->passedPawns[Us] = 0;
    e->kingSquares[Us] = SQ_NONE;
    e->pawnAttacks[Us] = e->pawnAttacksSpan[Us] = pawn_attacks_bb<Us>(ourPawns);
    e->blockedCount += popcount(shift<Up>(ourPawns) & (theirPawns | doubleAttackThem));

    // Loop through all pawns of the current color and score each pawn
    while (b)
    {
        s = pop_lsb(b);

        assert(pos.piece_on(s) == make_piece(Us, PAWN));

        Rank r = relative_rank(Us, s);

        // Flag the pawn
        opposed    = theirPawns & forward_file_bb(Us, s);
        blocked    = theirPawns & (s + Up);
        stoppers   = theirPawns & passed_pawn_span(Us, s);
        lever      = theirPawns & attacks_bb<PAWN>(s, Us);
        leverPush  = theirPawns & attacks_bb<PAWN>(s + Up, Us);
        doubled    = ourPawns & (s - Up);
        neighbours = ourPawns & adjacent_files_bb(s);
        phalanx    = neighbours & rank_bb(s);
        support    = neighbours & rank_bb(s - Up);

        if (doubled)
        {
            // Additional doubled penalty if none of their pawns is fixed
            if (!(ourPawns & shift<Down>(theirPawns | pawn_attacks_bb<Them>(theirPawns))))
                score -= DoubledEarly;
        }

        // A pawn is backward when it is behind all pawns of the same color on
        // the adjacent files and cannot safely advance.
        backward = !(neighbours & forward_ranks_bb(Them, s + Up)) && (leverPush | blocked);

        // Compute additional span if pawn is not backward nor blocked
        if (!backward && !blocked)
            e->pawnAttacksSpan[Us] |= pawn_attack_span(Us, s);

        // A pawn is passed if one of the three following conditions is true:
        // (a) there is no stoppers except some levers
        // (b) the only stoppers are the leverPush, but we outnumber them
        // (c) there is only one front stopper which can be levered.
        //     (Refined in Evaluation::passed)
        passed = !(stoppers ^ lever)
              || (!(stoppers ^ leverPush) && popcount(phalanx) >= popcount(leverPush))
              || (stoppers == blocked && r >= RANK_5
                  && (shift<Up>(support) & ~(theirPawns | doubleAttackThem)));

        passed &= !(forward_file_bb(Us, s) & ourPawns);

        // Passed pawns will be properly scored later in evaluation when we have
        // full attack info.
        if (passed)
            e->passedPawns[Us] |= s;

        // ScoreForClassical this pawn
        if (support | phalanx)
        {
            int v = Connected[r] * (2 + bool(phalanx) - bool(opposed)) + 22 * popcount(support);

            score += make_score(v, v * (r - 2) / 4);
        }

        else if (!neighbours)
        {
            if (opposed && (ourPawns & forward_file_bb(Them, s))
                && !(theirPawns & adjacent_files_bb(s)))
                score -= Doubled;
            else
                score -= Isolated + WeakUnopposed * !opposed;
        }

        else if (backward)
            score -= Backward + WeakUnopposed * !opposed * bool(~(FileABB | FileHBB) & s);

        if (!support)
            score -= Doubled * doubled + WeakLever * more_than_one(lever);

        if (blocked && r >= RANK_5)
            score += BlockedPawn[r - RANK_5];
    }

    return score;
}

}  // namespace

namespace Pawns {


/// Pawns::probe() looks up the current position's pawns configuration in
/// the pawns hash table. It returns a pointer to the Entry if the position
/// is found. Otherwise a new Entry is computed and stored there, so we don't
/// have to recompute all when the same pawns configuration occurs again.

Entry* probe(const Position& pos) {

    Key    key = pos.pawn_key();
    Entry* e   = pos.this_thread()->pawnsTable[key];

    if (e->key == key)
        return e;

    e->key           = key;
    e->blockedCount  = 0;
    e->scores[WHITE] = evaluate<WHITE>(pos, e);
    e->scores[BLACK] = evaluate<BLACK>(pos, e);

    return e;
}


/// Entry::evaluate_shelter() calculates the shelter bonus and the storm
/// penalty for a king, looking at the king file and the two closest files.

template<Color Us>
ScoreForClassical Entry::evaluate_shelter(const Position& pos, Square ksq) const {

    constexpr Color Them = ~Us;

    Bitboard b          = pos.pieces(PAWN) & ~forward_ranks_bb(Them, ksq);
    Bitboard ourPawns   = b & pos.pieces(Us) & ~pawnAttacks[Them];
    Bitboard theirPawns = b & pos.pieces(Them);

    ScoreForClassical bonus = make_score(5, 5);

    File center = std::clamp(file_of(ksq), FILE_B, FILE_G);
    for (File f = File(center - 1); f <= File(center + 1); ++f)
    {
        b           = ourPawns & file_bb(f);
        int ourRank = b ? relative_rank(Us, frontmost_sq(Them, b)) : 0;

        b             = theirPawns & file_bb(f);
        int theirRank = b ? relative_rank(Us, frontmost_sq(Them, b)) : 0;

        int d = edge_distance(f);
        bonus += make_score(ShelterStrength[d][ourRank], 0);

        if (ourRank && (ourRank == theirRank - 1))
            bonus -= BlockedStorm[theirRank];
        else
            bonus -= make_score(UnblockedStorm[d][theirRank], 0);
    }

    // King On File
    bonus -= KingOnFile[pos.is_on_semiopen_file(Us, ksq)][pos.is_on_semiopen_file(Them, ksq)];

    return bonus;
}


/// Entry::do_king_safety() calculates a bonus for king safety. It is called only
/// when king square changes, which is about 20% of total king_safety() calls.

template<Color Us>
ScoreForClassical Entry::do_king_safety(const Position& pos) {

    Square ksq         = pos.square<KING>(Us);
    kingSquares[Us]    = ksq;
    castlingRights[Us] = pos.castling_rights(Us);
    auto compare       = [](ScoreForClassical a, ScoreForClassical b) {
        return mg_value(a) < mg_value(b);
    };

    ScoreForClassical shelter = evaluate_shelter<Us>(pos, ksq);

    // If we can castle use the bonus after castling if it is bigger

    if (pos.can_castle(Us & KING_SIDE))
        shelter = std::max(shelter, evaluate_shelter<Us>(pos, relative_square(Us, SQ_G1)), compare);

    if (pos.can_castle(Us & QUEEN_SIDE))
        shelter = std::max(shelter, evaluate_shelter<Us>(pos, relative_square(Us, SQ_C1)), compare);

    // In endgame we like to bring our king near our closest pawn
    Bitboard pawns       = pos.pieces(Us, PAWN);
    int      minPawnDist = 6;

    if (pawns & attacks_bb<KING>(ksq))
        minPawnDist = 1;
    else
        while (pawns)
            minPawnDist = std::min(minPawnDist, distance(ksq, pop_lsb(pawns)));

    return shelter - make_score(0, 16 * minPawnDist);
}

// Explicit template instantiation
template ScoreForClassical Entry::do_king_safety<WHITE>(const Position& pos);
template ScoreForClassical Entry::do_king_safety<BLACK>(const Position& pos);

}  // namespace Pawns

}  // namespace Alexander


// ============================================================================
// FILE ORIGINALE: pawns.h
// ============================================================================

/*
  Alexander, a UCI chess playing engine derived from Stockfish
  Copyright (C) 2004-2023 The Stockfish developers (see AUTHORS file)

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

#ifndef PAWNS_H_INCLUDED
#define PAWNS_H_INCLUDED

#include "misc.h"
#include "position.h"
#include "types.h"

namespace Alexander::Pawns {

/// Pawns::Entry contains various information about a pawn structure. A lookup
/// to the pawn hash table (performed by calling the probe function) returns a
/// pointer to an Entry object.

struct Entry {

    ScoreForClassical pawn_score(Color c) const { return scores[c]; }
    Bitboard          pawn_attacks(Color c) const { return pawnAttacks[c]; }
    Bitboard          passed_pawns(Color c) const { return passedPawns[c]; }
    Bitboard          pawn_attacks_span(Color c) const { return pawnAttacksSpan[c]; }
    int passed_count() const { return popcount(passedPawns[WHITE] | passedPawns[BLACK]); }
    int blocked_count() const { return blockedCount; }

    template<Color Us>
    ScoreForClassical king_safety(const Position& pos) {
        return kingSquares[Us] == pos.square<KING>(Us)
                && castlingRights[Us] == pos.castling_rights(Us)
               ? kingSafety[Us]
               : (kingSafety[Us] = do_king_safety<Us>(pos));
    }

    template<Color Us>
    ScoreForClassical do_king_safety(const Position& pos);

    template<Color Us>
    ScoreForClassical evaluate_shelter(const Position& pos, Square ksq) const;

    Key               key;
    ScoreForClassical scores[COLOR_NB];
    Bitboard          passedPawns[COLOR_NB];
    Bitboard          pawnAttacks[COLOR_NB];
    Bitboard          pawnAttacksSpan[COLOR_NB];
    Square            kingSquares[COLOR_NB];
    ScoreForClassical kingSafety[COLOR_NB];
    int               castlingRights[COLOR_NB];
    int               blockedCount;
};

using Table = HashTable<Entry, 131072>;

Entry* probe(const Position& pos);

}  // namespace Alexander::Pawns

#endif  // #ifndef PAWNS_H_INCLUDED


// ============================================================================
// FILE ORIGINALE: perft.h
// ============================================================================

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

#ifndef PERFT_H_INCLUDED
#define PERFT_H_INCLUDED

#include <cstdint>

#include "movegen.h"
#include "position.h"
#include "types.h"
#include "uci.h"

namespace Alexander::Benchmark {

// Utility to verify move generation. All the leaf nodes up
// to the given depth are generated and counted, and the sum is returned.
template<bool Root>
uint64_t perft(Position& pos, Depth depth) {

    StateInfo st;

    uint64_t   cnt, nodes = 0;
    const bool leaf = (depth == 2);

    for (const auto& m : MoveList<LEGAL>(pos))
    {
        if (Root && depth <= 1)
            cnt = 1, nodes++;
        else
        {
            pos.do_move(m, st);
            cnt = leaf ? MoveList<LEGAL>(pos).size() : perft<false>(pos, depth - 1);
            nodes += cnt;
            pos.undo_move(m);
        }
        if (Root)
            sync_cout << UCIEngine::move(m, pos.is_chess960()) << ": " << cnt << sync_endl;
    }
    return nodes;
}

inline uint64_t perft(const std::string& fen, Depth depth, bool isChess960, Thread* th) {
    StateInfo st;
    Position  p;
    p.set(fen, isChess960, &st, th);

    return perft<true>(p, depth);
}
}

#endif  // PERFT_H_INCLUDED


// ============================================================================
// FILE ORIGINALE: position.cpp
// ============================================================================

/*
  Alexander, a UCI chess playing engine derived from Stockfish
  Copyright (C) 2004-2026 Andrea Manzo, F. Ferraguti, K.Kiniama and Alexander developers (see AUTHORS file)

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

#include "position.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <cctype>
#include <cstddef>
#include <cstring>
#include <initializer_list>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string_view>
#include <utility>

#include "bitboard.h"
#include "history.h"
#include "misc.h"
//shashin
#include "syzygy/tbprobe.h"
#include "tt.h"
#include "uci.h"

using std::string;

namespace Alexander {

namespace Zobrist {

Key psq[PIECE_NB][SQUARE_NB];
Key enpassant[FILE_NB];
Key castling[CASTLING_RIGHT_NB];
Key side, noPawns;
}

namespace {

constexpr std::string_view PieceToChar(" PNBRQK  pnbrqk");

static constexpr Piece Pieces[] = {W_PAWN, W_KNIGHT, W_BISHOP, W_ROOK, W_QUEEN, W_KING,
                                   B_PAWN, B_KNIGHT, B_BISHOP, B_ROOK, B_QUEEN, B_KING};
}  // namespace


// Returns an ASCII representation of the position
std::ostream& operator<<(std::ostream& os, const Position& pos) {

    os << "\n +---+---+---+---+---+---+---+---+\n";

    for (Rank r = RANK_8;; --r)
    {
        for (File f = FILE_A; f <= FILE_H; ++f)
            os << " | " << PieceToChar[pos.piece_on(make_square(f, r))];

        os << " | " << (1 + r) << "\n +---+---+---+---+---+---+---+---+\n";

        if (r == RANK_1)
            break;
    }

    os << "   a   b   c   d   e   f   g   h\n"
       << "\nFen: " << pos.fen() << "\nKey: " << std::hex << std::uppercase << std::setfill('0')
       << std::setw(16) << pos.key() << std::setfill(' ') << std::dec << "\nCheckers: ";

    for (Bitboard b = pos.checkers(); b;)
        os << UCIEngine::square(pop_lsb(b)) << " ";

    if (Tablebases::MaxCardinality >= popcount(pos.pieces()) && !pos.can_castle(ANY_CASTLING))
    {
        StateInfo st;

        Position p;
        p.set(pos.fen(), pos.is_chess960(), &st, pos.this_thread());  //for classical
        Tablebases::ProbeState s1, s2;
        Tablebases::WDLScore   wdl = Tablebases::probe_wdl(p, &s1);
        int                    dtz = Tablebases::probe_dtz(p, &s2);
        os << "\nTablebases WDL: " << std::setw(4) << wdl << " (" << s1 << ")"
           << "\nTablebases DTZ: " << std::setw(4) << dtz << " (" << s2 << ")";
    }

    return os;
}


// Implements Marcel van Kervinck's cuckoo algorithm to detect repetition of positions
// for 3-fold repetition draws. The algorithm uses two hash tables with Zobrist hashes
// to allow fast detection of recurring positions. For details see:
// http://web.archive.org/web/20201107002606/https://marcelk.net/2013-04-06/paper/upcoming-rep-v2.pdf

// First and second hash functions for indexing the cuckoo tables
inline int H1(Key h) { return h & 0x1fff; }
inline int H2(Key h) { return (h >> 16) & 0x1fff; }

// Cuckoo tables with Zobrist hashes of valid reversible moves, and the moves themselves
std::array<Key, 8192>  cuckoo;
std::array<Move, 8192> cuckooMove;

// Initializes at startup the various arrays used to compute hash keys
void Position::init() {
    //learning begin
    Zobrist::psq[1][0]   = 591679071752537765U;
    Zobrist::psq[1][1]   = 11781298203991720739U;
    Zobrist::psq[1][2]   = 17774509420834274491U;
    Zobrist::psq[1][3]   = 93833316982319649U;
    Zobrist::psq[1][4]   = 5077288827755375591U;
    Zobrist::psq[1][5]   = 12650468822090308278U;
    Zobrist::psq[1][6]   = 7282142511083249914U;
    Zobrist::psq[1][7]   = 10536503665313592279U;
    Zobrist::psq[1][8]   = 4539792784031873725U;
    Zobrist::psq[1][9]   = 2841870292508388689U;
    Zobrist::psq[1][10]  = 15413206348252250872U;
    Zobrist::psq[1][11]  = 7678569077154129441U;
    Zobrist::psq[1][12]  = 13346546310876667408U;
    Zobrist::psq[1][13]  = 18288271767696598454U;
    Zobrist::psq[1][14]  = 10369369943721775254U;
    Zobrist::psq[1][15]  = 18081987910875800766U;
    Zobrist::psq[1][16]  = 5538285989180528017U;
    Zobrist::psq[1][17]  = 1561342000895978098U;
    Zobrist::psq[1][18]  = 344529452680813775U;
    Zobrist::psq[1][19]  = 12666574946949763448U;
    Zobrist::psq[1][20]  = 11485456468243178719U;
    Zobrist::psq[1][21]  = 7930595158480463155U;
    Zobrist::psq[1][22]  = 14302725423041560508U;
    Zobrist::psq[1][23]  = 14331261293281981139U;
    Zobrist::psq[1][24]  = 4456874005134181239U;
    Zobrist::psq[1][25]  = 2824504039224593559U;
    Zobrist::psq[1][26]  = 10380971965294849792U;
    Zobrist::psq[1][27]  = 15120440200421969569U;
    Zobrist::psq[1][28]  = 2459658218254782268U;
    Zobrist::psq[1][29]  = 3478717432759217624U;
    Zobrist::psq[1][30]  = 3378985187684316967U;
    Zobrist::psq[1][31]  = 9696037458963191704U;
    Zobrist::psq[1][32]  = 13098241107727776933U;
    Zobrist::psq[1][33]  = 16711523013166202616U;
    Zobrist::psq[1][34]  = 10079083771611825891U;
    Zobrist::psq[1][35]  = 14137347994420603547U;
    Zobrist::psq[1][36]  = 4791805899784156187U;
    Zobrist::psq[1][37]  = 6078389034317276724U;
    Zobrist::psq[1][38]  = 5994547221653596060U;
    Zobrist::psq[1][39]  = 16213379374441749196U;
    Zobrist::psq[1][40]  = 4600174966381648954U;
    Zobrist::psq[1][41]  = 2382793282151591793U;
    Zobrist::psq[1][42]  = 5441064086789571698U;
    Zobrist::psq[1][43]  = 13211067155709920737U;
    Zobrist::psq[1][44]  = 8095577678192451481U;
    Zobrist::psq[1][45]  = 12870220845239618167U;
    Zobrist::psq[1][46]  = 18366225606586112739U;
    Zobrist::psq[1][47]  = 1482740430229529117U;
    Zobrist::psq[1][48]  = 18398763828894394702U;
    Zobrist::psq[1][49]  = 12894175299039183743U;
    Zobrist::psq[1][50]  = 5973205243991449651U;
    Zobrist::psq[1][51]  = 16073805277627490771U;
    Zobrist::psq[1][52]  = 11840382123049768615U;
    Zobrist::psq[1][53]  = 16782637305176790952U;
    Zobrist::psq[1][54]  = 16565939816889406374U;
    Zobrist::psq[1][55]  = 7611013259146743987U;
    Zobrist::psq[1][56]  = 4325631834421711187U;
    Zobrist::psq[1][57]  = 7084652077183601842U;
    Zobrist::psq[1][58]  = 14113904950837697704U;
    Zobrist::psq[1][59]  = 6952439085241219742U;
    Zobrist::psq[1][60]  = 11697893679396085013U;
    Zobrist::psq[1][61]  = 15932411745698688381U;
    Zobrist::psq[1][62]  = 333938476871428781U;
    Zobrist::psq[1][63]  = 10094356940478519713U;
    Zobrist::psq[2][0]   = 8854028305631117351U;
    Zobrist::psq[2][1]   = 18264149368209609558U;
    Zobrist::psq[2][2]   = 18152850504025660547U;
    Zobrist::psq[2][3]   = 445125824226036916U;
    Zobrist::psq[2][4]   = 7445032221575161576U;
    Zobrist::psq[2][5]   = 5887372625995221418U;
    Zobrist::psq[2][6]   = 12579614965563241976U;
    Zobrist::psq[2][7]   = 15542129933905340102U;
    Zobrist::psq[2][8]   = 4278411582816540073U;
    Zobrist::psq[2][9]   = 7817987688731403418U;
    Zobrist::psq[2][10]  = 16765308846548980593U;
    Zobrist::psq[2][11]  = 15594655397588023405U;
    Zobrist::psq[2][12]  = 11116801254932199266U;
    Zobrist::psq[2][13]  = 11592572287770353464U;
    Zobrist::psq[2][14]  = 10698558469286656858U;
    Zobrist::psq[2][15]  = 263236209937302172U;
    Zobrist::psq[2][16]  = 15461982991340303336U;
    Zobrist::psq[2][17]  = 3043744698521235658U;
    Zobrist::psq[2][18]  = 1070442759222213040U;
    Zobrist::psq[2][19]  = 650534245804607543U;
    Zobrist::psq[2][20]  = 5943000432800778858U;
    Zobrist::psq[2][21]  = 26206987068637543U;
    Zobrist::psq[2][22]  = 16737080395141468053U;
    Zobrist::psq[2][23]  = 13977415469856941557U;
    Zobrist::psq[2][24]  = 1052117838564742180U;
    Zobrist::psq[2][25]  = 9424311196719389450U;
    Zobrist::psq[2][26]  = 12167498318705983564U;
    Zobrist::psq[2][27]  = 4301764225574437137U;
    Zobrist::psq[2][28]  = 17360266336634281276U;
    Zobrist::psq[2][29]  = 13868884065264943813U;
    Zobrist::psq[2][30]  = 15952283905104982306U;
    Zobrist::psq[2][31]  = 4998386290424363477U;
    Zobrist::psq[2][32]  = 4893239286087369377U;
    Zobrist::psq[2][33]  = 17573528852960048629U;
    Zobrist::psq[2][34]  = 2412201799238683587U;
    Zobrist::psq[2][35]  = 16517545668683925387U;
    Zobrist::psq[2][36]  = 16978748896271686395U;
    Zobrist::psq[2][37]  = 8830712609912112615U;
    Zobrist::psq[2][38]  = 244676446090624528U;
    Zobrist::psq[2][39]  = 10801320743593590304U;
    Zobrist::psq[2][40]  = 13531918303924845431U;
    Zobrist::psq[2][41]  = 10527125009130628070U;
    Zobrist::psq[2][42]  = 17495106538955645767U;
    Zobrist::psq[2][43]  = 14203433425689676251U;
    Zobrist::psq[2][44]  = 13760149572603586785U;
    Zobrist::psq[2][45]  = 1273129856199637694U;
    Zobrist::psq[2][46]  = 3154213753511759364U;
    Zobrist::psq[2][47]  = 12760143787594064657U;
    Zobrist::psq[2][48]  = 1600035040276021173U;
    Zobrist::psq[2][49]  = 5414819345072334853U;
    Zobrist::psq[2][50]  = 7201040945210650872U;
    Zobrist::psq[2][51]  = 11015789609492649674U;
    Zobrist::psq[2][52]  = 7712150959425383900U;
    Zobrist::psq[2][53]  = 8543311100722720016U;
    Zobrist::psq[2][54]  = 13076185511676908731U;
    Zobrist::psq[2][55]  = 3922562784470822468U;
    Zobrist::psq[2][56]  = 2780562387024492132U;
    Zobrist::psq[2][57]  = 6697120216501611455U;
    Zobrist::psq[2][58]  = 13480343126040452106U;
    Zobrist::psq[2][59]  = 12173667680050468927U;
    Zobrist::psq[2][60]  = 3302171945877565923U;
    Zobrist::psq[2][61]  = 16568602182162993491U;
    Zobrist::psq[2][62]  = 14953223006496535120U;
    Zobrist::psq[2][63]  = 16457941142416543492U;
    Zobrist::psq[3][0]   = 2945262940327718556U;
    Zobrist::psq[3][1]   = 3775538624233802005U;
    Zobrist::psq[3][2]   = 4292201895252289600U;
    Zobrist::psq[3][3]   = 16433809973923446677U;
    Zobrist::psq[3][4]   = 1284774014851141252U;
    Zobrist::psq[3][5]   = 18314932087213148495U;
    Zobrist::psq[3][6]   = 8946796353798605717U;
    Zobrist::psq[3][7]   = 16445820069092145103U;
    Zobrist::psq[3][8]   = 7588664147775519679U;
    Zobrist::psq[3][9]   = 12896594212779880816U;
    Zobrist::psq[3][10]  = 14935880823937687725U;
    Zobrist::psq[3][11]  = 13400879436137989525U;
    Zobrist::psq[3][12]  = 13846969535995712591U;
    Zobrist::psq[3][13]  = 12484917729738156524U;
    Zobrist::psq[3][14]  = 17882592831712409952U;
    Zobrist::psq[3][15]  = 16637473249645425632U;
    Zobrist::psq[3][16]  = 15098223454147433904U;
    Zobrist::psq[3][17]  = 17631249017957605294U;
    Zobrist::psq[3][18]  = 12582001597670293135U;
    Zobrist::psq[3][19]  = 17902661106057732664U;
    Zobrist::psq[3][20]  = 10274060743048400565U;
    Zobrist::psq[3][21]  = 12005958760542442625U;
    Zobrist::psq[3][22]  = 6324932172735347303U;
    Zobrist::psq[3][23]  = 17192330553585486663U;
    Zobrist::psq[3][24]  = 9422872207407330841U;
    Zobrist::psq[3][25]  = 3177237980255163711U;
    Zobrist::psq[3][26]  = 14998024116488875998U;
    Zobrist::psq[3][27]  = 705793604453777656U;
    Zobrist::psq[3][28]  = 11327568552142987041U;
    Zobrist::psq[3][29]  = 7029368612848231507U;
    Zobrist::psq[3][30]  = 11062860980165499825U;
    Zobrist::psq[3][31]  = 2900628512702115887U;
    Zobrist::psq[3][32]  = 308431256844078091U;
    Zobrist::psq[3][33]  = 752802454931337639U;
    Zobrist::psq[3][34]  = 5576583881995601144U;
    Zobrist::psq[3][35]  = 8733594096989903760U;
    Zobrist::psq[3][36]  = 290737499942622970U;
    Zobrist::psq[3][37]  = 8992780576699235245U;
    Zobrist::psq[3][38]  = 10425616809589311900U;
    Zobrist::psq[3][39]  = 5493674620779310265U;
    Zobrist::psq[3][40]  = 12589103349525344891U;
    Zobrist::psq[3][41]  = 14857852059215963628U;
    Zobrist::psq[3][42]  = 13495551423272463104U;
    Zobrist::psq[3][43]  = 6944056268429507318U;
    Zobrist::psq[3][44]  = 3988842613368812515U;
    Zobrist::psq[3][45]  = 14815775969275954512U;
    Zobrist::psq[3][46]  = 17868612272134391879U;
    Zobrist::psq[3][47]  = 8436706119115607049U;
    Zobrist::psq[3][48]  = 7555807622404432493U;
    Zobrist::psq[3][49]  = 9144495607954586305U;
    Zobrist::psq[3][50]  = 6794801016890317083U;
    Zobrist::psq[3][51]  = 6072558259768997948U;
    Zobrist::psq[3][52]  = 10941535447546794938U;
    Zobrist::psq[3][53]  = 14043502401785556544U;
    Zobrist::psq[3][54]  = 8362621443508695308U;
    Zobrist::psq[3][55]  = 17736840905212253027U;
    Zobrist::psq[3][56]  = 2733031211210449030U;
    Zobrist::psq[3][57]  = 4350365705834634871U;
    Zobrist::psq[3][58]  = 1100550212031776323U;
    Zobrist::psq[3][59]  = 17430963890314521917U;
    Zobrist::psq[3][60]  = 7470064030368587841U;
    Zobrist::psq[3][61]  = 13387014036020469860U;
    Zobrist::psq[3][62]  = 7078824284187344392U;
    Zobrist::psq[3][63]  = 12312007608706932222U;
    Zobrist::psq[4][0]   = 3826719064958106391U;
    Zobrist::psq[4][1]   = 17580452432494632735U;
    Zobrist::psq[4][2]   = 4372818848456885156U;
    Zobrist::psq[4][3]   = 20778095608392735U;
    Zobrist::psq[4][4]   = 9517712183106565981U;
    Zobrist::psq[4][5]   = 16772576131911258204U;
    Zobrist::psq[4][6]   = 12158847832281029501U;
    Zobrist::psq[4][7]   = 18318866654963083744U;
    Zobrist::psq[4][8]   = 14355784966049388499U;
    Zobrist::psq[4][9]   = 1442237715923966096U;
    Zobrist::psq[4][10]  = 16767620159370203923U;
    Zobrist::psq[4][11]  = 13501017873225644439U;
    Zobrist::psq[4][12]  = 12414460951753850741U;
    Zobrist::psq[4][13]  = 1630390626826320339U;
    Zobrist::psq[4][14]  = 11056926288496765292U;
    Zobrist::psq[4][15]  = 17514919132679636196U;
    Zobrist::psq[4][16]  = 6737125905271376420U;
    Zobrist::psq[4][17]  = 3156370395448333753U;
    Zobrist::psq[4][18]  = 7372374977020439436U;
    Zobrist::psq[4][19]  = 5277883516136612451U;
    Zobrist::psq[4][20]  = 16544956564115640970U;
    Zobrist::psq[4][21]  = 14431129579433994133U;
    Zobrist::psq[4][22]  = 10776067565185448U;
    Zobrist::psq[4][23]  = 15235680854177679657U;
    Zobrist::psq[4][24]  = 12767627681826077225U;
    Zobrist::psq[4][25]  = 1324675096273909386U;
    Zobrist::psq[4][26]  = 3456463189867507715U;
    Zobrist::psq[4][27]  = 9195964142578403484U;
    Zobrist::psq[4][28]  = 10627443539470127577U;
    Zobrist::psq[4][29]  = 7083655917886846512U;
    Zobrist::psq[4][30]  = 14734414825071094346U;
    Zobrist::psq[4][31]  = 8833975264052769557U;
    Zobrist::psq[4][32]  = 2965232458494052289U;
    Zobrist::psq[4][33]  = 12786367183060552144U;
    Zobrist::psq[4][34]  = 6364751811635930008U;
    Zobrist::psq[4][35]  = 12304694438192434386U;
    Zobrist::psq[4][36]  = 4420057912710567321U;
    Zobrist::psq[4][37]  = 13121826629733594974U;
    Zobrist::psq[4][38]  = 3295424378969736960U;
    Zobrist::psq[4][39]  = 16543444358261923928U;
    Zobrist::psq[4][40]  = 13665696745413941685U;
    Zobrist::psq[4][41]  = 3585618043384929225U;
    Zobrist::psq[4][42]  = 14758422515963078108U;
    Zobrist::psq[4][43]  = 5444185746065710993U;
    Zobrist::psq[4][44]  = 6217807121864929894U;
    Zobrist::psq[4][45]  = 7617121805124236390U;
    Zobrist::psq[4][46]  = 2176332518208481987U;
    Zobrist::psq[4][47]  = 1435617355844826626U;
    Zobrist::psq[4][48]  = 17897291909516933347U;
    Zobrist::psq[4][49]  = 17430612766366810879U;
    Zobrist::psq[4][50]  = 13845907184570465897U;
    Zobrist::psq[4][51]  = 3432307431600566936U;
    Zobrist::psq[4][52]  = 2532253559171451888U;
    Zobrist::psq[4][53]  = 11643128737472459646U;
    Zobrist::psq[4][54]  = 13606171979107604790U;
    Zobrist::psq[4][55]  = 10012509558550373270U;
    Zobrist::psq[4][56]  = 5587706015190365982U;
    Zobrist::psq[4][57]  = 18189230678861289336U;
    Zobrist::psq[4][58]  = 5637318834313874969U;
    Zobrist::psq[4][59]  = 4728172345191419793U;
    Zobrist::psq[4][60]  = 13287099661014164329U;
    Zobrist::psq[4][61]  = 8475766932330124954U;
    Zobrist::psq[4][62]  = 2781312650135424674U;
    Zobrist::psq[4][63]  = 10552294945874175633U;
    Zobrist::psq[5][0]   = 14116194119706301666U;
    Zobrist::psq[5][1]   = 908994258594572803U;
    Zobrist::psq[5][2]   = 3835251526534030662U;
    Zobrist::psq[5][3]   = 3902806174142003247U;
    Zobrist::psq[5][4]   = 8404113168045990162U;
    Zobrist::psq[5][5]   = 10605456791970677788U;
    Zobrist::psq[5][6]   = 8371724936653327204U;
    Zobrist::psq[5][7]   = 10149265301602815302U;
    Zobrist::psq[5][8]   = 10280163375965480302U;
    Zobrist::psq[5][9]   = 12878458563073396434U;
    Zobrist::psq[5][10]  = 1480273033205949154U;
    Zobrist::psq[5][11]  = 15420639285122262859U;
    Zobrist::psq[5][12]  = 16040433549230388361U;
    Zobrist::psq[5][13]  = 10889445127567090568U;
    Zobrist::psq[5][14]  = 7154846977618541400U;
    Zobrist::psq[5][15]  = 15324267473561911299U;
    Zobrist::psq[5][16]  = 9123044315927273855U;
    Zobrist::psq[5][17]  = 18178395620988860923U;
    Zobrist::psq[5][18]  = 13937825686985326355U;
    Zobrist::psq[5][19]  = 6208640256728026680U;
    Zobrist::psq[5][20]  = 17803354189602776349U;
    Zobrist::psq[5][21]  = 8168466387959732965U;
    Zobrist::psq[5][22]  = 4747388335999020093U;
    Zobrist::psq[5][23]  = 8076893647775627477U;
    Zobrist::psq[5][24]  = 135355862477779318U;
    Zobrist::psq[5][25]  = 13727020784074293322U;
    Zobrist::psq[5][26]  = 16471001867829363208U;
    Zobrist::psq[5][27]  = 3944848361583366045U;
    Zobrist::psq[5][28]  = 6153835027004876065U;
    Zobrist::psq[5][29]  = 17541053953916494135U;
    Zobrist::psq[5][30]  = 830442639195732299U;
    Zobrist::psq[5][31]  = 5707759661195251524U;
    Zobrist::psq[5][32]  = 16745928189385382169U;
    Zobrist::psq[5][33]  = 13853872449862111272U;
    Zobrist::psq[5][34]  = 10763276423780512808U;
    Zobrist::psq[5][35]  = 528748578239178413U;
    Zobrist::psq[5][36]  = 1195366693239264477U;
    Zobrist::psq[5][37]  = 16072813688416096526U;
    Zobrist::psq[5][38]  = 9411878730995839744U;
    Zobrist::psq[5][39]  = 14250860229846220116U;
    Zobrist::psq[5][40]  = 3391112600086567492U;
    Zobrist::psq[5][41]  = 11283764167692931512U;
    Zobrist::psq[5][42]  = 1672248607577385754U;
    Zobrist::psq[5][43]  = 2130286739811077583U;
    Zobrist::psq[5][44]  = 18311727561747759139U;
    Zobrist::psq[5][45]  = 974583822133342724U;
    Zobrist::psq[5][46]  = 5061116103402273638U;
    Zobrist::psq[5][47]  = 3126855720952116346U;
    Zobrist::psq[5][48]  = 578870949780164607U;
    Zobrist::psq[5][49]  = 3776778176701636327U;
    Zobrist::psq[5][50]  = 14213795876687685078U;
    Zobrist::psq[5][51]  = 5613780124034108946U;
    Zobrist::psq[5][52]  = 6069741268072432820U;
    Zobrist::psq[5][53]  = 8893641350514130178U;
    Zobrist::psq[5][54]  = 15249957078178864452U;
    Zobrist::psq[5][55]  = 18092583129505773527U;
    Zobrist::psq[5][56]  = 11393903435307203091U;
    Zobrist::psq[5][57]  = 8119660695860781220U;
    Zobrist::psq[5][58]  = 13766130452052543028U;
    Zobrist::psq[5][59]  = 7096579372531132405U;
    Zobrist::psq[5][60]  = 7459026647266724422U;
    Zobrist::psq[5][61]  = 5897616920394564481U;
    Zobrist::psq[5][62]  = 4162427946331299898U;
    Zobrist::psq[5][63]  = 2527789185948800525U;
    Zobrist::psq[6][0]   = 17290988795360054066U;
    Zobrist::psq[6][1]   = 5240905960030703813U;
    Zobrist::psq[6][2]   = 12532957579127022568U;
    Zobrist::psq[6][3]   = 7321214839249116978U;
    Zobrist::psq[6][4]   = 17188130528816882357U;
    Zobrist::psq[6][5]   = 13649660060729335176U;
    Zobrist::psq[6][6]   = 7877670809777050873U;
    Zobrist::psq[6][7]   = 8603165736220767331U;
    Zobrist::psq[6][8]   = 3731409983944574110U;
    Zobrist::psq[6][9]   = 14311591814980160037U;
    Zobrist::psq[6][10]  = 16719365103710912831U;
    Zobrist::psq[6][11]  = 15645061390881301878U;
    Zobrist::psq[6][12]  = 15313601992567477463U;
    Zobrist::psq[6][13]  = 558437165307320475U;
    Zobrist::psq[6][14]  = 10107592147679710958U;
    Zobrist::psq[6][15]  = 217058993405149273U;
    Zobrist::psq[6][16]  = 11583857652496458642U;
    Zobrist::psq[6][17]  = 12813267508475749642U;
    Zobrist::psq[6][18]  = 12801463184548517903U;
    Zobrist::psq[6][19]  = 10205205656182355892U;
    Zobrist::psq[6][20]  = 12009517757124415757U;
    Zobrist::psq[6][21]  = 11711220569788417590U;
    Zobrist::psq[6][22]  = 601506575385147719U;
    Zobrist::psq[6][23]  = 2403800598476663693U;
    Zobrist::psq[6][24]  = 3185273191806365666U;
    Zobrist::psq[6][25]  = 16311384682203900813U;
    Zobrist::psq[6][26]  = 2147738008043402447U;
    Zobrist::psq[6][27]  = 11784653004849107439U;
    Zobrist::psq[6][28]  = 11363702615030984814U;
    Zobrist::psq[6][29]  = 4459820841160151625U;
    Zobrist::psq[6][30]  = 17238855191434604666U;
    Zobrist::psq[6][31]  = 16533107622905015899U;
    Zobrist::psq[6][32]  = 12580437090734268666U;
    Zobrist::psq[6][33]  = 9002238121826321187U;
    Zobrist::psq[6][34]  = 7209727037264965188U;
    Zobrist::psq[6][35]  = 15210303941751662984U;
    Zobrist::psq[6][36]  = 5957580827072516578U;
    Zobrist::psq[6][37]  = 16077971979351817631U;
    Zobrist::psq[6][38]  = 7451935491114626499U;
    Zobrist::psq[6][39]  = 14243752318712699139U;
    Zobrist::psq[6][40]  = 12737894796843349185U;
    Zobrist::psq[6][41]  = 1351996896321498360U;
    Zobrist::psq[6][42]  = 4395539424431256646U;
    Zobrist::psq[6][43]  = 14636926406778905296U;
    Zobrist::psq[6][44]  = 10637364485216545239U;
    Zobrist::psq[6][45]  = 4709900282812548306U;
    Zobrist::psq[6][46]  = 14703591130731831913U;
    Zobrist::psq[6][47]  = 1476367765688281237U;
    Zobrist::psq[6][48]  = 4113914727206496161U;
    Zobrist::psq[6][49]  = 8066049843497142643U;
    Zobrist::psq[6][50]  = 7809561412546830570U;
    Zobrist::psq[6][51]  = 4879538739185105394U;
    Zobrist::psq[6][52]  = 9498083046807871856U;
    Zobrist::psq[6][53]  = 17559505952950827343U;
    Zobrist::psq[6][54]  = 11763387757765891631U;
    Zobrist::psq[6][55]  = 10055035698587107604U;
    Zobrist::psq[6][56]  = 12844734664424373030U;
    Zobrist::psq[6][57]  = 330991544207939447U;
    Zobrist::psq[6][58]  = 8508732305896661743U;
    Zobrist::psq[6][59]  = 11153570973223855023U;
    Zobrist::psq[6][60]  = 10238055872248257461U;
    Zobrist::psq[6][61]  = 1773280948989896239U;
    Zobrist::psq[6][62]  = 8300833427522849187U;
    Zobrist::psq[6][63]  = 10832779467616436194U;
    Zobrist::psq[9][0]   = 11781789245711860189U;
    Zobrist::psq[9][1]   = 2747882707407274161U;
    Zobrist::psq[9][2]   = 3724767368808293169U;
    Zobrist::psq[9][3]   = 10298180063630105197U;
    Zobrist::psq[9][4]   = 10746438658164496957U;
    Zobrist::psq[9][5]   = 16037040440297371558U;
    Zobrist::psq[9][6]   = 17588125462232966688U;
    Zobrist::psq[9][7]   = 6880843334474042246U;
    Zobrist::psq[9][8]   = 560415017990002212U;
    Zobrist::psq[9][9]   = 6626394159937994533U;
    Zobrist::psq[9][10]  = 2670333323665803600U;
    Zobrist::psq[9][11]  = 4280458366389177326U;
    Zobrist::psq[9][12]  = 1467978672011198404U;
    Zobrist::psq[9][13]  = 7620133404071416883U;
    Zobrist::psq[9][14]  = 13350367343504972530U;
    Zobrist::psq[9][15]  = 10138430730509076413U;
    Zobrist::psq[9][16]  = 6785953884329063615U;
    Zobrist::psq[9][17]  = 4006903721835701728U;
    Zobrist::psq[9][18]  = 17529175408771439641U;
    Zobrist::psq[9][19]  = 2257868868401674686U;
    Zobrist::psq[9][20]  = 16350586259217027048U;
    Zobrist::psq[9][21]  = 12792669610269240489U;
    Zobrist::psq[9][22]  = 15445432911128260212U;
    Zobrist::psq[9][23]  = 3830919760132254685U;
    Zobrist::psq[9][24]  = 17463139367032047470U;
    Zobrist::psq[9][25]  = 15002266175994648649U;
    Zobrist::psq[9][26]  = 17680514289072042202U;
    Zobrist::psq[9][27]  = 362761448860517629U;
    Zobrist::psq[9][28]  = 2620716836644167551U;
    Zobrist::psq[9][29]  = 10876826577342073644U;
    Zobrist::psq[9][30]  = 14704635783604247913U;
    Zobrist::psq[9][31]  = 8370308497378149181U;
    Zobrist::psq[9][32]  = 16902199073103511157U;
    Zobrist::psq[9][33]  = 4712050710770633961U;
    Zobrist::psq[9][34]  = 2335277171236964126U;
    Zobrist::psq[9][35]  = 15454330651988402294U;
    Zobrist::psq[9][36]  = 6039398895644425870U;
    Zobrist::psq[9][37]  = 5330935207425949713U;
    Zobrist::psq[9][38]  = 6844204079868621004U;
    Zobrist::psq[9][39]  = 15018633515897982115U;
    Zobrist::psq[9][40]  = 5869887878873962697U;
    Zobrist::psq[9][41]  = 9619421978703093664U;
    Zobrist::psq[9][42]  = 7065039212033014872U;
    Zobrist::psq[9][43]  = 14085021312833583897U;
    Zobrist::psq[9][44]  = 17738639966636660046U;
    Zobrist::psq[9][45]  = 18274309123980813514U;
    Zobrist::psq[9][46]  = 16007640215959475868U;
    Zobrist::psq[9][47]  = 4326793000252505639U;
    Zobrist::psq[9][48]  = 11694193434453531305U;
    Zobrist::psq[9][49]  = 15789397716808962025U;
    Zobrist::psq[9][50]  = 8672273831614123897U;
    Zobrist::psq[9][51]  = 6109915657282875177U;
    Zobrist::psq[9][52]  = 6240221177136276484U;
    Zobrist::psq[9][53]  = 17650760467278016265U;
    Zobrist::psq[9][54]  = 13635783915766085055U;
    Zobrist::psq[9][55]  = 17178975703249397658U;
    Zobrist::psq[9][56]  = 690100752037560272U;
    Zobrist::psq[9][57]  = 846594232046156050U;
    Zobrist::psq[9][58]  = 11437611220054444781U;
    Zobrist::psq[9][59]  = 1050411833588837386U;
    Zobrist::psq[9][60]  = 10485589741397417446U;
    Zobrist::psq[9][61]  = 12844414679888429939U;
    Zobrist::psq[9][62]  = 6491358656106542835U;
    Zobrist::psq[9][63]  = 12575464921310399912U;
    Zobrist::psq[10][0]  = 14923825269739949453U;
    Zobrist::psq[10][1]  = 18375002115249413557U;
    Zobrist::psq[10][2]  = 3423036550911737589U;
    Zobrist::psq[10][3]  = 15250861506191355802U;
    Zobrist::psq[10][4]  = 15031961129285356212U;
    Zobrist::psq[10][5]  = 15435012606837965840U;
    Zobrist::psq[10][6]  = 6304673951675292305U;
    Zobrist::psq[10][7]  = 12785716655315370815U;
    Zobrist::psq[10][8]  = 9808873325341612945U;
    Zobrist::psq[10][9]  = 9783992785966697331U;
    Zobrist::psq[10][10] = 18138650430907468530U;
    Zobrist::psq[10][11] = 18431297401347671031U;
    Zobrist::psq[10][12] = 18148129570815566817U;
    Zobrist::psq[10][13] = 12696743950740820713U;
    Zobrist::psq[10][14] = 1854845205476015706U;
    Zobrist::psq[10][15] = 12865777516920439176U;
    Zobrist::psq[10][16] = 15636159047245426328U;
    Zobrist::psq[10][17] = 17373407353156678628U;
    Zobrist::psq[10][18] = 2495834645782650553U;
    Zobrist::psq[10][19] = 11247757644603045972U;
    Zobrist::psq[10][20] = 17130748698210142189U;
    Zobrist::psq[10][21] = 11422966446976074719U;
    Zobrist::psq[10][22] = 1595016003613213710U;
    Zobrist::psq[10][23] = 3899856913033553150U;
    Zobrist::psq[10][24] = 15470414105568996654U;
    Zobrist::psq[10][25] = 2572459120480840982U;
    Zobrist::psq[10][26] = 14288318049370965601U;
    Zobrist::psq[10][27] = 4034656711994978492U;
    Zobrist::psq[10][28] = 3619462250265206907U;
    Zobrist::psq[10][29] = 12564616267900212223U;
    Zobrist::psq[10][30] = 6563888989859451823U;
    Zobrist::psq[10][31] = 2454157599688795602U;
    Zobrist::psq[10][32] = 122761158351497116U;
    Zobrist::psq[10][33] = 4118064480546384385U;
    Zobrist::psq[10][34] = 13825342760651713002U;
    Zobrist::psq[10][35] = 3757958894065091138U;
    Zobrist::psq[10][36] = 3348351562535718824U;
    Zobrist::psq[10][37] = 11085064257829065607U;
    Zobrist::psq[10][38] = 4791949565677098244U;
    Zobrist::psq[10][39] = 16741859899153424134U;
    Zobrist::psq[10][40] = 13552228277894027114U;
    Zobrist::psq[10][41] = 18043793947072687525U;
    Zobrist::psq[10][42] = 18232133385309552782U;
    Zobrist::psq[10][43] = 17162542170033385071U;
    Zobrist::psq[10][44] = 17966719644677930276U;
    Zobrist::psq[10][45] = 4126374944389900134U;
    Zobrist::psq[10][46] = 7694029693525104626U;
    Zobrist::psq[10][47] = 7844796758498075948U;
    Zobrist::psq[10][48] = 15171322352384637386U;
    Zobrist::psq[10][49] = 4901284706517591019U;
    Zobrist::psq[10][50] = 11550611493505829690U;
    Zobrist::psq[10][51] = 8591758722916550176U;
    Zobrist::psq[10][52] = 6614280899913466481U;
    Zobrist::psq[10][53] = 15659292666557594854U;
    Zobrist::psq[10][54] = 8334845918197067198U;
    Zobrist::psq[10][55] = 14303347218899317731U;
    Zobrist::psq[10][56] = 18185681713739197231U;
    Zobrist::psq[10][57] = 10010957749676186008U;
    Zobrist::psq[10][58] = 6151588837035247399U;
    Zobrist::psq[10][59] = 15955998980864570780U;
    Zobrist::psq[10][60] = 14725804664707294906U;
    Zobrist::psq[10][61] = 9071111217904025772U;
    Zobrist::psq[10][62] = 4268551186589045976U;
    Zobrist::psq[10][63] = 3787505694838293655U;
    Zobrist::psq[11][0]  = 3463765996898474975U;
    Zobrist::psq[11][1]  = 1419043948633899671U;
    Zobrist::psq[11][2]  = 4738255775972431200U;
    Zobrist::psq[11][3]  = 10880687006345860054U;
    Zobrist::psq[11][4]  = 6083956890523873398U;
    Zobrist::psq[11][5]  = 15399367780949709721U;
    Zobrist::psq[11][6]  = 10077652868536637496U;
    Zobrist::psq[11][7]  = 4763774200646997281U;
    Zobrist::psq[11][8]  = 2058719554631509711U;
    Zobrist::psq[11][9]  = 16245257579300202929U;
    Zobrist::psq[11][10] = 12549234361408101229U;
    Zobrist::psq[11][11] = 5132111825598353706U;
    Zobrist::psq[11][12] = 13210867931726967807U;
    Zobrist::psq[11][13] = 8049587883156206974U;
    Zobrist::psq[11][14] = 14208790774466773366U;
    Zobrist::psq[11][15] = 15004789243215417478U;
    Zobrist::psq[11][16] = 2705161721287640173U;
    Zobrist::psq[11][17] = 6606951690346399114U;
    Zobrist::psq[11][18] = 9038858141657157738U;
    Zobrist::psq[11][19] = 9864507686211087503U;
    Zobrist::psq[11][20] = 8174211780307618304U;
    Zobrist::psq[11][21] = 16060351410629081351U;
    Zobrist::psq[11][22] = 5484951598904056885U;
    Zobrist::psq[11][23] = 12456759525904287919U;
    Zobrist::psq[11][24] = 8919252620379965524U;
    Zobrist::psq[11][25] = 15501107657356591656U;
    Zobrist::psq[11][26] = 3242949188225361282U;
    Zobrist::psq[11][27] = 5926058172544675863U;
    Zobrist::psq[11][28] = 6405123151097452666U;
    Zobrist::psq[11][29] = 172567736958909523U;
    Zobrist::psq[11][30] = 17292315564005737229U;
    Zobrist::psq[11][31] = 13464278685013338817U;
    Zobrist::psq[11][32] = 3686053955562449182U;
    Zobrist::psq[11][33] = 8857017014241158725U;
    Zobrist::psq[11][34] = 15421895718306499875U;
    Zobrist::psq[11][35] = 3815913251318905694U;
    Zobrist::psq[11][36] = 3432648465599995302U;
    Zobrist::psq[11][37] = 818320788389300537U;
    Zobrist::psq[11][38] = 4071520112108071604U;
    Zobrist::psq[11][39] = 13295466432639272442U;
    Zobrist::psq[11][40] = 2426572569594491679U;
    Zobrist::psq[11][41] = 10076303268977391406U;
    Zobrist::psq[11][42] = 8784192232334006419U;
    Zobrist::psq[11][43] = 2997181738853009670U;
    Zobrist::psq[11][44] = 15770398685934330580U;
    Zobrist::psq[11][45] = 13017264784195056557U;
    Zobrist::psq[11][46] = 4330776497582490757U;
    Zobrist::psq[11][47] = 10934498588458332802U;
    Zobrist::psq[11][48] = 10356579632341837397U;
    Zobrist::psq[11][49] = 2098241031318749487U;
    Zobrist::psq[11][50] = 14789448409803449028U;
    Zobrist::psq[11][51] = 11251433970760721438U;
    Zobrist::psq[11][52] = 7224004101031043677U;
    Zobrist::psq[11][53] = 15038935143876354117U;
    Zobrist::psq[11][54] = 13215483265469582733U;
    Zobrist::psq[11][55] = 1462298635979286935U;
    Zobrist::psq[11][56] = 5759284467508932139U;
    Zobrist::psq[11][57] = 5761810302276021825U;
    Zobrist::psq[11][58] = 1946852319481058342U;
    Zobrist::psq[11][59] = 8779292626819401953U;
    Zobrist::psq[11][60] = 9980275774854520963U;
    Zobrist::psq[11][61] = 9018156077605645253U;
    Zobrist::psq[11][62] = 10175632970326281074U;
    Zobrist::psq[11][63] = 17670251009423356428U;
    Zobrist::psq[12][0]  = 2047473063754745880U;
    Zobrist::psq[12][1]  = 4129462703004022451U;
    Zobrist::psq[12][2]  = 10030514736718131075U;
    Zobrist::psq[12][3]  = 8457187454173219884U;
    Zobrist::psq[12][4]  = 675824455430313366U;
    Zobrist::psq[12][5]  = 15722708499135010396U;
    Zobrist::psq[12][6]  = 1416150021210949828U;
    Zobrist::psq[12][7]  = 18340753630988628266U;
    Zobrist::psq[12][8]  = 4279562020148953383U;
    Zobrist::psq[12][9]  = 7599717795808621650U;
    Zobrist::psq[12][10] = 8493385059263161629U;
    Zobrist::psq[12][11] = 5448373608430482181U;
    Zobrist::psq[12][12] = 7975000343659144004U;
    Zobrist::psq[12][13] = 3661443877569162353U;
    Zobrist::psq[12][14] = 17436434418308603210U;
    Zobrist::psq[12][15] = 7723061412912586436U;
    Zobrist::psq[12][16] = 12478269109366344372U;
    Zobrist::psq[12][17] = 5260527761162561230U;
    Zobrist::psq[12][18] = 3664808336308943032U;
    Zobrist::psq[12][19] = 12246522629121956498U;
    Zobrist::psq[12][20] = 11421384233946319246U;
    Zobrist::psq[12][21] = 10711232448204740396U;
    Zobrist::psq[12][22] = 394033332107778027U;
    Zobrist::psq[12][23] = 1653867462011650260U;
    Zobrist::psq[12][24] = 10614247855083729040U;
    Zobrist::psq[12][25] = 3511207051989217747U;
    Zobrist::psq[12][26] = 14828688729293007936U;
    Zobrist::psq[12][27] = 12730238737606105501U;
    Zobrist::psq[12][28] = 9131161340116597330U;
    Zobrist::psq[12][29] = 10475424158865388660U;
    Zobrist::psq[12][30] = 12216784836515690585U;
    Zobrist::psq[12][31] = 12605719261947498045U;
    Zobrist::psq[12][32] = 55059904350528673U;
    Zobrist::psq[12][33] = 5668017292185949458U;
    Zobrist::psq[12][34] = 5318848626170854652U;
    Zobrist::psq[12][35] = 5812165408168894719U;
    Zobrist::psq[12][36] = 12436591089168384586U;
    Zobrist::psq[12][37] = 11456184110470635333U;
    Zobrist::psq[12][38] = 17354703890556504985U;
    Zobrist::psq[12][39] = 12819708191444916183U;
    Zobrist::psq[12][40] = 2051969874001439467U;
    Zobrist::psq[12][41] = 9752086654524583546U;
    Zobrist::psq[12][42] = 8598830537031500033U;
    Zobrist::psq[12][43] = 10803717843971298140U;
    Zobrist::psq[12][44] = 17386254373003795027U;
    Zobrist::psq[12][45] = 3490013643061567317U;
    Zobrist::psq[12][46] = 14966160920336416174U;
    Zobrist::psq[12][47] = 2716159408585464742U;
    Zobrist::psq[12][48] = 13704057180721116715U;
    Zobrist::psq[12][49] = 6139827121406310950U;
    Zobrist::psq[12][50] = 12045645008689575811U;
    Zobrist::psq[12][51] = 5879666907986225363U;
    Zobrist::psq[12][52] = 18332108852121545326U;
    Zobrist::psq[12][53] = 8302596541641486393U;
    Zobrist::psq[12][54] = 3337300269606353125U;
    Zobrist::psq[12][55] = 4641043901128821440U;
    Zobrist::psq[12][56] = 17552658021160699704U;
    Zobrist::psq[12][57] = 15245517114959849830U;
    Zobrist::psq[12][58] = 898774234328201642U;
    Zobrist::psq[12][59] = 13458365488972458856U;
    Zobrist::psq[12][60] = 17617352963801145870U;
    Zobrist::psq[12][61] = 12653043169047643133U;
    Zobrist::psq[12][62] = 3946055118622982785U;
    Zobrist::psq[12][63] = 78667567517654999U;
    Zobrist::psq[13][0]  = 7496345100749090134U;
    Zobrist::psq[13][1]  = 11141138397664383499U;
    Zobrist::psq[13][2]  = 9990861652354760086U;
    Zobrist::psq[13][3]  = 6136051413974204120U;
    Zobrist::psq[13][4]  = 14382251659553821084U;
    Zobrist::psq[13][5]  = 12222838175704680581U;
    Zobrist::psq[13][6]  = 9437743647758681312U;
    Zobrist::psq[13][7]  = 5321952072316248116U;
    Zobrist::psq[13][8]  = 9510472571572253025U;
    Zobrist::psq[13][9]  = 13968738580144591953U;
    Zobrist::psq[13][10] = 9048732621241245672U;
    Zobrist::psq[13][11] = 7070992119077796289U;
    Zobrist::psq[13][12] = 7585987196905721881U;
    Zobrist::psq[13][13] = 12797609451470009512U;
    Zobrist::psq[13][14] = 13831169997283951441U;
    Zobrist::psq[13][15] = 14062956797276305407U;
    Zobrist::psq[13][16] = 7195172102806297836U;
    Zobrist::psq[13][17] = 13763135782447679404U;
    Zobrist::psq[13][18] = 8729177333120200902U;
    Zobrist::psq[13][19] = 8228513033455726756U;
    Zobrist::psq[13][20] = 5827889096510108059U;
    Zobrist::psq[13][21] = 1541817158620711182U;
    Zobrist::psq[13][22] = 18002525473269359251U;
    Zobrist::psq[13][23] = 7210349805272776282U;
    Zobrist::psq[13][24] = 6760744891923215431U;
    Zobrist::psq[13][25] = 1684012349959865632U;
    Zobrist::psq[13][26] = 5422658641223860702U;
    Zobrist::psq[13][27] = 5964630753289401637U;
    Zobrist::psq[13][28] = 16048931659747747714U;
    Zobrist::psq[13][29] = 12995369105282084360U;
    Zobrist::psq[13][30] = 2210225853011473806U;
    Zobrist::psq[13][31] = 13310794355402477849U;
    Zobrist::psq[13][32] = 4356361331354780175U;
    Zobrist::psq[13][33] = 10920940233470324174U;
    Zobrist::psq[13][34] = 4480682637160025854U;
    Zobrist::psq[13][35] = 11920920861864075275U;
    Zobrist::psq[13][36] = 17830720560385394644U;
    Zobrist::psq[13][37] = 17667812763781863653U;
    Zobrist::psq[13][38] = 8584251371203620679U;
    Zobrist::psq[13][39] = 10083927648945854194U;
    Zobrist::psq[13][40] = 15175717840117055506U;
    Zobrist::psq[13][41] = 3402388332801799152U;
    Zobrist::psq[13][42] = 17983756367024412696U;
    Zobrist::psq[13][43] = 13633521765968038314U;
    Zobrist::psq[13][44] = 18197623828188242686U;
    Zobrist::psq[13][45] = 7159151014196207335U;
    Zobrist::psq[13][46] = 6329323109608928752U;
    Zobrist::psq[13][47] = 4596348075478973761U;
    Zobrist::psq[13][48] = 1929043772203993371U;
    Zobrist::psq[13][49] = 2942782730029388844U;
    Zobrist::psq[13][50] = 17616535832761962408U;
    Zobrist::psq[13][51] = 14638746212880920282U;
    Zobrist::psq[13][52] = 235408037287298392U;
    Zobrist::psq[13][53] = 15488773953079788133U;
    Zobrist::psq[13][54] = 14511691540381881087U;
    Zobrist::psq[13][55] = 4908241668947178463U;
    Zobrist::psq[13][56] = 8002325218109467205U;
    Zobrist::psq[13][57] = 384694259305835297U;
    Zobrist::psq[13][58] = 4413022859932656147U;
    Zobrist::psq[13][59] = 16084510603130945976U;
    Zobrist::psq[13][60] = 7817184652260023923U;
    Zobrist::psq[13][61] = 11521163704900182019U;
    Zobrist::psq[13][62] = 10633473972031941012U;
    Zobrist::psq[13][63] = 7028123206539359005U;
    Zobrist::psq[14][0]  = 12370129909167185711U;
    Zobrist::psq[14][1]  = 18282545875249343957U;
    Zobrist::psq[14][2]  = 11571910781648655955U;
    Zobrist::psq[14][3]  = 12044362528788437371U;
    Zobrist::psq[14][4]  = 15748959137105604538U;
    Zobrist::psq[14][5]  = 12433669315838447795U;
    Zobrist::psq[14][6]  = 3539341563356477798U;
    Zobrist::psq[14][7]  = 8229636981602574987U;
    Zobrist::psq[14][8]  = 18267920850505015981U;
    Zobrist::psq[14][9]  = 18135187956959905864U;
    Zobrist::psq[14][10] = 10122403804874825725U;
    Zobrist::psq[14][11] = 8577640427585662579U;
    Zobrist::psq[14][12] = 16947872026033056961U;
    Zobrist::psq[14][13] = 4498886674923994328U;
    Zobrist::psq[14][14] = 5110446196942225801U;
    Zobrist::psq[14][15] = 2443501881669395127U;
    Zobrist::psq[14][16] = 6915148508579620831U;
    Zobrist::psq[14][17] = 9154422921438056207U;
    Zobrist::psq[14][18] = 3578030806440286511U;
    Zobrist::psq[14][19] = 15315801991440539300U;
    Zobrist::psq[14][20] = 7070866824836391168U;
    Zobrist::psq[14][21] = 14817924832942381111U;
    Zobrist::psq[14][22] = 3001446271118775643U;
    Zobrist::psq[14][23] = 13000642695841600636U;
    Zobrist::psq[14][24] = 14370567463871457833U;
    Zobrist::psq[14][25] = 11030064684553339453U;
    Zobrist::psq[14][26] = 14239970918075645415U;
    Zobrist::psq[14][27] = 9415971121016597759U;
    Zobrist::psq[14][28] = 6665243610733579451U;
    Zobrist::psq[14][29] = 12729882327349519727U;
    Zobrist::psq[14][30] = 127495542892799647U;
    Zobrist::psq[14][31] = 6044073010763988256U;
    Zobrist::psq[14][32] = 13007064564721953048U;
    Zobrist::psq[14][33] = 13888665226332397302U;
    Zobrist::psq[14][34] = 13536486134713258398U;
    Zobrist::psq[14][35] = 16493663995181111698U;
    Zobrist::psq[14][36] = 2130152061385863810U;
    Zobrist::psq[14][37] = 5369940202574713097U;
    Zobrist::psq[14][38] = 4976109024626592507U;
    Zobrist::psq[14][39] = 17662718886951473514U;
    Zobrist::psq[14][40] = 10194604604769366768U;
    Zobrist::psq[14][41] = 9434649875492567077U;
    Zobrist::psq[14][42] = 9275344374679790988U;
    Zobrist::psq[14][43] = 13950395516943844512U;
    Zobrist::psq[14][44] = 4634019286100624619U;
    Zobrist::psq[14][45] = 17524913661501655732U;
    Zobrist::psq[14][46] = 12758868016771465513U;
    Zobrist::psq[14][47] = 3127147764315865797U;
    Zobrist::psq[14][48] = 3960938717909563730U;
    Zobrist::psq[14][49] = 14869830638616427590U;
    Zobrist::psq[14][50] = 305185646789997459U;
    Zobrist::psq[14][51] = 4139658351799906696U;
    Zobrist::psq[14][52] = 272667046354598132U;
    Zobrist::psq[14][53] = 15621274402096728762U;
    Zobrist::psq[14][54] = 16483498129229512495U;
    Zobrist::psq[14][55] = 12953368655171389128U;
    Zobrist::psq[14][56] = 10678035399177741929U;
    Zobrist::psq[14][57] = 18049652274331575310U;
    Zobrist::psq[14][58] = 7975081034372805163U;
    Zobrist::psq[14][59] = 10522098076497821829U;
    Zobrist::psq[14][60] = 12606359703294662790U;
    Zobrist::psq[14][61] = 13924857104548874958U;
    Zobrist::psq[14][62] = 6566773282407180921U;
    Zobrist::psq[14][63] = 3452471826952569846U;
    // pawns on these squares will promote
    std::fill_n(Zobrist::psq[W_PAWN] + SQ_A8, 8, 0);
    std::fill_n(Zobrist::psq[B_PAWN], 8, 0);
    Zobrist::enpassant[0] = 9031641776876329352U;
    Zobrist::enpassant[1] = 12228382040141709029U;
    Zobrist::enpassant[2] = 2494223668561036951U;
    Zobrist::enpassant[3] = 7849557628814744642U;
    Zobrist::enpassant[4] = 16000570245257669890U;
    Zobrist::enpassant[5] = 16614404541835922253U;
    Zobrist::enpassant[6] = 17787301719840479309U;
    Zobrist::enpassant[7] = 6371708097697762807U;
    Zobrist::castling[0]  = 7487338029351702425U;
    Zobrist::castling[1]  = 10138645747811604478U;
    Zobrist::castling[2]  = 16332212992845378228U;
    Zobrist::castling[3]  = 6795873897769436611U;
    Zobrist::castling[4]  = 4906379431808431525U;
    Zobrist::castling[5]  = 895963052000028445U;
    Zobrist::castling[6]  = 9908024112921185340U;
    Zobrist::castling[7]  = 11968150447396703004U;
    Zobrist::castling[8]  = 9479661950789363348U;
    Zobrist::castling[9]  = 13855803985643744043U;
    Zobrist::castling[10] = 10705462234079318642U;
    Zobrist::castling[11] = 15193185854920610048U;
    Zobrist::castling[12] = 11090572498526290765U;
    Zobrist::castling[13] = 2577199524176359085U;
    Zobrist::castling[14] = 14147118971151220805U;
    Zobrist::castling[15] = 7849635286513694150U;
    Zobrist::side         = 11598212307497933772U;
    Zobrist::noPawns      = 11773087935368354602U;
    //learning end

    // Prepare the cuckoo tables
    cuckoo.fill(0);
    cuckooMove.fill(Move::none());
    [[maybe_unused]] int count = 0;
    for (Piece pc : Pieces)
        for (Square s1 = SQ_A1; s1 <= SQ_H8; ++s1)
            for (Square s2 = Square(s1 + 1); s2 <= SQ_H8; ++s2)
                if ((type_of(pc) != PAWN) && (attacks_bb(type_of(pc), s1, 0) & s2))
                {
                    Move move = Move(s1, s2);
                    Key  key  = Zobrist::psq[pc][s1] ^ Zobrist::psq[pc][s2] ^ Zobrist::side;
                    int  i    = H1(key);
                    while (true)
                    {
                        std::swap(cuckoo[i], key);
                        std::swap(cuckooMove[i], move);
                        if (move == Move::none())  // Arrived at empty slot?
                            break;
                        i = (i == H1(key)) ? H2(key) : H1(key);  // Push victim to alternative slot
                    }
                    count++;
                }
    assert(count == 3668);
}


// Initializes the position object with the given FEN string.
// This function is not very robust - make sure that input FENs are correct,
// this is assumed to be the responsibility of the GUI.
Position&
Position::set(const string& fenStr, bool isChess960, StateInfo* si, Thread* th) {  //for classical
    /*
   A FEN string defines a particular position using only the ASCII character set.

   A FEN string contains six fields separated by a space. The fields are:

   1) Piece placement (from white's perspective). Each rank is described, starting
      with rank 8 and ending with rank 1. Within each rank, the contents of each
      square are described from file A through file H. Following the Standard
      Algebraic Notation (SAN), each piece is identified by a single letter taken
      from the standard English names. White pieces are designated using upper-case
      letters ("PNBRQK") whilst Black uses lowercase ("pnbrqk"). Blank squares are
      noted using digits 1 through 8 (the number of blank squares), and "/"
      separates ranks.

   2) Active color. "w" means white moves next, "b" means black.

   3) Castling availability. If neither side can castle, this is "-". Otherwise,
      this has one or more letters: "K" (White can castle kingside), "Q" (White
      can castle queenside), "k" (Black can castle kingside), and/or "q" (Black
      can castle queenside).

   4) En passant target square (in algebraic notation). If there's no en passant
      target square, this is "-". If a pawn has just made a 2-square move, this
      is the position "behind" the pawn. Following X-FEN standard, this is recorded
      only if there is a pawn in position to make an en passant capture, and if
      there really is a pawn that might have advanced two squares.

   5) Halfmove clock. This is the number of halfmoves since the last pawn advance
      or capture. This is used to determine if a draw can be claimed under the
      fifty-move rule.

   6) Fullmove number. The number of the full move. It starts at 1, and is
      incremented after Black's move.
*/

    unsigned char      col, row, token;
    size_t             idx;
    Square             sq = SQ_A8;
    std::istringstream ss(fenStr);

    std::memset(reinterpret_cast<char*>(this), 0, sizeof(Position));
    std::memset(si, 0, sizeof(StateInfo));
    st = si;

    ss >> std::noskipws;

    // 1. Piece placement
    while ((ss >> token) && !isspace(token))
    {
        if (isdigit(token))
            sq += (token - '0') * EAST;  // Advance the given number of files

        else if (token == '/')
            sq += 2 * SOUTH;

        else if ((idx = PieceToChar.find(token)) != string::npos)
        {
            put_piece(Piece(idx), sq);
            ++sq;
        }
    }

    // 2. Active color
    ss >> token;
    sideToMove = (token == 'w' ? WHITE : BLACK);
    ss >> token;

    // 3. Castling availability. Compatible with 3 standards: Normal FEN standard,
    // Shredder-FEN that uses the letters of the columns on which the rooks began
    // the game instead of KQkq and also X-FEN standard that, in case of Chess960,
    // if an inner rook is associated with the castling right, the castling tag is
    // replaced by the file letter of the involved rook, as for the Shredder-FEN.
    while ((ss >> token) && !isspace(token))
    {
        Square rsq;
        Color  c    = islower(token) ? BLACK : WHITE;
        Piece  rook = make_piece(c, ROOK);

        token = char(toupper(token));

        if (token == 'K')
            for (rsq = relative_square(c, SQ_H1); piece_on(rsq) != rook; --rsq)
            {}

        else if (token == 'Q')
            for (rsq = relative_square(c, SQ_A1); piece_on(rsq) != rook; ++rsq)
            {}

        else if (token >= 'A' && token <= 'H')
            rsq = make_square(File(token - 'A'), relative_rank(c, RANK_1));

        else
            continue;

        set_castling_right(c, rsq);
    }

    // 4. En passant square.
    // Ignore if square is invalid or not on side to move relative rank 6.
    bool enpassant = false;

    if (((ss >> col) && (col >= 'a' && col <= 'h'))
        && ((ss >> row) && (row == (sideToMove == WHITE ? '6' : '3'))))
    {
        st->epSquare = make_square(File(col - 'a'), Rank(row - '1'));

        // En passant square will be considered only if
        // a) side to move have a pawn threatening epSquare
        // b) there is an enemy pawn in front of epSquare
        // c) there is no piece on epSquare or behind epSquare
        enpassant = attacks_bb<PAWN>(st->epSquare, ~sideToMove) & pieces(sideToMove, PAWN)
                 && (pieces(~sideToMove, PAWN) & (st->epSquare + pawn_push(~sideToMove)))
                 && !(pieces() & (st->epSquare | (st->epSquare + pawn_push(sideToMove))));
    }

    if (!enpassant)
        st->epSquare = SQ_NONE;

    // 5-6. Halfmove clock and fullmove number
    ss >> std::skipws >> st->rule50 >> gamePly;

    // Convert from fullmove starting from 1 to gamePly starting from 0,
    // handle also common incorrect FEN with fullmove = 0.
    gamePly = std::max(2 * (gamePly - 1), 0) + (sideToMove == BLACK);

    chess960 = isChess960;
    //for classical begin
    if (th != nullptr)
    {
        thisThread = th;
    }
    //for classical end
    set_state();

    assert(pos_is_ok());

    return *this;
}


// Helper function used to set castling
// rights given the corresponding color and the rook starting square.
void Position::set_castling_right(Color c, Square rfrom) {

    Square         kfrom = square<KING>(c);
    CastlingRights cr    = c & (kfrom < rfrom ? KING_SIDE : QUEEN_SIDE);

    st->castlingRights |= cr;
    castlingRightsMask[kfrom] |= cr;
    castlingRightsMask[rfrom] |= cr;
    castlingRookSquare[cr] = rfrom;

    Square kto = relative_square(c, cr & KING_SIDE ? SQ_G1 : SQ_C1);
    Square rto = relative_square(c, cr & KING_SIDE ? SQ_F1 : SQ_D1);

    castlingPath[cr] = (between_bb(rfrom, rto) | between_bb(kfrom, kto)) & ~(kfrom | rfrom);
}


// Sets king attacks to detect if a move gives check
void Position::set_check_info() const {

    update_slider_blockers(WHITE);
    update_slider_blockers(BLACK);

    Square ksq = square<KING>(~sideToMove);

    st->checkSquares[PAWN]   = attacks_bb<PAWN>(ksq, ~sideToMove);
    st->checkSquares[KNIGHT] = attacks_bb<KNIGHT>(ksq);
    st->checkSquares[BISHOP] = attacks_bb<BISHOP>(ksq, pieces());
    st->checkSquares[ROOK]   = attacks_bb<ROOK>(ksq, pieces());
    st->checkSquares[QUEEN]  = st->checkSquares[BISHOP] | st->checkSquares[ROOK];
    st->checkSquares[KING]   = 0;
}


// Computes the hash keys of the position, and other
// data that once computed is updated incrementally as moves are made.
// The function is only used when a new position is set up
void Position::set_state() const {

    st->key               = 0;
    st->minorPieceKey     = 0;
    st->nonPawnKey[WHITE] = st->nonPawnKey[BLACK] = 0;
    st->pawnKey                                   = Zobrist::noPawns;
    st->nonPawnMaterial[WHITE] = st->nonPawnMaterial[BLACK] = VALUE_ZERO;
    st->checkersBB = attackers_to(square<KING>(sideToMove)) & pieces(~sideToMove);

    set_check_info();

    for (Bitboard b = pieces(); b;)
    {
        Square s  = pop_lsb(b);
        Piece  pc = piece_on(s);
        st->key ^= Zobrist::psq[pc][s];

        if (type_of(pc) == PAWN)
            st->pawnKey ^= Zobrist::psq[pc][s];

        else
        {
            st->nonPawnKey[color_of(pc)] ^= Zobrist::psq[pc][s];

            if (type_of(pc) != KING)
            {
                st->nonPawnMaterial[color_of(pc)] += PieceValue[pc];

                if (type_of(pc) <= BISHOP)
                    st->minorPieceKey ^= Zobrist::psq[pc][s];
            }
        }
    }

    if (st->epSquare != SQ_NONE)
        st->key ^= Zobrist::enpassant[file_of(st->epSquare)];

    if (sideToMove == BLACK)
        st->key ^= Zobrist::side;

    st->key ^= Zobrist::castling[st->castlingRights];
    st->materialKey = compute_material_key();
}

Key Position::compute_material_key() const {
    Key k = 0;
    for (Piece pc : Pieces)
        for (int cnt = 0; cnt < pieceCount[pc]; ++cnt)
            k ^= Zobrist::psq[pc][8 + cnt];
    return k;
}


// Overload to initialize the position object with the given endgame code string
// like "KBPKN". It's mainly a helper to get the material key out of an endgame code.
Position& Position::set(const string& code, Color c, StateInfo* si) {

    assert(code[0] == 'K');

    string sides[] = {code.substr(code.find('K', 1)),                                // Weak
                      code.substr(0, std::min(code.find('v'), code.find('K', 1)))};  // Strong

    assert(sides[0].length() > 0 && sides[0].length() < 8);
    assert(sides[1].length() > 0 && sides[1].length() < 8);

    std::transform(sides[c].begin(), sides[c].end(), sides[c].begin(), tolower);

    string fenStr = "8/" + sides[0] + char(8 - sides[0].length() + '0') + "/8/8/8/8/" + sides[1]
                  + char(8 - sides[1].length() + '0') + "/8 w - - 0 10";

    return set(fenStr, false, si, nullptr);  //for classical
}


// Returns a FEN representation of the position. In case of
// Chess960 the Shredder-FEN notation is used. This is mainly a debugging function.
string Position::fen() const {

    int                emptyCnt;
    std::ostringstream ss;

    for (Rank r = RANK_8;; --r)
    {
        for (File f = FILE_A; f <= FILE_H; ++f)
        {
            for (emptyCnt = 0; f <= FILE_H && empty(make_square(f, r)); ++f)
                ++emptyCnt;

            if (emptyCnt)
                ss << emptyCnt;

            if (f <= FILE_H)
                ss << PieceToChar[piece_on(make_square(f, r))];
        }

        if (r == RANK_1)
            break;
        ss << '/';
    }

    ss << (sideToMove == WHITE ? " w " : " b ");

    if (can_castle(WHITE_OO))
        ss << (chess960 ? char('A' + file_of(castling_rook_square(WHITE_OO))) : 'K');

    if (can_castle(WHITE_OOO))
        ss << (chess960 ? char('A' + file_of(castling_rook_square(WHITE_OOO))) : 'Q');

    if (can_castle(BLACK_OO))
        ss << (chess960 ? char('a' + file_of(castling_rook_square(BLACK_OO))) : 'k');

    if (can_castle(BLACK_OOO))
        ss << (chess960 ? char('a' + file_of(castling_rook_square(BLACK_OOO))) : 'q');

    if (!can_castle(ANY_CASTLING))
        ss << '-';

    ss << (ep_square() == SQ_NONE ? " - " : " " + UCIEngine::square(ep_square()) + " ")
       << st->rule50 << " " << 1 + (gamePly - (sideToMove == BLACK)) / 2;

    return ss.str();
}
//for classical begin
/// Position::slider_blockers() returns a bitboard of all the pieces (both colors)
/// that are blocking attacks on the square 's' from 'sliders'. A piece blocks a
/// slider if removing that piece from the board would result in a position where
/// square 's' is attacked. For example, a king-attack blocking piece can be either
/// a pinned or a discovered check piece, according if its color is the opposite
/// or the same of the color of the slider.

Bitboard Position::slider_blockers(Bitboard sliders, Square s, Bitboard& pinners) const {

    Bitboard blockers = 0;
    pinners           = 0;

    // Snipers are sliders that attack 's' when a piece and other snipers are removed
    Bitboard snipers = ((attacks_bb<ROOK>(s) & pieces(QUEEN, ROOK))
                        | (attacks_bb<BISHOP>(s) & pieces(QUEEN, BISHOP)))
                     & sliders;
    Bitboard occupancy = pieces() ^ snipers;

    while (snipers)
    {
        Square   sniperSq = pop_lsb(snipers);
        Bitboard b        = between_bb(s, sniperSq) & occupancy;

        if (b && !more_than_one(b))
        {
            blockers |= b;
            if (b & pieces(color_of(piece_on(s))))
                pinners |= sniperSq;
        }
    }
    return blockers;
}
//for classical end
// Calculates st->blockersForKing[c] and st->pinners[~c],
// which store respectively the pieces preventing king of color c from being in check
// and the slider pieces of color ~c pinning pieces of color c to the king.
void Position::update_slider_blockers(Color c) const {

    Square ksq = square<KING>(c);

    st->blockersForKing[c] = 0;
    st->pinners[~c]        = 0;

    // Snipers are sliders that attack 's' when a piece and other snipers are removed
    Bitboard snipers = ((attacks_bb<ROOK>(ksq) & pieces(QUEEN, ROOK))
                        | (attacks_bb<BISHOP>(ksq) & pieces(QUEEN, BISHOP)))
                     & pieces(~c);
    Bitboard occupancy = pieces() ^ snipers;

    while (snipers)
    {
        Square   sniperSq = pop_lsb(snipers);
        Bitboard b        = between_bb(ksq, sniperSq) & occupancy;

        if (b && !more_than_one(b))
        {
            st->blockersForKing[c] |= b;
            if (b & pieces(c))
                st->pinners[~c] |= sniperSq;
        }
    }
}


// Computes a bitboard of all pieces which attack a given square.
// Slider attacks use the occupied bitboard to indicate occupancy.
Bitboard Position::attackers_to(Square s, Bitboard occupied) const {

    return (attacks_bb<ROOK>(s, occupied) & pieces(ROOK, QUEEN))
         | (attacks_bb<BISHOP>(s, occupied) & pieces(BISHOP, QUEEN))
         | (attacks_bb<PAWN>(s, BLACK) & pieces(WHITE, PAWN))
         | (attacks_bb<PAWN>(s, WHITE) & pieces(BLACK, PAWN))
         | (attacks_bb<KNIGHT>(s) & pieces(KNIGHT)) | (attacks_bb<KING>(s) & pieces(KING));
}

bool Position::attackers_to_exist(Square s, Bitboard occupied, Color c) const {

    return ((attacks_bb<ROOK>(s) & pieces(c, ROOK, QUEEN))
            && (attacks_bb<ROOK>(s, occupied) & pieces(c, ROOK, QUEEN)))
        || ((attacks_bb<BISHOP>(s) & pieces(c, BISHOP, QUEEN))
            && (attacks_bb<BISHOP>(s, occupied) & pieces(c, BISHOP, QUEEN)))
        || (((attacks_bb<PAWN>(s, ~c) & pieces(PAWN)) | (attacks_bb<KNIGHT>(s) & pieces(KNIGHT))
             | (attacks_bb<KING>(s) & pieces(KING)))
            & pieces(c));
}

// Tests whether a pseudo-legal move is legal
bool Position::legal(Move m) const {

    assert(m.is_ok());

    Color  us   = sideToMove;
    Square from = m.from_sq();
    Square to   = m.to_sq();

    assert(color_of(moved_piece(m)) == us);
    assert(piece_on(square<KING>(us)) == make_piece(us, KING));

    // En passant captures are a tricky special case. Because they are rather
    // uncommon, we do it simply by testing whether the king is attacked after
    // the move is made.
    if (m.type_of() == EN_PASSANT)
    {
        Square   ksq      = square<KING>(us);
        Square   capsq    = to - pawn_push(us);
        Bitboard occupied = (pieces() ^ from ^ capsq) | to;

        assert(to == ep_square());
        assert(moved_piece(m) == make_piece(us, PAWN));
        assert(piece_on(capsq) == make_piece(~us, PAWN));
        assert(piece_on(to) == NO_PIECE);

        return !(attacks_bb<ROOK>(ksq, occupied) & pieces(~us, QUEEN, ROOK))
            && !(attacks_bb<BISHOP>(ksq, occupied) & pieces(~us, QUEEN, BISHOP));
    }

    // Castling moves generation does not check if the castling path is clear of
    // enemy attacks, it is delayed at a later time: now!
    if (m.type_of() == CASTLING)
    {
        // After castling, the rook and king final positions are the same in
        // Chess960 as they would be in standard chess.
        to             = relative_square(us, to > from ? SQ_G1 : SQ_C1);
        Direction step = to > from ? WEST : EAST;

        for (Square s = to; s != from; s += step)
            if (attackers_to_exist(s, pieces(), ~us))
                return false;

        // In case of Chess960, verify if the Rook blocks some checks.
        // For instance an enemy queen in SQ_A1 when castling rook is in SQ_B1.
        return !chess960 || !(blockers_for_king(us) & m.to_sq());
    }

    // If the moving piece is a king, check whether the destination square is
    // attacked by the opponent.
    if (type_of(piece_on(from)) == KING)
        return !(attackers_to_exist(to, pieces() ^ from, ~us));

    // A non-king move is legal if and only if it is not pinned or it
    // is moving along the ray towards or away from the king.
    return !(blockers_for_king(us) & from) || line_bb(from, to) & pieces(us, KING);
}


// Takes a random move and tests whether the move is
// pseudo-legal. It is used to validate moves from TT that can be corrupted
// due to SMP concurrent access or hash position key aliasing.
bool Position::pseudo_legal(const Move m) const {

    Color  us   = sideToMove;
    Square from = m.from_sq();
    Square to   = m.to_sq();
    Piece  pc   = moved_piece(m);

    // Use a slower but simpler function for uncommon cases
    // yet we skip the legality check of MoveList<LEGAL>().
    if (m.type_of() != NORMAL)
        return checkers() ? MoveList<EVASIONS>(*this).contains(m)
                          : MoveList<NON_EVASIONS>(*this).contains(m);

    // Is not a promotion, so the promotion piece must be empty
    assert(m.promotion_type() - KNIGHT == NO_PIECE_TYPE);

    // If the 'from' square is not occupied by a piece belonging to the side to
    // move, the move is obviously not legal.
    if (pc == NO_PIECE || color_of(pc) != us)
        return false;

    // The destination square cannot be occupied by a friendly piece
    if (pieces(us) & to)
        return false;

    // Handle the special case of a pawn move
    if (type_of(pc) == PAWN)
    {
        // We have already handled promotion moves, so destination cannot be on the 8th/1st rank
        if ((Rank8BB | Rank1BB) & to)
            return false;

        // Check if it's a valid capture, single push, or double push
        const bool isCapture    = bool(attacks_bb<PAWN>(from, us) & pieces(~us) & to);
        const bool isSinglePush = (from + pawn_push(us) == to) && empty(to);
        const bool isDoublePush = (from + 2 * pawn_push(us) == to)
                               && (relative_rank(us, from) == RANK_2) && empty(to)
                               && empty(to - pawn_push(us));

        if (!(isCapture || isSinglePush || isDoublePush))
            return false;
    }
    else if (!(attacks_bb(type_of(pc), from, pieces()) & to))
        return false;

    // Evasions generator already takes care to avoid some kind of illegal moves
    // and legal() relies on this. We therefore have to take care that the same
    // kind of moves are filtered out here.
    if (checkers())
    {
        if (type_of(pc) != KING)
        {
            // Double check? In this case, a king move is required
            if (more_than_one(checkers()))
                return false;

            // Our move must be a blocking interposition or a capture of the checking piece
            if (!(between_bb(square<KING>(us), lsb(checkers())) & to))
                return false;
        }
        // In case of king moves under check we have to remove the king so as to catch
        // invalid moves like b1a1 when opposite queen is on c1.
        else if (attackers_to_exist(to, pieces() ^ from, ~us))
            return false;
    }

    return true;
}


// Tests whether a pseudo-legal move gives a check
bool Position::gives_check(Move m) const {

    assert(m.is_ok());
    assert(color_of(moved_piece(m)) == sideToMove);

    Square from = m.from_sq();
    Square to   = m.to_sq();

    // Is there a direct check?
    if (check_squares(type_of(piece_on(from))) & to)
        return true;

    // Is there a discovered check?
    if (blockers_for_king(~sideToMove) & from)
        return !(line_bb(from, to) & pieces(~sideToMove, KING)) || m.type_of() == CASTLING;

    switch (m.type_of())
    {
    case NORMAL :
        return false;

    case PROMOTION :
        return attacks_bb(m.promotion_type(), to, pieces() ^ from) & pieces(~sideToMove, KING);

    // En passant capture with check? We have already handled the case of direct
    // checks and ordinary discovered check, so the only case we need to handle
    // is the unusual case of a discovered check through the captured pawn.
    case EN_PASSANT : {
        Square   capsq = make_square(file_of(to), rank_of(from));
        Bitboard b     = (pieces() ^ from ^ capsq) | to;

        return (attacks_bb<ROOK>(square<KING>(~sideToMove), b) & pieces(sideToMove, QUEEN, ROOK))
             | (attacks_bb<BISHOP>(square<KING>(~sideToMove), b)
                & pieces(sideToMove, QUEEN, BISHOP));
    }
    default :  //CASTLING
    {
        // Castling is encoded as 'king captures the rook'
        Square rto = relative_square(sideToMove, to > from ? SQ_F1 : SQ_D1);

        return check_squares(ROOK) & rto;
    }
    }
}


// Makes a move, and saves all information necessary
// to a StateInfo object. The move is assumed to be legal. Pseudo-legal
// moves should be filtered out before this function is called.
// If a pointer to the TT table is passed, the entry for the new position
// will be prefetched, and likewise for shared history.
void Position::do_move(Move                      m,
                       StateInfo&                newSt,
                       bool                      givesCheck,
                       const TranspositionTable* tt      = nullptr,
                       const SharedHistories*    history = nullptr) {

    assert(m.is_ok());
    assert(&newSt != st);

    Key k = st->key ^ Zobrist::side;

    // Copy some fields of the old state to our new StateInfo object except the
    // ones which are going to be recalculated from scratch anyway and then switch
    // our state pointer to point to the new (ready to be updated) state.
    std::memcpy(&newSt, st, offsetof(StateInfo, key));
    newSt.previous = st;
    st             = &newSt;

    // Increment ply counters. In particular, rule50 will be reset to zero later on
    // in case of a capture or a pawn move.
    ++gamePly;
    ++st->rule50;
    ++st->pliesFromNull;

    Color  us       = sideToMove;
    Color  them     = ~us;
    Square from     = m.from_sq();
    Square to       = m.to_sq();
    Piece  pc       = piece_on(from);
    Piece  captured = m.type_of() == EN_PASSANT ? make_piece(them, PAWN) : piece_on(to);

    bool checkEP = false;
    //for classical
    assert(color_of(pc) == us);
    assert(captured == NO_PIECE || color_of(captured) == (m.type_of() != CASTLING ? them : us));
    assert(type_of(captured) != KING);

    if (m.type_of() == CASTLING)
    {
        assert(pc == make_piece(us, KING));
        assert(captured == make_piece(us, ROOK));

        Square rfrom, rto;
        do_castling<true>(us, from, to, rfrom, rto);  //for classical

        k ^= Zobrist::psq[captured][rfrom] ^ Zobrist::psq[captured][rto];
        st->nonPawnKey[us] ^= Zobrist::psq[captured][rfrom] ^ Zobrist::psq[captured][rto];
        captured = NO_PIECE;
    }
    else if (captured)
    {
        Square capsq = to;

        // If the captured piece is a pawn, update pawn hash key, otherwise
        // update non-pawn material.
        if (type_of(captured) == PAWN)
        {
            if (m.type_of() == EN_PASSANT)
            {
                capsq -= pawn_push(us);

                assert(pc == make_piece(us, PAWN));
                assert(to == st->epSquare);
                assert(relative_rank(us, to) == RANK_6);
                assert(piece_on(to) == NO_PIECE);
                assert(piece_on(capsq) == make_piece(them, PAWN));

                // Update board and piece lists in ep case, normal captures are updated later
                remove_piece(capsq);
            }

            st->pawnKey ^= Zobrist::psq[captured][capsq];
        }
        else
        {
            st->nonPawnMaterial[them] -= PieceValue[captured];
            st->nonPawnKey[them] ^= Zobrist::psq[captured][capsq];

            if (type_of(captured) <= BISHOP)
                st->minorPieceKey ^= Zobrist::psq[captured][capsq];
        }

        //for classical

        k ^= Zobrist::psq[captured][capsq];
        st->materialKey ^=
          Zobrist::psq[captured][8 + pieceCount[captured] - (m.type_of() != EN_PASSANT)];
        prefetch(thisThread->materialTable[st->materialKey]);  //for classical
        // Reset rule 50 counter
        st->rule50 = 0;
    }
    //for classical

    // Update hash key
    k ^= Zobrist::psq[pc][from] ^ Zobrist::psq[pc][to];

    // Reset en passant square
    if (st->epSquare != SQ_NONE)
    {
        k ^= Zobrist::enpassant[file_of(st->epSquare)];
        st->epSquare = SQ_NONE;
    }

    // Update castling rights if needed
    if (st->castlingRights && (castlingRightsMask[from] | castlingRightsMask[to]))
    {
        k ^= Zobrist::castling[st->castlingRights];
        st->castlingRights &= ~(castlingRightsMask[from] | castlingRightsMask[to]);
        k ^= Zobrist::castling[st->castlingRights];
    }

    // Move the piece. The tricky Chess960 castling is handled earlier
    if (m.type_of() != CASTLING)
    {
        if (captured && m.type_of() != EN_PASSANT)
        {
            remove_piece(from);
            swap_piece(to, pc);
        }
        else
            move_piece(from, to);
    }

    // If the moving piece is a pawn do some special extra work
    if (type_of(pc) == PAWN)
    {
        // Check later if the en passant square needs to be set
        if ((int(to) ^ int(from)) == 16)
            checkEP = true;

        else if (m.type_of() == PROMOTION)
        {
            Piece     promotion     = make_piece(us, m.promotion_type());
            PieceType promotionType = type_of(promotion);

            assert(relative_rank(us, to) == RANK_8);
            assert(type_of(promotion) >= KNIGHT && type_of(promotion) <= QUEEN);

            swap_piece(to, promotion);

            //for classical
            // Update hash keys
            // Zobrist::psq[pc][to] is zero, so we don't need to clear it
            k ^= Zobrist::psq[promotion][to];
            st->materialKey ^= Zobrist::psq[promotion][8 + pieceCount[promotion] - 1]
                             ^ Zobrist::psq[pc][8 + pieceCount[pc]];
            st->nonPawnKey[us] ^= Zobrist::psq[promotion][to];
            if (promotionType <= BISHOP)
                st->minorPieceKey ^= Zobrist::psq[promotion][to];

            // Update material
            st->nonPawnMaterial[us] += PieceValue[promotion];
        }

        // Update pawn hash key
        st->pawnKey ^= Zobrist::psq[pc][from] ^ Zobrist::psq[pc][to];

        // Reset rule 50 draw counter
        st->rule50 = 0;
    }

    else
    {
        st->nonPawnKey[us] ^= Zobrist::psq[pc][from] ^ Zobrist::psq[pc][to];

        if (type_of(pc) <= BISHOP)
            st->minorPieceKey ^= Zobrist::psq[pc][from] ^ Zobrist::psq[pc][to];
    }

    // If en passant is impossible, then k will not change and we can prefetch earlier
    if (tt && !checkEP)
        prefetch(tt->first_entry(adjust_key50(k)));

    if (history)
    {
        prefetch(&history->pawn_entry(*this)[pc][to]);
        prefetch(&history->pawn_correction_entry(*this));
        prefetch(&history->minor_piece_correction_entry(*this));
        prefetch(&history->nonpawn_correction_entry<WHITE>(*this));
        prefetch(&history->nonpawn_correction_entry<BLACK>(*this));
    }

    // Set capture piece
    st->capturedPiece = captured;

    // Calculate checkers bitboard (if move gives check)
    st->checkersBB = givesCheck ? attackers_to(square<KING>(them)) & pieces(us) : 0;

    sideToMove = ~sideToMove;

    // Update king attacks used for fast check detection
    set_check_info();

    // Accurate e.p. info is needed for correct zobrist key generation and 3-fold checking
    while (checkEP)
    {
        auto updateEpSquare = [&] {
            st->epSquare = to - pawn_push(us);
            k ^= Zobrist::enpassant[file_of(st->epSquare)];
        };

        Bitboard pawns = attacks_bb<PAWN>(to - pawn_push(us), us) & pieces(them, PAWN);

        // If there are no pawns attacking the ep square, ep is not possible
        if (!pawns)
            break;

        // If there are checkers other than the to be captured pawn, ep is never legal
        if (checkers() & ~square_bb(to))
            break;

        if (more_than_one(pawns))
        {
            // If there are two pawns potentially being able to capture and at least one
            // is not pinned, ep is legal as there are no horizontal exposed checks
            if (!more_than_one(blockers_for_king(them) & pawns))
            {
                updateEpSquare();
                break;
            }

            // If there is no pawn on our king's file, and thus both pawns are pinned
            // by bishops, ep is not legal as the king square must be in front of the to square.
            // And because the ep square and the king are not on a common diagonal, either ep capture
            // would expose the king to a check from one of the bishops
            if (!(file_bb(square<KING>(them)) & pawns))
                break;

            // Otherwise remove the pawn on the king file, as an ep capture by it can never be legal and the
            // check below relies on there only being one pawn
            pawns &= ~file_bb(square<KING>(them));
        }

        Square   ksq      = square<KING>(them);
        Square   capsq    = to;
        Bitboard occupied = (pieces() ^ lsb(pawns) ^ capsq) | (to - pawn_push(us));

        // If our king is not attacked after making the move, ep is legal.
        if (!(attacks_bb<ROOK>(ksq, occupied) & pieces(us, QUEEN, ROOK))
            && !(attacks_bb<BISHOP>(ksq, occupied) & pieces(us, QUEEN, BISHOP)))
            updateEpSquare();

        break;
    }

    // Update the key with the final value
    st->key = k;
    if (tt)
        prefetch(tt->first_entry(key()));

    // Calculate the repetition info. It is the ply distance from the previous
    // occurrence of the same position, negative in the 3-fold case, or zero
    // if the position was not repeated.
    st->repetition = 0;
    int end        = std::min(st->rule50, st->pliesFromNull);
    if (end >= 4)
    {
        StateInfo* stp = st->previous->previous;
        for (int i = 4; i <= end; i += 2)
        {
            stp = stp->previous->previous;
            if (stp->key == st->key)
            {
                st->repetition = stp->repetition ? -i : i;
                break;
            }
        }
    }

    //for classical
    assert(pos_is_ok());
    //for classical
}


// Unmakes a move. When it returns, the position should
// be restored to exactly the same state as before the move was made.
void Position::undo_move(Move m) {

    assert(m.is_ok());

    sideToMove = ~sideToMove;

    Color  us   = sideToMove;
    Square from = m.from_sq();
    Square to   = m.to_sq();
    Piece  pc   = piece_on(to);

    assert(empty(from) || m.type_of() == CASTLING);
    assert(type_of(st->capturedPiece) != KING);

    if (m.type_of() == PROMOTION)
    {
        assert(relative_rank(us, to) == RANK_8);
        assert(type_of(pc) == m.promotion_type());
        assert(type_of(pc) >= KNIGHT && type_of(pc) <= QUEEN);

        remove_piece(to);
        pc = make_piece(us, PAWN);
        put_piece(pc, to);
    }

    if (m.type_of() == CASTLING)
    {
        Square rfrom, rto;
        do_castling<false>(us, from, to, rfrom, rto);
    }
    else
    {
        move_piece(to, from);  // Put the piece back at the source square

        if (st->capturedPiece)
        {
            Square capsq = to;

            if (m.type_of() == EN_PASSANT)
            {
                capsq -= pawn_push(us);

                assert(type_of(pc) == PAWN);
                assert(to == st->previous->epSquare);
                assert(relative_rank(us, to) == RANK_6);
                assert(piece_on(capsq) == NO_PIECE);
                assert(st->capturedPiece == make_piece(~us, PAWN));
            }

            put_piece(st->capturedPiece, capsq);  // Restore the captured piece
        }
    }

    // Finally point our state pointer back to the previous state
    st = st->previous;
    --gamePly;

    assert(pos_is_ok());
}

//for classical

// Helper used to do/undo a castling move. This is a bit
// tricky in Chess960 where from/to squares can overlap.
template<bool Do>
void Position::do_castling(
  Color us, Square from, Square& to, Square& rfrom, Square& rto) {  //for classical

    bool kingSide = to > from;
    rfrom         = to;  // Castling is encoded as "king captures friendly rook"
    rto           = relative_square(us, kingSide ? SQ_F1 : SQ_D1);
    to            = relative_square(us, kingSide ? SQ_G1 : SQ_C1);
    //for classical
    // Remove both pieces first since squares could overlap in Chess960
    remove_piece(Do ? from : to);
    remove_piece(Do ? rfrom : rto);
    put_piece(make_piece(us, KING), Do ? to : from);
    put_piece(make_piece(us, ROOK), Do ? rto : rfrom);
}


// Used to do a "null move": it flips
// the side to move without executing any move on the board.
void Position::do_null_move(StateInfo& newSt, const TranspositionTable& tt) {

    assert(!checkers());
    assert(&newSt != st);

    std::memcpy(&newSt, st, sizeof(StateInfo));

    newSt.previous = st;
    st             = &newSt;

    if (st->epSquare != SQ_NONE)
    {
        st->key ^= Zobrist::enpassant[file_of(st->epSquare)];
        st->epSquare = SQ_NONE;
    }

    st->key ^= Zobrist::side;
    prefetch(tt.first_entry(key()));

    st->pliesFromNull = 0;

    sideToMove = ~sideToMove;

    set_check_info();

    st->repetition = 0;

    assert(pos_is_ok());
}


// Must be used to undo a "null move"
void Position::undo_null_move() {

    assert(!checkers());

    st         = st->previous;
    sideToMove = ~sideToMove;
}


// Tests if the SEE (Static Exchange Evaluation)
// value of move is greater or equal to the given threshold. We'll use an
// algorithm similar to alpha-beta pruning with a null window.
bool Position::see_ge(Move m, int threshold) const {

    assert(m.is_ok());

    // Only deal with normal moves, assume others pass a simple SEE
    if (m.type_of() != NORMAL)
        return VALUE_ZERO >= threshold;

    Square from = m.from_sq(), to = m.to_sq();

    assert(piece_on(from) != NO_PIECE);

    int swap = PieceValue[piece_on(to)] - threshold;
    if (swap < 0)
        return false;

    swap = PieceValue[piece_on(from)] - swap;
    if (swap <= 0)
        return true;

    assert(color_of(piece_on(from)) == sideToMove);
    Bitboard occupied  = pieces() ^ from ^ to;  // xoring to is important for pinned piece logic
    Color    stm       = sideToMove;
    Bitboard attackers = attackers_to(to, occupied);
    Bitboard stmAttackers, bb;
    int      res = 1;

    while (true)
    {
        stm = ~stm;
        attackers &= occupied;

        // If stm has no more attackers then give up: stm loses
        if (!(stmAttackers = attackers & pieces(stm)))
            break;

        // Don't allow pinned pieces to attack as long as there are
        // pinners on their original square.
        if (pinners(~stm) & occupied)
        {
            stmAttackers &= ~blockers_for_king(stm);

            if (!stmAttackers)
                break;
        }

        res ^= 1;

        // Locate and remove the next least valuable attacker, and add to
        // the bitboard 'attackers' any X-ray attackers behind it.
        if ((bb = stmAttackers & pieces(PAWN)))
        {
            if ((swap = PawnValue - swap) < res)
                break;
            occupied ^= least_significant_square_bb(bb);

            attackers |= attacks_bb<BISHOP>(to, occupied) & pieces(BISHOP, QUEEN);
        }

        else if ((bb = stmAttackers & pieces(KNIGHT)))
        {
            if ((swap = KnightValue - swap) < res)
                break;
            occupied ^= least_significant_square_bb(bb);
        }

        else if ((bb = stmAttackers & pieces(BISHOP)))
        {
            if ((swap = BishopValue - swap) < res)
                break;
            occupied ^= least_significant_square_bb(bb);

            attackers |= attacks_bb<BISHOP>(to, occupied) & pieces(BISHOP, QUEEN);
        }

        else if ((bb = stmAttackers & pieces(ROOK)))
        {
            if ((swap = RookValue - swap) < res)
                break;
            occupied ^= least_significant_square_bb(bb);

            attackers |= attacks_bb<ROOK>(to, occupied) & pieces(ROOK, QUEEN);
        }

        else if ((bb = stmAttackers & pieces(QUEEN)))
        {
            swap = QueenValue - swap;
            //  implies that the previous recapture was done by a higher rated piece than a Queen (King is excluded)
            assert(swap >= res);
            occupied ^= least_significant_square_bb(bb);

            attackers |= (attacks_bb<BISHOP>(to, occupied) & pieces(BISHOP, QUEEN))
                       | (attacks_bb<ROOK>(to, occupied) & pieces(ROOK, QUEEN));
        }

        else  // KING
              // If we "capture" with the king but the opponent still has attackers,
              // reverse the result.
            return (attackers & ~pieces(stm)) ? res ^ 1 : res;
    }

    return bool(res);
}

// Tests whether the position is drawn by 50-move rule
// or by repetition. It does not detect stalemates.
bool Position::is_draw(int ply) const {

    if (st->rule50 > 99 && (!checkers() || MoveList<LEGAL>(*this).size()))
        return true;

    return is_repetition(ply);
}

// Return a draw score if a position repeats once earlier but strictly
// after the root, or repeats twice before or at the root.
bool Position::is_repetition(int ply) const { return st->repetition && st->repetition < ply; }

// Tests whether there has been at least one repetition
// of positions since the last capture or pawn move.
bool Position::has_repeated() const {

    StateInfo* stc = st;
    int        end = std::min(st->rule50, st->pliesFromNull);
    while (end-- >= 4)
    {
        if (stc->repetition)
            return true;

        stc = stc->previous;
    }
    return false;
}


// Tests if the position has a move which draws by repetition.
// This function accurately matches the outcome of is_draw() over all legal moves.
bool Position::upcoming_repetition(int ply) const {

    int j;

    int end = std::min(st->rule50, st->pliesFromNull);

    if (end < 3)
        return false;

    Key        originalKey = st->key;
    StateInfo* stp         = st->previous;
    Key        other       = originalKey ^ stp->key ^ Zobrist::side;

    for (int i = 3; i <= end; i += 2)
    {
        stp = stp->previous;
        other ^= stp->key ^ stp->previous->key ^ Zobrist::side;
        stp = stp->previous;

        if (other != 0)
            continue;

        Key moveKey = originalKey ^ stp->key;
        if ((j = H1(moveKey), cuckoo[j] == moveKey) || (j = H2(moveKey), cuckoo[j] == moveKey))
        {
            Move   move = cuckooMove[j];
            Square s1   = move.from_sq();
            Square s2   = move.to_sq();

            if (!((between_bb(s1, s2) ^ s2) & pieces()))
            {
                if (ply > i)
                    return true;

                // For nodes before or at the root, check that the move is a
                // repetition rather than a move to the current position.
                if (stp->repetition)
                    return true;
            }
        }
    }
    return false;
}


// Flips position with the white and black sides reversed. This
// is only useful for debugging e.g. for finding evaluation symmetry bugs.
void Position::flip() {

    string            f, token;
    std::stringstream ss(fen());

    for (Rank r = RANK_8;; --r)  // Piece placement
    {
        std::getline(ss, token, r > RANK_1 ? '/' : ' ');
        f.insert(0, token + (f.empty() ? " " : "/"));

        if (r == RANK_1)
            break;
    }

    ss >> token;                        // Active color
    f += (token == "w" ? "B " : "W ");  // Will be lowercased later

    ss >> token;  // Castling availability
    f += token + " ";

    std::transform(f.begin(), f.end(), f.begin(),
                   [](char c) { return char(islower(c) ? toupper(c) : tolower(c)); });

    ss >> token;  // En passant square
    f += (token == "-" ? token : token.replace(1, 1, token[1] == '3' ? "6" : "3"));

    std::getline(ss, token);  // Half and full moves
    f += token;

    set(f, is_chess960(), st, this_thread());  //for classical

    assert(pos_is_ok());
}


bool Position::material_key_is_ok() const { return compute_material_key() == st->materialKey; }


// Performs some consistency checks for the position object
// and raise an assert if something wrong is detected.
// This is meant to be helpful when debugging.
bool Position::pos_is_ok() const {

    constexpr bool Fast = true;  // Quick (default) or full check?

    if ((sideToMove != WHITE && sideToMove != BLACK) || piece_on(square<KING>(WHITE)) != W_KING
        || piece_on(square<KING>(BLACK)) != B_KING
        || (ep_square() != SQ_NONE && relative_rank(sideToMove, ep_square()) != RANK_6))
        assert(0 && "pos_is_ok: Default");

    if (Fast)
        return true;

    if (pieceCount[W_KING] != 1 || pieceCount[B_KING] != 1
        || attackers_to_exist(square<KING>(~sideToMove), pieces(), sideToMove))
        assert(0 && "pos_is_ok: Kings");

    if ((pieces(PAWN) & (Rank1BB | Rank8BB)) || pieceCount[W_PAWN] > 8 || pieceCount[B_PAWN] > 8)
        assert(0 && "pos_is_ok: Pawns");

    if ((pieces(WHITE) & pieces(BLACK)) || (pieces(WHITE) | pieces(BLACK)) != pieces()
        || popcount(pieces(WHITE)) > 16 || popcount(pieces(BLACK)) > 16)
        assert(0 && "pos_is_ok: Bitboards");

    for (PieceType p1 = PAWN; p1 <= KING; ++p1)
        for (PieceType p2 = PAWN; p2 <= KING; ++p2)
            if (p1 != p2 && (pieces(p1) & pieces(p2)))
                assert(0 && "pos_is_ok: Bitboards");


    for (Piece pc : Pieces)
        if (pieceCount[pc] != popcount(pieces(color_of(pc), type_of(pc)))
            || pieceCount[pc] != std::count(board.begin(), board.end(), pc))
            assert(0 && "pos_is_ok: Pieces");

    for (Color c : {WHITE, BLACK})
        for (CastlingRights cr : {c & KING_SIDE, c & QUEEN_SIDE})
        {
            if (!can_castle(cr))
                continue;

            if (piece_on(castlingRookSquare[cr]) != make_piece(c, ROOK)
                || castlingRightsMask[castlingRookSquare[cr]] != cr
                || (castlingRightsMask[square<KING>(c)] & cr) != cr)
                assert(0 && "pos_is_ok: Castling");
        }

    assert(material_key_is_ok() && "pos_is_ok: materialKey");

    return true;
}

}  // namespace Alexander

// ============================================================================
// FILE ORIGINALE: position.h
// ============================================================================

/*
  Alexander, a UCI chess playing engine derived from Stockfish
  Copyright (C) 2004-2026 Andrea Manzo, F. Ferraguti, K.Kiniama and Alexander developers (see AUTHORS file)

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

#ifndef POSITION_H_INCLUDED
#define POSITION_H_INCLUDED

#include <array>
#include <cassert>
#include <deque>
#include <iosfwd>
#include <memory>
#include <new>
#include <string>

#include "bitboard.h"
#include "types.h"
#include "psqt.h"     //for classical
#include "movegen.h"  //shashin

namespace Alexander {
//learning begin
extern void setStartPoint();
//learning end
struct SharedHistories;
class TranspositionTable;

// StateInfo struct stores information needed to restore a Position object to
// its previous state when we retract a move. Whenever a move is made on the
// board (by calling Position::do_move), a StateInfo object must be passed.

struct StateInfo {

    // Copied when making a move
    Key    materialKey;
    Key    pawnKey;
    Key    minorPieceKey;
    Key    nonPawnKey[COLOR_NB];
    Value  nonPawnMaterial[COLOR_NB];
    int    castlingRights;
    int    rule50;
    int    pliesFromNull;
    Square epSquare;

    // Not copied when making a move (will be recomputed anyhow)
    Key        key;
    Bitboard   checkersBB;
    StateInfo* previous;
    Bitboard   blockersForKing[COLOR_NB];
    Bitboard   pinners[COLOR_NB];
    Bitboard   checkSquares[PIECE_TYPE_NB];
    Piece      capturedPiece;
    int        repetition;
};


// A list to keep track of the position states along the setup moves (from the
// start position to the position just before the search starts). Needed by
// 'draw by repetition' detection. Use a std::deque because pointers to
// elements are not invalidated upon list resizing.
using StateListPtr = std::unique_ptr<std::deque<StateInfo>>;


// Position class stores information regarding the board representation as
// pieces, side to move, hash keys, castling info, etc. Important methods are
// do_move() and undo_move(), used by the search to update node info when
// traversing the search tree.
class Thread;  //for classical
class Position {
   public:
    static void init();

    Position()                           = default;
    Position(const Position&)            = delete;
    Position& operator=(const Position&) = delete;

    // FEN string input/output
    Position&
    set(const std::string& fenStr, bool isChess960, StateInfo* si, Thread* th);  //for classical
    Position&   set(const std::string& code, Color c, StateInfo* si);
    std::string fen() const;

    // Position representation
    Bitboard pieces() const;  // All pieces
    template<typename... PieceTypes>
    Bitboard pieces(PieceTypes... pts) const;
    Bitboard pieces(Color c) const;
    template<typename... PieceTypes>
    Bitboard                            pieces(Color c, PieceTypes... pts) const;
    Piece                               piece_on(Square s) const;
    const std::array<Piece, SQUARE_NB>& piece_array() const;
    Square                              ep_square() const;
    bool                                empty(Square s) const;
    template<PieceType Pt>
    int count(Color c) const;
    template<PieceType Pt>
    int count() const;
    template<PieceType Pt>
    Square square(Color c) const;
    bool   is_on_semiopen_file(Color c, Square s) const;  //for classical

    // Castling
    CastlingRights castling_rights(Color c) const;  //for classical
    bool           can_castle(CastlingRights cr) const;
    bool           castling_impeded(CastlingRights cr) const;
    Square         castling_rook_square(CastlingRights cr) const;

    // Checking
    Bitboard checkers() const;
    Bitboard blockers_for_king(Color c) const;
    Bitboard check_squares(PieceType pt) const;
    Bitboard pinners(Color c) const;

    // Attacks to/from a given square
    Bitboard attackers_to(Square s) const;
    Bitboard attackers_to(Square s, Bitboard occupied) const;
    bool     attackers_to_exist(Square s, Bitboard occupied, Color c) const;
    void     update_slider_blockers(Color c) const;
    Bitboard slider_blockers(Bitboard sliders, Square s, Bitboard& pinners) const;  //for classical
    template<PieceType Pt>
    Bitboard attacks_by(Color c) const;

    // Properties of moves
    bool  legal(Move m) const;
    bool  pseudo_legal(const Move m) const;
    bool  capture(Move m) const;
    bool  capture_stage(Move m) const;
    bool  gives_check(Move m) const;
    Piece moved_piece(Move m) const;
    Piece captured_piece() const;
    //for classical begin
    // Piece specific
    bool pawn_passed(Color c, Square s) const;
    bool opposite_bishops() const;
    int  pawns_on_same_color_squares(Color c, Square s) const;
    //for classical end
    // Doing and undoing moves
    void do_move(Move m, StateInfo& newSt, const TranspositionTable* tt);
    void do_move(Move                      m,
                 StateInfo&                newSt,
                 bool                      givesCheck,
                 const TranspositionTable* tt,
                 const SharedHistories*    worker);  //for classical
    void undo_move(Move m);
    void do_null_move(StateInfo& newSt, const TranspositionTable& tt);
    void undo_null_move();

    // Static Exchange Evaluation
    bool see_ge(Move m, int threshold = 0) const;

    // Accessing hash keys
    Key key() const;
    Key material_key() const;
    Key pawn_key() const;
    Key minor_piece_key() const;
    Key non_pawn_key(Color c) const;

    // Other properties of the position
    Color side_to_move() const;
    int   game_ply() const;
    bool  is_chess960() const;
    bool  is_draw(int ply) const;
    bool  is_repetition(int ply) const;
    bool  upcoming_repetition(int ply) const;
    bool  has_repeated() const;
    //for classical begin
    Thread*           this_thread() const;
    ScoreForClassical psq_score() const;
    //for classical end
    int   rule50_count() const;
    Value non_pawn_material(Color c) const;
    Value non_pawn_material() const;

    // Position consistency check, for debugging
    bool pos_is_ok() const;
    bool material_key_is_ok() const;
    void flip();

    StateInfo* state() const;

    void put_piece(Piece pc, Square s);   //for classical
    void remove_piece(Square s);          //for classical
    void swap_piece(Square s, Piece pc);  //for classical

   private:
    // Initialization helpers (used while setting up a position)
    void set_castling_right(Color c, Square rfrom);
    Key  compute_material_key() const;
    void set_state() const;
    void set_check_info() const;

    // Other helpers
    template<bool PutPiece, bool ComputeRay = true>
    void update_piece_threats(Piece    pc,
                              Square   s,
                              Bitboard noRaysContaining = -1ULL) const;  //for classical
    void move_piece(Square from, Square to);                             //for classical
    template<bool Do>
    void
        do_castling(Color us, Square from, Square& to, Square& rfrom, Square& rto);  //for classical
    Key adjust_key50(Key k) const;

    // Data members
    std::array<Piece, SQUARE_NB>        board;
    std::array<Bitboard, PIECE_TYPE_NB> byTypeBB;
    std::array<Bitboard, COLOR_NB>      byColorBB;
    int                                 pieceCount[PIECE_NB];
    int                                 castlingRightsMask[SQUARE_NB];
    Square                              castlingRookSquare[CASTLING_RIGHT_NB];
    Bitboard                            castlingPath[CASTLING_RIGHT_NB];
    Thread*                             thisThread;  //for classical
    StateInfo*                          st;
    int                                 gamePly;
    Color                               sideToMove;
    ScoreForClassical                   psq;  //for classical
    bool                                chess960;
    //for classical
};
extern void   putQLearningTrajectoryIntoLearningTable();  //learning
std::ostream& operator<<(std::ostream& os, const Position& pos);

inline Color Position::side_to_move() const { return sideToMove; }

inline Piece Position::piece_on(Square s) const {
    assert(is_ok(s));
    return board[s];
}

inline const std::array<Piece, SQUARE_NB>& Position::piece_array() const { return board; }

inline bool Position::empty(Square s) const { return piece_on(s) == NO_PIECE; }

inline Piece Position::moved_piece(Move m) const { return piece_on(m.from_sq()); }

inline Bitboard Position::pieces() const { return byTypeBB[ALL_PIECES]; }

template<typename... PieceTypes>
inline Bitboard Position::pieces(PieceTypes... pts) const {
    return (byTypeBB[pts] | ...);
}

inline Bitboard Position::pieces(Color c) const { return byColorBB[c]; }

template<typename... PieceTypes>
inline Bitboard Position::pieces(Color c, PieceTypes... pts) const {
    return pieces(c) & pieces(pts...);
}

template<PieceType Pt>
inline int Position::count(Color c) const {
    return pieceCount[make_piece(c, Pt)];
}

template<PieceType Pt>
inline int Position::count() const {
    return count<Pt>(WHITE) + count<Pt>(BLACK);
}

template<PieceType Pt>
inline Square Position::square(Color c) const {
    assert(count<Pt>(c) == 1);
    return lsb(pieces(c, Pt));
}

inline Square Position::ep_square() const { return st->epSquare; }
//for classical begin
inline bool Position::is_on_semiopen_file(Color c, Square s) const {
    return !(pieces(c, PAWN) & file_bb(s));
}
//for classical end
inline bool Position::can_castle(CastlingRights cr) const { return st->castlingRights & cr; }

//for classical begin
inline CastlingRights Position::castling_rights(Color c) const {
    return c & CastlingRights(st->castlingRights);
}
//for classical end

inline bool Position::castling_impeded(CastlingRights cr) const {
    assert(cr == WHITE_OO || cr == WHITE_OOO || cr == BLACK_OO || cr == BLACK_OOO);
    return pieces() & castlingPath[cr];
}

inline Square Position::castling_rook_square(CastlingRights cr) const {
    assert(cr == WHITE_OO || cr == WHITE_OOO || cr == BLACK_OO || cr == BLACK_OOO);
    return castlingRookSquare[cr];
}

inline Bitboard Position::attackers_to(Square s) const { return attackers_to(s, pieces()); }

template<PieceType Pt>
inline Bitboard Position::attacks_by(Color c) const {

    if constexpr (Pt == PAWN)
        return c == WHITE ? pawn_attacks_bb<WHITE>(pieces(WHITE, PAWN))
                          : pawn_attacks_bb<BLACK>(pieces(BLACK, PAWN));
    else
    {
        Bitboard threats   = 0;
        Bitboard attackers = pieces(c, Pt);
        while (attackers)
            threats |= attacks_bb<Pt>(pop_lsb(attackers), pieces());
        return threats;
    }
}

inline Bitboard Position::checkers() const { return st->checkersBB; }

inline Bitboard Position::blockers_for_king(Color c) const { return st->blockersForKing[c]; }

inline Bitboard Position::pinners(Color c) const { return st->pinners[c]; }

inline Bitboard Position::check_squares(PieceType pt) const { return st->checkSquares[pt]; }
//for classical begin
inline bool Position::pawn_passed(Color c, Square s) const {
    return !(pieces(~c, PAWN) & passed_pawn_span(c, s));
}

inline int Position::pawns_on_same_color_squares(Color c, Square s) const {
    return popcount(pieces(c, PAWN) & ((DarkSquares & s) ? DarkSquares : ~DarkSquares));
}
//for classical end
inline Key Position::key() const { return adjust_key50(st->key); }
inline Key Position::adjust_key50(Key k) const {
    return st->rule50 < 14 ? k : k ^ make_key((st->rule50 - 14) / 8);
}

inline Key Position::pawn_key() const { return st->pawnKey; }

inline Key Position::material_key() const { return st->materialKey; }

inline ScoreForClassical Position::psq_score() const { return psq; }  //for classical

inline Key Position::minor_piece_key() const { return st->minorPieceKey; }

inline Key Position::non_pawn_key(Color c) const { return st->nonPawnKey[c]; }

inline Value Position::non_pawn_material(Color c) const { return st->nonPawnMaterial[c]; }

inline Value Position::non_pawn_material() const {
    return non_pawn_material(WHITE) + non_pawn_material(BLACK);
}

inline int Position::game_ply() const { return gamePly; }

inline int Position::rule50_count() const { return st->rule50; }
//for classical begin
inline bool Position::opposite_bishops() const {
    return count<BISHOP>(WHITE) == 1 && count<BISHOP>(BLACK) == 1
        && opposite_colors(square<BISHOP>(WHITE), square<BISHOP>(BLACK));
}
//for classical end
inline bool Position::is_chess960() const { return chess960; }

inline bool Position::capture(Move m) const {
    assert(m.is_ok());
    return (!empty(m.to_sq()) && m.type_of() != CASTLING) || m.type_of() == EN_PASSANT;
}

// Returns true if a move is generated from the capture stage, having also
// queen promotions covered, i.e. consistency with the capture stage move
// generation is needed to avoid the generation of duplicate moves.
inline bool Position::capture_stage(Move m) const {
    assert(m.is_ok());
    return capture(m) || m.promotion_type() == QUEEN;
}

inline Piece Position::captured_piece() const { return st->capturedPiece; }

inline Thread* Position::this_thread() const { return thisThread; }  //for classical

inline void Position::put_piece(Piece pc, Square s) {  //for classical

    board[s] = pc;
    byTypeBB[ALL_PIECES] |= byTypeBB[type_of(pc)] |= s;
    byColorBB[color_of(pc)] |= s;
    pieceCount[pc]++;
    pieceCount[make_piece(color_of(pc), ALL_PIECES)]++;
    psq += PSQT::psq[pc][s];  //for classical
}

inline void Position::remove_piece(Square s) {

    Piece pc = board[s];
    byTypeBB[ALL_PIECES] ^= s;
    byTypeBB[type_of(pc)] ^= s;
    byColorBB[color_of(pc)] ^= s;
    board[s] = NO_PIECE;
    pieceCount[pc]--;
    pieceCount[make_piece(color_of(pc), ALL_PIECES)]--;
    psq -= PSQT::psq[pc][s];  //for classical
}

inline void Position::move_piece(Square from, Square to) {

    Piece    pc     = board[from];
    Bitboard fromTo = from | to;
    byTypeBB[ALL_PIECES] ^= fromTo;
    byTypeBB[type_of(pc)] ^= fromTo;
    byColorBB[color_of(pc)] ^= fromTo;
    board[from] = NO_PIECE;
    board[to]   = pc;
    psq += PSQT::psq[pc][to] - PSQT::psq[pc][from];  //for classical
}

inline void Position::swap_piece(Square s, Piece pc) {  //for classical
                                                        //no classical

    remove_piece(s);

    put_piece(pc, s);
}

inline void Position::do_move(Move m, StateInfo& newSt, const TranspositionTable* tt = nullptr) {
    do_move(m, newSt, gives_check(m), tt, nullptr);
}

inline StateInfo* Position::state() const { return st; }

}  // namespace Alexander

#endif  // #ifndef POSITION_H_INCLUDED


// ============================================================================
// FILE ORIGINALE: psqt.cpp
// ============================================================================

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


#include "psqt.h"

#include <algorithm>

#include "bitboard.h"
#include "types.h"

namespace Alexander {

namespace {

auto constexpr S = make_score;

// 'Bonus' contains Piece-Square parameters.
// Scores are explicit for files A to D, implicitly mirrored for E to H.
constexpr ScoreForClassical Bonus[][RANK_NB][int(FILE_NB) / 2] = {
  {},
  {},
  {// Knight
   {S(-175, -96), S(-92, -65), S(-74, -49), S(-73, -21)},
   {S(-77, -67), S(-41, -54), S(-27, -18), S(-15, 8)},
   {S(-61, -40), S(-17, -27), S(6, -8), S(12, 29)},
   {S(-35, -35), S(8, -2), S(40, 13), S(49, 28)},
   {S(-34, -45), S(13, -16), S(44, 9), S(51, 39)},
   {S(-9, -51), S(22, -44), S(58, -16), S(53, 17)},
   {S(-67, -69), S(-27, -50), S(4, -51), S(37, 12)},
   {S(-201, -100), S(-83, -88), S(-56, -56), S(-26, -17)}},
  {// Bishop
   {S(-37, -40), S(-4, -21), S(-6, -26), S(-16, -8)},
   {S(-11, -26), S(6, -9), S(13, -12), S(3, 1)},
   {S(-5, -11), S(15, -1), S(-4, -1), S(12, 7)},
   {S(-4, -14), S(8, -4), S(18, 0), S(27, 12)},
   {S(-8, -12), S(20, -1), S(15, -10), S(22, 11)},
   {S(-11, -21), S(4, 4), S(1, 3), S(8, 4)},
   {S(-12, -22), S(-10, -14), S(4, -1), S(0, 1)},
   {S(-34, -32), S(1, -29), S(-10, -26), S(-16, -17)}},
  {// Rook
   {S(-31, -9), S(-20, -13), S(-14, -10), S(-5, -9)},
   {S(-21, -12), S(-13, -9), S(-8, -1), S(6, -2)},
   {S(-25, 6), S(-11, -8), S(-1, -2), S(3, -6)},
   {S(-13, -6), S(-5, 1), S(-4, -9), S(-6, 7)},
   {S(-27, -5), S(-15, 8), S(-4, 7), S(3, -6)},
   {S(-22, 6), S(-2, 1), S(6, -7), S(12, 10)},
   {S(-2, 4), S(12, 5), S(16, 20), S(18, -5)},
   {S(-17, 18), S(-19, 0), S(-1, 19), S(9, 13)}},
  {// Queen
   {S(3, -69), S(-5, -57), S(-5, -47), S(4, -26)},
   {S(-3, -54), S(5, -31), S(8, -22), S(12, -4)},
   {S(-3, -39), S(6, -18), S(13, -9), S(7, 3)},
   {S(4, -23), S(5, -3), S(9, 13), S(8, 24)},
   {S(0, -29), S(14, -6), S(12, 9), S(5, 21)},
   {S(-4, -38), S(10, -18), S(6, -11), S(8, 1)},
   {S(-5, -50), S(6, -27), S(10, -24), S(8, -8)},
   {S(-2, -74), S(-2, -52), S(1, -43), S(-2, -34)}},
  {// King
   {S(271, 1), S(327, 45), S(271, 85), S(198, 76)},
   {S(278, 53), S(303, 100), S(234, 133), S(179, 135)},
   {S(195, 88), S(258, 130), S(169, 169), S(120, 175)},
   {S(164, 103), S(190, 156), S(138, 172), S(98, 172)},
   {S(154, 96), S(179, 166), S(105, 199), S(70, 199)},
   {S(123, 92), S(145, 172), S(81, 184), S(31, 191)},
   {S(88, 47), S(120, 121), S(65, 116), S(33, 131)},
   {S(59, 11), S(89, 59), S(45, 73), S(-1, 78)}}};

constexpr ScoreForClassical PBonus[RANK_NB][FILE_NB] = {  // Pawn (asymmetric distribution)
  {},
  {S(2, -8), S(4, -6), S(11, 9), S(18, 5), S(16, 16), S(21, 6), S(9, -6), S(-3, -18)},
  {S(-9, -9), S(-15, -7), S(11, -10), S(15, 5), S(31, 2), S(23, 3), S(6, -8), S(-20, -5)},
  {S(-3, 7), S(-20, 1), S(8, -8), S(19, -2), S(39, -14), S(17, -13), S(2, -11), S(-5, -6)},
  {S(11, 12), S(-4, 6), S(-11, 2), S(2, -6), S(11, -5), S(0, -4), S(-12, 14), S(5, 9)},
  {S(3, 27), S(-11, 18), S(-6, 19), S(22, 29), S(-8, 30), S(-5, 9), S(-14, 8), S(-11, 14)},
  {S(-7, -1), S(6, -14), S(-2, 13), S(-11, 22), S(4, 24), S(-14, 17), S(10, 7), S(-9, 7)}};

}  // namespace


namespace PSQT {

ScoreForClassical psq[PIECE_NB][SQUARE_NB];

// PSQT::init() initializes piece-square tables: the white halves of the tables are
// copied from Bonus[] and PBonus[], adding the piece value, then the black halves of
// the tables are initialized by flipping and changing the sign of the white scores.
void init() {

    for (Piece pc : {W_PAWN, W_KNIGHT, W_BISHOP, W_ROOK, W_QUEEN, W_KING})
    {
        ScoreForClassical score = make_score(PieceValueForPSQT[MG][pc], PieceValueForPSQT[EG][pc]);

        for (Square s = SQ_A1; s <= SQ_H8; ++s)
        {
            File f = File(edge_distance(file_of(s)));
            psq[pc][s] =
              score
              + (type_of(pc) == PAWN ? PBonus[rank_of(s)][file_of(s)] : Bonus[pc][rank_of(s)][f]);
            psq[~pc][flip_rank(s)] = -psq[pc][s];
        }
    }
}

}  // namespace PSQT

}  // namespace Alexander


// ============================================================================
// FILE ORIGINALE: psqt.h
// ============================================================================

/*
  Alexander, a UCI chess playing engine derived from Stockfish
  Copyright (C) 2004-2023 The Alexander developers (see AUTHORS file)

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


#ifndef PSQT_H_INCLUDED
#define PSQT_H_INCLUDED


#include "types.h"


namespace Alexander::PSQT {

extern ScoreForClassical psq[PIECE_NB][SQUARE_NB];

// Fill psqt array from a set of internally linked parameters
void init();

}  // namespace Alexander::PSQT


#endif  // PSQT_H_INCLUDED


// ============================================================================
// FILE ORIGINALE: score.cpp
// ============================================================================

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

#include "score.h"

#include <cassert>
#include <cmath>
#include <cstdlib>

#include "uci.h"

namespace Alexander {

Score::Score(Value v, const Position& pos) {
    assert(-VALUE_INFINITE < v && v < VALUE_INFINITE);

    if (!is_decisive(v))
    {
        score = InternalUnits{UCIEngine::to_cp(v, pos)};
    }
    else if (std::abs(v) <= VALUE_TB)
    {
        auto distance = VALUE_TB - std::abs(v);
        score         = (v > 0) ? Tablebase{distance, true} : Tablebase{-distance, false};
    }
    else
    {
        auto distance = VALUE_MATE - std::abs(v);
        score         = (v > 0) ? Mate{distance} : Mate{-distance};
    }
}

}

// ============================================================================
// FILE ORIGINALE: score.h
// ============================================================================

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


// ============================================================================
// FILE ORIGINALE: search.cpp
// ============================================================================

/*
  Alexander, a UCI chess playing engine derived from Stockfish
  Copyright (C) 2004-2026 Alexander developers (see AUTHORS file)

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
/*
search.cpp is the heart of the modifications for Shashin, as this is where the heuristics are applied and the engine's behavior during search is guided.
We can integrate information from Shashin in a targeted way to influence:
Pruning and search rationalization: Deciding which branches to explore based on the Shashin area.
Heuristics for move selection: Improve move prioritization based on tactical or strategic context.
MCTS: Intelligently activate or avoid it.
Local assessments: Penalize or reward based on king's security, tactical complexity, etc.
*/
#include "search.h"
//variety begin
#include <random>
#include <chrono>
#include <cstdint>
//variety end
#include <algorithm>
#include <array>
#include <atomic>
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
//from alexander
#include <iostream>
#include <list>
#include <ratio>
#include <string>
#include <utility>

#include "bitboard.h"
#include "evaluate.h"
#include "history.h"
#include "misc.h"
#include "movegen.h"
#include "movepick.h"
//from classical
#include "syzygy/tbprobe.h"
#include "thread.h"
#include "timeman.h"
#include "tt.h"
#include "types.h"
#include "uci.h"
#include "ucioption.h"
#include "wdl/win_probability.h"  //livebook +shashin theory
#include "learn/learn.h"          //learning
#include "mcts/montecarlo.h"      //Montecarlo
//Livebook begin
#include "livebook/BaseLivebook.h"
#include "livebook/LichessEndgame.h"
#include "livebook/LichessMaster.h"
#include "livebook/ChessDb.h"
#include "livebook/LichessPlayer.h"
#include "livebook/LichessGames.h"
#include "livebook/Proxy.h"
#include "livebook/ChessDBContributor.h"
//Livebook end
namespace Alexander {
using namespace Alexander::Shashin;  //shashin
// learning begin
bool                                    useLearning          = true;
bool                                    enabledLearningProbe = false;
thread_local std::vector<QLearningMove> qLearningTrajectory;
// learning end

namespace TB = Tablebases;

void syzygy_extend_pv(const OptionsMap&            options,
                      const Search::LimitsType&    limits,
                      Alexander::Position&         pos,
                      Alexander::Search::RootMove& rootMove,
                      Value&                       v);

using namespace Search;

namespace {

constexpr int SEARCHEDLIST_CAPACITY = 32;
constexpr int mainHistoryDefault    = 68;
using SearchedList                  = ValueList<Move, SEARCHEDLIST_CAPACITY>;

// (*Scalers):
// The values with Scaler asterisks have proven non-linear scaling.
// They are optimized to time controls of 180 + 1.8 and longer,
// so changing them or adding conditions that are similar requires
// tests at these types of time controls.

// (*Scaler) All tuned parameters at time controls shorter than
// optimized for require verifications at longer time controls

int correction_value(const Worker& w, const Position& pos, const Stack* const ss) {
    const Color us     = pos.side_to_move();
    const auto  m      = (ss - 1)->currentMove;
    const auto& shared = w.sharedHistory;
    const int   pcv    = shared.pawn_correction_entry(pos).at(us).pawn;
    const int   micv   = shared.minor_piece_correction_entry(pos).at(us).minor;
    const int   wnpcv  = shared.nonpawn_correction_entry<WHITE>(pos).at(us).nonPawnWhite;
    const int   bnpcv  = shared.nonpawn_correction_entry<BLACK>(pos).at(us).nonPawnBlack;
    const int   cntcv =
      m.is_ok() ? (*(ss - 2)->continuationCorrectionHistory)[pos.piece_on(m.to_sq())][m.to_sq()]
                    + (*(ss - 4)->continuationCorrectionHistory)[pos.piece_on(m.to_sq())][m.to_sq()]
                  : 8;

    return 10347 * pcv + 8821 * micv + 11665 * (wnpcv + bnpcv) + 7841 * cntcv;
}

// Add correctionHistory value to raw staticEval and guarantee evaluation
// does not hit the tablebase range.
Value to_corrected_static_eval(const Value v, const int cv) {
    return std::clamp(v + cv / 131072, VALUE_TB_LOSS_IN_MAX_PLY + 1, VALUE_TB_WIN_IN_MAX_PLY - 1);
}

void update_correction_history(const Position& pos,
                               Stack* const    ss,
                               Search::Worker& workerThread,
                               const int       bonus) {
    const Move  m  = (ss - 1)->currentMove;
    const Color us = pos.side_to_move();

    constexpr int nonPawnWeight = 178;
    auto&         shared        = workerThread.sharedHistory;

    shared.pawn_correction_entry(pos).at(us).pawn << bonus;
    shared.minor_piece_correction_entry(pos).at(us).minor << bonus * 156 / 128;
    shared.nonpawn_correction_entry<WHITE>(pos).at(us).nonPawnWhite << bonus * nonPawnWeight / 128;
    shared.nonpawn_correction_entry<BLACK>(pos).at(us).nonPawnBlack << bonus * nonPawnWeight / 128;

    if (m.is_ok())
    {
        const Square to = m.to_sq();
        const Piece  pc = pos.piece_on(m.to_sq());
        (*(ss - 2)->continuationCorrectionHistory)[pc][to] << bonus * 127 / 128;
        (*(ss - 4)->continuationCorrectionHistory)[pc][to] << bonus * 59 / 128;
    }
}

// Add a small random component to draw evaluations to avoid 3-fold blindness
Value value_draw(size_t nodes) { return VALUE_DRAW - 1 + Value(nodes & 0x2); }

int variety;  //variety


Value value_to_tt(Value v, int ply);
Value value_from_tt(Value v, int ply, int r50c);
void  update_pv(Move* pv, Move move, const Move* childPv);
void  update_continuation_histories(Stack* ss, Piece pc, Square to, int bonus);
void  update_quiet_histories(
   const Position& pos, Stack* ss, Search::Worker& workerThread, Move move, int bonus);
void update_all_stats(const Position& pos,
                      Stack*          ss,
                      Search::Worker& workerThread,
                      Move            bestMove,
                      Square          prevSq,
                      SearchedList&   quietsSearched,
                      SearchedList&   capturesSearched,
                      Depth           depth,
                      Move            TTMove,
                      int             moveCount);

bool is_shuffling(Move move, Stack* const ss, const Position& pos) {
    if (pos.capture_stage(move) || pos.rule50_count() < 10)
        return false;
    if (pos.state()->pliesFromNull <= 6 || ss->ply < 20)
        return false;
    return move.from_sq() == (ss - 2)->currentMove.to_sq()
        && (ss - 2)->currentMove.from_sq() == (ss - 4)->currentMove.to_sq();
}

}  // namespace


// livebook begin
#ifdef USE_LIVEBOOK
//livebook depth begin
int livebook_depth_count = 0;
int max_book_depth       = 255;
//livebook depth end

std::vector<Livebook::BaseLivebook*> opening_livebooks, endgame_livebooks;

std::string _proxy_url;
bool        _use_lichess_games   = false;
bool        _use_lichess_masters = false;
std::string _lichess_player;
std::string _lichess_player_color;
bool        _use_chess_db = false;

bool _use_chess_db_tablebase = false;
bool _use_lichess_tablebase  = false;

bool _chess_db_contribute = false;
bool _proxy_diversity     = false;

auto contributor = Livebook::ChessDBContributor();

void Search::set_livebook_depth(const int book_depth) { max_book_depth = book_depth; }

void Search::set_proxy_url(const std::string& proxy_url) {
    _proxy_url = proxy_url;
    update_livebooks();
}

void Search::set_use_lichess_games(const bool lichess_games) {
    _use_lichess_games = lichess_games;
    update_livebooks();
}

void Search::set_use_lichess_masters(const bool lichess_masters) {
    _use_lichess_masters = lichess_masters;
    update_livebooks();
}

void Search::set_lichess_player(const std::string& lichess_player) {
    _lichess_player = lichess_player;
    update_livebooks();
}

void Search::set_lichess_player_color(const std::string& lichess_player_color) {
    _lichess_player_color = lichess_player_color;
    update_livebooks();
}

void Search::set_use_chess_db(const bool chess_db) {
    _use_chess_db = chess_db;
    update_livebooks();
}

void Search::set_use_chess_db_tablebase(const bool chess_db) {
    _use_chess_db_tablebase = chess_db;
    update_online_tablebases();
}

void Search::set_use_lichess_tablebase(const bool lichess_tablebase) {
    _use_lichess_tablebase = lichess_tablebase;
    update_online_tablebases();
}

void Search::update_livebooks() {
    // Clear the existing opening livebooks
    opening_livebooks.clear();

    if (!_proxy_url.empty())
    {
        Livebook::Proxy* proxy = new Livebook::Proxy(_proxy_url);
        if (_proxy_diversity)
        {
            proxy->set_action(Livebook::Action::QUERY);
        }
        else
        {
            proxy->set_action(Livebook::Action::QUERY_BEST);
        }
        opening_livebooks.push_back(proxy);
    }

    if (!_lichess_player.empty())
    {
        if (_lichess_player_color.empty())
        {
            _lichess_player_color = "white";
        }

        opening_livebooks.push_back(
          new Livebook::LichessPlayer(_lichess_player, _lichess_player_color));
    }

    if (_use_lichess_games)
    {
        opening_livebooks.push_back(new Livebook::LichessGames());
    }

    if (_use_lichess_masters)
    {
        opening_livebooks.push_back(new Livebook::LichessMaster());
    }

    if (_use_chess_db)
    {
        opening_livebooks.push_back(new Livebook::ChessDb());
    }
}

void Search::update_online_tablebases() {
    // Clear the existing endgame livebooks
    endgame_livebooks.clear();

    if (_use_chess_db_tablebase)  // ChessDb tablebase
    {
        endgame_livebooks.push_back(new Livebook::ChessDb());
    }

    if (_use_lichess_tablebase)  // Lichess tablebase
    {
        endgame_livebooks.push_back(new Livebook::LichessEndgame());
    }
}

void Search::set_chess_db_contribute(const bool chess_db_contribute) {
    _chess_db_contribute = chess_db_contribute;
}

void Search::set_proxy_diversity(const bool proxy_diversity) {
    _proxy_diversity = proxy_diversity;
    update_livebooks();
}
#endif
// livebook end


//variety begin
void Search::set_variety(const std::string& varietyOption) {
    if (varietyOption == "Off")
    {
        variety = 0;
    };
    if (varietyOption == "Standard")
    {
        variety = 1;
    };
    if (varietyOption == "Psychological")
    {
        variety = 2;
    };
};
//variety end
//for learning begin
inline bool is_game_decided(const Position& pos, Value lastScore) {
    static constexpr const Value DecidedGameEvalThreeshold = PawnValue * 5;
    static constexpr const int   DecidedGameMaxPly         = 150;
    static constexpr const int   DecidedGameMaxPieceCount  = 5;

    //Assume game is decided if |last sent score| is above DecidedGameEvalThreeshold
    if (is_valid(lastScore) && std::abs(lastScore) > DecidedGameEvalThreeshold)
        return true;

    //Assume game is decided (draw) if game ply is above 150
    if (pos.game_ply() > DecidedGameMaxPly)
        return true;

    if (pos.count<ALL_PIECES>() < DecidedGameMaxPieceCount)
        return true;

    //Assume game is not decided!
    return false;
}
//for learning end

Search::Worker::Worker(SharedState&                    sharedState,
                       std::unique_ptr<ISearchManager> sm,
                       size_t                          threadId,
                       size_t                          numaThreadId,
                       size_t                          numaTotalThreads,
                       NumaReplicatedAccessToken       token) :
    // Unpack the SharedState struct into member variables
    //mcts begin
    threadIdx(threadId),
    sharedHistory(sharedState.sharedHistories.at(token.get_numa_index())),
    //mcts end
    numaThreadIdx(numaThreadId),
    numaTotal(numaTotalThreads),
    numaAccessToken(token),
    manager(std::move(sm)),
    bookMan(sharedState.bookMan),  //from book management
    options(sharedState.options),
    threads(sharedState.threads),
    tt(sharedState.tt),
    //for classical
    //Shashin begin
    shashinManager(std::make_unique<ShashinManager>()),
    shConfig(sharedState.shashinConfig) {
    //Shashin end
    clear();
}
//from shashin begin
Search::Worker::~Worker() = default;
ShashinManager& Worker::getShashinManager() { return *shashinManager; }
//fom Shashin end

//for classical
void Search::Worker::start_searching() {

    //no classical
    // Non-main threads go directly to iterative_deepening()
    if (!is_mainthread())
    {
        iterative_deepening();
        return;
    }
    else
    {
        // It inizializes the rootPosition static state
        shashinManager->setStaticState(rootPos);
    }
    main_manager()->tm.init(limits, rootPos.side_to_move(), rootPos.game_ply(), options,
                            main_manager()->originalTimeAdjust);
    tt.new_search();
    // learning begin
    enabledLearningProbe = false;
    useLearning          = true;
    // learning end

    set_variety(options["Variety"]);  // variety

    Move bookMove = Move::none();  //Books management

    if (rootMoves.empty())
    {
        rootMoves.emplace_back(Move::none());
        main_manager()->updates.onUpdateNoMoves(
          {0, {rootPos.checkers() ? -VALUE_MATE : VALUE_DRAW, rootPos}});
    }
    else
    // Books management begin
    {

        bool think = true;
        if (!(limits.infinite || limits.mate || limits.depth || limits.nodes || limits.perft)
            && !main_manager()->ponder)
        {
            //Probe the configured books
            bookMove = bookMan.probe(rootPos, options);
            //Probe experience book begin
            if (bookMove == Move::none() && (bool) options["Experience Book"]
                && rootPos.game_ply() / 2 < (int) options["Experience Book Max Moves"])
            {
                Depth expBookMinDepth = (Depth) options["Experience Book Min Depth"];
                std::vector<LearningMove*> learningMoves = LD.probe(rootPos.key());
                if (!learningMoves.empty())
                {
                    LD.sortLearningMoves(learningMoves);
                    std::vector<LearningMove*> bestMoves;
                    Depth                      bestDepth = learningMoves[0]->depth;
                    if (bestDepth >= expBookMinDepth)
                    {
                        int   bestPerformance = learningMoves[0]->performance;
                        Value bestScore       = learningMoves[0]->score;
                        if (bestPerformance >= 50)
                        {
                            for (const auto& move : learningMoves)
                            {
                                if (move->depth == bestDepth && move->performance == bestPerformance
                                    && move->score == bestScore)
                                {
                                    bestMoves.push_back(move);
                                }
                                else
                                {
                                    break;
                                }
                            }
                            if (!bestMoves.empty())
                            {
                                std::random_device              rd;
                                std::mt19937                    gen(rd());
                                std::uniform_int_distribution<> distr(0, bestMoves.size() - 1);
                                bookMove = bestMoves[distr(gen)]->move;
                            }
                        }
                    }
                }
            }
            //Probe experience book end
            if (bookMove != Move::none()
                && std::find(rootMoves.begin(), rootMoves.end(), bookMove) != rootMoves.end())
            {
                think = false;

                for (auto&& th : threads)
                    std::swap(th->worker->rootMoves[0],
                              *std::find(th->worker->rootMoves.begin(), th->worker->rootMoves.end(),
                                         bookMove));
            }
        }
// Live Book begin
#ifdef USE_LIVEBOOK
        if (think && !bookMove)
        {
            if (!limits.infinite && !limits.mate)
            {
                livebook_depth_count = rootPos.game_ply();

                std::vector<Livebook::BaseLivebook*> livebooks;

                // If there are no more than 7 units on the board and there aren't the Syzygy Tbs, use the endgame livebooks
                const std::string& syzygyPath = static_cast<std::string>(options["SyzygyPath"]);

                int totalUnits = rootPos.count<ALL_PIECES>();

                if ((totalUnits <= 7)
                    && (syzygyPath.empty()
                        || ((!syzygyPath.empty())
                            && ((int(options["SyzygyProbeLimit"]) < totalUnits)))))
                {
                    livebooks = endgame_livebooks;
                }
                // If we are in the first max_book_depth plies, use the opening livebooks
                else if (livebook_depth_count < max_book_depth)
                {
                    livebooks = opening_livebooks;
                }

                for (const auto livebook : livebooks)
                {
                    if (std::vector<std::pair<std::string, Analysis>> output =
                          livebook->lookup(rootPos);
                        !output.empty())
                    {
                        std::string     uci;
                        const Analysis* best = nullptr;

                        // Iterate through the vector of moves and their corresponding analysis
                        for (auto& [move, analysis] : output)
                        {
                            if (best == nullptr || analysis > *best)
                            {
                                best = &analysis;
                                uci  = move;
                            }
                        }

                        if (best != nullptr)
                        {
                            bookMove = UCIEngine::to_move(rootPos, uci);

                            if (bookMove)
                            {
                                livebook_depth_count++;
                            }
                        }

                        break;
                    }
                }
            }


            if (bookMove && std::count(rootMoves.begin(), rootMoves.end(), bookMove))
            {
                think = false;

                auto best_thread = threads.get_best_thread()->worker.get();
                std::swap(best_thread->rootMoves[0],
                          *std::find(best_thread->rootMoves.begin(), best_thread->rootMoves.end(),
                                     bookMove));

                /*
                for (auto th : threads)
                {
                    std::swap(th->worker->rootMoves[0],
                              *std::find(th->worker->rootMoves.begin(), th->worker->rootMoves.end(),
                                         bookMove));
                }
                */
            }
            else
            {
                bookMove = Move::none();
            }
        }
#endif
        // Live Book end

        //from Book and live book management begin
        if (!bookMove && think)
        {
            //Initialize `mctsThreads` threads only once before any thread have begun searching
            mctsThreads        = size_t(int(options["MCTSThreads"]));
            mctsMultiStrategy  = size_t(int(options["MCTS Multi Strategy"]));
            mctsMultiMinVisits = double(int(options["MCTS Multi MinVisits"]));

            threads.start_searching();  // start non-main threads
            iterative_deepening();      // main thread start searching
        }
        //from Book and live book management end
    }
    // Books management end

    // When we reach the maximum depth, we can arrive here without a raise of
    // threads.stop. However, if we are pondering or in an infinite search,
    // the UCI protocol states that we shouldn't print the best move before the
    // GUI sends a "stop" or "ponderhit" command. We therefore simply wait here
    // until the GUI sends one of those commands.
    while (!threads.stop && (main_manager()->ponder || limits.infinite))
    {}  // Busy wait for a stop or a ponder reset

    // Stop the threads if not already stopped (also raise the stop if
    // "ponderhit" just reset threads.ponder)
    threads.stop = true;

    // Wait until all threads have finished
    threads.wait_for_search_finished();

    // When playing in 'nodes as time' mode, subtract the searched nodes from
    // the available ones before exiting.
    if (limits.npmsec)
        main_manager()->tm.advance_nodes_time(threads.nodes_searched()
                                              - limits.inc[rootPos.side_to_move()]);

    Worker* bestThread = this;
    //no Skill

    if (int(options["MultiPV"]) == 1 && !limits.depth && !limits.mate  //no skill
        && rootMoves[0].pv[0] != Move::none())
        bestThread = threads.get_best_thread()->worker.get();

    main_manager()->bestPreviousScore        = bestThread->rootMoves[0].score;
    main_manager()->bestPreviousAverageScore = bestThread->rootMoves[0].averageScore;


    // learning begin
    if (!bookMove)
    {
        if (bestThread->completedDepth > 4 && LD.is_enabled() && !LD.is_paused())  // from learning
        {
            PersistedLearningMove plm;
            plm.key                = rootPos.key();
            plm.learningMove.depth = bestThread->completedDepth;
            plm.learningMove.move  = bestThread->rootMoves[0].pv[0];
            plm.learningMove.score = bestThread->rootMoves[0].score;
            plm.learningMove.performance =
              WDLModel::get_win_probability(bestThread->rootMoves[0].score, rootPos);

            if (LD.learning_mode() == LearningMode::Self)
            {
                const LearningMove* existingMove = LD.probe_move(plm.key, plm.learningMove.move);
                if (existingMove)
                    plm.learningMove.score = existingMove->score;
                QLearningMove qLearningMove;
                qLearningMove.persistedLearningMove = plm;
                const int qLearningMoveMaterial =
                  rootPos.count<PAWN>() + 3 * rootPos.count<KNIGHT>() + 3 * rootPos.count<BISHOP>()
                  + 5 * rootPos.count<ROOK>() + 9 * rootPos.count<QUEEN>();
                const int qLearningMoveMaterialClamp = std::clamp(qLearningMoveMaterial, 17, 78);
                qLearningMove.materialClamp          = qLearningMoveMaterialClamp;
                qLearningTrajectory.push_back(qLearningMove);
            }
            else
            {
                LD.add_new_learning(plm.key, plm.learningMove);
            }
        }
        if (!enabledLearningProbe)
        {
            useLearning = false;
        }
    }
    // learning end

    // Send again PV info if we have a new best thread
    if (bestThread != this)
    {                                                                                    //learning
        main_manager()->pv(*bestThread, threads, tt, bestThread->completedDepth, true);  //shashin
    }  //learning

    std::string ponder;

    if (bestThread->rootMoves[0].pv.size() > 1
        || bestThread->rootMoves[0].extract_ponder_from_tt(tt, rootPos))
        ponder = UCIEngine::move(bestThread->rootMoves[0].pv[1], rootPos.is_chess960());

    auto bestmove = UCIEngine::move(bestThread->rootMoves[0].pv[0], rootPos.is_chess960());
    main_manager()->updates.onBestmove(bestmove, ponder);
    // from learning begin
    // Save learning data if game is already decided
    if (!bookMove)
    {
        if (is_game_decided(rootPos, (bestThread->rootMoves[0].score)) && LD.is_enabled()
            && !LD.is_paused())
        {
            // Perform Q-learning if enabled
            if (LD.learning_mode() == LearningMode::Self)
            {
                putQLearningTrajectoryIntoLearningTable();
            }

            // Save to learning file
            if (!LD.is_readonly())
            {

                LD.persist(options);
            }
            // Stop learning until we receive *ucinewgame* command
            LD.pause();
        }
    }

    // from learning end

    // livebook begin
#ifdef USE_LIVEBOOK
    if (_chess_db_contribute)
    {
        contributor.contribute(rootPos, bestThread->rootMoves[0].pv[0]);
    }
#endif
    // livebook end
}

// Main iterative deepening loop. It calls search()
// repeatedly with increasing depth until the allocated thinking time has been
// consumed, the user stops the search, or the maximum search depth is reached.
void Search::Worker::iterative_deepening() {
    auto&          localShashinManager = *this->shashinManager;
    SearchManager* mainThread          = (is_mainthread() ? main_manager() : nullptr);

    Move pv[MAX_PLY + 1];

    Depth lastBestMoveDepth = 0;
    Value lastBestScore     = -VALUE_INFINITE;
    auto  lastBestPV        = std::vector{Move::none()};

    Value alpha, beta;
    Value bestValue = -VALUE_INFINITE;
    //for classical
    double timeReduction = 1, totBestMoveChanges = 0;
    int    delta, iterIdx                        = 0;

    // Allocate stack with extra size to allow access from (ss - 7) to (ss + 2):
    // (ss - 7) is needed for update_continuation_histories(ss - 1) which accesses (ss - 6),
    // (ss + 2) is needed for initialization of cutOffCnt.
    Stack  stack[MAX_PLY + 10] = {};
    Stack* ss                  = stack + 7;

    for (int i = 7; i > 0; --i)
    {
        (ss - i)->continuationHistory =
          &continuationHistory[0][0][NO_PIECE][0];  // Use as a sentinel
        (ss - i)->continuationCorrectionHistory = &continuationCorrectionHistory[NO_PIECE][0];
        (ss - i)->staticEval                    = VALUE_NONE;
    }

    for (int i = 0; i <= MAX_PLY + 2; ++i)
        (ss + i)->ply = i;

    ss->pv = pv;
    localShashinManager.initDynamicRootShashinState(rootPos, ss, shConfig, rootDepth);
    const RootShashinState& rootShashinState = localShashinManager.getState();
    const auto&             dynamicDerived   = rootShashinState.dynamicDerived;
    bool                    isAggressive     = localShashinManager.isAggressive();
    bool                    isHighTal        = dynamicDerived.isHighTal;
    if (mainThread)
    {
        if (mainThread->bestPreviousScore == VALUE_INFINITE)
            mainThread->iterValue.fill(VALUE_ZERO);
        else
            mainThread->iterValue.fill(mainThread->bestPreviousScore);
    }

    size_t multiPV  = size_t(options["MultiPV"]);
    antiDancingMode = bool(options["Anti Dancing Analysis"]);  //anti dancing patch
    //no skill

    multiPV                = std::min(multiPV, rootMoves.size());
    int searchAgainCounter = 0;

    lowPlyHistory.fill(97);

    for (Color c : {WHITE, BLACK})
        for (int i = 0; i < UINT_16_HISTORY_SIZE; i++)
            mainHistory[c][i] =
              (mainHistory[c][i] - mainHistoryDefault) * 3 / 4 + mainHistoryDefault;

    // from mcts begin
    // Initialize optimism for MCTS
    optimism[WHITE] = optimism[BLACK] =
      VALUE_ZERO;  //Must initialize optimism before calling static_value(). Not sure if 'VALUE_ZERO' is the right value

    if (bool(options["MCTS by Shashin"]))
    {
        bool maybeDraw = rootPos.rule50_count() >= 90 || rootPos.upcoming_repetition(2);
        // Calculate static evaluation for root position
        Value rootPosValue = localShashinManager.static_value(rootPos, ss);  //shashin
        bool  isMCTSExplorationApplicable = localShashinManager.isMCTSExplorationApplicable();
        bool  mctsExplore                 = options["MCTS Explore"]
                        && (isMCTSExplorationApplicable
                            || localShashinManager.isHighPieceDensityCapablancaPosition());

        // Check if MCTS can be applied
        bool possibleMCTSByValue = localShashinManager.isMCTSApplicableByValue();
        if (!mainThread && multiPV == 1 && !maybeDraw && (possibleMCTSByValue || mctsExplore)
            && (rootPos.count<ALL_PIECES>() > 7) && (this->threadIdx) <= (size_t) (mctsThreads)
            && !is_game_decided(rootPos, rootPosValue))
        {
            MonteCarlo* monteCarlo = new MonteCarlo(rootPos, this, tt);

            if (monteCarlo)
            {
#if !defined(NDEBUG) && !defined(_NDEBUG)
                sync_cout << "info string *** Thread[" << threadIdx << "] is running MCTS search"
                          << sync_endl;
#endif

                monteCarlo->search(threads, limits, is_mainthread(), this);
                if ((this->threadIdx) == 1 && limits.infinite
                    && threads.stop.load(std::memory_order_relaxed))
                    monteCarlo->print_children();

                delete monteCarlo;

#if !defined(NDEBUG) && !defined(_NDEBUG)
                sync_cout << "info string *** Thread[" << threadIdx << "] finished MCTS search"
                          << sync_endl;
#endif

                return;
            }
        }

#if !defined(NDEBUG) && !defined(_NDEBUG)
        sync_cout << "info string *** Thread[" << threadIdx << "] is running A/B search"
                  << sync_endl;
#endif
    }
    // from mcts end
    // Iterative deepening loop until requested to stop or the target depth is reached
    while (++rootDepth < MAX_PLY && !threads.stop
           && !(limits.depth && mainThread && rootDepth > limits.depth))
    {
        // Age out PV variability metric
        if (mainThread)
            totBestMoveChanges /= 2;

        // Save the last iteration's scores before the first PV line is searched and
        // all the move scores except the (new) PV are set to -VALUE_INFINITE.
        for (RootMove& rm : rootMoves)
            rm.previousScore = rm.score;

        size_t pvFirst = 0;
        pvLast         = 0;

        if (!threads.increaseDepth)
            searchAgainCounter++;

        // MultiPV loop. We perform a full root search for each PV line
        for (pvIdx = 0; pvIdx < multiPV; ++pvIdx)
        {
            if (pvIdx == pvLast)
            {
                pvFirst = pvLast;
                for (pvLast++; pvLast < rootMoves.size(); pvLast++)
                    if (rootMoves[pvLast].tbRank != rootMoves[pvFirst].tbRank)
                        break;
            }

            // Reset UCI info selDepth for each depth and each PV line
            selDepth = 0;

            // Reset aspiration window starting size
            // --- FIX ASPIRATION WINDOW (Hybrid Tattica) ---
            // 1. Base Stockfish solida (include meanSquaredScore per stabilità nei match)
            delta = 5 + threadIdx % 8 + std::abs(rootMoves[pvIdx].meanSquaredScore) / 9000;

#ifdef GOLD_DIGGER
            if (localShashinManager.isStrategical())
            {
                // Nelle posizioni strategiche, ci aspettiamo meno oscillazioni.
                // Stringiamo leggermente (90%) per guadagnare efficienza.
                delta = std::max(6, (delta * 9) / 10);

                // Se è una fortezza, stringiamo ancora di più
                if (MoveConfig::isFortress)
                    delta = std::max(5, delta - 2);
            }
            else if (isAggressive || isHighTal)
            {
                // Tattico puro: finestra allargata del 12.5% (compromesso stabilità/tattica)
                // Riduce i re-search inutili rispetto al 25% precedente.
                delta = (delta * 9) / 8;
            }
#else
            // 2. Modulazione Shashin (Match-Winner Tuning)
            // Solo TAL (Aggressive) beneficia di una finestra più larga per vedere tattiche nascoste.
            // Petrosian e Capablanca usano il default (stabilità massima nelle "pepite").
            if (isAggressive || isHighTal)
            {
                delta = (delta * 9) / 8;  // 12.5% expansion
            }
#endif


            // Sicurezza a basse profondità
            if (rootDepth < 10)
                delta = std::min(delta, 15);

            Value avg = rootMoves[pvIdx].averageScore;
            alpha     = std::max(avg - delta, -VALUE_INFINITE);
            beta      = std::min(avg + delta, VALUE_INFINITE);

            //classical
            // Start with a small aspiration window and, in the case of a fail
            // high/low, re-search with a bigger window until we don't fail
            // high/low anymore.
            int failedHighCnt = 0;
            while (true)
            {
                // Adjust the effective depth searched, but ensure at least one
                // effective increment for every four searchAgain steps (see issue #2717).
                Depth adjustedDepth =
                  std::max(1, rootDepth - failedHighCnt - 3 * (searchAgainCounter + 1) / 4);
                rootDelta = beta - alpha;
                bestValue = search<Root>(rootPos, ss, alpha, beta, adjustedDepth, false);

                // Bring the best move to the front. It is critical that sorting
                // is done with a stable algorithm because all the values but the
                // first and eventually the new best one is set to -VALUE_INFINITE
                // and we want to keep the same order for all the moves except the
                // new PV that goes to the front. Note that in the case of MultiPV
                // search the already searched PV lines are preserved.
                std::stable_sort(rootMoves.begin() + pvIdx, rootMoves.begin() + pvLast);

                // If search has been stopped, we break immediately. Sorting is
                // safe because RootMoves is still valid, although it refers to
                // the previous iteration.
                if (threads.stop)
                    break;

                // When failing high/low give some update before a re-search. To avoid
                // excessive output that could hang GUIs like Fritz 19, only start
                // at nodes > 10M (rather than depth N, which can be reached quickly)
                if (mainThread && multiPV == 1 && (bestValue <= alpha || bestValue >= beta)
                    && nodes > 10000000)
                {                                                              //no warning
                    main_manager()->pv(*this, threads, tt, rootDepth, false);  //shashin
                }  //no warning
                // In case of failing low/high increase aspiration window and re-search,
                // otherwise exit the loop.
                if (bestValue <= alpha)
                {
                    beta  = alpha;
                    alpha = std::max(bestValue - delta, -VALUE_INFINITE);

                    failedHighCnt = 0;
                    if (mainThread)
                        mainThread->stopOnPonderhit = false;
                }
                else if (bestValue >= beta)
                {
                    alpha = std::max(beta - delta, alpha);
                    beta  = std::min(bestValue + delta, VALUE_INFINITE);
                    ++failedHighCnt;
                }
                else
                    break;

                delta += delta / 3;

                assert(alpha >= -VALUE_INFINITE && beta <= VALUE_INFINITE);
            }

            // Sort the PV lines searched so far and update the GUI
            std::stable_sort(rootMoves.begin() + pvFirst, rootMoves.begin() + pvIdx + 1);

            if (mainThread
                && (threads.stop || pvIdx + 1 == multiPV || nodes > 10000000)
                // A thread that aborted search can have mated-in/TB-loss PV and
                // score that cannot be trusted, i.e. it can be delayed or refuted
                // if we would have had time to fully search other root-moves. Thus
                // we suppress this output and below pick a proven score/PV for this
                // thread (from the previous iteration).
                && !(threads.abortedSearch && is_loss(rootMoves[0].uciScore)))
            {                                                             //no warning
                main_manager()->pv(*this, threads, tt, rootDepth, true);  //shashin
            }  //no warning
            if (threads.stop)
                break;
        }

        if (!threads.stop)
            completedDepth = rootDepth;

        // We make sure not to pick an unproven mated-in score,
        // in case this thread prematurely stopped search (aborted-search).
        if (threads.abortedSearch && rootMoves[0].score != -VALUE_INFINITE
            && is_loss(rootMoves[0].score))
        {
            // Bring the last best move to the front for best thread selection.
            Utility::move_to_front(rootMoves, [&lastBestPV = std::as_const(lastBestPV)](
                                                const auto& rm) { return rm == lastBestPV[0]; });
            rootMoves[0].pv    = lastBestPV;
            rootMoves[0].score = rootMoves[0].uciScore = lastBestScore;
        }
        else if (rootMoves[0].pv[0] != lastBestPV[0])
        {
            lastBestPV        = rootMoves[0].pv;
            lastBestScore     = rootMoves[0].score;
            lastBestMoveDepth = rootDepth;
        }

        if (!mainThread)
            continue;

        // Have we found a "mate in x"?
        if (limits.mate && rootMoves[0].score == rootMoves[0].uciScore
            && ((rootMoves[0].score >= VALUE_MATE_IN_MAX_PLY
                 && VALUE_MATE - rootMoves[0].score <= 2 * limits.mate)
                || (rootMoves[0].score != -VALUE_INFINITE
                    && rootMoves[0].score <= VALUE_MATED_IN_MAX_PLY
                    && VALUE_MATE + rootMoves[0].score <= 2 * limits.mate)))
            threads.stop = true;

        //no Skill

        // Use part of the gained time from a previous stable move for the current move
        for (auto&& th : threads)
        {
            totBestMoveChanges += th->worker->bestMoveChanges;
            th->worker->bestMoveChanges = 0;
        }

        // Do we have time for the next iteration? Can we stop searching now?
        if (limits.use_time_management() && !threads.stop && !mainThread->stopOnPonderhit)
        {
            uint64_t nodesEffort =
              rootMoves[0].effort * 100000 / std::max(size_t(1), size_t(nodes));

            double fallingEval = (11.85 + 2.24 * (mainThread->bestPreviousAverageScore - bestValue)
                                  + 0.93 * (mainThread->iterValue[iterIdx] - bestValue))
                               / 100.0;

            fallingEval = std::clamp(fallingEval, 0.57, 1.70);

            // If the bestMove is stable over several iterations, reduce time accordingly
            double k      = 0.51;
            double center = lastBestMoveDepth + 12.15;

            timeReduction = 0.66 + 0.85 / (0.98 + std::exp(-k * (completedDepth - center)));

            double reduction = (1.43 + mainThread->previousTimeReduction) / (2.28 * timeReduction);

            double bestMoveInstability = 1.02 + 2.14 * totBestMoveChanges / threads.size();

            double highBestMoveEffort = nodesEffort >= 93340 ? 0.76 : 1.0;

            double totalTime = mainThread->tm.optimum() * fallingEval * reduction
                             * bestMoveInstability * highBestMoveEffort;

            // Cap used time in case of a single legal move for a better viewer experience
            if (rootMoves.size() == 1)
                totalTime = std::min(502.0, totalTime);

            auto elapsedTime = elapsed();

            // Stop the search if we have exceeded the totalTime or maximum
            if (elapsedTime > std::min(totalTime, double(mainThread->tm.maximum())))
            {
                // If we are allowed to ponder do not stop the search now but
                // keep pondering until the GUI sends "ponderhit" or "stop".
                if (mainThread->ponder)
                    mainThread->stopOnPonderhit = true;
                else
                    threads.stop = true;
            }
            else
                threads.increaseDepth = mainThread->ponder || elapsedTime <= totalTime * 0.50;
        }

        mainThread->iterValue[iterIdx] = bestValue;
        iterIdx                        = (iterIdx + 1) & 3;
    }

    if (!mainThread)
        return;

    mainThread->previousTimeReduction = timeReduction;
    //no Skill
}

void Search::Worker::do_move(Position& pos, const Move move, StateInfo& st, Stack* const ss) {
    do_move(pos, move, st, pos.gives_check(move), ss);
}

void Search::Worker::do_move(
  Position& pos, const Move move, StateInfo& st, const bool givesCheck, Stack* const ss) {
    bool capture = pos.capture_stage(move);
    // Preferable over fetch_add to avoid locking instructions
    nodes.store(nodes.load(std::memory_order_relaxed) + 1, std::memory_order_relaxed);
    pos.do_move(move, st, givesCheck, &tt, &sharedHistory);
    if (ss != nullptr)
    {
        ss->currentMove = move;

        // Recuperiamo il pezzo che si trova ora sulla casa di arrivo.
        // In HCE non c'è "dirtyPiece", ma la board è aggiornata, quindi funziona.
        Piece pc = pos.piece_on(move.to_sq());  //for classical

        // Usiamo 'pc' per soddisfare la dimensione [PIECE_NB] richiesta da history.h
        ss->continuationHistory = &continuationHistory[ss->inCheck][capture][pc][move.to_sq()];

        ss->continuationCorrectionHistory = &continuationCorrectionHistory[pc][move.to_sq()];
    }
}


void Search::Worker::do_null_move(Position& pos, StateInfo& st, Stack* const ss) {
    pos.do_null_move(st, tt);
    ss->currentMove                   = Move::null();
    ss->continuationHistory           = &continuationHistory[0][0][NO_PIECE][0];
    ss->continuationCorrectionHistory = &continuationCorrectionHistory[NO_PIECE][0];
}

void Search::Worker::undo_move(Position& pos, const Move move) { pos.undo_move(move); }

void Search::Worker::undo_null_move(Position& pos) { pos.undo_null_move(); }


// Reset histories, usually before a new game
void Search::Worker::clear() {
    mainHistory.fill(mainHistoryDefault);
    captureHistory.fill(-689);

    // Each thread is responsible for clearing their part of shared history
    sharedHistory.correctionHistory.clear_range(0, numaThreadIdx, numaTotal);
    sharedHistory.pawnHistory.clear_range(-1238, numaThreadIdx, numaTotal);

    ttMoveHistory = 0;

    for (auto& to : continuationCorrectionHistory)
        for (auto& h : to)
            h.fill(8);

    for (bool inCheck : {false, true})
        for (StatsType c : {NoCaptures, Captures})
            for (auto& to : continuationHistory[inCheck][c])
                for (auto& h : to)
                    h.fill(-529);

    for (size_t i = 1; i < reductions.size(); ++i)
        reductions[i] = int(2747 / 128.0 * std::log(i));

    //no classical
}


// Main search function for both PV and non-PV nodes
template<NodeType nodeType>
Value Search::Worker::search(
  Position& pos, Stack* ss, Value alpha, Value beta, Depth depth, bool cutNode) {

    constexpr bool PvNode   = nodeType != NonPV;
    constexpr bool rootNode = nodeType == Root;
    const bool     allNode  = !(PvNode || cutNode);

    //from Shashih begin
    auto&                   localShashinManager = *this->shashinManager;
    const RootShashinState& rootShashinState    = localShashinManager.getState();  //for Alexander
    bool                    isFortressNode      = localShashinManager.isFortress(rootPos);
    //from Shashih end

    // Dive into quiescence search when the depth reaches zero
    if (depth <= 0)
        return qsearch<PvNode ? PV : NonPV>(pos, ss, alpha, beta);

    // Limit the depth if extensions made it too large
    depth = std::min(depth, MAX_PLY - 1);

    // CACHE NEI REGISTRI: Copiamo il flag globale in una costante locale.
    // Questo dice al compilatore di tenere il valore nel registro della CPU per tutta la funzione,
    // garantendo lo stesso tempo di accesso di una variabile static ma in modo sicuro.
    const bool antiDancing = antiDancingMode;

    // Check if we have an upcoming move that draws by repetition
    // Lo "&& true" forza il compilatore a rispettare lo short-circuit al 100%
    if (!rootNode && (alpha < VALUE_DRAW || (antiDancing && true))
        && pos.upcoming_repetition(ss->ply))
    {
        // BRANCH HINTING ESTREMO: Diciamo alla CPU che questo if è statisticamente quasi sempre falso.
        // La pipeline del processore caricherà in anticipo le istruzioni del blocco standard.
        if (antiDancing && alpha >= VALUE_DRAW) [[unlikely]]
        {
            return value_draw(nodes);
        }

        // Comportamento Standard di Stockfish
        if (alpha < VALUE_DRAW)
        {
            alpha = value_draw(nodes);
            if (alpha >= beta)
                return alpha;
        }
    }

    assert(-VALUE_INFINITE <= alpha && alpha < beta && beta <= VALUE_INFINITE);
    assert(PvNode || (alpha == beta - 1));
    assert(0 < depth && depth < MAX_PLY);
    assert(!(PvNode && cutNode));

    Move      pv[MAX_PLY + 1];
    StateInfo st;

    Key  posKey;
    Move move, excludedMove = Move::none(), bestMove,
               expTTMove = Move::none();  // from learning
    Depth extension, newDepth;
    Value bestValue, value = VALUE_NONE, eval = VALUE_NONE, maxValue, expTTValue = VALUE_NONE,
                     probCutBeta = VALUE_NONE;  // from learning
    bool givesCheck, improving = false, priorCapture, opponentWorsening = false,
                     expTTHit = false;  //shashin +learning
    //from Shashin-Crystal begin
    bool gameCycle, capture, ttCapture;
    //from shashin crystal end
    int          priorReduction;
    Piece        movedPiece;
    bool         ourMove = !(ss->ply & 1);  // from Shashin Crystal
    SearchedList capturesSearched;
    SearchedList quietsSearched;
    // from learning begin
    bool updatedLearning = false;
    // flags to preserve node types
    bool disableNMAndPC = false;
    bool expectedPVNode = false;
    int  sibs           = 0;
    // from learning end
    // Step 1. Initialize node
    ss->inCheck   = pos.checkers();
    priorCapture  = pos.captured_piece();
    Color us      = pos.side_to_move();
    ss->moveCount = 0;
    bestValue     = -VALUE_INFINITE;
    maxValue      = VALUE_INFINITE;
    // from Crystal-shashin begin
    gameCycle = false;

    ss->secondaryLine = false;
    ss->mainLine      = false;
    // from Crystal-shashin end
    // Full Threads patch begin
    if (fullSearch)
        improving = true;
    // Full Threads patch end
    // Check for the available remaining time
    if (is_mainthread())
        main_manager()->check_time(*this);

    // Used to send selDepth info to GUI (selDepth counts from 1, ply from 0)
    if (PvNode && selDepth < ss->ply + 1)
        selDepth = ss->ply + 1;
    //from crystal shashin begin
    const auto& dynamicDerived      = rootShashinState.dynamicDerived;
    const auto& staticState         = rootShashinState.staticState;
    bool        isStrategical       = localShashinManager.isStrategical();
    bool        isTacticalReactive  = dynamicDerived.isTacticalReactive;
    bool        stmKingExposed      = staticState.stmKingExposed;
    bool        opponentKingExposed = staticState.opponentKingExposed;
    bool        isSacrificial       = staticState.isSacrificial;
    bool        quietKingSafe       = !isSacrificial && (!stmKingExposed);
    bool        isTal               = localShashinManager.isTal();
    bool        isPetrosian         = localShashinManager.isPetrosian();
    bool        isCapablanca        = localShashinManager.isCapablanca();
    bool        isAggressive        = localShashinManager.isAggressive();
    bool        isTactical          = localShashinManager.isTactical();
    bool        isComplex           = localShashinManager.isComplexPosition();
    uint8_t     legalMoveCount      = staticState.legalMoveCount;
    (void) legalMoveCount;
    //from crystal shashin end
    if (!rootNode)
    {
        // from Crystal-Shashin begin
        bool allowCycleDetection =
          (isStrategical && isComplex)
          || (localShashinManager.isTacticalDefensive() && localShashinManager.isLowActivity());

        if (allowCycleDetection && !ss->excludedMove)
        {
            if (pos.upcoming_repetition(ss->ply))
            {
                gameCycle = true;
            }
        }
        // from Crystal-Shashin end
        // Step 2. Check for aborted search and immediate draw
        if (threads.stop.load(std::memory_order_relaxed) || pos.is_draw(ss->ply)
            || ss->ply >= MAX_PLY)
            return (ss->ply >= MAX_PLY && !ss->inCheck) ? evaluate(pos) : value_draw(nodes);

        // Step 3. Mate distance pruning. Even if we mate at the next move our score
        // would be at best mate_in(ss->ply + 1), but if alpha is already bigger because
        // a shorter mate was found upward in the tree then there is no need to search
        // because we will never beat the current alpha. Same logic but with reversed
        // signs apply also in the opposite condition of being mated instead of giving
        // mate. In this case, return a fail-high score.
        alpha = std::max(mated_in(ss->ply), alpha);
        beta  = std::min(mate_in(ss->ply + 1), beta);
        if (alpha >= beta)
            return alpha;
    }

    assert(0 <= ss->ply && ss->ply < MAX_PLY);

    Square prevSq  = ((ss - 1)->currentMove).is_ok() ? ((ss - 1)->currentMove).to_sq() : SQ_NONE;
    bestMove       = Move::none();
    priorReduction = (ss - 1)->reduction;
    (ss - 1)->reduction = 0;
    ss->statScore       = 0;
    (ss + 2)->cutoffCnt = 0;

    // Step 4. Transposition table lookup
    excludedMove                   = ss->excludedMove;
    posKey                         = pos.key();
    auto [ttHit, ttData, ttWriter] = tt.probe(posKey);
    // Need further processing of the saved data
    ss->ttHit    = ttHit;
    ttData.move  = rootNode ? rootMoves[pvIdx].pv[0] : ttHit ? ttData.move : Move::none();
    ttData.value = ttHit ? value_from_tt(ttData.value, ss->ply, pos.rule50_count()) : VALUE_NONE;
    ss->ttPv     = excludedMove ? ss->ttPv : PvNode || (ttHit && ttData.is_pv);
    ttCapture    = ttData.move && pos.capture_stage(ttData.move);

    // Step 6. Static evaluation of the position
    Value      unadjustedStaticEval = VALUE_NONE;
    const auto correctionValue      = correction_value(*this, pos, ss);
    // Skip early pruning when in check
    if (ss->inCheck)
        ss->staticEval = eval = (ss - 2)->staticEval;
    else if (excludedMove)
        unadjustedStaticEval = eval = ss->staticEval;
    else if (ss->ttHit)
    {
        // Never assume anything about values stored in TT
        unadjustedStaticEval = ttData.eval;
        if (!is_valid(unadjustedStaticEval))
            unadjustedStaticEval = evaluate(pos);

        ss->staticEval = eval = to_corrected_static_eval(unadjustedStaticEval, correctionValue);

        // ttValue can be used as a better position evaluation
        if (is_valid(ttData.value)
            && (ttData.bound & (ttData.value > eval ? BOUND_LOWER : BOUND_UPPER)))
            eval = ttData.value;
    }
    else
    {
        //learning begin
        unadjustedStaticEval = evaluate(pos);
        if (!LD.is_enabled() || !expTTHit || !updatedLearning)
        {
            ss->staticEval = eval = to_corrected_static_eval(unadjustedStaticEval, correctionValue);

            // Static evaluation is saved as it was before adjustment by correction history
            ttWriter.write(posKey, VALUE_NONE, ss->ttPv, BOUND_NONE, DEPTH_UNSEARCHED, Move::none(),
                           unadjustedStaticEval, tt.generation());
        }
        else  // learning
        {
            // Never assume anything on values stored in Global Learning Table
            ss->staticEval = eval = expTTValue;
            if (!is_valid(eval))
            {
                ss->staticEval = eval =
                  to_corrected_static_eval(unadjustedStaticEval, correctionValue);
            }
            if (eval == VALUE_DRAW)
            {
                eval = value_draw(this->nodes);
            }
            // Can expTTValue be used as a better position evaluation?
            if (is_valid(expTTValue))
            {
                eval = expTTValue;
            }
        }
    }
    if (!expectedPVNode)
    {
        // Set up the improving flag, which is true if current static evaluation is
        // bigger than the previous static evaluation at our turn (if we were in
        // check at our previous move we go back until we weren't in check) and is
        // false otherwise. The improving flag is used in various pruning heuristics.
        // Similarly, opponentWorsening is true if our static evaluation is better
        // for us than at the last ply.
        improving = ss->staticEval > (ss - 2)->staticEval;
    }
    // from learning end
    opponentWorsening = ss->staticEval > -(ss - 1)->staticEval;

    // Hindsight adjustment of reductions based on static evaluation difference.
    if (priorReduction >= 3 && !opponentWorsening)
        depth++;
    if (priorReduction >= 2 && depth >= 2 && ss->staticEval + (ss - 1)->staticEval > 173)
        depth--;
    // At non-PV nodes we check for an early TT cutoff
    if (!PvNode && !excludedMove && ttData.depth > depth - (ttData.value <= beta)
        && is_valid(ttData.value)  // Can happen when !ttHit or when access race in probe()
        && (ttData.bound & (ttData.value >= beta ? BOUND_LOWER : BOUND_UPPER))
        && (cutNode == (ttData.value >= beta) || depth > 5))
    {
        // If ttMove is quiet, update move sorting heuristics on TT hit
        if (ttData.move && ttData.value >= beta)
        {
            // Bonus for a quiet ttMove that fails high
            if (!ttCapture)
                update_quiet_histories(pos, ss, *this, ttData.move,
                                       std::min(132 * depth - 72, 985));

            // Extra penalty for early quiet moves of the previous ply
            if (prevSq != SQ_NONE && (ss - 1)->moveCount < 4 && !priorCapture)
                update_continuation_histories(ss - 1, pos.piece_on(prevSq), prevSq, -2060);
        }

        // Partial workaround for the graph history interaction problem
        // For high rule50 counts don't produce transposition table cutoffs.
        if (pos.rule50_count() < 96)
        {
            if (depth >= 8 && ttData.move && pos.pseudo_legal(ttData.move) && pos.legal(ttData.move)
                && !is_decisive(ttData.value))
            {
                pos.do_move(ttData.move, st);
                Key nextPosKey                             = pos.key();
                auto [ttHitNext, ttDataNext, ttWriterNext] = tt.probe(nextPosKey);
                pos.undo_move(ttData.move);

                // Check that the ttValue after the tt move would also trigger a cutoff
                if (!is_valid(ttDataNext.value))
                    return ttData.value;
                if ((ttData.value >= beta) == (-ttDataNext.value >= beta))
                    return ttData.value;
            }
            else
                return ttData.value;
        }
    }
    // from learning begin
    // Step 4Bis. Global Learning Table lookup
    expTTHit        = false;
    updatedLearning = false;

    if (!excludedMove && LD.is_enabled() && useLearning)
    {
        const LearningMove* learningMove = nullptr;
        sibs                             = LD.probeByMaxDepthAndScore(posKey, learningMove);
        if (learningMove)
        {
            assert(sibs);

            enabledLearningProbe = true;
            expTTHit             = true;
            if (!ttData.move)
            {
                ttData.move = learningMove->move;
            }

            if (learningMove->depth >= depth)
            {
                expTTMove       = learningMove->move;
                expTTValue      = learningMove->score;
                updatedLearning = true;
            }

            if ((learningMove->depth == 0))
                updatedLearning = false;

            if (updatedLearning && is_valid(expTTValue))


            {
                if (expTTValue < alpha)
                {
                    disableNMAndPC = true;
                }
                if (expTTValue > alpha && expTTValue < beta)
                {
                    expectedPVNode = true;
                    improving      = true;
                }
            }
            bool expTTCapture = learningMove->move && pos.capture_stage(expTTMove);
            // At this point, if excluded, skip straight to step 6, static eval. However,
            // to save indentation, we list the condition in all code between here and there.
            // At non-PV nodes we check for an early Global Learning Table cutoff
            if (!PvNode && !excludedMove && updatedLearning
                && learningMove->depth > depth - (expTTValue <= beta) && is_valid(expTTValue)
                && (cutNode == (expTTValue >= beta) || depth > 5)
                // avoid a exp cutoff if the rule50 count is high and the exp move is zeroing
                && (depth > 8 || expTTMove == Move::none() || pos.rule50_count() < 80
                    || (!expTTCapture && type_of(pos.moved_piece(expTTMove)) != PAWN)))
            {
                // If expTTMove is quiet, update move sorting heuristics on Global learning table hit
                if (expTTMove && expTTValue >= beta)
                {
                    // Bonus for a quiet ttMove that fails high
                    if (!expTTCapture)
                        update_quiet_histories(pos, ss, *this, ttData.move,
                                               std::min(130 * depth - 71, 1043));

                    // Extra penalty for early quiet moves of
                    // the previous ply
                    if (prevSq != SQ_NONE && (ss - 1)->moveCount < 4 && !priorCapture)
                        update_continuation_histories(ss - 1, pos.piece_on(prevSq), prevSq, -2142);
                }

                // Partial workaround for the graph history interaction problem
                // For high rule50 counts don't produce transposition table cutoffs.
                if (pos.rule50_count() < 96)
                {
                    if (depth >= 8 && ttData.move && pos.pseudo_legal(ttData.move)
                        && pos.legal(ttData.move) && !is_decisive(expTTValue))
                    {
                        do_move(pos, ttData.move, st, nullptr);
                        Key nextPosKey                             = pos.key();
                        auto [ttHitNext, ttDataNext, ttWriterNext] = tt.probe(nextPosKey);
                        undo_move(pos, ttData.move);

                        // Check that the ttValue after the tt move would also trigger a cutoff
                        if (!is_valid(ttDataNext.value))
                            return expTTValue;
                        if ((expTTValue >= beta) == (-ttDataNext.value >= beta))
                            return expTTValue;
                    }
                    else
                        return expTTValue;
                }
            }
        }
    }
    // from learning end

    // Step 5. Tablebases probe
    if (!rootNode && !excludedMove && tbConfig.cardinality)
    {
        int piecesCount = pos.count<ALL_PIECES>();

        if (piecesCount <= tbConfig.cardinality
            && (piecesCount < tbConfig.cardinality || depth >= tbConfig.probeDepth)
            && pos.rule50_count() == 0 && !pos.can_castle(ANY_CASTLING))
        {
            TB::ProbeState err;
            TB::WDLScore   wdl = Tablebases::probe_wdl(pos, &err);

            // Force check of time on the next occasion
            if (is_mainthread())
                main_manager()->callsCnt = 0;

            if (err != TB::ProbeState::FAIL)
            {
                // Preferable over fetch_add to avoid locking instructions
                tbHits.store(tbHits.load(std::memory_order_relaxed) + 1, std::memory_order_relaxed);

                int drawScore = tbConfig.useRule50 ? 1 : 0;

                Value tbValue = VALUE_TB - ss->ply;

                // Use the range VALUE_TB to VALUE_TB_WIN_IN_MAX_PLY to score
                value = wdl < -drawScore ? -tbValue
                      : wdl > drawScore  ? tbValue
                                         : VALUE_DRAW + 2 * wdl * drawScore;

                Bound b = wdl < -drawScore ? BOUND_UPPER
                        : wdl > drawScore  ? BOUND_LOWER
                                           : BOUND_EXACT;

                if (b == BOUND_EXACT || (b == BOUND_LOWER ? value >= beta : value <= alpha))
                {
                    ttWriter.write(posKey, value_to_tt(value, ss->ply), ss->ttPv, b,
                                   std::min(MAX_PLY - 1, depth + 6), Move::none(), VALUE_NONE,
                                   tt.generation());

                    return value;
                }

                if (PvNode)
                {
                    if (b == BOUND_LOWER)
                        bestValue = value, alpha = std::max(alpha, bestValue);
                    else
                        maxValue = value;
                }
            }
        }
    }

    if (ss->inCheck)
        goto moves_loop;

    // Use static evaluation difference to improve quiet move ordering
    if (((ss - 1)->currentMove).is_ok() && !(ss - 1)->inCheck && !priorCapture)
    {
        int evalDiff = std::clamp(-int((ss - 1)->staticEval + ss->staticEval), -209, 167) + 59;
        mainHistory[~us][((ss - 1)->currentMove).raw()] << evalDiff * 9;
        if (!ttHit && type_of(pos.piece_on(prevSq)) != PAWN
            && ((ss - 1)->currentMove).type_of() != PROMOTION)
            sharedHistory.pawn_entry(pos)[pos.piece_on(prevSq)][prevSq] << evalDiff * 13;
    }
    // full threads patch begin
    if (fullSearch)
        goto moves_loop;  // full threads patch
    // full threads patch end

    // Step 7. Razoring
    // If eval is really low, skip search entirely and return the qsearch value.
    // For PvNodes, we must have a guard against mates being returned.
    if (!PvNode)
    {
        // Base razoring threshold (Standard Stockfish formula)
        Value razorThreshold = alpha - 485 - 281 * depth * depth;
#ifdef GOLD_DIGGER
        // FASE 3 MODERATA: Diamo margine (150) ai sacrifici per non potarli troppo presto.
        if (isAggressive || isSacrificial)
            razorThreshold -= 150;
        else if (isStrategical)
            razorThreshold -= 50;
#else
        // --- SHASHIN HYBRID ADJUSTMENTS ---
        // Bilanciamento tra visione tattica e velocità pura.

        if (isAggressive)
        {
            // Tal: Meno pruning (-50) per non perdere combinazioni d'attacco.
            razorThreshold -= 50;
        }
        // Fortezze: Sicurezza extra, non rischiare nulla.
        if (isFortressNode)
        {
            razorThreshold -= 60;
        }
        // Bonus mobilità (Solo se non siamo in modalità difensiva/strategica)
        // Aiuta a trovare risorse in posizioni dinamiche.
        if (legalMoveCount > 30 && !isStrategical)
        {
            razorThreshold -= 20;
        }
        // --- END SHASHIN ADJUSTMENTS ---
#endif
        if (eval < razorThreshold)
            return qsearch<NonPV>(pos, ss, alpha, beta);
    }
    // Step 8. Futility pruning: child node
    // The depth condition is important for mate finding.
    {
#ifdef GOLD_DIGGER
        // FASE 3 MODERATA: Aumentiamo il margine del 50% (150) per esplorare mosse dubbie
        int marginMultiplier = 100;
        if (isAggressive || isSacrificial)
            marginMultiplier = 150;
        else if (isStrategical)
            marginMultiplier = 120;
#else
        // --- SHASHIN ADJUSTMENTS (TIGHTENED) ---
        int marginMultiplier = 100;  // Base Stockfish (100%)

        if (isAggressive)
        {
            // Old: ~75-85%. New: 90%.
            // Riduciamo il margine (cerchiamo di più) ma senza esagerare.
            marginMultiplier = 90;
        }
        else if (isStrategical)
        {
            marginMultiplier = 100;
        }

        // King Danger: fattore critico
        if (staticState.kingDanger)
        {
            // Old: cap a 70. New: cap a 85.
            // Non disabilitare il pruning troppo aggressivamente.
            marginMultiplier = std::min(marginMultiplier, 85);
        }

        // Fortress: Eccezione necessaria
        // Le fortezze richiedono precisione assoluta, manteniamo 60.
        if (isFortressNode)
        {
            marginMultiplier = 60;
        }

        // Adjustment per mobilità (semplificato e sicuro)
        if (isStrategical && legalMoveCount > 30)
        {
            marginMultiplier = std::min(marginMultiplier + 5, 115);
        }
        // --- END SHASHIN ADJUSTMENTS ---
#endif
        auto futility_margin = [&](Depth d) {
            Value futilityMult = 76 - 23 * !ss->ttHit;

            // Applichiamo il moltiplicatore alla base
            futilityMult = (futilityMult * marginMultiplier) / 100;

            // FORMULA STOCKFISH ORIGINALE (Ripristinata)
            // Rimossi i termini custom 'statScore' e 'legalMoveCount' che
            // interferivano con il tuning di Stockfish.
            return futilityMult * d
                 - (2474 * improving + 331 * opponentWorsening) * futilityMult / 1024
                 + std::abs(correctionValue) / 174665;
        };

        if (!ss->ttPv && depth < 14 && eval - futility_margin(depth) >= beta && eval >= beta
            && (!ttData.move || ttCapture) && !is_loss(beta) && !is_win(eval))
        {
            // Ritorno al valore standard di Stockfish.
            // Rimuoviamo le variazioni (3*beta+eval)/4 per stabilità.
            return (2 * beta + eval) / 3;
        }
    }
    // Step 9. Null move search with verification search
    if (cutNode && !disableNMAndPC && ss->staticEval >= beta - 18 * depth + 350 && !excludedMove
        && pos.non_pawn_material(us) && ss->ply >= nmpMinPly && !is_loss(beta))
    {
        // --- SHASHIN HYBRID NMP SAFETY (OPTIMIZED) ---
        bool skipNullMove = false;

#ifdef GOLD_DIGGER
        // FASE 1 RAFFINATA v3 (Paranoid Heat)
        // Obiettivo: Eguagliare la Brutale (142) sui casi critici,
        // ma usare l'intelligenza Heat per i casi "grigi".

        const int HEAT_THRESHOLD = 4;
        int       tacticalHeat   = 0;

        // 1. KILLERS (Trigger Immediati):
        // Se c'è pericolo reale o sacrificio, NON contiamo punti. SALTANO SUBITO.
        // Assegniamo un valore superiore alla soglia per forzare il blocco NMP.
        if (staticState.kingDanger)
            tacticalHeat += 10;
        if (isSacrificial)
            tacticalHeat += 10;

        // 2. FATTORI DI STILE (Accumulo):
        // Qui usiamo la logica intelligente del tuo amico.
        // Servono per catturare posizioni "Tal" che non sono ancora esplose.
        if (dynamicDerived.isHighTal)
            tacticalHeat += 2;
        if (isAggressive)
            tacticalHeat += 2;
        if (staticState.pawnsNearPromotion)
            tacticalHeat += 2;

        // 3. FATTORI DI SUPPORTO
        if (staticState.legalMoveCount > 30)
            tacticalHeat += 1;
        if (pos.non_pawn_material(us) > 2000)
            tacticalHeat += 1;

        // 4. PROFONDITÀ (Il tocco di classe):
        // Più andiamo a fondo, più diventiamo sospettosi del Null Move.
        if (depth > 8)
            tacticalHeat += 1;
        if (depth > 12)
            tacticalHeat += 1;

        // Decisione
        if (tacticalHeat >= HEAT_THRESHOLD)
            skipNullMove = true;

#else
        // LOGICA ORIGINALE SHASHCHESS (MAIN 142)
        // Definiamo i casi "Hot" per la logica standard
        bool nodeHot = staticState.kingDanger || staticState.pawnsNearPromotion || isSacrificial;

        // 1. Sicurezza Tattica
        if (nodeHot || (isAggressive && staticState.kingDanger && depth < 10))
            skipNullMove = true;

        // 2. Sicurezza Difensiva
        if (isPetrosian && staticState.stmKingDanger)
            skipNullMove = true;
#endif
        // ---------------------------------------------

        if (!skipNullMove)
        {
            assert((ss - 1)->currentMove != Move::null());
            assert(eval - beta >= 0);

            Depth R = 7 + depth / 3;

            ss->currentMove                   = Move::none();  // Stockfish standard usa null()
            ss->continuationHistory           = &continuationHistory[0][0][NO_PIECE][0];
            ss->continuationCorrectionHistory = &continuationCorrectionHistory[NO_PIECE][0];

            do_null_move(pos, st, ss);
            nmpGuard        = true;
            Value nullValue = -search<NonPV>(pos, ss + 1, -beta, -beta + 1, depth - R, false);
            nmpGuard        = false;
            undo_null_move(pos);

            if (nullValue >= beta && !is_win(nullValue))
            {
                if (nmpMinPly || depth < 16)
                    return nullValue;
                assert(!nmpMinPly);

                nmpMinPly = ss->ply + 3 * (depth - R) / 4;
                Value v   = search<NonPV>(pos, ss, beta - 1, beta, depth - R, false);
                nmpMinPly = 0;

                if (v >= beta)
                    return nullValue;
            }
        }
    }

    improving |= ss->staticEval >= beta;

    // Step 10. Internal iterative reductions
    // At sufficient depth, reduce depth for PV/Cut nodes without a TTMove.
    // (*Scaler) Making IIR more aggressive scales poorly.
    if (!allNode && depth >= 6 && !ttData.move && priorReduction <= 3)
    {
        Depth originalDepth = depth;

        // Apply base Stockfish reduction
        depth--;

        // Shashin adaptive reduction: apply additional reductions based on position style
        const bool avoidStep10 = localShashinManager.avoidStep10();
        if (quietKingSafe && !avoidStep10
            && originalDepth >= 6)  // Use originalDepth for consistent conditions
        {
            const bool ttReduction = cutNode && ttData.move && ttData.bound == BOUND_UPPER;
            if (ttReduction)
            {
                int reduction = 0;

                // Determine reduction based on Shashin style
                // Strategical positions: more aggressive reduction
                if (isStrategical)
                {
                    reduction = 2;  // More aggressive reduction in strategical positions
                }
                // Tactical positions: preserve depth
                else if (isAggressive || isTacticalReactive || isTactical || isSacrificial)
                {
                    // Preserve depth, but allow some reduction at very high depths
                    if (depth > 12)
                        reduction = 1;
                    else
                        reduction = 0;
                }
                // Default: moderate reduction
                else
                {
                    reduction = 1;
                }

                // Additional adjustments for special conditions
                if (isFortressNode)
                {
                    // Extra reduction in fortresses
                    reduction = std::min(reduction + 1, 3);
                }

                if (gameCycle)
                {
                    reduction = std::max(reduction - 1, 0);  // Less reduction in game cycles
                }

                // Apply the additional reduction, ensuring depth doesn't drop below 1
                depth = std::max(originalDepth - reduction, 1);
            }
        }
    }
    //by Shashin end
    // Step 11. ProbCut
    // If we have a good enough capture (or queen promotion) and a reduced search
    // returns a value much above beta, we can (almost) safely prune the previous move.
    probCutBeta = beta + 235 - 63 * improving;
    if (depth >= 3 && !disableNMAndPC && !ss->inCheck
        && !is_decisive(beta)  //learning
        // If value from transposition table is lower than probCutBeta, don't attempt
        // probCut there
        && !(is_valid(ttData.value) && ttData.value < probCutBeta))
    {
        // Shashin adjustment: skip ProbCut in inappropriate positions
        bool skipProbCut = isFortressNode || (isStrategical && !staticState.kingDanger);

        if (!skipProbCut)
        {
            assert(probCutBeta < VALUE_INFINITE && probCutBeta > beta);

            MovePicker mp(pos, ttData.move, probCutBeta - ss->staticEval, &captureHistory);
            Depth      dynamicReduction = std::max((ss->staticEval - beta) / 315, -1);
            Depth      probCutDepth     = std::clamp(depth - 5 - dynamicReduction, 0, depth);

            // Shashin-based depth adjustments
            if (isCapablanca && depth > 8)
            {
                probCutDepth = std::clamp(depth - 7 - dynamicReduction, 0, depth);
            }
            else
            {
                // Adjust depth based on Shashin style
                if (isStrategical)
                    probCutDepth =
                      std::clamp(probCutDepth - 1, 0, depth);  // Less aggressive in strategical
                else if (isAggressive)
                    probCutDepth =
                      std::clamp(probCutDepth + 1, 0, depth);  // More aggressive in tactical
            }

            while ((move = mp.next_move()) != Move::none())
            {
                assert(move.is_ok());

                if (move == excludedMove || !pos.legal(move))
                    continue;

                assert(pos.capture_stage(move));

                do_move(pos, move, st, ss);

                // Perform a preliminary qsearch to verify that the move holds
                value = -qsearch<NonPV>(pos, ss + 1, -probCutBeta, -probCutBeta + 1);

                // If the qsearch held, perform the regular search
                if (value >= probCutBeta && probCutDepth > 0)
                {
                    value = -search<NonPV>(pos, ss + 1, -probCutBeta, -probCutBeta + 1,
                                           probCutDepth, !cutNode);
                }

                undo_move(pos, move);

                if (value >= probCutBeta)
                {
                    // Save ProbCut data into transposition table
                    ttWriter.write(posKey, value_to_tt(value, ss->ply), ss->ttPv, BOUND_LOWER,
                                   probCutDepth + 1, move, unadjustedStaticEval, tt.generation());

                    if (!is_decisive(value))
                        return value - (probCutBeta - beta);
                }
            }
        }
    }

moves_loop:  // When in check, search starts here

    // Step 12. A small Probcut idea - Balanced Hybrid Safety
    probCutBeta = beta + 418;
    //from Shashin Crystal begin
    const bool commonProbcut = (ttData.bound & BOUND_LOWER) && ttData.depth >= depth - 4
                            && ttData.value >= probCutBeta && !is_decisive(beta)
                            && is_valid(ttData.value) && !is_decisive(ttData.value);

    if (commonProbcut)
    {
        bool tacticalRisk = staticState.kingDanger || staticState.pawnsNearPromotion
                         || isSacrificial || stmKingExposed || opponentKingExposed;

        if (!tacticalRisk || ttData.depth >= depth - 2)
            return probCutBeta;
    }
    //from Shashin Crystal end

    const PieceToHistory* contHist[] = {
      (ss - 1)->continuationHistory, (ss - 2)->continuationHistory, (ss - 3)->continuationHistory,
      (ss - 4)->continuationHistory, (ss - 5)->continuationHistory, (ss - 6)->continuationHistory};


    MovePicker mp(pos, ttData.move, depth, &mainHistory, &lowPlyHistory, &captureHistory, contHist,
                  &sharedHistory, ss->ply);

    value = bestValue;

    int moveCount = 0;

    // Step 13. Loop through all pseudo-legal moves until no moves remain
    // or a beta cutoff occurs.
    while ((move = mp.next_move()) != Move::none())
    {
        assert(move.is_ok());

        if (move == excludedMove)
            continue;

        // Check for legality
        if (!pos.legal(move))
            continue;

        // At root obey the "searchmoves" option and skip moves not listed in Root
        // Move List. In MultiPV mode we also skip PV moves that have been already
        // searched and those of lower "TB rank" if we are in a TB root position.
        if (rootNode && !std::count(rootMoves.begin() + pvIdx, rootMoves.begin() + pvLast, move))
            continue;

        ss->moveCount = ++moveCount;

        if (rootNode && is_mainthread() && nodes > 10000000)
        {
            main_manager()->updates.onIter(
              {depth, UCIEngine::move(move, pos.is_chess960()), moveCount + pvIdx});
        }
        if (PvNode)
            (ss + 1)->pv = nullptr;

        extension  = 0;
        capture    = pos.capture_stage(move);
        movedPiece = pos.moved_piece(move);
        givesCheck = pos.gives_check(move);
        ss->secondaryLine =
          ((rootNode && moveCount > 1)
           || (!ourMove && (ss - 1)->secondaryLine && !excludedMove && moveCount == 1)
           || (ourMove && (ss - 1)->secondaryLine));

        // Calculate new depth for this move
        newDepth = depth - 1;

        int delta = beta - alpha;

        Depth r = reduction(improving, depth, moveCount, delta, isStrategical, isAggressive);

        // Increase reduction for ttPv nodes (*Scaler)
        // Larger values scale well
        if (ss->ttPv)
            r += 946;

        // full threads patch begin
        if (fullSearch)
        {
            goto skipExtensionAndPruning;
        }
        // full threads patch end


        // Step 14. Pruning at shallow depths.
        // Depth conditions are important for mate finding.
        if (!rootNode && pos.non_pawn_material(us) && !is_loss(bestValue))
        {
            // Calcolo soglia base Stockfish
            int moveCountThreshold = (3 + depth * depth) / (2 - improving);

            // --- SHASHIN HYBRID TUNING (MATCH EFFICENCY + PUZZLE VISION) ---
            if (depth > 8)
            {
                if (isStrategical)
                {
                    // CAPABLANCA: Il Turbo posizionale.
                    // Pruning forte (-2) per correre veloci, ma SOLO se siamo profondi
                    // e non ci sono minacce immediate da pedoni.
                    int extraReduction;
#ifdef GOLD_DIGGER
                    extraReduction = (depth > 12 && !staticState.pawnsNearPromotion) ? 2 : 1;
#else
                    extraReduction = 1;
#endif
                    moveCountThreshold -= extraReduction;
                }
                else if (isAggressive || isPetrosian)
                {
                    // TAL & PETROSIAN: La visione d'aquila.
                    // Se c'è attacco o dobbiamo difenderci, guardiamo di più (+1).
                    // Se c'è pericolo imminente (Re in pericolo o Promozione),
                    // allarghiamo drasticamente (+2) per non perdere il treno.
                    int extraExtension = 1;
                    if (staticState.pawnsNearPromotion || staticState.kingDanger)
                    {
                        extraExtension = 2;
                    }
                    moveCountThreshold += extraExtension;
                }
            }
            // -------------------------------------------------------------

            // Skip quiet moves if movecount exceeds our threshold
            if (moveCount >= moveCountThreshold)
                mp.skip_quiet_moves();

            // Reduced depth of the next LMR search
            int lmrDepth = newDepth - r / 1024;

            if (capture || givesCheck)
            {
                Piece capturedPiece = pos.piece_on(move.to_sq());
                int   captHist = captureHistory[movedPiece][move.to_sq()][type_of(capturedPiece)];

                // Futility pruning for captures
                if (!givesCheck && lmrDepth < 7)
                {
                    Value futilityValue = ss->staticEval + 232 + 217 * lmrDepth
                                        + PieceValue[capturedPiece] + 131 * captHist / 1024;

                    if (futilityValue <= alpha)
                        continue;
                }

                // SEE based pruning for captures and checks
                int margin = std::max(166 * depth + captHist / 29, 0);

                if ((alpha >= VALUE_DRAW || pos.non_pawn_material(us) != PieceValue[movedPiece])
                    && !pos.see_ge(move, -margin))
                    continue;
            }
            else
            {
                int history = (*contHist[0])[movedPiece][move.to_sq()]
                            + (*contHist[1])[movedPiece][move.to_sq()]
                            + sharedHistory.pawn_entry(pos)[movedPiece][move.to_sq()];

                // Continuation history based pruning
                if (history < -4083 * depth)
                    continue;

                history += 69 * mainHistory[us][move.raw()] / 32;

                // (*Scaler): Generally, lower divisors scales well
                lmrDepth += history / 3208;

                Value futilityValue = ss->staticEval + 42 + 161 * !bestMove + 127 * lmrDepth
                                    + 85 * (ss->staticEval > alpha);

                // Futility pruning: parent node
                if (!ss->inCheck && lmrDepth < 13 && futilityValue <= alpha)
                {
                    if (bestValue <= futilityValue && !is_decisive(bestValue)
                        && !is_win(futilityValue))
                        bestValue = futilityValue;
                    continue;
                }

                lmrDepth = std::max(lmrDepth, 0);

                // Prune moves with negative SEE
                if (!pos.see_ge(move, -25 * lmrDepth * lmrDepth))
                    continue;
            }
        }

        // Step 15. Extensions
        if (!rootNode && move == ttData.move && !excludedMove && depth >= 6 + ss->ttPv
            && is_valid(ttData.value) && !is_decisive(ttData.value) && (ttData.bound & BOUND_LOWER)
            && ttData.depth >= depth - 3 && !is_shuffling(move, ss, pos))
        {
            Value singularBeta  = ttData.value - (53 + 75 * (ss->ttPv && !PvNode)) * depth / 60;
            Depth singularDepth = newDepth / 2;

            // --- SHASHIN SNIPER EXTENSION (TRUE RADAR) ---
            // Se c'è pericolo (attacco o sacrifici nell'aria) e siamo in fase avanzata,
            // non ci accontentiamo di una ricerca di verifica superficiale.
            // Aumentiamo la profondità del 25% circa per non farci sfuggire combinazioni brutali.
            if ((isAggressive || isSacrificial) && staticState.kingDanger && depth > 8)
            {
                singularDepth += std::max(1, singularDepth / 4);
            }
            // ---------------------------------------------

            ss->excludedMove = move;
            value = search<NonPV>(pos, ss, singularBeta - 1, singularBeta, singularDepth, cutNode);
            ss->excludedMove = Move::none();

            if (value < singularBeta)
            {
                int corrValAdj   = std::abs(correctionValue) / 230673;
                int doubleMargin = -4 + 199 * PvNode - 201 * !ttCapture - corrValAdj
                                 - 897 * ttMoveHistory / 127649 - (ss->ply > rootDepth) * 42;
                int tripleMargin = 73 + 302 * PvNode - 248 * !ttCapture + 90 * ss->ttPv - corrValAdj
                                 - (ss->ply * 2 > rootDepth * 3) * 50;

                extension =
                  1 + (value < singularBeta - doubleMargin) + (value < singularBeta - tripleMargin);
                depth++;
            }
            // Multi-cut pruning
            else if (value >= beta && !is_decisive(value))
            {
                ttMoveHistory << std::max(-400 - 100 * depth, -4000);
                return value;
            }

            // Negative extensions
            else if (ttData.value >= beta)
            {
                extension = -3;
            }
            else if (cutNode)
            {
                extension = -2;
            }
        }

skipExtensionAndPruning:  // full threads search patch
        // Step 16. Make the move
        do_move(pos, move, st, givesCheck, ss);
        bool doLMRStep = !fullSearch;  // Full threads patch

        // Add extension to new depth
        newDepth += extension;
        uint64_t nodeCount = rootNode ? uint64_t(nodes) : 0;

        // Decrease reduction for PvNodes (*Scaler)
        if (ss->ttPv)
            r -= 2719 + PvNode * 983 + (ttData.value > alpha) * 922
               + (ttData.depth >= depth) * (934 + cutNode * 1011);
        // Shashin style based adjustements
        if (!fullSearch)
        {
            // Tactical and sacrificial positions: less reduction to preserve depth
            if ((isTactical || isSacrificial) && depth > 8)
            {
                r -= 256;  // Reduction by 0.5 plies
            }
            // Strategical style: more reduction in quiet phases
            else if (isStrategical && !ss->inCheck && depth > 6)
            {
                r += 256;  // Added reduction by 0.25 plies
            }
        }

        r += 714;  // Base reduction offset to compensate for other tweaks
        r -= moveCount * 73;
        r -= std::abs(correctionValue) / 30370;

        // Increase reduction for cut nodes
        if (cutNode)
            r += 3372 + 997 * !ttData.move;

        // Increase reduction if ttMove is a capture
        if (ttCapture)
            r += 1119;

        // Increase reduction if next ply has a lot of fail high
        //by shashin begin
        if ((ss + 1)->cutoffCnt > 1)
        {
            // Logica tattica: usa formula Alexander originale (meno aggressiva)
            if (dynamicDerived.isHighTal || isTacticalReactive || staticState.isSacrificial)
            {
                // Formula Alexander: conservativa per preservare profondità tattica
                r += 120 + 1024 * ((ss + 1)->cutoffCnt > 2) + 100 * ((ss + 1)->cutoffCnt > 3)
                   + 1024 * allNode;
            }
            // Logica strategica: usa formula Stockfish patch (più aggressiva)
            else if (isStrategical)
            {
                // Formula Stockfish patch: più aggressiva per efficienza
                r += 256 + 1024 * ((ss + 1)->cutoffCnt > 2) + 1024 * allNode;
            }
            // Default: compromesso bilanciato
            else
            {
                r += 180 + 1024 * ((ss + 1)->cutoffCnt > 2) + 75 * ((ss + 1)->cutoffCnt > 3)
                   + 1024 * allNode;
            }
        }
        //by shashin end
        // For first picked move (ttMove) reduce reduction
        if (move == ttData.move)
            r -= 2151;

        if (capture)
            ss->statScore = 868 * int(PieceValue[pos.captured_piece()]) / 128
                          + captureHistory[movedPiece][move.to_sq()][type_of(pos.captured_piece())];
        else
            ss->statScore = 2 * mainHistory[us][move.raw()]
                          + (*contHist[0])[movedPiece][move.to_sq()]
                          + (*contHist[1])[movedPiece][move.to_sq()];

        // Decrease/increase reduction for moves with a good/bad history
        r -= ss->statScore * 850 / 8192;

        // Scale up reductions for expected ALL nodes
        if (allNode)
            r += r / (depth + 1);

        // Step 17. Late moves reduction / extension (LMR)
        if (depth >= 2 && moveCount > std::max(sibs, 1) && doLMRStep)
        {
            int   original_r = r;
            Depth d          = std::max(1, std::min(newDepth - r / 1024, newDepth + 2)) + PvNode;

            // --- SHASHIN MATCH-WINNER TUNING ---
            if (depth > 5 && ss->ply > 1)
            {
                // 1. TAL (Aggressive - Attack): Visione profonda per il colpo del KO.
                if (isAggressive && (givesCheck || capture))
                {
#ifdef GOLD_DIGGER
                    r -= 60;  // Profondità extra per trovare il colpo del KO
#else
                    r -= 50;
#endif
                }
#ifdef GOLD_DIGGER
                else if (isPetrosian && !capture)
                {
                    // La difesa richiede una precisione millimetrica.
                    // Non tagliamo, anzi diamo un po' di respiro.
                    r -= 40;
                }
#else
                // 2. PETROSIAN (Defensive - Tactics): Precisione assoluta.
                // Se siamo in difesa (Petrosian) e la mossa non è una cattura,
                // riduciamo il taglio per non perdere la linea di salvezza.
                else if (isPetrosian && !capture && !givesCheck)
                {
                    r -= 50;  // Protezione tattica difensiva
                }
#endif
                // 3. CAPABLANCA (Strategic - Maneuver)
                else if (isStrategical && !capture && !givesCheck && depth > 8)
                {
#ifdef GOLD_DIGGER
                    // IL MOTORE DEL MATCH: Posizione chiusa/noiosa.
                    // Falciata in puro stile Stockfish Main (+80) per correre velocissimi.
                    r += 80;
#else
                    r += 45;
#endif
                }

                // Ricalcoliamo 'd' (profondità ridotta) solo se 'r' è cambiato
                if (r != original_r)
                {
                    d = std::max(1, std::min(newDepth - r / 1024, newDepth + 2)) + PvNode;
                }
            }

            // Fortezze: Riduzione minima per non sbagliare i finali
            if (isFortressNode)
            {
                d = std::max(1, d - 1);
            }
            // ------------------------------------------

            ss->reduction = newDepth - d;
            value         = -search<NonPV>(pos, ss + 1, -(alpha + 1), -alpha, d, true);
            ss->reduction = 0;

            // Do a full-depth search when reduced LMR search fails high
            if (value > alpha)
            {
                const bool doDeeperSearch    = d < newDepth && value > bestValue + 50;
                const bool doShallowerSearch = value < bestValue + 9;

                // Standard Stockfish re-search logic (Crystal Logic rimossa per stabilità)
                newDepth += doDeeperSearch - doShallowerSearch;
                if (newDepth > d)
                    value = -search<NonPV>(pos, ss + 1, -(alpha + 1), -alpha, newDepth, !cutNode);

                // --- MODIFICA A: BONUS HISTORY AUMENTATI ---
                // Aumentiamo leggermente i bonus per orientare l'apprendimento della history
                // senza stravolgere l'ordinamento immediato.
                int bonus = 1365;
                if (isAggressive)
                    bonus += 25;  // Da +40 a +25 (-37%)
                else if (isStrategical)
                    bonus += 12;  // Da +20 a +12 (-40%)
                if (isFortressNode)
                    bonus += 30;  // Da +60 a +30 (-50%)

                // ------------------------------------------
                update_continuation_histories(ss, movedPiece, move.to_sq(), bonus);
            }
        }

        // Step 18. Full-depth search when LMR is skipped
        else if (!PvNode || moveCount > 1)
        {
            // Increase reduction if ttMove is not present
            if (!ttData.move)
                r += 1140;

            // Note that if expected reduction is high, we reduce search depth here
            value = -search<NonPV>(pos, ss + 1, -(alpha + 1), -alpha,
                                   newDepth - (r > 3957) - (r > 5654 && newDepth > 2), !cutNode);
        }

        // For PV nodes only, do a full PV search on the first move or after a fail high,
        // otherwise let the parent node fail low with value <= alpha and try another move.
        if (PvNode && (moveCount == 1 || value > alpha))
        {
            (ss + 1)->pv    = pv;
            (ss + 1)->pv[0] = Move::none();

            // Extend move from transposition table if we are about to dive into qsearch.
            // decisive score handling improves mate finding and retrograde analysis.
            if (move == ttData.move
                && ((is_valid(ttData.value) && is_decisive(ttData.value) && ttData.depth > 0)
                    || ttData.depth > 1))
                newDepth = std::max(newDepth, 1);

            value = -search<PV>(pos, ss + 1, -beta, -alpha, newDepth, false);
        }

        // Step 19. Undo move
        undo_move(pos, move);

        assert(value > -VALUE_INFINITE && value < VALUE_INFINITE);

        // Step 20. Check for a new best move
        // Finished searching the move. If a stop occurred, the return value of
        // the search cannot be trusted, and we return immediately without updating
        // best move, principal variation nor transposition table.
        if (threads.stop.load(std::memory_order_relaxed))
            return VALUE_ZERO;

        if (rootNode)
        {
            RootMove& rm = *std::find(rootMoves.begin(), rootMoves.end(), move);

            rm.effort += nodes - nodeCount;

            rm.averageScore =
              rm.averageScore != -VALUE_INFINITE ? (value + rm.averageScore) / 2 : value;

            rm.meanSquaredScore = rm.meanSquaredScore != -VALUE_INFINITE * VALUE_INFINITE
                                  ? (value * std::abs(value) + rm.meanSquaredScore) / 2
                                  : value * std::abs(value);

            // PV move or new best move?
            if (moveCount == 1 || value > alpha)
            {
                rm.score = rm.uciScore = value;
                rm.selDepth            = selDepth;
                rm.scoreLowerbound = rm.scoreUpperbound = false;

                if (value >= beta)
                {
                    rm.scoreLowerbound = true;
                    rm.uciScore        = beta;
                }
                else if (value <= alpha)
                {
                    rm.scoreUpperbound = true;
                    rm.uciScore        = alpha;
                }

                rm.pv.resize(1);

                assert((ss + 1)->pv);

                for (Move* m = (ss + 1)->pv; *m != Move::none(); ++m)
                    rm.pv.push_back(*m);

                // We record how often the best move has been changed in each iteration.
                // This information is used for time management. In MultiPV mode,
                // we must take care to only do this for the first PV line.
                if (moveCount > 1 && !pvIdx)
                    ++bestMoveChanges;
            }
            else
                // All other moves but the PV, are set to the lowest value: this
                // is not a problem when sorting because the sort is stable and the
                // move position in the list is preserved - just the PV is pushed up.
                rm.score = -VALUE_INFINITE;
        }

        // In case we have an alternative move equal in eval to the current bestmove,
        // promote it to bestmove by pretending it just exceeds alpha (but not beta).
        int inc = (value == bestValue && ss->ply + 2 >= rootDepth && (int(nodes) & 14) == 0
                   && !is_win(std::abs(value) + 1));

        if (value + inc > bestValue)
        {
            bestValue = value;

            if (value + inc > alpha)
            {
                bestMove = move;

                if (PvNode && !rootNode)  // Update pv even in fail-high case
                    update_pv(ss->pv, move, (ss + 1)->pv);

                if (value >= beta)
                {
                    // (*Scaler) Infrequent and small updates scale well
                    ss->cutoffCnt += (extension < 2) || PvNode;
                    assert(value >= beta);  // Fail high
                    break;
                }

                //from crystal-shashin begin
                // Reduce other moves if we have found at least one score improvement
                if (depth > 2 && depth < 14 && !is_decisive(value))
                {
                    const int baseReduction = 2;
                    int       reduction     = baseReduction;

                    // Specific tactical conditions for style (Shashin-correct)
                    const bool criticalTactical =
                      (opponentKingExposed && isTal) || (stmKingExposed && isPetrosian);

                    // Hybrid Shashin Strategy
                    if (criticalTactical)
                    {
                        // Depth preserved in critical positions for style
                        reduction = 1;
                    }
                    else if (isStrategical && !stmKingExposed && !opponentKingExposed)
                    {
                        // Positional scenario: more aggressive reduction
                        reduction = (depth > 8) ? 3 : 2;

                        // Extra caution in congested positions
                        if (staticState.allPiecesCount > 20)
                        {
                            reduction = std::min(reduction, 2);
                        }
                    }
                    else if (gameCycle)
                    {
                        // Cycle management: balancing safety and progress
                        reduction = (pos.rule50_count() > 20) ? 3 : 1;
                    }

                    // Adaptive safety clamp
                    reduction = std::clamp(reduction, 1, depth - 1);

                    depth -= reduction;
                }
                //from crystal-shashin end

                assert(depth > 0);
                alpha = value;  // Update alpha! Always alpha < beta
            }
        }

        // If the move is worse than some previously searched move,
        // remember it, to update its stats later.
        if (move != bestMove && moveCount <= SEARCHEDLIST_CAPACITY)
        {
            if (capture)
                capturesSearched.push_back(move);
            else
                quietsSearched.push_back(move);
        }
    }

    // Step 21. Check for mate and stalemate
    // All legal moves have been searched and if there are no legal moves, it
    // must be a mate or a stalemate. If we are in a singular extension search then
    // return a fail low score.

    assert(moveCount || !ss->inCheck || excludedMove || !MoveList<LEGAL>(pos).size());

    // Adjust best value for fail high cases
    if (bestValue >= beta && !is_decisive(bestValue) && !is_decisive(alpha))
        bestValue = (bestValue * depth + beta) / (depth + 1);

    if (!moveCount)
        bestValue = excludedMove ? alpha : ss->inCheck ? mated_in(ss->ply) : VALUE_DRAW;

    // If there is a move that produces search value greater than alpha,
    // we update the stats of searched moves.
    else if (bestMove)
    {
        update_all_stats(pos, ss, *this, bestMove, prevSq, quietsSearched, capturesSearched, depth,
                         ttData.move, moveCount);
        if (!PvNode)
            ttMoveHistory << (bestMove == ttData.move ? 809 : -865);
    }

    // Bonus for prior quiet countermove that caused the fail low
    else if (!priorCapture && prevSq != SQ_NONE)
    {
        int bonusScale = -215;
        bonusScale -= (ss - 1)->statScore / 100;
        bonusScale += std::min(56 * depth, 489);
        bonusScale += 184 * ((ss - 1)->moveCount > 8);
        bonusScale += 147 * (!ss->inCheck && bestValue <= ss->staticEval - 107);
        bonusScale += 156 * (!(ss - 1)->inCheck && bestValue <= -(ss - 1)->staticEval - 65);

        bonusScale = std::max(bonusScale, 0);

        // scaledBonus ranges from 0 to roughly 2.3M, overflows happen for multipliers larger than 900
        const int scaledBonus = std::min(141 * depth - 87, 1351) * bonusScale;

        update_continuation_histories(ss - 1, pos.piece_on(prevSq), prevSq,
                                      scaledBonus * 406 / 32768);

        mainHistory[~us][((ss - 1)->currentMove).raw()] << scaledBonus * 243 / 32768;

        if (type_of(pos.piece_on(prevSq)) != PAWN && ((ss - 1)->currentMove).type_of() != PROMOTION)
            sharedHistory.pawn_entry(pos)[pos.piece_on(prevSq)][prevSq] << scaledBonus * 290 / 8192;
    }

    // Bonus for prior capture countermove that caused the fail low
    else if (priorCapture && prevSq != SQ_NONE)
    {
        Piece capturedPiece = pos.captured_piece();
        assert(capturedPiece != NO_PIECE);
        captureHistory[pos.piece_on(prevSq)][prevSq][type_of(capturedPiece)] << 1012;
    }

    if (PvNode)
        bestValue = std::min(bestValue, maxValue);

    // If no good move is found and the previous position was ttPv, then the previous
    // opponent move is probably good and the new position is added to the search tree.
    if (bestValue <= alpha)
        ss->ttPv = ss->ttPv || (ss - 1)->ttPv;

    // Write gathered information in transposition table. Note that the
    // static evaluation is saved as it was before correction history.
    if (!excludedMove && !(rootNode && pvIdx))
        ttWriter.write(posKey, value_to_tt(bestValue, ss->ply), ss->ttPv,
                       bestValue >= beta    ? BOUND_LOWER
                       : PvNode && bestMove ? BOUND_EXACT
                                            : BOUND_UPPER,
                       moveCount != 0 ? depth : std::min(MAX_PLY - 1, depth + 6), bestMove,
                       unadjustedStaticEval, tt.generation());

    // Adjust correction history if the best move is not a capture
    // and the error direction matches whether we are above/below bounds.
    if (!ss->inCheck && !(bestMove && pos.capture(bestMove))
        && (bestValue > ss->staticEval) == bool(bestMove))
    {
        auto bonus = std::clamp(int(bestValue - ss->staticEval) * depth / (bestMove ? 10 : 8),
                                -CORRECTION_HISTORY_LIMIT / 4, CORRECTION_HISTORY_LIMIT / 4);
        update_correction_history(pos, ss, *this, bonus);
    }

    assert(bestValue > -VALUE_INFINITE && bestValue < VALUE_INFINITE);

    return bestValue;
}


// Quiescence search function, which is called by the main search function with
// depth zero, or recursively with further decreasing depth. With depth <= 0, we
// "should" be using static eval only, but tactical moves may confuse the static eval.
// To fight this horizon effect, we implement this qsearch of tactical moves.
// See https://www.chessprogramming.org/Horizon_Effect
// and https://www.chessprogramming.org/Quiescence_Search
template<NodeType nodeType>
Value Search::Worker::qsearch(Position& pos, Stack* ss, Value alpha, Value beta) {

    static_assert(nodeType != Root);
    const RootShashinState& rootShashinState = shashinManager->getState();
    constexpr bool          PvNode           = nodeType == PV;

    assert(alpha >= -VALUE_INFINITE && alpha < beta && beta <= VALUE_INFINITE);
    assert(PvNode || (alpha == beta - 1));

    // CACHE NEI REGISTRI per la qsearch (critico per gli NPS)
    const bool antiDancing = antiDancingMode;

    // Check if we have an upcoming move that draws by repetition
    if ((alpha < VALUE_DRAW || (antiDancing && true)) && pos.upcoming_repetition(ss->ply))
    {
        // BRANCH HINTING: sposta fisicamente nel codice macchina questo blocco
        // in un'area "fredda" della memoria eseguibile.
        if (antiDancing && alpha >= VALUE_DRAW) [[unlikely]]
        {
            return value_draw(nodes);
        }

        // Comportamento Standard
        if (alpha < VALUE_DRAW)
        {
            alpha = value_draw(nodes);
            if (alpha >= beta)
                return alpha;
        }
    }

    Move      pv[MAX_PLY + 1];
    StateInfo st;

    Key   posKey;
    Move  move, bestMove, expTTMove = Move::none();                               //learning
    Value bestValue, value, futilityBase, expTTValue = VALUE_NONE;                //learning
    bool  pvHit, givesCheck, capture, expTTHit = false, updatedLearning = false;  //learning
    int   moveCount;

    // Step 1. Initialize node
    if (PvNode)
    {
        (ss + 1)->pv = pv;
        ss->pv[0]    = Move::none();
    }

    bestMove    = Move::none();
    ss->inCheck = pos.checkers();
    moveCount   = 0;

    // Used to send selDepth info to GUI (selDepth counts from 1, ply from 0)
    if (PvNode && selDepth < ss->ply + 1)
        selDepth = ss->ply + 1;

    // Step 2. Check for an immediate draw or maximum ply reached
    if (pos.is_draw(ss->ply) || ss->ply >= MAX_PLY)
        return (ss->ply >= MAX_PLY && !ss->inCheck) ? evaluate(pos) : VALUE_DRAW;

    assert(0 <= ss->ply && ss->ply < MAX_PLY);

    // Step 3. Transposition table lookup
    posKey                         = pos.key();
    auto [ttHit, ttData, ttWriter] = tt.probe(posKey);
    // Need further processing of the saved data
    ss->ttHit    = ttHit;
    ttData.move  = ttHit ? ttData.move : Move::none();
    ttData.value = ttHit ? value_from_tt(ttData.value, ss->ply, pos.rule50_count()) : VALUE_NONE;
    pvHit        = ttHit && ttData.is_pv;

    // Crystal-Shashin QSearch optimization begin (Early Exit logic)
    const auto& staticState    = rootShashinState.staticState;
    const auto& dynamicDerived = rootShashinState.dynamicDerived;

    // Accesso ottimizzato alle variabili di stato
    const bool isStrategical = dynamicDerived.isStrategical;
    const bool isAggressive  = dynamicDerived.isAggressive;

    if (!PvNode && is_valid(ttData.value)
        && (ttData.bound & (ttData.value >= beta ? BOUND_LOWER : BOUND_UPPER)))
    {
        if (ttData.depth >= DEPTH_QS)
            return ttData.value;

        // Logic for Quiet Kings early cutoff
        if (staticState.legalMoveCount < 35 && std::abs(ttData.value) > 60)
        {
            const bool areQuietKings =
              !staticState.stmKingExposed && !staticState.opponentKingExposed;
            if (!isStrategical && areQuietKings)
            {
                const bool depthCondition =
                  ttData.depth >= ADJUSTED_QS_DEPTH && ttData.depth > MIN_DEPTH_FOR_QS_CUTOFF;
                const bool kingSafety =
                  (!staticState.stmKingExposed && !staticState.opponentKingExposed)
                  || ttData.depth > DEPTH_QS + 4;

                if (depthCondition && kingSafety)
                    return ttData.value;
            }
        }
    }
    // Crystal-Shashin QSearch optimization end
    // Step 3 bis learning begin
    expTTHit        = false;
    updatedLearning = false;

    if (useLearning && LD.is_enabled())
    {
        const LearningMove* learningMove = nullptr;
        int                 siblings     = LD.probeByMaxDepthAndScore(posKey, learningMove);
        (void) siblings;

        if (learningMove)
        {
            expTTHit   = true;
            expTTMove  = learningMove->move;
            expTTValue = learningMove->score;

            if (!ttHit)
            {
                ttData.move = expTTMove;
            }
            const bool canUseExpTTValue = !PvNode && is_valid(expTTValue);
            if (canUseExpTTValue)
            {
                if (learningMove->depth >= DEPTH_QS)
                {
                    updatedLearning = true;
                    return expTTValue;
                }
                // (Logica Learning per Quiet Kings omessa per brevità,
                // mantenuta identica al blocco TT se necessario, o saltare allo step successivo)
                // Qui mantengo il flusso originale per coerenza col tuo file
                if (staticState.legalMoveCount < 35 && std::abs(expTTValue) > 60)
                {
                    const bool areQuietKings =
                      !staticState.stmKingExposed && !staticState.opponentKingExposed;
                    if (!isStrategical && areQuietKings)
                    {
                        const bool depthCondition = learningMove->depth >= ADJUSTED_QS_DEPTH
                                                 && learningMove->depth > MIN_DEPTH_FOR_QS_CUTOFF;
                        const bool kingSafety =
                          (!staticState.stmKingExposed && !staticState.opponentKingExposed)
                          || learningMove->depth > DEPTH_QS + 4;
                        if (depthCondition && kingSafety)
                        {
                            updatedLearning = true;
                            return expTTValue;
                        }
                    }
                }
            }
        }
    }
    // Step 3 bis learning end
    // Step 4. Static evaluation of the position
    Value unadjustedStaticEval = VALUE_NONE;
    if (ss->inCheck)
        bestValue = futilityBase = -VALUE_INFINITE;
    else
    {
        const auto correctionValue = correction_value(*this, pos, ss);

        if (ss->ttHit)
        {
            unadjustedStaticEval = ttData.eval;
            if (!is_valid(unadjustedStaticEval))
                unadjustedStaticEval = evaluate(pos);
            ss->staticEval = bestValue =
              to_corrected_static_eval(unadjustedStaticEval, correctionValue);

            if (is_valid(ttData.value) && !is_decisive(ttData.value)
                && (ttData.bound & (ttData.value > bestValue ? BOUND_LOWER : BOUND_UPPER)))
                bestValue = ttData.value;
        }
        else
        {
            if (expTTHit && updatedLearning && is_valid(expTTValue))
            {
                ss->staticEval = bestValue = expTTValue;
            }
            else
            {
                unadjustedStaticEval = evaluate(pos);
                ss->staticEval       = bestValue =
                  to_corrected_static_eval(unadjustedStaticEval, correctionValue);
            }
        }

        // Stand pat. Return immediately if static value is at least beta
        if (bestValue >= beta)
        {
            if (!is_decisive(bestValue))
                bestValue = (bestValue + beta) / 2;
            if (!ss->ttHit)
                ttWriter.write(posKey, value_to_tt(bestValue, ss->ply), false, BOUND_LOWER,
                               DEPTH_UNSEARCHED, Move::none(), unadjustedStaticEval,
                               tt.generation());
            return bestValue;
        }

        if (bestValue > alpha)
            alpha = bestValue;

        futilityBase = ss->staticEval + 351;

        // --- SHASHIN OPTIMIZATION 1: FUTILITY BASE (Outside Loop) ---
        // Correggiamo il valore ridicolo (+3) con qualcosa di sensato.
        if (dynamicDerived.isHighTal)
            futilityBase += 30;
        else if (isStrategical)
            futilityBase -= 20;
        // ------------------------------------------------------------
    }

    const PieceToHistory* contHist[] = {(ss - 1)->continuationHistory};
    Square prevSq = ((ss - 1)->currentMove).is_ok() ? ((ss - 1)->currentMove).to_sq() : SQ_NONE;

    MovePicker mp(pos, ttData.move, DEPTH_QS, &mainHistory, &lowPlyHistory, &captureHistory,
                  contHist, &sharedHistory, ss->ply);

    // --- SHASHIN OPTIMIZATION 2: PRE-CALCULATE CONSTANTS (Outside Loop) ---
    // A) Move Count Threshold
    int moveCountThreshold = 2;  // Default Stockfish
    if (dynamicDerived.isHighTal)
        moveCountThreshold += PvNode ? 1 : 0;
    else if (isStrategical)
        moveCountThreshold = 1;
    if (staticState.kingDanger)
        moveCountThreshold++;
    if (MoveConfig::isFortress)
        moveCountThreshold++;

    // B) SEE Margin
    int seeMargin = -80;  // Standard Stockfish
    if (isAggressive || dynamicDerived.isHighTal)
    {
        // TAL: Visione Tattica Estesa
        // -110 permette di vedere i sacrifici di pezzo speculativi ("Tal Magic").
        seeMargin = staticState.kingDanger ? -110 : -90;
    }
    else if (isStrategical)
    {
        // PETROSIAN: Taglio netto per sfruttare l'NPS alto.
        seeMargin = -50;
    }
    else if (MoveConfig::isFortress)
    {
        seeMargin = -10;
    }
    // C) Volatility Check (Flag)
    const bool extremelyVolatile = dynamicDerived.isHighTal && staticState.kingDanger;
    // ----------------------------------------------------------------------

    // Step 5. Loop through all pseudo-legal moves
    while ((move = mp.next_move()) != Move::none())
    {
        assert(move.is_ok());
        Piece movedPiece = pos.moved_piece(move);

        if (!pos.legal(move))
            continue;

        givesCheck = pos.gives_check(move);
        capture    = pos.capture_stage(move);

        moveCount++;

        // Step 6. Pruning (Optimized)
        if (!is_loss(bestValue))
        {
            // Futility pruning and moveCount pruning
            if (!givesCheck && move.to_sq() != prevSq && !is_loss(futilityBase)
                && move.type_of() != PROMOTION)
            {
                // Use pre-calculated threshold
                if (moveCount > moveCountThreshold)
                    continue;

                Value futilityValue = futilityBase + PieceValue[pos.piece_on(move.to_sq())];

                if (futilityValue <= alpha)
                {
                    bestValue = std::max(bestValue, futilityValue);
                    continue;
                }

                // === PATCH SHASHIN INTELLIGENTE (LOGICA INVERTITA) ===
                if (!pos.see_ge(move, alpha - futilityBase))
                {
                    // Logica tattica (vecchio comportamento) per:
                    // 1. Stili Tal aggressivi
                    // 2. Posizioni tattiche reattive
                    // 3. Pericolo del re (tranne in posizioni strategiche)
                    const bool useTacticalLogic = dynamicDerived.isHighTal
                                               || dynamicDerived.isTacticalReactive
                                               || (staticState.kingDanger && !isStrategical);

                    if (useTacticalLogic)
                    {
                        // COMPORTAMENTO TATTICO (Alexander originale)
                        // Permette alla valutazione di scendere per esplorare mosse apparentemente negative
                        // ma potenzialmente vincenti (sacrifici, combinazioni nascoste)
                        bestValue = std::min(alpha, futilityBase);
                    }
                    else
                    {
                        // COMPORTAMENTO STRATEGICO (patch Stockfish)
                        // Mantiene la valutazione stabile, evitando crolli ingiustificati
                        // Ottimo per posizioni strategiche e partite lunghe
                        bestValue = std::max(bestValue, std::min(alpha, futilityBase));
                    }
                    continue;
                }
            }

            // --- PHASE 4: HYBRID PRUNING OPTIMIZATION ---

            if (!capture)
            {
                if (extremelyVolatile)  // Use pre-calculated flag
                {
                    // Accesso costoso alla sharedHistory solo qui, nel caso critico
                    int historyScore =
                      sharedHistory.pawn_entry(pos)[pos.moved_piece(move)][move.to_sq()];

                    // Soglia fissa a 4000 (valore di sicurezza per mosse buone)
                    if (historyScore < 4000)
                        continue;
                }
                else
                {
                    // COMPORTAMENTO STANDARD STOCKFISH:
                    // Se non è una cattura e non siamo in 'extreme volatility', potiamo tutto.
                    continue;
                }
            }
            // --- END PHASE 4 ---
            // --- SHASHIN TACTICAL EXCEPTION (BALANCED) ---
            bool forceCheckSearch = false;

            // 1. CONTESTO: Attiviamo su TUTTI gli Aggressive (non solo HighTal)
            // se c'è KingDanger, altrimenti sprechiamo tempo.
            if (givesCheck
                && (dynamicDerived.isHighTal || (isAggressive && staticState.kingDanger)))
            {
                // 2. SICUREZZA MATERIALE:
                // Accettiamo sacrifici fino a -200 (pezzo leggero), non oltre.
                // Questo evita di esplorare "buchi neri" tattici irrecuperabili.
                if (pos.see_ge(move, -200))
                {
                    // 3. FILTRO SUICIDIO (Standard):
                    if (type_of(movedPiece) != QUEEN || type_of(pos.piece_on(move.to_sq())) != PAWN)
                        forceCheckSearch = true;
                }
            }

            // SEE pruning
            if (!forceCheckSearch && !pos.see_ge(move, seeMargin))
                continue;
        }

        // Step 7. Make and search the move
        do_move(pos, move, st, givesCheck, ss);

        value = -qsearch<nodeType>(pos, ss + 1, -beta, -alpha);
        undo_move(pos, move);

        assert(value > -VALUE_INFINITE && value < VALUE_INFINITE);

        // Step 8. Check for a new best move
        if (value > bestValue)
        {
            bestValue = value;

            if (value > alpha)
            {
                bestMove = move;

                if (PvNode)  // Update pv even in fail-high case
                    update_pv(ss->pv, move, (ss + 1)->pv);

                if (value < beta)  // Update alpha here!
                    alpha = value;
                else
                    break;  // Fail high
            }
        }
    }

    // variety begin
    if (std::abs(bestValue) == VALUE_INFINITE)
    {
        variety = 0;
    }

    if (variety != 0)
    {
        Value       maxIncrement = (variety == 1) ? 13 : 309;
        static PRNG rng(now());
        if (((variety == 2) && (bestValue <= 309) && (bestValue >= -309))
            || ((variety == 1) && (bestValue <= 13) && (bestValue >= -13)))
        {
            int maxValidIncrement = maxIncrement - std::abs(bestValue);
            if (maxValidIncrement < 0)
            {
                maxValidIncrement = 0;
            }
            int increment = static_cast<int>(rng.rand<uint64_t>() % (maxValidIncrement + 1));
            bestValue += increment;
        }
    }
    // end variety
    // Step 9. Check for mate
    if (ss->inCheck && bestValue == -VALUE_INFINITE)
    {
        assert(!MoveList<LEGAL>(pos).size());
        return mated_in(ss->ply);  // Plies to mate from the root
    }

    if (!is_decisive(bestValue) && bestValue > beta)
        bestValue = (bestValue + beta) / 2;


    Color us = pos.side_to_move();
    if (!ss->inCheck && !moveCount && !pos.non_pawn_material(us)
        && type_of(pos.captured_piece()) >= ROOK)
    {
        if (!((us == WHITE ? shift<NORTH>(pos.pieces(us, PAWN))
                           : shift<SOUTH>(pos.pieces(us, PAWN)))
              & ~pos.pieces()))  // no pawn pushes available
        {
            pos.state()->checkersBB = Rank1BB;  // search for legal king-moves only
            if (!MoveList<LEGAL>(pos).size())   // stalemate
                bestValue = VALUE_DRAW;
            pos.state()->checkersBB = 0;
        }
    }

    // Save gathered info in transposition table.
    ttWriter.write(posKey, value_to_tt(bestValue, ss->ply), pvHit,
                   bestValue >= beta ? BOUND_LOWER : BOUND_UPPER, DEPTH_QS, bestMove,
                   unadjustedStaticEval, tt.generation());

    assert(bestValue > -VALUE_INFINITE && bestValue < VALUE_INFINITE);

    return bestValue;
}

Depth Search::Worker::reduction(
  bool i, Depth d, int mn, int delta, bool isStrategical, bool isAggressive) const {
    int reductionScale = reductions[d] * reductions[mn];
#ifdef GOLD_DIGGER
    // --- SHASHIN STYLE LOGIC (SYMMETRICAL & TARGETED) ---

    // 1. TAL (Aggressive - Middle Tal):
    // La fase critica della lotta. Riduciamo la riduzione (-5%) per aumentare la profondità.
    // Nota: High Tal non è incluso, ed è giusto così: se stiamo già vincendo nettamente,
    // non serve rallentare la ricerca, basta l'efficienza standard.
    if (isAggressive)
        reductionScale = (reductionScale * 97) / 100;

    // 2. PETROSIAN (Strategical - Capablanca):
    // La fase di manovra/consolidamento.
    // Torniamo a 103% per massima stabilità nei match lunghi.
    else if (isStrategical)
        reductionScale = (reductionScale * 103) / 100;
#else
    // --- SHASHIN STYLE TUNING ---

    // 1. TAL (Aggressive): 98%
    // Mantiene la velocità della Prerelease ma con un occhio di riguardo alla tattica.
    if (isAggressive)
        reductionScale = (reductionScale * 98) / 100;

    // 2. CAPABLANCA (Strategical Puro): 104%
    // Il segreto per battere SF18 nei match posizionali. Tagliare di più per andare più a fondo.
    else if (isStrategical)
        reductionScale = (reductionScale * 104) / 100;

    // 3. PETROSIAN (Defensive): 99%
    // Leggero aiuto (1% in meno di riduzione) per calcolare meglio le difese.
    else if (shashinManager->isPetrosian())
        reductionScale = (reductionScale * 99) / 100;
    // -----------------------------------------
#endif
    //mcts begin
    if (rootDelta != 0)
        return reductionScale - delta * 608 / rootDelta + !i * reductionScale * 238 / 512 + 1182;
    else  // avoid divide by zero error
        return reductionScale - delta * 608 + !i * reductionScale * 238 / 512 + 1182;
    //mcts end
}

// elapsed() returns the time elapsed since the search started. If the
// 'nodestime' option is enabled, it will return the count of nodes searched
// instead. This function is called to check whether the search should be
// stopped based on predefined thresholds like time limits or nodes searched.
//
// elapsed_time() returns the actual time elapsed since the start of the search.
// This function is intended for use only when printing PV outputs, and not used
// for making decisions within the search algorithm itself.
TimePoint Search::Worker::elapsed() const {
    return main_manager()->tm.elapsed([this]() { return threads.nodes_searched(); });
}

TimePoint Search::Worker::elapsed_time() const { return main_manager()->tm.elapsed_time(); }

Value Search::Worker::evaluate(const Position& pos) { return Eval::evaluate(pos); }

namespace {
// Adjusts a mate or TB score from "plies to mate from the root" to
// "plies to mate from the current position". Standard scores are unchanged.
// The function is called before storing a value in the transposition table.
Value value_to_tt(Value v, int ply) { return is_win(v) ? v + ply : is_loss(v) ? v - ply : v; }


// Inverse of value_to_tt(): it adjusts a mate or TB score from the transposition
// table (which refers to the plies to mate/be mated from current position) to
// "plies to mate/be mated (TB win/loss) from the root". However, to avoid
// potentially false mate or TB scores related to the 50 moves rule and the
// graph history interaction, we return the highest non-TB score instead.
Value value_from_tt(Value v, int ply, int r50c) {

    if (!is_valid(v))
        return VALUE_NONE;

    // handle TB win or better
    if (is_win(v))
    {
        // Downgrade a potentially false mate score
        if (v >= VALUE_MATE_IN_MAX_PLY && VALUE_MATE - v > 100 - r50c)
            return VALUE_TB_WIN_IN_MAX_PLY - 1;

        // Downgrade a potentially false TB score.
        if (VALUE_TB - v > 100 - r50c)
            return VALUE_TB_WIN_IN_MAX_PLY - 1;

        return v - ply;
    }

    // handle TB loss or worse
    if (is_loss(v))
    {
        // Downgrade a potentially false mate score.
        if (v <= VALUE_MATED_IN_MAX_PLY && VALUE_MATE + v > 100 - r50c)
            return VALUE_TB_LOSS_IN_MAX_PLY + 1;

        // Downgrade a potentially false TB score.
        if (VALUE_TB + v > 100 - r50c)
            return VALUE_TB_LOSS_IN_MAX_PLY + 1;

        return v + ply;
    }

    return v;
}


// Adds current move and appends child pv[]
void update_pv(Move* pv, Move move, const Move* childPv) {

    for (*pv++ = move; childPv && *childPv != Move::none();)
        *pv++ = *childPv++;
    *pv = Move::none();
}


// Updates stats at the end of search() when a bestMove is found
void update_all_stats(const Position& pos,
                      Stack*          ss,
                      Search::Worker& workerThread,
                      Move            bestMove,
                      Square          prevSq,
                      SearchedList&   quietsSearched,
                      SearchedList&   capturesSearched,
                      Depth           depth,
                      Move            ttMove,
                      int             moveCount) {

    CapturePieceToHistory& captureHistory = workerThread.captureHistory;
    Piece                  movedPiece     = pos.moved_piece(bestMove);
    PieceType              capturedPiece;

    int bonus =
      std::min(116 * depth - 81, 1515) + 347 * (bestMove == ttMove) + (ss - 1)->statScore / 32;
    int malus = std::min(848 * depth - 207, 2446) - 17 * moveCount;

    if (!pos.capture_stage(bestMove))
    {
        update_quiet_histories(pos, ss, workerThread, bestMove, bonus * 910 / 1024);

        int i = 0;
        // Decrease stats for all non-best quiet moves
        for (Move move : quietsSearched)
        {
            i++;
            int actualMalus = malus * 1085 / 1024;
            if (i > 5)
                actualMalus -= actualMalus * (i - 5) / i;
            update_quiet_histories(pos, ss, workerThread, move, -actualMalus);
        }
    }
    else
    {
        // Increase stats for the best move in case it was a capture move
        capturedPiece = type_of(pos.piece_on(bestMove.to_sq()));
        captureHistory[movedPiece][bestMove.to_sq()][capturedPiece] << bonus * 1395 / 1024;
    }

    // Extra penalty for a quiet early move that was not a TT move in
    // previous ply when it gets refuted.
    if (prevSq != SQ_NONE && ((ss - 1)->moveCount == 1 + (ss - 1)->ttHit) && !pos.captured_piece())
        update_continuation_histories(ss - 1, pos.piece_on(prevSq), prevSq, -malus * 602 / 1024);

    // Decrease stats for all non-best capture moves
    for (Move move : capturesSearched)
    {
        movedPiece    = pos.moved_piece(move);
        capturedPiece = type_of(pos.piece_on(move.to_sq()));
        captureHistory[movedPiece][move.to_sq()][capturedPiece] << -malus * 1448 / 1024;
    }
}


// Updates histories of the move pairs formed by moves
// at ply -1, -2, -3, -4, and -6 with current move.
void update_continuation_histories(Stack* ss, Piece pc, Square to, int bonus) {
    static std::array<ConthistBonus, 6> conthist_bonuses = {
      {{1, 1133}, {2, 683}, {3, 312}, {4, 582}, {5, 149}, {6, 474}}};

    for (const auto [i, weight] : conthist_bonuses)
    {
        // Only update the first 2 continuation histories if we are in check
        if (ss->inCheck && i > 2)
            break;
        if (((ss - i)->currentMove).is_ok())
            (*(ss - i)->continuationHistory)[pc][to] << (bonus * weight / 1024) + 88 * (i < 2);
    }
}

// Updates move sorting heuristics

void update_quiet_histories(
  const Position& pos, Stack* ss, Search::Worker& workerThread, Move move, int bonus) {

    Color us = pos.side_to_move();
    workerThread.mainHistory[us][move.raw()] << bonus;  // Untuned to prevent duplicate effort

    if (ss->ply < LOW_PLY_HISTORY_SIZE)
        workerThread.lowPlyHistory[ss->ply][move.raw()] << bonus * 805 / 1024;

    update_continuation_histories(ss, pos.moved_piece(move), move.to_sq(), bonus * 896 / 1024);

    workerThread.sharedHistory.pawn_entry(pos)[pos.moved_piece(move)][move.to_sq()]
      << bonus * (bonus > 0 ? 905 : 505) / 1024;
}

}
// no skill


// mcts begin
//  minimax_value() is a wrapper around the search() and qsearch() functions
//  used to compute the minimax evaluation of a position at the given depth,
//  from the point of view of the side to move. It does not compute PV nor
//  emit anything on the output stream. Note: you can call this function
//  with depth == DEPTH_ZERO to compute the quiescence value of the position.

Value Search::Worker::minimax_value(Position& pos, Search::Stack* ss, Depth depth) {

    //    threads.stopOnPonderhit = threads.stop = false;
    Value alpha = -VALUE_INFINITE;
    Value beta  = VALUE_INFINITE;
    Move  pv[MAX_PLY + 1];
    ss->pv = pv;

    /*   if (pos.should_debug())
      {
          debug << "Entering minimax_value() for the following position:" << std::endl;
          debug << pos << std::endl;
          hit_any_key();
      }*/
    Value value = search<PV>(pos, ss, alpha, beta, depth, false);

    // Have we found a "mate in x"?
    if (limits.mate && value >= VALUE_MATE_IN_MAX_PLY && VALUE_MATE - value <= 2 * limits.mate)
        threads.stop = true;

    /*    if (pos.should_debug())
      {
          debug << pos << std::endl;
          debug << "... exiting minimax_value() with value = " << value << std::endl;
          hit_any_key();
      }
    */
    return value;
}

// minimax_value() is a wrapper around the search() and qsearch() functions
// used to compute the minimax evaluation of a position at the given depth,
// from the point of view of the side to move. It does not compute PV nor
// emit anything on the output stream. Note: you can call this function
// with depth == DEPTH_ZERO to compute the quiescence value of the position.

Value Search::Worker::minimax_value(
  Position& pos, Search::Stack* ss, Depth depth, Value alpha, Value beta) {

    //    threads.stopOnPonderhit = threads.stop = false;
    //   alpha = -VALUE_INFINITE;
    //   beta = VALUE_INFINITE;
    Move pv[MAX_PLY + 1];
    ss->pv = pv;

    /*   if (pos.should_debug())
      {
          debug << "Entering minimax_value() for the following position:" << std::endl;
          debug << pos << std::endl;
          hit_any_key();
      }*/
    Value value = VALUE_ZERO;
    Value delta = Value(18);

    while (!threads.stop.load(std::memory_order_relaxed))
    {
        value = search<PV>(pos, ss, alpha, beta, depth, false);
        if (value <= alpha)
        {
            beta  = (alpha + beta) / 2;
            alpha = std::max(value - delta, -VALUE_INFINITE);
        }
        else if (value >= beta)
        {
            beta = std::min(value + delta, VALUE_INFINITE);
            // ++failedHighCnt;
        }
        else
        {
            //++rootMoves[pvIdx].bestMoveCount;
            break;
        }
    }

    // Have we found a "mate in x"?
    if (limits.mate && value >= VALUE_MATE_IN_MAX_PLY && VALUE_MATE - value <= 2 * limits.mate)
        threads.stop = true;

    /*    if (pos.should_debug())
      {
          debug << pos << std::endl;
          debug << "... exiting minimax_value() with value = " << value << std::endl;
          hit_any_key();
      }
    */
    return value;
}
// mcts end


// Used to print debug info and, more importantly, to detect
// when we are out of available time and thus stop the search.
void SearchManager::check_time(Search::Worker& worker) {
    if (--callsCnt > 0)
        return;

    // When using nodes, ensure checking rate is not lower than 0.1% of nodes
    callsCnt = worker.limits.nodes ? std::min(512, int(worker.limits.nodes / 1024)) : 512;

    static TimePoint lastInfoTime = now();

    TimePoint elapsed = tm.elapsed([&worker]() { return worker.threads.nodes_searched(); });
    TimePoint tick    = worker.limits.startTime + elapsed;

    if (tick - lastInfoTime >= 1000)
    {
        lastInfoTime = tick;
        dbg_print();
    }

    // We should not stop pondering until told so by the GUI
    if (ponder)
        return;

    if (
      // Later we rely on the fact that we can at least use the mainthread previous
      // root-search score and PV in a multithreaded environment to prove mated-in scores.
      worker.completedDepth >= 1
      && ((worker.limits.use_time_management() && (elapsed > tm.maximum() || stopOnPonderhit))
          || (worker.limits.movetime && elapsed >= worker.limits.movetime)
          || (worker.limits.nodes && worker.threads.nodes_searched() >= worker.limits.nodes)))
        worker.threads.stop = worker.threads.abortedSearch = true;
}

// Used to correct and extend PVs for moves that have a TB (but not a mate) score.
// Keeps the search based PV for as long as it is verified to maintain the game
// outcome, truncates afterwards. Finally, extends to mate the PV, providing a
// possible continuation (but not a proven mating line).
void syzygy_extend_pv(const OptionsMap&         options,
                      const Search::LimitsType& limits,
                      Position&                 pos,
                      RootMove&                 rootMove,
                      Value&                    v) {

    auto t_start      = std::chrono::steady_clock::now();
    int  moveOverhead = int(options["Move Overhead"]);
    bool rule50       = bool(options["Syzygy50MoveRule"]);

    // Do not use more than moveOverhead / 2 time, if time management is active
    auto time_abort = [&t_start, &moveOverhead, &limits]() -> bool {
        auto t_end = std::chrono::steady_clock::now();
        return limits.use_time_management()
            && 2 * std::chrono::duration<double, std::milli>(t_end - t_start).count()
                 > moveOverhead;
    };

    std::list<StateInfo> sts;

    // Step 0, do the rootMove, no correction allowed, as needed for MultiPV in TB.
    auto& stRoot = sts.emplace_back();
    pos.do_move(rootMove.pv[0], stRoot);
    int ply = 1;

    // Step 1, walk the PV to the last position in TB with correct decisive score
    while (size_t(ply) < rootMove.pv.size())
    {
        Move& pvMove = rootMove.pv[ply];

        RootMoves legalMoves;
        for (const auto& m : MoveList<LEGAL>(pos))
            legalMoves.emplace_back(m);

        Tablebases::Config config =
          Tablebases::rank_root_moves(options, pos, legalMoves, false, time_abort);
        RootMove& rm = *std::find(legalMoves.begin(), legalMoves.end(), pvMove);

        if (legalMoves[0].tbRank != rm.tbRank)
            break;

        ply++;

        auto& st = sts.emplace_back();
        pos.do_move(pvMove, st);

        // Do not allow for repetitions or drawing moves along the PV in TB regime
        if (config.rootInTB && ((rule50 && pos.is_draw(ply)) || pos.is_repetition(ply)))
        {
            pos.undo_move(pvMove);
            ply--;
            break;
        }

        // Full PV shown will thus be validated and end in TB.
        // If we cannot validate the full PV in time, we do not show it.
        if (config.rootInTB && time_abort())
            break;
    }

    // Resize the PV to the correct part
    rootMove.pv.resize(ply);

    // Step 2, now extend the PV to mate, as if the user explored syzygy-tables.info
    // using top ranked moves (minimal DTZ), which gives optimal mates only for simple
    // endgames e.g. KRvK.
    while (!(rule50 && pos.is_draw(0)))
    {
        if (time_abort())
            break;

        RootMoves legalMoves;
        for (const auto& m : MoveList<LEGAL>(pos))
        {
            auto&     rm = legalMoves.emplace_back(m);
            StateInfo tmpSI;
            pos.do_move(m, tmpSI);
            // Give a score of each move to break DTZ ties restricting opponent mobility,
            // but not giving the opponent a capture.
            for (const auto& mOpp : MoveList<LEGAL>(pos))
                rm.tbRank -= pos.capture(mOpp) ? 100 : 1;
            pos.undo_move(m);
        }

        // Mate found
        if (legalMoves.size() == 0)
            break;

        // Sort moves according to their above assigned rank.
        // This will break ties for moves with equal DTZ in rank_root_moves.
        std::stable_sort(
          legalMoves.begin(), legalMoves.end(),
          [](const Search::RootMove& a, const Search::RootMove& b) { return a.tbRank > b.tbRank; });

        // The winning side tries to minimize DTZ, the losing side maximizes it
        Tablebases::Config config =
          Tablebases::rank_root_moves(options, pos, legalMoves, true, time_abort);

        // If DTZ is not available we might not find a mate, so we bail out
        if (!config.rootInTB || config.cardinality > 0)
            break;

        ply++;

        Move& pvMove = legalMoves[0].pv[0];
        rootMove.pv.push_back(pvMove);
        auto& st = sts.emplace_back();
        pos.do_move(pvMove, st);
    }

    // Finding a draw in this function is an exceptional case, that cannot happen when rule50 is false or
    // during engine game play, since we have a winning score, and play correctly
    // with TB support. However, it can be that a position is draw due to the 50 move
    // rule if it has been been reached on the board with a non-optimal 50 move counter
    // (e.g. 8/8/6k1/3B4/3K4/4N3/8/8 w - - 54 106 ) which TB with dtz counter rounding
    // cannot always correctly rank. See also
    // https://github.com/official-stockfish/Stockfish/issues/5175#issuecomment-2058893495
    // We adjust the score to match the found PV. Note that a TB loss score can be
    // displayed if the engine did not find a drawing move yet, but eventually search
    // will figure it out (e.g. 1kq5/q2r4/5K2/8/8/8/8/7Q w - - 96 1 )
    if (pos.is_draw(0))
        v = VALUE_DRAW;

    // Undo the PV moves
    for (auto it = rootMove.pv.rbegin(); it != rootMove.pv.rend(); ++it)
        pos.undo_move(*it);

    // Inform if we couldn't get a full extension in time
    if (time_abort())
        sync_cout
          << "info string Syzygy based PV extension requires more time, increase Move Overhead as needed."
          << sync_endl;
}

void SearchManager::pv(Search::Worker&           worker,
                       const ThreadPool&         threads,
                       const TranspositionTable& tt,
                       Depth                     depth,
                       bool                      updateShashin) {  //shashin

    const auto nodes     = threads.nodes_searched();
    auto&      rootMoves = worker.rootMoves;
    auto&      pos       = worker.rootPos;
    size_t     pvIdx     = worker.pvIdx;
    size_t     multiPV   = std::min(size_t(worker.options["MultiPV"]), rootMoves.size());
    uint64_t   tbHits    = threads.tb_hits() + (worker.tbConfig.rootInTB ? rootMoves.size() : 0);

    for (size_t i = 0; i < multiPV; ++i)
    {
        bool updated = rootMoves[i].score != -VALUE_INFINITE;

        if (depth == 1 && !updated && i > 0)
            continue;

        Depth d = updated ? depth : std::max(1, depth - 1);
        Value v = updated ? rootMoves[i].uciScore : rootMoves[i].previousScore;

        if (v == -VALUE_INFINITE)
            v = VALUE_ZERO;

        bool tb = worker.tbConfig.rootInTB && std::abs(v) <= VALUE_TB;
        v       = tb ? rootMoves[i].tbScore : v;

        bool isExact = i != pvIdx || tb || !updated;  // tablebase- and previous-scores are exact

        // Potentially correct and extend the PV, and in exceptional cases v
        if (is_decisive(v) && std::abs(v) < VALUE_MATE_IN_MAX_PLY
            && ((!rootMoves[i].scoreLowerbound && !rootMoves[i].scoreUpperbound) || isExact))
            syzygy_extend_pv(worker.options, worker.limits, pos, rootMoves[i], v);
        //from shashin begin
        // Update the root Shashin state only for the main (first) PV line.
        if (updateShashin && (i == 0) && d > worker.lastShashinUpdatedDepth)
        {
            worker.getShashinManager().updateRootShashinState(v, pos, d, worker.rootDepth);
            worker.lastShashinUpdatedDepth = d;
        }
        //from shashin end
        std::string pv;
        for (Move m : rootMoves[i].pv)
            pv += UCIEngine::move(m, pos.is_chess960()) + " ";

        // Remove last whitespace
        if (!pv.empty())
            pv.pop_back();

        auto wdl   = worker.options["UCI_ShowWDL"] ? WDLModel::wdl(v, pos) : "";
        auto bound = rootMoves[i].scoreLowerbound
                     ? "lowerbound"
                     : (rootMoves[i].scoreUpperbound ? "upperbound" : "");

        InfoFull info;

        info.depth    = d;
        info.selDepth = rootMoves[i].selDepth;
        info.multiPV  = i + 1;
        info.score    = {v, pos};
        info.wdl      = wdl;

        if (!isExact)
            info.bound = bound;

        TimePoint time = std::max(TimePoint(1), tm.elapsed_time());
        info.timeMs    = time;
        info.nodes     = nodes;
        info.nps       = nodes * 1000 / time;
        info.tbHits    = tbHits;
        info.pv        = pv;
        info.hashfull  = tt.hashfull();

        updates.onUpdateFull(info);
    }
}

// Called in case we have no ponder move before exiting the search,
// for instance, in case we stop the search during a fail high at root.
// We try hard to have a ponder move to return to the GUI,
// otherwise in case of 'ponder on' we have nothing to think about.
bool RootMove::extract_ponder_from_tt(const TranspositionTable& tt, Position& pos) {

    StateInfo st;

    assert(pv.size() == 1);
    if (pv[0] == Move::none())
        return false;

    pos.do_move(pv[0], st, &tt);

    auto [ttHit, ttData, ttWriter] = tt.probe(pos.key());
    if (ttHit)
    {
        if (MoveList<LEGAL>(pos).contains(ttData.move))
            pv.push_back(ttData.move);
    }

    pos.undo_move(pv[0]);
    return pv.size() > 1;
}


// learning begin
void putQLearningTrajectoryIntoLearningTable() {
    const double learning_rate = 0.5;
    const double gamma         = 0.99;

    if (qLearningTrajectory.size() <= 1)
        return;

    for (size_t index = qLearningTrajectory.size() - 1; index > 0; --index)
    {
        LearningMove prevLearningMove =
          qLearningTrajectory[index - 1].persistedLearningMove.learningMove;
        const LearningMove currentLearningMove =
          qLearningTrajectory[index].persistedLearningMove.learningMove;

        prevLearningMove.score = prevLearningMove.score * (1 - learning_rate)
                               + learning_rate * (gamma * currentLearningMove.score);
        prevLearningMove.performance = WDLModel::get_win_probability_by_material(
          prevLearningMove.score, qLearningTrajectory[index - 1].materialClamp);

        LD.add_new_learning(qLearningTrajectory[index - 1].persistedLearningMove.key,
                            prevLearningMove);
    }

    qLearningTrajectory.clear();
}

void setStartPoint() {
    useLearning = true;
    LD.resume();
    qLearningTrajectory.clear();
}
// learning end
// shashin begin
Value ShashinManager::static_value(Position& rootPos, Search::Stack* ss) {
    if (ss->ply >= MAX_PLY || rootPos.is_draw(ss->ply))
        return VALUE_DRAW;

    MoveList<LEGAL> legalMoves(rootPos);
    if (legalMoves.size() == 0)
        return rootPos.checkers() ? VALUE_MATE : VALUE_DRAW;

    if (!rootPos.checkers())
        return Eval::evaluate(rootPos);

    Value bestValue = -VALUE_INFINITE;
    for (const auto& move : legalMoves)
    {
        StateInfo st;
        // Esegue la mossa
        rootPos.do_move(move, st, rootPos.gives_check(move), nullptr, nullptr);
        // --- FIX PLY UPDATE ---
        (ss + 1)->ply = ss->ply + 1;  // <--- AGGIUNGERE QUESTA RIGA
        // ----------------------
        Value val = -static_value(rootPos, ss + 1);
        // Ripristina la posizione
        rootPos.undo_move(move);

        if (val > bestValue)
        {
            bestValue = val;
            if (bestValue == VALUE_MATE)
                break;
        }
    }

    return bestValue;
}
//shashin end
}  // namespace Alexander

// ============================================================================
// FILE ORIGINALE: search.h
// ============================================================================

/*
  Alexander, a UCI chess playing engine derived from Stockfish
  Copyright (C) 2004-2026 Alexander developers (see AUTHORS file)

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

#ifndef SEARCH_H_INCLUDED
#define SEARCH_H_INCLUDED

#include <algorithm>
#include <array>
#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "history.h"
#include "misc.h"
//from classical
#include "numa.h"
#include "position.h"
#include "score.h"
#include "syzygy/tbprobe.h"
#include "timeman.h"
#include "types.h"
//from Shashin begin
#include "shashin/shashin_position.h"
#include "evaluate.h"
#include "book/book_manager.h"
#include <memory>
#include "shashin/shashin_manager.h"
//from Shashin end

namespace Alexander {

class ShashinManager;  //shashin
// Different node types, used as a template parameter
enum NodeType {
    NonPV,
    PV,
    Root
};

class TranspositionTable;
class ThreadPool;
class OptionsMap;

namespace Search {

// Stack struct keeps track of the information we need to remember from nodes
// shallower and deeper in the tree during the search. Each search thread has
// its own array of Stack objects, indexed by the current ply.
struct Stack {
    Move*                       pv;
    PieceToHistory*             continuationHistory;
    CorrectionHistory<PieceTo>* continuationCorrectionHistory;
    int                         ply;
    Move                        currentMove;
    Move                        excludedMove;
    Value                       staticEval;
    int                         statScore;
    int                         moveCount;
    bool                        inCheck;
    bool                        ttPv;
    bool                        ttHit;
    int                         cutoffCnt;
    int                         reduction;
    //from Shashin Crystal begin
    bool secondaryLine = false;
    bool mainLine      = false;
    //from Shashin Crystal end
};


// RootMove struct is used for moves at the root of the tree. For each root move
// we store a score and a PV (really a refutation in the case of moves which
// fail low). Score is normally set at -VALUE_INFINITE for all non-pv moves.
struct RootMove {

    explicit RootMove(Move m) :
        pv(1, m) {}
    bool extract_ponder_from_tt(const TranspositionTable& tt, Position& pos);
    bool operator==(const Move& m) const { return pv[0] == m; }
    // Sort in descending order
    bool operator<(const RootMove& m) const {
        return m.score != score ? m.score < score : m.previousScore < previousScore;
    }

    uint64_t          effort           = 0;
    Value             score            = -VALUE_INFINITE;
    Value             previousScore    = -VALUE_INFINITE;
    Value             averageScore     = -VALUE_INFINITE;
    Value             meanSquaredScore = -VALUE_INFINITE * VALUE_INFINITE;
    Value             uciScore         = -VALUE_INFINITE;
    bool              scoreLowerbound  = false;
    bool              scoreUpperbound  = false;
    int               selDepth         = 0;
    int               tbRank           = 0;
    Value             tbScore          = 0;  //for windows build
    std::vector<Move> pv;
};

using RootMoves = std::vector<RootMove>;


// LimitsType struct stores information sent by the caller about the analysis required.
struct LimitsType {

    // Init explicitly due to broken value-initialization of non POD in MSVC
    LimitsType() {
        time[WHITE] = time[BLACK] = inc[WHITE] = inc[BLACK] = npmsec = movetime = TimePoint(0);
        movestogo = depth = mate = perft = infinite = 0;
        nodes                                       = 0;
        ponderMode                                  = false;
    }

    bool use_time_management() const { return time[WHITE] || time[BLACK]; }

    std::vector<std::string> searchmoves;
    TimePoint                time[COLOR_NB], inc[COLOR_NB], npmsec, movetime, startTime;
    int                      movestogo, depth, mate, perft, infinite;
    uint64_t                 nodes;
    bool                     ponderMode;
};


// The UCI stores the uci options, thread pool, and transposition table.
// This struct is used to easily forward data to the Search::Worker class.
struct SharedState {
    //from Polyfish begin
    SharedState(BookManager&                          bm,
                const OptionsMap&                     optionsMap,
                ThreadPool&                           threadPool,
                TranspositionTable&                   transpositionTable,
                std::map<NumaIndex, SharedHistories>& sharedHists,
                const Alexander::ShashinConfig&       shCfg) :
        bookMan(bm),
        options(optionsMap),
        threads(threadPool),
        tt(transpositionTable),
        sharedHistories(sharedHists),
        shashinConfig(shCfg) {}  //shashin
    BookManager& bookMan;
    //from Polyfish end
    const OptionsMap&                     options;
    ThreadPool&                           threads;
    TranspositionTable&                   tt;
    std::map<NumaIndex, SharedHistories>& sharedHistories;
    const Alexander::ShashinConfig        shashinConfig;  //Shashin
};

class Worker;

// Null Object Pattern, implement a common interface for the SearchManagers.
// A Null Object will be given to non-mainthread workers.
class ISearchManager {
   public:
    virtual ~ISearchManager() {}
    virtual void check_time(Search::Worker&) = 0;
};

struct InfoShort {
    int   depth;
    Score score;
};

struct InfoFull: InfoShort {
    int              selDepth;
    size_t           multiPV;
    std::string_view wdl;
    std::string_view bound;
    size_t           timeMs;
    size_t           nodes;
    size_t           nps;
    size_t           tbHits;
    std::string_view pv;
    int              hashfull;
};

struct InfoIteration {
    int              depth;
    std::string_view currmove;
    size_t           currmovenumber;
};

//no skill
// SearchManager manages the search from the main thread. It is responsible for
// keeping track of the time, and storing data strictly related to the main thread.
class SearchManager: public ISearchManager {
   public:
    using UpdateShort    = std::function<void(const InfoShort&)>;
    using UpdateFull     = std::function<void(const InfoFull&)>;
    using UpdateIter     = std::function<void(const InfoIteration&)>;
    using UpdateBestmove = std::function<void(std::string_view, std::string_view)>;

    struct UpdateContext {
        UpdateShort    onUpdateNoMoves;
        UpdateFull     onUpdateFull;
        UpdateIter     onIter;
        UpdateBestmove onBestmove;
    };


    SearchManager(const UpdateContext& updateContext) :
        updates(updateContext) {}

    void check_time(Search::Worker& worker) override;

    void pv(Search::Worker&           worker,
            const ThreadPool&         threads,
            const TranspositionTable& tt,
            Depth                     depth,
            bool                      updateShashin);  //shashin

    Alexander::TimeManagement tm;
    double                    originalTimeAdjust;
    int                       callsCnt;
    std::atomic_bool          ponder;

    std::array<Value, 4> iterValue;
    double               previousTimeReduction;
    Value                bestPreviousScore;
    Value                bestPreviousAverageScore;
    bool                 stopOnPonderhit;

    size_t id;

    const UpdateContext& updates;
};

class NullSearchManager: public ISearchManager {
   public:
    void check_time(Search::Worker&) override {}
};


// Search::Worker is the class that does the actual search.
// It is instantiated once per thread, and it is responsible for keeping track
// of the search history, and storing data required for the search.
class Worker {
   public:
    size_t threadIdx;  //mcts
    Worker(SharedState&,
           std::unique_ptr<ISearchManager>,
           size_t,
           size_t,
           size_t,
           NumaReplicatedAccessToken);
    ~Worker();  //shashin
    // Called at instantiation to initialize reductions tables.
    // Reset histories, usually before a new game.
    void clear();

    // Called when the program receives the UCI 'go' command.
    // It searches from the root position and outputs the "bestmove".
    void start_searching();

    bool is_mainthread() const { return threadIdx == 0; }

    //for classical
    //from Montecarlo begin
    Value minimax_value(Position& pos, Search::Stack* ss, Depth depth);
    Value minimax_value(Position& pos, Search::Stack* ss, Depth depth, Value alpha, Value beta);
    //from Montecarlo end

    // Public because they need to be updatable by the stats
    ButterflyHistory mainHistory;
    LowPlyHistory    lowPlyHistory;

    CapturePieceToHistory           captureHistory;
    ContinuationHistory             continuationHistory[2][2];
    CorrectionHistory<Continuation> continuationCorrectionHistory;

    TTMoveHistory    ttMoveHistory;
    SharedHistories& sharedHistory;
    RootMoves        rootMoves;                          //mcts
    Depth            completedDepth;                     //mcts
    bool             nmpGuard = false, nmpSide = false;  //from Crystal-shashin
    ShashinManager&  getShashinManager();                //from crystal-Shashin
    int              lastShashinUpdatedDepth = 0;        //from shashin

   private:
    void iterative_deepening();

    void do_move(Position& pos, const Move move, StateInfo& st, Stack* const ss);
    void
    do_move(Position& pos, const Move move, StateInfo& st, const bool givesCheck, Stack* const ss);
    void do_null_move(Position& pos, StateInfo& st, Stack* const ss);
    void undo_move(Position& pos, const Move move);
    void undo_null_move(Position& pos);

    // This is the main search function, for both PV and non-PV nodes
    template<NodeType nodeType>
    Value search(Position& pos, Stack* ss, Value alpha, Value beta, Depth depth, bool cutNode);

    // Quiescence search function, which is called by the main search
    template<NodeType nodeType>
    Value qsearch(Position& pos, Stack* ss, Value alpha, Value beta);

    Depth
    reduction(bool i, Depth d, int mn, int delta, bool isStrategical, bool isAggressive) const;

    // Pointer to the search manager, only allowed to be called by the main thread
    SearchManager* main_manager() const {
        assert(threadIdx == 0);
        return static_cast<SearchManager*>(manager.get());
    }

    TimePoint elapsed() const;
    TimePoint elapsed_time() const;

    Value evaluate(const Position&);

    LimitsType limits;

    size_t                pvIdx, pvLast;
    std::atomic<uint64_t> nodes, tbHits, bestMoveChanges;
    int                   selDepth, nmpMinPly;
    Value                 optimism[COLOR_NB];

    Position  rootPos;
    StateInfo rootState;
    //mcts
    Depth rootDepth;  //mcts
    Value rootDelta;

    size_t                    numaThreadIdx, numaTotal;  //for mcts
    bool                      fullSearch      = false;   //full threads patch
    bool                      antiDancingMode = false;   //anti dancing option
    NumaReplicatedAccessToken numaAccessToken;

    // Reductions lookup table initialized at startup
    std::array<int, MAX_MOVES> reductions;  // [depth or moveNumber]

    // The main thread has a SearchManager, the others have a NullSearchManager
    std::unique_ptr<ISearchManager> manager;

    Tablebases::Config tbConfig;

    //From PolyFish begin
    BookManager& bookMan;
    //From PolyFish end
    const OptionsMap&   options;
    ThreadPool&         threads;
    TranspositionTable& tt;
    //for classical
    //Shashin begin
    std::unique_ptr<ShashinManager> shashinManager;
    const Alexander::ShashinConfig& shConfig;
    friend class Alexander::ThreadPool;
    //Shashin end
    friend class SearchManager;
};

struct ConthistBonus {
    int index;
    int weight;
};
//livebook begin
#ifdef USE_LIVEBOOK
void set_livebook_depth(int book_depth);
void set_proxy_url(const std::string& proxy_url);
void set_use_lichess_games(bool lichess_games);
void set_use_lichess_masters(bool lichess_masters);
void set_lichess_player(const std::string& lichess_player);
void set_lichess_player_color(const std::string& lichess_player_color);
void set_use_chess_db(bool chess_db);

void set_use_chess_db_tablebase(bool chess_db);
void set_use_lichess_tablebase(bool lichess_tablebase);

void update_livebooks();
void update_online_tablebases();

void set_chess_db_contribute(bool chess_db_contribute);
void set_proxy_diversity(bool proxy_diversity);


#endif
//livebook end

void set_variety(const std::string& varietyOption);  //variety
}  // namespace Search
//from Livebook begin
#ifdef USE_LIVEBOOK
size_t cURL_WriteFunc(void* contents, size_t size, size_t nmemb, std::string* s);
#endif
//from Livebook end
}  // namespace Alexander

#endif  // #ifndef SEARCH_H_INCLUDED


// ============================================================================
// FILE ORIGINALE: shm.h
// ============================================================================

/*
  Alexander, a UCI chess playing engine derived from Glaurung 2.1
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

#ifndef SHM_H_INCLUDED
#define SHM_H_INCLUDED

#include <algorithm>
#include <cinttypes>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <functional>
#include <iomanip>
#include <iostream>
#include <memory>
#include "misc.h" //Alexander
#include <new>
#include <optional>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>

#if defined(__linux__) && !defined(__ANDROID__)
    #include "shm_linux.h"
#endif

#if defined(__ANDROID__)
    #include <limits.h>
    #define SF_MAX_SEM_NAME_LEN NAME_MAX
#endif

#include "types.h"

#include "memory.h"

#if defined(_WIN32)

    #if _WIN32_WINNT < 0x0601
        #undef _WIN32_WINNT
        #define _WIN32_WINNT 0x0601  // Force to include needed API prototypes
    #endif

    #if !defined(NOMINMAX)
        #define NOMINMAX
    #endif
    #include <windows.h>
#elif defined(__linux__)
    #include <cstring>
    #include <fcntl.h>
    #include <pthread.h>
    #include <semaphore.h>
    #include <sys/mman.h>
    #include <sys/stat.h>
    #include <unistd.h>
#endif


#if defined(__APPLE__)
    #include <mach-o/dyld.h>
    #include <sys/syslimits.h>

#elif defined(__sun)
    #include <stdlib.h>

#elif defined(__FreeBSD__)
    #include <sys/sysctl.h>
    #include <sys/types.h>
    #include <unistd.h>

#elif defined(__NetBSD__) || defined(__DragonFly__) || defined(__linux__)
    #include <limits.h>
    #include <unistd.h>
#endif


namespace Alexander {

// argv[0] CANNOT be used because we need to identify the executable.
// argv[0] contains the command used to invoke it, which does not involve the full path.
// Just using a path is not fully resilient either, as the executable could
// have changed if it wasn't locked by the OS. Ideally we would hash the executable
// but it's not really that important at this point.
// If the path is longer than 4095 bytes the hash will be computed from an unspecified
// amount of bytes of the path; in particular it can a hash of an empty string.

inline std::string getExecutablePathHash() {
    char        executable_path[4096] = {0};
    std::size_t path_length           = 0;

#if defined(_WIN32)
    path_length = GetModuleFileNameA(NULL, executable_path, sizeof(executable_path));

#elif defined(__APPLE__)
    uint32_t size = sizeof(executable_path);
    if (_NSGetExecutablePath(executable_path, &size) == 0)
    {
        path_length = std::strlen(executable_path);
    }

#elif defined(__sun)  // Solaris
    const char* path = getexecname();
    if (path)
    {
        std::strncpy(executable_path, path, sizeof(executable_path) - 1);
        path_length = std::strlen(executable_path);
    }

#elif defined(__FreeBSD__)
    size_t size   = sizeof(executable_path);
    int    mib[4] = {CTL_KERN, KERN_PROC, KERN_PROC_PATHNAME, -1};
    if (sysctl(mib, 4, executable_path, &size, NULL, 0) == 0)
    {
        path_length = std::strlen(executable_path);
    }

#elif defined(__NetBSD__) || defined(__DragonFly__)
    ssize_t len = readlink("/proc/curproc/exe", executable_path, sizeof(executable_path) - 1);
    if (len >= 0)
    {
        executable_path[len] = '\0';
        path_length          = len;
    }

#elif defined(__linux__)
    ssize_t len = readlink("/proc/self/exe", executable_path, sizeof(executable_path) - 1);
    if (len >= 0)
    {
        executable_path[len] = '\0';
        path_length          = len;
    }

#endif

    // In case of any error the path will be empty.
    return std::string(executable_path, path_length);
}

enum class SystemWideSharedConstantAllocationStatus {
    NoAllocation,
    LocalMemory,
    SharedMemory
};

#if defined(_WIN32)

inline std::string GetLastErrorAsString(DWORD error) {
    //Get the error message ID, if any.
    DWORD errorMessageID = error;
    if (errorMessageID == 0)
    {
        return std::string();  //No error message has been recorded
    }

    LPSTR messageBuffer = nullptr;

    //Ask Win32 to give us the string version of that message ID.
    //The parameters we pass in, tell Win32 to create the buffer that holds the message for us (because we don't yet know how long the message string will be).
    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM
                                   | FORMAT_MESSAGE_IGNORE_INSERTS,
                                 NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                 (LPSTR) &messageBuffer, 0, NULL);

    //Copy the error message into a std::string.
    std::string message(messageBuffer, size);

    //Free the Win32's string's buffer.
    LocalFree(messageBuffer);

    return message;
}

// Utilizes shared memory to store the value. It is deduplicated system-wide (for the single user).
template<typename T>
class SharedMemoryBackend {
   public:
    enum class Status {
        Success,
        LargePageAllocationError,
        FileMappingError,
        MapViewError,
        MutexCreateError,
        MutexWaitError,
        MutexReleaseError,
        NotInitialized
    };

    static constexpr DWORD IS_INITIALIZED_VALUE = 1;

    SharedMemoryBackend() :
        status(Status::NotInitialized) {};

    SharedMemoryBackend(const std::string& shm_name, const T& value) :
        status(Status::NotInitialized) {

        initialize(shm_name, value);
    }

    bool is_valid() const { return status == Status::Success; }

    std::optional<std::string> get_error_message() const {
        switch (status)
        {
        case Status::Success :
            return std::nullopt;
        case Status::LargePageAllocationError :
            return "Failed to allocate large page memory";
        case Status::FileMappingError :
            return "Failed to create file mapping: " + last_error_message;
        case Status::MapViewError :
            return "Failed to map view: " + last_error_message;
        case Status::MutexCreateError :
            return "Failed to create mutex: " + last_error_message;
        case Status::MutexWaitError :
            return "Failed to wait on mutex: " + last_error_message;
        case Status::MutexReleaseError :
            return "Failed to release mutex: " + last_error_message;
        case Status::NotInitialized :
            return "Not initialized";
        default :
            return "Unknown error";
        }
    }

    void* get() const { return is_valid() ? pMap : nullptr; }

    ~SharedMemoryBackend() { cleanup(); }

    SharedMemoryBackend(const SharedMemoryBackend&)            = delete;
    SharedMemoryBackend& operator=(const SharedMemoryBackend&) = delete;

    SharedMemoryBackend(SharedMemoryBackend&& other) noexcept :
        pMap(other.pMap),
        hMapFile(other.hMapFile),
        status(other.status),
        last_error_message(std::move(other.last_error_message)) {

        other.pMap     = nullptr;
        other.hMapFile = 0;
        other.status   = Status::NotInitialized;
    }

    SharedMemoryBackend& operator=(SharedMemoryBackend&& other) noexcept {
        if (this != &other)
        {
            cleanup();
            pMap               = other.pMap;
            hMapFile           = other.hMapFile;
            status             = other.status;
            last_error_message = std::move(other.last_error_message);

            other.pMap     = nullptr;
            other.hMapFile = 0;
            other.status   = Status::NotInitialized;
        }
        return *this;
    }

    SystemWideSharedConstantAllocationStatus get_status() const {
        return status == Status::Success ? SystemWideSharedConstantAllocationStatus::SharedMemory
                                         : SystemWideSharedConstantAllocationStatus::NoAllocation;
    }

   private:
    void initialize(const std::string& shm_name, const T& value) {
        const size_t total_size = sizeof(T) + sizeof(IS_INITIALIZED_VALUE);

        // Try allocating with large pages first.
        hMapFile = windows_try_with_large_page_priviliges(
          [&](size_t largePageSize) {
              const size_t total_size_aligned =
                (total_size + largePageSize - 1) / largePageSize * largePageSize;

    #if defined(_WIN64)
              DWORD total_size_low  = total_size_aligned & 0xFFFFFFFFu;
              DWORD total_size_high = total_size_aligned >> 32u;
    #else
              DWORD total_size_low  = total_size_aligned;
              DWORD total_size_high = 0;
    #endif

              return CreateFileMappingA(INVALID_HANDLE_VALUE, NULL,
                                        PAGE_READWRITE | SEC_COMMIT | SEC_LARGE_PAGES,
                                        total_size_high, total_size_low, shm_name.c_str());
          },
          []() { return (void*) nullptr; });

        // Fallback to normal allocation if no large pages available.
        if (!hMapFile)
        {
            hMapFile = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0,
                                          static_cast<DWORD>(total_size), shm_name.c_str());
        }

        if (!hMapFile)
        {
            const DWORD err    = GetLastError();
            last_error_message = GetLastErrorAsString(err);
            status             = Status::FileMappingError;
            return;
        }

        pMap = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, total_size);
        if (!pMap)
        {
            const DWORD err    = GetLastError();
            last_error_message = GetLastErrorAsString(err);
            status             = Status::MapViewError;
            cleanup_partial();
            return;
        }

        // Use named mutex to ensure only one initializer
        std::string mutex_name = shm_name + "$mutex";
        HANDLE      hMutex     = CreateMutexA(NULL, FALSE, mutex_name.c_str());
        if (!hMutex)
        {
            const DWORD err    = GetLastError();
            last_error_message = GetLastErrorAsString(err);
            status             = Status::MutexCreateError;
            cleanup_partial();
            return;
        }

        DWORD wait_result = WaitForSingleObject(hMutex, INFINITE);
        if (wait_result != WAIT_OBJECT_0)
        {
            const DWORD err    = GetLastError();
            last_error_message = GetLastErrorAsString(err);
            status             = Status::MutexWaitError;
            CloseHandle(hMutex);
            cleanup_partial();
            return;
        }

        // Crucially, we place the object first to ensure alignment.
        volatile DWORD* is_initialized =
          std::launder(reinterpret_cast<DWORD*>(reinterpret_cast<char*>(pMap) + sizeof(T)));
        T* object = std::launder(reinterpret_cast<T*>(pMap));

        if (*is_initialized != IS_INITIALIZED_VALUE)
        {
            // First time initialization, message for debug purposes
            new (object) T{value};
            *is_initialized = IS_INITIALIZED_VALUE;
        }

        BOOL release_result = ReleaseMutex(hMutex);
        CloseHandle(hMutex);

        if (!release_result)
        {
            const DWORD err    = GetLastError();
            last_error_message = GetLastErrorAsString(err);
            status             = Status::MutexReleaseError;
            cleanup_partial();
            return;
        }

        status = Status::Success;
    }

    void cleanup_partial() {
        if (pMap != nullptr)
        {
            UnmapViewOfFile(pMap);
            pMap = nullptr;
        }
        if (hMapFile)
        {
            CloseHandle(hMapFile);
            hMapFile = 0;
        }
    }

    void cleanup() {
        if (pMap != nullptr)
        {
            UnmapViewOfFile(pMap);
            pMap = nullptr;
        }
        if (hMapFile)
        {
            CloseHandle(hMapFile);
            hMapFile = 0;
        }
    }

    void*       pMap     = nullptr;
    HANDLE      hMapFile = 0;
    Status      status   = Status::NotInitialized;
    std::string last_error_message;
};

#elif defined(__linux__) && !defined(__ANDROID__)

template<typename T>
class SharedMemoryBackend {
   public:
    SharedMemoryBackend() = default;

    SharedMemoryBackend(const std::string& shm_name, const T& value) :
        shm1(shm::create_shared<T>(shm_name, value)) {}

    void* get() const {
        const T* ptr = &shm1->get();
        return reinterpret_cast<void*>(const_cast<T*>(ptr));
    }

    bool is_valid() const { return shm1 && shm1->is_open() && shm1->is_initialized(); }

    SystemWideSharedConstantAllocationStatus get_status() const {
        return is_valid() ? SystemWideSharedConstantAllocationStatus::SharedMemory
                          : SystemWideSharedConstantAllocationStatus::NoAllocation;
    }

    std::optional<std::string> get_error_message() const {
        if (!shm1)
            return "Shared memory not initialized";

        if (!shm1->is_open())
            return "Shared memory is not open";

        if (!shm1->is_initialized())
            return "Not initialized";

        return std::nullopt;
    }

   private:
    std::optional<shm::SharedMemory<T>> shm1;
};

#else

// For systems that don't have shared memory, or support is troublesome.
// The way fallback is done is that we need a dummy backend.

template<typename T>
class SharedMemoryBackend {
   public:
    SharedMemoryBackend() = default;

    SharedMemoryBackend([[maybe_unused]] const std::string& shm_name,
                        [[maybe_unused]] const T&           value) {}

    void* get() const { return nullptr; }

    bool is_valid() const { return false; }

    SystemWideSharedConstantAllocationStatus get_status() const {
        return SystemWideSharedConstantAllocationStatus::NoAllocation;
    }

    std::optional<std::string> get_error_message() const { return "Dummy SharedMemoryBackend"; }
};

#endif

template<typename T>
struct SharedMemoryBackendFallback {
    SharedMemoryBackendFallback() = default;

    SharedMemoryBackendFallback(const std::string&, const T& value) :
        fallback_object(make_unique_large_page<T>(value)) {}

    void* get() const { return fallback_object.get(); }

    SharedMemoryBackendFallback(const SharedMemoryBackendFallback&)            = delete;
    SharedMemoryBackendFallback& operator=(const SharedMemoryBackendFallback&) = delete;

    SharedMemoryBackendFallback(SharedMemoryBackendFallback&& other) noexcept :
        fallback_object(std::move(other.fallback_object)) {}

    SharedMemoryBackendFallback& operator=(SharedMemoryBackendFallback&& other) noexcept {
        fallback_object = std::move(other.fallback_object);
        return *this;
    }

    SystemWideSharedConstantAllocationStatus get_status() const {
        return fallback_object == nullptr ? SystemWideSharedConstantAllocationStatus::NoAllocation
                                          : SystemWideSharedConstantAllocationStatus::LocalMemory;
    }

    std::optional<std::string> get_error_message() const {
        if (fallback_object == nullptr)
            return "Not initialized";

        return "Shared memory not supported by the OS. Local allocation fallback.";
    }

   private:
    LargePagePtr<T> fallback_object;
};

// Platform-independent wrapper
template<typename T>
struct SystemWideSharedConstant {
   private:
    static std::string createHashString(const std::string& input) {
        char buf[1024];
        std::snprintf(buf, sizeof(buf), "%016" PRIx64, hash_string(input));
        return buf;
    }

   public:
    // We can't run the destructor because it may be in a completely different process.
    // The object stored must also be obviously in-line but we can't check for that, other than some basic checks that cover most cases.
    static_assert(std::is_trivially_destructible_v<T>);
    static_assert(std::is_trivially_move_constructible_v<T>);
    static_assert(std::is_trivially_copy_constructible_v<T>);

    SystemWideSharedConstant() = default;


    // Content is addressed by its hash. An additional discriminator can be added to account for differences
    // that are not present in the content, for example NUMA node allocation.
    SystemWideSharedConstant(const T& value, std::size_t discriminator = 0) {
        std::size_t content_hash    = std::hash<T>{}(value);
        std::size_t executable_hash = hash_string(getExecutablePathHash());

        char buf[1024];
        std::snprintf(buf, sizeof(buf), "Local\\sf_%zu$%zu$%zu", content_hash, executable_hash, discriminator);
        std::string shm_name = buf;

#if defined(__linux__) && !defined(__ANDROID__)
        // POSIX shared memory names must start with a slash
        shm_name = "/sf_" + createHashString(shm_name);

        // hash name and make sure it is not longer than SF_MAX_SEM_NAME_LEN
        if (shm_name.size() > SF_MAX_SEM_NAME_LEN)
        {
            shm_name = shm_name.substr(0, SF_MAX_SEM_NAME_LEN - 1);
        }
#endif

        SharedMemoryBackend<T> shm_backend(shm_name, value);

        if (shm_backend.is_valid())
        {
            backend = std::move(shm_backend);
        }
        else
        {
            backend = SharedMemoryBackendFallback<T>(shm_name, value);
        }
    }

    SystemWideSharedConstant(const SystemWideSharedConstant&)            = delete;
    SystemWideSharedConstant& operator=(const SystemWideSharedConstant&) = delete;

    SystemWideSharedConstant(SystemWideSharedConstant&& other) noexcept :
        backend(std::move(other.backend)) {}

    SystemWideSharedConstant& operator=(SystemWideSharedConstant&& other) noexcept {
        backend = std::move(other.backend);
        return *this;
    }

    const T& operator*() const { return *std::launder(reinterpret_cast<const T*>(get_ptr())); }

    bool operator==(std::nullptr_t) const noexcept { return get_ptr() == nullptr; }

    bool operator!=(std::nullptr_t) const noexcept { return get_ptr() != nullptr; }

    SystemWideSharedConstantAllocationStatus get_status() const {
        return std::visit(
          [](const auto& end) -> SystemWideSharedConstantAllocationStatus {
              if constexpr (std::is_same_v<std::decay_t<decltype(end)>, std::monostate>)
              {
                  return SystemWideSharedConstantAllocationStatus::NoAllocation;
              }
              else
              {
                  return end.get_status();
              }
          },
          backend);
    }

    std::optional<std::string> get_error_message() const {
        return std::visit(
          [](const auto& end) -> std::optional<std::string> {
              if constexpr (std::is_same_v<std::decay_t<decltype(end)>, std::monostate>)
              {
                  return std::nullopt;
              }
              else
              {
                  return end.get_error_message();
              }
          },
          backend);
    }

   private:
    auto get_ptr() const {
        return std::visit(
          [](const auto& end) -> void* {
              if constexpr (std::is_same_v<std::decay_t<decltype(end)>, std::monostate>)
              {
                  return nullptr;
              }
              else
              {
                  return end.get();
              }
          },
          backend);
    }

    std::variant<std::monostate, SharedMemoryBackend<T>, SharedMemoryBackendFallback<T>> backend;
};


}  // namespace Alexander

#endif  // #ifndef SHM_H_INCLUDED


// ============================================================================
// FILE ORIGINALE: shm_linux.h
// ============================================================================

/*
  Alexander, a UCI chess playing engine derived from Glaurung 2.1
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

#ifndef SHM_LINUX_H_INCLUDED
#define SHM_LINUX_H_INCLUDED

#if !defined(__linux__) || defined(__ANDROID__)
#error shm_linux.h should not be included on this platform.
#endif

#include <atomic>
#include <cassert>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <dirent.h>
#include <mutex>
#include <new>
#include <optional>
#include <pthread.h>
#include <string>
#include <inttypes.h>
#include <type_traits>

#include <fcntl.h>
#include <signal.h>
#include <sys/file.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#define SF_MAX_SEM_NAME_LEN NAME_MAX

#include "misc.h"


namespace Alexander::shm {

namespace detail {

struct ShmHeader {
    static constexpr uint32_t SHM_MAGIC = 0xAD5F1A12;
    pthread_mutex_t           mutex;
    std::atomic<uint32_t>     ref_count{0};
    std::atomic<bool>         initialized{false};
    uint32_t                  magic = SHM_MAGIC;
};

class SharedMemoryBase {
   public:
    virtual ~SharedMemoryBase()                      = default;
    virtual void               close(bool skip_unmap = false) noexcept      = 0;
    virtual const std::string& name() const noexcept = 0;
};

class SharedMemoryRegistry {
   private:
    static std::mutex                            registry_mutex_;
    static std::vector<SharedMemoryBase*> active_instances_;

   public:
    static void register_instance(SharedMemoryBase* instance) {
        std::scoped_lock lock(registry_mutex_);
        active_instances_.push_back(instance);
    }

    static void unregister_instance(SharedMemoryBase* instance) {
        std::scoped_lock lock(registry_mutex_);
        active_instances_.erase(
            std::remove(active_instances_.begin(), active_instances_.end(), instance), active_instances_.end());
    }

    static void cleanup_all(bool skip_unmap = false) noexcept {
        std::scoped_lock lock(registry_mutex_);
        for (auto* instance : active_instances_)
            instance->close(skip_unmap);
        active_instances_.clear();
    }
};

inline std::mutex                            SharedMemoryRegistry::registry_mutex_;
inline std::vector<SharedMemoryBase*> SharedMemoryRegistry::active_instances_;

class CleanupHooks {
   private:
    static std::once_flag register_once_;

    static void handle_signal(int sig) noexcept {
        // Search threads may still be running, so skip munmap (but still perform
        // other cleanup actions). The memory mappings will be released on exit.
        SharedMemoryRegistry::cleanup_all(true);

        // Invoke the default handler, which will exit
        struct sigaction sa;
        sa.sa_handler = SIG_DFL;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;
        if (sigaction(sig, &sa, nullptr) == -1)
            _Exit(128 + sig);

        raise(sig);
    }

    static void register_signal_handlers() noexcept {
        std::atexit([]() { SharedMemoryRegistry::cleanup_all(true); });

        constexpr int signals[] = {SIGHUP,  SIGINT,  SIGQUIT, SIGILL, SIGABRT, SIGFPE,
                                   SIGSEGV, SIGTERM, SIGBUS,  SIGSYS, SIGXCPU, SIGXFSZ};

        struct sigaction sa;
        sa.sa_handler = handle_signal;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;

        for (int sig : signals)
            sigaction(sig, &sa, nullptr);
    }

   public:
    static void ensure_registered() noexcept {
        std::call_once(register_once_, register_signal_handlers);
    }
};

inline std::once_flag CleanupHooks::register_once_;


inline int portable_fallocate(int fd, off_t offset, off_t length) {
#ifdef __APPLE__
    fstore_t store = {F_ALLOCATECONTIG, F_PEOFPOSMODE, offset, length, 0};
    int      ret   = fcntl(fd, F_PREALLOCATE, &store);
    if (ret == -1)
    {
        store.fst_flags = F_ALLOCATEALL;
        ret             = fcntl(fd, F_PREALLOCATE, &store);
    }
    if (ret != -1)
        ret = ftruncate(fd, offset + length);
    return ret;
#else
    return posix_fallocate(fd, offset, length);
#endif
}

}  // namespace detail

template<typename T>
class SharedMemory: public detail::SharedMemoryBase {
    static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable");
    static_assert(!std::is_pointer_v<T>, "T cannot be a pointer type");

   private:
    std::string        name_;
    int                fd_         = -1;
    void*              mapped_ptr_ = nullptr;
    T*                 data_ptr_   = nullptr;
    detail::ShmHeader* header_ptr_ = nullptr;
    size_t             total_size_ = 0;
    std::string        sentinel_base_;
    std::string        sentinel_path_;

    static constexpr size_t calculate_total_size() noexcept {
        return sizeof(T) + sizeof(detail::ShmHeader);
    }

    static std::string make_sentinel_base(const std::string& name) {
        char     buf[32];
        // Using std::to_string here causes non-deterministic PGO builds.
        // snprintf, being part of libc, is insensitive to the formatted values.
        std::snprintf(buf, sizeof(buf), "sfshm_%016" PRIu64, hash_string(name));
        return buf;
    }

   public:
    explicit SharedMemory(const std::string& name) noexcept :
        name_(name),
        total_size_(calculate_total_size()),
        sentinel_base_(make_sentinel_base(name)) {}

    ~SharedMemory() noexcept override {
        detail::SharedMemoryRegistry::unregister_instance(this);
        close();
    }

    SharedMemory(const SharedMemory&)            = delete;
    SharedMemory& operator=(const SharedMemory&) = delete;

    SharedMemory(SharedMemory&& other) noexcept :
        name_(std::move(other.name_)),
        fd_(other.fd_),
        mapped_ptr_(other.mapped_ptr_),
        data_ptr_(other.data_ptr_),
        header_ptr_(other.header_ptr_),
        total_size_(other.total_size_),
        sentinel_base_(std::move(other.sentinel_base_)),
        sentinel_path_(std::move(other.sentinel_path_)) {

        detail::SharedMemoryRegistry::unregister_instance(&other);
        detail::SharedMemoryRegistry::register_instance(this);
        other.reset();
    }

    SharedMemory& operator=(SharedMemory&& other) noexcept {
        if (this != &other)
        {
            detail::SharedMemoryRegistry::unregister_instance(this);
            close();

            name_          = std::move(other.name_);
            fd_            = other.fd_;
            mapped_ptr_    = other.mapped_ptr_;
            data_ptr_      = other.data_ptr_;
            header_ptr_    = other.header_ptr_;
            total_size_    = other.total_size_;
            sentinel_base_ = std::move(other.sentinel_base_);
            sentinel_path_ = std::move(other.sentinel_path_);

            detail::SharedMemoryRegistry::unregister_instance(&other);
            detail::SharedMemoryRegistry::register_instance(this);

            other.reset();
        }
        return *this;
    }

    [[nodiscard]] bool open(const T& initial_value) noexcept {
        detail::CleanupHooks::ensure_registered();

        bool retried_stale = false;

        while (true)
        {
            if (is_open())
                return false;

            bool created_new = false;
            fd_              = shm_open(name_.c_str(), O_CREAT | O_EXCL | O_RDWR, 0666);

            if (fd_ == -1)
            {
                fd_ = shm_open(name_.c_str(), O_RDWR, 0666);
                if (fd_ == -1)
                    return false;
            }
            else
                created_new = true;

            if (!lock_file(LOCK_EX))
            {
                ::close(fd_);
                reset();
                return false;
            }

            bool invalid_header = false;
            bool success =
              created_new ? setup_new_region(initial_value) : setup_existing_region(invalid_header);

            if (!success)
            {
                if (created_new || invalid_header)
                    shm_unlink(name_.c_str());
                if (mapped_ptr_)
                    unmap_region();
                unlock_file();
                ::close(fd_);
                reset();

                if (!created_new && invalid_header && !retried_stale)
                {
                    retried_stale = true;
                    continue;
                }
                return false;
            }

            if (!lock_shared_mutex())
            {
                if (created_new)
                    shm_unlink(name_.c_str());
                if (mapped_ptr_)
                    unmap_region();
                unlock_file();
                ::close(fd_);
                reset();

                if (!created_new && !retried_stale)
                {
                    retried_stale = true;
                    continue;
                }
                return false;
            }

            if (!create_sentinel_file_locked())
            {
                unlock_shared_mutex();
                unmap_region();
                if (created_new)
                    shm_unlink(name_.c_str());
                unlock_file();
                ::close(fd_);
                reset();
                return false;
            }

            header_ptr_->ref_count.fetch_add(1, std::memory_order_acq_rel);

            unlock_shared_mutex();
            unlock_file();
            detail::SharedMemoryRegistry::register_instance(this);
            return true;
        }
    }

    void close(bool skip_unmap = false) noexcept override {
        if (fd_ == -1 && mapped_ptr_ == nullptr)
            return;

        bool remove_region = false;
        bool file_locked   = lock_file(LOCK_EX);
        bool mutex_locked  = false;

        if (file_locked && header_ptr_ != nullptr)
            mutex_locked = lock_shared_mutex();

        if (mutex_locked)
        {
            if (header_ptr_)
            {
                header_ptr_->ref_count.fetch_sub(1, std::memory_order_acq_rel);
            }
            remove_sentinel_file();
            remove_region = !has_other_live_sentinels_locked();
            unlock_shared_mutex();
        }
        else
        {
            remove_sentinel_file();
            decrement_refcount_relaxed();
        }

        if (skip_unmap)
            mapped_ptr_ = nullptr;
        else
            unmap_region();

        if (remove_region)
            shm_unlink(name_.c_str());

        if (file_locked)
            unlock_file();

        if (fd_ != -1)
        {
            ::close(fd_);
            fd_ = -1;
        }

        if (!skip_unmap)
            reset();
    }

    const std::string& name() const noexcept override { return name_; }

    [[nodiscard]] bool is_open() const noexcept { return fd_ != -1 && mapped_ptr_ && data_ptr_; }

    [[nodiscard]] const T& get() const noexcept { return *data_ptr_; }

    [[nodiscard]] const T* operator->() const noexcept { return data_ptr_; }

    [[nodiscard]] const T& operator*() const noexcept { return *data_ptr_; }

    [[nodiscard]] uint32_t ref_count() const noexcept {
        return header_ptr_ ? header_ptr_->ref_count.load(std::memory_order_acquire) : 0;
    }

    [[nodiscard]] bool is_initialized() const noexcept {
        return header_ptr_ ? header_ptr_->initialized.load(std::memory_order_acquire) : false;
    }

    static void cleanup_all_instances() noexcept { detail::SharedMemoryRegistry::cleanup_all(); }

   private:
    void reset() noexcept {
        fd_         = -1;
        mapped_ptr_ = nullptr;
        data_ptr_   = nullptr;
        header_ptr_ = nullptr;
        sentinel_path_.clear();
    }

    void unmap_region() noexcept {
        if (mapped_ptr_)
        {
            munmap(mapped_ptr_, total_size_);
            mapped_ptr_ = nullptr;
            data_ptr_   = nullptr;
            header_ptr_ = nullptr;
        }
    }

    [[nodiscard]] bool lock_file(int operation) noexcept {
        if (fd_ == -1)
            return false;

        while (flock(fd_, operation) == -1)
        {
            if (errno == EINTR)
                continue;
            return false;
        }
        return true;
    }

    void unlock_file() noexcept {
        if (fd_ == -1)
            return;

        while (flock(fd_, LOCK_UN) == -1)
        {
            if (errno == EINTR)
                continue;
            break;
        }
    }

    std::string sentinel_full_path(pid_t pid) const {
        char buf[1024];
        // See above snprintf comment
        std::snprintf(buf, sizeof(buf), "/dev/shm/%s.%ld", sentinel_base_.c_str(), long(pid));
        return buf;
    }

    void decrement_refcount_relaxed() noexcept {
        if (!header_ptr_)
            return;

        uint32_t expected = header_ptr_->ref_count.load(std::memory_order_relaxed);
        while (expected != 0
               && !header_ptr_->ref_count.compare_exchange_weak(
                 expected, expected - 1, std::memory_order_acq_rel, std::memory_order_relaxed))
        {}
    }

    bool create_sentinel_file_locked() noexcept {
        if (!header_ptr_)
            return false;

        const pid_t self_pid = getpid();
        sentinel_path_       = sentinel_full_path(self_pid);

        for (int attempt = 0; attempt < 2; ++attempt)
        {
            int fd = ::open(sentinel_path_.c_str(), O_CREAT | O_EXCL | O_WRONLY | O_CLOEXEC, 0600);
            if (fd != -1)
            {
                ::close(fd);
                return true;
            }

            if (errno == EEXIST)
            {
                ::unlink(sentinel_path_.c_str());
                decrement_refcount_relaxed();
                continue;
            }

            break;
        }

        sentinel_path_.clear();
        return false;
    }

    void remove_sentinel_file() noexcept {
        if (!sentinel_path_.empty())
        {
            ::unlink(sentinel_path_.c_str());
            sentinel_path_.clear();
        }
    }

    static bool pid_is_alive(pid_t pid) noexcept {
        if (pid <= 0)
            return false;

        if (kill(pid, 0) == 0)
            return true;

        return errno == EPERM;
    }

    [[nodiscard]] bool initialize_shared_mutex() noexcept {
        if (!header_ptr_)
            return false;

        pthread_mutexattr_t attr;
        if (pthread_mutexattr_init(&attr) != 0)
            return false;

        bool success = pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED) == 0;
#if _POSIX_C_SOURCE >= 200809L
        if (success)
            success = pthread_mutexattr_setrobust(&attr, PTHREAD_MUTEX_ROBUST) == 0;
#endif

        if (success)
            success = pthread_mutex_init(&header_ptr_->mutex, &attr) == 0;

        pthread_mutexattr_destroy(&attr);
        return success;
    }

    [[nodiscard]] bool lock_shared_mutex() noexcept {
        if (!header_ptr_)
            return false;

        while (true)
        {
            int rc = pthread_mutex_lock(&header_ptr_->mutex);
            if (rc == 0)
                return true;

#if _POSIX_C_SOURCE >= 200809L
            if (rc == EOWNERDEAD)
            {
                if (pthread_mutex_consistent(&header_ptr_->mutex) == 0)
                    return true;
                return false;
            }
#endif

            if (rc == EINTR)
                continue;

            return false;
        }
    }

    void unlock_shared_mutex() noexcept {
        if (header_ptr_)
            pthread_mutex_unlock(&header_ptr_->mutex);
    }

    bool has_other_live_sentinels_locked() const noexcept {
        DIR* dir = opendir("/dev/shm");
        if (!dir)
            return false;

        std::string prefix = sentinel_base_ + ".";
        bool        found  = false;

        while (dirent* entry = readdir(dir))
        {
            std::string name = entry->d_name;
            if (name.rfind(prefix, 0) != 0)
                continue;

            auto  pid_str = name.substr(prefix.size());
            char* end     = nullptr;
            long  value   = std::strtol(pid_str.c_str(), &end, 10);
            if (!end || *end != '\0')
                continue;

            pid_t pid = static_cast<pid_t>(value);
            if (pid_is_alive(pid))
            {
                found = true;
                break;
            }

            std::string stale_path = std::string("/dev/shm/") + name;
            ::unlink(stale_path.c_str());
            const_cast<SharedMemory*>(this)->decrement_refcount_relaxed();
        }

        closedir(dir);
        return found;
    }

    [[nodiscard]] bool setup_new_region(const T& initial_value) noexcept {
        if (ftruncate(fd_, static_cast<off_t>(total_size_)) == -1)
            return false;

        if (detail::portable_fallocate(fd_, 0, static_cast<off_t>(total_size_)) != 0)
            return false;

        mapped_ptr_ = mmap(nullptr, total_size_, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0);
        if (mapped_ptr_ == MAP_FAILED)
        {
            mapped_ptr_ = nullptr;
            return false;
        }

        data_ptr_ = static_cast<T*>(mapped_ptr_);
        header_ptr_ =
          reinterpret_cast<detail::ShmHeader*>(static_cast<char*>(mapped_ptr_) + sizeof(T));

        new (header_ptr_) detail::ShmHeader{};
        new (data_ptr_) T{initial_value};

        if (!initialize_shared_mutex())
            return false;

        header_ptr_->ref_count.store(0, std::memory_order_release);
        header_ptr_->initialized.store(true, std::memory_order_release);
        return true;
    }

    [[nodiscard]] bool setup_existing_region(bool& invalid_header) noexcept {
        invalid_header = false;

        struct stat st;
        fstat(fd_, &st);
        if (static_cast<size_t>(st.st_size) < total_size_)
        {
            invalid_header = true;
            return false;
        }

        mapped_ptr_ = mmap(nullptr, total_size_, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0);
        if (mapped_ptr_ == MAP_FAILED)
        {
            mapped_ptr_ = nullptr;
            return false;
        }

        data_ptr_   = static_cast<T*>(mapped_ptr_);
        header_ptr_ = std::launder(
          reinterpret_cast<detail::ShmHeader*>(static_cast<char*>(mapped_ptr_) + sizeof(T)));

        if (!header_ptr_->initialized.load(std::memory_order_acquire)
            || header_ptr_->magic != detail::ShmHeader::SHM_MAGIC)
        {
            invalid_header = true;
            unmap_region();
            return false;
        }

        return true;
    }
};

template<typename T>
[[nodiscard]] std::optional<SharedMemory<T>> create_shared(const std::string& name,
                                                           const T& initial_value) noexcept {
    SharedMemory<T> shm(name);
    if (shm.open(initial_value))
        return shm;
    return std::nullopt;
}

}  // namespace Alexander::shm

#endif  // #ifndef SHM_LINUX_H_INCLUDED


// ============================================================================
// FILE ORIGINALE: thread.cpp
// ============================================================================

/*
  Alexander, a UCI chess playing engine derived from Stockfish
  Copyright (C) 2004-2026 Alexander developers (see AUTHORS file)

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

#include "thread.h"

#include <algorithm>
#include <cassert>
#include <deque>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

#include "bitboard.h"
#include "history.h"
#include "memory.h"
#include "movegen.h"
#include "search.h"
#include "syzygy/tbprobe.h"
#include "timeman.h"
#include "types.h"
#include "uci.h"
#include "ucioption.h"

namespace Alexander {

// Constructor launches the thread and waits until it goes to sleep
// in idle_loop(). Note that 'searching' and 'exit' should be already set.
Thread::Thread(Search::SharedState&                    sharedState,
               std::unique_ptr<Search::ISearchManager> sm,
               size_t                                  n,
               size_t                                  numaN,
               size_t                                  totalNumaCount,
               OptionalThreadToNumaNodeBinder          binder) :
    idx(n),
    idxInNuma(numaN),
    totalNuma(totalNumaCount),
    nthreads(sharedState.options["Threads"]),
    stdThread(&Thread::idle_loop, this) {

    wait_for_search_finished();

    run_custom_job([this, &binder, &sharedState, &sm, n]() {
        // Use the binder to [maybe] bind the threads to a NUMA node before doing
        // the Worker allocation. Ideally we would also allocate the SearchManager
        // here, but that's minor.
        this->numaAccessToken = binder();
        this->worker          = make_unique_large_page<Search::Worker>(
          sharedState, std::move(sm), n, idxInNuma, totalNuma, this->numaAccessToken);
    });

    wait_for_search_finished();
}


// Destructor wakes up the thread in idle_loop() and waits
// for its termination. Thread should be already waiting.
Thread::~Thread() {

    assert(!searching);

    exit = true;
    start_searching();
    stdThread.join();
}

// Wakes up the thread that will start the search
void Thread::start_searching() {
    assert(worker != nullptr);
    run_custom_job([this]() { worker->start_searching(); });
}

// Clears the histories for the thread worker (usually before a new game)
void Thread::clear_worker() {
    assert(worker != nullptr);
    run_custom_job([this]() { worker->clear(); });
}

// Blocks on the condition variable until the thread has finished searching
void Thread::wait_for_search_finished() {

    std::unique_lock<std::mutex> lk(mutex);
    cv.wait(lk, [&] { return !searching; });
}

// Launching a function in the thread
void Thread::run_custom_job(std::function<void()> f) {
    {
        std::unique_lock<std::mutex> lk(mutex);
        cv.wait(lk, [&] { return !searching; });
        jobFunc   = std::move(f);
        searching = true;
    }
    cv.notify_one();
}

//no classical
// Thread gets parked here, blocked on the condition variable
// when the thread has no work to do.

void Thread::idle_loop() {
    while (true)
    {
        std::unique_lock<std::mutex> lk(mutex);
        searching = false;
        cv.notify_one();  // Wake up anyone waiting for search finished
        cv.wait(lk, [&] { return searching; });

        if (exit)
            return;

        std::function<void()> job = std::move(jobFunc);
        jobFunc                   = nullptr;

        lk.unlock();

        if (job)
            job();
    }
}

Search::SearchManager* ThreadPool::main_manager() { return main_thread()->worker->main_manager(); }

uint64_t ThreadPool::nodes_searched() const { return accumulate(&Search::Worker::nodes); }
uint64_t ThreadPool::tb_hits() const { return accumulate(&Search::Worker::tbHits); }

static size_t next_power_of_two(uint64_t count) { return count > 1 ? (2ULL << msb(count - 1)) : 1; }

// Creates/destroys threads to match the requested number.
// Created and launched threads will immediately go to sleep in idle_loop.
// Upon resizing, threads are recreated to allow for binding if necessary.
void ThreadPool::set(const NumaConfig&                           numaConfig,
                     Search::SharedState                         sharedState,
                     const Search::SearchManager::UpdateContext& updateContext) {

    if (threads.size() > 0)  // destroy any existing thread(s)
    {
        main_thread()->wait_for_search_finished();

        threads.clear();

        boundThreadToNumaNode.clear();
    }

    const size_t requested = sharedState.options["Threads"];

    if (requested > 0)  // create new thread(s)
    {
        // Binding threads may be problematic when there's multiple NUMA nodes and
        // multiple Stockfish instances running. In particular, if each instance
        // runs a single thread then they would all be mapped to the first NUMA node.
        // This is undesirable, and so the default behaviour (i.e. when the user does not
        // change the NumaConfig UCI setting) is to not bind the threads to processors
        // unless we know for sure that we span NUMA nodes and replication is required.
        const std::string numaPolicy(sharedState.options["NumaPolicy"]);
        const bool        doBindThreads = [&]() {
            if (numaPolicy == "none")
                return false;

            if (numaPolicy == "auto")
                return numaConfig.suggests_binding_threads(requested);

            // numaPolicy == "system", or explicitly set by the user
            return true;
        }();

        std::map<NumaIndex, size_t> counts;
        boundThreadToNumaNode = doBindThreads
                                ? numaConfig.distribute_threads_among_numa_nodes(requested)
                                : std::vector<NumaIndex>{};

        if (boundThreadToNumaNode.empty())
            counts[0] = requested;  // Pretend all threads are part of numa node 0
        else
        {
            for (size_t i = 0; i < boundThreadToNumaNode.size(); ++i)
                counts[boundThreadToNumaNode[i]]++;
        }

        sharedState.sharedHistories.clear();
        for (auto pair : counts)
        {
            NumaIndex numaIndex = pair.first;
            uint64_t  count     = pair.second;
            auto      f         = [&]() {
                sharedState.sharedHistories.try_emplace(numaIndex, next_power_of_two(count));
            };
            if (doBindThreads)
                numaConfig.execute_on_numa_node(numaIndex, f);
            else
                f();
        }

        auto threadsPerNode = counts;
        counts.clear();

        while (threads.size() < requested)
        {
            const size_t    threadId      = threads.size();
            const NumaIndex numaId        = doBindThreads ? boundThreadToNumaNode[threadId] : 0;
            auto            create_thread = [&]() {
                auto manager = threadId == 0
                                          ? std::unique_ptr<Search::ISearchManager>(
                                   std::make_unique<Search::SearchManager>(updateContext))
                                          : std::make_unique<Search::NullSearchManager>();

                // When not binding threads we want to force all access to happen
                // from the same NUMA node, because in case of NUMA replicated memory
                // accesses we don't want to trash cache in case the threads get scheduled
                // on the same NUMA node.
                auto binder = doBindThreads ? OptionalThreadToNumaNodeBinder(numaConfig, numaId)
                                                       : OptionalThreadToNumaNodeBinder(numaId);

                threads.emplace_back(std::make_unique<Thread>(sharedState, std::move(manager),
                                                                         threadId, counts[numaId]++,
                                                                         threadsPerNode[numaId], binder));
            };

            // Ensure the worker thread inherits the intended NUMA affinity at creation.
            if (doBindThreads)
                numaConfig.execute_on_numa_node(numaId, create_thread);
            else
                create_thread();
        }

        clear();

        main_thread()->wait_for_search_finished();
    }
}

//full threads patch begin
/// ThreadPool::setFull() ensures that requested threads are set to fullSearch
/// Note that fullSearch is false for each thread when they are created or rereated.
void ThreadPool::setFull(size_t requested) {

    int thisidx   = 0;
    int startfull = size() - requested;
    for (auto&& th : *this)
        th->worker->fullSearch = thisidx++ >= startfull;
}
//full threads patch end

// Sets threadPool data to initial values
void ThreadPool::clear() {
    if (threads.size() == 0)
        return;

    for (auto&& th : threads)
        th->clear_worker();

    for (auto&& th : threads)
        th->wait_for_search_finished();

    // These two affect the time taken on the first move of a game:
    main_manager()->bestPreviousAverageScore = VALUE_INFINITE;
    main_manager()->previousTimeReduction    = 0.85;

    main_manager()->callsCnt           = 0;
    main_manager()->bestPreviousScore  = VALUE_INFINITE;
    main_manager()->originalTimeAdjust = -1;
    main_manager()->tm.clear();
}

void ThreadPool::run_on_thread(size_t threadId, std::function<void()> f) {
    assert(threads.size() > threadId);
    threads[threadId]->run_custom_job(std::move(f));
}

void ThreadPool::wait_on_thread(size_t threadId) {
    assert(threads.size() > threadId);
    threads[threadId]->wait_for_search_finished();
}

size_t ThreadPool::num_threads() const { return threads.size(); }


// Wakes up main thread waiting in idle_loop() and returns immediately.
// Main thread will wake up other threads and start the search.
void ThreadPool::start_thinking(const OptionsMap&  options,
                                Position&          pos,
                                StateListPtr&      states,
                                Search::LimitsType limits) {

    main_thread()->wait_for_search_finished();

    main_manager()->stopOnPonderhit = stop = abortedSearch = false;
    main_manager()->ponder                                 = limits.ponderMode;

    increaseDepth = true;

    Search::RootMoves rootMoves;
    const auto        legalmoves = MoveList<LEGAL>(pos);

    for (const auto& uciMove : limits.searchmoves)
    {
        auto move = UCIEngine::to_move(pos, uciMove);

        if (std::find(legalmoves.begin(), legalmoves.end(), move) != legalmoves.end())
            rootMoves.emplace_back(move);
    }

    if (rootMoves.empty())
        for (const auto& m : legalmoves)
            rootMoves.emplace_back(m);

    Tablebases::Config tbConfig = Tablebases::rank_root_moves(options, pos, rootMoves);

    // After ownership transfer 'states' becomes empty, so if we stop the search
    // and call 'go' again without setting a new position states.get() == nullptr.
    assert(states.get() || setupStates.get());

    if (states.get())
        setupStates = std::move(states);  // Ownership transfer, states is now empty

    // We use Position::set() to set root position across threads. But there are
    // some StateInfo fields (previous, pliesFromNull, capturedPiece) that cannot
    // be deduced from a fen string, so set() clears them and they are set from
    // setupStates->back() later. The rootState is per thread, earlier states are
    // shared since they are read-only.
    for (auto&& th : threads)
    {
        th->run_custom_job([&]() {
            th->worker->limits = limits;
            //from Shashin Crystal begin
            (th->worker->nodes =
               (th->worker->tbHits = (th->worker->nmpGuard = (th->worker->nmpMinPly =
                                                                th->worker->bestMoveChanges = 0))));
            //from Shashin Crystal end
            th->worker->rootDepth = th->worker->completedDepth = 0;
            th->worker->nmpSide                                = 0;  //from crystal
            th->worker->rootMoves                              = rootMoves;
            th->worker->rootPos.set(pos.fen(), pos.is_chess960(), &th->worker->rootState,
                                    th.get());  //for classical
            th->worker->rootState = setupStates->back();
            th->worker->tbConfig  = tbConfig;
        });
    }

    for (auto&& th : threads)
        th->wait_for_search_finished();

    main_thread()->start_searching();
}

Thread* ThreadPool::get_best_thread() const {

    Thread* bestThread = threads.front().get();
    Value   minScore   = VALUE_NONE;

    std::unordered_map<Move, int64_t, Move::MoveHash> votes(
      2 * std::min(size(), bestThread->worker->rootMoves.size()));

    // Find the minimum score of all threads
    for (auto&& th : threads)
        minScore = std::min(minScore, th->worker->rootMoves[0].score);

    // Vote according to score and depth, and select the best thread
    auto thread_voting_value = [minScore](Thread* th) {
        return (th->worker->rootMoves[0].score - minScore + 14) * int(th->worker->completedDepth);
    };

    for (auto&& th : threads)
        votes[th->worker->rootMoves[0].pv[0]] += thread_voting_value(th.get());

    for (auto&& th : threads)
    {
        const auto bestThreadScore = bestThread->worker->rootMoves[0].score;
        const auto newThreadScore  = th->worker->rootMoves[0].score;

        const auto& bestThreadPV = bestThread->worker->rootMoves[0].pv;
        const auto& newThreadPV  = th->worker->rootMoves[0].pv;

        const auto bestThreadMoveVote = votes[bestThreadPV[0]];
        const auto newThreadMoveVote  = votes[newThreadPV[0]];

        const bool bestThreadInProvenWin = is_win(bestThreadScore);
        const bool newThreadInProvenWin  = is_win(newThreadScore);

        const bool bestThreadInProvenLoss =
          bestThreadScore != -VALUE_INFINITE && is_loss(bestThreadScore);
        const bool newThreadInProvenLoss =
          newThreadScore != -VALUE_INFINITE && is_loss(newThreadScore);

        // We make sure not to pick a thread with truncated principal variation
        const bool betterVotingValue =
          thread_voting_value(th.get()) * int(newThreadPV.size() > 2)
          > thread_voting_value(bestThread) * int(bestThreadPV.size() > 2);

        if (bestThreadInProvenWin)
        {
            // Make sure we pick the shortest mate / TB conversion
            if (newThreadScore > bestThreadScore)
                bestThread = th.get();
        }
        else if (bestThreadInProvenLoss)
        {
            // Make sure we pick the shortest mated / TB conversion
            if (newThreadInProvenLoss && newThreadScore < bestThreadScore)
                bestThread = th.get();
        }
        else if (newThreadInProvenWin || newThreadInProvenLoss
                 || (!is_loss(newThreadScore)
                     && (newThreadMoveVote > bestThreadMoveVote
                         || (newThreadMoveVote == bestThreadMoveVote && betterVotingValue))))
            bestThread = th.get();
    }

    return bestThread;
}


// Start non-main threads.
// Will be invoked by main thread after it has started searching.
void ThreadPool::start_searching() {

    for (auto&& th : threads)
        if (th != threads.front())
            th->start_searching();
}


// Wait for non-main threads
void ThreadPool::wait_for_search_finished() const {

    for (auto&& th : threads)
        if (th != threads.front())
            th->wait_for_search_finished();
}

std::vector<size_t> ThreadPool::get_bound_thread_count_by_numa_node() const {
    std::vector<size_t> counts;

    if (!boundThreadToNumaNode.empty())
    {
        NumaIndex highestNumaNode = 0;
        for (NumaIndex n : boundThreadToNumaNode)
            if (n > highestNumaNode)
                highestNumaNode = n;

        counts.resize(highestNumaNode + 1, 0);

        for (NumaIndex n : boundThreadToNumaNode)
            counts[n] += 1;
    }

    return counts;
}

}  // namespace Alexander


// ============================================================================
// FILE ORIGINALE: thread.h
// ============================================================================

/*
  Alexander, a UCI chess playing engine derived from Stockfish
  Copyright (C) 2004-2026 Alexander developers (see AUTHORS file)

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

#ifndef THREAD_H_INCLUDED
#define THREAD_H_INCLUDED

#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <vector>

#include "memory.h"
#include "numa.h"
//for classical begin
#include "material.h"
#include "pawns.h"
//for classical end
#include "position.h"
#include "search.h"
#include "thread_win32_osx.h"

namespace Alexander {


class OptionsMap;
using Value = int;

// Sometimes we don't want to actually bind the threads, but the recipient still
// needs to think it runs on *some* NUMA node, such that it can access structures
// that rely on NUMA node knowledge. This class encapsulates this optional process
// such that the recipient does not need to know whether the binding happened or not.
class OptionalThreadToNumaNodeBinder {
   public:
    OptionalThreadToNumaNodeBinder(NumaIndex n) :
        numaConfig(nullptr),
        numaId(n) {}

    OptionalThreadToNumaNodeBinder(const NumaConfig& cfg, NumaIndex n) :
        numaConfig(&cfg),
        numaId(n) {}

    NumaReplicatedAccessToken operator()() const {
        if (numaConfig != nullptr)
            return numaConfig->bind_current_thread_to_numa_node(numaId);
        else
            return NumaReplicatedAccessToken(numaId);
    }

   private:
    const NumaConfig* numaConfig;
    NumaIndex         numaId;
};

// Abstraction of a thread. It contains a pointer to the worker and a native thread.
// After construction, the native thread is started with idle_loop()
// waiting for a signal to start searching.
// When the signal is received, the thread starts searching and when
// the search is finished, it goes back to idle_loop() waiting for a new signal.
class Thread {
   public:
    Thread(Search::SharedState&,
           std::unique_ptr<Search::ISearchManager>,
           size_t,
           size_t,
           size_t,
           OptionalThreadToNumaNodeBinder);
    virtual ~Thread();

    void idle_loop();
    void start_searching();
    void clear_worker();
    void run_custom_job(std::function<void()> f);

    //no classical
    // Thread has been slightly altered to allow running custom jobs, so
    // this name is no longer correct. However, this class (and ThreadPool)
    // require further work to make them properly generic while maintaining
    // appropriate specificity regarding search, from the point of view of an
    // outside user, so renaming of this function is left for whenever that happens.
    void   wait_for_search_finished();
    size_t id() const { return idx; }

    LargePagePtr<Search::Worker> worker;
    //for classical begin
    Pawns::Table          pawnsTable;
    Material::Table       materialTable;
    Value                 bestValue;
    std::atomic<uint64_t> nodes;
    //for classical end
    std::function<void()> jobFunc;

   private:
    std::mutex                mutex;
    std::condition_variable   cv;
    size_t                    idx, idxInNuma, totalNuma, nthreads;
    bool                      exit = false, searching = true;  // Set before starting std::thread
    NativeThread              stdThread;
    NumaReplicatedAccessToken numaAccessToken;
};


// ThreadPool struct handles all the threads-related stuff like init, starting,
// parking and, most importantly, launching a thread. All the access to threads
// is done through this class.
class ThreadPool {
   public:
    ThreadPool() {}

    ~ThreadPool() {
        // destroy any existing thread(s)
        if (threads.size() > 0)
        {
            main_thread()->wait_for_search_finished();

            threads.clear();
        }
    }

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool(ThreadPool&&)      = delete;

    ThreadPool& operator=(const ThreadPool&) = delete;
    ThreadPool& operator=(ThreadPool&&)      = delete;

    void   start_thinking(const OptionsMap&, Position&, StateListPtr&, Search::LimitsType);
    void   run_on_thread(size_t threadId, std::function<void()> f);
    void   wait_on_thread(size_t threadId);
    size_t num_threads() const;
    void   clear();
    void   set(const NumaConfig& numaConfig,
               Search::SharedState,
               const Search::SearchManager::UpdateContext&);
    void   setFull(size_t);  //full threads patch

    Search::SearchManager* main_manager();
    Thread*                main_thread() const { return threads.front().get(); }
    uint64_t               nodes_searched() const;
    uint64_t               tb_hits() const;
    Thread*                get_best_thread() const;
    void                   start_searching();
    void                   wait_for_search_finished() const;

    std::vector<size_t> get_bound_thread_count_by_numa_node() const;

    //omitted for classical
    std::atomic_bool stop, abortedSearch, increaseDepth;

    auto cbegin() const noexcept { return threads.cbegin(); }
    auto begin() noexcept { return threads.begin(); }
    auto end() noexcept { return threads.end(); }
    auto cend() const noexcept { return threads.cend(); }
    auto size() const noexcept { return threads.size(); }
    auto empty() const noexcept { return threads.empty(); }

   private:
    StateListPtr                         setupStates;
    std::vector<std::unique_ptr<Thread>> threads;
    std::vector<NumaIndex>               boundThreadToNumaNode;

    uint64_t accumulate(std::atomic<uint64_t> Search::Worker::* member) const {

        uint64_t sum = 0;
        for (auto&& th : threads)
            sum += (th->worker.get()->*member).load(std::memory_order_relaxed);
        return sum;
    }
};

}  // namespace Alexander

#endif  // #ifndef THREAD_H_INCLUDED


// ============================================================================
// FILE ORIGINALE: thread_win32_osx.h
// ============================================================================

/*
  Alexander, a UCI chess playing engine derived from Stockfish
  Copyright (C) 2004-2026 Alexander developers (see AUTHORS file)

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

#ifndef THREAD_WIN32_OSX_H_INCLUDED
#define THREAD_WIN32_OSX_H_INCLUDED

#include <thread>

// On OSX threads other than the main thread are created with a reduced stack
// size of 512KB by default, this is too low for deep searches, which require
// somewhat more than 1MB stack, so adjust it to TH_STACK_SIZE.
// The implementation calls pthread_create() with the stack size parameter
// equal to the Linux 8MB default, on platforms that support it.

#if defined(__APPLE__) || defined(__MINGW32__) || defined(__MINGW64__) || defined(USE_PTHREADS)

    #include <pthread.h>
    #include <functional>

namespace Alexander {

class NativeThread {
    pthread_t thread;

    static constexpr size_t TH_STACK_SIZE = 8 * 1024 * 1024;

   public:
    template<class Function, class... Args>
    explicit NativeThread(Function&& fun, Args&&... args) {
        auto func = new std::function<void()>(
          std::bind(std::forward<Function>(fun), std::forward<Args>(args)...));

        pthread_attr_t attr_storage, *attr = &attr_storage;
        pthread_attr_init(attr);
        pthread_attr_setstacksize(attr, TH_STACK_SIZE);

        auto start_routine = [](void* ptr) -> void* {
            auto f = reinterpret_cast<std::function<void()>*>(ptr);
            // Call the function
            (*f)();
            delete f;
            return nullptr;
        };

        pthread_create(&thread, attr, start_routine, func);
    }

    void join() { pthread_join(thread, nullptr); }
};

}  // namespace Alexander

#else  // Default case: use STL classes

namespace Alexander {

using NativeThread = std::thread;

}  // namespace Alexander

#endif

#endif  // #ifndef THREAD_WIN32_OSX_H_INCLUDED


// ============================================================================
// FILE ORIGINALE: timeman.cpp
// ============================================================================

/*
  Alexander, a UCI chess playing engine derived from Stockfish
  Copyright (C) 2004-2026 Alexander developers (see AUTHORS file)

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

#include "timeman.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>

#include "search.h"
#include "ucioption.h"

namespace Alexander {

TimePoint TimeManagement::optimum() const { return optimumTime; }
TimePoint TimeManagement::maximum() const { return maximumTime; }

void TimeManagement::clear() {
    availableNodes = -1;  // When in 'nodes as time' mode
}

void TimeManagement::advance_nodes_time(std::int64_t nodes) {
    assert(useNodesTime);
    availableNodes = std::max(int64_t(0), availableNodes - nodes);
}

// Called at the beginning of the search and calculates
// the bounds of time allowed for the current game ply. We currently support:
//      1) x basetime (+ z increment)
//      2) x moves in y seconds (+ z increment)
void TimeManagement::init(Search::LimitsType& limits,
                          Color               us,
                          int                 ply,
                          const OptionsMap&   options,
                          double&             originalTimeAdjust) {
    TimePoint npmsec = TimePoint(options["nodestime"]);

    // If we have no time, we don't need to fully initialize TM.
    // startTime is used by movetime and useNodesTime is used in elapsed calls.
    startTime    = limits.startTime;
    useNodesTime = npmsec != 0;

    if (limits.time[us] == 0)
    //minThinkigTime begin
    { return; }
    TimePoint minThinkingTime = TimePoint(options["Minimum Thinking Time"]);
    //minThinkigTime end
    TimePoint moveOverhead = TimePoint(options["Move Overhead"]);
    TimePoint slowMover    = TimePoint(options["Slow Mover"]);  //for SlowMover

    // optScale is a percentage of available time to use for the current move.
    // maxScale is a multiplier applied to optimumTime.
    double optScale, maxScale;

    // If we have to play in 'nodes as time' mode, then convert from time
    // to nodes, and use resulting values in time management formulas.
    // WARNING: to avoid time losses, the given npmsec (nodes per millisecond)
    // must be much lower than the real engine speed.
    if (useNodesTime)
    {
        if (availableNodes == -1)                       // Only once at game start
            availableNodes = npmsec * limits.time[us];  // Time is in msec

        // Convert from milliseconds to nodes
        limits.time[us] = TimePoint(availableNodes);
        limits.inc[us] *= npmsec;
        limits.npmsec = npmsec;
        moveOverhead *= npmsec;
    }

    // These numbers are used where multiplications, divisions or comparisons
    // with constants are involved.
    const int64_t   scaleFactor = useNodesTime ? npmsec : 1;
    const TimePoint scaledTime  = limits.time[us] / scaleFactor;

    // Maximum move horizon
    int centiMTG = limits.movestogo ? std::min(limits.movestogo * 100, 5000) : 5051;

    // If less than one second, gradually reduce mtg
    if (scaledTime < 1000)
        centiMTG = int(scaledTime * 5.051);

    // Make sure timeLeft is > 0 since we may use it as a divisor
    TimePoint timeLeft =
      std::max(TimePoint(1),
               limits.time[us]
                 + (limits.inc[us] * (centiMTG - 100) - moveOverhead * (200 + centiMTG)) / 100);

    //from SlowMover begin
    // A user may scale time usage by setting UCI option "Slow Mover"
    // Default is 100 and changing this value will probably lose elo.
    timeLeft = slowMover * timeLeft / 100;
    //from SlowMover end

    // x basetime (+ z increment)
    // If there is a healthy increment, timeLeft can exceed the actual available
    // game time for the current move, so also cap to a percentage of available game time.
    if (limits.movestogo == 0)
    {
        // Extra time according to timeLeft
        if (originalTimeAdjust < 0)
            originalTimeAdjust = 0.3128 * std::log10(timeLeft) - 0.4354;

        // Calculate time constants based on current time left.
        double logTimeInSec = std::log10(scaledTime / 1000.0);
        double optConstant  = std::min(0.0032116 + 0.000321123 * logTimeInSec, 0.00508017);
        double maxConstant  = std::max(3.3977 + 3.03950 * logTimeInSec, 2.94761);

        optScale = std::min(0.0121431 + std::pow(ply + 2.94693, 0.461073) * optConstant,
                            0.213035 * limits.time[us] / timeLeft)
                 * originalTimeAdjust;

        maxScale = std::min(6.67704, maxConstant + ply / 11.9847);
    }

    // x moves in y seconds (+ z increment)
    else
    {
        optScale =
          std::min((0.88 + ply / 116.4) / (centiMTG / 100.0), 0.88 * limits.time[us] / timeLeft);
        maxScale = 1.3 + 0.11 * (centiMTG / 100.0);
    }

    // Limit the maximum possible time for this move
    optimumTime = std::max(minThinkingTime, TimePoint(optScale * timeLeft));  //minThinkingTime
    maximumTime =
      TimePoint(std::min(0.825179 * limits.time[us] - moveOverhead, maxScale * optimumTime)) - 10;

    if (options["Ponder"])
        optimumTime += optimumTime / 4;
}

}  // namespace Alexander


// ============================================================================
// FILE ORIGINALE: timeman.h
// ============================================================================

/*
  Alexander, a UCI chess playing engine derived from Stockfish
  Copyright (C) 2004-2026 Alexander developers (see AUTHORS file)

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

#ifndef TIMEMAN_H_INCLUDED
#define TIMEMAN_H_INCLUDED

#include <cstdint>

#include "misc.h"

namespace Alexander {

class OptionsMap;
enum Color : uint8_t;

namespace Search {
struct LimitsType;
}

// The TimeManagement class computes the optimal time to think depending on
// the maximum available time, the game move number, and other parameters.
class TimeManagement {
   public:
    void init(Search::LimitsType& limits,
              Color               us,
              int                 ply,
              const OptionsMap&   options,
              double&             originalTimeAdjust);

    TimePoint optimum() const;
    TimePoint maximum() const;
    template<typename FUNC>
    TimePoint elapsed(FUNC nodes) const {
        return useNodesTime ? TimePoint(nodes()) : elapsed_time();
    }
    TimePoint elapsed_time() const { return now() - startTime; };

    void clear();
    void advance_nodes_time(std::int64_t nodes);

   private:
    TimePoint startTime;
    TimePoint optimumTime;
    TimePoint maximumTime;

    std::int64_t availableNodes = -1;     // When in 'nodes as time' mode
    bool         useNodesTime   = false;  // True if we are in 'nodes as time' mode
};

}  // namespace Alexander

#endif  // #ifndef TIMEMAN_H_INCLUDED


// ============================================================================
// FILE ORIGINALE: tt.cpp
// ============================================================================

/*
  Alexander, a UCI chess playing engine derived from Stockfish
  Copyright (C) 2004-2026 Alexander developers (see AUTHORS file)

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

#include "tt.h"

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include "memory.h"
#include "misc.h"
#include "syzygy/tbprobe.h"
#include "thread.h"

namespace Alexander {
// TTEntry struct is the 10 bytes transposition table entry, defined as below:
//
// key        16 bit
// depth       8 bit
// generation  5 bit
// pv node     1 bit
// bound type  2 bit
// move       16 bit
// value      16 bit
// evaluation 16 bit
//
// These fields are in the same order as accessed by TT::probe(), since memory is fastest sequentially.
// Equally, the store order in save() matches this order.

struct TTEntry {

    // Convert internal bitfields to external types
    TTData read() const {
        return TTData{Move(move16),           Value(value16),
                      Value(eval16),          Depth(depth8 + DEPTH_ENTRY_OFFSET),
                      Bound(genBound8 & 0x3), bool(genBound8 & 0x4)};
    }

    bool is_occupied() const;
    void save(Key k, Value v, bool pv, Bound b, Depth d, Move m, Value ev, uint8_t generation8);
    // The returned age is a multiple of TranspositionTable::GENERATION_DELTA
    uint8_t relative_age(const uint8_t generation8) const;

   private:
    friend class TranspositionTable;

    uint16_t key16;
    uint8_t  depth8;
    uint8_t  genBound8;
    Move     move16;
    int16_t  value16;
    int16_t  eval16;
};

// `genBound8` is where most of the details are. We use the following constants to manipulate 5 leading generation bits
// and 3 trailing miscellaneous bits.

// These bits are reserved for other things.
static constexpr unsigned GENERATION_BITS = 3;
// increment for generation field
static constexpr int GENERATION_DELTA = (1 << GENERATION_BITS);
// cycle length
static constexpr int GENERATION_CYCLE = 255 + GENERATION_DELTA;
// mask to pull out generation number
static constexpr int GENERATION_MASK = (0xFF << GENERATION_BITS) & 0xFF;

// DEPTH_ENTRY_OFFSET exists because 1) we use `bool(depth8)` as the occupancy check, but
// 2) we need to store negative depths for QS. (`depth8` is the only field with "spare bits":
// we sacrifice the ability to store depths greater than 1<<8 less the offset, as asserted in `save`.)
bool TTEntry::is_occupied() const { return bool(depth8); }

// Populates the TTEntry with a new node's data, possibly
// overwriting an old position. The update is not atomic and can be racy.
void TTEntry::save(
  Key k, Value v, bool pv, Bound b, Depth d, Move m, Value ev, uint8_t generation8) {

    // Preserve the old ttmove if we don't have a new one
    if (m || uint16_t(k) != key16)
        move16 = m;

    // Overwrite less valuable entries (cheapest checks first)
    if (b == BOUND_EXACT || uint16_t(k) != key16 || d - DEPTH_ENTRY_OFFSET + 2 * pv > depth8 - 4
        || relative_age(generation8))
    {
        assert(d > DEPTH_ENTRY_OFFSET);
        assert(d < 256 + DEPTH_ENTRY_OFFSET);

        key16     = uint16_t(k);
        depth8    = uint8_t(d - DEPTH_ENTRY_OFFSET);
        genBound8 = uint8_t(generation8 | uint8_t(pv) << 2 | b);
        value16   = int16_t(v);
        eval16    = int16_t(ev);
    }
}


uint8_t TTEntry::relative_age(const uint8_t generation8) const {
    // Due to our packed storage format for generation and its cyclic
    // nature we add GENERATION_CYCLE (256 is the modulus, plus what
    // is needed to keep the unrelated lowest n bits from affecting
    // the result) to calculate the entry age correctly even after
    // generation8 overflows into the next cycle.
    return (GENERATION_CYCLE + generation8 - genBound8) & GENERATION_MASK;
}


// TTWriter is but a very thin wrapper around the pointer
TTWriter::TTWriter(TTEntry* tte) :
    entry(tte) {}

void TTWriter::write(
  Key k, Value v, bool pv, Bound b, Depth d, Move m, Value ev, uint8_t generation8) {
    entry->save(k, v, pv, b, d, m, ev, generation8);
}


// A TranspositionTable is an array of Cluster, of size clusterCount. Each cluster consists of ClusterSize number
// of TTEntry. Each non-empty TTEntry contains information on exactly one position. The size of a Cluster should
// divide the size of a cache line for best performance, as the cacheline is prefetched when possible.

static constexpr int ClusterSize = 3;

struct Cluster {
    TTEntry entry[ClusterSize];
    char    padding[2];  // Pad to 32 bytes
};

static_assert(sizeof(Cluster) == 32, "Suboptimal Cluster size");


// Sets the size of the transposition table,
// measured in megabytes. Transposition table consists
// of clusters and each cluster consists of ClusterSize number of TTEntry.
void TranspositionTable::resize(size_t mbSize, ThreadPool& threads) {
    aligned_large_pages_free(table);

    clusterCount = mbSize * 1024 * 1024 / sizeof(Cluster);

    table = static_cast<Cluster*>(aligned_large_pages_alloc(clusterCount * sizeof(Cluster)));

    if (!table)
    {
        std::cerr << "Failed to allocate " << mbSize << "MB for transposition table." << std::endl;
        exit(EXIT_FAILURE);
    }

    clear(threads);
}


// Initializes the entire transposition table to zero,
// in a multi-threaded way.
void TranspositionTable::clear(ThreadPool& threads) {
    generation8              = 0;
    const size_t threadCount = threads.num_threads();

    for (size_t i = 0; i < threadCount; ++i)
    {
        threads.run_on_thread(i, [this, i, threadCount]() {
            // Each thread will zero its part of the hash table
            const size_t stride = clusterCount / threadCount;
            const size_t start  = stride * i;
            const size_t len    = i + 1 != threadCount ? stride : clusterCount - start;

            std::memset(&table[start], 0, len * sizeof(Cluster));
        });
    }

    for (size_t i = 0; i < threadCount; ++i)
        threads.wait_on_thread(i);
}


// Returns an approximation of the hashtable
// occupation during a search. The hash is x permill full, as per UCI protocol.
// Only counts entries which match the current generation.
int TranspositionTable::hashfull(int maxAge) const {
    int maxAgeInternal = maxAge << GENERATION_BITS;
    int cnt            = 0;
    for (int i = 0; i < 1000; ++i)
        for (int j = 0; j < ClusterSize; ++j)
            cnt += table[i].entry[j].is_occupied()
                && table[i].entry[j].relative_age(generation8) <= maxAgeInternal;

    return cnt / ClusterSize;
}


void TranspositionTable::new_search() {
    // increment by delta to keep lower bits as is
    generation8 += GENERATION_DELTA;
}


uint8_t TranspositionTable::generation() const { return generation8; }


// Looks up the current position in the transposition
// table. It returns true if the position is found.
// Otherwise, it returns false and a pointer to an empty or least valuable TTEntry
// to be replaced later. The replace value of an entry is calculated as its depth
// minus 8 times its relative age. TTEntry t1 is considered more valuable than
// TTEntry t2 if its replace value is greater than that of t2.
std::tuple<bool, TTData, TTWriter> TranspositionTable::probe(const Key key) const {

    TTEntry* const tte   = first_entry(key);
    const uint16_t key16 = uint16_t(key);  // Use the low 16 bits as key inside the cluster

    for (int i = 0; i < ClusterSize; ++i)
        if (tte[i].key16 == key16)
            // This gap is the main place for read races.
            // After `read()` completes that copy is final, but may be self-inconsistent.
            return {tte[i].is_occupied(), tte[i].read(), TTWriter(&tte[i])};

    // Find an entry to be replaced according to the replacement strategy
    TTEntry* replace = tte;
    for (int i = 1; i < ClusterSize; ++i)
        if (replace->depth8 - replace->relative_age(generation8)
            > tte[i].depth8 - tte[i].relative_age(generation8))
            replace = &tte[i];

    return {false,
            TTData{Move::none(), VALUE_NONE, VALUE_NONE, DEPTH_ENTRY_OFFSET, BOUND_NONE, false},
            TTWriter(replace)};
}


TTEntry* TranspositionTable::first_entry(const Key key) const {
    return &table[mul_hi64(key, clusterCount)].entry[0];
}

}  // namespace Alexander


// ============================================================================
// FILE ORIGINALE: tt.h
// ============================================================================

/*
  Alexander, a UCI chess playing engine derived from Stockfish
  Copyright (C) 2004-2026 Alexander developers (see AUTHORS file)

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

#ifndef TT_H_INCLUDED
#define TT_H_INCLUDED

#include <cstddef>
#include <cstdint>
#include <tuple>

#include "memory.h"
#include "types.h"

namespace Alexander {

class ThreadPool;
struct TTEntry;
struct Cluster;

// There is only one global hash table for the engine and all its threads. For chess in particular, we even allow racy
// updates between threads to and from the TT, as taking the time to synchronize access would cost thinking time and
// thus elo. As a hash table, collisions are possible and may cause chess playing issues (bizarre blunders, faulty mate
// reports, etc). Fixing these also loses elo; however such risk decreases quickly with larger TT size.
//
// `probe` is the primary method: given a board position, we lookup its entry in the table, and return a tuple of:
//   1) whether the entry already has this position
//   2) a copy of the prior data (if any) (may be inconsistent due to read races)
//   3) a writer object to this entry
// The copied data and the writer are separated to maintain clear boundaries between local vs global objects.


// A copy of the data already in the entry (possibly collided). `probe` may be racy, resulting in inconsistent data.
struct TTData {
    Move  move;
    Value value, eval;
    Depth depth;
    Bound bound;
    bool  is_pv;

    TTData() = delete;

    // clang-format off
    TTData(Move m, Value v, Value ev, Depth d, Bound b, bool pv) :
        move(m),
        value(v),
        eval(ev),
        depth(d),
        bound(b),
        is_pv(pv) {};
    // clang-format on
};


// This is used to make racy writes to the global TT.
struct TTWriter {
   public:
    void write(Key k, Value v, bool pv, Bound b, Depth d, Move m, Value ev, uint8_t generation8);

   private:
    friend class TranspositionTable;
    TTEntry* entry;
    TTWriter(TTEntry* tte);
};


class TranspositionTable {

   public:
    ~TranspositionTable() { aligned_large_pages_free(table); }

    void resize(size_t mbSize, ThreadPool& threads);  // Set TT size
    void clear(ThreadPool& threads);                  // Re-initialize memory, multithreaded
    int  hashfull(int maxAge = 0)
      const;  // Approximate what fraction of entries (permille) have been written to during this root search

    void
    new_search();  // This must be called at the beginning of each root search to track entry aging
    uint8_t generation() const;  // The current age, used when writing new data to the TT
    std::tuple<bool, TTData, TTWriter>
    probe(const Key key) const;  // The main method, whose retvals separate local vs global objects
    TTEntry* first_entry(const Key key)
      const;  // This is the hash function; its only external use is memory prefetching.

   private:
    friend struct TTEntry;

    size_t   clusterCount;
    Cluster* table = nullptr;

    uint8_t generation8 = 0;  // Size must be not bigger than TTEntry::genBound8
};

}  // namespace Alexander

#endif  // #ifndef TT_H_INCLUDED


// ============================================================================
// FILE ORIGINALE: tune.cpp
// ============================================================================

/*
  Alexander, a UCI chess playing engine derived from Stockfish
  Copyright (C) 2004-2026 Alexander developers (see AUTHORS file)

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

#include "tune.h"

#include <algorithm>
#include <iostream>
#include <map>
#include <optional>
#include <sstream>
#include <string>

#include "ucioption.h"

using std::string;

namespace Alexander {

bool          Tune::update_on_last;
const Option* LastOption = nullptr;
OptionsMap*   Tune::options;
namespace {
std::map<std::string, int> TuneResults;

std::optional<std::string> on_tune(const Option& o) {

    if (!Tune::update_on_last || LastOption == &o)
        Tune::read_options();

    return std::nullopt;
}
}

void Tune::make_option(OptionsMap* opts, const string& n, int v, const SetRange& r) {

    // Do not generate option when there is nothing to tune (ie. min = max)
    if (r(v).first == r(v).second)
        return;

    if (TuneResults.count(n))
        v = TuneResults[n];

    opts->add(n, Option(v, r(v).first, r(v).second, on_tune));
    LastOption = &((*opts)[n]);

    // Print formatted parameters, ready to be copy-pasted in Fishtest
    std::cout << n << ","                                  //
              << v << ","                                  //
              << r(v).first << ","                         //
              << r(v).second << ","                        //
              << (r(v).second - r(v).first) / 20.0 << ","  //
              << "0.0020" << std::endl;
}

string Tune::next(string& names, bool pop) {

    string name;

    do
    {
        string token = names.substr(0, names.find(','));

        if (pop)
            names.erase(0, token.size() + 1);

        std::stringstream ws(token);
        name += (ws >> token, token);  // Remove trailing whitespace

    } while (std::count(name.begin(), name.end(), '(') - std::count(name.begin(), name.end(), ')'));

    return name;
}


template<>
void Tune::Entry<int>::init_option() {
    make_option(options, name, value, range);
}

template<>
void Tune::Entry<int>::read_option() {
    if (options->count(name))
        value = int((*options)[name]);
}

// Instead of a variable here we have a PostUpdate function: just call it
template<>
void Tune::Entry<Tune::PostUpdate>::init_option() {}
template<>
void Tune::Entry<Tune::PostUpdate>::read_option() {
    value();
}

}  // namespace Alexander


// Init options with tuning session results instead of default values. Useful to
// get correct bench signature after a tuning session or to test tuned values.
// Just copy fishtest tuning results in a result.txt file and extract the
// values with:
//
// cat results.txt | sed 's/^param: \([^,]*\), best: \([^,]*\).*/  TuneResults["\1"] = int(round(\2));/'
//
// Then paste the output below, as the function body


namespace Alexander {

void Tune::read_results() { /* ...insert your values here... */ }

}  // namespace Alexander


// ============================================================================
// FILE ORIGINALE: tune.h
// ============================================================================

/*
  Alexander, a UCI chess playing engine derived from Stockfish
  Copyright (C) 2004-2026 Alexander developers (see AUTHORS file)

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

#ifndef TUNE_H_INCLUDED
#define TUNE_H_INCLUDED

#include <cstddef>
#include <memory>
#include <string>
#include <type_traits>  // IWYU pragma: keep
#include <utility>
#include <vector>

namespace Alexander {

class OptionsMap;

using Range    = std::pair<int, int>;  // Option's min-max values
using RangeFun = Range(int);

// Default Range function, to calculate Option's min-max values
inline Range default_range(int v) { return v > 0 ? Range(0, 2 * v) : Range(2 * v, 0); }

struct SetRange {
    explicit SetRange(RangeFun f) :
        fun(f) {}
    SetRange(int min, int max) :
        fun(nullptr),
        range(min, max) {}
    Range operator()(int v) const { return fun ? fun(v) : range; }

    RangeFun* fun;
    Range     range;
};

#define SetDefaultRange SetRange(default_range)


// Tune class implements the 'magic' code that makes the setup of a fishtest tuning
// session as easy as it can be. Mainly you have just to remove const qualifiers
// from the variables you want to tune and flag them for tuning, so if you have:
//
//   const Value myValue[][2] = { { V(100), V(20) }, { V(7), V(78) } };
//
// If you have a my_post_update() function to run after values have been updated,
// and a my_range() function to set custom Option's min-max values, then you just
// remove the 'const' qualifiers and write somewhere below in the file:
//
//   TUNE(SetRange(my_range), myValue, my_post_update);
//
// You can also set the range directly, and restore the default at the end
//
//   TUNE(SetRange(-100, 100), myValue, SetDefaultRange);
//
// In case update function is slow and you have many parameters, you can add:
//
//   UPDATE_ON_LAST();
//
// And the values update, including post update function call, will be done only
// once, after the engine receives the last UCI option, that is the one defined
// and created as the last one, so the GUI should send the options in the same
// order in which have been defined.

class Tune {

    using PostUpdate = void();  // Post-update function

    Tune() { read_results(); }
    Tune(const Tune&)           = delete;
    void operator=(const Tune&) = delete;
    void read_results();

    static Tune& instance() {
        static Tune t;
        return t;
    }  // Singleton

    // Use polymorphism to accommodate Entry of different types in the same vector
    struct EntryBase {
        virtual ~EntryBase()       = default;
        virtual void init_option() = 0;
        virtual void read_option() = 0;
    };

    template<typename T>
    struct Entry: public EntryBase {

        static_assert(!std::is_const_v<T>, "Parameter cannot be const!");

        static_assert(std::is_same_v<T, int> || std::is_same_v<T, PostUpdate>,
                      "Parameter type not supported!");

        Entry(const std::string& n, T& v, const SetRange& r) :
            name(n),
            value(v),
            range(r) {}
        void operator=(const Entry&) = delete;  // Because 'value' is a reference
        void init_option() override;
        void read_option() override;

        std::string name;
        T&          value;
        SetRange    range;
    };

    // Our facility to fill the container, each Entry corresponds to a parameter
    // to tune. We use variadic templates to deal with an unspecified number of
    // entries, each one of a possible different type.
    static std::string next(std::string& names, bool pop = true);

    int add(const SetRange&, std::string&&) { return 0; }

    template<typename T, typename... Args>
    int add(const SetRange& range, std::string&& names, T& value, Args&&... args) {
        list.push_back(std::unique_ptr<EntryBase>(new Entry<T>(next(names), value, range)));
        return add(range, std::move(names), args...);
    }

    // Template specialization for arrays: recursively handle multi-dimensional arrays
    template<typename T, size_t N, typename... Args>
    int add(const SetRange& range, std::string&& names, T (&value)[N], Args&&... args) {
        for (size_t i = 0; i < N; i++)
            add(range, next(names, i == N - 1) + "[" + std::to_string(i) + "]", value[i]);
        return add(range, std::move(names), args...);
    }

    // Template specialization for SetRange
    template<typename... Args>
    int add(const SetRange&, std::string&& names, SetRange& value, Args&&... args) {
        return add(value, (next(names), std::move(names)), args...);
    }

    static void make_option(OptionsMap* options, const std::string& n, int v, const SetRange& r);

    std::vector<std::unique_ptr<EntryBase>> list;

   public:
    template<typename... Args>
    static int add(const std::string& names, Args&&... args) {
        return instance().add(SetDefaultRange, names.substr(1, names.size() - 2),
                              args...);  // Remove trailing parenthesis
    }
    static void init(OptionsMap& o) {
        options = &o;
        for (auto& e : instance().list)
            e->init_option();
        read_options();
    }  // Deferred, due to UCIEngine::Options access
    static void read_options() {
        for (auto& e : instance().list)
            e->read_option();
    }

    static bool        update_on_last;
    static OptionsMap* options;
};

template<typename... Args>
constexpr void tune_check_args(Args&&...) {
    static_assert((!std::is_fundamental_v<Args> && ...), "TUNE macro arguments wrong");
}

// Some macro magic :-) we define a dummy int variable that the compiler initializes calling Tune::add()
#define STRINGIFY(x) #x
#define UNIQUE2(x, y) x##y
#define UNIQUE(x, y) UNIQUE2(x, y)  // Two indirection levels to expand __LINE__
#define TUNE(...) \
    int UNIQUE(p, __LINE__) = []() -> int { \
        tune_check_args(__VA_ARGS__); \
        return Tune::add(STRINGIFY((__VA_ARGS__)), __VA_ARGS__); \
    }();

#define UPDATE_ON_LAST() bool UNIQUE(p, __LINE__) = Tune::update_on_last = true

}  // namespace Alexander

#endif  // #ifndef TUNE_H_INCLUDED


// ============================================================================
// FILE ORIGINALE: types.h
// ============================================================================

/*
  Alexander, a UCI chess playing engine derived from Stockfish
  Copyright (C) 2004-2026 Andrea Manzo, F. Ferraguti, K.Kiniama and Stockfish developers (see AUTHORS file)

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

#ifndef TYPES_H_INCLUDED
    #define TYPES_H_INCLUDED

// When compiling with provided Makefile (e.g. for Linux and OSX), configuration
// is done automatically. To get started type 'make help'.
//
// When Makefile is not used (e.g. with Microsoft Visual Studio) some switches
// need to be set manually:
//
// -DNDEBUG      | Disable debugging mode. Always use this for release.
//
// -DNO_PREFETCH | Disable use of prefetch asm-instruction. You may need this to
//               | run on some very old machines.
//
// -DUSE_POPCNT  | Add runtime support for use of popcnt asm-instruction. Works
//               | only in 64-bit mode and requires hardware with popcnt support.
//
// -DUSE_PEXT    | Add runtime support for use of pext asm-instruction. Works
//               | only in 64-bit mode and requires hardware with pext support.

    #include <cassert>
    #include <cstddef>
    #include <cstdint>
    #include <type_traits>
//shashchess begin
    #if defined(__GNUC__) && !defined(__clang__)
        #if __GNUC__ >= 13
            #define sf_assume(cond) __attribute__((assume(cond)))
        #else
            #define sf_assume(cond) \
                do \
                { \
                    if (!(cond)) \
                        __builtin_unreachable(); \
                } while (0)
        #endif
    #else
        // do nothing for other compilers
        #define sf_assume(cond)
    #endif

// Definizione ValueList spostata qui
template<typename T, std::size_t MaxSize>
class ValueList {

   public:
    std::size_t size() const { return size_; }
    int         ssize() const { return int(size_); }
    void        push_back(const T& value) {
        assert(size_ < MaxSize);
        values_[size_++] = value;
    }
    const T* begin() const { return values_; }
    const T* end() const { return values_ + size_; }
    const T& operator[](int index) const { return values_[index]; }

    T* make_space(size_t count) {
        T* result = &values_[size_];
        size_ += count;
        assert(size_ <= MaxSize);
        return result;
    }

   private:
    T           values_[MaxSize];
    std::size_t size_ = 0;
};
    //shashchess end
    #if defined(_MSC_VER)
        // Disable some silly and noisy warnings from MSVC compiler
        #pragma warning(disable: 4127)  // Conditional expression is constant
        #pragma warning(disable: 4146)  // Unary minus operator applied to unsigned type
        #pragma warning(disable: 4800)  // Forcing value to bool 'true' or 'false'
    #endif

// Predefined macros hell:
//
// __GNUC__                Compiler is GCC, Clang or ICX
// __clang__               Compiler is Clang or ICX
// __INTEL_LLVM_COMPILER   Compiler is ICX
// _MSC_VER                Compiler is MSVC
// _WIN32                  Building on Windows (any)
// _WIN64                  Building on Windows 64 bit

// Enforce minimum GCC version
    #if defined(__GNUC__) && !defined(__clang__) \
      && (__GNUC__ < 9 || (__GNUC__ == 9 && __GNUC_MINOR__ < 3))
        #error "Stockfish requires GCC 9.3 or later for correct compilation"
    #endif

    // Enforce minimum Clang version
    #if defined(__clang__) && (__clang_major__ < 10)
        #error "Stockfish requires Clang 10.0 or later for correct compilation"
    #endif

    #define ASSERT_ALIGNED(ptr, alignment) assert(reinterpret_cast<uintptr_t>(ptr) % alignment == 0)

    #if defined(_WIN64) && defined(_MSC_VER)  // No Makefile used
        #include <intrin.h>                   // Microsoft header for _BitScanForward64()
        #define IS_64BIT
    #endif

    #if defined(USE_POPCNT) && defined(_MSC_VER)
        #include <nmmintrin.h>  // Microsoft header for _mm_popcnt_u64()
    #endif

    #if !defined(NO_PREFETCH) && defined(_MSC_VER)
        #include <xmmintrin.h>  // Microsoft header for _mm_prefetch()
    #endif

    #if defined(USE_PEXT)
        #include <immintrin.h>  // Header for _pext_u64() intrinsic
        #define pext(b, m) _pext_u64(b, m)
    #else
        #define pext(b, m) 0
    #endif

namespace Alexander {

    #ifdef USE_POPCNT
constexpr bool HasPopCnt = true;
    #else
constexpr bool HasPopCnt = false;
    #endif

    #ifdef USE_PEXT
constexpr bool HasPext = true;
    #else
constexpr bool HasPext = false;
    #endif

    #ifdef IS_64BIT
constexpr bool Is64Bit = true;
    #else
constexpr bool Is64Bit = false;
    #endif

using Key      = uint64_t;
using Bitboard = uint64_t;

constexpr int MAX_MOVES = 256;
constexpr int MAX_PLY   = 246;

enum Color : uint8_t {
    WHITE,
    BLACK,
    COLOR_NB = 2
};
//learning begin
constexpr Color Colors[2] = {WHITE, BLACK};

enum CastlingRights : uint8_t {
    NO_CASTLING,
    WHITE_OO,
    WHITE_OOO = WHITE_OO << 1,
    BLACK_OO  = WHITE_OO << 2,
    BLACK_OOO = WHITE_OO << 3,

    KING_SIDE      = WHITE_OO | BLACK_OO,
    QUEEN_SIDE     = WHITE_OOO | BLACK_OOO,
    WHITE_CASTLING = WHITE_OO | WHITE_OOO,
    BLACK_CASTLING = BLACK_OO | BLACK_OOO,
    ANY_CASTLING   = WHITE_CASTLING | BLACK_CASTLING,

    CASTLING_RIGHT_NB = 16
};
//learning end
//for classical begin
enum Phase {
    PHASE_ENDGAME,
    PHASE_MIDGAME = 128,
    MG            = 0,
    EG            = 1,
    PHASE_NB      = 2
};

enum ScaleFactor {
    SCALE_FACTOR_DRAW   = 0,
    SCALE_FACTOR_NORMAL = 64,
    SCALE_FACTOR_MAX    = 128,
    SCALE_FACTOR_NONE   = 255
};
//for classical end
enum Bound : uint8_t {
    BOUND_NONE,
    BOUND_UPPER,
    BOUND_LOWER,
    BOUND_EXACT = BOUND_UPPER | BOUND_LOWER
};

// Value is used as an alias for int, this is done to differentiate between a search
// value and any other integer value. The values used in search are always supposed
// to be in the range (-VALUE_NONE, VALUE_NONE] and should not exceed this range.
using Value = int;

constexpr Value VALUE_ZERO      = 0;
constexpr Value VALUE_DRAW      = 0;
constexpr Value VALUE_KNOWN_WIN = 10000;  //mcts
constexpr Value VALUE_NONE      = 32002;
constexpr Value VALUE_INFINITE  = 32001;

constexpr Value VALUE_MATE             = 32000;
constexpr Value VALUE_MATE_IN_MAX_PLY  = VALUE_MATE - MAX_PLY;
constexpr Value VALUE_MATED_IN_MAX_PLY = -VALUE_MATE_IN_MAX_PLY;

constexpr Value VALUE_TB                 = VALUE_MATE_IN_MAX_PLY - 1;
constexpr Value VALUE_TB_WIN_IN_MAX_PLY  = VALUE_TB - MAX_PLY;
constexpr Value VALUE_TB_LOSS_IN_MAX_PLY = -VALUE_TB_WIN_IN_MAX_PLY;


constexpr bool is_valid(Value value) { return value != VALUE_NONE; }

constexpr bool is_win(Value value) {
    assert(is_valid(value));
    return value >= VALUE_TB_WIN_IN_MAX_PLY;
}

constexpr bool is_loss(Value value) {
    assert(is_valid(value));
    return value <= VALUE_TB_LOSS_IN_MAX_PLY;
}

constexpr bool is_decisive(Value value) { return is_win(value) || is_loss(value); }

// In the code, we make the assumption that these values
// are such that non_pawn_material() can be used to uniquely
// identify the material on the board.
constexpr Value PawnValue   = 208;
constexpr Value KnightValue = 781;
constexpr Value BishopValue = 825;
constexpr Value RookValue   = 1276;
constexpr Value QueenValue  = 2538;
//from classical eval begin
constexpr Value PawnValueMg   = 126;
constexpr Value PawnValueEg   = 208;
constexpr Value KnightValueMg = 781;
constexpr Value KnightValueEg = 854;
constexpr Value BishopValueMg = 825;
constexpr Value BishopValueEg = 915;
constexpr Value RookValueMg   = 1276;
constexpr Value RookValueEg   = 1380;
constexpr Value QueenValueMg  = 2538;
constexpr Value QueenValueEg  = 2682;
constexpr Value MidgameLimit  = 15258;
constexpr Value EndgameLimit  = 3915;
//from classical eval end
constexpr Value RandomValue = 1200;  //handicap mode Michael Byrne
// clang-format off
enum PieceType : std::uint8_t {
    NO_PIECE_TYPE, PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING,
    ALL_PIECES = 0,
    PIECE_TYPE_NB = 8
};

enum Piece : std::uint8_t {
    NO_PIECE,
    W_PAWN = PAWN,     W_KNIGHT, W_BISHOP, W_ROOK, W_QUEEN, W_KING,
    B_PAWN = PAWN + 8, B_KNIGHT, B_BISHOP, B_ROOK, B_QUEEN, B_KING,
    PIECE_NB = 16
};
// clang-format on
//for classical begin
constexpr Value PieceValueForPSQT[PHASE_NB][PIECE_NB] = {
  {VALUE_ZERO, PawnValueMg, KnightValueMg, BishopValueMg, RookValueMg, QueenValueMg, VALUE_ZERO,
   VALUE_ZERO, VALUE_ZERO, PawnValueMg, KnightValueMg, BishopValueMg, RookValueMg, QueenValueMg,
   VALUE_ZERO, VALUE_ZERO},
  {VALUE_ZERO, PawnValueEg, KnightValueEg, BishopValueEg, RookValueEg, QueenValueEg, VALUE_ZERO,
   VALUE_ZERO, VALUE_ZERO, PawnValueEg, KnightValueEg, BishopValueEg, RookValueEg, QueenValueEg,
   VALUE_ZERO, VALUE_ZERO}};
//for classical end
constexpr Value PieceValue[PIECE_NB] = {
  VALUE_ZERO, PawnValue, KnightValue, BishopValue, RookValue, QueenValue, VALUE_ZERO, VALUE_ZERO,
  VALUE_ZERO, PawnValue, KnightValue, BishopValue, RookValue, QueenValue, VALUE_ZERO, VALUE_ZERO};

using Depth = int;

// The following DEPTH_ constants are used for transposition table entries
// and quiescence search move generation stages. In regular search, the
// depth stored in the transposition table is literal: the search depth
// (effort) used to make the corresponding transposition table value. In
// quiescence search, however, the transposition table entries only store
// the current quiescence move generation stage (which should thus compare
// lower than any regular search depth).
constexpr Depth DEPTH_QS = 0;
// For transposition table entries where no searching at all was done
// (whether regular or qsearch) we use DEPTH_UNSEARCHED, which should thus
// compare lower than any quiescence or regular depth. DEPTH_ENTRY_OFFSET
// is used only for the transposition table entry occupancy check (see tt.cpp),
// and should thus be lower than DEPTH_UNSEARCHED.
constexpr Depth DEPTH_UNSEARCHED   = -2;
constexpr Depth DEPTH_ENTRY_OFFSET = -3;

// clang-format off
enum Square : uint8_t {
    SQ_A1, SQ_B1, SQ_C1, SQ_D1, SQ_E1, SQ_F1, SQ_G1, SQ_H1,
    SQ_A2, SQ_B2, SQ_C2, SQ_D2, SQ_E2, SQ_F2, SQ_G2, SQ_H2,
    SQ_A3, SQ_B3, SQ_C3, SQ_D3, SQ_E3, SQ_F3, SQ_G3, SQ_H3,
    SQ_A4, SQ_B4, SQ_C4, SQ_D4, SQ_E4, SQ_F4, SQ_G4, SQ_H4,
    SQ_A5, SQ_B5, SQ_C5, SQ_D5, SQ_E5, SQ_F5, SQ_G5, SQ_H5,
    SQ_A6, SQ_B6, SQ_C6, SQ_D6, SQ_E6, SQ_F6, SQ_G6, SQ_H6,
    SQ_A7, SQ_B7, SQ_C7, SQ_D7, SQ_E7, SQ_F7, SQ_G7, SQ_H7,
    SQ_A8, SQ_B8, SQ_C8, SQ_D8, SQ_E8, SQ_F8, SQ_G8, SQ_H8,
    SQ_NONE,

    SQUARE_ZERO = 0,
    SQUARE_NB   = 64
};
// clang-format on

enum Direction : int8_t {
    NORTH = 8,
    EAST  = 1,
    SOUTH = -NORTH,
    WEST  = -EAST,

    NORTH_EAST = NORTH + EAST,
    SOUTH_EAST = SOUTH + EAST,
    SOUTH_WEST = SOUTH + WEST,
    NORTH_WEST = NORTH + WEST
};

enum File : uint8_t {
    FILE_A,
    FILE_B,
    FILE_C,
    FILE_D,
    FILE_E,
    FILE_F,
    FILE_G,
    FILE_H,
    FILE_NB
};

enum Rank : uint8_t {
    RANK_1,
    RANK_2,
    RANK_3,
    RANK_4,
    RANK_5,
    RANK_6,
    RANK_7,
    RANK_8,
    RANK_NB
};

//for classical begin
/// ScoreForClassical enum stores a middlegame and an endgame value in a single integer (enum).
/// The least significant 16 bits are used to store the middlegame value and the
/// upper 16 bits are used to store the endgame value. We have to take care to
/// avoid left-shifting a signed int to avoid undefined behavior.
enum ScoreForClassical : int {
    SCORE_ZERO
};

constexpr ScoreForClassical make_score(int mg, int eg) {
    return ScoreForClassical((int) ((unsigned int) eg << 16) + mg);
}

/// Extracting the signed lower and upper 16 bits is not so trivial because
/// according to the standard a simple cast to short is implementation defined
/// and so is a right shift of a signed integer.
inline Value eg_value(ScoreForClassical s) {
    union {
        uint16_t u;
        int16_t  s;
    } eg = {uint16_t(unsigned(s + 0x8000) >> 16)};
    return Value(eg.s);
}

inline Value mg_value(ScoreForClassical s) {
    union {
        uint16_t u;
        int16_t  s;
    } mg = {uint16_t(unsigned(s))};
    return Value(mg.s);
}

    #define ENABLE_BASE_OPERATORS_ON(T) \
        constexpr T operator+(T d1, int d2) { return T(int(d1) + d2); } \
        constexpr T operator-(T d1, int d2) { return T(int(d1) - d2); } \
        constexpr T operator-(T d) { return T(-int(d)); } \
        inline T&   operator+=(T& d1, int d2) { return d1 = d1 + d2; } \
        inline T&   operator-=(T& d1, int d2) { return d1 = d1 - d2; }
//for classical end
    #define ENABLE_INCR_OPERATORS_ON(T) \
        constexpr T& operator++(T& d) { return d = T(int(d) + 1); } \
        constexpr T& operator--(T& d) { return d = T(int(d) - 1); }
//for classical begin
    #define ENABLE_FULL_OPERATORS_ON(T) \
        ENABLE_BASE_OPERATORS_ON(T) \
        constexpr T   operator*(int i, T d) { return T(i * int(d)); } \
        constexpr T   operator*(T d, int i) { return T(int(d) * i); } \
        constexpr T   operator/(T d, int i) { return T(int(d) / i); } \
        constexpr int operator/(T d1, T d2) { return int(d1) / int(d2); } \
        inline T&     operator*=(T& d, int i) { return d = T(int(d) * i); } \
        inline T&     operator/=(T& d, int i) { return d = T(int(d) / i); }
ENABLE_FULL_OPERATORS_ON(Direction)

ENABLE_INCR_OPERATORS_ON(Piece)
//for classical end
ENABLE_INCR_OPERATORS_ON(PieceType)
ENABLE_INCR_OPERATORS_ON(Square)
ENABLE_INCR_OPERATORS_ON(File)
ENABLE_INCR_OPERATORS_ON(Rank)
//for classical begin
ENABLE_BASE_OPERATORS_ON(ScoreForClassical)

    #undef ENABLE_FULL_OPERATORS_ON
//for classical end
    #undef ENABLE_INCR_OPERATORS_ON
    #undef ENABLE_BASE_OPERATORS_ON  //for classical

//omitted for classical eval

// Additional operators to add a Direction to a Square
constexpr Square  operator+(Square s, Direction d) { return Square(int(s) + int(d)); }
constexpr Square  operator-(Square s, Direction d) { return Square(int(s) - int(d)); }
constexpr Square& operator+=(Square& s, Direction d) { return s = s + d; }
constexpr Square& operator-=(Square& s, Direction d) { return s = s - d; }
//for classical begin
/// Only declared but not defined. We don't want to multiply two scores due to
/// a very high risk of overflow. So user should explicitly convert to integer.
ScoreForClassical operator*(ScoreForClassical, ScoreForClassical) = delete;

/// Division of a Score must be handled separately for each term
inline ScoreForClassical operator/(ScoreForClassical s, int i) {
    return make_score(mg_value(s) / i, eg_value(s) / i);
}

/// Multiplication of a Score by an integer. We check for overflow in debug mode.
inline ScoreForClassical operator*(ScoreForClassical s, int i) {

    ScoreForClassical result = ScoreForClassical(int(s) * i);

    assert(eg_value(result) == (i * eg_value(s)));
    assert(mg_value(result) == (i * mg_value(s)));
    assert((i == 0) || (result / i) == s);

    return result;
}

/// Multiplication of a Score by a boolean
inline ScoreForClassical operator*(ScoreForClassical s, bool b) { return b ? s : SCORE_ZERO; }
//for classical end

// Toggle color
constexpr Color operator~(Color c) { return Color(c ^ BLACK); }

// Swap A1 <-> A8
constexpr Square flip_rank(Square s) { return Square(s ^ SQ_A8); }

// Swap A1 <-> H1
constexpr Square flip_file(Square s) { return Square(s ^ SQ_H1); }

// Swap color of piece B_KNIGHT <-> W_KNIGHT
constexpr Piece operator~(Piece pc) { return Piece(pc ^ 8); }

constexpr CastlingRights operator&(Color c, CastlingRights cr) {
    return CastlingRights((c == WHITE ? WHITE_CASTLING : BLACK_CASTLING) & cr);
}

constexpr Value mate_in(int ply) { return VALUE_MATE - ply; }

constexpr Value mated_in(int ply) { return -VALUE_MATE + ply; }

constexpr Square make_square(File f, Rank r) { return Square((r << 3) + f); }

constexpr Piece make_piece(Color c, PieceType pt) { return Piece((c << 3) + pt); }

constexpr PieceType type_of(Piece pc) { return PieceType(pc & 7); }

constexpr Color color_of(Piece pc) {
    assert(pc != NO_PIECE);
    return Color(pc >> 3);
}

constexpr bool is_ok(Square s) { return s >= SQ_A1 && s <= SQ_H8; }

constexpr File file_of(Square s) { return File(s & 7); }

constexpr Rank rank_of(Square s) { return Rank(s >> 3); }

constexpr Square relative_square(Color c, Square s) { return Square(s ^ (c * 56)); }

constexpr Rank relative_rank(Color c, Rank r) { return Rank(r ^ (c * 7)); }

constexpr Rank relative_rank(Color c, Square s) { return relative_rank(c, rank_of(s)); }

constexpr Direction pawn_push(Color c) { return c == WHITE ? NORTH : SOUTH; }


// Based on a congruential pseudo-random number generator
constexpr Key make_key(uint64_t seed) {
    return seed * 6364136223846793005ULL + 1442695040888963407ULL;
}


enum MoveType : uint16_t {
    NORMAL,
    PROMOTION  = 1 << 14,
    EN_PASSANT = 2 << 14,
    CASTLING   = 3 << 14
};

// A move needs 16 bits to be stored
//
// bit  0- 5: destination square (from 0 to 63)
// bit  6-11: origin square (from 0 to 63)
// bit 12-13: promotion piece type - 2 (from KNIGHT-2 to QUEEN-2)
// bit 14-15: special move flag: promotion (1), en passant (2), castling (3)
// NOTE: en passant bit is set only when a pawn can be captured
//
// Special cases are Move::none() and Move::null(). We can sneak these in because
// in any normal move the destination square and origin square are always different,
// but Move::none() and Move::null() have the same origin and destination square.

class Move {
   public:
    Move() = default;
    constexpr explicit Move(std::uint16_t d) :
        data(d) {}

    constexpr Move(Square from, Square to) :
        data((from << 6) + to) {}

    template<MoveType T>
    static constexpr Move make(Square from, Square to, PieceType pt = KNIGHT) {
        return Move(T + ((pt - KNIGHT) << 12) + (from << 6) + to);
    }

    constexpr Square from_sq() const {
        assert(is_ok());
        return Square((data >> 6) & 0x3F);
    }

    constexpr Square to_sq() const {
        assert(is_ok());
        return Square(data & 0x3F);
    }

    constexpr MoveType type_of() const { return MoveType(data & (3 << 14)); }

    constexpr PieceType promotion_type() const { return PieceType(((data >> 12) & 3) + KNIGHT); }

    constexpr bool is_ok() const { return none().data != data && null().data != data; }

    static constexpr Move null() { return Move(65); }
    static constexpr Move none() { return Move(0); }

    constexpr bool operator==(const Move& m) const { return data == m.data; }
    constexpr bool operator!=(const Move& m) const { return data != m.data; }

    constexpr explicit operator bool() const { return data != 0; }

    constexpr std::uint16_t raw() const { return data; }

    struct MoveHash {
        std::size_t operator()(const Move& m) const { return make_key(m.data); }
    };

   protected:
    std::uint16_t data;
};

template<typename T, typename... Ts>
struct is_all_same {
    static constexpr bool value = (std::is_same_v<T, Ts> && ...);
};

template<typename... Ts>
constexpr auto is_all_same_v = is_all_same<Ts...>::value;
}  // namespace Alexander

#endif  // #ifndef TYPES_H_INCLUDED

#include "tune.h"  // Global visibility to tuning setup


// ============================================================================
// FILE ORIGINALE: uci.cpp
// ============================================================================

/*
  Alexander, a UCI chess playing engine derived from Stockfish
  Copyright (C) 2004-2026 Alexander developers (see AUTHORS file)

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

#include "uci.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <iterator>
#include <optional>
#include <sstream>
#include <string_view>
#include <utility>
#include <vector>

#include "benchmark.h"
#include "engine.h"
#include "memory.h"
#include "movegen.h"
#include "position.h"
#include "score.h"
#include "search.h"
#include "types.h"
#include "ucioption.h"
//From Alexander begin

#include "learn/learn.h"
#include "book/book.h"
#include "mcts/montecarlo.h"
//From Alexander end
namespace Alexander {
constexpr auto BenchmarkCommand = "speedtest";

constexpr auto StartFEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
template<typename... Ts>
struct overload: Ts... {
    using Ts::operator()...;
};

template<typename... Ts>
overload(Ts...) -> overload<Ts...>;

void UCIEngine::print_info_string(std::string_view str) {
    sync_cout_start();
    for (auto& line : split(str, "\n"))
    {
        if (!is_whitespace(line))
        {
            std::cout << "info string " << line << '\n';
        }
    }
    sync_cout_end();
}

UCIEngine::UCIEngine(int argc, char** argv) :
    engine(argv[0]),
    cli(argc, argv) {

    engine.get_options().add_info_listener([](const std::optional<std::string>& str) {
        if (str.has_value())
            print_info_string(*str);
    });

    init_search_update_listeners();
}

void UCIEngine::init_search_update_listeners() {
    engine.set_on_iter([](const auto& i) { on_iter(i); });
    engine.set_on_update_no_moves([](const auto& i) { on_update_no_moves(i); });
    engine.set_on_update_full(
      [this](const auto& i) { on_update_full(i, engine.get_options()["UCI_ShowWDL"]); });
    engine.set_on_bestmove([](const auto& bm, const auto& p) { on_bestmove(bm, p); });
    //from classical
}

void UCIEngine::loop() {
    //learning begin
    Position     pos;
    StateListPtr states(new std::deque<StateInfo>(1));
    pos.set(StartFEN, false, &states->back(), nullptr);  //for classical
    //learning end
    std::string token, cmd;

    for (int i = 1; i < cli.argc; ++i)
        cmd += std::string(cli.argv[i]) + " ";

    do
    {
        if (cli.argc == 1
            && !getline(std::cin, cmd))  // Wait for an input or an end-of-file (EOF) indication
            cmd = "quit";

        std::istringstream is(cmd);

        token.clear();  // Avoid a stale if getline() returns nothing or a blank line
        is >> std::skipws >> token;

        if (token == "quit" || token == "stop")
        {  //learning
            engine.stop();

            //learning begin
            if (token == "quit" && LD.is_enabled() && !LD.is_paused())
            {
                //Wait for the current search operation (if any) to stop
                //before proceeding to save experience data
                engine.wait_for_search_finished();

                //Perform Q-learning if enabled
                if (LD.learning_mode() == LearningMode::Self)
                {
                    putQLearningTrajectoryIntoLearningTable();
                }
                if (!LD.is_readonly())
                {
                    //Save to learning file
                    LD.persist(engine.get_options());
                }
            }
            //learning end
        }
        // The GUI sends 'ponderhit' to tell that the user has played the expected move.
        // So, 'ponderhit' is sent if pondering was done on the same move that the user
        // has played. The search should continue, but should also switch from pondering
        // to the normal search.
        else if (token == "ponderhit")
            engine.set_ponderhit(false);

        else if (token == "uci")
        {
            sync_cout << "id name " << engine_info(true) << "\n"
                      << engine.get_options() << sync_endl;

            sync_cout << "uciok" << sync_endl;
        }

        else if (token == "setoption")
            setoption(is);
        else if (token == "go")
        {
            // send info strings after the go command is sent for old GUIs and python-chess
            print_info_string(engine.numa_config_information_as_string());
            print_info_string(engine.thread_allocation_information_as_string());
            go(is);
        }
        else if (token == "position")
        {  //Experience Book
            position(is);
            pos.set(engine.fen(), engine.get_options()["UCI_Chess960"], &states->back(), nullptr);
        }  //Experience Book
        else if (token == "ucinewgame")
        //learning begin
        {
            if (LD.is_enabled())
            {
                //Perform Q-learning if enabled
                if (LD.learning_mode() == LearningMode::Self)
                {
                    putQLearningTrajectoryIntoLearningTable();
                }

                if (!LD.is_readonly())
                {
                    //Save to learning file
                    LD.persist(engine.get_options());
                }
                setStartPoint();
            }
            engine.search_clear();
        }
        //learning end
        else if (token == "isready")
            sync_cout << "readyok" << sync_endl;

        // Add custom non-UCI commands, mainly for debugging purposes.
        // These commands must not be used during a search!
        else if (token == "flip")
            engine.flip();
        else if (token == "bench")
            bench(is);
        else if (token == BenchmarkCommand)
            benchmark(is);
        else if (token == "d")
            sync_cout << engine.visualize() << sync_endl;
        else if (token == "eval")
            engine.trace_eval();
        //book and exp begin
        else if (token == "book")
            engine.show_moves_bookMan(pos);
        else if (token == "showexp")
            LD.show_exp(pos);
        else if (token == "quickresetexp")
            LD.quick_reset_exp();
        //book and exp end
        else if (token == "compiler")
            sync_cout << compiler_info() << sync_endl;
        else if (token == "--help" || token == "help" || token == "--license" || token == "license")
            sync_cout
              << "\nAlexander is a powerful chess engine for playing and analyzing."
                 "\nIt is released as free software licensed under the GNU GPLv3 License."
                 "\nAlexander is normally used with a graphical user interface (GUI) and implements"
                 "\nthe Universal Chess Interface (UCI) protocol to communicate with a GUI, an API, etc."
                 "\nFor any further information, visit https://github.com/official-stockfish/Stockfish#readme"
                 "\nor read the corresponding README.md and Copying.txt files distributed along with this program.\n"
              << sync_endl;
        else if (!token.empty() && token[0] != '#')
            sync_cout << "Unknown command: '" << cmd << "'. Type help for more information."
                      << sync_endl;

    } while (token != "quit" && cli.argc == 1);  // The command-line arguments are one-shot
}

// handicapMode begin
inline int getHandicapDepth(int elo) {
    if (elo <= 1350)
    {
        return (int) (3 * elo / 1350 + 1);
    }
    if (elo <= 1999)
    {
        return (int) ((2 * elo - 104) / 649);
    }
    if (elo <= 2199)
    {
        return (int) ((2 * elo - 2607) / 199);
    }
    if (elo <= 2399)
    {
        return (int) ((2 * elo - 2410) / 199);
    }
    return (int) ((7 * elo - 10950) / 450);
}
// handicapMode end

Search::LimitsType UCIEngine::parse_limits(std::istream& is) {
    Search::LimitsType limits;
    std::string        token;

    limits.startTime = now();  // The search starts as early as possible
    //from true handicap mode begin
    if (Eval::handicapConfig.limitStrength && Eval::handicapConfig.handicappedDepth)
    {
        limits.depth = getHandicapDepth(Eval::handicapConfig.uciElo);
    }
    //from true handicap mode end
    while (is >> token)
        if (token == "searchmoves")  // Needs to be the last command on the line
            while (is >> token)
                limits.searchmoves.push_back(to_lower(token));

        else if (token == "wtime")
            is >> limits.time[WHITE];
        else if (token == "btime")
            is >> limits.time[BLACK];
        else if (token == "winc")
            is >> limits.inc[WHITE];
        else if (token == "binc")
            is >> limits.inc[BLACK];
        else if (token == "movestogo")
            is >> limits.movestogo;
        else if (token == "depth")
            is >> limits.depth;
        else if (token == "nodes")
            is >> limits.nodes;
        else if (token == "movetime")
            is >> limits.movetime;
        else if (token == "mate")
            is >> limits.mate;
        else if (token == "perft")
            is >> limits.perft;
        else if (token == "infinite")
            limits.infinite = 1;
        else if (token == "ponder")
            limits.ponderMode = true;

    return limits;
}

void UCIEngine::go(std::istringstream& is) {

    Search::LimitsType limits = parse_limits(is);

    if (limits.perft)
        perft(limits, nullptr);  //from classical
    else
        engine.go(limits);
}

void UCIEngine::bench(std::istream& args) {
    std::string token;
    uint64_t    num, nodes = 0, cnt = 1;
    uint64_t    nodesSearched = 0;
    const auto& options       = engine.get_options();

    engine.set_on_update_full([&](const auto& i) {
        nodesSearched = i.nodes;
        on_update_full(i, options["UCI_ShowWDL"]);
    });

    std::vector<std::string> list = Benchmark::setup_bench(engine.fen(), args);

    num = count_if(list.begin(), list.end(),
                   [](const std::string& s) { return s.find("go ") == 0 || s.find("eval") == 0; });

    TimePoint elapsed = now();

    for (const auto& cmd : list)
    {
        std::istringstream is(cmd);
        is >> std::skipws >> token;

        if (token == "go" || token == "eval")
        {
            std::cerr << "\nPosition: " << cnt++ << '/' << num << " (" << engine.fen() << ")"
                      << std::endl;
            if (token == "go")
            {
                Search::LimitsType limits = parse_limits(is);

                if (limits.perft)
                    nodesSearched = perft(limits, nullptr);  //from classical
                else
                {
                    engine.go(limits);
                    engine.wait_for_search_finished();
                }

                nodes += nodesSearched;
                nodesSearched = 0;
            }
            else
                engine.trace_eval();
        }
        else if (token == "setoption")
            setoption(is);
        else if (token == "position")
            position(is);
        else if (token == "ucinewgame")
        {
            //learning begin
            if (LD.is_enabled())
            {
                if (LD.learning_mode() == LearningMode::Self && !LD.is_paused())
                {
                    putQLearningTrajectoryIntoLearningTable();
                }
                setStartPoint();
            }
            //learning end
            engine.search_clear();  // search_clear may take a while
            elapsed = now();
        }
    }

    elapsed = now() - elapsed + 1;  // Ensure positivity to avoid a 'divide by zero'

    dbg_print();

    std::cerr << "\n==========================="    //
              << "\nTotal time (ms) : " << elapsed  //
              << "\nNodes searched  : " << nodes    //
              << "\nNodes/second    : " << 1000 * nodes / elapsed << std::endl;

    // reset callback, to not capture a dangling reference to nodesSearched
    engine.set_on_update_full([&](const auto& i) { on_update_full(i, options["UCI_ShowWDL"]); });
}

void UCIEngine::benchmark(std::istream& args) {
    // Probably not very important for a test this long, but include for completeness and sanity.
    static constexpr int NUM_WARMUP_POSITIONS = 3;

    std::string token;
    uint64_t    nodes = 0, cnt = 1;
    uint64_t    nodesSearched = 0;

    engine.set_on_update_full([&](const Engine::InfoFull& i) { nodesSearched = i.nodes; });

    engine.set_on_iter([](const auto&) {});
    engine.set_on_update_no_moves([](const auto&) {});
    engine.set_on_bestmove([](const auto&, const auto&) {});

    Benchmark::BenchmarkSetup setup = Benchmark::setup_benchmark(args);

    const auto numGoCommands = count_if(setup.commands.begin(), setup.commands.end(),
                                        [](const std::string& s) { return s.find("go ") == 0; });

    TimePoint totalTime = 0;

    // Set options once at the start.
    auto ss = std::istringstream("name Threads value " + std::to_string(setup.threads));
    setoption(ss);
    ss = std::istringstream("name Hash value " + std::to_string(setup.ttSize));
    setoption(ss);
    ss = std::istringstream("name UCI_Chess960 value false");
    setoption(ss);

    // Warmup
    for (const auto& cmd : setup.commands)
    {
        std::istringstream is(cmd);
        is >> std::skipws >> token;

        if (token == "go")
        {
            // One new line is produced by the search, so omit it here
            std::cerr << "\rWarmup position " << cnt++ << '/' << NUM_WARMUP_POSITIONS;

            Search::LimitsType limits = parse_limits(is);

            // Run with silenced network verification
            engine.go(limits);
            engine.wait_for_search_finished();
        }
        else if (token == "position")
            position(is);
        else if (token == "ucinewgame")
        {
            engine.search_clear();  // search_clear may take a while
        }

        if (cnt > NUM_WARMUP_POSITIONS)
            break;
    }

    std::cerr << "\n";

    cnt   = 1;
    nodes = 0;

    int           numHashfullReadings = 0;
    constexpr int hashfullAges[]      = {0, 999};  // Only normal hashfull and touched hash.
    constexpr int hashfullAgeCount    = std::size(hashfullAges);
    int           totalHashfull[hashfullAgeCount] = {0};
    int           maxHashfull[hashfullAgeCount]   = {0};

    auto updateHashfullReadings = [&]() {
        numHashfullReadings += 1;

        for (int i = 0; i < hashfullAgeCount; ++i)
        {
            const int hashfull = engine.get_hashfull(hashfullAges[i]);
            maxHashfull[i]     = std::max(maxHashfull[i], hashfull);
            totalHashfull[i] += hashfull;
        }
    };

    engine.search_clear();  // search_clear may take a while

    for (const auto& cmd : setup.commands)
    {
        std::istringstream is(cmd);
        is >> std::skipws >> token;

        if (token == "go")
        {
            // One new line is produced by the search, so omit it here
            std::cerr << "\rPosition " << cnt++ << '/' << numGoCommands;

            Search::LimitsType limits = parse_limits(is);

            nodesSearched     = 0;
            TimePoint elapsed = now();

            // Run with silenced network verification
            engine.go(limits);
            engine.wait_for_search_finished();

            totalTime += now() - elapsed;

            updateHashfullReadings();

            nodes += nodesSearched;
        }
        else if (token == "position")
            position(is);
        else if (token == "ucinewgame")
        {
            engine.search_clear();  // search_clear may take a while
        }
    }

    totalTime = std::max<TimePoint>(totalTime, 1);  // Ensure positivity to avoid a 'divide by zero'

    dbg_print();

    std::cerr << "\n";

    static_assert(
      std::size(hashfullAges) == 2 && hashfullAges[0] == 0 && hashfullAges[1] == 999,
      "Hardcoded for display. Would complicate the code needlessly in the current state.");

    std::string threadBinding = engine.thread_binding_information_as_string();
    if (threadBinding.empty())
        threadBinding = "none";

    // clang-format off

    std::cerr << "==========================="
              << "\nVersion                    : "
              << engine_version_info()
              // "\nCompiled by                : "
              << compiler_info()
              << "Large pages                : " << (has_large_pages() ? "yes" : "no")
              << "\nUser invocation            : " << BenchmarkCommand << " "
              << setup.originalInvocation << "\nFilled invocation          : " << BenchmarkCommand
              << " " << setup.filledInvocation
              << "\nAvailable processors       : " << engine.get_numa_config_as_string()
              << "\nThread count               : " << setup.threads
              << "\nThread binding             : " << threadBinding
              << "\nTT size [MiB]              : " << setup.ttSize
              << "\nHash max, avg [per mille]  : "
              << "\n    single search          : " << maxHashfull[0] << ", "
              << totalHashfull[0] / numHashfullReadings
              << "\n    single game            : " << maxHashfull[1] << ", "
              << totalHashfull[1] / numHashfullReadings
              << "\nTotal nodes searched       : " << nodes
              << "\nTotal search time [s]      : " << totalTime / 1000.0
              << "\nNodes/second               : " << 1000 * nodes / totalTime << std::endl;

    // clang-format on

    init_search_update_listeners();
}

void UCIEngine::setoption(std::istringstream& is) {
    engine.wait_for_search_finished();
    engine.get_options().setoption(is);
}

std::uint64_t UCIEngine::perft(const Search::LimitsType& limits, Thread* th) {  //for classical
    auto nodes = engine.perft(engine.fen(), limits.perft, engine.get_options()["UCI_Chess960"], th);
    sync_cout << "\nNodes searched: " << nodes << "\n" << sync_endl;
    return nodes;
}

void UCIEngine::position(std::istringstream& is) {
    std::string token, fen;

    is >> token;

    if (token == "startpos")
    {
        fen = StartFEN;
        is >> token;  // Consume the "moves" token, if any
    }
    else if (token == "fen")
        while (is >> token && token != "moves")
            fen += token + " ";
    else
        return;

    std::vector<std::string> moves;

    while (is >> token)
    {
        moves.push_back(token);
    }

    engine.set_position(fen, moves);
}

std::string UCIEngine::format_score(const Score& s) {
    constexpr int TB_CP = 20000;
    const auto    format =
      overload{[](Score::Mate mate) -> std::string {
                   auto m = (mate.plies > 0 ? (mate.plies + 1) : mate.plies) / 2;
                   return std::string("mate ") + std::to_string(m);
               },
               [](Score::Tablebase tb) -> std::string {
                   return std::string("cp ")
                        + std::to_string((tb.win ? TB_CP - tb.plies : -TB_CP - tb.plies));
               },
               [](Score::InternalUnits units) -> std::string {
                   return std::string("cp ") + std::to_string(units.value);
               }};

    return s.visit(format);
}
//from learning begin
int UCIEngine::getNormalizeToPawnValue(Position& pos) {
    auto [a, b] = WDLModel::win_rate_params(pos);
    return a;
}
//from learning end

// Turns a Value to an integer centipawn number,
// without treatment of mate and similar special scores.
int UCIEngine::to_cp(Value v, const Position& pos) {

    // In general, the score can be defined via the WDL as
    // (log(1/L - 1) - log(1/W - 1)) / (log(1/L - 1) + log(1/W - 1)).
    // Based on our win_rate_model, this simply yields v / a.

    auto [a, b] = WDLModel::win_rate_params(pos);

    return int(std::round(100 * int(v) / a));
}

//in wdl package for wdl model
std::string UCIEngine::square(Square s) {
    return std::string{char('a' + file_of(s)), char('1' + rank_of(s))};
}

std::string UCIEngine::move(Move m, bool chess960) {
    if (m == Move::none())
        return "(none)";

    if (m == Move::null())
        return "0000";

    Square from = m.from_sq();
    Square to   = m.to_sq();

    if (m.type_of() == CASTLING && !chess960)
        to = make_square(to > from ? FILE_G : FILE_C, rank_of(from));

    std::string move = square(from) + square(to);

    if (m.type_of() == PROMOTION)
        move += " pnbrqk"[m.promotion_type()];

    return move;
}


std::string UCIEngine::to_lower(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), [](auto c) { return std::tolower(c); });

    return str;
}

Move UCIEngine::to_move(const Position& pos, std::string str) {
    str = to_lower(str);

    for (const auto& m : MoveList<LEGAL>(pos))
        if (str == move(m, pos.is_chess960()))
            return m;

    return Move::none();
}

void UCIEngine::on_update_no_moves(const Engine::InfoShort& info) {
    sync_cout << "info depth " << info.depth << " score " << format_score(info.score) << sync_endl;
}

void UCIEngine::on_update_full(const Engine::InfoFull& info, bool showWDL) {
    std::stringstream ss;

    ss << "info";
    ss << " depth " << info.depth                 //
       << " seldepth " << info.selDepth           //
       << " multipv " << info.multiPV             //
       << " score " << format_score(info.score);  //

    if (!info.bound.empty())
        ss << " " << info.bound;

    if (showWDL)
        ss << " wdl " << info.wdl;

    ss << " nodes " << info.nodes        //
       << " nps " << info.nps            //
       << " hashfull " << info.hashfull  //
       << " tbhits " << info.tbHits      //
       << " time " << info.timeMs        //
       << " pv " << info.pv;             //

    sync_cout << ss.str() << sync_endl;
}

void UCIEngine::on_iter(const Engine::InfoIter& info) {
    std::stringstream ss;

    ss << "info";
    ss << " depth " << info.depth                     //
       << " currmove " << info.currmove               //
       << " currmovenumber " << info.currmovenumber;  //

    sync_cout << ss.str() << sync_endl;
}

void UCIEngine::on_bestmove(std::string_view bestmove, std::string_view ponder) {
    sync_cout << "bestmove " << bestmove;
    if (!ponder.empty())
        std::cout << " ponder " << ponder;
    std::cout << sync_endl;
}

}  // namespace Alexander


// ============================================================================
// FILE ORIGINALE: uci.h
// ============================================================================

/*
  Alexander, a UCI chess playing engine derived from Stockfish
  Copyright (C) 2004-2026 Alexander developers (see AUTHORS file)

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

#ifndef UCI_H_INCLUDED
#define UCI_H_INCLUDED

#include <cstdint>
#include <iostream>
#include <string>
#include <string_view>

#include "engine.h"
#include "misc.h"
#include "search.h"
#include "book/book_manager.h"  //book management
#include "wdl/win_probability.h"
namespace Alexander {

class Position;
class Move;
class Score;
enum Square : uint8_t;
using Value = int;

class UCIEngine {
   public:
    UCIEngine(int argc, char** argv);

    void loop();

    static int         to_cp(Value v, const Position& pos);
    static int         getNormalizeToPawnValue(Position& pos);  //learning
    static std::string format_score(const Score& s);
    static std::string square(Square s);
    static std::string move(Move m, bool chess960);
    //from shashin and learning
    static std::string to_lower(std::string str);
    static Move        to_move(const Position& pos, std::string str);

    static Search::LimitsType parse_limits(std::istream& is);

    auto& engine_options() { return engine.get_options(); }

   private:
    Engine      engine;
    CommandLine cli;

    static void print_info_string(std::string_view str);

    void          go(std::istringstream& is);
    void          bench(std::istream& args);
    void          benchmark(std::istream& args);
    void          position(std::istringstream& is);
    void          setoption(std::istringstream& is);
    std::uint64_t perft(const Search::LimitsType& limits, Thread* th);  //for classical

    static void on_update_no_moves(const Engine::InfoShort& info);
    static void on_update_full(const Engine::InfoFull& info, bool showWDL);
    static void on_iter(const Engine::InfoIter& info);
    static void on_bestmove(std::string_view bestmove, std::string_view ponder);

    void init_search_update_listeners();
};

}  // namespace Alexander

#endif  // #ifndef UCI_H_INCLUDED


// ============================================================================
// FILE ORIGINALE: ucioption.cpp
// ============================================================================

/*
  Alexander, a UCI chess playing engine derived from Stockfish
  Copyright (C) 2004-2026 Alexander developers (see AUTHORS file)

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

#include "ucioption.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <utility>

#include "misc.h"
#include <set>  //combo management

namespace Alexander {

bool CaseInsensitiveLess::operator()(const std::string& s1, const std::string& s2) const {

    return std::lexicographical_compare(
      s1.begin(), s1.end(), s2.begin(), s2.end(),
      [](char c1, char c2) { return std::tolower(c1) < std::tolower(c2); });
}

void OptionsMap::add_info_listener(InfoListener&& message_func) { info = std::move(message_func); }

void OptionsMap::setoption(std::istringstream& is) {
    std::string token, name, value;

    is >> token;  // Consume the "name" token

    // Read the option name (can contain spaces)
    while (is >> token && token != "value")
        name += (name.empty() ? "" : " ") + token;

    // Read the option value (can contain spaces)
    while (is >> token)
        value += (value.empty() ? "" : " ") + token;

    if (options_map.count(name))
        options_map[name] = value;
    else
        sync_cout << "No such option: " << name << sync_endl;
}

const Option& OptionsMap::operator[](const std::string& name) const {
    auto it = options_map.find(name);
    assert(it != options_map.end());
    return it->second;
}

// Inits options and assigns idx in the correct printing order
void OptionsMap::add(const std::string& name, const Option& option) {
    //combo management begin
    auto it = options_map.find(name);

    if (it == options_map.end())  // Se l'opzione NON esiste ancora, la aggiungiamo
    {
        static size_t insert_order = 0;
        options_map[name]          = option;
        options_map[name].parent   = this;
        options_map[name].idx      = insert_order++;
    }
    else
    {
        const Option& existingOption = it->second;
        if (existingOption.type == "combo" && option.type == "combo")
        {
            if (existingOption.defaultValue == option.defaultValue)
            {
                return;
            }
            else
            {
                options_map[name] = option;
                return;
            }
        }

        std::cerr << "Option \"" << name << "\" was already added!" << std::endl;
        std::exit(EXIT_FAILURE);
    }
    //combo management end
}


std::size_t OptionsMap::count(const std::string& name) const { return options_map.count(name); }

Option::Option(const OptionsMap* map) :
    parent(map) {}

Option::Option(const char* v, OnChange f) :
    type("string"),
    min(0),
    max(0),
    on_change(std::move(f)) {
    defaultValue = currentValue = v;
}

Option::Option(bool v, OnChange f) :
    type("check"),
    min(0),
    max(0),
    on_change(std::move(f)) {
    defaultValue = currentValue = (v ? "true" : "false");
}

Option::Option(OnChange f) :
    type("button"),
    min(0),
    max(0),
    on_change(std::move(f)) {}

Option::Option(int v, int minv, int maxv, OnChange f) :
    type("spin"),
    min(minv),
    max(maxv),
    on_change(std::move(f)) {
    defaultValue = currentValue = std::to_string(v);
}

Option::Option(const char* v, const char* cur, OnChange f) :
    type("combo"),
    min(0),
    max(0),
    on_change(std::move(f)) {
    defaultValue = v;
    currentValue = cur;
}

Option::operator int() const {
    assert(type == "check" || type == "spin");
    return (type == "spin" ? std::stoi(currentValue) : currentValue == "true");
}

Option::operator std::string() const {
    assert(type == "string" || type == "combo");  //combo
    return currentValue;
}

bool Option::operator==(const char* s) const {
    assert(type == "combo");
    return !CaseInsensitiveLess()(currentValue, s) && !CaseInsensitiveLess()(s, currentValue);
}

bool Option::operator!=(const char* s) const { return !(*this == s); }


// Updates currentValue and triggers on_change() action. It's up to
// the GUI to check for option's limits, but we could receive the new value
// from the user by console window, so let's check the bounds anyway.
Option& Option::operator=(const std::string& v) {

    assert(!type.empty());
    //combo management begin
    if ((type != "button" && type != "string" && v.empty())
        || (type == "check" && v != "true" && v != "false")
        || (type == "spin" && (std::stoi(v) < min || std::stoi(v) > max)))
    {
        return *this;
    }

    if (type == "combo")
    {
        std::set<std::string, CaseInsensitiveLess> comboSet;
        std::istringstream                         ss(defaultValue);
        std::string                                token;

        while (ss >> token)
            if (token != "var")
                comboSet.insert(token);
        if (!comboSet.count(v))
            return *this;
        //combo management end
    }

    if (type == "string")
        currentValue = v == "<empty>" ? "" : v;
    else if (type != "button")
        currentValue = v;

    if (on_change)
    {
        const auto ret = on_change(*this);

        if (ret && parent != nullptr && parent->info != nullptr)
            parent->info(ret);
    }

    return *this;
}

std::ostream& operator<<(std::ostream& os, const OptionsMap& om) {
    for (size_t idx = 0; idx < om.options_map.size(); ++idx)
        for (const auto& it : om.options_map)
            if (it.second.idx == idx)
            {
                const Option& o = it.second;
                os << "\noption name " << it.first << " type " << o.type;

                if (o.type == "check" || o.type == "combo")
                    os << " default " << o.defaultValue;

                else if (o.type == "string")
                {
                    std::string defaultValue = o.defaultValue.empty() ? "<empty>" : o.defaultValue;
                    os << " default " << defaultValue;
                }

                else if (o.type == "spin")
                    os << " default " << stoi(o.defaultValue) << " min " << o.min << " max "
                       << o.max;

                break;
            }

    return os;
}
}


// ============================================================================
// FILE ORIGINALE: ucioption.h
// ============================================================================

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

#ifndef UCIOPTION_H_INCLUDED
#define UCIOPTION_H_INCLUDED

#include <cstddef>
#include <functional>
#include <iosfwd>
#include <map>
#include <optional>
#include <string>

namespace Alexander {
// Define a custom comparator, because the UCI options should be case-insensitive
struct CaseInsensitiveLess {
    bool operator()(const std::string&, const std::string&) const;
};

class OptionsMap;

// The Option class implements each option as specified by the UCI protocol
class Option {
   public:
    using OnChange = std::function<std::optional<std::string>(const Option&)>;

    Option(const OptionsMap*);
    Option(OnChange = nullptr);
    Option(bool v, OnChange = nullptr);
    Option(const char* v, OnChange = nullptr);
    Option(int v, int minv, int maxv, OnChange = nullptr);
    Option(const char* v, const char* cur, OnChange = nullptr);

    Option& operator=(const std::string&);
            operator int() const;
            operator std::string() const;
    bool    operator==(const char*) const;
    bool    operator!=(const char*) const;

    friend std::ostream& operator<<(std::ostream&, const OptionsMap&);

    int operator<<(const Option&) = delete;

   private:
    friend class OptionsMap;
    friend class Engine;
    friend class Tune;


    std::string       defaultValue, currentValue, type;
    int               min, max;
    size_t            idx;
    OnChange          on_change;
    const OptionsMap* parent = nullptr;
};

class OptionsMap {
   public:
    using InfoListener = std::function<void(std::optional<std::string>)>;

    OptionsMap()                             = default;
    OptionsMap(const OptionsMap&)            = delete;
    OptionsMap(OptionsMap&&)                 = delete;
    OptionsMap& operator=(const OptionsMap&) = delete;
    OptionsMap& operator=(OptionsMap&&)      = delete;

    void add_info_listener(InfoListener&&);

    void setoption(std::istringstream&);

    const Option& operator[](const std::string&) const;

    void add(const std::string&, const Option& option);

    std::size_t count(const std::string&) const;

   private:
    friend class Engine;
    friend class Option;

    friend std::ostream& operator<<(std::ostream&, const OptionsMap&);

    // The options container is defined as a std::map
    using OptionsStore = std::map<std::string, Option, CaseInsensitiveLess>;

    OptionsStore options_map;
    InfoListener info;
};

}
#endif  // #ifndef UCIOPTION_H_INCLUDED


