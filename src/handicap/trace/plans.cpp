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

// [SC:trace] Counterfactual analysis of the trace.
//
// The classical evaluation is a sum of hand-written terms, so we can ask it "what if" questions
// and read the answer term by term:
//  - what is each piece really worth here?          -> remove it and re-evaluate
//  - which trades help whom?                         -> remove the two pieces and re-evaluate
//  - what does the opponent threaten?                -> pass the move (null move)
//  - which moves improve the position, and why?      -> play them and compare the term tables
//  - where should the worst piece go?                -> relocate it and re-evaluate
// Every evaluation is a static evaluation resolved by a small quiescence search (captures,
// SEE >= 0, plus check evasions), so that hanging pieces and simple exchanges are accounted for.

#include "plans.h"

#include <algorithm>
#include <array>
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

constexpr int   QS_DEPTH   = 8;
constexpr int   NODE_LIMIT = 60000;
constexpr Value MATE_SCORE = 20000;

constexpr std::string_view PieceChars(" PNBRQK  pnbrqk");

std::string color_name(Color c) { return c == WHITE ? "White" : "Black"; }

std::string piece_label(Piece pc, Square s) {
    return std::string(1, "PNBRQK"[type_of(pc) - 1]) + UCIEngine::square(s);
}

// Small static quiescence search: stand pat + captures with SEE >= 0, all evasions when in check
struct QSearch {
    int  nodes = 0;
    Move bestRootMove = Move::none();

    // 'parentStand' is the static value of the parent node (its side to move point of view):
    // a position in check cannot be evaluated statically by the classical evaluation, so at the
    // limits of the search a node in check returns the parent's static value instead.
    Value search(Position& pos, Value alpha, Value beta, int depth, int ply,
                 Value parentStand = VALUE_ZERO) {
        ++nodes;
        const bool inCheck = bool(pos.checkers());

        if (inCheck && (depth <= -4 || nodes >= NODE_LIMIT))
            return -parentStand;
        if (inCheck)
        {
            Value best = -MATE_SCORE + ply;
            for (const auto& m : MoveList<LEGAL>(pos))
            {
                StateInfo st;
                pos.do_move(m, st);
                Value v = -search(pos, -beta, -alpha, depth - 1, ply + 1, -parentStand);
                pos.undo_move(m);
                if (v > best)
                {
                    best = v;
                    if (ply == 0)
                        bestRootMove = m;
                    if (v > alpha)
                    {
                        alpha = v;
                        if (v >= beta)
                            break;
                    }
                }
            }
            return best;
        }

        Value stand = Eval::evaluate_position_any(pos);
        if (stand >= beta || depth <= 0 || nodes >= NODE_LIMIT)
            return stand;
        alpha = std::max(alpha, stand);

        std::vector<std::pair<int, Move>> caps;
        for (const auto& m : MoveList<CAPTURES>(pos))
        {
            if (!pos.legal(m) || !pos.see_ge(m, 0))
                continue;
            const Piece captured = pos.piece_on(m.to_sq());
            const int   order    = (captured == NO_PIECE ? 0 : PieceValue[captured]) * 8
                              - PieceValue[pos.piece_on(m.from_sq())] / 64;
            caps.emplace_back(order, m);
        }
        std::sort(caps.begin(), caps.end(),
                  [](const auto& a, const auto& b) { return a.first > b.first; });

        Value best = stand;
        for (const auto& [order, m] : caps)
        {
            StateInfo st;
            pos.do_move(m, st);
            Value v = -search(pos, -beta, -alpha, depth - 1, ply + 1, stand);
            pos.undo_move(m);
            if (v > best)
            {
                best = v;
                if (ply == 0)
                    bestRootMove = m;
                if (v > alpha)
                {
                    alpha = v;
                    if (v >= beta)
                        break;
                }
            }
        }
        return best;
    }
};

// Quiet static value, side to move point of view
Value quiet(Position& pos, Move* bestCapture = nullptr) {
    QSearch q;
    Value   v = q.search(pos, -VALUE_INFINITE, VALUE_INFINITE, QS_DEPTH, 0);
    if (bestCapture)
        *bestCapture = q.bestRootMove;
    return v;
}

}  // namespace

Value quiet_evaluation(Position& pos) { return quiet(pos); }

namespace {

Value quiet_white(Position& pos) {
    Value v = quiet(pos);
    return pos.side_to_move() == WHITE ? v : -v;
}

// When the root position contains an immediate tactic (a forced mate or a large capture
// sequence), every counterfactual comparison would be dominated by that tactic: in that case the
// structural probes (piece values, trades, maneuvers) fall back to the bare static evaluation.
bool StaticProbes = false;

Value probe_white(Position& pos) {
    if (!StaticProbes)
        return quiet_white(pos);
    Value v = Eval::evaluate_position_any(pos);
    return pos.side_to_move() == WHITE ? v : -v;
}

// Builds a FEN from an edited board, keeping only the castling rights that are still valid
std::string board_fen(const std::array<Piece, SQUARE_NB>& b, Color stm, const Position& ref) {
    std::stringstream ss;
    for (int ri = RANK_8; ri >= RANK_1; --ri)
    {
        const Rank r     = Rank(ri);
        int        empty = 0;
        for (File f = FILE_A; f <= FILE_H; ++f)
        {
            Piece pc = b[make_square(f, r)];
            if (pc == NO_PIECE)
                ++empty;
            else
            {
                if (empty)
                    ss << empty;
                empty = 0;
                ss << PieceChars[pc];
            }
        }
        if (empty)
            ss << empty;
        if (ri > RANK_1)
            ss << '/';
    }
    ss << (stm == WHITE ? " w " : " b ");

    std::string castling;
    if (!ref.is_chess960())
    {
        if (ref.can_castle(WHITE_OO) && b[SQ_E1] == W_KING && b[SQ_H1] == W_ROOK)
            castling += 'K';
        if (ref.can_castle(WHITE_OOO) && b[SQ_E1] == W_KING && b[SQ_A1] == W_ROOK)
            castling += 'Q';
        if (ref.can_castle(BLACK_OO) && b[SQ_E8] == B_KING && b[SQ_H8] == B_ROOK)
            castling += 'k';
        if (ref.can_castle(BLACK_OOO) && b[SQ_E8] == B_KING && b[SQ_A8] == B_ROOK)
            castling += 'q';
    }
    ss << (castling.empty() ? "-" : castling) << " - 0 1";
    return ss.str();
}

// Evaluates an edited board (quiet value, White point of view). Returns false if the edited
// position is not legal (e.g. the side not to move is in check after a blocker was removed).
bool eval_edited(const std::array<Piece, SQUARE_NB>& b,
                 const Position&                     ref,
                 Value&                              outWhite,
                 Color                               stmEdited = COLOR_NB) {
    StateInfo st;
    Position  tmp;
    const Color stmFen = stmEdited == COLOR_NB ? ref.side_to_move() : stmEdited;
    if (tmp.set(board_fen(b, stmFen, ref), ref.is_chess960(), &st, ref.this_thread()))
        return false;
    const Color stm = tmp.side_to_move();
    if (tmp.attackers_to(tmp.square<KING>(~stm)) & tmp.pieces(stm))
        return false;
    if (StaticProbes && tmp.checkers())  // no static evaluation in check
        return false;
    outWhite = probe_white(tmp);
    return true;
}

bool is_mate(Value v) { return std::abs(v) >= MATE_SCORE - 2 * MAX_PLY; }

std::string fmt_cp(int cp) {
    std::stringstream ss;
    ss << std::showpos << cp << std::noshowpos << " cp";
    return ss.str();
}

std::string fmt_pawns(double p) {
    std::stringstream ss;
    ss << std::showpos << std::fixed << std::setprecision(2) << p << std::noshowpos;
    return ss.str();
}

// Terms used to explain the effect of a move
struct TermInfo {
    int         term;
    const char* name;
};
constexpr TermInfo ExplainTerms[] = {
  {Trace::MATERIAL, "Material+PSQT"}, {Trace::IMBALANCE, "Imbalance"},
  {PAWN, "Pawn structure"},           {KNIGHT, "Knights"},
  {BISHOP, "Bishops"},                {ROOK, "Rooks"},
  {QUEEN, "Queens"},                  {Trace::MOBILITY, "Mobility"},
  {KING, "King safety"},              {Trace::THREAT, "Threats"},
  {Trace::PASSED, "Passed pawns"},    {Trace::SPACE, "Space"}};

// Term-by-term difference between two snapshots (b - a), point of view of 'side':
// (cp, term) pairs with |cp| >= 3, largest first
std::vector<std::pair<int, int>> term_deltas(const TermSnapshot& a,
                                             const TermSnapshot& b,
                                             Color               side,
                                             int                 phase,
                                             const Position&     pos) {
    std::vector<std::pair<int, int>> d;
    for (const auto& ti : ExplainTerms)
    {
        ScoreForClassical na = a.s[ti.term][WHITE] - a.s[ti.term][BLACK];
        ScoreForClassical nb = b.s[ti.term][WHITE] - b.s[ti.term][BLACK];
        int v  = Trace::Helpers::interpolate(nb - na, phase) * (side == WHITE ? 1 : -1);
        int cp = UCIEngine::to_cp(v, pos);
        if (std::abs(cp) >= 3)
            d.emplace_back(cp, ti.term);
    }
    std::stable_sort(d.begin(), d.end(), [](const auto& x, const auto& y) {
        return std::abs(x.first) > std::abs(y.first);
    });
    return d;
}

std::string explain_terms(const std::vector<std::pair<int, int>>& d, int maxItems = 3) {
    std::stringstream ss;
    for (int i = 0; i < int(d.size()) && i < maxItems; ++i)
        ss << (i ? ", " : "") << plan_term_name(d[i].second) << " " << std::showpos << d[i].first
           << std::noshowpos;
    return d.empty() ? "no significant change in the evaluation terms" : ss.str();
}

struct PieceValue_ {
    Square    sq;
    Piece     pc;
    int       cp;          // value for its owner (cp, UCI scale)
    double    pawns;       // same value in pawn units
    int       efficiency;  // value / nominal value of the piece type, in %
};

struct Candidate {
    Move        m;
    Value       q;     // quiet value after the move, mover point of view
    int         cp;    // q in cp
    int         gain;  // cp gained with respect to passing
    int         wp;    // win probability for the mover
    bool        capture, check, pawnBreak, kingMove, sacrifice;
    std::string why;
    std::vector<std::pair<int, int>> deltas;
};

}  // namespace

const char* plan_term_name(int term) {
    for (const auto& ti : ExplainTerms)
        if (ti.term == term)
            return ti.name;
    return "?";
}

std::string generate_plans_analysis(Position&          pos,
                                    Value              v_white,
                                    uint8_t            winProb,
                                    const std::string& shashinZone,
                                    const std::string& gamePhase,
                                    int                phase,
                                    PlanFacts*         facts) {
    std::stringstream ss;
    PlanFacts         localFacts;
    PlanFacts&        F = facts ? *facts : localFacts;
    (void) winProb;
    (void) shashinZone;
    (void) gamePhase;
    const Color us   = pos.side_to_move();
    const Color them = ~us;
    auto        cp   = [&](Value v) { return UCIEngine::to_cp(v, pos); };

    // Unit of the piece values: the endgame value of a pawn (the classical "pawn" of the
    // engine, PawnValue), so that a knight is worth about 4 and a queen about 12
    const double pawnUnit = double(PawnValueEg);
    auto nominal = [&](PieceType pt) {
        return (PieceValueForPSQT[MG][pt] * phase + PieceValueForPSQT[EG][pt] * (PHASE_MIDGAME - phase))
             / PHASE_MIDGAME;
    };

    const Value staticUs   = Eval::evaluate_position_any(pos);
    Move        rootTactic = Move::none();
    const Value quietUs    = quiet(pos, &rootTactic);
    StaticProbes           = is_mate(quietUs) || std::abs(quietUs - staticUs) > 2 * PawnValueEg;
    const Value q0White    = probe_white(pos);
    auto        vstr       = [&](Value v) {  // value (mover point of view) as text
        return is_mate(v) ? std::string(v > 0 ? "MATE" : "MATED") : fmt_cp(cp(v));
    };

    ss << "================================================================\n";
    ss << "=== COUNTERFACTUAL ANALYSIS: PLANS (classical evaluation) ===\n";
    ss << "All values come from static evaluations resolved by a capture-only quiescence\n"
       << "search; cp are on the UCI scale of the engine, piece values in pawn units.\n"
       << "Quiet evaluation (" << color_name(us) << "): " << vstr(quietUs)
       << " | static evaluation (White): " << fmt_cp(cp(v_white)) << "\n";
    if (StaticProbes)
        ss << "*** TACTICAL ALERT: " << color_name(us) << " has an immediate tactic ("
           << UCIEngine::move(rootTactic, pos.is_chess960()) << ", " << vstr(quietUs)
           << "). Piece values, trades and maneuvers below use the bare static evaluation, "
              "otherwise the tactic would dominate every comparison. ***\n";
    ss << "\n";

    // ---------------------------------------------------------------------
    // 1. PIECE VALUES BY ABLATION
    // ---------------------------------------------------------------------
    std::vector<PieceValue_> values;
    std::array<int, SQUARE_NB> boardValue{};
    std::array<bool, SQUARE_NB> hasValue{};
    for (Square s = SQ_A1; s <= SQ_H8; ++s)
    {
        Piece pc = pos.piece_on(s);
        if (pc == NO_PIECE || type_of(pc) == KING)
            continue;
        auto b = pos.piece_array();
        b[s]   = NO_PIECE;
        Value w;
        if (!eval_edited(b, pos, w))
            continue;
        const Value v   = color_of(pc) == WHITE ? q0White - w : w - q0White;
        PieceValue_ pv;
        pv.sq         = s;
        pv.pc         = pc;
        pv.cp         = cp(v);
        pv.pawns      = double(v) / pawnUnit;
        pv.efficiency = int(std::lround(100.0 * v / std::max(1, int(nominal(type_of(pc))))));
        values.push_back(pv);
        boardValue[s] = int(std::lround(pv.pawns * 100));
        hasValue[s]   = true;
    }

    ss << "--- 1. PIECE VALUES BY ABLATION (what each piece is worth HERE) ---\n";
    ss << "Value = evaluation lost by the owner if the piece disappeared (pawn units).\n";
    for (int ri = RANK_8; ri >= RANK_1; --ri)
    {
        const Rank r = Rank(ri);
        ss << "+-------+-------+-------+-------+-------+-------+-------+-------+\n|";
        for (File f = FILE_A; f <= FILE_H; ++f)
        {
            Piece pc = pos.piece_on(make_square(f, r));
            ss << "   " << (pc == NO_PIECE ? ' ' : PieceChars[pc]) << "   |";
        }
        ss << "\n|";
        for (File f = FILE_A; f <= FILE_H; ++f)
        {
            Square s = make_square(f, r);
            if (hasValue[s])
                ss << " " << std::setw(5) << std::showpos << std::fixed
                   << std::setprecision(std::abs(boardValue[s]) >= 1000 ? 1 : 2)
                   << boardValue[s] / 100.0 << std::noshowpos << " |";
            else
                ss << "       |";
        }
        ss << "\n";
    }
    ss << "+-------+-------+-------+-------+-------+-------+-------+-------+\n";

    auto ranked = [&](Color c, bool piecesOnly) {
        std::vector<PieceValue_> v;
        for (const auto& p : values)
            if (color_of(p.pc) == c && (!piecesOnly || type_of(p.pc) != PAWN))
                v.push_back(p);
        std::sort(v.begin(), v.end(),
                  [](const auto& a, const auto& b) { return a.efficiency < b.efficiency; });
        return v;
    };

    ss << "Piece efficiency (value / normal value of the piece type), worst to best:\n";
    for (Color c : {WHITE, BLACK})
    {
        auto v = ranked(c, true);
        ss << "  " << color_name(c) << ": ";
        for (size_t i = 0; i < v.size(); ++i)
            ss << (i ? ", " : "") << piece_label(v[i].pc, v[i].sq) << " " << v[i].efficiency
               << "%";
        if (v.empty())
            ss << "(no pieces)";
        ss << "\n";
    }
    // Pawns: the most valuable pawn of each side (structural keystone) and the weakest one
    for (Color c : {WHITE, BLACK})
    {
        std::vector<PieceValue_> pw;
        for (const auto& p : values)
            if (color_of(p.pc) == c && type_of(p.pc) == PAWN)
                pw.push_back(p);
        if (pw.empty())
            continue;
        auto mm = std::minmax_element(pw.begin(), pw.end(),
                                      [](const auto& a, const auto& b) { return a.cp < b.cp; });
        ss << "  " << color_name(c) << " pawns: key pawn " << UCIEngine::square(mm.second->sq)
           << " (" << fmt_pawns(mm.second->pawns) << "), least useful "
           << UCIEngine::square(mm.first->sq) << " (" << fmt_pawns(mm.first->pawns) << ")\n";
    }
    ss << "  (Efficiency > 120%: the piece does more than its material; < 80%: passive or "
          "badly placed; a negative pawn value means the pawn is a liability.)\n\n";

    // ---------------------------------------------------------------------
    // 2. TRADE ADVISOR
    // ---------------------------------------------------------------------
    struct Trade {
        std::string desc;
        int         cpWhite;  // effect for White
    };
    std::vector<Trade> trades;
    auto               cls = [](PieceType pt) { return pt == BISHOP ? KNIGHT : pt; };
    for (const auto& a : values)
    {
        if (color_of(a.pc) != WHITE || type_of(a.pc) == PAWN)
            continue;
        for (const auto& b : values)
        {
            if (color_of(b.pc) != BLACK || cls(type_of(b.pc)) != cls(type_of(a.pc)))
                continue;
            auto bd  = pos.piece_array();
            bd[a.sq] = bd[b.sq] = NO_PIECE;
            Value w;
            if (!eval_edited(bd, pos, w))
                continue;
            trades.push_back({piece_label(a.pc, a.sq) + " for " + piece_label(b.pc, b.sq),
                              cp(w - q0White)});
            F.trades.push_back({a.pc, b.pc, a.sq, b.sq, cp(w - q0White)});
        }
    }
    auto mass_trade = [&](PieceType pt, int& out) {
        if (!popcount(pos.pieces(WHITE, pt)) || !popcount(pos.pieces(BLACK, pt)))
            return false;
        auto bd = pos.piece_array();
        for (auto& p : bd)
            if (p != NO_PIECE && type_of(p) == pt)
                p = NO_PIECE;
        Value w;
        if (!eval_edited(bd, pos, w))
            return false;
        out = cp(w - q0White);
        return true;
    };

    ss << "--- 2. TRADE ADVISOR (effect of removing both pieces, White point of view) ---\n";
    int queenTrade = 0, rookTrade = 0;
    bool hasQueenTrade = mass_trade(QUEEN, queenTrade);
    bool hasRookTrade  = mass_trade(ROOK, rookTrade);
    F.hasQueenTrade    = hasQueenTrade;
    F.hasRookTrade     = hasRookTrade;
    F.queenTrade       = queenTrade;
    F.rookTrade        = rookTrade;
    std::sort(F.trades.begin(), F.trades.end(),
              [](const auto& a, const auto& b) { return a.cpWhite > b.cpWhite; });
    for (const auto& p : values)
        F.pieces.push_back({p.sq, p.pc, p.cp, p.efficiency});
    if (hasQueenTrade)
        ss << "  Queen trade      : " << fmt_cp(queenTrade) << " -> "
           << (queenTrade > 8    ? "White should seek it, Black should avoid it"
               : queenTrade < -8 ? "Black should seek it, White should avoid it"
                                 : "neutral")
           << "\n";
    if (hasRookTrade)
        ss << "  All rooks off    : " << fmt_cp(rookTrade) << " -> "
           << (rookTrade > 8    ? "favors White"
               : rookTrade < -8 ? "favors Black"
                                : "neutral")
           << "\n";
    std::sort(trades.begin(), trades.end(),
              [](const auto& a, const auto& b) { return a.cpWhite > b.cpWhite; });
    if (!trades.empty())
    {
        ss << "  Best trades for White: ";
        int n = 0;
        for (const auto& t : trades)
            if (t.cpWhite > 3 && n < 3)
                ss << (n++ ? " | " : "") << t.desc << " (" << fmt_cp(t.cpWhite) << ")";
        ss << (n ? "\n" : "none that helps\n");
        ss << "  Best trades for Black: ";
        n = 0;
        for (auto it = trades.rbegin(); it != trades.rend(); ++it)
            if (it->cpWhite < -3 && n < 3)
                ss << (n++ ? " | " : "") << it->desc << " (" << fmt_cp(-it->cpWhite)
                   << " for Black)";
        ss << (n ? "\n" : "none that helps\n");
    }
    if (!hasQueenTrade && !hasRookTrade && trades.empty())
        ss << "  No like-for-like trades are available.\n";
    ss << "\n";

    // ---------------------------------------------------------------------
    // 3. THREATS: what if the side to move passed?
    // ---------------------------------------------------------------------
    const bool inCheck = bool(pos.checkers());
    Value      qNullUs = quietUs;
    Move       threatMove = Move::none();
    ss << "--- 3. THREATS (what the opponent would do if " << color_name(us) << " passed) ---\n";
    if (inCheck)
        ss << "  " << color_name(us) << " is in check: no null move analysis.\n\n";
    else
    {
        StateInfo st;
        pos.do_null_move(st);
        qNullUs = -quiet(pos, &threatMove);
        pos.undo_null_move();
        const int lossCp = cp((StaticProbes ? staticUs : quietUs) - qNullUs);
        F.threat         = threatMove && (is_mate(qNullUs) || lossCp >= 10);
        F.threatMate     = is_mate(qNullUs);
        F.threatMove     = threatMove;
        F.threatCp       = lossCp;
        const int loss = cp((StaticProbes ? staticUs : quietUs) - qNullUs);
        if (threatMove && (is_mate(qNullUs) || loss >= 10))
            ss << "  THREAT: " << color_name(them) << " threatens "
               << UCIEngine::move(threatMove, pos.is_chess960()) << ", worth "
               << (is_mate(qNullUs) ? std::string("MATE") : std::to_string(loss) + " cp")
               << ". It must be prevented or met.\n\n";
        else
            ss << "  No immediate tactical threat against " << color_name(us)
               << " (passing costs " << std::max(0, loss) << " cp).\n\n";
    }

    // ---------------------------------------------------------------------
    // 4. CANDIDATE PLANS
    // ---------------------------------------------------------------------
    std::vector<Candidate> cands;
    for (const auto& m : MoveList<LEGAL>(pos))
    {
        Candidate c;
        c.m         = m;
        c.capture   = pos.capture_stage(m);
        c.check     = pos.gives_check(m);
        c.kingMove  = type_of(pos.piece_on(m.from_sq())) == KING;
        c.sacrifice = !pos.see_ge(m, -Value(PawnValueMg / 2));  // B-for-N trades are not sacrifices
        c.pawnBreak = false;
        StateInfo st;
        pos.do_move(m, st);
        if (type_of(pos.piece_on(m.to_sq())) == PAWN)
        {
            Bitboard att = attacks_bb<PAWN>(m.to_sq(), us);
            c.pawnBreak  = c.capture || (att & pos.pieces(them, PAWN));
        }
        c.q = -quiet(pos);
        pos.undo_move(m);
        c.cp   = cp(c.q);
        c.gain = cp(c.q - qNullUs);
        c.wp   = WDLModel::get_win_probability(c.q, pos);
        cands.push_back(c);
    }
    // Near-ties are broken in favour of queen promotions (a small sorting bonus, larger in
    // clearly won positions where the static evaluation is flat); displayed values unchanged
    auto key = [](const Candidate& c) {
        const bool qp = c.m.type_of() == PROMOTION && c.m.promotion_type() == QUEEN;
        return c.q + (qp ? (c.q > 2 * PawnValueEg ? 2 * PawnValueEg : 15) : 0);
    };
    std::stable_sort(cands.begin(), cands.end(),
                     [&](const auto& a, const auto& b) { return key(a) > key(b); });

    // Term-by-term explanation: compare the position after the move with the position after a
    // pass (same side to move in both, so the comparison is tempo neutral)
    TermSnapshot nullSnap{};
    bool         haveNullSnap = false;
    if (!inCheck)
    {
        StateInfo st;
        pos.do_null_move(st);
        nullSnap     = snapshot_terms(pos);
        haveNullSnap = true;
        pos.undo_null_move();
    }
    auto explain = [&](Candidate& c) {
        if (!haveNullSnap || c.capture)
            return;
        if (c.check)
        {
            c.why = "forcing move (check): the classical evaluation is not defined in check";
            return;
        }
        StateInfo st;
        pos.do_move(c.m, st);
        TermSnapshot after = snapshot_terms(pos);
        pos.undo_move(c.m);
        c.deltas = term_deltas(nullSnap, after, us, phase, pos);
        c.why    = explain_terms(c.deltas);
    };

    ss << "--- 4. CANDIDATE PLANS for " << color_name(us)
       << " (quiet static value; gain = cp gained versus passing) ---\n";
    const int topN = std::min<int>(8, int(cands.size()));
    for (int i = 0; i < topN; ++i)
    {
        auto& c = cands[i];
        explain(c);
        ss << "  " << std::left << std::setw(6) << UCIEngine::move(c.m, pos.is_chess960())
           << std::right << " " << std::setw(8) << vstr(c.q) << " (" << std::setw(3) << c.wp
           << "%) gain " << std::setw(8) << (is_mate(c.q) ? std::string("-") : fmt_cp(c.gain));
        std::string tags;
        if (c.m.type_of() == PROMOTION)
            tags += " promotion";
        if (c.capture && pos.capture(c.m))
            tags += " capture";
        if (c.check)
            tags += " check";
        if (c.pawnBreak && !c.capture)
            tags += " pawn-break";
        if (c.kingMove)
            tags += " king";
        if (c.sacrifice)
            tags += " SACRIFICE(SEE<0)";
        if (!tags.empty())
            ss << " [" << tags.substr(1) << "]";
        ss << "\n";
        if (!c.why.empty())
            ss << "         why: " << c.why << "\n";
    }
    if (cands.empty())
        ss << "  No legal moves.\n";
    ss << "\n";
    for (const auto& c : cands)
        F.cands.push_back({c.m, c.q, c.cp, c.gain, c.wp, c.capture, c.check, c.kingMove,
                           c.sacrifice, c.pawnBreak, c.deltas});

    // Two-ply check of the best candidates: the capture-only probe after a move counts a threat
    // created by that move as if it were already executed (the opponent cannot retreat in a
    // capture search). Letting the opponent choose its best reply, each reply followed by a
    // quiet probe, removes most of this optimism; the Master's Verdict ranks and assesses the
    // top candidates by this value.
    {
        const int K = std::min<int>(4, int(F.cands.size()));
        bool      any = false;
        for (int i = 0; i < K; ++i)
        {
            auto& c = F.cands[i];
            if (is_mate(c.q))
                continue;
            StateInfo st;
            pos.do_move(c.m, st);
            Value worst = VALUE_INFINITE;
            Move  reply = Move::none();
            for (const auto& r : MoveList<LEGAL>(pos))
            {
                StateInfo st2;
                pos.do_move(r, st2);
                const Value v = quiet(pos);  // our point of view (we are to move again)
                pos.undo_move(r);
                if (v < worst)
                {
                    worst = v;
                    reply = r;
                }
            }
            pos.undo_move(c.m);
            if (reply)
            {
                c.q2    = worst;
                c.reply = reply;
                any     = true;
            }
        }
        if (any)
        {
            ss << "--- 4b. TWO-PLY CHECK (quiet value after the opponent's best reply) ---\n";
            for (int i = 0; i < K; ++i)
            {
                const auto& c = F.cands[i];
                if (c.q2 == VALUE_NONE)
                    continue;
                StateInfo st;
                pos.do_move(c.m, st);
                const std::string rs = UCIEngine::move(c.reply, pos.is_chess960());
                pos.undo_move(c.m);
                ss << "  " << std::left << std::setw(6) << UCIEngine::move(c.m, pos.is_chess960())
                   << std::right << " " << std::setw(8) << vstr(c.q2) << "  (best reply " << rs
                   << ")\n";
            }
            ss << "\n";
            std::stable_sort(F.cands.begin(), F.cands.begin() + K, [](const auto& a, const auto& b) {
                const Value va = a.q2 == VALUE_NONE ? a.q : a.q2;
                const Value vb = b.q2 == VALUE_NONE ? b.q : b.q2;
                return va > vb;
            });
        }
    }

    // ---------------------------------------------------------------------
    // 5. PAWN BREAKS AND TENSION
    // ---------------------------------------------------------------------
    ss << "--- 5. PAWN BREAKS AND TENSION (" << color_name(us) << ") ---\n";
    Value bestNonPawnCapture = -VALUE_INFINITE;
    for (const auto& c : cands)
        if (!(c.capture && type_of(pos.piece_on(c.m.from_sq())) == PAWN
              && type_of(pos.piece_on(c.m.to_sq())) == PAWN))
        {
            bestNonPawnCapture = c.q;
            break;
        }
    std::vector<const Candidate*> breaks, tension;
    for (const auto& c : cands)
    {
        const bool pawnMove = type_of(pos.piece_on(c.m.from_sq())) == PAWN;
        if (pawnMove && c.capture && type_of(pos.piece_on(c.m.to_sq())) == PAWN)
            tension.push_back(&c);
        else if (pawnMove && c.pawnBreak)
            breaks.push_back(&c);
    }
    if (tension.empty())
        ss << "  No pawn tension to resolve.\n";
    if (!tension.empty())
    {
        F.tension        = true;
        F.tensionMove    = tension.front()->m;
        F.tensionCp      = cp(tension.front()->q - bestNonPawnCapture);
        F.releaseTension = F.tensionCp >= 0;
    }
    for (const auto* b : breaks)
    {
        if (b->gain >= 5)
            F.goodBreaks.push_back(b->m);
        else if (b->gain < -15)
            F.badBreaks.push_back(b->m);
    }
    for (const auto* c : tension)
    {
        const int diff = cp(c->q - bestNonPawnCapture);
        ss << "  Tension " << UCIEngine::move(c->m, pos.is_chess960()) << ": "
           << (diff >= 0 ? "RELEASE IT (the capture is at least as good as the best quiet move, "
                         : "MAINTAIN IT (capturing scores ")
           << (diff >= 0 ? "" : fmt_cp(diff) + " versus the best alternative)")
           << (diff >= 0 ? fmt_cp(diff) + ")" : "") << "\n";
    }
    if (breaks.empty())
        ss << "  No pawn break available.\n";
    for (size_t i = 0; i < breaks.size() && i < 4; ++i)
        ss << "  Break " << UCIEngine::move(breaks[i]->m, pos.is_chess960()) << ": "
           << fmt_cp(breaks[i]->cp) << ", gain " << fmt_cp(breaks[i]->gain)
           << (breaks[i]->gain >= 5   ? " -> recommended"
               : breaks[i]->gain < -15 ? " -> premature"
                                       : " -> playable")
           << "\n";
    // Pushes that weaken the king's shelter
    const Square ksq = pos.square<KING>(us);
    for (const auto& c : cands)
    {
        if (type_of(pos.piece_on(c.m.from_sq())) != PAWN || c.capture)
            continue;
        if (std::abs(int(file_of(c.m.from_sq())) - int(file_of(ksq))) > 1
            || distance(c.m.from_sq(), ksq) > 2 || c.gain > -15 || !haveNullSnap)
            continue;
        if (!pos.see_ge(c.m, 0) || c.check)
        {
            if (!c.check)
                ss << "  Avoid " << UCIEngine::move(c.m, pos.is_chess960())
                   << ": it loses material (" << fmt_cp(c.gain) << ").\n";
            continue;
        }
        StateInfo st;
        pos.do_move(c.m, st);
        TermSnapshot after = snapshot_terms(pos);
        pos.undo_move(c.m);
        const int ks = UCIEngine::to_cp(
          Trace::Helpers::interpolate((after.s[KING][WHITE] - after.s[KING][BLACK])
                                        - (nullSnap.s[KING][WHITE] - nullSnap.s[KING][BLACK]),
                                      phase)
            * (us == WHITE ? 1 : -1),
          pos);
        if (ks <= -8)
            F.shelterBad.push_back(c.m);
        if (ks <= -8)
            ss << "  Avoid " << UCIEngine::move(c.m, pos.is_chess960())
               << ": it weakens the king's shelter (King safety " << fmt_cp(ks) << ", total "
               << fmt_cp(c.gain) << ").\n";
    }
    ss << "\n";

    // ---------------------------------------------------------------------
    // 6. MANEUVERS: Makogonov rerouting of the worst pieces
    // ---------------------------------------------------------------------
    // Baseline for the maneuvers of the side to move: the same position with the opponent to
    // move (a pass), so that a relocation is not credited with an extra free tempo
    Value nullProbeWhite = q0White;
    if (!inCheck)
    {
        StateInfo st;
        pos.do_null_move(st);
        nullProbeWhite = probe_white(pos);
        pos.undo_null_move();
    }
    struct Reroute {
        std::string piece, path;
        int         gain = 0;
        Piece       pc   = NO_PIECE;
        Square      from = SQ_NONE, via = SQ_NONE, to = SQ_NONE;
    };
    auto reroute = [&](const PieceValue_& p) {
        Reroute     r;
        const Color c  = color_of(p.pc);
        PieceType   pt = type_of(p.pc);
        r.piece        = piece_label(p.pc, p.sq);
        Bitboard occ   = pos.pieces();
        Bitboard first = attacks_bb(pt, p.sq, occ) & ~pos.pieces();
        Bitboard reach = first;
        Bitboard tmp   = first;
        while (tmp)
            reach |= attacks_bb(pt, pop_lsb(tmp), occ ^ p.sq) & ~pos.pieces();
        int    best = 0;
        Square bestSq = SQ_NONE;
        while (reach)
        {
            Square to = pop_lsb(reach);
            auto   bd = pos.piece_array();
            bd[p.sq]  = NO_PIECE;
            bd[to]    = p.pc;
            // The piece "arrives" on the target square, so the opponent is to move there: the
            // baseline is the current position with the opponent to move as well
            Value w;
            if (!eval_edited(bd, pos, w, ~c))
                continue;
            const Value base = c == us ? nullProbeWhite : q0White;
            int         g    = cp(c == WHITE ? w - base : base - w);
            if (g > best)
            {
                best   = g;
                bestSq = to;
            }
        }
        if (bestSq != SQ_NONE)
        {
            r.gain = best;
            r.pc   = p.pc;
            r.from = p.sq;
            r.to   = bestSq;
            if (!(first & bestSq))
            {
                Bitboard via0 = first & attacks_bb(pt, bestSq, occ ^ p.sq);
                r.via         = via0 ? lsb(via0) : SQ_NONE;
            }
            if (first & bestSq)
                r.path = r.piece + "-" + UCIEngine::square(bestSq);
            else
            {
                Bitboard via = first & attacks_bb(pt, bestSq, occ ^ p.sq);
                r.path       = r.piece + "-" + (via ? UCIEngine::square(lsb(via)) + "-" : "?-")
                       + UCIEngine::square(bestSq);
            }
        }
        return r;
    };

    ss << "--- 6. MANEUVERS (Makogonov: improve the worst piece; target square found by "
          "relocating it) ---\n";
    std::vector<Reroute> usReroutes, themReroutes;
    for (Color c : {us, them})
    {
        if (c == us && inCheck)
            continue;
        auto v = ranked(c, true);
        if (v.empty())
            continue;
        std::vector<std::pair<Reroute, int>> all;  // reroute, efficiency
        for (const auto& p : v)
            all.emplace_back(reroute(p), p.efficiency);
        const auto& worst = all.front();
        ss << "  " << color_name(c) << " worst piece " << worst.first.piece << " (efficiency "
           << worst.second << "%): ";
        if (worst.first.gain >= 5)
            ss << "reroute " << worst.first.path << " (" << fmt_cp(worst.first.gain)
               << " once there)\n";
        else
            ss << "no better square within two moves; free it with a pawn break or trade it.\n";
        std::sort(all.begin(), all.end(),
                  [](const auto& a, const auto& b) { return a.first.gain > b.first.gain; });
        for (const auto& [r, eff] : all)
            if (r.gain >= 5)
                F.reroutes[c].push_back({r.pc, r.from, r.via, r.to, r.gain});
        ss << "  " << color_name(c) << " most profitable maneuvers: ";
        int n = 0;
        for (const auto& [r, eff] : all)
            if (r.gain >= 5 && n < 3)
            {
                ss << (n++ ? " | " : "") << r.path << " (" << fmt_cp(r.gain) << ")";
                (c == us ? usReroutes : themReroutes).push_back(r);
            }
        ss << (n ? "\n" : "none\n");
    }
    ss << "  (Target squares come from relocating the piece and re-evaluating: they show where "
          "the piece would work best, the path is a suggestion.)\n\n";

    // ---------------------------------------------------------------------
    // 7. OPPONENT'S IDEAS (prophylaxis: what would the opponent play if it were its turn?)
    // ---------------------------------------------------------------------
    std::vector<std::pair<Value, Move>> oppIdeas;
    if (!inCheck)
    {
        StateInfo st;
        pos.do_null_move(st);
        if (!pos.checkers())
            for (const auto& m : MoveList<LEGAL>(pos))
            {
                StateInfo st2;
                pos.do_move(m, st2);
                Value q = -quiet(pos);
                pos.undo_move(m);
                oppIdeas.emplace_back(q, m);
            }
        pos.undo_null_move();
        std::sort(oppIdeas.begin(), oppIdeas.end(),
                  [](const auto& a, const auto& b) { return a.first > b.first; });
    }
    if (!oppIdeas.empty())
    {
        F.oppIdea   = oppIdeas.front().second;
        F.oppIdeaCp = cp(oppIdeas.front().first + (StaticProbes ? staticUs : quietUs));
    }
    ss << "--- 7. OPPONENT'S IDEAS (best " << color_name(them)
       << " moves if it were its turn: candidates for prophylaxis) ---\n  ";
    for (size_t i = 0; i < oppIdeas.size() && i < 5; ++i)
        ss << (i ? " | " : "") << UCIEngine::move(oppIdeas[i].second, pos.is_chess960()) << " ("
           << fmt_cp(cp(oppIdeas[i].first + (StaticProbes ? staticUs : quietUs))) << ")";
    if (oppIdeas.empty())
        ss << "n/a";
    ss << "\n  (values: improvement for " << color_name(them)
       << " with respect to the current position, " << color_name(us) << " to move in both)\n\n";

    // The Master's Verdict is composed from these facts in verdict.cpp and printed first
    F.staticUs    = staticUs;
    F.quietUs     = quietUs;
    F.nullQuietUs = qNullUs;
    F.tactical    = StaticProbes;
    F.inCheck     = inCheck;
    F.rootTactic  = rootTactic;

    return ss.str();
}

}  // namespace Eval

}  // namespace Alexander
