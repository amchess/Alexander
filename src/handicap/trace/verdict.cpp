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

// [SC:trace] The Master's Verdict of Alexander.
//
// It is composed only from structured facts (the counterfactual facts of plans.cpp, the term
// table of the classical evaluation and the position itself), never from the text of the
// technical sections, so that every combination of cases (checkmate, stalemate, check, forced
// mate, tactics, threats, material imbalances, pawn endings, ...) produces coherent sentences.
// Its sections are those of the ShashChess verdict, so that the two traces can be compared;
// "EVALUATION FACTORS" is specific to the classical evaluation.

#include "verdict.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

#include "../../evaluate.h"
#include "../../movegen.h"
#include "../../uci.h"
#include "../../wdl/win_probability.h"
#include "trace.h"

namespace Alexander {

using namespace Attacks;

namespace Eval {

namespace {

constexpr Value MATE_SCORE = 20000;  // same as plans.cpp

bool is_mate(Value v) { return std::abs(v) >= MATE_SCORE - 2 * MAX_PLY; }

std::string color_name(Color c) { return c == WHITE ? "White" : "Black"; }

const char* piece_word(PieceType pt) {
    static const char* N[] = {"", "pawn", "knight", "bishop", "rook", "queen", "king"};
    return N[pt];
}

std::string piece_on(Piece pc, Square s) {
    return std::string("the ") + piece_word(type_of(pc)) + " on " + UCIEngine::square(s);
}

std::string capitalize(std::string s) {
    if (!s.empty())
        s[0] = char(std::toupper(s[0]));
    return s;
}

std::string join_words(const std::vector<std::string>& v, const std::string& last = " and ") {
    std::string s;
    for (size_t i = 0; i < v.size(); ++i)
        s += (i == 0 ? "" : i + 1 == v.size() ? last : ", ") + v[i];
    return s;
}

std::string join_all(const std::vector<std::string>& v, const std::string& sep) {
    std::string s;
    for (size_t i = 0; i < v.size(); ++i)
        s += (i ? sep : "") + v[i];
    return s;
}

Bitboard pawn_attacks_of(Color c, Bitboard b) {
    return c == WHITE ? pawn_attacks_bb<WHITE>(b) : pawn_attacks_bb<BLACK>(b);
}

bool light_square(Square s) { return (int(file_of(s)) + int(rank_of(s))) % 2 != 0; }

// Standard algebraic notation of a legal move (Nf3, exd5, O-O, e8=Q+, Qxf7#)
std::string san(Position& pos, Move m) {
    if (!m)
        return "-";
    const Square    from = m.from_sq(), to = m.to_sq();
    const PieceType pt   = type_of(pos.piece_on(from));
    std::string     s;
    if (m.type_of() == CASTLING)
        s = to > from ? "O-O" : "O-O-O";
    else
    {
        const bool capture = pos.capture(m);
        if (pt == PAWN)
        {
            if (capture)
                s += char('a' + file_of(from));
        }
        else
        {
            s += "PNBRQK"[pt - 1];
            bool sameFile = false, sameRank = false, ambiguous = false;
            for (const auto& o : MoveList<LEGAL>(pos))
                if (o != m && o.to_sq() == to && o.from_sq() != from
                    && type_of(pos.piece_on(o.from_sq())) == pt)
                {
                    ambiguous = true;
                    sameFile |= file_of(o.from_sq()) == file_of(from);
                    sameRank |= rank_of(o.from_sq()) == rank_of(from);
                }
            if (ambiguous)
            {
                if (!sameFile)
                    s += char('a' + file_of(from));
                else if (!sameRank)
                    s += char('1' + rank_of(from));
                else
                    s += UCIEngine::square(from);
            }
        }
        if (capture)
            s += 'x';
        s += UCIEngine::square(to);
        if (m.type_of() == PROMOTION)
            s += std::string("=") + "PNBRQK"[m.promotion_type() - 1];
    }
    if (pos.gives_check(m))
    {
        StateInfo st;
        pos.do_move(m, st);
        const bool mate = MoveList<LEGAL>(pos).size() == 0;
        pos.undo_move(m);
        s += mate ? '#' : '+';
    }
    return s;
}

// SAN of a move of the opponent, played after a pass of the side to move
std::string san_after_null(Position& pos, Move m) {
    if (!m || pos.checkers())
        return m ? UCIEngine::move(m, pos.is_chess960()) : "-";
    StateInfo st;
    pos.do_null_move(st);
    std::string s = san(pos, m);
    pos.undo_null_move();
    return s;
}

// Signed pawns with two decimals (cp / 100, UCI scale): "+0.35"
std::string pawns_str(const Position& pos, Value v) {
    std::stringstream ss;
    ss << std::showpos << std::fixed << std::setprecision(2) << 0.01 * UCIEngine::to_cp(v, pos);
    return ss.str();
}

std::string cp_str(int cp) {
    std::stringstream ss;
    ss << std::showpos << cp << std::noshowpos << " cp";
    return ss.str();
}

// Material balance in words, from the side to move's point of view
std::string material_sentence(const Position& pos, Color us) {
    const Color them = ~us;
    int         d[PIECE_TYPE_NB]{};
    for (PieceType pt : {PAWN, KNIGHT, BISHOP, ROOK, QUEEN})
        d[pt] = popcount(pos.pieces(us, pt)) - popcount(pos.pieces(them, pt));
    const int points = d[PAWN] + 3 * (d[KNIGHT] + d[BISHOP]) + 5 * d[ROOK] + 9 * d[QUEEN];
    auto      describe = [&](int sign) {
        std::vector<std::string> v;
        for (PieceType pt : {QUEEN, ROOK, BISHOP, KNIGHT, PAWN})
        {
            const int n = sign * d[pt];
            if (n == 1)
                v.push_back(std::string("a ") + piece_word(pt));
            else if (n > 1)
                v.push_back(std::to_string(n) + " " + piece_word(pt) + "s");
        }
        return join_words(v);
    };
    auto pawnsWord = [](int n) {
        return n == 1 ? std::string("a pawn") : std::to_string(n) + " pawns";
    };
    const std::string ours = describe(+1), theirs = describe(-1);
    if (ours.empty() && theirs.empty())
        return "Material is level.";
    if (d[ROOK] == 1 && d[KNIGHT] + d[BISHOP] == -1 && !d[QUEEN] && std::abs(d[PAWN]) <= 2)
        return "You are the exchange up"
             + (d[PAWN] < 0   ? " for " + pawnsWord(-d[PAWN])
                : d[PAWN] > 0 ? " and " + pawnsWord(d[PAWN]) + " up"
                              : std::string())
             + ".";
    if (d[ROOK] == -1 && d[KNIGHT] + d[BISHOP] == 1 && !d[QUEEN] && std::abs(d[PAWN]) <= 2)
        return "You are the exchange down"
             + (d[PAWN] > 0   ? ", with " + pawnsWord(d[PAWN]) + " as compensation"
                : d[PAWN] < 0 ? " and " + pawnsWord(-d[PAWN]) + " down"
                              : std::string())
             + ".";
    const bool onlyPawns = !d[KNIGHT] && !d[BISHOP] && !d[ROOK] && !d[QUEEN];
    if (theirs.empty())
        return "You are " + ours + " up"
             + (onlyPawns ? std::string() : " (" + std::to_string(points) + " in classical points)") + ".";
    if (ours.empty())
        return "You are " + theirs + " down"
             + (onlyPawns ? std::string() : " (" + std::to_string(-points) + " in classical points)") + ".";
    return "You have " + ours + " for " + theirs + " ("
         + (points > 0   ? "+" + std::to_string(points)
            : points < 0 ? std::to_string(points)
                         : "even")
         + " in classical points): a material imbalance.";
}

// Net value of a term for 'c' (phase interpolated), cp
int term_cp(const Position& pos, int term, Color c, int phase) {
    const int v = Trace::Helpers::interpolate(Trace::scores[term][WHITE] - Trace::scores[term][BLACK],
                                              phase);
    return UCIEngine::to_cp(c == WHITE ? v : -v, pos);
}

// Natural language effect of a term change (mover's point of view)
std::string term_effect(int term, bool positive) {
    switch (term)
    {
    case Trace::MATERIAL :
        return positive ? "improves the placement of the pieces" : "a worse piece placement";
    case Trace::IMBALANCE :
        return positive ? "improves the piece configuration" : "a worse piece configuration";
    case PAWN :
        return positive ? "improves the pawn structure" : "a weaker pawn structure";
    case KNIGHT :
        return positive ? "improves the knights" : "less effective knights";
    case BISHOP :
        return positive ? "improves the bishops" : "less effective bishops";
    case ROOK :
        return positive ? "improves the rooks" : "less effective rooks";
    case QUEEN :
        return positive ? "improves the queen" : "a less effective queen";
    case Trace::MOBILITY :
        return positive ? "increases the mobility of the pieces" : "less mobility";
    case KING :
        return positive ? "makes your king safer" : "a less safe king";
    case Trace::THREAT :
        return positive ? "creates threats" : "fewer threats";
    case Trace::PASSED :
        return positive ? "strengthens the passed pawns" : "weaker passed pawns";
    case Trace::SPACE :
        return positive ? "gains space" : "less space";
    default :
        return positive ? "improves the position" : "a small concession";
    }
}

// Value of a candidate for the verdict: the two-ply value when available
Value cand_value(const PlanCandidate& c) { return c.q2 != VALUE_NONE ? c.q2 : c.q; }

// "it increases the mobility of the pieces and makes your king safer (at the cost of ...)"
std::string explanation_sentence(Position& pos, const PlanCandidate& c) {
    if (c.m.type_of() == CASTLING)
        return "it brings the king to safety and connects the rooks";
    std::vector<std::string> pros, cons;
    if (c.m.type_of() == PROMOTION)
        pros.push_back(std::string("promotes the pawn to a ") + piece_word(c.m.promotion_type()));
    else if (c.capture && pos.capture(c.m) && pos.piece_on(c.m.to_sq()) != NO_PIECE)
        pros.push_back("captures " + piece_on(pos.piece_on(c.m.to_sq()), c.m.to_sq()));
    if (c.check)
        pros.push_back("gives check");
    for (const auto& [d, term] : c.termDeltas)
    {
        if (std::abs(d) < 4)
            continue;
        if (d > 0 && pros.size() < 3)
            pros.push_back(term_effect(term, true));
        else if (d < 0 && cons.size() < 2)
            cons.push_back(term_effect(term, false));
    }
    std::string s = pros.empty() ? (c.kingMove ? std::string("it improves the king's position")
                                               : std::string("it makes a small all-round "
                                                             "improvement"))
                                 : "it " + join_words(pros);
    if (!cons.empty())
        s += " (at the cost of " + join_words(cons) + ")";
    return s;
}

}  // namespace

std::string compose_verdict(Position&          pos,
                            const PlanFacts&   f,
                            Value              v_white,
                            uint8_t            winProb,
                            const std::string& shashinZone,
                            const std::string& gamePhase,
                            int                phase) {
    std::stringstream ss;
    const Color       us       = pos.side_to_move();
    const Color       them     = ~us;
    const std::string Us       = color_name(us);
    const std::string Them     = color_name(them);
    const bool        noPieces = !(pos.pieces() & ~pos.pieces(PAWN, KING));
    const bool        endgame  = phase < 48 || !pos.pieces(QUEEN);
    const bool        kingPhase =
      noPieces || (endgame && !pos.pieces(QUEEN) && popcount(pos.pieces() & ~pos.pieces(PAWN, KING)) <= 4);
    auto cp = [&](Value v) { return UCIEngine::to_cp(v, pos); };

    ss << "================================================================\n";
    ss << ">>> COMPLETE HUMAN EVALUATION (The Master's Verdict) <<<\n";
    ss << "Seen from " << Us << "'s side: " << Us << " is to move.\n\n";

    // ---- Terminal positions ------------------------------------------------------------------
    if (f.cands.empty())
    {
        if (pos.checkers())
            ss << "CHECKMATE: " << Us << " is mated, " << Them << " has won the game.\n";
        else
            ss << "STALEMATE: " << Us << " has no legal move and is not in check: the game is "
                  "drawn.\n";
        ss << "================================================================\n";
        return ss.str();
    }

    const PlanCandidate& best    = f.cands.front();
    const bool           mateFor = is_mate(best.q) && best.q > 0;
    const bool           mated   = is_mate(best.q) && best.q < 0;
    const std::string    bestSan = san(pos, best.m);

    if (mateFor || mated)
    {
        ss << "OVERALL EVALUATION:\n"
           << (mateFor ? Us + " has a forced mate (" + bestSan + "): the game is decided."
                       : Us + " cannot avoid mate: the position is lost.")
           << "\n\nTHE PLAN:\n  1. "
           << (mateFor ? "Deliver mate: " + bestSan + "." : "Prolong the resistance with " + bestSan + ".")
           << "\n\nIN A NUTSHELL:\n"
           << (mateFor ? Us + " mates with " + bestSan + "." : Us + " is getting mated.") << "\n";
        ss << "================================================================\n";
        return ss.str();
    }

    // Assessed value: after an immediate winning tactic the quiet value is the real one
    const bool  tacticalWin  = f.tactical && f.quietUs > f.staticUs && f.rootTactic && !f.inCheck;
    // In check, or under a threat, the static evaluation counts the threat as if it were already
    // executed: the value after the best reply is the one to assess.
    const Value bestV        = cand_value(best);
    const bool  afterReply   = !tacticalWin && (f.inCheck || f.threat) && !is_mate(bestV);
    const Value vUs          = us == WHITE ? v_white : -v_white;
    const Value vAssess      = tacticalWin ? f.quietUs : afterReply ? bestV : vUs;
    const Value vAssessWhite = us == WHITE ? vAssess : -vAssess;
    const int   wpA          = WDLModel::get_win_probability(vAssessWhite, pos);  // White
    const WDLModel::WDL wdl  = WDLModel::get_wdl(vAssessWhite, pos);
    const int   wpUs         = us == WHITE ? wpA : 100 - wpA;
    // Same bands as the ShashChess verdict, on the same UCI cp scale, so that the two traces
    // can be compared directly
    const int   cpA          = cp(vAssess);  // side to move's point of view
    const int   aA           = std::abs(cpA);
    const std::string leader = cpA >= 0 ? Us : Them;
    // The UCI cp scale is calibrated on the NNUE: a classical evaluation of a decisive advantage
    // can stay below 400 cp, so a decisive expected score also upgrades the band.
    const int   wpLead       = cpA >= 0 ? wpUs : 100 - wpUs;

    // Dynamic (piece activity) and structural components of the classical evaluation
    const int dynamicCp = std::abs(term_cp(pos, Trace::MOBILITY, WHITE, phase))
                        + std::abs(term_cp(pos, KING, WHITE, phase))
                        + std::abs(term_cp(pos, Trace::THREAT, WHITE, phase));
    const int structCp = std::abs(term_cp(pos, PAWN, WHITE, phase))
                       + std::abs(term_cp(pos, Trace::PASSED, WHITE, phase))
                       + std::abs(term_cp(pos, Trace::SPACE, WHITE, phase));
    const int sharpness = std::abs(term_cp(pos, KING, WHITE, phase))
                        + std::abs(term_cp(pos, Trace::THREAT, WHITE, phase));
    const bool sharp = f.tactical || f.threat || sharpness >= 30;
    const bool quiet = !sharp && sharpness < 10;

    // ---- Overall evaluation ------------------------------------------------------------------
    ss << "OVERALL EVALUATION:\n";
    {
        if (tacticalWin)
            ss << "The static evaluation (" << pawns_str(pos, v_white)
               << " for White) does not tell the whole story: after "
               << (f.rootTactic.type_of() == PROMOTION ? "the promotion " : "the tactic ")
               << san(pos, f.rootTactic) << ", ";
        else if (afterReply && std::abs(cp(bestV) - cp(vUs)) >= 25)
            ss << "The static evaluation (" << pawns_str(pos, v_white) << " for White) counts the "
               << (f.inCheck ? "check" : "threat") << " as already executed: after the best reply "
               << bestSan << ", ";
        const bool lower = tacticalWin || (afterReply && std::abs(cp(bestV) - cp(vUs)) >= 25);
        if (aA < 20)
            ss << (lower ? "the" : "The") << " position is about equal (evaluation "
               << pawns_str(pos, vAssessWhite) << " from White's point of view, expected score "
               << wpA << "% for White). ";
        else
            ss << leader
               << (aA >= 400 || wpLead >= 95 ? " is winning"
                   : aA >= 200                 ? " has a large advantage"
                   : aA >= 100                 ? " is clearly better"
                   : aA >= 50                  ? " is better"
                                               : " is slightly better")
               << " (evaluation " << pawns_str(pos, vAssessWhite)
               << " from White's point of view, expected score " << wpLead << "% for " << leader
               << "). ";
        ss << material_sentence(pos, us) << " ";
        if (noPieces)
            ss << "It is a pure king and pawn ending: count the tempi and use the opposition. ";
        else if (phase < 24)
            ss << "In this endgame technique and king activity matter most. ";
        else
            ss << (sharp   ? "The position is sharp: concrete calculation matters more than "
                             "general principles. "
                   : quiet ? "The position is quiet and suited to maneuvering: long-term factors "
                             "decide. "
                           : "The position mixes tactics and strategy: plan, but check every move "
                             "concretely. ");
        if (dynamicCp >= 5 || structCp >= 5)
            ss << "The evaluation rests mainly on "
               << (dynamicCp >= structCp ? "the activity of the pieces (mobility, king safety, "
                                           "threats)"
                                         : "the pawn structure (pawns, passed pawns, space)")
               << " (activity terms " << dynamicCp << " cp, structural terms " << structCp
               << " cp, absolute values). ";
        // The classical phase (non-pawn material) is reliable from any FEN; the game stage of
        // get_game_phase() also depends on the move number, so it is quoted only with a history.
        ss << "Phase: "
           << (noPieces       ? "king and pawn ending"
               : phase >= 110 ? "opening or early middlegame (almost all the pieces are on)"
               : phase >= 64  ? "middlegame"
               : phase >= 24  ? "late middlegame, close to the endgame"
                              : "endgame")
           << " (material phase " << phase << "/128"
           << (pos.game_ply() > 0 ? "; game stage: " + gamePhase : std::string()) << "). ";
        if (wdl.draw >= 70 && aA < 100 && !lower)
            ss << "A draw is the most likely result (" << int(wdl.draw)
               << "%): to play for a win, someone must take calculated risks. ";
        if (wpUs >= 85)
            ss << "With accurate play the advantage should be decisive: convert it without "
                  "allowing counterplay.";
        else if (wpUs <= 15)
            ss << "The position is very difficult: look for complications, active counterplay or "
                  "a fortress.";
        else if (noPieces)
            ss << "In a pawn ending every tempo counts: activate the king and calculate the races "
                  "to the end.";
        else
            ss << "Shashin's advice: "
               << (wpUs <= 25   ? "defend solidly and apply strict prophylaxis."
                   : wpUs <= 45 ? "absorb the pressure while keeping a solid, elastic position."
                   : wpUs <= 54 ? "apply classical positional principles, improve the "
                                  "coordination of the pieces and accumulate small advantages."
                   : wpUs <= 74 ? "keep the initiative and force the opponent to create "
                                  "permanent weaknesses."
                                : "calculate forcing lines and convert the advantage.");
        ss << " (Shashin zone, White's view: " << shashinZone << ".)";
        (void) winProb;
        ss << "\n\n";
    }

    // ---- Evaluation factors (classical evaluation only) ---------------------------------------
    ss << "EVALUATION FACTORS:\n";
    {
        struct T {
            int         term;
            const char* name;
        };
        const T terms[] = {{Trace::IMBALANCE, "the piece configuration (imbalance)"},
                           {PAWN, "the pawn structure"},
                           {KNIGHT, "the knights"},
                           {BISHOP, "the bishops"},
                           {ROOK, "the rooks"},
                           {QUEEN, "the queens"},
                           {Trace::MOBILITY, "mobility"},
                           {KING, "king safety"},
                           {Trace::THREAT, "threats"},
                           {Trace::PASSED, "passed pawns"},
                           {Trace::SPACE, "space"}};
        std::vector<std::pair<int, std::string>> fac;
        for (const auto& t : terms)
        {
            const int v = term_cp(pos, t.term, us, phase);
            if (std::abs(v) >= 3)
                fac.emplace_back(v, t.name);
        }
        std::sort(fac.begin(), fac.end(),
                  [](auto& a, auto& b) { return std::abs(a.first) > std::abs(b.first); });
        std::vector<std::string> pro, contra;
        for (size_t i = 0; i < fac.size() && i < 5; ++i)
            (fac[i].first > 0 ? pro : contra)
              .push_back(fac[i].second + " (" + cp_str(std::abs(fac[i].first)) + ")");
        if (fac.empty())
            ss << "No evaluation term clearly favours either side. ";
        else
        {
            if (!pro.empty())
                ss << "In your favour: " << join_words(pro) << ". ";
            if (!contra.empty())
                ss << "In " << Them << "'s favour: " << join_words(contra) << ". ";
        }
        // Middlegame vs endgame prospects: unweighted sums of the MG and EG halves
        ScoreForClassical tot = SCORE_ZERO;
        for (int term : {int(Trace::MATERIAL), int(Trace::IMBALANCE), int(PAWN), int(KNIGHT),
                         int(BISHOP), int(ROOK), int(QUEEN), int(Trace::MOBILITY), int(KING),
                         int(Trace::THREAT), int(Trace::PASSED), int(Trace::SPACE)})
            tot += Trace::scores[term][WHITE] - Trace::scores[term][BLACK];
        const int sign = us == WHITE ? 1 : -1;
        const int mg = cp(sign * mg_value(tot)), eg = cp(sign * eg_value(tot));
        if (mg > eg + 15)
            ss << "Your prospects are better in the middlegame (" << cp_str(mg) << ") than in the "
               << "endgame (" << cp_str(eg) << "): keep the pieces on and play for the initiative. ";
        else if (eg > mg + 15)
            ss << "Your prospects improve in the endgame (" << cp_str(eg) << " against "
               << cp_str(mg) << " in the middlegame): simplifications favour you. ";
        else
            ss << "Middlegame and endgame prospects are similar (" << cp_str(mg) << " / "
               << cp_str(eg) << "). ";
        // Endgame scaling (winnable term)
        const int win = cp(eg_value(Trace::scores[Trace::WINNABLE][WHITE]));
        const int egW = cp(eg_value(tot));
        if (std::abs(win) >= 5 && (win > 0) != (egW > 0) && egW != 0)
            ss << "The engine scales the endgame advantage down (" << cp_str(win)
               << " for White): drawish features such as few pawns, symmetry or opposite-coloured "
                  "bishops.";
        ss << "\n\n";
    }

    // ---- Urgent matters ----------------------------------------------------------------------
    ss << "URGENT MATTERS:\n";
    {
        std::vector<std::string> u;
        if (f.inCheck)
            u.push_back("You are in check: the only question is how to answer it. The best reply "
                        "is " + bestSan + ".");
        else if (tacticalWin)
            u.push_back("There is a tactic to exploit: " + san(pos, f.rootTactic)
                        + " wins material (about " + pawns_str(pos, f.quietUs - f.staticUs)
                        + ").");
        if (f.threat)
            u.push_back(Them + " threatens " + san_after_null(pos, f.threatMove)
                        + (f.threatMate ? ", with mate"
                                        : ", which would cost about " + std::to_string(f.threatCp)
                                            + " cp")
                        + ": deal with it first.");
        if (best.sacrifice && !f.inCheck)
            u.push_back("The engine's favourite " + bestSan
                        + " is a sacrifice (it loses material by exchange count, but the "
                          "resulting position is judged good): calculate it carefully.");
        if (u.empty())
            u.push_back("Nothing urgent: neither side has an immediate tactic, so there is time to "
                        "improve the position.");
        ss << join_all(u, " ") << "\n\n";
    }

    // ---- Pieces --------------------------------------------------------------------------------
    // Best / worst piece: efficiency of the paired ablation value (100% = as good as the
    // equivalent enemy pieces); single ablation for the pieces without counterpart
    const PlanPiece *ub = nullptr, *uw = nullptr, *tb = nullptr, *tw = nullptr;
    for (const auto& p : f.pieces)
    {
        if (type_of(p.pc) == PAWN)
            continue;
        const bool mine = color_of(p.pc) == us;
        auto&      b    = mine ? ub : tb;
        auto&      w    = mine ? uw : tw;
        if (!b || p.efficiency > b->efficiency)
            b = &p;
        if (!w || p.efficiency < w->efficiency)
            w = &p;
    }
    const PlanReroute* uwRoute = nullptr;
    if (uw)
        for (const auto& r : f.reroutes[us])
            if (r.from == uw->sq)
            {
                uwRoute = &r;
                break;
            }

    ss << "PIECE EVALUATION:\n";
    {
        if (noPieces)
            ss << "Only kings and pawns are left: the king is now a fighting piece, and the race "
                  "of the passed pawns (or the opposition) decides. ";
        else
        {
            // "efficiency 107%, +0.20 pawns compared with the equivalent enemy pieces"
            auto effText = [&](const PlanPiece& p) {
                std::stringstream e;
                e << "efficiency " << p.efficiency << "%";
                if (p.counterparts)
                    e << ", " << cp_str(p.cp) << " compared with the equivalent enemy pieces";
                else
                    e << " of its normal value, no equivalent enemy piece";
                return e.str();
            };
            if (ub && uw && ub != uw)
                ss << "Your most effective piece is " << piece_on(ub->pc, ub->sq) << " ("
                   << effText(*ub) << "); your least effective is " << piece_on(uw->pc, uw->sq)
                   << " (" << effText(*uw)
                   << "): improving it is the first positional task (Makogonov's principle). ";
            else if (ub)
                ss << "Your only piece, " << piece_on(ub->pc, ub->sq)
                   << ", must be kept as active as possible. ";
            else
                ss << "You have no pieces left besides the king and pawns. ";
            if (tb && tw && tb != tw)
                ss << "On the other side, " << piece_on(tw->pc, tw->sq) << " is " << Them
                   << "'s problem piece, while " << piece_on(tb->pc, tb->sq)
                   << " is its most effective unit. ";
            for (const auto& p : f.pieces)
                if (type_of(p.pc) != PAWN && p.singleCp < 0)
                    ss << (color_of(p.pc) == us
                             ? "The evaluation would even prefer you without " + piece_on(p.pc, p.sq)
                                 + ": it is badly placed, tied down or in the way. "
                             : "Likewise " + piece_on(p.pc, p.sq) + " is a burden for " + Them + ". ");
            // Piece-specific remarks (classical patterns)
            for (Bitboard b = pos.pieces(us, BISHOP); b;)
            {
                const Square s    = pop_lsb(b);
                int          same = 0;
                for (Bitboard pw = pos.pieces(us, PAWN); pw;)
                    same += light_square(pop_lsb(pw)) == light_square(s);
                const PlanPiece* pi = nullptr;
                for (const auto& p : f.pieces)
                    if (p.sq == s)
                        pi = &p;
                if (same >= 4 && pi && pi->efficiency < 100)
                    ss << "Your bishop on " << UCIEngine::square(s)
                       << " is a bad bishop, hemmed in by its own pawns. ";
            }
            for (Bitboard b = pos.pieces(us, KNIGHT); b;)
            {
                const Square s = pop_lsb(b);
                const int    r = relative_rank(us, s);
                if (r >= RANK_4 && r <= RANK_6 && (pawn_attacks_of(us, pos.pieces(us, PAWN)) & s)
                    && !(pos.pieces(them, PAWN) & pawn_attack_span(us, s)))
                    ss << "Your knight on " << UCIEngine::square(s) << " sits on a strong outpost. ";
            }
            for (Bitboard b = pos.pieces(us, ROOK); b;)
            {
                const Square s = pop_lsb(b);
                if (!(pos.pieces(PAWN) & file_bb(s)))
                    ss << "Your rook on " << UCIEngine::square(s) << " controls an open file. ";
                else if (relative_rank(us, s) == RANK_7)
                    ss << "Your rook on " << UCIEngine::square(s) << " is on the seventh rank. ";
            }
        }
        const int ubish = popcount(pos.pieces(us, BISHOP)), tbish = popcount(pos.pieces(them, BISHOP));
        if (ubish >= 2 && tbish < 2)
            ss << "You have the bishop pair: open the position to make it count. ";
        else if (tbish >= 2 && ubish < 2)
            ss << Them << " has the bishop pair: keep the position closed and trade a bishop if "
                          "possible. ";
        if (ubish == 1 && tbish == 1
            && light_square(lsb(pos.pieces(us, BISHOP))) != light_square(lsb(pos.pieces(them, BISHOP))))
            ss << "The bishops are of opposite colours: this favours the attacker while other "
                  "pieces remain, but makes the endgame drawish. ";
        ss << "\n\n";
    }

    // ---- Trades --------------------------------------------------------------------------------
    ss << "TRADES AND SIMPLIFICATION:\n";
    if (noPieces)
        ss << "No pieces are left to trade: every pawn exchange changes the nature of the ending, "
              "so calculate them precisely.\n\n";
    else
    {
        const int sign = us == WHITE ? 1 : -1;
        if (!pos.pieces(QUEEN))
            ss << "The queens are off: king activity and the pawn structure gain weight. ";
        else if (f.hasQueenTrade)
        {
            const int q = sign * f.queenTrade;
            ss << (q > 5    ? "A queen trade favours you (about " + cp_str(q) + "): welcome it. "
                   : q < -5 ? "Avoid the queen trade: it would cost about " + cp_str(-q).substr(1) + ". "
                            : std::string("A queen trade would not change the balance much. "));
        }
        else
            ss << "Only one side has a queen. ";
        if (f.hasRookTrade)
        {
            const int r = sign * f.rookTrade;
            ss << (r > 5    ? "Trading all the rooks would help you. "
                   : r < -5 ? "Keep at least one pair of rooks: trading them all helps " + Them + ". "
                            : std::string());
        }
        // Trades from the side to move's point of view
        const PlanTrade *good = nullptr, *bad = nullptr;
        for (const auto& t : f.trades)
        {
            const int v = sign * t.cpWhite;
            if (v > 5 && (!good || v > sign * good->cpWhite))
                good = &t;
            if (v < -5 && (!bad || v < sign * bad->cpWhite))
                bad = &t;
        }
        auto tradeText = [&](const PlanTrade& t) {
            const Piece  mine   = us == WHITE ? t.a : t.b;
            const Square mineSq = us == WHITE ? t.sa : t.sb;
            const Piece  other  = us == WHITE ? t.b : t.a;
            const Square othSq  = us == WHITE ? t.sb : t.sa;
            return "your " + std::string(piece_word(type_of(mine))) + " on " + UCIEngine::square(mineSq)
                 + " for " + piece_on(other, othSq);
        };
        if (good)
            ss << "The most useful exchange is " << tradeText(*good) << " ("
               << cp_str(sign * good->cpWhite) << "). ";
        if (bad)
            ss << "Do not give " << tradeText(*bad) << " (" << cp_str(sign * bad->cpWhite) << "). ";
        int points = 0;
        for (PieceType pt : {PAWN, KNIGHT, BISHOP, ROOK, QUEEN})
            points += (pt == PAWN ? 1 : pt == ROOK ? 5 : pt == QUEEN ? 9 : 3)
                    * (popcount(pos.pieces(us, pt)) - popcount(pos.pieces(them, pt)));
        const int space = term_cp(pos, Trace::SPACE, us, phase);
        if (points >= 2)
            ss << "Being ahead in material, trade pieces (not pawns) to approach a won endgame.";
        else if (points <= -2)
            ss << "Being behind in material, avoid trades and keep pieces for counterplay.";
        else if (space <= -5)
            ss << "Being cramped, piece trades will relieve the pressure.";
        else if (space >= 5)
            ss << "With more space, avoid unnecessary trades: they would free the opponent.";
        ss << "\n\n";
    }

    // ---- Geometry and structure ----------------------------------------------------------------
    ss << "GEOMETRY AND STRUCTURE:\n";
    {
        // Pawn structure
        auto analyse = [&](Color c, std::vector<std::string>& passed,
                           std::vector<std::string>& weak) {
            const Bitboard own = pos.pieces(c, PAWN), enemy = pos.pieces(~c, PAWN);
            for (Bitboard b = own; b;)
            {
                const Square s = pop_lsb(b);
                if (!(enemy & passed_pawn_span(c, s)))
                {
                    passed.push_back(UCIEngine::square(s));
                    continue;
                }
                if (!(own & adjacent_files_bb(s)))
                    weak.push_back("isolated " + UCIEngine::square(s));
                else if (!(own & adjacent_files_bb(s) & ~forward_ranks_bb(c, s))
                         && !(enemy & forward_file_bb(c, s))
                         && (pawn_attacks_of(~c, enemy) & (s + pawn_push(c))))
                    weak.push_back("backward " + UCIEngine::square(s));
            }
            for (File fl = FILE_A; fl <= FILE_H; ++fl)
                if (popcount(own & file_bb(fl)) >= 2)
                    weak.push_back("doubled on the " + std::string(1, char('a' + fl)) + "-file");
        };
        std::vector<std::string> passedUs, weakUs, passedThem, weakThem;
        analyse(us, passedUs, weakUs);
        analyse(them, passedThem, weakThem);

        std::vector<std::string> p;
        if (!passedUs.empty())
            p.push_back("your passed pawn on " + join_words(passedUs)
                        + " is an asset: support its advance"
                        + (endgame ? " (in the endgame it is often decisive)" : ""));
        if (!passedThem.empty())
            p.push_back(Them + "'s passed pawn on " + join_words(passedThem)
                        + " must be blockaded, ideally by a knight");
        if (!weakUs.empty())
            p.push_back("defend your weak pawns (" + join_words(weakUs) + ") or trade them off");
        if (!weakThem.empty())
            p.push_back("target the enemy weaknesses (" + join_words(weakThem)
                        + "), blockading and piling up on them");
        if (p.empty())
            p.push_back("both structures are healthy, without passed or weak pawns");
        // Center type
        const Bitboard center = FileDBB | FileEBB;
        const int      cw = popcount(pos.pieces(WHITE, PAWN) & center),
                  cb = popcount(pos.pieces(BLACK, PAWN) & center);
        const Bitboard rams = pos.pieces(WHITE, PAWN) & shift<SOUTH>(pos.pieces(BLACK, PAWN)) & center;
        if (!pos.pieces(PAWN) || noPieces || phase < 24)
            ;
        else if (cw + cb == 0)
            p.push_back("the center is open: piece activity and the bishops count most");
        else if (popcount(rams) >= 2)
            p.push_back("the center is closed: play on the wings, where your pawn chain points");
        else if (cw + cb <= 2 && (cw == 0 || cb == 0))
            p.push_back("the center is half-open: the side with the central pawn must use it for "
                        "space and piece activity");
        const bool keepTension = f.tension && !f.releaseTension && f.tensionCp <= -10
                              && best.m.from_sq() != f.tensionMove.from_sq()
                              && !(best.capture && type_of(pos.piece_on(best.m.from_sq())) == PAWN);
        if (f.tension)
            p.push_back(f.releaseTension ? "resolving the pawn tension with " + san(pos, f.tensionMove)
                                             + " is fine"
                        : !keepTension ? "there is no hurry to resolve the pawn tension with "
                                           + san(pos, f.tensionMove)
                                       : "keep the pawn tension: capturing first with "
                                           + san(pos, f.tensionMove) + " would help the opponent");
        if (!f.goodBreaks.empty())
            p.push_back("the pawn break " + san(pos, f.goodBreaks.front()) + " is playable now");
        else if (!f.badBreaks.empty())
            p.push_back("the break " + san(pos, f.badBreaks.front()) + " is premature: prepare it first"
                        + (!f.shelterBad.empty() && f.shelterBad.front() == f.badBreaks.front()
                             ? " (it would also weaken your king)"
                             : ""));
        if (!f.shelterBad.empty()
            && (f.badBreaks.empty() || f.badBreaks.front() != f.shelterBad.front()
                || !f.goodBreaks.empty()))
            p.push_back("do not push the pawns in front of your king ("
                        + san(pos, f.shelterBad.front()) + " would weaken it)");
        ss << "Pawns: " << capitalize(join_all(p, "; ")) << ".\n";

        // Space, mobility sectors (Kasparov's principle) and files
        const char* areaName[Trace::AREA_NB] = {"on the queen side", "in the center",
                                                "on the king side"};
        int         bestArea = -1, bestDiff = 0, worstArea = -1, worstDiff = 0;
        for (int a = 0; a < Trace::AREA_NB; ++a)
        {
            const int d = Trace::mobility_area_counts[us][a] - Trace::mobility_area_counts[them][a];
            if (d > bestDiff)
            {
                bestDiff = d;
                bestArea = a;
            }
            if (d < worstDiff)
            {
                worstDiff = d;
                worstArea = a;
            }
        }
        const int space = term_cp(pos, Trace::SPACE, us, phase);
        ss << "Space and files: ";
        ss << (space >= 5    ? "you have more space (" + cp_str(space) + "); "
               : space <= -5 ? Them + " controls more space (" + cp_str(-space) + "); "
                             : std::string("space is balanced; "));
        if (!noPieces && bestArea >= 0)
            ss << "your pieces are most mobile " << areaName[bestArea]
               << ", so play there (Kasparov's principle)"
               << (worstArea >= 0 ? std::string(", while ") + Them + " is stronger "
                                      + areaName[worstArea]
                                  : std::string())
               << "; ";
        std::string open, semiUs, semiThem;
        for (File fl = FILE_A; fl <= FILE_H; ++fl)
        {
            const bool o = pos.pieces(us, PAWN) & file_bb(fl), t = pos.pieces(them, PAWN) & file_bb(fl);
            if (!o && !t)
                open += char('a' + fl);
            else if (!o)
                semiUs += char('a' + fl);
            else if (!t)
                semiThem += char('a' + fl);
        }
        auto files = [](const std::string& s) {
            std::vector<std::string> v;
            for (char c : s)
                v.push_back(std::string(1, c));
            return join_words(v);
        };
        if (!pos.pieces(us, ROOK, QUEEN))
            ss << "the files matter little without rooks and queens";
        else if (!open.empty())
            ss << "the open " << files(open) << "-file" << (open.size() > 1 ? "s are" : " is")
               << " the main road for the rooks";
        else if (!semiUs.empty())
            ss << "your rooks belong on the half-open " << files(semiUs) << "-file"
               << (semiUs.size() > 1 ? "s" : "");
        else
            ss << "the files are closed: the rooks need a pawn break to come into play";
        if (!semiThem.empty() && pos.pieces(them, ROOK, QUEEN))
            ss << ", while " << Them << " will use the " << files(semiThem) << "-file"
               << (semiThem.size() > 1 ? "s" : "");
        ss << ".\n";

        // Key squares
        std::vector<std::string> k;
        if (!noPieces)
        {
            std::vector<std::string> outposts;
            const Bitboard zone = us == WHITE ? (Rank4BB | Rank5BB | Rank6BB) : (Rank5BB | Rank4BB | Rank3BB);
            for (Bitboard b = zone & pawn_attacks_of(us, pos.pieces(us, PAWN)) & ~pos.pieces(us); b;)
            {
                const Square s = pop_lsb(b);
                if (!(pos.pieces(them, PAWN) & pawn_attack_span(us, s)) && outposts.size() < 3)
                    outposts.push_back(UCIEngine::square(s));
            }
            if (!outposts.empty())
                k.push_back("the outpost" + std::string(outposts.size() > 1 ? "s " : " ")
                            + join_words(outposts) + " would suit a knight");
            if (!f.reroutes[us].empty())
            {
                const auto& r = f.reroutes[us].front();
                k.push_back("the evaluation would most like your " + std::string(piece_word(type_of(r.pc)))
                            + " on " + UCIEngine::square(r.to));
            }
        }
        for (bool light : {true, false})
        {
            bool mine = false, theirs = false;
            for (Bitboard b = pos.pieces(us, BISHOP); b;)
                mine |= light_square(pop_lsb(b)) == light;
            for (Bitboard b = pos.pieces(them, BISHOP); b;)
                theirs |= light_square(pop_lsb(b)) == light;
            int targets = 0;
            for (Bitboard b = pos.pieces(them, PAWN); b;)
                targets += light_square(pop_lsb(b)) == light;
            if (mine && !theirs && targets >= 3)
                k.push_back(std::string("you dominate the ") + (light ? "light" : "dark")
                            + " squares (only you have a " + (light ? "light" : "dark")
                            + "-squared bishop)");
        }
        // The most contested occupied square: attacked by both sides
        Square hot = SQ_NONE;
        int    hotScore = 0, hotUs = 0, hotThem = 0;
        for (Bitboard b = pos.pieces() & ~pos.pieces(KING); b;)
        {
            const Square s  = pop_lsb(b);
            const int    au = popcount(pos.attackers_to(s) & pos.pieces(us));
            const int    at = popcount(pos.attackers_to(s) & pos.pieces(them));
            const int    sc = std::min(au, at) * 10 + au + at;
            if (std::min(au, at) >= 2 && sc > hotScore)
            {
                hotScore = sc;
                hot      = s;
                hotUs    = au;
                hotThem  = at;
            }
        }
        if (hot != SQ_NONE)
            k.push_back("the fight revolves around " + UCIEngine::square(hot) + " (attacked "
                        + std::to_string(hotUs) + " times by you, " + std::to_string(hotThem)
                        + " times by " + Them + ")");
        if (k.empty())
            k.push_back("no square is critical yet");
        ss << "Key squares: " << capitalize(join_all(k, "; ")) << ".\n\n";
    }

    // ---- King safety ---------------------------------------------------------------------------
    ss << "KING SAFETY:\n";
    {
        auto shield = [&](Color c) {
            const Square ksq     = pos.square<KING>(c);
            int          missing = 0;
            const Bitboard shieldRanks = c == WHITE ? (Rank2BB | Rank3BB) : (Rank7BB | Rank6BB);
            for (int fl = std::max(0, int(file_of(ksq)) - 1); fl <= std::min(7, int(file_of(ksq)) + 1); ++fl)
                missing += !(pos.pieces(c, PAWN) & file_bb(File(fl)) & shieldRanks);
            return missing;
        };
        auto ring = [&](Color c) {
            int      n = 0;
            Bitboard r = attacks_bb<KING>(pos.square<KING>(c)) | pos.square<KING>(c);
            while (r)
                n += popcount(pos.attackers_to(pop_lsb(r)) & pos.pieces(~c));
            return n;
        };
        // King safety term of each king, for its owner (negative = danger)
        auto kingTerm = [&](Color c) {
            return UCIEngine::to_cp(Trace::Helpers::interpolate(Trace::scores[KING][c], phase), pos);
        };
        const int missUs = shield(us), ringUs = ring(us), ktUs = kingTerm(us);
        const int missThem = shield(them), ktThem = kingTerm(them);
        const bool danger = (missUs >= 2 && pos.pieces(them, QUEEN)) || ringUs >= 5 || ktUs <= -25;

        if (kingPhase)
            ss << "With few pieces left the king is no longer a target but an active piece: "
                  "centralize it and use it to support the pawns. ";
        else if (danger)
            ss << "Your king is under dangerous pressure (" << missUs << " missing shield pawn"
               << (missUs == 1 ? "" : "s") << ", " << ringUs
               << " attacks on the squares around it, king safety term " << cp_str(ktUs)
               << "): defence comes first. ";
        else
            ss << "Your king is safe for now (king safety term " << cp_str(ktUs) << "). ";
        const PlanCandidate* bestK = nullptr;
        for (const auto& c : f.cands)
            if (c.kingMove && (!bestK || c.gain > bestK->gain))
                bestK = &c;
        if (bestK && bestK->gain > 3 && !noPieces)
            ss << (bestK->m.type_of() == CASTLING
                     ? "Castling (" + san(pos, bestK->m) + ") is useful. "
                     : "A prophylactic king move such as " + san(pos, bestK->m) + " is useful. ");
        if (!kingPhase && pos.pieces(us, QUEEN) && (missThem >= 2 || ktThem <= -25))
            ss << "The enemy king is exposed (" << missThem << " open or missing shield file"
               << (missThem == 1 ? "" : "s") << ", king safety term " << cp_str(ktThem)
               << "): bring more pieces towards it and keep the queens on.";
        else if (!kingPhase)
            ss << "The enemy king is well protected: a direct attack is not justified yet.";
        ss << "\n\n";
    }

    // ---- The plan ------------------------------------------------------------------------------
    ss << "THE PLAN:\n";
    {
        std::vector<std::string> steps;
        const std::string        after = " (your evaluation afterwards: " + pawns_str(pos, bestV)
                                + "): " + explanation_sentence(pos, best) + ".";
        if (f.inCheck)
            steps.push_back("Get out of check with " + bestSan + after);
        else if (f.threat)
            steps.push_back("Parry the threat " + san_after_null(pos, f.threatMove)
                            + "; the best way is " + bestSan + after);
        else
            steps.push_back("Play " + bestSan + after);
        std::vector<std::string> alts;
        for (size_t i = 1; i < f.cands.size() && alts.size() < 2; ++i)
            if ((best.q2 == VALUE_NONE) == (f.cands[i].q2 == VALUE_NONE)
                && cp(bestV) - cp(cand_value(f.cands[i])) <= 10 && !is_mate(f.cands[i].q))
                alts.push_back(san(pos, f.cands[i].m));
        if (!alts.empty())
            steps.push_back("Good alternatives: " + join_words(alts, " or ") + ".");
        if (uw && best.m.from_sq() != uw->sq)
        {
            std::string route;
            if (uwRoute)
                route = uwRoute->via != SQ_NONE
                          ? ", for example via " + UCIEngine::square(uwRoute->via) + " to "
                              + UCIEngine::square(uwRoute->to)
                          : ", for example to " + UCIEngine::square(uwRoute->to);
            steps.push_back("Then improve " + piece_on(uw->pc, uw->sq) + route + ".");
        }
        const bool keepTension = f.tension && !f.releaseTension && f.tensionCp <= -10
                              && best.m.from_sq() != f.tensionMove.from_sq()
                              && !(best.capture && type_of(pos.piece_on(best.m.from_sq())) == PAWN);
        if (!f.goodBreaks.empty() && f.goodBreaks.front() != best.m)
            steps.push_back("Consider the pawn break " + san(pos, f.goodBreaks.front()) + ".");
        else if (keepTension)
            steps.push_back("Keep the pawn tension until it can be resolved favourably.");
        if (f.oppIdea && !f.inCheck && !(f.threat && f.oppIdea == f.threatMove))
            steps.push_back("Prophylaxis: watch " + Them + "'s idea " + san_after_null(pos, f.oppIdea)
                            + ", its most useful move if it had a free tempo.");
        if (endgame)
            for (Bitboard b = pos.pieces(us, PAWN); b;)
            {
                const Square s = pop_lsb(b);
                if (!(pos.pieces(them, PAWN) & passed_pawn_span(us, s)) && best.m.from_sq() != s)
                {
                    steps.push_back("In the endgame, push the passed pawn on " + UCIEngine::square(s)
                                    + " with the support of the king.");
                    break;
                }
            }
        for (size_t i = 0; i < steps.size(); ++i)
            ss << "  " << i + 1 << ". " << steps[i] << "\n";
        ss << "\n";
    }

    // ---- In a nutshell -------------------------------------------------------------------------
    ss << "IN A NUTSHELL:\n";
    {
        const char* kind = phase < 24 || noPieces ? nullptr : sharp ? "sharp" : quiet ? "quiet" : nullptr;
        if (aA < 20)
            ss << (kind ? std::string("Balanced, ") + kind + " position" : std::string("Balanced position"));
        else
            ss << (cpA > 0 ? std::string("You are ") : Them + " is ")
               << (aA < 50 ? "slightly better" : aA < 200 ? "better" : "much better")
               << (kind ? std::string(" in a ") + kind + " position" : std::string());
        ss
           << ": " << (f.inCheck ? "answer the check with " : f.threat ? "meet the threat with " : "play ")
           << bestSan;
        if (!f.goodBreaks.empty() && f.goodBreaks.front() != best.m)
            ss << ", aim for the break " << san(pos, f.goodBreaks.front());
        if (uw && uw->sq != best.m.from_sq())
            ss << " and improve " << piece_on(uw->pc, uw->sq);
        ss << ".\n";
    }
    ss << "================================================================\n";

    std::string out = ss.str();
    for (size_t k; (k = out.find(" \n")) != std::string::npos;)
        out.erase(k, 1);
    return out;
}

}  // namespace Eval

}  // namespace Alexander
