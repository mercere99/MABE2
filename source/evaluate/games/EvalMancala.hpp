/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021.
 *
 *  @file  EvalMancala.hpp
 *  @brief MABE Evaluation module that has organisms play Mancala.
 */

#ifndef MABE_EVAL_MANCALA_HPP
#define MABE_EVAL_MANCALA_HPP

#include "emp/games/Mancala.hpp"

#include "../../core/MABE.hpp"
#include "../../core/Module.hpp"

namespace mabe {

  class EvalMancala : public Module {
  private:
    Collection target_collect;  // Which organisms should we evaluate?

    std::string input_trait;    // Name of trait to put input values.
    std::string output_trait;   // Name of trait to find output values.
    std::string score_trait;    // Trait to indicate game results.

    /// What type of opponent should we use?
    enum Opponent {
      RANDOM_MOVES,     // Opponent will always choose a random, legal move.
      AI,               // Opponent is a human-crafted AI.
      RANDOM_ORG,       // Opponent is a random organism from the population.
      UNKNOWN
    };

    Opponent opponent_type;

  public:
    EvalMancala(mabe::MABE & control,
                   const std::string & name="EvalMancala",
                   const std::string & desc="Evaluate organisms by having them play Mancala.",
                   const std::string & _itrait="input",
                   const std::string & _otrait="output",
                   const std::string & _strait="score")
      : Module(control, name, desc)
      , target_collect(control.GetPopulation(0))
      , input_trait(_itrait)
      , output_trait(_otrait)
      , score_trait(_strait)
    {
      SetEvaluateMod(true);
    }
    ~EvalMancala() { }

    void SetupConfig() override {
      LinkCollection(target_collect, "target", "Which population(s) should we evaluate?");
      LinkVar(input_trait, "input_trait", "Into which trait should input values be placed?");
      LinkVar(output_trait, "output_trait", "Out of which trait should output values be read?");
      LinkVar(score_trait, "score_trait", "Which trait should we store success rating?");
      LinkMenu(opponent_type, "opponent_type", "Which type of opponent should organisms face?",
               RANDOM_MOVES, "random_moves", "Always choose a random, legal move.",
               AI, "ai", "Human supplied (but not very good) AI",
               RANDOM_ORG, "random_org", "Pick another random organism from collection."
      );
    }

    void SetupModule() override {
      AddOwnedTrait<emp::vector<double>>(input_trait, "Input values (curret board state)", emp::vector<double>({0.0}));
      AddRequiredTrait<emp::vector<double>>(output_trait); // Output values (move to make)
      AddOwnedTrait<double>(score_trait, "Play success", 0.0);
    }


    // Determine the next move of an organism.
    size_t EvalMove(emp::Mancala & game, Organism & org) {
      // Setup the hardware with proper inputs.
      org.GetVar<emp::vector<double>>(input_trait) = game.AsVectorInput(game.GetCurPlayer());

      // Run the code.
      org.GenerateOutput();

      emp::vector<double> results = org.GetVar<emp::vector<double>>(output_trait);

      // Determine the chosen move.
      size_t best_move = 0;
      for (int i = 1; i < 6; i++) {
        if (results[best_move] < results[i]) { best_move = i; }
      }

      return best_move;
    }


    void OnUpdate(size_t /* update */) override {
      emp_assert(control.GetNumPopulations() >= 1);

      // Loop through the living organisms in the target collection to evaluate each.
      mabe::Collection alive_collect( target_collect.GetAlive() );
      for (Organism & org : alive_collect) {

        // Make sure this organism has its values ready for us to access.
        org.GenerateOutput();

        // Get access to the data_map elements that we need.
        const emp::vector<double> & vals = org.GetVar<emp::vector<double>>(vals_trait);
        emp::vector<double> & scores = org.GetVar<emp::vector<double>>(scores_trait);
        double & total_score = org.GetVar<double>(total_trait);

        // Initialize output values.
        scores.resize(vals.size());
        total_score = 0.0;
        size_t pos = 0;

        // Determine the scores based on the diagnostic type that we're using.
        switch (diagnostic_id) {
        case EXPLOIT:
          scores = vals;
          for (double x : scores) total_score += x;
          break;
        case STRUCT_EXPLOIT:
          total_score = scores[0] = vals[0];

          // Use values as long as they are monotonically decreasing.
          for (pos = 1; pos < vals.size() && vals[pos] <= vals[pos-1]; ++pos) {
            total_score += (scores[pos] = vals[pos]);
          }

          // Clear out the remaining values.
          while (pos < scores.size()) { scores[pos] = 0.0; ++pos; }
          break;
        case EXPLORE:
          // Start at highest value (clearing everything before it)
          pos = emp::FindMaxIndex(vals);  // Find the position to start.
          for (size_t i = 0; i < pos; i++) scores[i] = 0.0;

          total_score = scores[pos] = vals[pos];
          pos++;

          // Use values as long as they are monotonically decreasing.
          while (pos < vals.size() && vals[pos] <= vals[pos-1]) {
            total_score += (scores[pos] = vals[pos]);
            pos++;
          }

          // Clear out the remaining values.
          while (pos < scores.size()) { scores[pos] = 0.0; ++pos; }

          break;
        case DIVERSITY:
          // Only count highest value
          pos = emp::FindMaxIndex(vals);  // Find the position to start.
          total_score = scores[pos] = vals[pos];

          // All others are subtracted from max and divided by two, creating a
          // pressure to minimize.
          for (size_t i = 0; i < vals.size(); i++) {
            if (i != pos) total_score += (scores[i] = (vals[pos] - vals[i]) / 2.0);
          }

          break;
        case WEAK_DIVERSITY:
          // Only count highest value
          pos = emp::FindMaxIndex(vals);  // Find the position to start.
          total_score = scores[pos] = vals[pos];

          // Clear all other schores.
          for (size_t i = 0; i < vals.size(); i++) {
            if (i != pos) scores[i] = 0.0;
          }

          break;
        default:
          emp_error("Unknown Diganostic.");
        }

      }
    }
  };

  MABE_REGISTER_MODULE(EvalMancala, "Evaluate organisms on their ability to play Mancala.");
}

#endif
