/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021-2022.
 *
 *  @file  EvalDiagnostic.hpp
 *  @brief MABE Evaluation module for counting the number of ones (or zeros) in an output.
 * 
 *  Developer notes:
 *  - Can allow vals_trait to also be a vector.
 */

#ifndef MABE_EVAL_DIAGNOSTIC_H
#define MABE_EVAL_DIAGNOSTIC_H

#include <emp/math/constants.hpp>

#include "../../core/MABE.hpp"
#include "../../core/Module.hpp"

namespace mabe {

  class EvalDiagnostic : public Module {
  private:
    size_t num_vals = 100;                     // Cardinality of the problem space.
    RequiredMultiTrait<double> vals_trait{this, "vals", "Set of values to evaluate.", AsConfig(num_vals)};
    OwnedMultiTrait<double> scores_trait{this, "scores", "Set of scores for each value.", AsConfig(num_vals)};
    OwnedTrait<double> total_trait{this, "total", "A single value totalling all scores."};
    OwnedTrait<size_t> first_trait{this, "first", "Location of first active positions."};
    OwnedTrait<size_t> active_count_trait{this, "active_count", "Number of activation positions."};

    enum Type {
      EXPLOIT,                  // Must drive values as close to 100 as possible.
      STRUCT_EXPLOIT,           // Start at first value; only count values smaller than previous.
      EXPLORE,                  // Start at max value; keep counting values if less than previous.
      DIVERSITY,                // ONLY count max value; all others are max - their current value.
      WEAK_DIVERSITY,           // ONLY count max value; all others don't count (and can drift)
      NUM_DIAGNOSTICS,
      UNKNOWN
    };

    Type diagnostic_id;

  public:
    EvalDiagnostic(mabe::MABE & control,
                   const std::string & name="EvalDiagnostic",
                   const std::string & desc="Evaluate value sets using a specified diagnostic.")
      : Module(control, name, desc) { SetEvaluateMod(true); }
    ~EvalDiagnostic() { }

    // Setup member functions associated with this class.
    static void InitType(emplode::TypeInfo & info) {
      info.AddMemberFunction(
        "EVAL",
        [](EvalDiagnostic & mod, Collection orgs) { return mod.Evaluate(orgs); },
        "Evaluate organisms using the specified diagnostic."
      );
      info.AddMemberFunction(
        "COLLECTIVE_SCORE",
        [](EvalDiagnostic & mod, Collection orgs) { return mod.CalcCollectiveScore(orgs); },
        "Sum the best scores in the whole population, trait-by-trait."
      );
      info.AddMemberFunction(
        "LOWEST_ACTIVE",
        [](EvalDiagnostic & mod, Collection orgs) { return mod.FindLowestActive(orgs); },
        "Determine the earliest active position."
      );
    }

    void SetupConfig() override {
      LinkVar(num_vals, "N", "Cardinality of the problem (number of values to analyze)");
      LinkMenu(diagnostic_id, "diagnostic", "Which Diagnostic should we use?",
               EXPLOIT,        "exploit",        "Fitness = sum of all values",
               STRUCT_EXPLOIT, "struct_exploit", "Fitness = sum of descending values from start",
               EXPLORE,        "explore",        "Fitness = sum of descending values from max",
               DIVERSITY,      "diversity",      "Fitness = max value minus all others",
               WEAK_DIVERSITY, "weak_diversity", "Fitness = max value"
      );
    }

    void SetupModule() override {
      // Nothing needed here yet...
    }

    double Evaluate(Collection orgs) {
      // Track the organism with the highest total score.
      double max_total = 0.0;
      emp::Ptr<Organism> max_org = nullptr;

      // Loop through the living organisms in the target collection to evaluate each.
      mabe::Collection alive_orgs( orgs.GetAlive() );
      for (Organism & org : alive_orgs) {        
        // Make sure this organism has its values ready for us to access.
        org.GenerateOutput();

        // Get access to the data_map elements that we need.
        std::span<double> vals = vals_trait(org);
        std::span<double> scores = scores_trait(org);
        emp_assert(vals.size() == scores.size());

        double & total_score = total_trait(org);
        size_t & first_active = first_trait(org);
        size_t & active_count = active_count_trait(org);

        // Initialize output values.
        total_score = 0.0;
        size_t pos = 0;

        // Determine the scores based on the diagnostic type that we're using.
        switch (diagnostic_id) {
        case EXPLOIT:
          std::copy(vals.begin(), vals.end(), scores.begin());
          total_score = std::accumulate(scores.begin(), scores.end(), 0.0);
          first_active = 0;
          active_count = vals.size();
          break;
        case STRUCT_EXPLOIT:
          total_score = scores[0] = vals[0];
          first_active = 0;

          // Use values as long as they are monotonically decreasing.
          for (pos = 1; pos < vals.size() && vals[pos] <= vals[pos-1]; ++pos) {
            total_score += (scores[pos] = vals[pos]);
          }
          active_count = pos;

          // Clear out the remaining values.
          std::fill(scores.begin()+pos, scores.end(), 0.0);
          break;
        case EXPLORE:
          // Start at highest value (clearing everything before it)
          pos = emp::FindMaxIndex(vals);  // Find the position to start.
          std::fill(scores.begin(), scores.begin()+pos, 0.0);

          total_score = scores[pos] = vals[pos];
          first_active = pos;
          pos++;

          // Use values as long as they are monotonically non-increasing.
          while (pos < vals.size() && vals[pos] <= vals[pos-1]) {
            total_score += (scores[pos] = vals[pos]);
            pos++;
          }
          active_count = pos - first_active;

          // Clear out the remaining values.
          std::fill(scores.begin()+pos, scores.end(), 0.0);

          break;
        case DIVERSITY:
          // Only count highest value
          pos = emp::FindMaxIndex(vals);  // Find the sole active position.
          total_score = scores[pos] = vals[pos];
          first_active = pos;
          active_count = 1;

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
          first_active = pos;
          active_count = 1;

          // Clear all other schores.
          for (size_t i = 0; i < vals.size(); i++) {
            if (i != pos) scores[i] = 0.0;
          }

          break;
        default:
          emp_error("Unknown Diganostic.");
        }

        if (total_score > max_total || !max_org) {
          max_total = total_score;
          max_org = &org;
        }
      }
      return max_total;
    }

    double CalcCollectiveScore(Collection orgs) const {
      mabe::Collection alive_orgs( orgs.GetAlive() );
      emp::vector<double> best_scores(num_vals, 0.0);
      for (Organism & org : alive_orgs) {
        std::span<double> scores = scores_trait(org);
        for (size_t i = 0; i < scores.size(); ++i) {
          if (scores[i] > best_scores[i]) best_scores[i] = scores[i];
        }
      }
      double total_score = std::accumulate(best_scores.begin(), best_scores.end(), 0.0);
      return total_score;
    }

    double FindLowestActive(Collection orgs) const {
      mabe::Collection alive_orgs( orgs.GetAlive() );
      size_t lowest_active = num_vals;
      for (Organism & org : alive_orgs) {
        // Get access to the data_map elements that we need.
        std::span<double> vals = vals_trait(org);
        size_t pos = emp::FindMaxIndex(vals);  // Find the first active position
        if (pos < lowest_active) lowest_active = pos;
      }

      return lowest_active;
    }
  };

  MABE_REGISTER_MODULE(EvalDiagnostic, "Evaluate set of values with a specified diagnostic problem.");
}

#endif
