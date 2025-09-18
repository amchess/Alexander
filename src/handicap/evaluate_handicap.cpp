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

#include "evaluate_handicap.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include "../misc.h"
#include "../shashin/shashin_position.h"
#include "../types.h"
#include "../uci.h"
#include "../ucioption.h"
#include "../bitboard.h"
#include <string>
#include <ctime>
#include "../movegen.h"
#include "../shashin/shashin_types.h"
#include "../shashin/shashin_manager.h"
#include <random>  //from Handicap mode

namespace Alexander {

namespace Eval {
HandicapConfig handicapConfig;

double get_error_coefficient(int uciElo) {
    auto sigmoid = [](double x, double midpoint, double scale) -> double {
        return 1.0 / (1.0 + std::exp((x - midpoint) / (scale * 1.2)));
    };

    if (uciElo <= ABSOLUTE_BEGINNER_MAX_ELO)
        return sigmoid(uciElo, MIN_ELO, 100.0);
    else if (uciElo <= BEGINNER_MAX_ELO)
        return sigmoid(uciElo, ABSOLUTE_BEGINNER_MAX_ELO, 150.0);
    else if (uciElo <= INTERMEDIATE_MAX_ELO)
        return sigmoid(uciElo, BEGINNER_MAX_ELO, 200.0);
    else if (uciElo <= ADVANCED_MAX_ELO)
        return sigmoid(uciElo, INTERMEDIATE_MAX_ELO, 250.0);
    else
        return sigmoid(uciElo, ADVANCED_MAX_ELO, 300.0);
}

int get_dynamic_phase_limit(int uciElo, bool isOpening) {
    return std::clamp(
      isOpening ? (10 + (uciElo - 1400) / 200 * 5) : (30 + (uciElo - 1400) / 200 * 10), 10, 75);
}

// Thread-local random number generator
static thread_local std::mt19937_64 tls_rng(std::random_device{}());

std::vector<MinMax> min_max_threshold;
#include <array>
#include <cstdint>

constexpr std::array<uint8_t, 12> maxWinProbabilities = {
  MIDDLE_HIGH_TAL_MAX,      MIDDLE_TAL_MAX,       MIDDLE_LOW_TAL_MAX,        LOW_TAL_MAX,
  CAPABLANCA_TAL_MAX,       CAPABLANCA_MAX,       CAPABLANCA_PETROSIAN_MAX,  LOW_PETROSIAN_MAX,
  MIDDLE_LOW_PETROSIAN_MAX, MIDDLE_PETROSIAN_MAX, MIDDLE_HIGH_PETROSIAN_MAX, HIGH_PETROSIAN_MAX};

uint8_t get_handicap_max_win_probability(uint8_t wp) {
    auto it = std::lower_bound(maxWinProbabilities.begin(), maxWinProbabilities.end(), wp,
                               std::greater<>());
    return (it != maxWinProbabilities.end() - 1) ? *(it + 1)
                                                 : static_cast<uint8_t>(HIGH_PETROSIAN_MAX);
}

constexpr std::array<uint8_t, 12> minWinProbabilities = {
  HIGH_PETROSIAN_MAX, MIDDLE_HIGH_PETROSIAN_MAX, MIDDLE_PETROSIAN_MAX, MIDDLE_LOW_PETROSIAN_MAX,
  LOW_PETROSIAN_MAX,  CAPABLANCA_PETROSIAN_MAX,  CAPABLANCA_MAX,       CAPABLANCA_TAL_MAX,
  LOW_TAL_MAX,        MIDDLE_LOW_TAL_MAX,        MIDDLE_TAL_MAX,       MIDDLE_HIGH_TAL_MAX};

uint8_t get_handicap_min_win_probability(uint8_t wp) {
    for (size_t i = 0; i < minWinProbabilities.size() - 2; ++i)
    {
        if (wp <= minWinProbabilities[i])
            return minWinProbabilities[i + 1] + 1;
    }
    return MIDDLE_HIGH_TAL_MAX + 1;
}

void initHandicapMinMaxValueThresholds() {
    min_max_threshold.resize(WDLModel::MAX_WIN_PROBABILITY + 1, {4000, -4000});
    for (int wp = 0; wp <= WDLModel::MAX_WIN_PROBABILITY; ++wp)
    {
        int     minValue = 4000;
        int     maxValue = -4000;
        uint8_t max_wp   = get_handicap_max_win_probability(wp);
        uint8_t min_wp   = get_handicap_min_win_probability(wp);

        for (int materialClamp = 17; materialClamp <= 78; ++materialClamp)
        {
            for (int valueClamp = 4000; valueClamp >= -4000; --valueClamp)
            {
                if (WDLModel::get_win_probability_by_material(valueClamp, materialClamp) == max_wp)
                {
                    if (valueClamp > maxValue)
                    {
                        maxValue = valueClamp;
                    }
                }
                if (WDLModel::get_win_probability_by_material(valueClamp, materialClamp) == min_wp)
                {
                    if (valueClamp < minValue)
                    {
                        minValue = valueClamp;
                    }
                }
            }
        }

        min_max_threshold[wp] = {minValue, maxValue};
    }
}
Value get_handicap_value(Value baseEvaluation, int uciElo, const Position& pos) {
    bool complexPosition = isComplex(pos);

    struct HandicapErrorRange {
        int minErrorMagnitude;
        int maxErrorMagnitude;
    };

    HandicapErrorRange handicapErrorRanges[] = {
      {MIN_ERROR_MAGNITUDE_BEGINNER, MAX_ERROR_MAGNITUDE_BEGINNER},
      {MIN_ERROR_MAGNITUDE_INTERMEDIATE, MAX_ERROR_MAGNITUDE_INTERMEDIATE},
      {MIN_ERROR_MAGNITUDE_ADVANCED, MAX_ERROR_MAGNITUDE_ADVANCED},
      {MIN_ERROR_MAGNITUDE_EXPERT, MAX_ERROR_MAGNITUDE_EXPERT}};

    HandicapErrorRange selectedRange;
    if (uciElo <= BEGINNER_MAX_ELO)
        selectedRange = handicapErrorRanges[0];
    else if (uciElo <= INTERMEDIATE_MAX_ELO)
        selectedRange = handicapErrorRanges[1];
    else if (uciElo <= ADVANCED_MAX_ELO)
        selectedRange = handicapErrorRanges[2];
    else
        selectedRange = handicapErrorRanges[3];

    // **Gradual transition with sigmoid to reduce the error near the max elo**
    int eloMin = (uciElo <= BEGINNER_MAX_ELO)     ? MIN_ELO
               : (uciElo <= INTERMEDIATE_MAX_ELO) ? BEGINNER_MAX_ELO
               : (uciElo <= ADVANCED_MAX_ELO)     ? INTERMEDIATE_MAX_ELO
                                                  : ADVANCED_MAX_ELO;
    int eloMax = (uciElo <= BEGINNER_MAX_ELO)     ? BEGINNER_MAX_ELO
               : (uciElo <= INTERMEDIATE_MAX_ELO) ? INTERMEDIATE_MAX_ELO
               : (uciElo <= ADVANCED_MAX_ELO)     ? ADVANCED_MAX_ELO
                                                  : MAX_ELO;

    double progress = 1.0 / (1.0 + std::exp(-(uciElo - (eloMin + eloMax) / 2.0) / 50.0));

    // **Error coefficient to apply based on the elo**
    double errorCoefficient = get_error_coefficient(uciElo) * (1.0 - progress);

    int minError = static_cast<int>(selectedRange.minErrorMagnitude * errorCoefficient);
    int maxError = static_cast<int>(selectedRange.maxErrorMagnitude * errorCoefficient);

    std::uniform_int_distribution<> errorDis(minError, maxError);
    int                             errorMagnitude = errorDis(tls_rng);

    // **Amplify the error in complex positions**
    if (complexPosition)
        errorMagnitude = static_cast<int>(errorMagnitude * COMPLEX_POSITION_MULTIPLIER);

    // **Gradual reduction based on the game phase**
    int gamePly         = pos.game_ply();
    int openingLimit    = get_dynamic_phase_limit(uciElo, true);
    int middlegameLimit = get_dynamic_phase_limit(uciElo, false);

    if (gamePly < openingLimit)
    {
        std::uniform_int_distribution<> openingDis(-errorMagnitude / 3, errorMagnitude / 3);
        errorMagnitude += openingDis(tls_rng);
    }
    else if (gamePly <= middlegameLimit)
    {
        std::uniform_int_distribution<> middlegameDis(-errorMagnitude / 4, errorMagnitude / 4);
        errorMagnitude += middlegameDis(tls_rng);
    }
    else
    {
        std::uniform_int_distribution<> endgameDis(-errorMagnitude / 3, errorMagnitude / 3);
        errorMagnitude -= endgameDis(tls_rng);

        //gradual redduction
        double endgameFactor = (uciElo <= BEGINNER_MAX_ELO)     ? 4.0
                             : (uciElo <= INTERMEDIATE_MAX_ELO) ? 3.2
                             : (uciElo <= ADVANCED_MAX_ELO)     ? 2.3
                                                                : 1.5;
        errorMagnitude       = static_cast<int>(errorMagnitude * endgameFactor);
    }

    // **Random decision about the error direction**
    std::uniform_int_distribution<> decisionDis(0, 99);
    bool increaseEvaluation = (decisionDis(tls_rng) < (uciElo < 1600 ? 15 : 5));

    baseEvaluation += (increaseEvaluation ? errorMagnitude : -errorMagnitude);

    // **Clamp final to ensure the value remains valid**
    uint8_t wp = WDLModel::get_win_probability(baseEvaluation, pos);
    baseEvaluation =
      std::clamp(baseEvaluation, min_max_threshold[wp].min_value, min_max_threshold[wp].max_value);

    return baseEvaluation;
}

bool isComplex(const Position& pos) {
    size_t legalMoveCount = MoveList<LEGAL>(pos).size();
    bool   highMaterial   = pos.non_pawn_material(WHITE) + pos.non_pawn_material(BLACK) > 2400;
    bool   kingDanger     = Shashin::king_danger(pos, WHITE) || Shashin::king_danger(pos, BLACK);
    ShashinManager shashinManager;
    bool           pawnsNearPromotion = shashinManager.isPawnNearPromotion(pos);
    return ShashinManager::isComplexPosition(legalMoveCount, highMaterial, kingDanger,
                                             pawnsNearPromotion);
}

// Function to compute the adjusted complexity factor based on Elo and game phase
double computeAdjustedComplexityFactor(int uciElo, const Position& pos) {
    // Complexity factor based on position
    double complexityFactor = isComplex(pos) ? 1.0 : 0.5;

    // Factor based on Elo (lower Elo => higher weight for complexity)
    double eloFactor = std::clamp(1.0 - (uciElo - MIN_ELO) / 1870.0, 0.3, 1.0);

    // Factor for opening phase (less complexity weight in early moves)
    double openingFactor = std::clamp(1.0 - (pos.game_ply() / 40.0), 0.3, 1.0);

    // Adjust complexity factor using Elo and opening phase
    return complexityFactor * openingFactor * eloFactor;
}

// Function to decide whether to apply perturbation based on Elo, material, and position complexity
bool should_apply_perturbation(int uciElo, const Position& pos) {
    double complexityFactor = computeAdjustedComplexityFactor(uciElo, pos);
    int    material         = pos.count<PAWN>() + 3 * pos.count<KNIGHT>() + 3 * pos.count<BISHOP>()
                 + 5 * pos.count<ROOK>() + 9 * pos.count<QUEEN>();
    int materialClamp = std::clamp(material, 17, 78);

    struct EloRange {
        int MIN_ELO, MAX_ELO;
        int baseThreshold, minThreshold;
    };

    constexpr EloRange eloRanges[] = {{MIN_ELO, BEGINNER_MAX_ELO, 99, 90},
                                      {BEGINNER_MAX_ELO + 1, INTERMEDIATE_MAX_ELO, 98, 85},
                                      {INTERMEDIATE_MAX_ELO + 1, ADVANCED_MAX_ELO, 95, 75},
                                      {ADVANCED_MAX_ELO + 1, MAX_ELO, 75, 55}};

    for (const auto& range : eloRanges)
    {
        if (uciElo >= range.MIN_ELO && uciElo <= range.MAX_ELO)
        {
            // Se l'elo è esattamente il massimo della fascia, nessuna perturbazione
            if (uciElo == range.MAX_ELO)
                return false;

            // Transizione graduale con una sigmoide invece di lineare
            double midElo = (range.MIN_ELO + range.MAX_ELO) / 2.0;
            double t      = 1.0 / (1.0 + std::exp(-(uciElo - midElo) / 50.0));

            int baseThreshold = range.baseThreshold * (1 - t) + range.minThreshold * t;

            // Fattori di aggiustamento per la posizione
            int complexityAdjustment = static_cast<int>(complexityFactor * 10);
            int materialAdjustment   = (materialClamp - 17) / 6;

            int finalThreshold = std::max(range.minThreshold, baseThreshold - complexityAdjustment
                                                                - materialAdjustment);

            return std::uniform_int_distribution<>(0, 100)(tls_rng) < finalThreshold;
        }
    }

    return false;  // Non dovrebbe mai succedere
}


// Main function
Value get_perturbated_value(const Position& pos, Value baseEvaluation) {
    // Verifica se applicare la perturbazione
    if (should_apply_perturbation(handicapConfig.uciElo, pos))
    {
        baseEvaluation = get_handicap_value(baseEvaluation, handicapConfig.uciElo, pos);
    }

    return baseEvaluation;
}

// init the true handicap mode
void initHandicapMode(const OptionsMap& options) {
    //true handicap mode begin
    handicapConfig.limitStrength = options["UCI_LimitStrength"] || options["LimitStrength_CB"];
    handicapConfig.uciElo        = handicapConfig.limitStrength
                                   ? std::min({(int) options["UCI_Elo"], (int) options["ELO_CB"], MAX_ELO})
                                   : MAX_ELO;
    handicapConfig.pawnsToEvaluate =
      handicapConfig.limitStrength ? (handicapConfig.uciElo > BEGINNER_MAX_ELO) : 1;
    handicapConfig.winnableToEvaluate =
      handicapConfig.limitStrength ? (handicapConfig.uciElo > INTERMEDIATE_MAX_ELO) : 1;
    handicapConfig.imbalancesToEvaluate =
      handicapConfig.limitStrength ? (handicapConfig.uciElo > ADVANCED_MAX_ELO) : 1;
    handicapConfig.simulateHumanBlunders =
      handicapConfig.limitStrength ? (bool) options["Simulate human blunders"] : false;
    handicapConfig.handicappedDepth = options["Handicapped Depth"];
    initHandicapMinMaxValueThresholds();
    //true handicap mode end
}
// load() reads avatar values
void loadAvatar(const std::string& fname) {

    if (fname.empty())
    {
        std::cerr << "Avatar file name is empty. Skipping loading." << std::endl;
        return;
    }

    std::ifstream file(fname);
    if (!file)
    {
        std::cerr << "Warning: Unable to open avatar file: " << Util::map_path(fname)
                  << ". Proceeding without it." << std::endl;
        return;
    }

    using WeightsMap = std::map<std::string, int, CaseInsensitiveLess>;
    WeightsMap weightsProperties;

    //Read weights from Avatar file into a map
    std::string line;
    while (std::getline(file, line))
    {

        size_t delimiterPos;
        if (line.empty() || line[0] == '#' || (delimiterPos = line.find('=')) == std::string::npos)
        {
            continue;  // Ignora righe vuote o commenti
        }

        std::string wName  = line.substr(0, delimiterPos);
        std::string wValue = line.substr(delimiterPos + 1);

        try
        {
            int value = std::stoi(wValue);
            if (value < 0 || value > 100)
            {
                throw std::out_of_range("Value out of valid range (0-100)");
            }
            weightsProperties[wName] = value;
        } catch (const std::invalid_argument&)
        {
            std::cerr << "Warning: Avatar option '" << wName
                      << "' contains an invalid number: " << wValue << std::endl;
            continue;  // Salta questo valore e continua il caricamento
        } catch (const std::out_of_range&)
        {
            std::cerr << "Warning: Avatar option '" << wName
                      << "' has a value out of range: " << wValue << std::endl;
            continue;
        }
    }

    file.close();
    if (!fname.empty())
    {
        sync_cout << "info string Avatar file " << fname << " loaded successfully" << sync_endl;
    }
    //Assign to Weights array
    WeightsMap::const_iterator it;
    for (int i = 0; i < AVATAR_NB; ++i)
    {
        if ((it = weightsProperties.find(Weights[i].mgName)) != weightsProperties.end())
            Weights[i].mg = it->second;

        if ((it = weightsProperties.find(Weights[i].egName)) != weightsProperties.end())
            Weights[i].eg = it->second;
    }
}
}  // namespace Eval
}  // namespace Alexander