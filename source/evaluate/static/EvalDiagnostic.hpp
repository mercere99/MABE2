/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021-2022.
 *
 *  @file  EvalDiagnostic.hpp
 *  @brief MABE Evaluation module for counting the number of ones (or zeros) in an output.
 */

#ifndef MABE_EVAL_DIAGNOSTIC_H
#define MABE_EVAL_DIAGNOSTIC_H

#include "../../core/MABE.hpp"
#include "../../core/Module.hpp"

namespace mabe {

  class EvalDiagnostic : public Module {
  private:
    std::string vals_trait = "vals";           // Set of values to evaluate
    std::string scores_trait = "scores";       // Vector of scores for each value
    std::string total_trait = "total";         // A single value totalling all of the scores.
    std::string first_trait = "first_active";  // Location of first activation position.
    std::string count_trait = "active_count";  // Number of activation positions.
    size_t num_vals = 100;                     // Cardinality of the problem space.

    // Track the DataMap ID for each trait.
    size_t vals_id = static_cast<size_t>(-1);
    size_t scores_id = static_cast<size_t>(-1);
    size_t total_id = static_cast<size_t>(-1);
    size_t first_id = static_cast<size_t>(-1);
    size_t count_id = static_cast<size_t>(-1);

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
      : Module(control, name, desc)
    {
      SetEvaluateMod(true);
    }
    ~EvalDiagnostic() { }

    // Setup member functions associated with this class.
    static void InitType(emplode::TypeInfo & info) {
      info.AddMemberFunction(
        "EVAL",
        [](EvalDiagnostic & mod, Collection orgs) { return mod.Evaluate(orgs); },
        "Evaluate organisms using the specified diagnostic."
      );
    }

    void SetupConfig() override {
      LinkVar(vals_trait, "vals_trait", "Trait that stores the values to evaluate");
      LinkVar(num_vals, "N", "Cardinality of the problem (number of values to analyze)");
      LinkVar(scores_trait, "scores_trait", "Trait to store activated scores");
      LinkVar(total_trait, "total_trait", "Trait to store total score");
      LinkVar(first_trait, "first_trait", "Trait to store first activation position");
      LinkVar(count_trait, "count_trait", "Trait to store count of activation positions");
      LinkMenu(diagnostic_id, "diagnostic", "Which Diagnostic should we use?",
               EXPLOIT,        "exploit",        "Fitness = sum of all values",
               STRUCT_EXPLOIT, "struct_exploit", "Fitness = sum of descending values from start",
               EXPLORE,        "explore",        "Fitness = sum of descending values from max",
               DIVERSITY,      "diversity",      "Fitness = max value minus all others",
               WEAK_DIVERSITY, "weak_diversity", "Fitness = max value"
      );
    }

    void SetupModule() override {
      AddRequiredTrait<double>(vals_trait, num_vals);
      AddOwnedTrait<double>(scores_trait, "Individual scores for current diagnostic.", 0.0, num_vals);
      AddOwnedTrait<double>(total_trait, "Combined score for current diagnostic.", 0.0);
      AddOwnedTrait<size_t>(first_trait, "First activated position.", 0.0);
      AddOwnedTrait<size_t>(count_trait, "Total number of activated positions.", 0.0);
    }

    double Evaluate(Collection orgs) {
      // If we haven't calculated the IDs, do so now.
      if (vals_id == static_cast<size_t>(-1)) {
        vals_id = orgs.GetDataLayout().GetID(vals_trait);
        scores_id = orgs.GetDataLayout().GetID(scores_trait);
        total_id = orgs.GetDataLayout().GetID(total_trait);
        first_id = orgs.GetDataLayout().GetID(first_trait);
        count_id = orgs.GetDataLayout().GetID(count_trait);
      }

      // Track the organism with the highest total score.
      double max_total = 0.0;
      emp::Ptr<Organism> max_org = nullptr;

      // Loop through the living organisms in the target collection to evaluate each.
      mabe::Collection alive_collect( orgs.GetAlive() );
      for (Organism & org : alive_collect) {        
        // Make sure this organism has its values ready for us to access.
        org.GenerateOutput();

        // Get access to the data_map elements that we need.
        const emp::vector<double> & vals = org.GetTrait<emp::vector<double>>(vals_id);
        emp::vector<double> & scores = org.GetTrait<emp::vector<double>>(scores_id);
        double & total_score = org.GetTrait<double>(total_id);
        size_t & first_active = org.GetTrait<size_t>(first_id);
        size_t & active_count = org.GetTrait<size_t>(count_id);

        // Initialize output values.
        scores.resize(vals.size());
        total_score = 0.0;
        size_t pos = 0;

        // Determine the scores based on the diagnostic type that we're using.
        switch (diagnostic_id) {
        case EXPLOIT:
          scores = vals;
          for (double x : scores) total_score += x;
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
          while (pos < scores.size()) { scores[pos] = 0.0; ++pos; }
          break;
        case EXPLORE:
          // Start at highest value (clearing everything before it)
          pos = emp::FindMaxIndex(vals);  // Find the position to start.
          for (size_t i = 0; i < pos; i++) scores[i] = 0.0;

          total_score = scores[pos] = vals[pos];
          first_active = pos;
          pos++;

          // Use values as long as they are monotonically decreasing.
          while (pos < vals.size() && vals[pos] <= vals[pos-1]) {
            total_score += (scores[pos] = vals[pos]);
            pos++;
          }
          active_count = pos - first_active;

          // Clear out the remaining values.
          while (pos < scores.size()) { scores[pos] = 0.0; ++pos; }

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
  };

  MABE_REGISTER_MODULE(EvalDiagnostic, "Evaluate set of values with a specified diagnostic problem.");
}

#endif
